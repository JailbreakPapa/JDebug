/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Profiling/Profiling.h>
#include <GuiFoundation/Action/ActionMapManager.h>
#include <GuiFoundation/Action/DocumentActions.h>
#include <GuiFoundation/ActionViews/MenuActionMapView.moc.h>
#include <GuiFoundation/ActionViews/MenuBarActionMapView.moc.h>
#include <GuiFoundation/ActionViews/QtProxy.moc.h>
#include <GuiFoundation/ContainerWindow/ContainerWindow.moc.h>
#include <GuiFoundation/DocumentWindow/DocumentWindow.moc.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <QDockWidget>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QStatusBar>
#include <QTimer>
#include <ToolsFoundation/Document/Document.h>
#include <ads/DockWidget.h>

nsEvent<const nsQtDocumentWindowEvent&> nsQtDocumentWindow::s_Events;
nsDynamicArray<nsQtDocumentWindow*> nsQtDocumentWindow::s_AllDocumentWindows;
bool nsQtDocumentWindow::s_bAllowRestoreWindowLayout = true;

void nsQtDocumentWindow::Constructor()
{
  s_AllDocumentWindows.PushBack(this);

  // status bar
  {
    connect(statusBar(), &QStatusBar::messageChanged, this, &nsQtDocumentWindow::OnStatusBarMessageChanged);

    m_pPermanentDocumentStatusText = new QLabel();
    statusBar()->addWidget(m_pPermanentDocumentStatusText, 1);

    m_pPermanentGlobalStatusButton = new QToolButton();
    m_pPermanentGlobalStatusButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_pPermanentGlobalStatusButton->setVisible(false);
    statusBar()->addPermanentWidget(m_pPermanentGlobalStatusButton, 0);

    NS_VERIFY(connect(m_pPermanentGlobalStatusButton, &QToolButton::clicked, this, &nsQtDocumentWindow::OnPermanentGlobalStatusClicked), "");
  }

  setDockNestingEnabled(true);

  nsQtMenuBarActionMapView* pMenuBar = new nsQtMenuBarActionMapView(this);
  setMenuBar(pMenuBar);

  nsToolsProject::SuggestContainerWindow(m_pDocument);
  nsQtContainerWindow* pContainer = nsQtContainerWindow::GetContainerWindow();
  pContainer->AddDocumentWindow(this);

  nsQtUiServices::s_Events.AddEventHandler(nsMakeDelegate(&nsQtDocumentWindow::UIServicesEventHandler, this));
  nsQtUiServices::s_TickEvent.AddEventHandler(nsMakeDelegate(&nsQtDocumentWindow::UIServicesTickEventHandler, this));
}

nsQtDocumentWindow::nsQtDocumentWindow(nsDocument* pDocument)
{
  m_pDocument = pDocument;
  m_sUniqueName = m_pDocument->GetDocumentPath();
  setObjectName(GetUniqueName());

  nsDocumentManager::s_Events.AddEventHandler(nsMakeDelegate(&nsQtDocumentWindow::DocumentManagerEventHandler, this));
  pDocument->m_EventsOne.AddEventHandler(nsMakeDelegate(&nsQtDocumentWindow::DocumentEventHandler, this));

  Constructor();
}

nsQtDocumentWindow::nsQtDocumentWindow(const char* szUniqueName)
{
  m_pDocument = nullptr;
  m_sUniqueName = szUniqueName;
  setObjectName(GetUniqueName());

  Constructor();
}


nsQtDocumentWindow::~nsQtDocumentWindow()
{
  nsQtUiServices::s_Events.RemoveEventHandler(nsMakeDelegate(&nsQtDocumentWindow::UIServicesEventHandler, this));
  nsQtUiServices::s_TickEvent.RemoveEventHandler(nsMakeDelegate(&nsQtDocumentWindow::UIServicesTickEventHandler, this));

  s_AllDocumentWindows.RemoveAndSwap(this);

  if (m_pDocument)
  {
    m_pDocument->m_EventsOne.RemoveEventHandler(nsMakeDelegate(&nsQtDocumentWindow::DocumentEventHandler, this));
    nsDocumentManager::s_Events.RemoveEventHandler(nsMakeDelegate(&nsQtDocumentWindow::DocumentManagerEventHandler, this));
  }
}

void nsQtDocumentWindow::SetVisibleInContainer(bool bVisible)
{
  if (m_bIsVisibleInContainer == bVisible)
    return;

  m_bIsVisibleInContainer = bVisible;
  InternalVisibleInContainerChanged(bVisible);

  if (m_bIsVisibleInContainer)
  {
    // if the window is now visible, immediately do a redraw and trigger the timers
    SlotRedraw();
    // Make sure the window gains focus as well when it becomes visible so that shortcuts will immediately work.
    setFocus();
  }
}

void nsQtDocumentWindow::SetTargetFramerate(nsInt16 iTargetFPS)
{
  if (m_iTargetFramerate == iTargetFPS)
    return;

  m_iTargetFramerate = iTargetFPS;

  if (m_iTargetFramerate != 0)
    SlotRedraw();
}

void nsQtDocumentWindow::TriggerRedraw()
{
  SlotRedraw();
}

void nsQtDocumentWindow::UIServicesTickEventHandler(const nsQtUiServices::TickEvent& e)
{
  if (e.m_Type == nsQtUiServices::TickEvent::Type::StartFrame && m_bIsVisibleInContainer)
  {
    const nsInt32 iSystemFramerate = static_cast<nsInt32>(nsMath::Round(e.m_fRefreshRate));

    nsInt32 iTargetFramerate = m_iTargetFramerate;
    if (iTargetFramerate <= 0)
      iTargetFramerate = iSystemFramerate;

    // if the application does not have focus, drastically reduce the update rate to limit CPU draw etc.
    if (QApplication::activeWindow() == nullptr)
      iTargetFramerate = nsMath::Min(10, iTargetFramerate / 4);

    // We do not hit the requested framerate directly if the system framerate can't be evenly divided. We will chose the next higher framerate.
    if (iTargetFramerate < iSystemFramerate)
    {
      nsUInt32 mod = nsMath::Max(1u, (nsUInt32)nsMath::Floor(iSystemFramerate / (double)iTargetFramerate));
      if ((e.m_uiFrame % mod) != 0)
        return;
    }

    SlotRedraw();
  }
}


void nsQtDocumentWindow::SlotRedraw()
{
  nsStringBuilder sFilename = nsPathUtils::GetFileName(this->GetUniqueName());
  NS_PROFILE_SCOPE(sFilename.GetData());
  {
    nsQtDocumentWindowEvent e;
    e.m_Type = nsQtDocumentWindowEvent::Type::BeforeRedraw;
    e.m_pWindow = this;
    s_Events.Broadcast(e, 1);
  }

  // if our window is not visible, interrupt the redrawing, and do nothing
  if (!m_bIsVisibleInContainer)
    return;

  m_bIsDrawingATM = true;
  InternalRedraw();
  m_bIsDrawingATM = false;
}

void nsQtDocumentWindow::DocumentEventHandler(const nsDocumentEvent& e)
{
  switch (e.m_Type)
  {
    case nsDocumentEvent::Type::DocumentRenamed:
    {
      m_sUniqueName = m_pDocument->GetDocumentPath();
      setObjectName(GetUniqueName());
      nsQtContainerWindow* pContainer = nsQtContainerWindow::GetContainerWindow();
      pContainer->DocumentWindowRenamed(this);

      [[fallthrough]];
    }
    case nsDocumentEvent::Type::ModifiedChanged:
    {
      nsQtDocumentWindowEvent dwe;
      dwe.m_pWindow = this;
      dwe.m_Type = nsQtDocumentWindowEvent::Type::WindowDecorationChanged;
      s_Events.Broadcast(dwe);
    }
    break;

    case nsDocumentEvent::Type::EnsureVisible:
    {
      EnsureVisible();
    }
    break;

    case nsDocumentEvent::Type::DocumentStatusMsg:
    {
      ShowTemporaryStatusBarMsg(e.m_sStatusMsg);
    }
    break;

    default:
      break;
  }
}

void nsQtDocumentWindow::DocumentManagerEventHandler(const nsDocumentManager::Event& e)
{
  switch (e.m_Type)
  {
    case nsDocumentManager::Event::Type::DocumentClosing:
    {
      if (e.m_pDocument == m_pDocument)
      {
        ShutdownDocumentWindow();
        return;
      }
    }
    break;

    default:
      break;
  }
}

void nsQtDocumentWindow::UIServicesEventHandler(const nsQtUiServices::Event& e)
{
  switch (e.m_Type)
  {
    case nsQtUiServices::Event::Type::ShowDocumentTemporaryStatusBarText:
      ShowTemporaryStatusBarMsg(nsFmt(e.m_sText), e.m_Time);
      break;

    case nsQtUiServices::Event::Type::ShowDocumentPermanentStatusBarText:
    {
      if (m_pPermanentGlobalStatusButton)
      {
        QPalette pal = palette();

        switch (e.m_TextType)
        {
          case nsQtUiServices::Event::Info:
            m_pPermanentGlobalStatusButton->setIcon(QIcon(":/GuiFoundation/Icons/Log.svg"));
            break;

          case nsQtUiServices::Event::Warning:
            pal.setColor(QPalette::WindowText, QColor(255, 100, 0));
            m_pPermanentGlobalStatusButton->setIcon(QIcon(":/GuiFoundation/Icons/Warning.svg"));
            break;

          case nsQtUiServices::Event::Error:
            pal.setColor(QPalette::WindowText, QColor(Qt::red));
            m_pPermanentGlobalStatusButton->setIcon(QIcon(":/GuiFoundation/Icons/Error.svg"));
            break;
        }

        m_pPermanentGlobalStatusButton->setPalette(pal);
        m_pPermanentGlobalStatusButton->setText(QString::fromUtf8(e.m_sText, e.m_sText.GetElementCount()));
        m_pPermanentGlobalStatusButton->setVisible(!m_pPermanentGlobalStatusButton->text().isEmpty());
      }
    }
    break;

    default:
      break;
  }
}

nsString nsQtDocumentWindow::GetDisplayNameShort() const
{
  nsStringBuilder s = GetDisplayName();
  s = s.GetFileName();

  if (m_pDocument && m_pDocument->IsModified())
    s.Append('*');

  return s;
}

void nsQtDocumentWindow::showEvent(QShowEvent* event)
{
  QMainWindow::showEvent(event);
  SetVisibleInContainer(true);
}

void nsQtDocumentWindow::hideEvent(QHideEvent* event)
{
  QMainWindow::hideEvent(event);
  SetVisibleInContainer(false);
}

bool nsQtDocumentWindow::eventFilter(QObject* obj, QEvent* e)
{
  if (e->type() == QEvent::ShortcutOverride)
  {
    // This filter is added by nsQtContainerWindow::AddDocumentWindow as that ones is the ony code path that can connect dock container to their content.
    // This filter is necessary as clicking any action in a menu bar sets the focus to the parent CDockWidget at which point further shortcuts would stop working.
    if (qobject_cast<ads::CDockWidget*>(obj))
    {
      QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);
      if (nsQtProxy::TriggerDocumentAction(m_pDocument, keyEvent))
        return true;
    }
  }
  return false;
}

bool nsQtDocumentWindow::event(QEvent* event)
{
  if (event->type() == QEvent::ShortcutOverride)
  {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
    if (nsQtProxy::TriggerDocumentAction(m_pDocument, keyEvent))
      return true;
  }
  return QMainWindow::event(event);
}

void nsQtDocumentWindow::FinishWindowCreation()
{
  ScheduleRestoreWindowLayout();
}

void nsQtDocumentWindow::ScheduleRestoreWindowLayout()
{
  QTimer::singleShot(0, this, SLOT(SlotRestoreLayout()));
}

void nsQtDocumentWindow::SlotRestoreLayout()
{
  RestoreWindowLayout();
}

void nsQtDocumentWindow::SaveWindowLayout()
{
  // This is a workaround for newer Qt versions (5.13 or so) that seem to change the state of QDockWidgets to "closed" once the parent
  // QMainWindow gets the closeEvent, even though they still exist and the QMainWindow is not yet deleted. Previously this function was
  // called multiple times, including once after the QMainWindow got its closeEvent, which would then save a corrupted state. Therefore,
  // once the parent nsQtContainerWindow gets the closeEvent, we now prevent further saving of the window layout.
  if (!m_bAllowSaveWindowLayout)
    return;

  const bool bMaximized = isMaximized();

  if (bMaximized)
    showNormal();

  nsStringBuilder sGroup;
  sGroup.Format("DocumentWnd_{0}", GetWindowLayoutGroupName());

  QSettings Settings;
  Settings.beginGroup(QString::fromUtf8(sGroup, sGroup.GetElementCount()));
  {
    // All other properties are defined by the outer container window.
    Settings.setValue("WindowState", saveState());
  }
  Settings.endGroup();
}

void nsQtDocumentWindow::RestoreWindowLayout()
{
  if (!s_bAllowRestoreWindowLayout)
    return;

  nsQtScopedUpdatesDisabled _(this);

  nsStringBuilder sGroup;
  sGroup.Format("DocumentWnd_{0}", GetWindowLayoutGroupName());

  {
    QSettings Settings;
    Settings.beginGroup(QString::fromUtf8(sGroup, sGroup.GetElementCount()));
    {
      restoreState(Settings.value("WindowState", saveState()).toByteArray());
    }
    Settings.endGroup();

    // with certain Qt versions the window state could be saved corrupted
    // if that is the case, make sure that non-closable widgets get restored to be visible
    // otherwise the user would need to delete the serialized state from the registry
    {
      for (QDockWidget* dockWidget : findChildren<QDockWidget*>())
      {
        // not closable means the user can generally not change the visible state -> make sure it is visible
        if (!dockWidget->features().testFlag(QDockWidget::DockWidgetClosable) && dockWidget->isHidden())
        {
          dockWidget->show();
        }
      }
    }
  }

  statusBar()->clearMessage();
}

void nsQtDocumentWindow::DisableWindowLayoutSaving()
{
  m_bAllowSaveWindowLayout = false;
}

nsStatus nsQtDocumentWindow::SaveDocument()
{
  if (m_pDocument)
  {
    {
      if (m_pDocument->GetUnknownObjectTypeInstances() > 0)
      {
        if (nsQtUiServices::MessageBoxQuestion("Warning! This document contained unknown object types that could not be loaded. Saving the "
                                               "document means those objects will get lost permanently.\n\nDo you really want to save this "
                                               "document?",
              QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No, QMessageBox::StandardButton::No) != QMessageBox::StandardButton::Yes)
          return nsStatus(NS_SUCCESS); // failed successfully
      }
    }

    nsStatus res = m_pDocument->SaveDocument();

    nsStringBuilder s, s2;
    s.Format("Failed to save document:\n'{0}'", m_pDocument->GetDocumentPath());
    s2.Format("Successfully saved document:\n'{0}'", m_pDocument->GetDocumentPath());

    nsQtUiServices::MessageBoxStatus(res, s, s2);

    if (res.m_Result.Failed())
    {
      ShowTemporaryStatusBarMsg("Failed to save document");
      return res;
    }

    ShowTemporaryStatusBarMsg("Document saved");
  }

  return nsStatus(NS_SUCCESS);
}

void nsQtDocumentWindow::ShowTemporaryStatusBarMsg(const nsFormatString& msg, nsTime duration)
{
  nsStringBuilder tmp;
  statusBar()->showMessage(QString::fromUtf8(msg.GetTextCStr(tmp)), (int)duration.GetMilliseconds());
}


void nsQtDocumentWindow::SetPermanentStatusBarMsg(const nsFormatString& text)
{
  if (!text.IsEmpty())
  {
    // clear temporary message
    statusBar()->clearMessage();
  }

  nsStringBuilder tmp;
  m_pPermanentDocumentStatusText->setText(QString::fromUtf8(text.GetTextCStr(tmp)));
}

void nsQtDocumentWindow::CreateImageCapture(const char* szOutputPath)
{
  NS_ASSERT_NOT_IMPLEMENTED;
}

bool nsQtDocumentWindow::CanCloseWindow()
{
  return InternalCanCloseWindow();
}

bool nsQtDocumentWindow::InternalCanCloseWindow()
{
  // I guess this is to remove the focus from other widgets like input boxes, such that they may modify the document.
  setFocus();
  clearFocus();

  if (m_pDocument && m_pDocument->IsModified())
  {
    QMessageBox::StandardButton res = nsQtUiServices::MessageBoxQuestion("Save before closing?", QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No | QMessageBox::StandardButton::Cancel, QMessageBox::StandardButton::Cancel);

    if (res == QMessageBox::StandardButton::Cancel)
      return false;

    if (res == QMessageBox::StandardButton::Yes)
    {
      nsStatus err = SaveDocument();

      if (err.Failed())
      {
        nsQtUiServices::GetSingleton()->MessageBoxStatus(err, "Saving the scene failed.");
        return false;
      }
    }
  }

  return true;
}

void nsQtDocumentWindow::CloseDocumentWindow()
{
  QMetaObject::invokeMethod(this, "SlotQueuedDelete", Qt::ConnectionType::QueuedConnection);
}

void nsQtDocumentWindow::SlotQueuedDelete()
{
  setFocus();
  clearFocus();

  if (m_pDocument)
  {
    m_pDocument->GetDocumentManager()->CloseDocument(m_pDocument);
    return;
  }
  else
  {
    ShutdownDocumentWindow();
  }
}

void nsQtDocumentWindow::OnPermanentGlobalStatusClicked(bool)
{
  nsQtUiServices::Event e;
  e.m_Type = nsQtUiServices::Event::ClickedDocumentPermanentStatusBarText;

  nsQtUiServices::GetSingleton()->s_Events.Broadcast(e);
}

void nsQtDocumentWindow::OnStatusBarMessageChanged(const QString& sNewText)
{
  QPalette pal = palette();

  if (sNewText.startsWith("Error:"))
  {
    pal.setColor(QPalette::WindowText, nsToQtColor(nsColorScheme::LightUI(nsColorScheme::Red)));
  }
  else if (sNewText.startsWith("Warning:"))
  {
    pal.setColor(QPalette::WindowText, nsToQtColor(nsColorScheme::LightUI(nsColorScheme::Yellow)));
  }
  else if (sNewText.startsWith("Note:"))
  {
    pal.setColor(QPalette::WindowText, nsToQtColor(nsColorScheme::LightUI(nsColorScheme::Blue)));
  }

  statusBar()->setPalette(pal);
}

void nsQtDocumentWindow::ShutdownDocumentWindow()
{
  SaveWindowLayout();

  InternalCloseDocumentWindow();

  nsQtDocumentWindowEvent e;
  e.m_pWindow = this;
  e.m_Type = nsQtDocumentWindowEvent::Type::WindowClosing;
  s_Events.Broadcast(e);

  InternalDeleteThis();

  e.m_Type = nsQtDocumentWindowEvent::Type::WindowClosed;
  s_Events.Broadcast(e);
}

void nsQtDocumentWindow::InternalCloseDocumentWindow() {}

void nsQtDocumentWindow::EnsureVisible()
{
  m_pContainerWindow->EnsureVisible(this).IgnoreResult();
}

void nsQtDocumentWindow::RequestWindowTabContextMenu(const QPoint& globalPos)
{
  nsQtMenuActionMapView menu(nullptr);

  nsActionContext context;
  context.m_sMapping = "DocumentWindowTabMenu";
  context.m_pDocument = GetDocument();
  context.m_pWindow = this;
  menu.SetActionContext(context);

  menu.exec(globalPos);
}

nsQtDocumentWindow* nsQtDocumentWindow::FindWindowByDocument(const nsDocument* pDocument)
{
  // Sub-documents never have a window, so go to the main document instead
  pDocument = pDocument->GetMainDocument();

  for (auto pWnd : s_AllDocumentWindows)
  {
    if (pWnd->GetDocument() == pDocument)
      return pWnd;
  }

  return nullptr;
}

nsQtContainerWindow* nsQtDocumentWindow::GetContainerWindow() const
{
  return m_pContainerWindow;
}

nsString nsQtDocumentWindow::GetWindowIcon() const
{
  if (GetDocument() != nullptr)
    return GetDocument()->GetDocumentTypeDescriptor()->m_sIcon;

  return ":/GuiFoundation/NS-logo.svg";
}

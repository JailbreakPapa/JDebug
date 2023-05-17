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

wdEvent<const wdQtDocumentWindowEvent&> wdQtDocumentWindow::s_Events;
wdDynamicArray<wdQtDocumentWindow*> wdQtDocumentWindow::s_AllDocumentWindows;
bool wdQtDocumentWindow::s_bAllowRestoreWindowLayout = true;

void wdQtDocumentWindow::Constructor()
{
  s_AllDocumentWindows.PushBack(this);

  // status bar
  {
    connect(statusBar(), &QStatusBar::messageChanged, this, &wdQtDocumentWindow::OnStatusBarMessageChanged);

    m_pPermanentDocumentStatusText = new QLabel();
    statusBar()->addWidget(m_pPermanentDocumentStatusText, 1);

    m_pPermanentGlobalStatusButton = new QToolButton();
    m_pPermanentGlobalStatusButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_pPermanentGlobalStatusButton->setVisible(false);
    statusBar()->addPermanentWidget(m_pPermanentGlobalStatusButton, 0);

    WD_VERIFY(connect(m_pPermanentGlobalStatusButton, &QToolButton::clicked, this, &wdQtDocumentWindow::OnPermanentGlobalStatusClicked), "");
  }

  setDockNestingEnabled(true);

  wdQtMenuBarActionMapView* pMenuBar = new wdQtMenuBarActionMapView(this);
  setMenuBar(pMenuBar);

  wdInt32 iContainerWindowIndex = wdToolsProject::SuggestContainerWindow(m_pDocument);
  wdQtContainerWindow* pContainer = wdQtContainerWindow::GetContainerWindow();
  pContainer->AddDocumentWindow(this);

  wdQtUiServices::s_Events.AddEventHandler(wdMakeDelegate(&wdQtDocumentWindow::UIServicesEventHandler, this));
  wdQtUiServices::s_TickEvent.AddEventHandler(wdMakeDelegate(&wdQtDocumentWindow::UIServicesTickEventHandler, this));
}

wdQtDocumentWindow::wdQtDocumentWindow(wdDocument* pDocument)
{
  m_pDocument = pDocument;
  m_sUniqueName = m_pDocument->GetDocumentPath();
  setObjectName(GetUniqueName());

  wdDocumentManager::s_Events.AddEventHandler(wdMakeDelegate(&wdQtDocumentWindow::DocumentManagerEventHandler, this));
  pDocument->m_EventsOne.AddEventHandler(wdMakeDelegate(&wdQtDocumentWindow::DocumentEventHandler, this));

  Constructor();
}

wdQtDocumentWindow::wdQtDocumentWindow(const char* szUniqueName)
{
  m_pDocument = nullptr;
  m_sUniqueName = szUniqueName;
  setObjectName(GetUniqueName());

  Constructor();
}


wdQtDocumentWindow::~wdQtDocumentWindow()
{
  wdQtUiServices::s_Events.RemoveEventHandler(wdMakeDelegate(&wdQtDocumentWindow::UIServicesEventHandler, this));
  wdQtUiServices::s_TickEvent.RemoveEventHandler(wdMakeDelegate(&wdQtDocumentWindow::UIServicesTickEventHandler, this));

  s_AllDocumentWindows.RemoveAndSwap(this);

  if (m_pDocument)
  {
    m_pDocument->m_EventsOne.RemoveEventHandler(wdMakeDelegate(&wdQtDocumentWindow::DocumentEventHandler, this));
    wdDocumentManager::s_Events.RemoveEventHandler(wdMakeDelegate(&wdQtDocumentWindow::DocumentManagerEventHandler, this));
  }
}

void wdQtDocumentWindow::SetVisibleInContainer(bool bVisible)
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

void wdQtDocumentWindow::SetTargetFramerate(wdInt16 iTargetFPS)
{
  if (m_iTargetFramerate == iTargetFPS)
    return;

  m_iTargetFramerate = iTargetFPS;

  if (m_iTargetFramerate != 0)
    SlotRedraw();
}

void wdQtDocumentWindow::TriggerRedraw()
{
  SlotRedraw();
}

void wdQtDocumentWindow::UIServicesTickEventHandler(const wdQtUiServices::TickEvent& e)
{
  if (e.m_Type == wdQtUiServices::TickEvent::Type::StartFrame && m_bIsVisibleInContainer)
  {
    const wdInt32 iSystemFramerate = static_cast<wdInt32>(wdMath::Round(e.m_fRefreshRate));

    wdInt32 iTargetFramerate = m_iTargetFramerate;
    if (iTargetFramerate <= 0)
      iTargetFramerate = iSystemFramerate;

    // if the application does not have focus, drastically reduce the update rate to limit CPU draw etc.
    if (QApplication::activeWindow() == nullptr)
      iTargetFramerate = wdMath::Min(10, iTargetFramerate / 4);

    // We do not hit the requested framerate directly if the system framerate can't be evenly divided. We will chose the next higher framerate.
    if (iTargetFramerate < iSystemFramerate)
    {
      wdUInt32 mod = wdMath::Max(1u, (wdUInt32)wdMath::Floor(iSystemFramerate / (double)iTargetFramerate));
      if ((e.m_uiFrame % mod) != 0)
        return;
    }

    SlotRedraw();
  }
}


void wdQtDocumentWindow::SlotRedraw()
{
  wdStringBuilder sFilename = wdPathUtils::GetFileName(this->GetUniqueName());
  WD_PROFILE_SCOPE(sFilename.GetData());
  {
    wdQtDocumentWindowEvent e;
    e.m_Type = wdQtDocumentWindowEvent::Type::BeforeRedraw;
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

void wdQtDocumentWindow::DocumentEventHandler(const wdDocumentEvent& e)
{
  switch (e.m_Type)
  {
    case wdDocumentEvent::Type::ModifiedChanged:
    {
      wdQtDocumentWindowEvent dwe;
      dwe.m_pWindow = this;
      dwe.m_Type = wdQtDocumentWindowEvent::Type::WindowDecorationChanged;
      s_Events.Broadcast(dwe);
    }
    break;

    case wdDocumentEvent::Type::EnsureVisible:
    {
      EnsureVisible();
    }
    break;

    case wdDocumentEvent::Type::DocumentStatusMsg:
    {
      ShowTemporaryStatusBarMsg(e.m_szStatusMsg);
    }
    break;

    default:
      break;
  }
}

void wdQtDocumentWindow::DocumentManagerEventHandler(const wdDocumentManager::Event& e)
{
  switch (e.m_Type)
  {
    case wdDocumentManager::Event::Type::DocumentClosing:
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

void wdQtDocumentWindow::UIServicesEventHandler(const wdQtUiServices::Event& e)
{
  switch (e.m_Type)
  {
    case wdQtUiServices::Event::Type::ShowDocumentTemporaryStatusBarText:
      ShowTemporaryStatusBarMsg(wdFmt(e.m_sText), e.m_Time);
      break;

    case wdQtUiServices::Event::Type::ShowDocumentPermanentStatusBarText:
    {
      if (m_pPermanentGlobalStatusButton)
      {
        QPalette pal = palette();

        switch (e.m_TextType)
        {
          case wdQtUiServices::Event::Info:
            m_pPermanentGlobalStatusButton->setIcon(QIcon(":/GuiFoundation/Icons/Log.png"));
            break;

          case wdQtUiServices::Event::Warning:
            pal.setColor(QPalette::WindowText, QColor(255, 100, 0));
            m_pPermanentGlobalStatusButton->setIcon(QIcon(":/GuiFoundation/Icons/Warning16.png"));
            break;

          case wdQtUiServices::Event::Error:
            pal.setColor(QPalette::WindowText, QColor(Qt::red));
            m_pPermanentGlobalStatusButton->setIcon(QIcon(":/GuiFoundation/Icons/Error16.png"));
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

wdString wdQtDocumentWindow::GetDisplayNameShort() const
{
  wdStringBuilder s = GetDisplayName();
  s = s.GetFileName();

  if (m_pDocument && m_pDocument->IsModified())
    s.Append('*');

  return s;
}

void wdQtDocumentWindow::showEvent(QShowEvent* event)
{
  QMainWindow::showEvent(event);
  SetVisibleInContainer(true);
}

void wdQtDocumentWindow::hideEvent(QHideEvent* event)
{
  QMainWindow::hideEvent(event);
  SetVisibleInContainer(false);
}

bool wdQtDocumentWindow::eventFilter(QObject* obj, QEvent* e)
{
  if (e->type() == QEvent::ShortcutOverride)
  {
    // This filter is added by wdQtContainerWindow::AddDocumentWindow as that ones is the ony code path that can connect dock container to their content.
    // This filter is necessary as clicking any action in a menu bar sets the focus to the parent CDockWidget at which point further shortcuts would stop working.
    if (qobject_cast<ads::CDockWidget*>(obj))
    {
      QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);
      if (wdQtProxy::TriggerDocumentAction(m_pDocument, keyEvent))
        return true;
    }
  }
  return false;
}

bool wdQtDocumentWindow::event(QEvent* event)
{
  if (event->type() == QEvent::ShortcutOverride)
  {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
    if (wdQtProxy::TriggerDocumentAction(m_pDocument, keyEvent))
      return true;
  }
  return QMainWindow::event(event);
}

void wdQtDocumentWindow::FinishWindowCreation()
{
  ScheduleRestoreWindowLayout();
}

void wdQtDocumentWindow::ScheduleRestoreWindowLayout()
{
  QTimer::singleShot(0, this, SLOT(SlotRestoreLayout()));
}

void wdQtDocumentWindow::SlotRestoreLayout()
{
  RestoreWindowLayout();
}

void wdQtDocumentWindow::SaveWindowLayout()
{
  // This is a workaround for newer Qt versions (5.13 or so) that seem to change the state of QDockWidgets to "closed" once the parent
  // QMainWindow gets the closeEvent, even though they still exist and the QMainWindow is not yet deleted. Previously this function was
  // called multiple times, including once after the QMainWindow got its closeEvent, which would then save a corrupted state. Therefore,
  // once the parent wdQtContainerWindow gets the closeEvent, we now prevent further saving of the window layout.
  if (!m_bAllowSaveWindowLayout)
    return;

  const bool bMaximized = isMaximized();

  if (bMaximized)
    showNormal();

  wdStringBuilder sGroup;
  sGroup.Format("DocumentWnd_{0}", GetWindowLayoutGroupName());

  QSettings Settings;
  Settings.beginGroup(QString::fromUtf8(sGroup, sGroup.GetElementCount()));
  {
    // All other properties are defined by the outer container window.
    Settings.setValue("WindowState", saveState());
  }
  Settings.endGroup();
}

void wdQtDocumentWindow::RestoreWindowLayout()
{
  if (!s_bAllowRestoreWindowLayout)
    return;

  wdQtScopedUpdatesDisabled _(this);

  wdStringBuilder sGroup;
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

void wdQtDocumentWindow::DisableWindowLayoutSaving()
{
  m_bAllowSaveWindowLayout = false;
}

wdStatus wdQtDocumentWindow::SaveDocument()
{
  if (m_pDocument)
  {
    {
      if (m_pDocument->GetUnknownObjectTypeInstances() > 0)
      {
        if (wdQtUiServices::MessageBoxQuestion("Warning! This document contained unknown object types that could not be loaded. Saving the "
                                               "document means those objects will get lost permanently.\n\nDo you really want to save this "
                                               "document?",
              QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No, QMessageBox::StandardButton::No) != QMessageBox::StandardButton::Yes)
          return wdStatus(WD_SUCCESS); // failed successfully
      }
    }

    wdStatus res = m_pDocument->SaveDocument();

    wdStringBuilder s, s2;
    s.Format("Failed to save document:\n'{0}'", m_pDocument->GetDocumentPath());
    s2.Format("Successfully saved document:\n'{0}'", m_pDocument->GetDocumentPath());

    wdQtUiServices::MessageBoxStatus(res, s, s2);

    if (res.m_Result.Failed())
    {
      ShowTemporaryStatusBarMsg("Failed to save document");
      return res;
    }

    ShowTemporaryStatusBarMsg("Document saved");
  }

  return wdStatus(WD_SUCCESS);
}

void wdQtDocumentWindow::ShowTemporaryStatusBarMsg(const wdFormatString& msg, wdTime duration)
{
  wdStringBuilder tmp;
  statusBar()->showMessage(QString::fromUtf8(msg.GetText(tmp)), (int)duration.GetMilliseconds());
}


void wdQtDocumentWindow::SetPermanentStatusBarMsg(const wdFormatString& text)
{
  if (!text.IsEmpty())
  {
    // clear temporary message
    statusBar()->clearMessage();
  }

  wdStringBuilder tmp;
  m_pPermanentDocumentStatusText->setText(QString::fromUtf8(text.GetText(tmp)));
}

void wdQtDocumentWindow::CreateImageCapture(const char* szOutputPath)
{
  WD_ASSERT_NOT_IMPLEMENTED;
}

bool wdQtDocumentWindow::CanCloseWindow()
{
  return InternalCanCloseWindow();
}

bool wdQtDocumentWindow::InternalCanCloseWindow()
{
  // I guess this is to remove the focus from other widgets like input boxes, such that they may modify the document.
  setFocus();
  clearFocus();

  if (m_pDocument && m_pDocument->IsModified())
  {
    QMessageBox::StandardButton res = wdQtUiServices::MessageBoxQuestion("Save before closing?", QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No | QMessageBox::StandardButton::Cancel, QMessageBox::StandardButton::Cancel);

    if (res == QMessageBox::StandardButton::Cancel)
      return false;

    if (res == QMessageBox::StandardButton::Yes)
    {
      wdStatus err = SaveDocument();

      if (err.Failed())
      {
        wdQtUiServices::GetSingleton()->MessageBoxStatus(err, "Saving the scene failed.");
        return false;
      }
    }
  }

  return true;
}

void wdQtDocumentWindow::CloseDocumentWindow()
{
  QMetaObject::invokeMethod(this, "SlotQueuedDelete", Qt::ConnectionType::QueuedConnection);
}

void wdQtDocumentWindow::SlotQueuedDelete()
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

void wdQtDocumentWindow::OnPermanentGlobalStatusClicked(bool)
{
  wdQtUiServices::Event e;
  e.m_Type = wdQtUiServices::Event::ClickedDocumentPermanentStatusBarText;

  wdQtUiServices::GetSingleton()->s_Events.Broadcast(e);
}

void wdQtDocumentWindow::OnStatusBarMessageChanged(const QString& sNewText)
{
  QPalette pal = palette();

  if (sNewText.startsWith("Error:"))
  {
    pal.setColor(QPalette::WindowText, wdToQtColor(wdColorScheme::LightUI(wdColorScheme::Red)));
  }
  else if (sNewText.startsWith("Warning:"))
  {
    pal.setColor(QPalette::WindowText, wdToQtColor(wdColorScheme::LightUI(wdColorScheme::Yellow)));
  }
  else if (sNewText.startsWith("Note:"))
  {
    pal.setColor(QPalette::WindowText, wdToQtColor(wdColorScheme::LightUI(wdColorScheme::Blue)));
  }

  statusBar()->setPalette(pal);
}

void wdQtDocumentWindow::ShutdownDocumentWindow()
{
  SaveWindowLayout();

  InternalCloseDocumentWindow();

  wdQtDocumentWindowEvent e;
  e.m_pWindow = this;
  e.m_Type = wdQtDocumentWindowEvent::Type::WindowClosing;
  s_Events.Broadcast(e);

  InternalDeleteThis();

  e.m_Type = wdQtDocumentWindowEvent::Type::WindowClosed;
  s_Events.Broadcast(e);
}

void wdQtDocumentWindow::InternalCloseDocumentWindow() {}

void wdQtDocumentWindow::EnsureVisible()
{
  m_pContainerWindow->EnsureVisible(this).IgnoreResult();
}

void wdQtDocumentWindow::RequestWindowTabContextMenu(const QPoint& globalPos)
{
  wdQtMenuActionMapView menu(nullptr);

  wdActionContext context;
  context.m_sMapping = "DocumentWindowTabMenu";
  context.m_pDocument = GetDocument();
  context.m_pWindow = this;
  menu.SetActionContext(context);

  menu.exec(globalPos);
}

wdQtDocumentWindow* wdQtDocumentWindow::FindWindowByDocument(const wdDocument* pDocument)
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

wdQtContainerWindow* wdQtDocumentWindow::GetContainerWindow() const
{
  return m_pContainerWindow;
}

wdString wdQtDocumentWindow::GetWindowIcon() const
{
  if (GetDocument() != nullptr)
    return GetDocument()->GetDocumentTypeDescriptor()->m_sIcon;

  return ":/GuiFoundation/EZ-logo.svg";
}

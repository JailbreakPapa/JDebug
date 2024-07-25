#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Application/Application.h>
#include <Foundation/Types/ScopeExit.h>
#include <GuiFoundation/ContainerWindow/ContainerWindow.moc.h>
#include <GuiFoundation/DockPanels/ApplicationPanel.moc.h>
#include <QCloseEvent>
#include <QLabel>
#include <QSettings>
#include <QStatusBar>
#include <QTabBar>
#include <QTimer>
#include <ToolsFoundation/Application/ApplicationServices.h>
#include <ads/DockAreaWidget.h>
#include <ads/DockManager.h>
#include <ads/DockWidgetTab.h>
#include <ads/FloatingDockContainer.h>

nsQtContainerWindow* nsQtContainerWindow::s_pContainerWindow = nullptr;
bool nsQtContainerWindow::s_bForceClose = false;

namespace
{
  bool GetProjectLayoutPath(nsStringBuilder& out_sFile, bool bWrite)
  {
    if (!nsToolsProject::IsProjectOpen())
    {
      out_sFile.Clear();
      return false;
    }
    out_sFile = nsApplicationServices::GetSingleton()->GetProjectPreferencesFolder();
    out_sFile.AppendPath("layout.settings");
    if (!bWrite && !QFile::exists(out_sFile.GetData()))
    {
      out_sFile.Clear();
      return false;
    }
    return true;
  }

  bool GetApplicationLayoutPath(nsStringBuilder& out_sFile, bool bWrite)
  {
    out_sFile = nsApplicationServices::GetSingleton()->GetApplicationPreferencesFolder();
    out_sFile.AppendPath("layout.settings");
    if (!bWrite && !QFile::exists(out_sFile.GetData()))
    {
      out_sFile.Clear();
      return false;
    }
    return true;
  }
} // namespace

nsQtContainerWindow::nsQtContainerWindow()
{
  setMinimumSize(QSize(800, 600));
  m_bWindowLayoutRestored = false;
  m_pStatusBarLabel = nullptr;
  m_iWindowLayoutRestoreScheduled = 0;

  s_pContainerWindow = this;

  setObjectName("nsEditor");
  setWindowIcon(QIcon(QStringLiteral(":/GuiFoundation/NS-logo.svg")));

  nsQtDocumentWindow::s_Events.AddEventHandler(nsMakeDelegate(&nsQtContainerWindow::DocumentWindowEventHandler, this));
  nsToolsProject::s_Events.AddEventHandler(nsMakeDelegate(&nsQtContainerWindow::ProjectEventHandler, this));
  nsQtUiServices::s_Events.AddEventHandler(nsMakeDelegate(&nsQtContainerWindow::UIServicesEventHandler, this));

  UpdateWindowTitle();

  ads::CDockManager::ConfigFlags flags =
    ads::CDockManager::DefaultDockAreaButtons |
    ads::CDockManager::ActiveTabHasCloseButton |
    ads::CDockManager::XmlCompressionEnabled |
    ads::CDockManager::FloatingContainerHasWidgetTitle |
    ads::CDockManager::DragPreviewShowsContentPixmap |
    ads::CDockManager::FocusHighlighting |
    ads::CDockManager::AlwaysShowTabs |
    ads::CDockManager::DockAreaHasCloseButton |
    ads::CDockManager::DockAreaCloseButtonClosesTab |
    ads::CDockManager::MiddleMouseButtonClosesTab |
    ads::CDockManager::DockAreaHasTabsMenuButton |
    ads::CDockManager::FloatingContainerHasWidgetIcon |
    ads::CDockManager::AllTabsHaveCloseButton |
    ads::CDockManager::OpaqueSplitterResize;
  ads::CDockManager::setConfigFlags(flags);

  m_pDockManager = new ads::CDockManager(this);

  connect(m_pDockManager, &ads::CDockManager::floatingWidgetCreated, this, &nsQtContainerWindow::SlotFloatingWidgetOpened);
}

nsQtContainerWindow::~nsQtContainerWindow()
{
  s_pContainerWindow = nullptr;

  nsQtDocumentWindow::s_Events.RemoveEventHandler(nsMakeDelegate(&nsQtContainerWindow::DocumentWindowEventHandler, this));
  nsToolsProject::s_Events.RemoveEventHandler(nsMakeDelegate(&nsQtContainerWindow::ProjectEventHandler, this));
  nsQtUiServices::s_Events.RemoveEventHandler(nsMakeDelegate(&nsQtContainerWindow::UIServicesEventHandler, this));
}

void nsQtContainerWindow::UpdateWindowTitle()
{
  nsStringBuilder sTitle;

  if (nsToolsProject::IsProjectOpen())
  {
    sTitle = nsToolsProject::GetSingleton()->GetProjectName(false);
    sTitle.Append(" - ");
  }

  sTitle.Append(nsApplication::GetApplicationInstance()->GetApplicationName().GetView());

  setWindowTitle(QString::fromUtf8(sTitle.GetData()));
}

void nsQtContainerWindow::ScheduleRestoreWindowLayout()
{
  m_iWindowLayoutRestoreScheduled++;
  QTimer::singleShot(0, this, SLOT(SlotRestoreLayout()));
}

void nsQtContainerWindow::SlotRestoreLayout()
{
  RestoreWindowLayout();
}

void nsQtContainerWindow::closeEvent(QCloseEvent* e)
{
  SaveWindowLayout();
  SaveDocumentLayouts();

  if (s_bForceClose)
    return;

  s_bForceClose = true;
  NS_SCOPE_EXIT(s_bForceClose = false);

  e->setAccepted(true);

  if (!nsToolsProject::CanCloseProject())
  {
    e->setAccepted(false);
    return;
  }

  nsToolsProject::SaveProjectState();

  // do not close the documents in the main container window here,
  // as that would remove them from the recently-open documents list and not restore them when opening the editor again
  nsDynamicArray<nsQtDocumentWindow*> windows = m_DocumentWindows;
  for (nsQtDocumentWindow* pWindow : windows)
  {
    pWindow->DisableWindowLayoutSaving();
    pWindow->ShutdownDocumentWindow();
  }

  // We need to destroy the dock manager here, doing it in the constructor leads to an access violation.
  m_pDockManager->deleteLater();
  m_pDockManager = nullptr;
  QMainWindow::closeEvent(e);
}

void nsQtContainerWindow::SaveWindowLayout()
{
  if (!m_pDockManager)
    return;

  nsStringBuilder sFile;
  GetApplicationLayoutPath(sFile, true);

  nsStringBuilder sProjectFile;
  GetProjectLayoutPath(sProjectFile, true);

  QSettings Settings(nsToolsProject::IsProjectOpen() ? sProjectFile.GetData() : sFile.GetData(), QSettings::IniFormat);
  Settings.beginGroup(QString::fromUtf8("ContainerWnd_nsEditor"));
  {
    Settings.setValue("DockManagerState", m_pDockManager->saveState(1));
    Settings.setValue("WindowGeometry", saveGeometry());
    Settings.setValue("WindowState", saveState());
  }
  Settings.endGroup();

  if (nsToolsProject::IsProjectOpen())
  {
    // The last open project always serves as the default layout in case
    // a new project is created or a project without layout data is opened.
    QFile::remove(sFile.GetData());
    QFile::copy(sProjectFile.GetData(), sFile.GetData());
  }
}

void nsQtContainerWindow::SaveDocumentLayouts()
{
  for (nsUInt32 i = 0; i < m_DocumentWindows.GetCount(); ++i)
    m_DocumentWindows[i]->SaveWindowLayout();
}

void nsQtContainerWindow::RestoreWindowLayout()
{
  --m_iWindowLayoutRestoreScheduled;
  if (m_iWindowLayoutRestoreScheduled > 0)
    return;

  bool bCreteDefaultLayout = true;
  NS_SCOPE_EXIT(bCreteDefaultLayout ? showMaximized() : show(););

  nsStringBuilder sFile;
  if (!GetProjectLayoutPath(sFile, false))
  {
    if (!GetApplicationLayoutPath(sFile, false))
    {
      // No project or app settings file found, exiting.
      return;
    }
  }

  {
    QSettings Settings(sFile.GetData(), QSettings::IniFormat);
    Settings.beginGroup(QString::fromUtf8("ContainerWnd_nsEditor"));
    {
      QByteArray geom = Settings.value("WindowGeometry", QByteArray()).toByteArray();
      if (!geom.isEmpty())
      {
        bCreteDefaultLayout = false;
        restoreGeometry(geom);
        restoreState(Settings.value("WindowState", saveState()).toByteArray());
        auto dockState = Settings.value("DockManagerState");
        if (dockState.isValid() && dockState.typeId() == QMetaType::QByteArray)
        {
          m_pDockManager->restoreState(dockState.toByteArray(), 1);
          // As document windows can't be in a closed state (as pressing x destroys them),
          // we need to fix any document window that was accidentally saved in its closed state.
          for (ads::CDockWidget* dock : m_DocumentDocks)
          {
            if (dock->isClosed())
            {
              if (dock->dockContainer() == nullptr)
              {
                if (m_DocumentDocks.GetCount() >= 2)
                {
                  // If we can (we are not the only dock window), we are going to attach to a window that isn't us, ideally the settings window.
                  nsUInt32 uiBestIndex = 0;
                  for (nsUInt32 i = 0; i < m_DocumentDocks.GetCount(); i++)
                  {
                    if (nsStringUtils::IsEqual(m_DocumentWindows[i]->GetUniqueName(), "Settings"))
                    {
                      uiBestIndex = i;
                      break;
                    }
                    else if (m_DocumentDocks[i] != dock)
                    {
                      uiBestIndex = i;
                    }
                  }

                  ads::CDockAreaWidget* dockArea = m_DocumentDocks[uiBestIndex]->dockAreaWidget();
                  m_pDockManager->addDockWidgetTabToArea(dock, dockArea);
                }
                else
                {
                  m_pDockManager->addDockWidgetTab(ads::LeftDockWidgetArea, dock);
                }
              }
              dock->toggleView();
            }
          }
        }
      }
    }
    Settings.endGroup();
  }

  for (nsUInt32 i = 0; i < m_DocumentWindows.GetCount(); ++i)
    m_DocumentWindows[i]->RestoreWindowLayout();

  m_bWindowLayoutRestored = true;
}

void nsQtContainerWindow::SlotUpdateWindowDecoration(void* pDocWindow)
{
  UpdateWindowDecoration(static_cast<nsQtDocumentWindow*>(pDocWindow));
}

void nsQtContainerWindow::SlotFloatingWidgetOpened(ads::CFloatingDockContainer* FloatingWidget)
{
  FloatingWidget->installEventFilter(this);
}

void nsQtContainerWindow::SlotDockWidgetFloatingChanged(bool bFloating)
{
  if (!bFloating)
    return;

  for (auto pDoc : m_DocumentWindows)
  {
    UpdateWindowDecoration(pDoc);
  }
}

void nsQtContainerWindow::UpdateWindowDecoration(nsQtDocumentWindow* pDocWindow)
{
  const nsUInt32 uiListIndex = m_DocumentWindows.IndexOf(pDocWindow);
  if (uiListIndex == nsInvalidIndex)
    return;

  ads::CDockWidget* dock = m_DocumentDocks[uiListIndex];

  dock->setTabToolTip(QString::fromUtf8(pDocWindow->GetDisplayName().GetData()));
  dock->setIcon(nsQtUiServices::GetCachedIconResource(pDocWindow->GetWindowIcon().GetData()));
  dock->setWindowTitle(QString::fromUtf8(pDocWindow->GetDisplayNameShort().GetData()));

  // this is a hacky way to detect the nsQtSettingsTab
  if (pDocWindow->GetDisplayNameShort().IsEmpty())
  {
    dock->setFeature(ads::CDockWidget::DockWidgetClosable, false);
    dock->setFeature(ads::CDockWidget::DockWidgetMovable, false);
    dock->setFeature(ads::CDockWidget::DockWidgetFloatable, false);
    dock->setFeature(ads::CDockWidget::NoTab, true);
  }

  if (dock->isFloating())
  {
    dock->dockContainer()->floatingWidget()->setWindowTitle(dock->windowTitle());
    dock->dockContainer()->floatingWidget()->setWindowIcon(dock->icon());
  }
}

void nsQtContainerWindow::RemoveDocumentWindow(nsQtDocumentWindow* pDocWindow)
{
  const nsUInt32 uiListIndex = m_DocumentWindows.IndexOf(pDocWindow);
  if (uiListIndex == nsInvalidIndex)
    return;

  ads::CDockWidget* dock = m_DocumentDocks[uiListIndex];

  int iCurIdx = -1;

  const bool bIsTabbed = dock->isTabbed();
  ads::CDockAreaWidget* pDockArea = dock->dockAreaWidget();

  iCurIdx = pDockArea->currentIndex();

  m_pDockManager->removeDockWidget(dock);

  m_DocumentWindows.RemoveAtAndSwap(uiListIndex);
  m_DocumentDocks.RemoveAtAndSwap(uiListIndex);
  NS_ASSERT_DEV(m_DockNames.contains(dock->objectName()), "Object name must not change during lifetime.");
  m_DockNames.remove(dock->objectName());
  dock->hide();
  dock->deleteLater();
  pDocWindow->m_pContainerWindow = nullptr;

  if (bIsTabbed)
  {
    iCurIdx = nsMath::Min(iCurIdx, pDockArea->openDockWidgetsCount() - 1);
    pDockArea->setCurrentIndex(iCurIdx);
  }

  if (pDockArea && pDockArea->openDockWidgetsCount() == 1)
  {
    for (auto pDocWindow2 : m_DocumentWindows)
    {
      UpdateWindowDecoration(pDocWindow);
    }
  }
}

void nsQtContainerWindow::RemoveApplicationPanel(nsQtApplicationPanel* pPanel)
{
  const auto uiListIndex = m_ApplicationPanels.IndexOf(pPanel);

  if (uiListIndex == nsInvalidIndex)
    return;

  m_pDockManager->removeDockWidget(pPanel);
  m_ApplicationPanels.RemoveAtAndSwap(uiListIndex);

  pPanel->m_pContainerWindow = nullptr;
}

void nsQtContainerWindow::AddDocumentWindow(nsQtDocumentWindow* pDocWindow)
{
  NS_ASSERT_DEV(!pDocWindow->objectName().isEmpty(), "Panel name must be unique and not empty.");

  if (m_DocumentWindows.IndexOf(pDocWindow) != nsInvalidIndex)
    return;

  NS_ASSERT_DEV(pDocWindow->m_pContainerWindow == nullptr, "Implementation error");

  // NOTE: This function is called by the nsQtDocumentWindow constructor
  // that means any derived classes are not yet constructed!
  // therefore calling virtual functions here, like GetDisplayNameShort() will still call
  // the base class implementation, NOT the derived one !
  // therefore, we do some stuff in nsQtContainerWindow::UpdateWindowDecoration() instead

  m_DocumentWindows.PushBack(pDocWindow);
  nsString displayName = pDocWindow->GetDisplayNameShort();
  ads::CDockWidget* dock = new ads::CDockWidget(QString::fromUtf8(displayName.GetData(), displayName.GetElementCount()));
  dock->installEventFilter(pDocWindow);

  dock->setObjectName(pDocWindow->GetUniqueName());
  NS_ASSERT_DEV(!dock->objectName().isEmpty(), "Dock name must not be empty.");
  NS_ASSERT_DEV(!m_DockNames.contains(dock->objectName()), "Dock name must be unique.");
  m_DockNames.insert(dock->objectName());
  dock->setWidget(pDocWindow);
  dock->tabWidget()->setContextMenuPolicy(Qt::CustomContextMenu);
  if (!m_DocumentDocks.IsEmpty())
  {
    ads::CDockAreaWidget* dockArea = m_DocumentDocks.PeekBack()->dockAreaWidget();
    m_pDockManager->addDockWidgetTabToArea(dock, dockArea);
  }
  else
  {
    m_pDockManager->addDockWidgetTab(ads::LeftDockWidgetArea, dock);
  }
  m_DocumentDocks.PushBack(dock);
  connect(dock, &ads::CDockWidget::closed, this, &nsQtContainerWindow::SlotDocumentTabCloseRequested);
  connect(dock->tabWidget(), &QWidget::customContextMenuRequested, this, &nsQtContainerWindow::SlotTabsContextMenuRequested);
  connect(dock, &ads::CDockWidget::topLevelChanged, this, &nsQtContainerWindow::SlotDockWidgetFloatingChanged);


  pDocWindow->m_pContainerWindow = this;

  // we cannot call virtual functions on pDocWindow here, because the object might still be under construction
  // so we delay it until later
  QMetaObject::invokeMethod(this, "SlotUpdateWindowDecoration", Qt::ConnectionType::QueuedConnection, Q_ARG(void*, pDocWindow));
}

void nsQtContainerWindow::DocumentWindowRenamed(nsQtDocumentWindow* pDocWindow)
{
  const nsUInt32 uiListIndex = m_DocumentWindows.IndexOf(pDocWindow);
  if (uiListIndex == nsInvalidIndex)
    return;

  ads::CDockWidget* dock = m_DocumentDocks[uiListIndex];
  NS_ASSERT_DEV(m_DockNames.contains(dock->objectName()), "Object name must not change during lifetime.");
  m_DockNames.remove(dock->objectName());

  dock->setObjectName(pDocWindow->GetUniqueName());
  NS_ASSERT_DEV(!dock->objectName().isEmpty(), "Dock name must not be empty.");
  NS_ASSERT_DEV(!m_DockNames.contains(dock->objectName()), "Dock name must be unique.");
  m_DockNames.insert(dock->objectName());
}

void nsQtContainerWindow::AddApplicationPanel(nsQtApplicationPanel* pPanel)
{
  // panel already in container window ?
  if (m_ApplicationPanels.IndexOf(pPanel) != nsInvalidIndex)
    return;

  NS_ASSERT_DEV(!pPanel->objectName().isEmpty(), "Dock name must not be empty.");
  NS_ASSERT_DEV(!m_DockNames.contains(pPanel->objectName()), "Dock name must be unique.");
  m_DockNames.insert(pPanel->objectName());
  NS_ASSERT_DEV(pPanel->m_pContainerWindow == nullptr, "Implementation error");

  m_ApplicationPanels.PushBack(pPanel);
  pPanel->m_pContainerWindow = this;
  m_pDockManager->addDockWidgetTab(ads::RightDockWidgetArea, pPanel);
}

nsResult nsQtContainerWindow::EnsureVisible(nsQtDocumentWindow* pDocWindow)
{
  const auto uiListIndex = m_DocumentWindows.IndexOf(pDocWindow);

  if (uiListIndex == nsInvalidIndex)
    return NS_FAILURE;

  ads::CDockWidget* dock = m_DocumentDocks[uiListIndex];

  dock->toggleView(true);
  return NS_SUCCESS;
}

nsResult nsQtContainerWindow::EnsureVisible(nsDocument* pDocument)
{
  for (auto doc : m_DocumentWindows)
  {
    if (doc->GetDocument() == pDocument)
      return EnsureVisible(doc);
  }

  return NS_FAILURE;
}

nsResult nsQtContainerWindow::EnsureVisible(nsQtApplicationPanel* pPanel)
{
  if (m_ApplicationPanels.IndexOf(pPanel) == nsInvalidIndex)
    return NS_FAILURE;

  if (pPanel->isClosed())
  {
    pPanel->toggleView();
  }
  pPanel->raise();
  return NS_SUCCESS;
}

nsResult nsQtContainerWindow::EnsureVisibleAnyContainer(nsDocument* pDocument)
{
  // make sure there is a window to make visible in the first place
  pDocument->GetDocumentManager()->EnsureWindowRequested(pDocument);

  if (s_pContainerWindow->EnsureVisible(pDocument).Succeeded())
    return NS_SUCCESS;

  return NS_FAILURE;
}

void nsQtContainerWindow::GetDocumentWindows(nsHybridArray<nsQtDocumentWindow*, 16>& ref_windows)
{
  ref_windows = m_DocumentWindows;
}

bool nsQtContainerWindow::eventFilter(QObject* obj, QEvent* e)
{
  if (e->type() == QEvent::Type::Close)
  {
    if (auto* pFloatingWidget = qobject_cast<ads::CFloatingDockContainer*>(obj))
    {
      nsHybridArray<nsDocument*, 32> docs;
      docs.Reserve(m_DocumentWindows.GetCount());
      nsHybridArray<nsQtDocumentWindow*, 32> windows;
      windows.Reserve(m_DocumentWindows.GetCount());

      QList<ads::CDockWidget*> floatingDocks = pFloatingWidget->dockWidgets();
      for (nsUInt32 i = 0; i < m_DocumentWindows.GetCount(); ++i)
      {
        if (floatingDocks.contains(m_DocumentDocks[i]))
        {
          docs.PushBack(m_DocumentWindows[i]->GetDocument());
          windows.PushBack(m_DocumentWindows[i]);
        }
      }

      if (!nsToolsProject::CanCloseDocuments(docs))
      {
        e->setAccepted(false);
        return true;
      }

      // closing a non-main window should close all documents as well
      // this will remove them from the recently-open documents list and not restore them next time
      for (nsQtDocumentWindow* pWindow : windows)
      {
        pWindow->CloseDocumentWindow();
      }
      // This is necessary to clean up some 'delete later' Qt objects before the document is closed as they need to remove their references to the doc.
      qApp->processEvents();
    }
  }
  return false;
}

void nsQtContainerWindow::SlotDocumentTabCloseRequested()
{
  auto dock = qobject_cast<ads::CDockWidget*>(sender());
  const auto uiListIndex = m_DocumentDocks.IndexOf(dock);
  NS_ASSERT_DEV(uiListIndex != nsInvalidIndex, "Can't close non-existing document.");

  nsQtDocumentWindow* pDocWindow = m_DocumentWindows[uiListIndex];

  if (!pDocWindow->CanCloseWindow())
  {
    // TODO: There is no CloseRequested event so we just reopen on a timer.
    QTimer::singleShot(1, [dock]()
      { dock->toggleView(); });
    return;
  }
  pDocWindow->CloseDocumentWindow();
}

void nsQtContainerWindow::DocumentWindowEventHandler(const nsQtDocumentWindowEvent& e)
{
  switch (e.m_Type)
  {
    case nsQtDocumentWindowEvent::Type::WindowClosing:
      RemoveDocumentWindow(e.m_pWindow);
      break;
    case nsQtDocumentWindowEvent::Type::WindowDecorationChanged:
      UpdateWindowDecoration(e.m_pWindow);
      break;

    default:
      break;
  }
}

void nsQtContainerWindow::ProjectEventHandler(const nsToolsProjectEvent& e)
{
  switch (e.m_Type)
  {
    case nsToolsProjectEvent::Type::ProjectOpened:
    case nsToolsProjectEvent::Type::ProjectClosed:
      UpdateWindowTitle();
      break;

    default:
      break;
  }
}

void nsQtContainerWindow::UIServicesEventHandler(const nsQtUiServices::Event& e)
{
  switch (e.m_Type)
  {
    case nsQtUiServices::Event::Type::ShowGlobalStatusBarText:
    {
      if (statusBar() == nullptr)
        setStatusBar(new QStatusBar());

      if (m_pStatusBarLabel == nullptr)
      {
        m_pStatusBarLabel = new QLabel();
        statusBar()->addWidget(m_pStatusBarLabel);

        QPalette pal = m_pStatusBarLabel->palette();
        pal.setColor(QPalette::WindowText, QColor(Qt::red));
        m_pStatusBarLabel->setPalette(pal);
      }

      statusBar()->setHidden(e.m_sText.IsEmpty());
      m_pStatusBarLabel->setText(QString::fromUtf8(e.m_sText.GetData()));
    }
    break;

    default:
      break;
  }
}

void nsQtContainerWindow::SlotTabsContextMenuRequested(const QPoint& pos)
{
  auto tab = qobject_cast<ads::CDockWidgetTab*>(sender());
  ads::CDockWidget* dock = tab->dockWidget();
  const auto uiListIndex = m_DocumentDocks.IndexOf(dock);
  NS_ASSERT_DEV(uiListIndex != nsInvalidIndex, "Can't close non-existing document.");

  nsQtDocumentWindow* pDoc = m_DocumentWindows[uiListIndex];
  pDoc->RequestWindowTabContextMenu(tab->mapToGlobal(pos));
}

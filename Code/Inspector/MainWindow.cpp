#include <Inspector/InspectorPCH.h>

#include <Inspector/CVarsWidget.moc.h>
#include <Inspector/DataTransferWidget.moc.h>
#include <Inspector/FileWidget.moc.h>
#include <Inspector/GlobalEventsWidget.moc.h>
#include <Inspector/InputWidget.moc.h>
#include <Inspector/LogDockWidget.moc.h>
#include <Inspector/MainWidget.moc.h>
#include <Inspector/MainWindow.moc.h>
#include <Inspector/MemoryWidget.moc.h>
#include <Inspector/PluginsWidget.moc.h>
#include <Inspector/ReflectionWidget.moc.h>
#include <Inspector/ResourceWidget.moc.h>
#include <Inspector/SubsystemsWidget.moc.h>
#include <Inspector/TimeWidget.moc.h>

const int g_iDockingStateVersion = 1;

nsQtMainWindow* nsQtMainWindow::s_pWidget = nullptr;

nsQtMainWindow::nsQtMainWindow()
  : QMainWindow()
{
  s_pWidget = this;

  setupUi(this);

  m_DockManager = new ads::CDockManager(this);
  m_DockManager->setConfigFlags(
    static_cast<ads::CDockManager::ConfigFlags>(ads::CDockManager::DockAreaHasCloseButton | ads::CDockManager::DockAreaCloseButtonClosesTab |
                                                ads::CDockManager::OpaqueSplitterResize | ads::CDockManager::AllTabsHaveCloseButton));

  QSettings Settings;
  SetAlwaysOnTop((OnTopMode)Settings.value("AlwaysOnTop", (int)WhenConnected).toInt());

  Settings.beginGroup("MainWindow");

  const bool bRestoreDockingState = Settings.value("DockingVersion") == g_iDockingStateVersion;

  if (bRestoreDockingState)
  {
    restoreGeometry(Settings.value("WindowGeometry", saveGeometry()).toByteArray());
  }

  // The dock manager will set ownership to null on add so there is no reason to provide an owner here.
  // Setting one will actually cause memory corruptions on shutdown for unknown reasons.
  nsQtMainWidget* pMainWidget = new nsQtMainWidget();
  nsQtLogDockWidget* pLogWidget = new nsQtLogDockWidget();
  nsQtMemoryWidget* pMemoryWidget = new nsQtMemoryWidget();
  nsQtTimeWidget* pTimeWidget = new nsQtTimeWidget();
  nsQtInputWidget* pInputWidget = new nsQtInputWidget();
  nsQtCVarsWidget* pCVarsWidget = new nsQtCVarsWidget();
  nsQtSubsystemsWidget* pSubsystemsWidget = new nsQtSubsystemsWidget();
  nsQtFileWidget* pFileWidget = new nsQtFileWidget();
  nsQtPluginsWidget* pPluginsWidget = new nsQtPluginsWidget();
  nsQtGlobalEventsWidget* pGlobalEventesWidget = new nsQtGlobalEventsWidget();
  nsQtReflectionWidget* pReflectionWidget = new nsQtReflectionWidget();
  nsQtDataWidget* pDataWidget = new nsQtDataWidget();
  nsQtResourceWidget* pResourceWidget = new nsQtResourceWidget();

  NS_VERIFY(nullptr != QWidget::connect(pMainWidget, &ads::CDockWidget::viewToggled, this, &nsQtMainWindow::DockWidgetVisibilityChanged), "");
  NS_VERIFY(nullptr != QWidget::connect(pLogWidget, &ads::CDockWidget::viewToggled, this, &nsQtMainWindow::DockWidgetVisibilityChanged), "");
  NS_VERIFY(nullptr != QWidget::connect(pTimeWidget, &ads::CDockWidget::viewToggled, this, &nsQtMainWindow::DockWidgetVisibilityChanged), "");
  NS_VERIFY(nullptr != QWidget::connect(pMemoryWidget, &ads::CDockWidget::viewToggled, this, &nsQtMainWindow::DockWidgetVisibilityChanged), "");
  NS_VERIFY(nullptr != QWidget::connect(pInputWidget, &ads::CDockWidget::viewToggled, this, &nsQtMainWindow::DockWidgetVisibilityChanged), "");
  NS_VERIFY(nullptr != QWidget::connect(pCVarsWidget, &ads::CDockWidget::viewToggled, this, &nsQtMainWindow::DockWidgetVisibilityChanged), "");
  NS_VERIFY(nullptr != QWidget::connect(pReflectionWidget, &ads::CDockWidget::viewToggled, this, &nsQtMainWindow::DockWidgetVisibilityChanged), "");
  NS_VERIFY(nullptr != QWidget::connect(pSubsystemsWidget, &ads::CDockWidget::viewToggled, this, &nsQtMainWindow::DockWidgetVisibilityChanged), "");
  NS_VERIFY(nullptr != QWidget::connect(pFileWidget, &ads::CDockWidget::viewToggled, this, &nsQtMainWindow::DockWidgetVisibilityChanged), "");
  NS_VERIFY(nullptr != QWidget::connect(pPluginsWidget, &ads::CDockWidget::viewToggled, this, &nsQtMainWindow::DockWidgetVisibilityChanged), "");
  NS_VERIFY(
    nullptr != QWidget::connect(pGlobalEventesWidget, &ads::CDockWidget::viewToggled, this, &nsQtMainWindow::DockWidgetVisibilityChanged), "");
  NS_VERIFY(nullptr != QWidget::connect(pDataWidget, &ads::CDockWidget::viewToggled, this, &nsQtMainWindow::DockWidgetVisibilityChanged), "");
  NS_VERIFY(nullptr != QWidget::connect(pResourceWidget, &ads::CDockWidget::viewToggled, this, &nsQtMainWindow::DockWidgetVisibilityChanged), "");

  QMenu* pHistoryMenu = new QMenu;
  pHistoryMenu->setTearOffEnabled(true);
  pHistoryMenu->setTitle(QLatin1String("Stat Histories"));
  pHistoryMenu->setIcon(QIcon(":/Icons/Icons/StatHistory.svg"));

  for (nsUInt32 i = 0; i < 10; ++i)
  {
    m_pStatHistoryWidgets[i] = new nsQtStatVisWidget(this, i);
    m_DockManager->addDockWidgetTab(ads::BottomDockWidgetArea, m_pStatHistoryWidgets[i]);

    NS_VERIFY(
      nullptr != QWidget::connect(m_pStatHistoryWidgets[i], &ads::CDockWidget::viewToggled, this, &nsQtMainWindow::DockWidgetVisibilityChanged), "");

    pHistoryMenu->addAction(&m_pStatHistoryWidgets[i]->m_ShowWindowAction);

    m_pActionShowStatIn[i] = new QAction(this);

    NS_VERIFY(nullptr != QWidget::connect(m_pActionShowStatIn[i], &QAction::triggered, nsQtMainWidget::s_pWidget, &nsQtMainWidget::ShowStatIn), "");
  }

  // delay this until after all widgets are created
  for (nsUInt32 i = 0; i < 10; ++i)
  {
    m_pStatHistoryWidgets[i]->toggleView(false); // hide
  }

  setContextMenuPolicy(Qt::NoContextMenu);

  menuWindows->addMenu(pHistoryMenu);

  pMemoryWidget->raise();

  m_DockManager->addDockWidget(ads::LeftDockWidgetArea, pMainWidget);
  m_DockManager->addDockWidget(ads::CenterDockWidgetArea, pLogWidget);

  m_DockManager->addDockWidget(ads::RightDockWidgetArea, pCVarsWidget);
  m_DockManager->addDockWidgetTab(ads::RightDockWidgetArea, pGlobalEventesWidget);
  m_DockManager->addDockWidgetTab(ads::RightDockWidgetArea, pDataWidget);
  m_DockManager->addDockWidgetTab(ads::RightDockWidgetArea, pInputWidget);
  m_DockManager->addDockWidgetTab(ads::RightDockWidgetArea, pPluginsWidget);
  m_DockManager->addDockWidgetTab(ads::RightDockWidgetArea, pReflectionWidget);
  m_DockManager->addDockWidgetTab(ads::RightDockWidgetArea, pResourceWidget);
  m_DockManager->addDockWidgetTab(ads::RightDockWidgetArea, pSubsystemsWidget);

  m_DockManager->addDockWidget(ads::BottomDockWidgetArea, pFileWidget);
  m_DockManager->addDockWidgetTab(ads::BottomDockWidgetArea, pMemoryWidget);
  m_DockManager->addDockWidgetTab(ads::BottomDockWidgetArea, pTimeWidget);


  pLogWidget->raise();
  pCVarsWidget->raise();

  if (bRestoreDockingState)
  {
    auto dockState = Settings.value("DockManagerState");
    if (dockState.isValid() && dockState.typeId() == QMetaType::QByteArray)
    {
      m_DockManager->restoreState(dockState.toByteArray(), 1);
    }

    move(Settings.value("WindowPosition", pos()).toPoint());
    resize(Settings.value("WindowSize", size()).toSize());

    if (Settings.value("IsMaximized", isMaximized()).toBool())
    {
      showMaximized();
    }

    restoreState(Settings.value("WindowState", saveState()).toByteArray());
  }

  Settings.endGroup();

  for (nsInt32 i = 0; i < 10; ++i)
    m_pStatHistoryWidgets[i]->Load();

  SetupNetworkTimer();
}

nsQtMainWindow::~nsQtMainWindow()
{
  for (nsInt32 i = 0; i < 10; ++i)
  {
    m_pStatHistoryWidgets[i]->Save();
  }
  // The dock manager does not take ownership of dock widgets.
  auto dockWidgets = m_DockManager->dockWidgetsMap();
  for (auto it = dockWidgets.begin(); it != dockWidgets.end(); ++it)
  {
    m_DockManager->removeDockWidget(it.value());
    delete it.value();
  }
}

void nsQtMainWindow::closeEvent(QCloseEvent* pEvent)
{
  const bool bMaximized = isMaximized();
  if (bMaximized)
    showNormal();

  QSettings Settings;

  Settings.beginGroup("MainWindow");

  Settings.setValue("DockingVersion", g_iDockingStateVersion);
  Settings.setValue("DockManagerState", m_DockManager->saveState(1));
  Settings.setValue("WindowGeometry", saveGeometry());
  Settings.setValue("WindowState", saveState());
  Settings.setValue("IsMaximized", bMaximized);
  Settings.setValue("WindowPosition", pos());
  if (!bMaximized)
    Settings.setValue("WindowSize", size());

  Settings.endGroup();
}

void nsQtMainWindow::SetupNetworkTimer()
{
  // reset the timer to fire again
  if (m_pNetworkTimer == nullptr)
    m_pNetworkTimer = new QTimer(this);

  m_pNetworkTimer->singleShot(40, this, SLOT(UpdateNetworkTimeOut()));
}

void nsQtMainWindow::UpdateNetworkTimeOut()
{
  UpdateNetwork();

  SetupNetworkTimer();
}

void nsQtMainWindow::UpdateNetwork()
{
  bool bResetStats = false;

  {
    static nsUInt32 uiServerID = 0;
    static bool bConnected = false;
    static nsString sLastServerName;

    if (nsTelemetry::IsConnectedToServer())
    {
      if (uiServerID != nsTelemetry::GetServerID())
      {
        uiServerID = nsTelemetry::GetServerID();
        bResetStats = true;

        nsStringBuilder s;
        s.SetFormat("Connected to new Server with ID {0}", uiServerID);

        nsQtLogDockWidget::s_pWidget->Log(s.GetData());
      }
      else if (!bConnected)
      {
        nsQtLogDockWidget::s_pWidget->Log("Reconnected to Server.");
      }

      if (sLastServerName != nsTelemetry::GetServerName())
      {
        sLastServerName = nsTelemetry::GetServerName();
        setWindowTitle(QString("nsInspector - %1").arg(sLastServerName.GetData()));
      }

      bConnected = true;
    }
    else
    {
      if (bConnected)
      {
        nsQtLogDockWidget::s_pWidget->Log("Lost Connection to Server.");
        setWindowTitle(QString("nsInspector - disconnected"));
        sLastServerName.Clear();
      }

      bConnected = false;
    }
  }

  if (bResetStats)
  {


    nsQtMainWidget::s_pWidget->ResetStats();
    nsQtLogDockWidget::s_pWidget->ResetStats();
    nsQtMemoryWidget::s_pWidget->ResetStats();
    nsQtTimeWidget::s_pWidget->ResetStats();
    nsQtInputWidget::s_pWidget->ResetStats();
    nsQtCVarsWidget::s_pWidget->ResetStats();
    nsQtReflectionWidget::s_pWidget->ResetStats();
    nsQtFileWidget::s_pWidget->ResetStats();
    nsQtPluginsWidget::s_pWidget->ResetStats();
    nsQtSubsystemsWidget::s_pWidget->ResetStats();
    nsQtGlobalEventsWidget::s_pWidget->ResetStats();
    nsQtDataWidget::s_pWidget->ResetStats();
    nsQtResourceWidget::s_pWidget->ResetStats();
  }

  UpdateAlwaysOnTop();

  nsQtMainWidget::s_pWidget->UpdateStats();
  nsQtPluginsWidget::s_pWidget->UpdateStats();
  nsQtSubsystemsWidget::s_pWidget->UpdateStats();
  nsQtMemoryWidget::s_pWidget->UpdateStats();
  nsQtTimeWidget::s_pWidget->UpdateStats();
  nsQtFileWidget::s_pWidget->UpdateStats();
  nsQtResourceWidget::s_pWidget->UpdateStats();
  // nsQtDataWidget::s_pWidget->UpdateStats();

  for (nsInt32 i = 0; i < 10; ++i)
    m_pStatHistoryWidgets[i]->UpdateStats();

  nsTelemetry::PerFrameUpdate();
}

void nsQtMainWindow::DockWidgetVisibilityChanged(bool bVisible)
{
  // TODO: add menu entry for qt main widget

  ActionShowWindowLog->setChecked(!nsQtLogDockWidget::s_pWidget->isClosed());
  ActionShowWindowMemory->setChecked(!nsQtMemoryWidget::s_pWidget->isClosed());
  ActionShowWindowTime->setChecked(!nsQtTimeWidget::s_pWidget->isClosed());
  ActionShowWindowInput->setChecked(!nsQtInputWidget::s_pWidget->isClosed());
  ActionShowWindowCVar->setChecked(!nsQtCVarsWidget::s_pWidget->isClosed());
  ActionShowWindowReflection->setChecked(!nsQtReflectionWidget::s_pWidget->isClosed());
  ActionShowWindowSubsystems->setChecked(!nsQtSubsystemsWidget::s_pWidget->isClosed());
  ActionShowWindowFile->setChecked(!nsQtFileWidget::s_pWidget->isClosed());
  ActionShowWindowPlugins->setChecked(!nsQtPluginsWidget::s_pWidget->isClosed());
  ActionShowWindowGlobalEvents->setChecked(!nsQtGlobalEventsWidget::s_pWidget->isClosed());
  ActionShowWindowData->setChecked(!nsQtDataWidget::s_pWidget->isClosed());
  ActionShowWindowResource->setChecked(!nsQtResourceWidget::s_pWidget->isClosed());

  for (nsInt32 i = 0; i < 10; ++i)
    m_pStatHistoryWidgets[i]->m_ShowWindowAction.setChecked(!m_pStatHistoryWidgets[i]->isClosed());
}


void nsQtMainWindow::SetAlwaysOnTop(OnTopMode Mode)
{
  m_OnTopMode = Mode;

  QSettings Settings;
  Settings.setValue("AlwaysOnTop", (int)m_OnTopMode);

  ActionNeverOnTop->setChecked((m_OnTopMode == Never) ? Qt::Checked : Qt::Unchecked);
  ActionAlwaysOnTop->setChecked((m_OnTopMode == Always) ? Qt::Checked : Qt::Unchecked);
  ActionOnTopWhenConnected->setChecked((m_OnTopMode == WhenConnected) ? Qt::Checked : Qt::Unchecked);

  UpdateAlwaysOnTop();
}

void nsQtMainWindow::UpdateAlwaysOnTop()
{
  static bool bOnTop = false;

  bool bNewState = bOnTop;
  NS_IGNORE_UNUSED(bNewState);

  if (m_OnTopMode == Always || (m_OnTopMode == WhenConnected && nsTelemetry::IsConnectedToServer()))
    bNewState = true;
  else
    bNewState = false;

  if (bOnTop != bNewState)
  {
    bOnTop = bNewState;

    hide();

    if (bOnTop)
      setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    else
      setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);

    show();
  }
}

void nsQtMainWindow::ProcessTelemetry(void* pUnuseed)
{
  if (!s_pWidget)
    return;

  nsTelemetryMessage Msg;

  while (nsTelemetry::RetrieveMessage(' APP', Msg) == NS_SUCCESS)
  {
    switch (Msg.GetMessageID())
    {
      case 'ASRT':
      {
        nsString sSourceFile, sFunction, sExpression, sMessage;
        nsUInt32 uiLine = 0;

        Msg.GetReader() >> sSourceFile;
        Msg.GetReader() >> uiLine;
        Msg.GetReader() >> sFunction;
        Msg.GetReader() >> sExpression;
        Msg.GetReader() >> sMessage;

        nsQtLogDockWidget::s_pWidget->Log("");
        nsQtLogDockWidget::s_pWidget->Log("<<< Application Assertion >>>");
        nsQtLogDockWidget::s_pWidget->Log("");

        nsQtLogDockWidget::s_pWidget->Log(nsFmt("    Expression: '{0}'", sExpression));
        nsQtLogDockWidget::s_pWidget->Log("");

        nsQtLogDockWidget::s_pWidget->Log(nsFmt("    Message: '{0}'", sMessage));
        nsQtLogDockWidget::s_pWidget->Log("");

        nsQtLogDockWidget::s_pWidget->Log(nsFmt("   File: '{0}'", sSourceFile));

        nsQtLogDockWidget::s_pWidget->Log(nsFmt("   Line: {0}", uiLine));

        nsQtLogDockWidget::s_pWidget->Log(nsFmt("   In Function: '{0}'", sFunction));

        nsQtLogDockWidget::s_pWidget->Log("");

        nsQtLogDockWidget::s_pWidget->Log(">>> Application Assertion <<<");
        nsQtLogDockWidget::s_pWidget->Log("");
      }
      break;
    }
  }
}

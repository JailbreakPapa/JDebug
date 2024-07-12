#include <Proto/MainWindow.moc.h>

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

  SetupNetworkTimer();
}

nsQtMainWindow::~nsQtMainWindow()
{
  for (nsInt32 i = 0; i < 10; ++i)
  {
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

}

void nsQtMainWindow::SetupNetworkTimer()
{

}


void nsQtMainWindow::UpdateNetwork()
{

  UpdateAlwaysOnTop();

}

void nsQtMainWindow::DockWidgetVisibilityChanged(bool bVisible)
{
  // TODO: add menu entry for qt main widget
}


void nsQtMainWindow::SetAlwaysOnTop(OnTopMode Mode)
{
}

void nsQtMainWindow::UpdateAlwaysOnTop()
{
  static bool bOnTop = false;

  bool bNewState = bOnTop;
  NS_IGNORE_UNUSED(bNewState);

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
}

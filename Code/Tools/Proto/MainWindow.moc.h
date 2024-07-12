#pragma once

#include <Foundation/Basics.h>
#include <QMainWindow>
#include <ads/DockManager.h>

#include <Proto/ui_MainWindow.h>

class nsQtMainWindow : public QMainWindow, public Ui_MainWindow
{
  enum OnTopMode
  {
    Never,
    Always,
    WhenConnected
  };

public:
  Q_OBJECT

public:
  nsQtMainWindow();
  ~nsQtMainWindow();

  static nsQtMainWindow* s_pWidget;

  static void ProcessTelemetry(void* pUnuseed);

  virtual void closeEvent(QCloseEvent* pEvent);

public Q_SLOTS:
  void DockWidgetVisibilityChanged(bool bVisible);

private Q_SLOTS:

private:
  void SetAlwaysOnTop(OnTopMode Mode);
  void UpdateAlwaysOnTop();
  void SetupNetworkTimer();
  void UpdateNetwork();

private:
  OnTopMode m_OnTopMode;
  QTimer* m_pNetworkTimer;

public:
  ads::CDockManager* m_DockManager = nullptr;
  QAction* m_pActionShowStatIn[10];
};


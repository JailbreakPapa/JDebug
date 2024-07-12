#pragma once

#include <TestFramework/TestFrameworkDLL.h>

struct android_app;

int nsAndroidMain(int argc, char** argv);

// \brief A small wrapper class around the android message loop to wait for window creation before starting tests.
class NS_TEST_DLL nsAndroidTestApplication
{
public:
  nsAndroidTestApplication(struct android_app* pApp);
  void HandleCmd(int32_t cmd);
  void AndroidRun();

  static void nsAndroidHandleCmd(struct android_app* pApp, int32_t cmd);

private:
  struct android_app* m_pApp = nullptr;
  bool m_bStarted = false;
};

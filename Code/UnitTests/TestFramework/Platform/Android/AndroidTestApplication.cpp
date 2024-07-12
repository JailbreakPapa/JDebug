#include <TestFramework/TestFrameworkPCH.h>

#if NS_ENABLED(NS_PLATFORM_ANDROID)

#  include <Foundation/Basics/Platform/Android/AndroidUtils.h>
#  include <Foundation/Logging/Log.h>
#  include <TestFramework/Platform/Android/AndroidTestApplication.h>
#  include <TestFramework/Utilities/TestSetup.h>
#  include <android/native_activity.h>
#  include <android_native_app_glue.h>

nsAndroidTestApplication::nsAndroidTestApplication(struct android_app* pApp)
  : m_pApp(pApp)
{
  pApp->userData = this;
  pApp->onAppCmd = nsAndroidHandleCmd;
  nsAndroidUtils::SetAndroidApp(pApp);
}

void nsAndroidTestApplication::HandleCmd(int32_t cmd)
{
  switch (cmd)
  {
    case APP_CMD_INIT_WINDOW:
      if (m_pApp->window != nullptr)
      {
        nsAndroidMain(0, nullptr);
        m_bStarted = true;

        int width = ANativeWindow_getWidth(m_pApp->window);
        int height = ANativeWindow_getHeight(m_pApp->window);
        nsLog::Info("Init Window: {}x{}", width, height);
      }
      break;
    default:
      break;
  }
}
void nsAndroidTestApplication::AndroidRun()
{
  bool bRun = true;
  while (true)
  {
    struct android_poll_source* pSource = nullptr;
    int iIdent = 0;
    int iEvents = 0;
    while ((iIdent = ALooper_pollAll(0, nullptr, &iEvents, (void**)&pSource)) >= 0)
    {
      if (pSource != nullptr)
        pSource->process(m_pApp, pSource);
    }

    // APP_CMD_INIT_WINDOW has not triggered yet. Engine is not yet started.
    if (!m_bStarted)
      continue;

    if (bRun && nsTestSetup::RunTests() != nsTestAppRun::Continue)
    {
      bRun = false;
      ANativeActivity_finish(m_pApp->activity);
    }
    if (m_pApp->destroyRequested)
    {
      break;
    }
  }
}

void nsAndroidTestApplication::nsAndroidHandleCmd(struct android_app* pApp, int32_t cmd)
{
  nsAndroidTestApplication* pAndroidApp = static_cast<nsAndroidTestApplication*>(pApp->userData);
  pAndroidApp->HandleCmd(cmd);
}

#endif
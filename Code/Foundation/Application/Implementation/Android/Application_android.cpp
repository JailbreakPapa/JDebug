#include <Foundation/FoundationPCH.h>

#if WD_ENABLED(WD_PLATFORM_ANDROID)

#  include <Foundation/Application/Application.h>
#  include <Foundation/Application/Implementation/Android/Application_android.h>
#  include <android/log.h>
#  include <android_native_app_glue.h>

static void wdAndroidHandleCmd(struct android_app* pApp, int32_t cmd)
{
  wdAndroidApplication* pAndroidApp = static_cast<wdAndroidApplication*>(pApp->userData);
  pAndroidApp->HandleCmd(cmd);
}

static int32_t wdAndroidHandleInput(struct android_app* pApp, AInputEvent* pEvent)
{
  wdAndroidApplication* pAndroidApp = static_cast<wdAndroidApplication*>(pApp->userData);
  return pAndroidApp->HandleInput(pEvent);
}

wdAndroidApplication::wdAndroidApplication(struct android_app* pApp, wdApplication* pEzApp)
  : m_pApp(pApp)
  , m_pEzApp(pEzApp)
{
  pApp->userData = this;
  pApp->onAppCmd = wdAndroidHandleCmd;
  pApp->onInputEvent = wdAndroidHandleInput;
  //#TODO: acquire sensors, set app->onAppCmd, set app->onInputEvent
}

wdAndroidApplication::~wdAndroidApplication() {}

void wdAndroidApplication::AndroidRun()
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

      HandleIdent(iIdent);
    }
    if (bRun && m_pEzApp->Run() != wdApplication::Execution::Continue)
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

void wdAndroidApplication::HandleCmd(int32_t cmd)
{
  //#TODO:
}

int32_t wdAndroidApplication::HandleInput(AInputEvent* pEvent)
{
  //#TODO:
  return 0;
}

void wdAndroidApplication::HandleIdent(wdInt32 iIdent)
{
  //#TODO:
}

WD_FOUNDATION_DLL void wdAndroidRun(struct android_app* pApp, wdApplication* pEzApp)
{
  wdAndroidApplication androidApp(pApp, pEzApp);

  if (wdRun_Startup(pEzApp).Succeeded())
  {
    androidApp.AndroidRun();
  }
  wdRun_Shutdown(pEzApp);

  const int iReturnCode = pEzApp->GetReturnCode();
  if (iReturnCode != 0)
  {
    const char* szReturnCode = pEzApp->TranslateReturnCode();
    if (szReturnCode != nullptr && szReturnCode[0] != '\0')
      __android_log_print(ANDROID_LOG_ERROR, "wdEngine", "Return Code: '%s'", szReturnCode);
  }
}

#endif

WD_STATICLINK_FILE(Foundation, Foundation_Application_Implementation_Android_Application_android);

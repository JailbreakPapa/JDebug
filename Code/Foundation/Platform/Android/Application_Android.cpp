#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_ANDROID)

#  include <Foundation/Application/Application.h>
#  include <Foundation/Application/Implementation/Android/Application_android.h>
#  include <Foundation/Logging/Log.h>
#  include <android/log.h>
#  include <android_native_app_glue.h>

static void nsAndroidHandleCmd(struct android_app* pApp, int32_t cmd)
{
  nsAndroidApplication* pAndroidApp = static_cast<nsAndroidApplication*>(pApp->userData);
  pAndroidApp->HandleCmd(cmd);
}

static int32_t nsAndroidHandleInput(struct android_app* pApp, AInputEvent* pEvent)
{
  nsAndroidApplication* pAndroidApp = static_cast<nsAndroidApplication*>(pApp->userData);
  return pAndroidApp->HandleInput(pEvent);
}

nsAndroidApplication::nsAndroidApplication(struct android_app* pApp, nsApplication* pNsApp)
  : m_pApp(pApp)
  , m_pNsApp(pNsApp)
{
  pApp->userData = this;
  pApp->onAppCmd = nsAndroidHandleCmd;
  pApp->onInputEvent = nsAndroidHandleInput;
  // #TODO: acquire sensors, set app->onAppCmd, set app->onInputEvent
}

nsAndroidApplication::~nsAndroidApplication() {}

void nsAndroidApplication::AndroidRun()
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

    // APP_CMD_INIT_WINDOW has not triggered yet. Engine is not yet started.
    if (!m_bStarted)
      continue;

    if (bRun && m_pNsApp->Run() != nsApplication::Execution::Continue)
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

void nsAndroidApplication::HandleCmd(int32_t cmd)
{
  switch (cmd)
  {
    case APP_CMD_INIT_WINDOW:
      if (m_pApp->window != nullptr)
      {
        NS_VERIFY(nsRun_Startup(m_pNsApp).Succeeded(), "Failed to startup engine");
        m_bStarted = true;

        int width = ANativeWindow_getWidth(m_pApp->window);
        int height = ANativeWindow_getHeight(m_pApp->window);
        nsLog::Info("Init Window: {}x{}", width, height);
      }
      break;
    case APP_CMD_TERM_WINDOW:
      m_pNsApp->RequestQuit();
      break;
    default:
      break;
  }
  nsAndroidUtils::s_AppCommandEvent.Broadcast(cmd);
}

int32_t nsAndroidApplication::HandleInput(AInputEvent* pEvent)
{
  nsAndroidInputEvent event;
  event.m_pEvent = pEvent;
  event.m_bHandled = false;

  nsAndroidUtils::s_InputEvent.Broadcast(event);
  return event.m_bHandled ? 1 : 0;
}

void nsAndroidApplication::HandleIdent(nsInt32 iIdent)
{
  // #TODO:
}

NS_FOUNDATION_DLL void nsAndroidRun(struct android_app* pApp, nsApplication* pNsApp)
{
  nsAndroidApplication androidApp(pApp, pNsApp);

  // This call will loop until APP_CMD_INIT_WINDOW is emitted which triggers nsRun_Startup
  androidApp.AndroidRun();

  nsRun_Shutdown(pNsApp);

  const int iReturnCode = pNsApp->GetReturnCode();
  if (iReturnCode != 0)
  {
    const char* szReturnCode = pNsApp->TranslateReturnCode();
    if (szReturnCode != nullptr && szReturnCode[0] != '\0')
      __android_log_print(ANDROID_LOG_ERROR, "nsEngine", "Return Code: '%s'", szReturnCode);
  }
}

#endif

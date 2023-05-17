#pragma once

#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#if WD_ENABLED(WD_PLATFORM_ANDROID)

#  include <Foundation/Basics.h>
#  include <Foundation/Strings/String.h>

class wdApplication;
struct AInputEvent;

class wdAndroidApplication
{
public:
  wdAndroidApplication(struct android_app* pApp, wdApplication* pEzApp);
  ~wdAndroidApplication();
  void AndroidRun();
  void HandleCmd(int32_t cmd);
  int32_t HandleInput(AInputEvent* pEvent);
  void HandleIdent(wdInt32 iIdent);

private:
  struct android_app* m_pApp;
  wdApplication* m_pEzApp;
};

#endif

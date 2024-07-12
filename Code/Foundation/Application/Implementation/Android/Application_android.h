#pragma once

#include <Foundation/FoundationInternal.h>
NS_FOUNDATION_INTERNAL_HEADER

#if NS_ENABLED(NS_PLATFORM_ANDROID)

#  include <Foundation/Basics.h>
#  include <Foundation/Strings/String.h>

class nsApplication;
struct AInputEvent;

class nsAndroidApplication
{
public:
  nsAndroidApplication(struct android_app* pApp, nsApplication* pNsApp);
  ~nsAndroidApplication();
  void AndroidRun();
  void HandleCmd(int32_t cmd);
  int32_t HandleInput(AInputEvent* pEvent);
  void HandleIdent(nsInt32 iIdent);

private:
  struct android_app* m_pApp;
  nsApplication* m_pNsApp;
  bool m_bStarted = false;
};

#endif

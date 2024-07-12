#pragma once

#if NS_DISABLED(NS_PLATFORM_ANDROID)
#  error "android util header should only be included in android builds!"
#endif
#include <Foundation/Communication/Event.h>

struct android_app;
struct _JavaVM;
using JavaVM = _JavaVM;
struct _JNIEnv;
using JNIEnv = _JNIEnv;
class _jobject;
using jobject = _jobject*;
struct AInputEvent;

/// \brief Event fired by nsAndroidUtils::s_InputEvent.
/// Event listeners should inspect m_pEvent and set m_bHandled to true if they handled the event.
struct nsAndroidInputEvent
{
  AInputEvent* m_pEvent = nullptr;
  bool m_bHandled = false;
};

class NS_FOUNDATION_DLL nsAndroidUtils
{
public:
  static void SetAndroidApp(android_app* app);
  static android_app* GetAndroidApp();

  static void SetAndroidJavaVM(JavaVM* vm);
  static JavaVM* GetAndroidJavaVM();

  static void SetAndroidNativeActivity(jobject nativeActivity);
  static jobject GetAndroidNativeActivity();

public:
  static nsEvent<nsAndroidInputEvent&> s_InputEvent;
  static nsEvent<nsInt32> s_AppCommandEvent;

private:
  static android_app* s_app;
  static JavaVM* s_vm;
  static jobject s_na;
};

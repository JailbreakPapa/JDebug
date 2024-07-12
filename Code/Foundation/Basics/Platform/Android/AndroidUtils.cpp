#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_ANDROID)
#  include <Foundation/Basics/Platform/Android/AndroidUtils.h>
#  include <android_native_app_glue.h>

android_app* nsAndroidUtils::s_app;
JavaVM* nsAndroidUtils::s_vm;
jobject nsAndroidUtils::s_na;
nsEvent<nsAndroidInputEvent&> nsAndroidUtils::s_InputEvent;
nsEvent<nsInt32> nsAndroidUtils::s_AppCommandEvent;

void nsAndroidUtils::SetAndroidApp(android_app* app)
{
  s_app = app;
  SetAndroidJavaVM(s_app->activity->vm);
  SetAndroidNativeActivity(s_app->activity->clazz);
}

android_app* nsAndroidUtils::GetAndroidApp()
{
  return s_app;
}

void nsAndroidUtils::SetAndroidJavaVM(JavaVM* vm)
{
  s_vm = vm;
}

JavaVM* nsAndroidUtils::GetAndroidJavaVM()
{
  return s_vm;
}

void nsAndroidUtils::SetAndroidNativeActivity(jobject nativeActivity)
{
  s_na = nativeActivity;
}

jobject nsAndroidUtils::GetAndroidNativeActivity()
{
  return s_na;
}

#endif

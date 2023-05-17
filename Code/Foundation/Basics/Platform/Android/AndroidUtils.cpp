#include <Foundation/FoundationPCH.h>

#if WD_ENABLED(WD_PLATFORM_ANDROID)
#  include <Foundation/Basics/Platform/Android/AndroidUtils.h>
#  include <android_native_app_glue.h>

android_app* wdAndroidUtils::s_app;
JavaVM* wdAndroidUtils::s_vm;

void wdAndroidUtils::SetAndroidApp(android_app* app)
{
  s_app = app;
  SetAndroidJavaVM(s_app->activity->vm);
}

android_app* wdAndroidUtils::GetAndroidApp()
{
  return s_app;
}

void wdAndroidUtils::SetAndroidJavaVM(JavaVM* vm)
{
  s_vm = vm;
}

JavaVM* wdAndroidUtils::GetAndroidJavaVM()
{
  return s_vm;
}

#endif


WD_STATICLINK_FILE(Foundation, Foundation_Basics_Platform_Android_AndroidUtils);

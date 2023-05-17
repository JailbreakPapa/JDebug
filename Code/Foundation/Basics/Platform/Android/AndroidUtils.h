#pragma once

#if WD_DISABLED(WD_PLATFORM_ANDROID)
#  error "android util header should only be included in android builds!"
#endif

struct android_app;
struct _JavaVM;
typedef _JavaVM JavaVM;
struct _JNIEnv;
typedef _JNIEnv JNIEnv;

class WD_FOUNDATION_DLL wdAndroidUtils
{
public:
  static void SetAndroidApp(android_app* app);
  static android_app* GetAndroidApp();

  static void SetAndroidJavaVM(JavaVM* vm);
  static JavaVM* GetAndroidJavaVM();

private:
  static android_app* s_app;
  static JavaVM* s_vm;
};

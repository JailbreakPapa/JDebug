#pragma once

/// \file
#include <Foundation/Application/Application.h>
#include <Foundation/Basics/Platform/Android/AndroidUtils.h>

class wdApplication;

extern WD_FOUNDATION_DLL void wdAndroidRun(struct android_app* pAndroidApp, wdApplication* pApp);

namespace wdApplicationDetails
{
  template <typename AppClass, typename... Args>
  void EntryFunc(struct android_app* pAndroidApp, Args&&... arguments)
  {
    alignas(WD_ALIGNMENT_OF(AppClass)) static char appBuffer[sizeof(AppClass)]; // Not on the stack to cope with smaller stacks.
    wdAndroidUtils::SetAndroidApp(pAndroidApp);
    AppClass* pApp = new (appBuffer) AppClass(std::forward<Args>(arguments)...);

    wdAndroidRun(pAndroidApp, pApp);

    pApp->~AppClass();
    memset(pApp, 0, sizeof(AppClass));
  }
} // namespace wdApplicationDetails


/// \brief Same as WD_APPLICATION_ENTRY_POINT but should be used for applications that shall always show a console window.
#define WD_CONSOLEAPP_ENTRY_POINT(...) WD_APPLICATION_ENTRY_POINT(__VA_ARGS__)

/// \brief This macro allows for easy creation of application entry points (since they can't be placed in DLLs)
///
/// Just use the macro in a cpp file of your application and supply your app class (must be derived from wdApplication).
/// The additional (optional) parameters are passed to the constructor of your app class.
#define WD_APPLICATION_ENTRY_POINT(AppClass, ...)                                                                                \
  alignas(WD_ALIGNMENT_OF(AppClass)) static char appBuffer[sizeof(AppClass)]; /* Not on the stack to cope with smaller stacks */ \
  WD_APPLICATION_ENTRY_POINT_CODE_INJECTION                                                                                      \
  extern "C" void android_main(struct android_app* app) { ::wdApplicationDetails::EntryFunc<AppClass>(app, ##__VA_ARGS__); }

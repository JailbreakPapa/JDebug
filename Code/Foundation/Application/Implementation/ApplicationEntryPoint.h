#pragma once

#include <Foundation/Basics.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)

#  include <Foundation/Application/Implementation/Win/ApplicationEntryPoint_win.h>

#elif WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)

#  include <Foundation/Application/Implementation/uwp/ApplicationEntryPoint_uwp.h>

#elif WD_ENABLED(WD_PLATFORM_OSX) || WD_ENABLED(WD_PLATFORM_LINUX)

#  include <Foundation/Application/Implementation/Posix/ApplicationEntryPoint_posix.h>

#elif WD_ENABLED(WD_PLATFORM_ANDROID)

#  include <Foundation/Application/Implementation/Android/ApplicationEntryPoint_android.h>

#else
#  error "Missing definition of platform specific entry point!"
#endif

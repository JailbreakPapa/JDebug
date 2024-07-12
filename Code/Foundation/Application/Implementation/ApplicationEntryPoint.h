#pragma once

#include <Foundation/Basics.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)

#  include <Foundation/Application/Implementation/Win/ApplicationEntryPoint_win.h>

#elif NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)

#  include <Foundation/Application/Implementation/uwp/ApplicationEntryPoint_uwp.h>

#elif NS_ENABLED(NS_PLATFORM_OSX) || NS_ENABLED(NS_PLATFORM_LINUX) || NS_ENABLED(NS_PLATFORM_PLAYSTATION_5)

#  include <Foundation/Application/Implementation/Posix/ApplicationEntryPoint_posix.h>

#elif NS_ENABLED(NS_PLATFORM_ANDROID)

#  include <Foundation/Application/Implementation/Android/ApplicationEntryPoint_android.h>
#else
#  error "Missing definition of platform specific entry point!"
#endif

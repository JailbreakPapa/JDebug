#pragma once

#if defined(_WINDOWS) || defined(_WIN32)
#  undef NS_PLATFORM_WINDOWS
#  define NS_PLATFORM_WINDOWS NS_ON

// further distinction between desktop, UWP etc. is done in Platform_win.h

#elif defined(__APPLE__) && defined(__MACH__)
#  include <TargetConditionals.h>

#  if TARGET_OS_MAC == 1
#    undef NS_PLATFORM_OSX
#    define NS_PLATFORM_OSX NS_ON
#  elif TARGET_OS_IPHONE == 1 || TARGET_IPHONE_SIMULATOR == 1
#    undef NS_PLATFORM_IOS
#    define NS_PLATFORM_IOS NS_ON
#  endif

#elif defined(ANDROID)

#  undef NS_PLATFORM_ANDROID
#  define NS_PLATFORM_ANDROID NS_ON

#elif defined(__linux)

#  undef NS_PLATFORM_LINUX
#  define NS_PLATFORM_LINUX NS_ON

// #elif defined(...)
//   #undef NS_PLATFORM_LINUX
//   #define NS_PLATFORM_LINUX NS_ON
#elif defined(__PROSPERO__)

#  undef NS_PLATFORM_CONSOLE
#  undef NS_PLATFORM_PLAYSTATION_5
#  define NS_PLATFORM_CONSOLE NS_ON
#  define NS_PLATFORM_PLAYSTATION_5 NS_ON

#else
#  error "Unknown Platform."
#endif

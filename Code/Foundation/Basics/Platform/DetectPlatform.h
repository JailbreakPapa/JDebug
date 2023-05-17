#pragma once

#if defined(_WINDOWS) || defined(_WIN32)
#  undef WD_PLATFORM_WINDOWS
#  define WD_PLATFORM_WINDOWS WD_ON

// further distinction between desktop, UWP etc. is done in Platform_win.h

#elif defined(__APPLE__) && defined(__MACH__)
#  include <TargetConditionals.h>

#  if TARGET_OS_MAC == 1
#    undef WD_PLATFORM_OSX
#    define WD_PLATFORM_OSX WD_ON
#  elif TARGET_OS_IPHONE == 1 || TARGET_IPHONE_SIMULATOR == 1
#    undef WD_PLATFORM_IOS
#    define WD_PLATFORM_IOS WD_ON
#  endif

#elif defined(ANDROID)

#  undef WD_PLATFORM_ANDROID
#  define WD_PLATFORM_ANDROID WD_ON

#elif defined(__linux)

#  undef WD_PLATFORM_LINUX
#  define WD_PLATFORM_LINUX WD_ON

//#elif defined(...)
//  #undef WD_PLATFORM_LINUX
//  #define WD_PLATFORM_LINUX WD_ON
#else
#  error "Unknown Platform."
#endif

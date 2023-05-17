#pragma once

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/Threading/Implementation/Win/ThreadingDeclarations_win.h>
#elif WD_ENABLED(WD_PLATFORM_OSX) || WD_ENABLED(WD_PLATFORM_LINUX) || WD_ENABLED(WD_PLATFORM_ANDROID)
#  include <Foundation/Threading/Implementation/Posix/ThreadingDeclarations_posix.h>
#else
#  error "Unknown Platform."
#endif

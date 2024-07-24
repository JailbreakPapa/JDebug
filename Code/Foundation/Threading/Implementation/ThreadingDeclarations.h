#pragma once

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
#  include <Foundation/Threading/Implementation/Win/ThreadingDeclarations_win.h>
#elif NS_ENABLED(NS_PLATFORM_OSX) || NS_ENABLED(NS_PLATFORM_LINUX) || NS_ENABLED(NS_PLATFORM_ANDROID)
#  include <Foundation/Threading/Implementation/Posix/ThreadingDeclarations_posix.h>
#else
#  error "Unknown Platform."
#endif

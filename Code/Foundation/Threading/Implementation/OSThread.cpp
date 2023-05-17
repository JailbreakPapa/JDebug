#include <Foundation/FoundationPCH.h>

#include <Foundation/Threading/Thread.h>

// Include inline file
#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/Threading/Implementation/Win/OSThread_win.h>
#elif WD_ENABLED(WD_PLATFORM_OSX) || WD_ENABLED(WD_PLATFORM_LINUX) || WD_ENABLED(WD_PLATFORM_ANDROID)
#  include <Foundation/Threading/Implementation/Posix/OSThread_posix.h>
#else
#  error "Thread functions are not implemented on current platform"
#endif



WD_STATICLINK_FILE(Foundation, Foundation_Threading_Implementation_OSThread);

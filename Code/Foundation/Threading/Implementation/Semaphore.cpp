#include <Foundation/FoundationPCH.h>

#include <Foundation/Threading/Semaphore.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/Threading/Implementation/Win/Semaphore_win.h>
#elif WD_ENABLED(WD_PLATFORM_OSX) || WD_ENABLED(WD_PLATFORM_LINUX) || WD_ENABLED(WD_PLATFORM_ANDROID)
#  include <Foundation/Threading/Implementation/Posix/Semaphore_posix.h>
#else
#  error "Semaphore is not implemented on current platform"
#endif


WD_STATICLINK_FILE(Foundation, Foundation_Threading_Implementation_Semaphore);

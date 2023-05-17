#include <Foundation/FoundationPCH.h>

#include <Foundation/Memory/Policies/GuardedAllocation.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/Memory/Policies/Win/GuardedAllocation_win.h>
#elif WD_ENABLED(WD_PLATFORM_OSX) || WD_ENABLED(WD_PLATFORM_LINUX) || WD_ENABLED(WD_PLATFORM_ANDROID)
#  include <Foundation/Memory/Policies/Posix/GuardedAllocation_posix.h>
#else
#  error "wdGuardedAllocation is not implemented on current platform"
#endif

WD_STATICLINK_FILE(Foundation, Foundation_Memory_Policies_GuardedAllocation);

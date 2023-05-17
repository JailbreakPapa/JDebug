#include <Foundation/FoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/Threading/ThreadUtils.h>
#include <Foundation/Time/Time.h>

// clang-format off
WD_BEGIN_SUBSYSTEM_DECLARATION(Foundation, ThreadUtils)

  // no dependencies

  ON_BASESYSTEMS_STARTUP
  {
    wdThreadUtils::Initialize();
  }

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

// Include inline file
#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/Threading/Implementation/Win/ThreadUtils_win.h>
#elif WD_ENABLED(WD_PLATFORM_OSX) || WD_ENABLED(WD_PLATFORM_LINUX) || WD_ENABLED(WD_PLATFORM_ANDROID)
#  include <Foundation/Threading/Implementation/Posix/ThreadUtils_posix.h>
#else
#  error "ThreadUtils functions are not implemented on current platform"
#endif

WD_STATICLINK_FILE(Foundation, Foundation_Threading_Implementation_ThreadUtils);

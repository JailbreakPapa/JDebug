#include <Foundation/FoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/Threading/ThreadUtils.h>
#include <Foundation/Time/Time.h>

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(Foundation, ThreadUtils)

  // no dependencies

  ON_BASESYSTEMS_STARTUP
  {
    nsThreadUtils::Initialize();
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

NS_STATICLINK_FILE(Foundation, Foundation_Threading_Implementation_ThreadUtils);

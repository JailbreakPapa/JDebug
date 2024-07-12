#include <Foundation/FoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/System/StackTracer.h>

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(Foundation, StackTracer)

  BEGIN_SUBSYSTEM_DEPENDENCIES
  "Time"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    nsPlugin::Events().AddEventHandler(nsStackTracer::OnPluginEvent);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsPlugin::Events().RemoveEventHandler(nsStackTracer::OnPluginEvent);
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

void nsStackTracer::PrintStackTrace(const nsArrayPtr<void*>& trace, nsStackTracer::PrintFunc printFunc)
{
  char buffer[32];
  const nsUInt32 uiNumTraceEntries = trace.GetCount();
  for (nsUInt32 i = 0; i < uiNumTraceEntries; i++)
  {
    nsStringUtils::snprintf(buffer, NS_ARRAY_SIZE(buffer), "%s%p", i == 0 ? "" : "|", trace[i]);
    printFunc(buffer);
  }
}

NS_STATICLINK_FILE(Foundation, Foundation_System_Implementation_StackTracer);

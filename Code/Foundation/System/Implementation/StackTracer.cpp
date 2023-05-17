#include <Foundation/FoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/System/StackTracer.h>

// Include inline file
#if WD_ENABLED(WD_PLATFORM_WINDOWS)

#  if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)
#    include <Foundation/System/Implementation/Win/StackTracer_uwp.h>
#  else
#    include <Foundation/System/Implementation/Win/StackTracer_win.h>
#  endif

#elif WD_ENABLED(WD_PLATFORM_OSX) || WD_ENABLED(WD_PLATFORM_LINUX)
#  include <Foundation/System/Implementation/Posix/StackTracer_posix.h>
#elif WD_ENABLED(WD_PLATFORM_ANDROID)
#  include <Foundation/System/Implementation/Android/StackTracer_android.h>
#else
#  error "StackTracer is not implemented on current platform"
#endif

// clang-format off
WD_BEGIN_SUBSYSTEM_DECLARATION(Foundation, StackTracer)

  BEGIN_SUBSYSTEM_DEPENDENCIES
  "Time"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    wdPlugin::Events().AddEventHandler(wdStackTracer::OnPluginEvent);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    wdPlugin::Events().RemoveEventHandler(wdStackTracer::OnPluginEvent);
  }

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

void wdStackTracer::PrintStackTrace(const wdArrayPtr<void*>& trace, wdStackTracer::PrintFunc printFunc)
{
  char buffer[32];
  const wdUInt32 uiNumTraceEntries = trace.GetCount();
  for (wdUInt32 i = 0; i < uiNumTraceEntries; i++)
  {
    wdStringUtils::snprintf(buffer, WD_ARRAY_SIZE(buffer), "%s%p", i == 0 ? "" : "|", trace[i]);
    printFunc(buffer);
  }
}

WD_STATICLINK_FILE(Foundation, Foundation_System_Implementation_StackTracer);

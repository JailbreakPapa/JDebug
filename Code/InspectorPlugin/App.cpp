#include <InspectorPlugin/InspectorPluginPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <Foundation/System/SystemInformation.h>
#include <Foundation/Threading/Thread.h>
#include <Foundation/Utilities/Stats.h>

static nsAssertHandler g_PreviousAssertHandler = nullptr;

static bool TelemetryAssertHandler(const char* szSourceFile, nsUInt32 uiLine, const char* szFunction, const char* szExpression, const char* szAssertMsg)
{
  if (nsTelemetry::IsConnectedToClient())
  {
    nsTelemetryMessage msg;
    msg.SetMessageID(' APP', 'ASRT');
    msg.GetWriter() << szSourceFile;
    msg.GetWriter() << uiLine;
    msg.GetWriter() << szFunction;
    msg.GetWriter() << szExpression;
    msg.GetWriter() << szAssertMsg;

    nsTelemetry::Broadcast(nsTelemetry::Reliable, msg);

    // messages might not arrive, if the network does not get enough time to transmit them
    // since we are crashing the application in (half) 'a second', we need to make sure the network traffic has indeed been sent
    for (nsUInt32 i = 0; i < 5; ++i)
    {
      nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(100));
      nsTelemetry::UpdateNetwork();
    }
  }

  if (g_PreviousAssertHandler)
    return g_PreviousAssertHandler(szSourceFile, uiLine, szFunction, szExpression, szAssertMsg);

  return true;
}

void AddTelemetryAssertHandler()
{
  g_PreviousAssertHandler = nsGetAssertHandler();
  nsSetAssertHandler(TelemetryAssertHandler);
}

void RemoveTelemetryAssertHandler()
{
  nsSetAssertHandler(g_PreviousAssertHandler);
  g_PreviousAssertHandler = nullptr;
}

void SetAppStats()
{
  nsStringBuilder sOut;
  const nsSystemInformation info = nsSystemInformation::Get();

  nsStats::SetStat("Platform/Name", info.GetPlatformName());

  nsStats::SetStat("Hardware/CPU Cores", info.GetCPUCoreCount());

  nsStats::SetStat("Hardware/RAM[GB]", info.GetInstalledMainMemory() / 1024.0f / 1024.0f / 1024.0f);

  sOut = info.Is64BitOS() ? "64 Bit" : "32 Bit";
  nsStats::SetStat("Platform/Architecture", sOut.GetData());

#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  sOut = "Debug";
#elif NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  sOut = "Dev";
#else
  sOut = "Release";
#endif
  nsStats::SetStat("Platform/Build", sOut.GetData());

#if NS_ENABLED(NS_USE_PROFILING)
  sOut = "Enabled";
#else
  sOut = "Disabled";
#endif
  nsStats::SetStat("Features/Profiling", sOut.GetData());

  if constexpr (nsAllocatorTrackingMode::Default >= nsAllocatorTrackingMode::AllocationStats)
    sOut = "Enabled";
  else
    sOut = "Disabled";

  nsStats::SetStat("Features/Allocation Tracking", sOut.GetData());

  if constexpr (nsAllocatorTrackingMode::Default >= nsAllocatorTrackingMode::AllocationStatsAndStacktraces)
    sOut = "Enabled";
  else
    sOut = "Disabled";

  nsStats::SetStat("Features/Allocation Stack Tracing", sOut.GetData());

#if NS_ENABLED(NS_PLATFORM_LITTLE_ENDIAN)
  sOut = "Little";
#else
  sOut = "Big";
#endif
  nsStats::SetStat("Platform/Endianess", sOut.GetData());
}



NS_STATICLINK_FILE(InspectorPlugin, InspectorPlugin_App);

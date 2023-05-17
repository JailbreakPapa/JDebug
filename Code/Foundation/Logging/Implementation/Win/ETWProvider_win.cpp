#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Implementation/Win/ETWProvider_win.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS)

#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#  include <TraceLoggingProvider.h>

// Workaround to support TraceLoggingProvider.h and /utf-8 compiler switch.
#  undef _TlgPragmaUtf8Begin
#  undef _TlgPragmaUtf8End
#  define _TlgPragmaUtf8Begin
#  define _TlgPragmaUtf8End
#  undef _tlgPragmaUtf8Begin
#  undef _tlgPragmaUtf8End
#  define _tlgPragmaUtf8Begin
#  define _tlgPragmaUtf8End

TRACELOGGING_DECLARE_PROVIDER(g_wdETWLogProvider);

// Define the GUID to use for the wd ETW Logger
// {BFD4350A-BA77-463D-B4BE-E30374E42494}
#  define WD_LOGGER_GUID (0xbfd4350a, 0xba77, 0x463d, 0xb4, 0xbe, 0xe3, 0x3, 0x74, 0xe4, 0x24, 0x94)

TRACELOGGING_DEFINE_PROVIDER(g_wdETWLogProvider, "wdLogProvider", WD_LOGGER_GUID);

wdETWProvider::wdETWProvider()
{
  TraceLoggingRegister(g_wdETWLogProvider);
}

wdETWProvider::~wdETWProvider()
{
  TraceLoggingUnregister(g_wdETWLogProvider);
}

void wdETWProvider::LogMessge(wdLogMsgType::Enum eventType, wdUInt8 uiIndentation, wdStringView sText)
{
  const wdStringBuilder sTemp = sText;

  TraceLoggingWrite(g_wdETWLogProvider, "LogMessge", TraceLoggingValue((int)eventType, "Type"), TraceLoggingValue(uiIndentation, "Indentation"),
    TraceLoggingValue(sTemp.GetData(), "Text"));
}

wdETWProvider& wdETWProvider::GetInstance()
{
  static wdETWProvider instance;
  return instance;
}
#endif


WD_STATICLINK_FILE(Foundation, Foundation_Logging_Implementation_Win_ETWProvider_win);

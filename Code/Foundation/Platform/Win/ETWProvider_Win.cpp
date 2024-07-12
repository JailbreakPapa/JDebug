#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS)

#  include <Foundation/Platform/Win/ETWProvider_Win.h>

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

TRACELOGGING_DECLARE_PROVIDER(g_nsETWLogProvider);

// Define the GUID to use for the ns ETW Logger
// {BFD4350A-BA77-463D-B4BE-E30374E42494}
#  define NS_LOGGER_GUID (0xbfd4350a, 0xba77, 0x463d, 0xb4, 0xbe, 0xe3, 0x3, 0x74, 0xe4, 0x24, 0x94)

TRACELOGGING_DEFINE_PROVIDER(g_nsETWLogProvider, "nsLogProvider", NS_LOGGER_GUID);

nsETWProvider::nsETWProvider()
{
  TraceLoggingRegister(g_nsETWLogProvider);
}

nsETWProvider::~nsETWProvider()
{
  TraceLoggingUnregister(g_nsETWLogProvider);
}

void nsETWProvider::LogMessage(nsLogMsgType::Enum eventType, nsUInt8 uiIndentation, nsStringView sText)
{
  const nsStringBuilder sTemp = sText;

  TraceLoggingWrite(g_nsETWLogProvider, "LogMessage", TraceLoggingValue((int)eventType, "Type"), TraceLoggingValue(uiIndentation, "Indentation"),
    TraceLoggingValue(sTemp.GetData(), "Text"));
}

nsETWProvider& nsETWProvider::GetInstance()
{
  static nsETWProvider instance;
  return instance;
}
#endif

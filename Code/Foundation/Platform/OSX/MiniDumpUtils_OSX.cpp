#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_OSX)

#  include <Foundation/System/MiniDumpUtils.h>

nsStatus nsMiniDumpUtils::WriteExternalProcessMiniDump(nsStringView sDumpFile, nsUInt32 uiProcessID, nsDumpType dumpTypeOverride)
{
  return nsStatus("Not implemented on OSX");
}

nsStatus nsMiniDumpUtils::LaunchMiniDumpTool(nsStringView sDumpFile, nsDumpType dumpTypeOverride)
{
  return nsStatus("Not implemented on OSX");
}

#endif

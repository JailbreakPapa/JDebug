#include <Foundation/FoundationInternal.h>
NS_FOUNDATION_INTERNAL_HEADER

#include <Foundation/System/MiniDumpUtils.h>

nsStatus nsMiniDumpUtils::WriteExternalProcessMiniDump(nsStringView sDumpFile, nsUInt32 uiProcessID, nsDumpType dumpTypeOverride)
{
  return nsStatus("Not implemented on Posix");
}

nsStatus nsMiniDumpUtils::LaunchMiniDumpTool(nsStringView sDumpFile, nsDumpType dumpTypeOverride)
{
  return nsStatus("Not implemented on Posix");
}

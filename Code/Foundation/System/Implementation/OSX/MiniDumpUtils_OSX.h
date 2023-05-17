#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <Foundation/System/MiniDumpUtils.h>

wdStatus wdMiniDumpUtils::WriteExternalProcessMiniDump(const char* szDumpFile, wdUInt32 uiProcessID)
{
  return wdStatus("Not implemented on OSX");
}

wdStatus wdMiniDumpUtils::LaunchMiniDumpTool(const char* szDumpFile)
{
  return wdStatus("Not implemented on OSX");
}

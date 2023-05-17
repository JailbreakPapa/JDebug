#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <Foundation/Configuration/Plugin.h>

typedef void* wdPluginModule;

void wdPlugin::GetPluginPaths(const char* szPluginName, wdStringBuilder& sOriginalFile, wdStringBuilder& sCopiedFile, wdUInt8 uiFileCopyNumber)
{
  WD_ASSERT_NOT_IMPLEMENTED;
}

wdResult UnloadPluginModule(wdPluginModule& Module, const char* szPluginFile)
{
  WD_ASSERT_NOT_IMPLEMENTED;

  return WD_FAILURE;
}

wdResult LoadPluginModule(const char* szFileToLoad, wdPluginModule& Module, const char* szPluginFile)
{
  WD_ASSERT_NOT_IMPLEMENTED;
  return WD_FAILURE;
}
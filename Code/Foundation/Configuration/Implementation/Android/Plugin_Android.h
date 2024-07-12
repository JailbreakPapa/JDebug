#include <Foundation/FoundationInternal.h>
NS_FOUNDATION_INTERNAL_HEADER

#include <Foundation/Configuration/Plugin.h>

using nsPluginModule = void*;

bool nsPlugin::PlatformNeedsPluginCopy()
{
  NS_ASSERT_NOT_IMPLEMENTED;
  return false;
}

void nsPlugin::GetPluginPaths(nsStringView sPluginName, nsStringBuilder& sOriginalFile, nsStringBuilder& sCopiedFile, nsUInt8 uiFileCopyNumber)
{
  NS_ASSERT_NOT_IMPLEMENTED;
}

nsResult UnloadPluginModule(nsPluginModule& Module, nsStringView sPluginFile)
{
  NS_ASSERT_NOT_IMPLEMENTED;

  return NS_FAILURE;
}

nsResult LoadPluginModule(nsStringView sFileToLoad, nsPluginModule& Module, nsStringView sPluginFile)
{
  NS_ASSERT_NOT_IMPLEMENTED;
  return NS_FAILURE;
}

#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Assets/AssetFileExtensionWhitelist.h>

wdMap<wdString, wdSet<wdString>> wdAssetFileExtensionWhitelist::s_ExtensionWhitelist;

void wdAssetFileExtensionWhitelist::AddAssetFileExtension(const char* szAssetType, const char* szAllowedFileExtension)
{
  wdStringBuilder sLowerType = szAssetType;
  sLowerType.ToLower();

  wdStringBuilder sLowerExt = szAllowedFileExtension;
  sLowerExt.ToLower();

  s_ExtensionWhitelist[sLowerType].Insert(sLowerExt);
}


bool wdAssetFileExtensionWhitelist::IsFileOnAssetWhitelist(const char* szAssetType, const char* szFile)
{
  wdStringBuilder sLowerExt = wdPathUtils::GetFileExtension(szFile);
  sLowerExt.ToLower();

  wdStringBuilder sLowerType = szAssetType;
  sLowerType.ToLower();

  wdHybridArray<wdString, 16> Types;
  sLowerType.Split(false, Types, ";");

  for (const auto& filter : Types)
  {
    if (s_ExtensionWhitelist[filter].Contains(sLowerExt))
      return true;
  }

  return false;
}

const wdSet<wdString>& wdAssetFileExtensionWhitelist::GetAssetFileExtensions(const char* szAssetType)
{
  return s_ExtensionWhitelist[szAssetType];
}

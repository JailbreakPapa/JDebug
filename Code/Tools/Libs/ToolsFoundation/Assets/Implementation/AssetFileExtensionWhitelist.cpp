#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Assets/AssetFileExtensionWhitelist.h>

nsMap<nsString, nsSet<nsString>> nsAssetFileExtensionWhitelist::s_ExtensionWhitelist;

void nsAssetFileExtensionWhitelist::AddAssetFileExtension(nsStringView sAssetType, nsStringView sAllowedFileExtension)
{
  nsStringBuilder sLowerType = sAssetType;
  sLowerType.ToLower();

  nsStringBuilder sLowerExt = sAllowedFileExtension;
  sLowerExt.ToLower();

  s_ExtensionWhitelist[sLowerType].Insert(sLowerExt);
}


bool nsAssetFileExtensionWhitelist::IsFileOnAssetWhitelist(nsStringView sAssetType, nsStringView sFile)
{
  nsStringBuilder sLowerExt = nsPathUtils::GetFileExtension(sFile);
  sLowerExt.ToLower();

  nsStringBuilder sLowerType = sAssetType;
  sLowerType.ToLower();

  nsHybridArray<nsString, 16> Types;
  sLowerType.Split(false, Types, ";");

  for (const auto& filter : Types)
  {
    if (s_ExtensionWhitelist[filter].Contains(sLowerExt))
      return true;
  }

  return false;
}

const nsSet<nsString>& nsAssetFileExtensionWhitelist::GetAssetFileExtensions(nsStringView sAssetType)
{
  return s_ExtensionWhitelist[sAssetType];
}

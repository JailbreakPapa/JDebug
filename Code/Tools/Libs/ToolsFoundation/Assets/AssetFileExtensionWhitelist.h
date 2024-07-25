#pragma once

#include <Foundation/Containers/Map.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/Strings/String.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

/// \brief A global whitelist for file extension that may be used as certain asset types
///
/// UI elements etc. may use this whitelist to detect whether a selected file is a valid candidate for an asset slot
class NS_TOOLSFOUNDATION_DLL nsAssetFileExtensionWhitelist
{
public:
  static void AddAssetFileExtension(nsStringView sAssetType, nsStringView sAllowedFileExtension);

  static bool IsFileOnAssetWhitelist(nsStringView sAssetType, nsStringView sFile);

  static const nsSet<nsString>& GetAssetFileExtensions(nsStringView sAssetType);

private:
  static nsMap<nsString, nsSet<nsString>> s_ExtensionWhitelist;
};

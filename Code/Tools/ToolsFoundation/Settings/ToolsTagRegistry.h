#pragma once

#include <Foundation/Containers/Map.h>
#include <Foundation/Types/Status.h>
#include <Foundation/Types/Variant.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

struct WD_TOOLSFOUNDATION_DLL wdToolsTag
{
  wdToolsTag() {}
  wdToolsTag(const char* szCategory, const char* szName, bool bBuiltIn = false)
    : m_sCategory(szCategory)
    , m_sName(szName)
    , m_bBuiltInTag(bBuiltIn)
  {
  }

  wdString m_sCategory;
  wdString m_sName;
  bool m_bBuiltInTag = false; ///< If set to true, this is a tag created by code that the user is not allowed to remove
};

class WD_TOOLSFOUNDATION_DLL wdToolsTagRegistry
{
public:
  /// \brief Removes all tags that are not specified as 'built-in'
  static void Clear();

  static void WriteToDDL(wdStreamWriter& inout_stream);
  static wdStatus ReadFromDDL(wdStreamReader& inout_stream);

  static bool AddTag(const wdToolsTag& tag);
  static bool RemoveTag(const char* szName);

  static void GetAllTags(wdHybridArray<const wdToolsTag*, 16>& out_tags);
  static void GetTagsByCategory(const wdArrayPtr<wdStringView>& categories, wdHybridArray<const wdToolsTag*, 16>& out_tags);

private:
  static wdMap<wdString, wdToolsTag> s_NameToTags;
};

#pragma once

#include <Foundation/Containers/Map.h>
#include <Foundation/Types/Status.h>
#include <Foundation/Types/Variant.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

struct NS_TOOLSFOUNDATION_DLL nsToolsTag
{
  nsToolsTag() = default;
  nsToolsTag(nsStringView sCategory, nsStringView sName, bool bBuiltIn = false)
    : m_sCategory(sCategory)
    , m_sName(sName)
    , m_bBuiltInTag(bBuiltIn)
  {
  }

  nsString m_sCategory;
  nsString m_sName;
  bool m_bBuiltInTag = false; ///< If set to true, this is a tag created by code that the user is not allowed to remove
};

class NS_TOOLSFOUNDATION_DLL nsToolsTagRegistry
{
public:
  /// \brief Removes all tags that are not specified as 'built-in'
  static void Clear();

  static void WriteToDDL(nsStreamWriter& inout_stream);
  static nsStatus ReadFromDDL(nsStreamReader& inout_stream);

  static bool AddTag(const nsToolsTag& tag);
  static bool RemoveTag(nsStringView sName);

  static void GetAllTags(nsHybridArray<const nsToolsTag*, 16>& out_tags);
  static void GetTagsByCategory(const nsArrayPtr<nsStringView>& categories, nsHybridArray<const nsToolsTag*, 16>& out_tags);

private:
  static nsMap<nsString, nsToolsTag> s_NameToTags;
};

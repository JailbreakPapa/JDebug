#pragma once

#include <Foundation/Strings/String.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

/// \brief A small helper class to implement a simple search pattern filter that can contain multiple parts.
///
/// The search text is split into multiple parts by spaces. A text passes the filter if it contains all parts.
/// The check is always case insensitive and the order of the parts does not matter.
/// It is also possible to exclude parts by prefixing them with a minus.
/// E.g. "com mesh" would pass all texts that contain "com" and "mesh" like nsMeshComponent.
/// "com -mesh" would pass nsLightComponent but would fail nsMeshComponent.
class NS_TOOLSFOUNDATION_DLL nsSearchPatternFilter
{
public:
  /// \brief Sets the search text and splits it into its part for faster checks.
  void SetSearchText(nsStringView sSearchText);

  const nsString& GetSearchText() const { return m_sSearchText; }
  bool IsEmpty() const { return m_sSearchText.IsEmpty(); }

  bool ContainsExclusions() const;

  /// \brief Determines whether the given text matches the filter patterns.
  bool PassesFilters(nsStringView sText) const;

private:
  nsString m_sSearchText;

  struct Part
  {
    nsStringView m_sPart;
    bool m_bExclude = false;
  };

  nsHybridArray<Part, 4> m_Parts;
};

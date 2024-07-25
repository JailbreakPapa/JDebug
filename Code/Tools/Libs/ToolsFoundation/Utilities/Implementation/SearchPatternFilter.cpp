#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Utilities/SearchPatternFilter.h>

void nsSearchPatternFilter::SetSearchText(nsStringView sSearchText)
{
  m_sSearchText = sSearchText;
  m_Parts.Clear();

  nsHybridArray<nsStringView, 4> searchParts;
  m_sSearchText.Split(false, searchParts, " ");

  for (auto& searchPart : searchParts)
  {
    auto& part = m_Parts.ExpandAndGetRef();
    part.m_sPart = searchPart;
    if (part.m_sPart.StartsWith("-"))
    {
      part.m_sPart.Shrink(1, 0);
      part.m_bExclude = true;
    }
  }
}

bool nsSearchPatternFilter::ContainsExclusions() const
{
  for (auto& part : m_Parts)
  {
    if (part.m_bExclude)
      return true;
  }

  return false;
}

bool nsSearchPatternFilter::PassesFilters(nsStringView sText) const
{
  for (auto& part : m_Parts)
  {
    bool failureResult = part.m_bExclude;
    if ((sText.FindSubString_NoCase(part.m_sPart) != nullptr) == failureResult)
      return false;
  }

  return true;
}

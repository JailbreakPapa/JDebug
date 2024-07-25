#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/CodeUtils/Preprocessor.h>
#include <ToolsFoundation/Utilities/PathPatternFilter.h>

void nsPathPattern::Configure(const nsStringView sText0)
{
  nsStringView text = sText0;

  text.Trim(" \t\r\n");

  const bool bStart = text.StartsWith("*");
  const bool bEnd = text.EndsWith("*");

  text.Trim("*");
  m_sString = text;

  if (bStart && bEnd)
    m_MatchType = MatchType::Contains;
  else if (bStart)
    m_MatchType = MatchType::EndsWith;
  else if (bEnd)
    m_MatchType = MatchType::StartsWith;
  else
    m_MatchType = MatchType::Exact;
}

bool nsPathPattern::Matches(const nsStringView sText) const
{
  switch (m_MatchType)
  {
    case MatchType::Exact:
      return sText.IsEqual_NoCase(m_sString.GetView());
    case MatchType::StartsWith:
      return sText.StartsWith_NoCase(m_sString);
    case MatchType::EndsWith:
      return sText.EndsWith_NoCase(m_sString);
    case MatchType::Contains:
      return sText.FindSubString_NoCase(m_sString) != nullptr;
  }

  NS_ASSERT_NOT_IMPLEMENTED;
  return false;
}

//////////////////////////////////////////////////////////////////////////

bool nsPathPatternFilter::PassesFilters(nsStringView sText) const
{
  for (const auto& filter : m_IncludePatterns)
  {
    // if any include pattern matches, that overrides the exclude patterns
    if (filter.Matches(sText))
      return true;
  }

  for (const auto& filter : m_ExcludePatterns)
  {
    // no include pattern matched, but any exclude pattern matches -> filter out
    if (filter.Matches(sText))
      return false;
  }

  // no filter matches at all -> include by default
  return true;
}

void nsPathPatternFilter::AddFilter(nsStringView sText, bool bIncludeFilter)
{
  nsStringBuilder text = sText;
  text.MakeCleanPath();
  text.Trim(" \t\r\n");

  if (text.IsEmpty() || text.StartsWith("//"))
    return;

  if (!text.StartsWith("*") && !text.StartsWith("/"))
    text.Prepend("/");

  if (bIncludeFilter)
    m_IncludePatterns.ExpandAndGetRef().Configure(text);
  else
    m_ExcludePatterns.ExpandAndGetRef().Configure(text);
}

nsResult nsPathPatternFilter::ReadConfigFile(nsStringView sFile, const nsDynamicArray<nsString>& preprocessorDefines)
{
  nsStringBuilder content;

  nsPreprocessor pp;
  pp.SetPassThroughLine(false);
  pp.SetPassThroughPragma(false);

  for (const auto& def : preprocessorDefines)
  {
    pp.AddCustomDefine(def).IgnoreResult();
  }

  // keep comments, because * and / can form a multi-line comment, and then we could lose vital information
  // instead only allow single-line comments and filter those out in AddFilter().
  if (pp.Process(sFile, content, true, true).Failed())
    return NS_FAILURE;

  nsDynamicArray<nsStringView> lines;

  content.Split(false, lines, "\n", "\r");

  bool bIncludeFilter = false;

  for (auto line : lines)
  {
    if (line.IsEqual_NoCase("[INCLUDE]"))
    {
      bIncludeFilter = true;
      continue;
    }

    if (line.IsEqual_NoCase("[EXCLUDE]"))
    {
      bIncludeFilter = false;
      continue;
    }

    AddFilter(line, bIncludeFilter);
  }

  return NS_SUCCESS;
}

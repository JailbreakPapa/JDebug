#include <Foundation/FoundationPCH.h>

#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Strings/StringUtils.h>
#include <Foundation/Strings/StringView.h>

nsUInt32 nsStringView::GetCharacter() const
{
  if (!IsValid())
    return 0;

  return nsUnicodeUtils::ConvertUtf8ToUtf32(m_pStart);
}

const char* nsStringView::GetData(nsStringBuilder& ref_sTempStorage) const
{
  ref_sTempStorage = *this;
  return ref_sTempStorage.GetData();
}

bool nsStringView::IsEqualN(nsStringView sOther, nsUInt32 uiCharsToCompare) const
{
  return nsStringUtils::IsEqualN(GetStartPointer(), sOther.GetStartPointer(), uiCharsToCompare, GetEndPointer(), sOther.GetEndPointer());
}

bool nsStringView::IsEqualN_NoCase(nsStringView sOther, nsUInt32 uiCharsToCompare) const
{
  return nsStringUtils::IsEqualN_NoCase(GetStartPointer(), sOther.GetStartPointer(), uiCharsToCompare, GetEndPointer(), sOther.GetEndPointer());
}

nsInt32 nsStringView::Compare(nsStringView sOther) const
{
  return nsStringUtils::Compare(GetStartPointer(), sOther.GetStartPointer(), GetEndPointer(), sOther.GetEndPointer());
}

nsInt32 nsStringView::CompareN(nsStringView sOther, nsUInt32 uiCharsToCompare) const
{
  return nsStringUtils::CompareN(GetStartPointer(), sOther.GetStartPointer(), uiCharsToCompare, GetEndPointer(), sOther.GetEndPointer());
}

nsInt32 nsStringView::Compare_NoCase(nsStringView sOther) const
{
  return nsStringUtils::Compare_NoCase(GetStartPointer(), sOther.GetStartPointer(), GetEndPointer(), sOther.GetEndPointer());
}

nsInt32 nsStringView::CompareN_NoCase(nsStringView sOther, nsUInt32 uiCharsToCompare) const
{
  return nsStringUtils::CompareN_NoCase(GetStartPointer(), sOther.GetStartPointer(), uiCharsToCompare, GetEndPointer(), sOther.GetEndPointer());
}

const char* nsStringView::ComputeCharacterPosition(nsUInt32 uiCharacterIndex) const
{
  const char* pos = GetStartPointer();
  if (nsUnicodeUtils::MoveToNextUtf8(pos, GetEndPointer(), uiCharacterIndex).Failed())
    return nullptr;

  return pos;
}

const char* nsStringView::FindSubString(nsStringView sStringToFind, const char* szStartSearchAt /*= nullptr*/) const
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = GetStartPointer();

  NS_ASSERT_DEV((szStartSearchAt >= GetStartPointer()) && (szStartSearchAt <= GetEndPointer()), "The given pointer to start searching at is not inside this strings valid range.");

  return nsStringUtils::FindSubString(szStartSearchAt, sStringToFind.GetStartPointer(), GetEndPointer(), sStringToFind.GetEndPointer());
}

const char* nsStringView::FindSubString_NoCase(nsStringView sStringToFind, const char* szStartSearchAt /*= nullptr*/) const
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = GetStartPointer();

  NS_ASSERT_DEV((szStartSearchAt >= GetStartPointer()) && (szStartSearchAt <= GetEndPointer()), "The given pointer to start searching at is not inside this strings valid range.");

  return nsStringUtils::FindSubString_NoCase(szStartSearchAt, sStringToFind.GetStartPointer(), GetEndPointer(), sStringToFind.GetEndPointer());
}

const char* nsStringView::FindLastSubString(nsStringView sStringToFind, const char* szStartSearchAt /*= nullptr*/) const
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = GetEndPointer();

  NS_ASSERT_DEV((szStartSearchAt >= GetStartPointer()) && (szStartSearchAt <= GetEndPointer()), "The given pointer to start searching at is not inside this strings valid range.");

  return nsStringUtils::FindLastSubString(GetStartPointer(), sStringToFind.GetStartPointer(), szStartSearchAt, GetEndPointer(), sStringToFind.GetEndPointer());
}

const char* nsStringView::FindLastSubString_NoCase(nsStringView sStringToFind, const char* szStartSearchAt /*= nullptr*/) const
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = GetEndPointer();

  NS_ASSERT_DEV((szStartSearchAt >= GetStartPointer()) && (szStartSearchAt <= GetEndPointer()), "The given pointer to start searching at is not inside this strings valid range.");

  return nsStringUtils::FindLastSubString_NoCase(GetStartPointer(), sStringToFind.GetStartPointer(), szStartSearchAt, GetEndPointer(), sStringToFind.GetEndPointer());
}

const char* nsStringView::FindWholeWord(const char* szSearchFor, nsStringUtils::NS_CHARACTER_FILTER isDelimiterCB, const char* szStartSearchAt /*= nullptr*/) const
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = GetStartPointer();

  NS_ASSERT_DEV((szStartSearchAt >= GetStartPointer()) && (szStartSearchAt <= GetEndPointer()), "The given pointer to start searching at is not inside this strings valid range.");

  return nsStringUtils::FindWholeWord(szStartSearchAt, szSearchFor, isDelimiterCB, GetEndPointer());
}

const char* nsStringView::FindWholeWord_NoCase(const char* szSearchFor, nsStringUtils::NS_CHARACTER_FILTER isDelimiterCB, const char* szStartSearchAt /*= nullptr*/) const
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = GetStartPointer();

  NS_ASSERT_DEV((szStartSearchAt >= GetStartPointer()) && (szStartSearchAt <= GetEndPointer()), "The given pointer to start searching at is not inside this strings valid range.");

  return nsStringUtils::FindWholeWord_NoCase(szStartSearchAt, szSearchFor, isDelimiterCB, GetEndPointer());
}

void nsStringView::Shrink(nsUInt32 uiShrinkCharsFront, nsUInt32 uiShrinkCharsBack)
{
  const char* pEnd = m_pStart + m_uiElementCount;

  while (IsValid() && (uiShrinkCharsFront > 0))
  {
    if (nsUnicodeUtils::MoveToNextUtf8(m_pStart, pEnd, 1).Failed())
    {
      *this = {};
      return;
    }

    --uiShrinkCharsFront;
  }

  while (IsValid() && (uiShrinkCharsBack > 0))
  {
    if (nsUnicodeUtils::MoveToPriorUtf8(pEnd, m_pStart, 1).Failed())
    {
      *this = {};
      return;
    }

    --uiShrinkCharsBack;
  }

  m_uiElementCount = static_cast<nsUInt32>(pEnd - m_pStart);
}

nsStringView nsStringView::GetShrunk(nsUInt32 uiShrinkCharsFront, nsUInt32 uiShrinkCharsBack) const
{
  nsStringView tmp = *this;
  tmp.Shrink(uiShrinkCharsFront, uiShrinkCharsBack);
  return tmp;
}

nsStringView nsStringView::GetSubString(nsUInt32 uiFirstCharacter, nsUInt32 uiNumCharacters) const
{
  if (!IsValid())
  {
    return {};
  }

  const char* pEnd = m_pStart + m_uiElementCount;

  const char* pSubStart = m_pStart;
  if (nsUnicodeUtils::MoveToNextUtf8(pSubStart, pEnd, uiFirstCharacter).Failed() || pSubStart == pEnd)
  {
    return {};
  }

  const char* pSubEnd = pSubStart;
  nsUnicodeUtils::MoveToNextUtf8(pSubEnd, pEnd, uiNumCharacters).IgnoreResult(); // if it fails, it just points to the end

  return nsStringView(pSubStart, pSubEnd);
}

void nsStringView::ChopAwayFirstCharacterUtf8()
{
  if (IsValid())
  {
    const char* pEnd = m_pStart + m_uiElementCount;
    nsUnicodeUtils::MoveToNextUtf8(m_pStart, pEnd, 1).AssertSuccess();
    m_uiElementCount = static_cast<nsUInt32>(pEnd - m_pStart);
  }
}

void nsStringView::ChopAwayFirstCharacterAscii()
{
  if (IsValid())
  {
    NS_ASSERT_DEBUG(nsUnicodeUtils::IsASCII(*m_pStart), "ChopAwayFirstCharacterAscii() was called on a non-ASCII character.");

    m_pStart += 1;
    m_uiElementCount--;
  }
}

bool nsStringView::TrimWordStart(nsStringView sWord)
{
  const bool bTrimAll = false;

  bool trimmed = false;

  do
  {
    if (!sWord.IsEmpty() && StartsWith_NoCase(sWord))
    {
      Shrink(nsStringUtils::GetCharacterCount(sWord.GetStartPointer(), sWord.GetEndPointer()), 0);
      trimmed = true;
    }

  } while (bTrimAll);

  return trimmed;
}

bool nsStringView::TrimWordEnd(nsStringView sWord)
{
  const bool bTrimAll = false;

  bool trimmed = false;

  do
  {
    if (!sWord.IsEmpty() && EndsWith_NoCase(sWord))
    {
      Shrink(0, nsStringUtils::GetCharacterCount(sWord.GetStartPointer(), sWord.GetEndPointer()));
      trimmed = true;
    }

  } while (bTrimAll);

  return trimmed;
}

nsStringView::iterator nsStringView::GetIteratorFront() const
{
  return begin(*this);
}

nsStringView::reverse_iterator nsStringView::GetIteratorBack() const
{
  return rbegin(*this);
}

bool nsStringView::HasAnyExtension() const
{
  return nsPathUtils::HasAnyExtension(*this);
}

bool nsStringView::HasExtension(nsStringView sExtension) const
{
  return nsPathUtils::HasExtension(*this, sExtension);
}

nsStringView nsStringView::GetFileExtension(bool bFullExtension /*= false*/) const
{
  return nsPathUtils::GetFileExtension(*this, bFullExtension);
}

nsStringView nsStringView::GetFileName() const
{
  return nsPathUtils::GetFileName(*this);
}

nsStringView nsStringView::GetFileNameAndExtension() const
{
  return nsPathUtils::GetFileNameAndExtension(*this);
}

nsStringView nsStringView::GetFileDirectory() const
{
  return nsPathUtils::GetFileDirectory(*this);
}

bool nsStringView::IsAbsolutePath() const
{
  return nsPathUtils::IsAbsolutePath(*this);
}

bool nsStringView::IsRelativePath() const
{
  return nsPathUtils::IsRelativePath(*this);
}

bool nsStringView::IsRootedPath() const
{
  return nsPathUtils::IsRootedPath(*this);
}

nsStringView nsStringView::GetRootedPathRootName() const
{
  return nsPathUtils::GetRootedPathRootName(*this);
}

#if NS_ENABLED(NS_INTEROP_STL_STRINGS)
nsStringView::nsStringView(const std::string_view& rhs)
{
  if (!rhs.empty())
  {
    m_pStart = rhs.data();
    m_uiElementCount = static_cast<nsUInt32>(rhs.size());
  }
}

nsStringView::nsStringView(const std::string& rhs)
{
  if (!rhs.empty())
  {
    m_pStart = rhs.data();
    m_uiElementCount = static_cast<nsUInt32>(rhs.size());
  }
}

std::string_view nsStringView::GetAsStdView() const
{
  return std::string_view(m_pStart, static_cast<size_t>(m_uiElementCount));
}

nsStringView::operator std::string_view() const
{
  return GetAsStdView();
}
#endif

#include <Foundation/FoundationPCH.h>

#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Strings/StringUtils.h>
#include <Foundation/Strings/StringView.h>

wdUInt32 wdStringView::GetCharacter() const
{
  if (!IsValid())
    return 0;

  return wdUnicodeUtils::ConvertUtf8ToUtf32(m_pStart);
}

const char* wdStringView::GetData(wdStringBuilder& ref_sTempStorage) const
{
  ref_sTempStorage = *this;
  return ref_sTempStorage.GetData();
}

bool wdStringView::IsEqualN(wdStringView sOther, wdUInt32 uiCharsToCompare) const
{
  return wdStringUtils::IsEqualN(GetStartPointer(), sOther.GetStartPointer(), uiCharsToCompare, GetEndPointer(), sOther.GetEndPointer());
}

bool wdStringView::IsEqualN_NoCase(wdStringView sOther, wdUInt32 uiCharsToCompare) const
{
  return wdStringUtils::IsEqualN_NoCase(GetStartPointer(), sOther.GetStartPointer(), uiCharsToCompare, GetEndPointer(), sOther.GetEndPointer());
}

wdInt32 wdStringView::Compare(wdStringView sOther) const
{
  return wdStringUtils::Compare(GetStartPointer(), sOther.GetStartPointer(), GetEndPointer(), sOther.GetEndPointer());
}

wdInt32 wdStringView::CompareN(wdStringView sOther, wdUInt32 uiCharsToCompare) const
{
  return wdStringUtils::CompareN(GetStartPointer(), sOther.GetStartPointer(), uiCharsToCompare, GetEndPointer(), sOther.GetEndPointer());
}

wdInt32 wdStringView::Compare_NoCase(wdStringView sOther) const
{
  return wdStringUtils::Compare_NoCase(GetStartPointer(), sOther.GetStartPointer(), GetEndPointer(), sOther.GetEndPointer());
}

wdInt32 wdStringView::CompareN_NoCase(wdStringView sOther, wdUInt32 uiCharsToCompare) const
{
  return wdStringUtils::CompareN_NoCase(GetStartPointer(), sOther.GetStartPointer(), uiCharsToCompare, GetEndPointer(), sOther.GetEndPointer());
}

const char* wdStringView::ComputeCharacterPosition(wdUInt32 uiCharacterIndex) const
{
  const char* pos = GetStartPointer();
  wdUnicodeUtils::MoveToNextUtf8(pos, GetEndPointer(), uiCharacterIndex);
  return pos;
}

const char* wdStringView::FindSubString(wdStringView sStringToFind, const char* szStartSearchAt /*= nullptr*/) const
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = GetStartPointer();

  WD_ASSERT_DEV((szStartSearchAt >= GetStartPointer()) && (szStartSearchAt <= GetEndPointer()), "The given pointer to start searching at is not inside this strings valid range.");

  return wdStringUtils::FindSubString(szStartSearchAt, sStringToFind.GetStartPointer(), GetEndPointer(), sStringToFind.GetEndPointer());
}

const char* wdStringView::FindSubString_NoCase(wdStringView sStringToFind, const char* szStartSearchAt /*= nullptr*/) const
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = GetStartPointer();

  WD_ASSERT_DEV((szStartSearchAt >= GetStartPointer()) && (szStartSearchAt <= GetEndPointer()), "The given pointer to start searching at is not inside this strings valid range.");

  return wdStringUtils::FindSubString_NoCase(szStartSearchAt, sStringToFind.GetStartPointer(), GetEndPointer(), sStringToFind.GetEndPointer());
}

const char* wdStringView::FindLastSubString(wdStringView sStringToFind, const char* szStartSearchAt /*= nullptr*/) const
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = GetEndPointer();

  WD_ASSERT_DEV((szStartSearchAt >= GetStartPointer()) && (szStartSearchAt <= GetEndPointer()), "The given pointer to start searching at is not inside this strings valid range.");

  return wdStringUtils::FindLastSubString(GetStartPointer(), sStringToFind.GetStartPointer(), szStartSearchAt, GetEndPointer(), sStringToFind.GetEndPointer());
}

const char* wdStringView::FindLastSubString_NoCase(wdStringView sStringToFind, const char* szStartSearchAt /*= nullptr*/) const
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = GetEndPointer();

  WD_ASSERT_DEV((szStartSearchAt >= GetStartPointer()) && (szStartSearchAt <= GetEndPointer()), "The given pointer to start searching at is not inside this strings valid range.");

  return wdStringUtils::FindLastSubString_NoCase(GetStartPointer(), sStringToFind.GetStartPointer(), szStartSearchAt, GetEndPointer(), sStringToFind.GetEndPointer());
}

const char* wdStringView::FindWholeWord(const char* szSearchFor, wdStringUtils::WD_CHARACTER_FILTER isDelimiterCB, const char* szStartSearchAt /*= nullptr*/) const
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = GetStartPointer();

  WD_ASSERT_DEV((szStartSearchAt >= GetStartPointer()) && (szStartSearchAt <= GetEndPointer()), "The given pointer to start searching at is not inside this strings valid range.");

  return wdStringUtils::FindWholeWord(szStartSearchAt, szSearchFor, isDelimiterCB, GetEndPointer());
}

const char* wdStringView::FindWholeWord_NoCase(const char* szSearchFor, wdStringUtils::WD_CHARACTER_FILTER isDelimiterCB, const char* szStartSearchAt /*= nullptr*/) const
{
  if (szStartSearchAt == nullptr)
    szStartSearchAt = GetStartPointer();

  WD_ASSERT_DEV((szStartSearchAt >= GetStartPointer()) && (szStartSearchAt <= GetEndPointer()), "The given pointer to start searching at is not inside this strings valid range.");

  return wdStringUtils::FindWholeWord_NoCase(szStartSearchAt, szSearchFor, isDelimiterCB, GetEndPointer());
}

void wdStringView::Shrink(wdUInt32 uiShrinkCharsFront, wdUInt32 uiShrinkCharsBack)
{
  while (IsValid() && (uiShrinkCharsFront > 0))
  {
    wdUnicodeUtils::MoveToNextUtf8(m_pStart, m_pEnd, 1);
    --uiShrinkCharsFront;
  }

  while (IsValid() && (uiShrinkCharsBack > 0))
  {
    wdUnicodeUtils::MoveToPriorUtf8(m_pEnd, 1);
    --uiShrinkCharsBack;
  }
}

wdStringView wdStringView::GetShrunk(wdUInt32 uiShrinkCharsFront, wdUInt32 uiShrinkCharsBack) const
{
  wdStringView tmp = *this;
  tmp.Shrink(uiShrinkCharsFront, uiShrinkCharsBack);
  return tmp;
}

void wdStringView::ChopAwayFirstCharacterUtf8()
{
  if (IsValid())
  {
    wdUnicodeUtils::MoveToNextUtf8(m_pStart, m_pEnd, 1);
  }
}

void wdStringView::ChopAwayFirstCharacterAscii()
{
  if (IsValid())
  {
    WD_ASSERT_DEBUG(wdUnicodeUtils::IsASCII(*m_pStart), "ChopAwayFirstCharacterAscii() was called on a non-ASCII character.");

    m_pStart += 1;
  }
}

bool wdStringView::TrimWordStart(wdStringView sWord1, wdStringView sWord2, wdStringView sWord3, wdStringView sWord4, wdStringView sWord5)
{
  /// \test TrimWordStart
  bool trimmed = false;

  while (true)
  {
    if (!sWord1.IsEmpty() && StartsWith_NoCase(sWord1))
    {
      Shrink(wdStringUtils::GetCharacterCount(sWord1.GetStartPointer(), sWord1.GetEndPointer()), 0);
      trimmed = true;
      continue;
    }

    if (!sWord2.IsEmpty() && StartsWith_NoCase(sWord2))
    {
      Shrink(wdStringUtils::GetCharacterCount(sWord2.GetStartPointer(), sWord2.GetEndPointer()), 0);
      trimmed = true;
      continue;
    }

    if (!sWord3.IsEmpty() && StartsWith_NoCase(sWord3))
    {
      Shrink(wdStringUtils::GetCharacterCount(sWord3.GetStartPointer(), sWord3.GetEndPointer()), 0);
      trimmed = true;
      continue;
    }

    if (!sWord4.IsEmpty() && StartsWith_NoCase(sWord4))
    {
      Shrink(wdStringUtils::GetCharacterCount(sWord4.GetStartPointer(), sWord4.GetEndPointer()), 0);
      trimmed = true;
      continue;
    }

    if (!sWord5.IsEmpty() && StartsWith_NoCase(sWord5))
    {
      Shrink(wdStringUtils::GetCharacterCount(sWord5.GetStartPointer(), sWord5.GetEndPointer()), 0);
      trimmed = true;
      continue;
    }

    return trimmed;
  }
}

bool wdStringView::TrimWordEnd(wdStringView sWord1, wdStringView sWord2, wdStringView sWord3, wdStringView sWord4, wdStringView sWord5)
{
  /// \test TrimWordEnd

  bool trimmed = false;

  while (true)
  {
    if (!sWord1.IsEmpty() && EndsWith_NoCase(sWord1))
    {
      Shrink(0, wdStringUtils::GetCharacterCount(sWord1.GetStartPointer(), sWord1.GetEndPointer()));
      trimmed = true;
      continue;
    }

    if (!sWord2.IsEmpty() && EndsWith_NoCase(sWord2))
    {
      Shrink(0, wdStringUtils::GetCharacterCount(sWord2.GetStartPointer(), sWord2.GetEndPointer()));
      trimmed = true;
      continue;
    }

    if (!sWord3.IsEmpty() && EndsWith_NoCase(sWord3))
    {
      Shrink(0, wdStringUtils::GetCharacterCount(sWord3.GetStartPointer(), sWord3.GetEndPointer()));
      trimmed = true;
      continue;
    }

    if (!sWord4.IsEmpty() && EndsWith_NoCase(sWord4))
    {
      Shrink(0, wdStringUtils::GetCharacterCount(sWord4.GetStartPointer(), sWord4.GetEndPointer()));
      trimmed = true;
      continue;
    }

    if (!sWord5.IsEmpty() && EndsWith_NoCase(sWord5))
    {
      Shrink(0, wdStringUtils::GetCharacterCount(sWord5.GetStartPointer(), sWord5.GetEndPointer()));
      trimmed = true;
      continue;
    }

    return trimmed;
  }
}

wdStringView::iterator wdStringView::GetIteratorFront() const
{
  return begin(*this);
}

wdStringView::reverse_iterator wdStringView::GetIteratorBack() const
{
  return rbegin(*this);
}

bool wdStringView::HasAnyExtension() const
{
  return wdPathUtils::HasAnyExtension(*this);
}

bool wdStringView::HasExtension(wdStringView sExtension) const
{
  return wdPathUtils::HasExtension(*this, sExtension);
}

wdStringView wdStringView::GetFileExtension() const
{
  return wdPathUtils::GetFileExtension(*this);
}

wdStringView wdStringView::GetFileName() const
{
  return wdPathUtils::GetFileName(*this);
}

wdStringView wdStringView::GetFileNameAndExtension() const
{
  return wdPathUtils::GetFileNameAndExtension(*this);
}

wdStringView wdStringView::GetFileDirectory() const
{
  return wdPathUtils::GetFileDirectory(*this);
}

bool wdStringView::IsAbsolutePath() const
{
  return wdPathUtils::IsAbsolutePath(*this);
}

bool wdStringView::IsRelativePath() const
{
  return wdPathUtils::IsRelativePath(*this);
}

bool wdStringView::IsRootedPath() const
{
  return wdPathUtils::IsRootedPath(*this);
}

wdStringView wdStringView::GetRootedPathRootName() const
{
  return wdPathUtils::GetRootedPathRootName(*this);
}

WD_STATICLINK_FILE(Foundation, Foundation_Strings_Implementation_StringView);

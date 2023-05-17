#pragma once

WD_ALWAYS_INLINE wdInt32 wdStringUtils::CompareChars(wdUInt32 uiCharacter1, wdUInt32 uiCharacter2)
{
  return (wdInt32)uiCharacter1 - (wdInt32)uiCharacter2;
}

inline wdInt32 wdStringUtils::CompareChars_NoCase(wdUInt32 uiCharacter1, wdUInt32 uiCharacter2)
{
  return (wdInt32)ToUpperChar(uiCharacter1) - (wdInt32)ToUpperChar(uiCharacter2);
}

inline wdInt32 wdStringUtils::CompareChars(const char* szUtf8Char1, const char* szUtf8Char2)
{
  return CompareChars(wdUnicodeUtils::ConvertUtf8ToUtf32(szUtf8Char1), wdUnicodeUtils::ConvertUtf8ToUtf32(szUtf8Char2));
}

inline wdInt32 wdStringUtils::CompareChars_NoCase(const char* szUtf8Char1, const char* szUtf8Char2)
{
  return CompareChars_NoCase(wdUnicodeUtils::ConvertUtf8ToUtf32(szUtf8Char1), wdUnicodeUtils::ConvertUtf8ToUtf32(szUtf8Char2));
}

template <typename T>
WD_ALWAYS_INLINE constexpr bool wdStringUtils::IsNullOrEmpty(const T* pString)
{
  return (pString == nullptr) || (pString[0] == '\0');
}

template <typename T>
WD_ALWAYS_INLINE bool wdStringUtils::IsNullOrEmpty(const T* pString, const T* pStringEnd)
{
  return (pString == nullptr) || (pString[0] == '\0') || pString == pStringEnd;
}

template <typename T>
WD_ALWAYS_INLINE void wdStringUtils::UpdateStringEnd(const T* pStringStart, const T*& ref_pStringEnd)
{
  if (ref_pStringEnd != wdUnicodeUtils::GetMaxStringEnd<T>())
    return;

  ref_pStringEnd = pStringStart + GetStringElementCount(pStringStart, wdUnicodeUtils::GetMaxStringEnd<T>());
}

template <typename T>
constexpr wdUInt32 wdStringUtils::GetStringElementCount(const T* pString)
{
  if (IsNullOrEmpty(pString))
    return 0;

  wdUInt32 uiCount = 0;
  while ((*pString != '\0'))
  {
    ++pString;
    ++uiCount;
  }

  return uiCount;
}

template <typename T>
wdUInt32 wdStringUtils::GetStringElementCount(const T* pString, const T* pStringEnd)
{
  if (IsNullOrEmpty(pString))
    return 0;

  if (pStringEnd != wdUnicodeUtils::GetMaxStringEnd<T>())
    return (wdUInt32)(pStringEnd - pString);

  wdUInt32 uiCount = 0;
  while ((*pString != '\0') && (pString < pStringEnd))
  {
    ++pString;
    ++uiCount;
  }

  return uiCount;
}

inline wdUInt32 wdStringUtils::GetCharacterCount(const char* szUtf8, const char* pStringEnd)
{
  if (IsNullOrEmpty(szUtf8))
    return 0;

  wdUInt32 uiCharacters = 0;

  while ((*szUtf8 != '\0') && (szUtf8 < pStringEnd))
  {
    // skip all the Utf8 continuation bytes
    if (!wdUnicodeUtils::IsUtf8ContinuationByte(*szUtf8))
      ++uiCharacters;

    ++szUtf8;
  }

  return uiCharacters;
}

inline void wdStringUtils::GetCharacterAndElementCount(
  const char* szUtf8, wdUInt32& ref_uiCharacterCount, wdUInt32& ref_uiElementCount, const char* pStringEnd)
{
  ref_uiCharacterCount = 0;
  ref_uiElementCount = 0;

  if (IsNullOrEmpty(szUtf8))
    return;

  while (szUtf8 < pStringEnd)
  {
    char uiByte = *szUtf8;
    if (uiByte == '\0')
    {
      break;
    }

    // skip all the Utf8 continuation bytes
    if (!wdUnicodeUtils::IsUtf8ContinuationByte(uiByte))
      ++ref_uiCharacterCount;

    ++szUtf8;
    ++ref_uiElementCount;
  }
}

WD_ALWAYS_INLINE bool wdStringUtils::IsEqual(const char* pString1, const char* pString2, const char* pString1End, const char* pString2End)
{
  return wdStringUtils::Compare(pString1, pString2, pString1End, pString2End) == 0;
}

WD_ALWAYS_INLINE bool wdStringUtils::IsEqualN(
  const char* pString1, const char* pString2, wdUInt32 uiCharsToCompare, const char* pString1End, const char* pString2End)
{
  return wdStringUtils::CompareN(pString1, pString2, uiCharsToCompare, pString1End, pString2End) == 0;
}

WD_ALWAYS_INLINE bool wdStringUtils::IsEqual_NoCase(const char* pString1, const char* pString2, const char* pString1End, const char* pString2End)
{
  return wdStringUtils::Compare_NoCase(pString1, pString2, pString1End, pString2End) == 0;
}

WD_ALWAYS_INLINE bool wdStringUtils::IsEqualN_NoCase(
  const char* pString1, const char* pString2, wdUInt32 uiCharsToCompare, const char* pString1End, const char* pString2End)
{
  return wdStringUtils::CompareN_NoCase(pString1, pString2, uiCharsToCompare, pString1End, pString2End) == 0;
}

WD_ALWAYS_INLINE bool wdStringUtils::IsDecimalDigit(wdUInt32 uiChar)
{
  return (uiChar >= '0' && uiChar <= '9');
}

WD_ALWAYS_INLINE bool wdStringUtils::IsHexDigit(wdUInt32 uiChar)
{
  return IsDecimalDigit(uiChar) || (uiChar >= 'A' && uiChar <= 'F') || (uiChar >= 'a' && uiChar <= 'f');
}

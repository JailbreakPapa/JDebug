#pragma once

NS_ALWAYS_INLINE nsInt32 nsStringUtils::CompareChars(nsUInt32 uiCharacter1, nsUInt32 uiCharacter2)
{
  return (nsInt32)uiCharacter1 - (nsInt32)uiCharacter2;
}

inline nsInt32 nsStringUtils::CompareChars_NoCase(nsUInt32 uiCharacter1, nsUInt32 uiCharacter2)
{
  return (nsInt32)ToUpperChar(uiCharacter1) - (nsInt32)ToUpperChar(uiCharacter2);
}

inline nsInt32 nsStringUtils::CompareChars(const char* szUtf8Char1, const char* szUtf8Char2)
{
  return CompareChars(nsUnicodeUtils::ConvertUtf8ToUtf32(szUtf8Char1), nsUnicodeUtils::ConvertUtf8ToUtf32(szUtf8Char2));
}

inline nsInt32 nsStringUtils::CompareChars_NoCase(const char* szUtf8Char1, const char* szUtf8Char2)
{
  return CompareChars_NoCase(nsUnicodeUtils::ConvertUtf8ToUtf32(szUtf8Char1), nsUnicodeUtils::ConvertUtf8ToUtf32(szUtf8Char2));
}

template <typename T>
NS_ALWAYS_INLINE constexpr bool nsStringUtils::IsNullOrEmpty(const T* pString)
{
  return (pString == nullptr) || (pString[0] == '\0');
}

template <typename T>
NS_ALWAYS_INLINE bool nsStringUtils::IsNullOrEmpty(const T* pString, const T* pStringEnd)
{
  return (pString == nullptr) || (pString[0] == '\0') || pString == pStringEnd;
}

template <typename T>
NS_ALWAYS_INLINE void nsStringUtils::UpdateStringEnd(const T* pStringStart, const T*& ref_pStringEnd)
{
  if (ref_pStringEnd != nsUnicodeUtils::GetMaxStringEnd<T>())
    return;

  ref_pStringEnd = pStringStart + GetStringElementCount(pStringStart, nsUnicodeUtils::GetMaxStringEnd<T>());
}

template <typename T>
constexpr nsUInt32 nsStringUtils::GetStringElementCount(const T* pString)
{
  // can't use strlen here as long as it's not constexpr (C++ 23)

  if (pString == nullptr)
    return 0;

  nsUInt32 uiCount = 0;
  while (*pString != '\0')
  {
    ++pString;
    ++uiCount;
  }

  return uiCount;
}

template <typename T>
nsUInt32 nsStringUtils::GetStringElementCount(const T* pString, const T* pStringEnd)
{
  if (IsNullOrEmpty(pString))
    return 0;

  if (pStringEnd != nsUnicodeUtils::GetMaxStringEnd<T>())
    return (nsUInt32)(pStringEnd - pString);

  nsUInt32 uiCount = 0;
  while ((*pString != '\0') && (pString < pStringEnd))
  {
    ++pString;
    ++uiCount;
  }

  return uiCount;
}

inline nsUInt32 nsStringUtils::GetCharacterCount(const char* szUtf8, const char* pStringEnd)
{
  if (IsNullOrEmpty(szUtf8))
    return 0;

  nsUInt32 uiCharacters = 0;

  while ((*szUtf8 != '\0') && (szUtf8 < pStringEnd))
  {
    // skip all the Utf8 continuation bytes
    if (!nsUnicodeUtils::IsUtf8ContinuationByte(*szUtf8))
      ++uiCharacters;

    ++szUtf8;
  }

  return uiCharacters;
}

inline void nsStringUtils::GetCharacterAndElementCount(const char* szUtf8, nsUInt32& ref_uiCharacterCount, nsUInt32& ref_uiElementCount, const char* pStringEnd)
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
    if (!nsUnicodeUtils::IsUtf8ContinuationByte(uiByte))
      ++ref_uiCharacterCount;

    ++szUtf8;
    ++ref_uiElementCount;
  }
}

NS_ALWAYS_INLINE bool nsStringUtils::IsEqual(const char* pString1, const char* pString2, const char* pString1End, const char* pString2End)
{
  return nsStringUtils::Compare(pString1, pString2, pString1End, pString2End) == 0;
}

NS_ALWAYS_INLINE bool nsStringUtils::IsEqualN(
  const char* pString1, const char* pString2, nsUInt32 uiCharsToCompare, const char* pString1End, const char* pString2End)
{
  return nsStringUtils::CompareN(pString1, pString2, uiCharsToCompare, pString1End, pString2End) == 0;
}

NS_ALWAYS_INLINE bool nsStringUtils::IsEqual_NoCase(const char* pString1, const char* pString2, const char* pString1End, const char* pString2End)
{
  return nsStringUtils::Compare_NoCase(pString1, pString2, pString1End, pString2End) == 0;
}

NS_ALWAYS_INLINE bool nsStringUtils::IsEqualN_NoCase(
  const char* pString1, const char* pString2, nsUInt32 uiCharsToCompare, const char* pString1End, const char* pString2End)
{
  return nsStringUtils::CompareN_NoCase(pString1, pString2, uiCharsToCompare, pString1End, pString2End) == 0;
}

NS_ALWAYS_INLINE bool nsStringUtils::IsDecimalDigit(nsUInt32 uiChar)
{
  return (uiChar >= '0' && uiChar <= '9');
}

NS_ALWAYS_INLINE bool nsStringUtils::IsHexDigit(nsUInt32 uiChar)
{
  return IsDecimalDigit(uiChar) || (uiChar >= 'A' && uiChar <= 'F') || (uiChar >= 'a' && uiChar <= 'f');
}

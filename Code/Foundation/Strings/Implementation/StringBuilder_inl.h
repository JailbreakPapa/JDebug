#pragma once

#include <Foundation/Strings/StringConversion.h>

inline wdStringBuilder::wdStringBuilder(wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  m_uiCharacterCount = 0;
  AppendTerminator();
}

inline wdStringBuilder::wdStringBuilder(const wdStringBuilder& rhs)
  : m_Data(rhs.GetAllocator())
{
  m_uiCharacterCount = 0;
  AppendTerminator();

  *this = rhs;
}

inline wdStringBuilder::wdStringBuilder(wdStringBuilder&& rhs) noexcept
  : m_Data(rhs.GetAllocator())
{
  m_uiCharacterCount = 0;
  AppendTerminator();

  *this = std::move(rhs);
}

inline wdStringBuilder::wdStringBuilder(const char* szUTF8, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  m_uiCharacterCount = 0;
  AppendTerminator();

  *this = szUTF8;
}

inline wdStringBuilder::wdStringBuilder(const wchar_t* pWChar, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  m_uiCharacterCount = 0;
  AppendTerminator();

  *this = pWChar;
}

inline wdStringBuilder::wdStringBuilder(wdStringView rhs, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  m_uiCharacterCount = 0;
  AppendTerminator();

  *this = rhs;
}

WD_ALWAYS_INLINE wdAllocatorBase* wdStringBuilder::GetAllocator() const
{
  return m_Data.GetAllocator();
}

WD_ALWAYS_INLINE void wdStringBuilder::operator=(const char* szUTF8)
{
  Set(szUTF8);
}

WD_FORCE_INLINE void wdStringBuilder::operator=(const wchar_t* pWChar)
{
  // fine to do this, szWChar can never come from the stringbuilder's own data array
  Clear();
  Append(pWChar);
}

WD_ALWAYS_INLINE void wdStringBuilder::operator=(const wdStringBuilder& rhs)
{
  m_uiCharacterCount = rhs.m_uiCharacterCount;
  m_Data = rhs.m_Data;
}

WD_ALWAYS_INLINE void wdStringBuilder::operator=(wdStringBuilder&& rhs) noexcept
{
  m_uiCharacterCount = rhs.m_uiCharacterCount;
  m_Data = std::move(rhs.m_Data);
}

WD_ALWAYS_INLINE wdUInt32 wdStringBuilder::GetElementCount() const
{
  return m_Data.GetCount() - 1; // exclude the '\0' terminator
}

WD_ALWAYS_INLINE wdUInt32 wdStringBuilder::GetCharacterCount() const
{
  return m_uiCharacterCount;
}

WD_FORCE_INLINE void wdStringBuilder::Clear()
{
  m_uiCharacterCount = 0;
  m_Data.SetCountUninitialized(1);
  m_Data[0] = '\0';
}

inline void wdStringBuilder::Append(wdUInt32 uiChar)
{
  char szChar[6] = {0, 0, 0, 0, 0, 0};
  char* pChar = &szChar[0];

  wdUnicodeUtils::EncodeUtf32ToUtf8(uiChar, pChar);
  wdUInt32 uiCharLen = (wdUInt32)(pChar - szChar);
  wdUInt32 uiOldCount = m_Data.GetCount();
  m_Data.SetCountUninitialized(uiOldCount + uiCharLen);
  uiOldCount--;
  for (wdUInt32 i = 0; i < uiCharLen; i++)
  {
    m_Data[uiOldCount + i] = szChar[i];
  }
  m_Data[uiOldCount + uiCharLen] = '\0';
  ++m_uiCharacterCount;
}

inline void wdStringBuilder::Prepend(wdUInt32 uiChar)
{
  char szChar[6] = {0, 0, 0, 0, 0, 0};
  char* pChar = &szChar[0];

  wdUnicodeUtils::EncodeUtf32ToUtf8(uiChar, pChar);
  Prepend(szChar);
}

inline void wdStringBuilder::Append(
  const wchar_t* pData1, const wchar_t* pData2, const wchar_t* pData3, const wchar_t* pData4, const wchar_t* pData5, const wchar_t* pData6)
{
  // this is a bit heavy on the stack size (6KB)
  // but it is really only a convenience function, as one could always just use the char* Append function and convert explicitly
  wdStringUtf8 s1(pData1, m_Data.GetAllocator());
  wdStringUtf8 s2(pData2, m_Data.GetAllocator());
  wdStringUtf8 s3(pData3, m_Data.GetAllocator());
  wdStringUtf8 s4(pData4, m_Data.GetAllocator());
  wdStringUtf8 s5(pData5, m_Data.GetAllocator());
  wdStringUtf8 s6(pData6, m_Data.GetAllocator());

  Append(s1.GetView(), s2.GetView(), s3.GetView(), s4.GetView(), s5.GetView(), s6.GetView());
}

inline void wdStringBuilder::Prepend(
  const wchar_t* pData1, const wchar_t* pData2, const wchar_t* pData3, const wchar_t* pData4, const wchar_t* pData5, const wchar_t* pData6)
{
  // this is a bit heavy on the stack size (6KB)
  // but it is really only a convenience function, as one could always just use the char* Append function and convert explicitly
  wdStringUtf8 s1(pData1, m_Data.GetAllocator());
  wdStringUtf8 s2(pData2, m_Data.GetAllocator());
  wdStringUtf8 s3(pData3, m_Data.GetAllocator());
  wdStringUtf8 s4(pData4, m_Data.GetAllocator());
  wdStringUtf8 s5(pData5, m_Data.GetAllocator());
  wdStringUtf8 s6(pData6, m_Data.GetAllocator());

  Prepend(s1.GetView(), s2.GetView(), s3.GetView(), s4.GetView(), s5.GetView(), s6.GetView());
}

WD_ALWAYS_INLINE const char* wdStringBuilder::GetData() const
{
  WD_ASSERT_DEBUG(!m_Data.IsEmpty(), "wdStringBuilder has been corrupted, the array can never be empty.");

  return &m_Data[0];
}

inline void wdStringBuilder::AppendTerminator()
{
  // make sure the string terminates with a zero.
  if (m_Data.IsEmpty() || (m_Data.PeekBack() != '\0'))
    m_Data.PushBack('\0');
}

inline void wdStringBuilder::ToUpper()
{
  const wdUInt32 uiNewStringLength = wdStringUtils::ToUpperString(&m_Data[0]);

  // the array stores the number of bytes, so set the count to the actually used number of bytes
  m_Data.SetCountUninitialized(uiNewStringLength + 1);
}

inline void wdStringBuilder::ToLower()
{
  const wdUInt32 uiNewStringLength = wdStringUtils::ToLowerString(&m_Data[0]);

  // the array stores the number of bytes, so set the count to the actually used number of bytes
  m_Data.SetCountUninitialized(uiNewStringLength + 1);
}

inline void wdStringBuilder::ChangeCharacter(iterator& ref_it, wdUInt32 uiCharacter)
{
  WD_ASSERT_DEV(ref_it.IsValid(), "The given character iterator does not point to a valid character.");
  WD_ASSERT_DEV(ref_it.GetData() >= GetData() && ref_it.GetData() < GetData() + GetElementCount(),
    "The given character iterator does not point into this string. It was either created from another string, or this string "
    "has been reallocated in the mean time.");

  // this is only an optimization for pure ASCII strings
  // without it, the code below would still work
  if (wdUnicodeUtils::IsASCII(*ref_it) && wdUnicodeUtils::IsASCII(uiCharacter))
  {
    char* pPos = const_cast<char*>(ref_it.GetData()); // yes, I know...
    *pPos = uiCharacter & 0xFF;
    return;
  }

  ChangeCharacterNonASCII(ref_it, uiCharacter);
}

WD_ALWAYS_INLINE bool wdStringBuilder::IsPureASCII() const
{
  return m_uiCharacterCount + 1 == m_Data.GetCount();
}

WD_ALWAYS_INLINE void wdStringBuilder::Reserve(wdUInt32 uiNumElements)
{
  m_Data.Reserve(uiNumElements);
}

WD_ALWAYS_INLINE void wdStringBuilder::Insert(const char* szInsertAtPos, wdStringView sTextToInsert)
{
  ReplaceSubString(szInsertAtPos, szInsertAtPos, sTextToInsert);
}

WD_ALWAYS_INLINE void wdStringBuilder::Remove(const char* szRemoveFromPos, const char* szRemoveToPos)
{
  ReplaceSubString(szRemoveFromPos, szRemoveToPos, wdStringView());
}

template <typename Container>
bool wdUnicodeUtils::RepairNonUtf8Text(const char* pStartData, const char* pEndData, Container& out_result)
{
  if (wdUnicodeUtils::IsValidUtf8(pStartData, pEndData))
  {
    out_result = wdStringView(pStartData, pEndData);
    return false;
  }

  out_result.Clear();

  wdHybridArray<char, 1024> fixedText;
  wdUnicodeUtils::UtfInserter<char, decltype(fixedText)> inserter(&fixedText);

  while (pStartData < pEndData)
  {
    const wdUInt32 uiChar = wdUnicodeUtils::DecodeUtf8ToUtf32(pStartData);
    wdUnicodeUtils::EncodeUtf32ToUtf8(uiChar, inserter);
  }

  WD_ASSERT_DEV(wdUnicodeUtils::IsValidUtf8(fixedText.GetData(), fixedText.GetData() + fixedText.GetCount()), "Repaired text is still not a valid Utf8 string.");

  out_result = wdStringView(fixedText.GetData(), fixedText.GetCount());
  return true;
}

#include <Foundation/Strings/Implementation/AllStrings_inl.h>

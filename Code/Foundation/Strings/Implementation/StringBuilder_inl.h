#pragma once

#include <Foundation/Strings/StringConversion.h>

inline nsStringBuilder::nsStringBuilder(nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  AppendTerminator();
}

inline nsStringBuilder::nsStringBuilder(const nsStringBuilder& rhs)
  : m_Data(rhs.GetAllocator())
{
  AppendTerminator();

  *this = rhs;
}

inline nsStringBuilder::nsStringBuilder(nsStringBuilder&& rhs) noexcept
  : m_Data(rhs.GetAllocator())
{
  AppendTerminator();

  *this = std::move(rhs);
}

inline nsStringBuilder::nsStringBuilder(const char* szUTF8, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  AppendTerminator();

  *this = szUTF8;
}

inline nsStringBuilder::nsStringBuilder(const wchar_t* pWChar, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  AppendTerminator();

  *this = pWChar;
}

inline nsStringBuilder::nsStringBuilder(nsStringView rhs, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  AppendTerminator();

  *this = rhs;
}

NS_ALWAYS_INLINE nsAllocator* nsStringBuilder::GetAllocator() const
{
  return m_Data.GetAllocator();
}

NS_ALWAYS_INLINE void nsStringBuilder::operator=(const char* szUTF8)
{
  Set(szUTF8);
}

NS_FORCE_INLINE void nsStringBuilder::operator=(const wchar_t* pWChar)
{
  // fine to do this, szWChar can never come from the stringbuilder's own data array
  Clear();
  Append(pWChar);
}

NS_ALWAYS_INLINE void nsStringBuilder::operator=(const nsStringBuilder& rhs)
{
  m_Data = rhs.m_Data;
}

NS_ALWAYS_INLINE void nsStringBuilder::operator=(nsStringBuilder&& rhs) noexcept
{
  m_Data = std::move(rhs.m_Data);
}

NS_ALWAYS_INLINE nsUInt32 nsStringBuilder::GetElementCount() const
{
  return m_Data.GetCount() - 1; // exclude the '\0' terminator
}

NS_ALWAYS_INLINE nsUInt32 nsStringBuilder::GetCharacterCount() const
{
  return nsStringUtils::GetCharacterCount(m_Data.GetData());
}

NS_FORCE_INLINE void nsStringBuilder::Clear()
{
  m_Data.SetCountUninitialized(1);
  m_Data[0] = '\0';
}

inline void nsStringBuilder::Append(nsUInt32 uiChar)
{
  char szChar[6] = {0, 0, 0, 0, 0, 0};
  char* pChar = &szChar[0];

  nsUnicodeUtils::EncodeUtf32ToUtf8(uiChar, pChar);
  nsUInt32 uiCharLen = (nsUInt32)(pChar - szChar);
  nsUInt32 uiOldCount = m_Data.GetCount();
  m_Data.SetCountUninitialized(uiOldCount + uiCharLen);
  uiOldCount--;
  for (nsUInt32 i = 0; i < uiCharLen; i++)
  {
    m_Data[uiOldCount + i] = szChar[i];
  }
  m_Data[uiOldCount + uiCharLen] = '\0';
}

inline void nsStringBuilder::Prepend(nsUInt32 uiChar)
{
  char szChar[6] = {0, 0, 0, 0, 0, 0};
  char* pChar = &szChar[0];

  nsUnicodeUtils::EncodeUtf32ToUtf8(uiChar, pChar);
  Prepend(szChar);
}

inline void nsStringBuilder::Append(const wchar_t* pData1, const wchar_t* pData2, const wchar_t* pData3, const wchar_t* pData4, const wchar_t* pData5, const wchar_t* pData6)
{
  // this is a bit heavy on the stack size (6KB)
  // but it is really only a convenience function, as one could always just use the char* Append function and convert explicitly
  nsStringUtf8 s1(pData1, m_Data.GetAllocator());
  nsStringUtf8 s2(pData2, m_Data.GetAllocator());
  nsStringUtf8 s3(pData3, m_Data.GetAllocator());
  nsStringUtf8 s4(pData4, m_Data.GetAllocator());
  nsStringUtf8 s5(pData5, m_Data.GetAllocator());
  nsStringUtf8 s6(pData6, m_Data.GetAllocator());

  Append(s1.GetView(), s2.GetView(), s3.GetView(), s4.GetView(), s5.GetView(), s6.GetView());
}

inline void nsStringBuilder::Prepend(const wchar_t* pData1, const wchar_t* pData2, const wchar_t* pData3, const wchar_t* pData4, const wchar_t* pData5, const wchar_t* pData6)
{
  // this is a bit heavy on the stack size (6KB)
  // but it is really only a convenience function, as one could always just use the char* Append function and convert explicitly
  nsStringUtf8 s1(pData1, m_Data.GetAllocator());
  nsStringUtf8 s2(pData2, m_Data.GetAllocator());
  nsStringUtf8 s3(pData3, m_Data.GetAllocator());
  nsStringUtf8 s4(pData4, m_Data.GetAllocator());
  nsStringUtf8 s5(pData5, m_Data.GetAllocator());
  nsStringUtf8 s6(pData6, m_Data.GetAllocator());

  Prepend(s1.GetView(), s2.GetView(), s3.GetView(), s4.GetView(), s5.GetView(), s6.GetView());
}

NS_ALWAYS_INLINE const char* nsStringBuilder::GetData() const
{
  NS_ASSERT_DEBUG(!m_Data.IsEmpty(), "nsStringBuilder has been corrupted, the array can never be empty.");

  return &m_Data[0];
}

inline void nsStringBuilder::AppendTerminator()
{
  // make sure the string terminates with a zero.
  if (m_Data.IsEmpty() || (m_Data.PeekBack() != '\0'))
    m_Data.PushBack('\0');
}

inline void nsStringBuilder::ToUpper()
{
  const nsUInt32 uiNewStringLength = nsStringUtils::ToUpperString(&m_Data[0]);

  // the array stores the number of bytes, so set the count to the actually used number of bytes
  m_Data.SetCountUninitialized(uiNewStringLength + 1);
}

inline void nsStringBuilder::ToLower()
{
  const nsUInt32 uiNewStringLength = nsStringUtils::ToLowerString(&m_Data[0]);

  // the array stores the number of bytes, so set the count to the actually used number of bytes
  m_Data.SetCountUninitialized(uiNewStringLength + 1);
}

inline void nsStringBuilder::ChangeCharacter(iterator& ref_it, nsUInt32 uiCharacter)
{
  NS_ASSERT_DEV(ref_it.IsValid(), "The given character iterator does not point to a valid character.");
  NS_ASSERT_DEV(ref_it.GetData() >= GetData() && ref_it.GetData() < GetData() + GetElementCount(),
    "The given character iterator does not point into this string. It was either created from another string, or this string "
    "has been reallocated in the mean time.");

  // this is only an optimization for pure ASCII strings
  // without it, the code below would still work
  if (nsUnicodeUtils::IsASCII(*ref_it) && nsUnicodeUtils::IsASCII(uiCharacter))
  {
    char* pPos = const_cast<char*>(ref_it.GetData()); // yes, I know...
    *pPos = uiCharacter & 0xFF;
    return;
  }

  ChangeCharacterNonASCII(ref_it, uiCharacter);
}

NS_ALWAYS_INLINE void nsStringBuilder::Reserve(nsUInt32 uiNumElements)
{
  m_Data.Reserve(uiNumElements);
}

NS_ALWAYS_INLINE void nsStringBuilder::Insert(const char* szInsertAtPos, nsStringView sTextToInsert)
{
  ReplaceSubString(szInsertAtPos, szInsertAtPos, sTextToInsert);
}

NS_ALWAYS_INLINE void nsStringBuilder::Remove(const char* szRemoveFromPos, const char* szRemoveToPos)
{
  ReplaceSubString(szRemoveFromPos, szRemoveToPos, nsStringView());
}

template <typename Container>
bool nsUnicodeUtils::RepairNonUtf8Text(const char* pStartData, const char* pEndData, Container& out_result)
{
  if (nsUnicodeUtils::IsValidUtf8(pStartData, pEndData))
  {
    out_result = nsStringView(pStartData, pEndData);
    return false;
  }

  out_result.Clear();

  nsHybridArray<char, 1024> fixedText;
  nsUnicodeUtils::UtfInserter<char, decltype(fixedText)> inserter(&fixedText);

  while (pStartData < pEndData)
  {
    const nsUInt32 uiChar = nsUnicodeUtils::DecodeUtf8ToUtf32(pStartData);
    nsUnicodeUtils::EncodeUtf32ToUtf8(uiChar, inserter);
  }

  NS_ASSERT_DEV(nsUnicodeUtils::IsValidUtf8(fixedText.GetData(), fixedText.GetData() + fixedText.GetCount()), "Repaired text is still not a valid Utf8 string.");

  out_result = nsStringView(fixedText.GetData(), fixedText.GetCount());
  return true;
}

#include <Foundation/Strings/Implementation/AllStrings_inl.h>

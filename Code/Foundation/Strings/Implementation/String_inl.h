#pragma once

template <wdUInt16 Size>
wdHybridStringBase<Size>::wdHybridStringBase(wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  Clear();
}

template <wdUInt16 Size>
wdHybridStringBase<Size>::wdHybridStringBase(const wdHybridStringBase& rhs, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  *this = rhs;
}

template <wdUInt16 Size>
wdHybridStringBase<Size>::wdHybridStringBase(wdHybridStringBase&& rhs, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  operator=(std::move(rhs));
}

template <wdUInt16 Size>
wdHybridStringBase<Size>::wdHybridStringBase(const char* rhs, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  *this = rhs;
}

template <wdUInt16 Size>
wdHybridStringBase<Size>::wdHybridStringBase(const wchar_t* rhs, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  *this = rhs;
}

template <wdUInt16 Size>
wdHybridStringBase<Size>::wdHybridStringBase(const wdStringView& rhs, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  *this = rhs;
}

template <wdUInt16 Size>
wdHybridStringBase<Size>::~wdHybridStringBase() = default;

template <wdUInt16 Size>
void wdHybridStringBase<Size>::Clear()
{
  m_Data.SetCountUninitialized(1);
  m_Data[0] = '\0';
  m_uiCharacterCount = 0;
}

template <wdUInt16 Size>
WD_ALWAYS_INLINE const char* wdHybridStringBase<Size>::GetData() const
{
  WD_ASSERT_DEBUG(!m_Data.IsEmpty(), "wdHybridString has been corrupted, the array can never be empty. This can happen when you access a "
                                     "string that was previously std::move'd into another string.");

  return &m_Data[0];
}

template <wdUInt16 Size>
WD_ALWAYS_INLINE wdUInt32 wdHybridStringBase<Size>::GetElementCount() const
{
  return m_Data.GetCount() - 1;
}

template <wdUInt16 Size>
WD_ALWAYS_INLINE wdUInt32 wdHybridStringBase<Size>::GetCharacterCount() const
{
  return m_uiCharacterCount;
}

template <wdUInt16 Size>
void wdHybridStringBase<Size>::operator=(const char* szString)
{
  wdUInt32 uiElementCount = 0;
  wdStringUtils::GetCharacterAndElementCount(szString, m_uiCharacterCount, uiElementCount);

  if (szString + uiElementCount < m_Data.GetData() || szString >= m_Data.GetData() + m_Data.GetCount())
  {
    // source string is outside our own memory, so no overlapped copy
  }
  else
  {
    // source string overlaps with our own memory -> we can't increase the size of our memory, as that might invalidate the source data
    WD_ASSERT_DEBUG(uiElementCount < m_Data.GetCount(), "Invalid copy of overlapping string data.");
  }

  m_Data.SetCountUninitialized(uiElementCount + 1);
  wdStringUtils::Copy(&m_Data[0], uiElementCount + 1, szString);
}

template <wdUInt16 Size>
void wdHybridStringBase<Size>::operator=(const wdHybridStringBase& rhs)
{
  if (this == &rhs)
    return;

  m_uiCharacterCount = rhs.m_uiCharacterCount;
  m_Data = rhs.m_Data;
}

template <wdUInt16 Size>
void wdHybridStringBase<Size>::operator=(wdHybridStringBase&& rhs)
{
  if (this == &rhs)
    return;

  m_uiCharacterCount = rhs.m_uiCharacterCount;
  m_Data = std::move(rhs.m_Data);
}

template <wdUInt16 Size>
void wdHybridStringBase<Size>::operator=(const wchar_t* szString)
{
  wdStringUtf8 sConversion(szString, m_Data.GetAllocator());
  *this = sConversion.GetData();
}

template <wdUInt16 Size>
void wdHybridStringBase<Size>::operator=(const wdStringView& rhs)
{
  WD_ASSERT_DEBUG(rhs.GetStartPointer() < m_Data.GetData() || rhs.GetStartPointer() >= m_Data.GetData() + m_Data.GetCount(),
    "Can't assign string a value that points to ourself!");

  m_Data.SetCountUninitialized(rhs.GetElementCount() + 1);
  wdStringUtils::Copy(&m_Data[0], m_Data.GetCount(), rhs.GetStartPointer(), rhs.GetEndPointer());
  m_uiCharacterCount = wdStringUtils::GetCharacterCount(GetData());
}

template <wdUInt16 Size>
wdStringView wdHybridStringBase<Size>::GetSubString(wdUInt32 uiFirstCharacter, wdUInt32 uiNumCharacters) const
{
  WD_ASSERT_DEV(uiFirstCharacter + uiNumCharacters <= m_uiCharacterCount,
    "The string only has {0} characters, cannot get a sub-string up to character {1}.", m_uiCharacterCount, uiFirstCharacter + uiNumCharacters);

  const char* szStart = GetData();
  wdUnicodeUtils::MoveToNextUtf8(szStart, uiFirstCharacter);

  const char* szEnd = szStart;
  wdUnicodeUtils::MoveToNextUtf8(szEnd, uiNumCharacters);

  return wdStringView(szStart, szEnd);
}

template <wdUInt16 Size>
wdStringView wdHybridStringBase<Size>::GetFirst(wdUInt32 uiNumCharacters) const
{
  return GetSubString(0, uiNumCharacters);
}

template <wdUInt16 Size>
wdStringView wdHybridStringBase<Size>::GetLast(wdUInt32 uiNumCharacters) const
{
  WD_ASSERT_DEV(uiNumCharacters < m_uiCharacterCount, "The string only contains {0} characters, cannot return the last {1} characters.",
    m_uiCharacterCount, uiNumCharacters);
  return GetSubString(m_uiCharacterCount - uiNumCharacters, uiNumCharacters);
}


template <wdUInt16 Size, typename A>
WD_ALWAYS_INLINE wdHybridString<Size, A>::wdHybridString()
  : wdHybridStringBase<Size>(A::GetAllocator())
{
}

template <wdUInt16 Size, typename A>
WD_ALWAYS_INLINE wdHybridString<Size, A>::wdHybridString(wdAllocatorBase* pAllocator)
  : wdHybridStringBase<Size>(pAllocator)
{
}

template <wdUInt16 Size, typename A>
WD_ALWAYS_INLINE wdHybridString<Size, A>::wdHybridString(const wdHybridString<Size, A>& other)
  : wdHybridStringBase<Size>(other, A::GetAllocator())
{
}

template <wdUInt16 Size, typename A>
WD_ALWAYS_INLINE wdHybridString<Size, A>::wdHybridString(const wdHybridStringBase<Size>& other)
  : wdHybridStringBase<Size>(other, A::GetAllocator())
{
}

template <wdUInt16 Size, typename A>
WD_ALWAYS_INLINE wdHybridString<Size, A>::wdHybridString(wdHybridString<Size, A>&& other)
  : wdHybridStringBase<Size>(std::move(other), A::GetAllocator())
{
}

template <wdUInt16 Size, typename A>
WD_ALWAYS_INLINE wdHybridString<Size, A>::wdHybridString(wdHybridStringBase<Size>&& other)
  : wdHybridStringBase<Size>(std::move(other), A::GetAllocator())
{
}

template <wdUInt16 Size, typename A>
WD_ALWAYS_INLINE wdHybridString<Size, A>::wdHybridString(const char* rhs)
  : wdHybridStringBase<Size>(rhs, A::GetAllocator())
{
}

template <wdUInt16 Size, typename A>
WD_ALWAYS_INLINE wdHybridString<Size, A>::wdHybridString(const wchar_t* rhs)
  : wdHybridStringBase<Size>(rhs, A::GetAllocator())
{
}

template <wdUInt16 Size, typename A>
WD_ALWAYS_INLINE wdHybridString<Size, A>::wdHybridString(const wdStringView& rhs)
  : wdHybridStringBase<Size>(rhs, A::GetAllocator())
{
}

template <wdUInt16 Size, typename A>
WD_ALWAYS_INLINE void wdHybridString<Size, A>::operator=(const wdHybridString<Size, A>& rhs)
{
  wdHybridStringBase<Size>::operator=(rhs);
}

template <wdUInt16 Size, typename A>
WD_ALWAYS_INLINE void wdHybridString<Size, A>::operator=(const wdHybridStringBase<Size>& rhs)
{
  wdHybridStringBase<Size>::operator=(rhs);
}

template <wdUInt16 Size, typename A>
WD_ALWAYS_INLINE void wdHybridString<Size, A>::operator=(wdHybridString<Size, A>&& rhs)
{
  wdHybridStringBase<Size>::operator=(std::move(rhs));
}

template <wdUInt16 Size, typename A>
WD_ALWAYS_INLINE void wdHybridString<Size, A>::operator=(wdHybridStringBase<Size>&& rhs)
{
  wdHybridStringBase<Size>::operator=(std::move(rhs));
}

template <wdUInt16 Size, typename A>
WD_ALWAYS_INLINE void wdHybridString<Size, A>::operator=(const char* rhs)
{
  wdHybridStringBase<Size>::operator=(rhs);
}

template <wdUInt16 Size, typename A>
WD_ALWAYS_INLINE void wdHybridString<Size, A>::operator=(const wchar_t* rhs)
{
  wdHybridStringBase<Size>::operator=(rhs);
}

template <wdUInt16 Size, typename A>
WD_ALWAYS_INLINE void wdHybridString<Size, A>::operator=(const wdStringView& rhs)
{
  wdHybridStringBase<Size>::operator=(rhs);
}

#include <Foundation/Strings/Implementation/AllStrings_inl.h>

#pragma once

template <nsUInt16 Size>
nsHybridStringBase<Size>::nsHybridStringBase(nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  Clear();
}

template <nsUInt16 Size>
nsHybridStringBase<Size>::nsHybridStringBase(const nsHybridStringBase& rhs, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  *this = rhs;
}

template <nsUInt16 Size>
nsHybridStringBase<Size>::nsHybridStringBase(nsHybridStringBase&& rhs, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  operator=(std::move(rhs));
}

template <nsUInt16 Size>
nsHybridStringBase<Size>::nsHybridStringBase(const char* rhs, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  *this = rhs;
}

template <nsUInt16 Size>
nsHybridStringBase<Size>::nsHybridStringBase(const wchar_t* rhs, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  *this = rhs;
}

template <nsUInt16 Size>
nsHybridStringBase<Size>::nsHybridStringBase(const nsStringView& rhs, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  *this = rhs;
}

template <nsUInt16 Size>
nsHybridStringBase<Size>::~nsHybridStringBase() = default;

template <nsUInt16 Size>
void nsHybridStringBase<Size>::Clear()
{
  m_Data.SetCountUninitialized(1);
  m_Data[0] = '\0';
}

template <nsUInt16 Size>
NS_ALWAYS_INLINE const char* nsHybridStringBase<Size>::GetData() const
{
  NS_ASSERT_DEBUG(!m_Data.IsEmpty(), "nsHybridString has been corrupted, the array can never be empty. This can happen when you access a "
                                     "string that was previously std::move'd into another string.");

  return &m_Data[0];
}

template <nsUInt16 Size>
NS_ALWAYS_INLINE nsUInt32 nsHybridStringBase<Size>::GetElementCount() const
{
  return m_Data.GetCount() - 1;
}

template <nsUInt16 Size>
NS_ALWAYS_INLINE nsUInt32 nsHybridStringBase<Size>::GetCharacterCount() const
{
  return nsStringUtils::GetCharacterCount(GetData());
}

template <nsUInt16 Size>
void nsHybridStringBase<Size>::operator=(const char* szString)
{
  nsUInt32 uiElementCount = nsStringUtils::GetStringElementCount(szString);

  if (szString + uiElementCount < m_Data.GetData() || szString >= m_Data.GetData() + m_Data.GetCount())
  {
    // source string is outside our own memory, so no overlapped copy
  }
  else
  {
    // source string overlaps with our own memory -> we can't increase the size of our memory, as that might invalidate the source data
    NS_ASSERT_DEBUG(uiElementCount < m_Data.GetCount(), "Invalid copy of overlapping string data.");
  }

  m_Data.SetCountUninitialized(uiElementCount + 1);
  nsStringUtils::Copy(&m_Data[0], uiElementCount + 1, szString);
}

template <nsUInt16 Size>
void nsHybridStringBase<Size>::operator=(const nsHybridStringBase& rhs)
{
  if (this == &rhs)
    return;

  m_Data = rhs.m_Data;
}

template <nsUInt16 Size>
void nsHybridStringBase<Size>::operator=(nsHybridStringBase&& rhs)
{
  if (this == &rhs)
    return;

  m_Data = std::move(rhs.m_Data);
}

template <nsUInt16 Size>
void nsHybridStringBase<Size>::operator=(const wchar_t* szString)
{
  nsStringUtf8 sConversion(szString, m_Data.GetAllocator());
  *this = sConversion.GetData();
}

template <nsUInt16 Size>
void nsHybridStringBase<Size>::operator=(const nsStringView& rhs)
{
  NS_ASSERT_DEBUG(rhs.GetStartPointer() < m_Data.GetData() || rhs.GetStartPointer() >= m_Data.GetData() + m_Data.GetCount(),
    "Can't assign string a value that points to ourself!");

  m_Data.SetCountUninitialized(rhs.GetElementCount() + 1);
  nsStringUtils::Copy(&m_Data[0], m_Data.GetCount(), rhs.GetStartPointer(), rhs.GetEndPointer());
}

template <nsUInt16 Size>
nsStringView nsHybridStringBase<Size>::GetSubString(nsUInt32 uiFirstCharacter, nsUInt32 uiNumCharacters) const
{
  const char* szStart = GetData();
  if (nsUnicodeUtils::MoveToNextUtf8(szStart, uiFirstCharacter).Failed())
    return {};                                                           // szStart was moved too far, the result is just an empty string

  const char* szEnd = szStart;
  nsUnicodeUtils::MoveToNextUtf8(szEnd, uiNumCharacters).IgnoreResult(); // if it fails, szEnd just points to the end of this string

  return nsStringView(szStart, szEnd);
}

template <nsUInt16 Size>
nsStringView nsHybridStringBase<Size>::GetFirst(nsUInt32 uiNumCharacters) const
{
  return GetSubString(0, uiNumCharacters);
}

template <nsUInt16 Size>
nsStringView nsHybridStringBase<Size>::GetLast(nsUInt32 uiNumCharacters) const
{
  const nsUInt32 uiMaxCharacterCount = GetCharacterCount();
  NS_ASSERT_DEV(uiNumCharacters < uiMaxCharacterCount, "The string only contains {0} characters, cannot return the last {1} characters.",
    uiMaxCharacterCount, uiNumCharacters);
  return GetSubString(uiMaxCharacterCount - uiNumCharacters, uiNumCharacters);
}


template <nsUInt16 Size, typename A>
NS_ALWAYS_INLINE nsHybridString<Size, A>::nsHybridString()
  : nsHybridStringBase<Size>(A::GetAllocator())
{
}

template <nsUInt16 Size, typename A>
NS_ALWAYS_INLINE nsHybridString<Size, A>::nsHybridString(nsAllocator* pAllocator)
  : nsHybridStringBase<Size>(pAllocator)
{
}

template <nsUInt16 Size, typename A>
NS_ALWAYS_INLINE nsHybridString<Size, A>::nsHybridString(const nsHybridString<Size, A>& other)
  : nsHybridStringBase<Size>(other, A::GetAllocator())
{
}

template <nsUInt16 Size, typename A>
NS_ALWAYS_INLINE nsHybridString<Size, A>::nsHybridString(const nsHybridStringBase<Size>& other)
  : nsHybridStringBase<Size>(other, A::GetAllocator())
{
}

template <nsUInt16 Size, typename A>
NS_ALWAYS_INLINE nsHybridString<Size, A>::nsHybridString(nsHybridString<Size, A>&& other)
  : nsHybridStringBase<Size>(std::move(other), A::GetAllocator())
{
}

template <nsUInt16 Size, typename A>
NS_ALWAYS_INLINE nsHybridString<Size, A>::nsHybridString(nsHybridStringBase<Size>&& other)
  : nsHybridStringBase<Size>(std::move(other), A::GetAllocator())
{
}

template <nsUInt16 Size, typename A>
NS_ALWAYS_INLINE nsHybridString<Size, A>::nsHybridString(const char* rhs)
  : nsHybridStringBase<Size>(rhs, A::GetAllocator())
{
}

template <nsUInt16 Size, typename A>
NS_ALWAYS_INLINE nsHybridString<Size, A>::nsHybridString(const wchar_t* rhs)
  : nsHybridStringBase<Size>(rhs, A::GetAllocator())
{
}

template <nsUInt16 Size, typename A>
NS_ALWAYS_INLINE nsHybridString<Size, A>::nsHybridString(const nsStringView& rhs)
  : nsHybridStringBase<Size>(rhs, A::GetAllocator())
{
}

template <nsUInt16 Size, typename A>
NS_ALWAYS_INLINE void nsHybridString<Size, A>::operator=(const nsHybridString<Size, A>& rhs)
{
  nsHybridStringBase<Size>::operator=(rhs);
}

template <nsUInt16 Size, typename A>
NS_ALWAYS_INLINE void nsHybridString<Size, A>::operator=(const nsHybridStringBase<Size>& rhs)
{
  nsHybridStringBase<Size>::operator=(rhs);
}

template <nsUInt16 Size, typename A>
NS_ALWAYS_INLINE void nsHybridString<Size, A>::operator=(nsHybridString<Size, A>&& rhs)
{
  nsHybridStringBase<Size>::operator=(std::move(rhs));
}

template <nsUInt16 Size, typename A>
NS_ALWAYS_INLINE void nsHybridString<Size, A>::operator=(nsHybridStringBase<Size>&& rhs)
{
  nsHybridStringBase<Size>::operator=(std::move(rhs));
}

template <nsUInt16 Size, typename A>
NS_ALWAYS_INLINE void nsHybridString<Size, A>::operator=(const char* rhs)
{
  nsHybridStringBase<Size>::operator=(rhs);
}

template <nsUInt16 Size, typename A>
NS_ALWAYS_INLINE void nsHybridString<Size, A>::operator=(const wchar_t* rhs)
{
  nsHybridStringBase<Size>::operator=(rhs);
}

template <nsUInt16 Size, typename A>
NS_ALWAYS_INLINE void nsHybridString<Size, A>::operator=(const nsStringView& rhs)
{
  nsHybridStringBase<Size>::operator=(rhs);
}

#if NS_ENABLED(NS_INTEROP_STL_STRINGS)

template <nsUInt16 Size>
nsHybridStringBase<Size>::nsHybridStringBase(const std::string_view& rhs, nsAllocator* pAllocator)
{
  *this = rhs;
}

template <nsUInt16 Size>
nsHybridStringBase<Size>::nsHybridStringBase(const std::string& rhs, nsAllocator* pAllocator)
{
  *this = rhs;
}

template <nsUInt16 Size>
void nsHybridStringBase<Size>::operator=(const std::string_view& rhs)
{
  if (rhs.empty())
  {
    Clear();
  }
  else
  {
    m_Data.SetCountUninitialized(((nsUInt32)rhs.size() + 1));
    nsStringUtils::Copy(&m_Data[0], m_Data.GetCount(), rhs.data(), rhs.data() + rhs.size());
  }
}

template <nsUInt16 Size>
void nsHybridStringBase<Size>::operator=(const std::string& rhs)
{
  *this = std::string_view(rhs);
}

template <nsUInt16 Size, typename A>
NS_ALWAYS_INLINE nsHybridString<Size, A>::nsHybridString(const std::string_view& rhs)
  : nsHybridStringBase<Size>(rhs, A::GetAllocator())
{
}

template <nsUInt16 Size, typename A>
NS_ALWAYS_INLINE nsHybridString<Size, A>::nsHybridString(const std::string& rhs)
  : nsHybridStringBase<Size>(rhs, A::GetAllocator())
{
}

template <nsUInt16 Size, typename A>
NS_ALWAYS_INLINE void nsHybridString<Size, A>::operator=(const std::string_view& rhs)
{
  nsHybridStringBase<Size>::operator=(rhs);
}

template <nsUInt16 Size, typename A>
NS_ALWAYS_INLINE void nsHybridString<Size, A>::operator=(const std::string& rhs)
{
  nsHybridStringBase<Size>::operator=(rhs);
}

#endif

#include <Foundation/Strings/Implementation/AllStrings_inl.h>

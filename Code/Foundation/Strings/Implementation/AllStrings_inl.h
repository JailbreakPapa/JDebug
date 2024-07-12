#pragma once

#include <Foundation/IO/Stream.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Strings/StringBuilder.h>

template <nsUInt16 Size>
nsHybridStringBase<Size>::nsHybridStringBase(const nsStringBuilder& rhs, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  *this = rhs;
}

template <nsUInt16 Size>
nsHybridStringBase<Size>::nsHybridStringBase(nsStringBuilder&& rhs, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  *this = std::move(rhs);
}

template <nsUInt16 Size, typename A>
NS_ALWAYS_INLINE nsHybridString<Size, A>::nsHybridString(const nsStringBuilder& rhs)
  : nsHybridStringBase<Size>(rhs, A::GetAllocator())
{
}

template <nsUInt16 Size, typename A>
NS_ALWAYS_INLINE nsHybridString<Size, A>::nsHybridString(nsStringBuilder&& rhs)
  : nsHybridStringBase<Size>(std::move(rhs), A::GetAllocator())
{
}

template <nsUInt16 Size>
void nsHybridStringBase<Size>::operator=(const nsStringBuilder& rhs)
{
  m_Data = rhs.m_Data;
}

template <nsUInt16 Size>
void nsHybridStringBase<Size>::operator=(nsStringBuilder&& rhs)
{
  m_Data = std::move(rhs.m_Data);
}

template <nsUInt16 Size, typename A>
NS_ALWAYS_INLINE void nsHybridString<Size, A>::operator=(const nsStringBuilder& rhs)
{
  nsHybridStringBase<Size>::operator=(rhs);
}

template <nsUInt16 Size, typename A>
NS_ALWAYS_INLINE void nsHybridString<Size, A>::operator=(nsStringBuilder&& rhs)
{
  nsHybridStringBase<Size>::operator=(std::move(rhs));
}

template <nsUInt16 Size>
void nsHybridStringBase<Size>::ReadAll(nsStreamReader& inout_stream)
{
  Clear();

  nsHybridArray<nsUInt8, 1024 * 4> Bytes(m_Data.GetAllocator());
  nsUInt8 Temp[1024];

  while (true)
  {
    const nsUInt32 uiRead = (nsUInt32)inout_stream.ReadBytes(Temp, 1024);

    if (uiRead == 0)
      break;

    Bytes.PushBackRange(nsArrayPtr<nsUInt8>(Temp, uiRead));
  }

  Bytes.PushBack('\0');

  *this = (const char*)&Bytes[0];
}

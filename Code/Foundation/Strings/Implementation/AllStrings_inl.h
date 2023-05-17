#pragma once

#include <Foundation/IO/Stream.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Strings/StringBuilder.h>

template <wdUInt16 Size>
wdHybridStringBase<Size>::wdHybridStringBase(const wdStringBuilder& rhs, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  *this = rhs;
}

template <wdUInt16 Size>
wdHybridStringBase<Size>::wdHybridStringBase(wdStringBuilder&& rhs, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  *this = std::move(rhs);
}

template <wdUInt16 Size, typename A>
WD_ALWAYS_INLINE wdHybridString<Size, A>::wdHybridString(const wdStringBuilder& rhs)
  : wdHybridStringBase<Size>(rhs, A::GetAllocator())
{
}

template <wdUInt16 Size, typename A>
WD_ALWAYS_INLINE wdHybridString<Size, A>::wdHybridString(wdStringBuilder&& rhs)
  : wdHybridStringBase<Size>(std::move(rhs), A::GetAllocator())
{
}

template <wdUInt16 Size>
void wdHybridStringBase<Size>::operator=(const wdStringBuilder& rhs)
{
  m_uiCharacterCount = rhs.m_uiCharacterCount;
  m_Data = rhs.m_Data;
}

template <wdUInt16 Size>
void wdHybridStringBase<Size>::operator=(wdStringBuilder&& rhs)
{
  m_uiCharacterCount = rhs.m_uiCharacterCount;
  m_Data = std::move(rhs.m_Data);
}

template <wdUInt16 Size, typename A>
WD_ALWAYS_INLINE void wdHybridString<Size, A>::operator=(const wdStringBuilder& rhs)
{
  wdHybridStringBase<Size>::operator=(rhs);
}

template <wdUInt16 Size, typename A>
WD_ALWAYS_INLINE void wdHybridString<Size, A>::operator=(wdStringBuilder&& rhs)
{
  wdHybridStringBase<Size>::operator=(std::move(rhs));
}

template <wdUInt16 Size>
void wdHybridStringBase<Size>::ReadAll(wdStreamReader& inout_stream)
{
  Clear();

  wdHybridArray<wdUInt8, 1024 * 4> Bytes(m_Data.GetAllocator());
  wdUInt8 Temp[1024];

  while (true)
  {
    const wdUInt32 uiRead = (wdUInt32)inout_stream.ReadBytes(Temp, 1024);

    if (uiRead == 0)
      break;

    Bytes.PushBackRange(wdArrayPtr<wdUInt8>(Temp, uiRead));
  }

  Bytes.PushBack('\0');

  *this = (const char*)&Bytes[0];
}

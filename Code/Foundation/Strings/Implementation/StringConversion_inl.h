#pragma once

#include <Foundation/ThirdParty/utf8/utf8.h>

#include <Foundation/Strings/UnicodeUtils.h>

// **************** nsStringWChar ****************

inline nsStringWChar::nsStringWChar(nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  m_Data.PushBack('\0');
}

inline nsStringWChar::nsStringWChar(const nsUInt16* pUtf16, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  *this = pUtf16;
}

inline nsStringWChar::nsStringWChar(const nsUInt32* pUtf32, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  *this = pUtf32;
}

inline nsStringWChar::nsStringWChar(const wchar_t* pWChar, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  *this = pWChar;
}

inline nsStringWChar::nsStringWChar(nsStringView sUtf8, nsAllocator* pAllocator /*= nsFoundation::GetDefaultAllocator()*/)
{
  *this = sUtf8;
}


// **************** nsStringUtf8 ****************

inline nsStringUtf8::nsStringUtf8(nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  m_Data.PushBack('\0');
}

inline nsStringUtf8::nsStringUtf8(const char* szUtf8, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  *this = szUtf8;
}

inline nsStringUtf8::nsStringUtf8(const nsUInt16* pUtf16, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  *this = pUtf16;
}

inline nsStringUtf8::nsStringUtf8(const nsUInt32* pUtf32, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  *this = pUtf32;
}

inline nsStringUtf8::nsStringUtf8(const wchar_t* pWChar, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  *this = pWChar;
}

#if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)

inline nsStringUtf8::nsStringUtf8(
  const Microsoft::WRL::Wrappers::HString& hstring, nsAllocator* pAllocator /*= nsFoundation::GetDefaultAllocator()*/)
  : m_Data(pAllocator)
{
  *this = hstring;
}

inline nsStringUtf8::nsStringUtf8(const HSTRING& hstring, nsAllocator* pAllocator /*= nsFoundation::GetDefaultAllocator()*/)
{
  *this = hstring;
}

#endif

// **************** nsStringUtf16 ****************

inline nsStringUtf16::nsStringUtf16(nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  m_Data.PushBack('\0');
}

inline nsStringUtf16::nsStringUtf16(const char* szUtf8, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  *this = szUtf8;
}

inline nsStringUtf16::nsStringUtf16(const nsUInt16* pUtf16, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  *this = pUtf16;
}

inline nsStringUtf16::nsStringUtf16(const nsUInt32* pUtf32, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  *this = pUtf32;
}

inline nsStringUtf16::nsStringUtf16(const wchar_t* pWChar, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  *this = pWChar;
}



// **************** nsStringUtf32 ****************

inline nsStringUtf32::nsStringUtf32(nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  m_Data.PushBack('\0');
}

inline nsStringUtf32::nsStringUtf32(const char* szUtf8, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  *this = szUtf8;
}

inline nsStringUtf32::nsStringUtf32(const nsUInt16* pUtf16, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  *this = pUtf16;
}

inline nsStringUtf32::nsStringUtf32(const nsUInt32* pUtf32, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  *this = pUtf32;
}

inline nsStringUtf32::nsStringUtf32(const wchar_t* pWChar, nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  *this = pWChar;
}

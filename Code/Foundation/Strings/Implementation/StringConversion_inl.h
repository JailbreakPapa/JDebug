#pragma once

#include <Foundation/ThirdParty/utf8/utf8.h>

#include <Foundation/Strings/UnicodeUtils.h>

// **************** wdStringWChar ****************

inline wdStringWChar::wdStringWChar(wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  m_Data.PushBack('\0');
}

inline wdStringWChar::wdStringWChar(const wdUInt16* pUtf16, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  *this = pUtf16;
}

inline wdStringWChar::wdStringWChar(const wdUInt32* pUtf32, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  *this = pUtf32;
}

inline wdStringWChar::wdStringWChar(const wchar_t* pWChar, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  *this = pWChar;
}

inline wdStringWChar::wdStringWChar(wdStringView sUtf8, wdAllocatorBase* pAllocator /*= wdFoundation::GetDefaultAllocator()*/)
{
  *this = sUtf8;
}


// **************** wdStringUtf8 ****************

inline wdStringUtf8::wdStringUtf8(wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  m_Data.PushBack('\0');
}

inline wdStringUtf8::wdStringUtf8(const char* szUtf8, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  *this = szUtf8;
}

inline wdStringUtf8::wdStringUtf8(const wdUInt16* pUtf16, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  *this = pUtf16;
}

inline wdStringUtf8::wdStringUtf8(const wdUInt32* pUtf32, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  *this = pUtf32;
}

inline wdStringUtf8::wdStringUtf8(const wchar_t* pWChar, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  *this = pWChar;
}

#if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)

inline wdStringUtf8::wdStringUtf8(
  const Microsoft::WRL::Wrappers::HString& hstring, wdAllocatorBase* pAllocator /*= wdFoundation::GetDefaultAllocator()*/)
  : m_Data(pAllocator)
{
  *this = hstring;
}

inline wdStringUtf8::wdStringUtf8(const HSTRING& hstring, wdAllocatorBase* pAllocator /*= wdFoundation::GetDefaultAllocator()*/)
{
  *this = hstring;
}

#endif

// **************** wdStringUtf16 ****************

inline wdStringUtf16::wdStringUtf16(wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  m_Data.PushBack('\0');
}

inline wdStringUtf16::wdStringUtf16(const char* szUtf8, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  *this = szUtf8;
}

inline wdStringUtf16::wdStringUtf16(const wdUInt16* pUtf16, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  *this = pUtf16;
}

inline wdStringUtf16::wdStringUtf16(const wdUInt32* pUtf32, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  *this = pUtf32;
}

inline wdStringUtf16::wdStringUtf16(const wchar_t* pWChar, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  *this = pWChar;
}



// **************** wdStringUtf32 ****************

inline wdStringUtf32::wdStringUtf32(wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  m_Data.PushBack('\0');
}

inline wdStringUtf32::wdStringUtf32(const char* szUtf8, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  *this = szUtf8;
}

inline wdStringUtf32::wdStringUtf32(const wdUInt16* pUtf16, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  *this = pUtf16;
}

inline wdStringUtf32::wdStringUtf32(const wdUInt32* pUtf32, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  *this = pUtf32;
}

inline wdStringUtf32::wdStringUtf32(const wchar_t* pWChar, wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  *this = pWChar;
}

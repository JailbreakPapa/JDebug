#pragma once

#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Strings/StringView.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)
// Include our windows.h header first to get rid of defines.
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
// For HString, HStringReference and co.
#  include <wrl/wrappers/corewrappers.h>
#endif

/// \brief A very simple string class that should only be used to temporarily convert text to the OSes native wchar_t convention (16 or 32
/// Bit).
///
/// This should be used when one needs to output text via some function that only accepts wchar_t strings.
/// DO NOT use this for storage or anything else that is not temporary.
/// wchar_t is 16 Bit on Windows and 32 Bit on most other platforms. This class will always automatically convert to the correct format.
class WD_FOUNDATION_DLL wdStringWChar
{
public:
  wdStringWChar(wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator());
  wdStringWChar(const wdUInt16* pUtf16, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator());
  wdStringWChar(const wdUInt32* pUtf32, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator());
  wdStringWChar(const wchar_t* pUtf32, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator());
  wdStringWChar(wdStringView sUtf8, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator());

  void operator=(const wdUInt16* pUtf16);
  void operator=(const wdUInt32* pUtf32);
  void operator=(const wchar_t* pUtf32);
  void operator=(wdStringView sUtf8);

  WD_ALWAYS_INLINE operator const wchar_t*() const { return &m_Data[0]; }
  WD_ALWAYS_INLINE const wchar_t* GetData() const { return &m_Data[0]; }
  WD_ALWAYS_INLINE wdUInt32 GetElementCount() const { return m_Data.GetCount() - 1; /* exclude the '\0' terminator */ }

private:
  static constexpr wdUInt32 BufferSize = 1024;
  wdHybridArray<wchar_t, BufferSize> m_Data;
};


/// \brief A small string class that converts any other encoding to Utf8.
///
/// Use this class only temporarily. Do not use it for storage.
class WD_FOUNDATION_DLL wdStringUtf8
{
public:
  wdStringUtf8(wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator());
  wdStringUtf8(const char* szUtf8, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator());
  wdStringUtf8(const wdUInt16* pUtf16, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator());
  wdStringUtf8(const wdUInt32* pUtf32, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator());
  wdStringUtf8(const wchar_t* pWChar, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator());

#if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)
  wdStringUtf8(const Microsoft::WRL::Wrappers::HString& hstring, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator());
  wdStringUtf8(const HSTRING& hstring, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator());
#endif

  void operator=(const char* szUtf8);
  void operator=(const wdUInt16* pUtf16);
  void operator=(const wdUInt32* pUtf32);
  void operator=(const wchar_t* pWChar);

#if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)
  void operator=(const Microsoft::WRL::Wrappers::HString& hstring);
  void operator=(const HSTRING& hstring);
#endif

  WD_ALWAYS_INLINE operator const char*() const
  {
    return &m_Data[0];
  }
  WD_ALWAYS_INLINE const char* GetData() const
  {
    return &m_Data[0];
  }
  WD_ALWAYS_INLINE wdUInt32 GetElementCount() const
  {
    return m_Data.GetCount() - 1; /* exclude the '\0' terminator */
  }
  WD_ALWAYS_INLINE operator wdStringView() const
  {
    return GetView();
  }
  WD_ALWAYS_INLINE wdStringView GetView() const
  {
    return wdStringView(&m_Data[0], GetElementCount());
  }

private:
  static constexpr wdUInt32 BufferSize = 1024;
  wdHybridArray<char, BufferSize> m_Data;
};



/// \brief A very simple class to convert text to Utf16 encoding.
///
/// Use this class only temporarily, if you need to output something in Utf16 format, e.g. for writing it to a file.
/// Never use this for storage.
/// When working with OS functions that expect '16 Bit strings', use wdStringWChar instead.
class WD_FOUNDATION_DLL wdStringUtf16
{
public:
  wdStringUtf16(wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator());
  wdStringUtf16(const char* szUtf8, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator());
  wdStringUtf16(const wdUInt16* pUtf16, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator());
  wdStringUtf16(const wdUInt32* pUtf32, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator());
  wdStringUtf16(const wchar_t* pUtf32, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator());

  void operator=(const char* szUtf8);
  void operator=(const wdUInt16* pUtf16);
  void operator=(const wdUInt32* pUtf32);
  void operator=(const wchar_t* pUtf32);

  WD_ALWAYS_INLINE const wdUInt16* GetData() const { return &m_Data[0]; }
  WD_ALWAYS_INLINE wdUInt32 GetElementCount() const { return m_Data.GetCount() - 1; /* exclude the '\0' terminator */ }

private:
  static constexpr wdUInt32 BufferSize = 1024;
  wdHybridArray<wdUInt16, BufferSize> m_Data;
};



/// \brief This class only exists for completeness.
///
/// There should be no case where it is preferred over other classes.
class WD_FOUNDATION_DLL wdStringUtf32
{
public:
  wdStringUtf32(wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator());
  wdStringUtf32(const char* szUtf8, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator());
  wdStringUtf32(const wdUInt16* pUtf16, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator());
  wdStringUtf32(const wdUInt32* pUtf32, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator());
  wdStringUtf32(const wchar_t* pWChar, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator());

  void operator=(const char* szUtf8);
  void operator=(const wdUInt16* pUtf16);
  void operator=(const wdUInt32* pUtf32);
  void operator=(const wchar_t* pWChar);

  WD_ALWAYS_INLINE const wdUInt32* GetData() const { return &m_Data[0]; }
  WD_ALWAYS_INLINE wdUInt32 GetElementCount() const { return m_Data.GetCount() - 1; /* exclude the '\0' terminator */ }

private:
  static constexpr wdUInt32 BufferSize = 1024;
  wdHybridArray<wdUInt32, BufferSize> m_Data;
};


#if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)

/// \brief A very simple string class that should only be used to temporarily convert text to the OSes native HString (on UWP platforms).
///
/// This should be used when one needs to output text via some function that only accepts HString strings.
/// DO NOT use this for storage or anything else that is not temporary.
class WD_FOUNDATION_DLL wdStringHString
{
public:
  wdStringHString();
  wdStringHString(const char* szUtf8);
  wdStringHString(const wdUInt16* szUtf16);
  wdStringHString(const wdUInt32* szUtf32);
  wdStringHString(const wchar_t* szWChar);

  void operator=(const char* szUtf8);
  void operator=(const wdUInt16* szUtf16);
  void operator=(const wdUInt32* szUtf32);
  void operator=(const wchar_t* szWChar);

  /// \brief Unfortunately you cannot assign HStrings, so you cannot copy the result to another HString, you have to use this result
  /// directly
  WD_ALWAYS_INLINE const Microsoft::WRL::Wrappers::HString& GetData() const { return m_Data; }

private:
  Microsoft::WRL::Wrappers::HString m_Data;
};

#endif


#include <Foundation/Strings/Implementation/StringConversion_inl.h>

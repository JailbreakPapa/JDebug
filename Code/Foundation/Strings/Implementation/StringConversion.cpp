#include <Foundation/FoundationPCH.h>

#include <Foundation/Strings/StringConversion.h>

// **************** wdStringWChar ****************

void wdStringWChar::operator=(const wdUInt16* pUtf16)
{
  m_Data.Clear();

  if (pUtf16 != nullptr)
  {
    // skip any Utf16 little endian Byte Order Mark
    wdUnicodeUtils::SkipUtf16BomLE(pUtf16);
    WD_ASSERT_DEV(!wdUnicodeUtils::SkipUtf16BomBE(pUtf16), "Utf-16 Big Endian is currently not supported.");

    wdUnicodeUtils::UtfInserter<wchar_t, wdHybridArray<wchar_t, BufferSize>> tempInserter(&m_Data);

    while (*pUtf16 != '\0')
    {
      // decode utf8 to utf32
      const wdUInt32 uiUtf32 = wdUnicodeUtils::DecodeUtf16ToUtf32(pUtf16);

      // encode utf32 to wchar_t
      wdUnicodeUtils::EncodeUtf32ToWChar(uiUtf32, tempInserter);
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}

void wdStringWChar::operator=(const wdUInt32* pUtf32)
{
  m_Data.Clear();

  if (pUtf32 != nullptr)
  {
    wdUnicodeUtils::UtfInserter<wchar_t, wdHybridArray<wchar_t, BufferSize>> tempInserter(&m_Data);

    while (*pUtf32 != '\0')
    {
      // decode utf8 to utf32
      const wdUInt32 uiUtf32 = *pUtf32;
      ++pUtf32;

      // encode utf32 to wchar_t
      wdUnicodeUtils::EncodeUtf32ToWChar(uiUtf32, tempInserter);
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}

void wdStringWChar::operator=(const wchar_t* pWChar)
{
  m_Data.Clear();

  if (pWChar != nullptr)
  {

    while (*pWChar != '\0')
    {
      m_Data.PushBack(*pWChar);
      ++pWChar;
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}

void wdStringWChar::operator=(wdStringView sUtf8)
{
  m_Data.Clear();

  if (!sUtf8.IsEmpty())
  {
    const char* szUtf8 = sUtf8.GetStartPointer();

    WD_ASSERT_DEV(wdUnicodeUtils::IsValidUtf8(szUtf8), "Input Data is not a valid Utf8 string. Did you intend to use a Wide-String and forget the 'L' prefix?");

    // skip any Utf8 Byte Order Mark
    wdUnicodeUtils::SkipUtf8Bom(szUtf8);

    wdUnicodeUtils::UtfInserter<wchar_t, wdHybridArray<wchar_t, BufferSize>> tempInserter(&m_Data);

    while (szUtf8 < sUtf8.GetEndPointer() && *szUtf8 != '\0')
    {
      // decode utf8 to utf32
      const wdUInt32 uiUtf32 = wdUnicodeUtils::DecodeUtf8ToUtf32(szUtf8);

      // encode utf32 to wchar_t
      wdUnicodeUtils::EncodeUtf32ToWChar(uiUtf32, tempInserter);
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}

// **************** wdStringUtf8 ****************

void wdStringUtf8::operator=(const char* szUtf8)
{
  WD_ASSERT_DEV(
    wdUnicodeUtils::IsValidUtf8(szUtf8), "Input Data is not a valid Utf8 string. Did you intend to use a Wide-String and forget the 'L' prefix?");

  m_Data.Clear();

  if (szUtf8 != nullptr)
  {
    // skip any Utf8 Byte Order Mark
    wdUnicodeUtils::SkipUtf8Bom(szUtf8);

    while (*szUtf8 != '\0')
    {
      m_Data.PushBack(*szUtf8);
      ++szUtf8;
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}


void wdStringUtf8::operator=(const wdUInt16* pUtf16)
{
  m_Data.Clear();

  if (pUtf16 != nullptr)
  {
    // skip any Utf16 little endian Byte Order Mark
    wdUnicodeUtils::SkipUtf16BomLE(pUtf16);
    WD_ASSERT_DEV(!wdUnicodeUtils::SkipUtf16BomBE(pUtf16), "Utf-16 Big Endian is currently not supported.");

    wdUnicodeUtils::UtfInserter<char, wdHybridArray<char, BufferSize>> tempInserter(&m_Data);

    while (*pUtf16 != '\0')
    {
      // decode utf8 to utf32
      const wdUInt32 uiUtf32 = wdUnicodeUtils::DecodeUtf16ToUtf32(pUtf16);

      // encode utf32 to wchar_t
      wdUnicodeUtils::EncodeUtf32ToUtf8(uiUtf32, tempInserter);
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}


void wdStringUtf8::operator=(const wdUInt32* pUtf32)
{
  m_Data.Clear();

  if (pUtf32 != nullptr)
  {
    wdUnicodeUtils::UtfInserter<char, wdHybridArray<char, BufferSize>> tempInserter(&m_Data);

    while (*pUtf32 != '\0')
    {
      // decode utf8 to utf32
      const wdUInt32 uiUtf32 = *pUtf32;
      ++pUtf32;

      // encode utf32 to wchar_t
      wdUnicodeUtils::EncodeUtf32ToUtf8(uiUtf32, tempInserter);
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}

void wdStringUtf8::operator=(const wchar_t* pWChar)
{
  m_Data.Clear();

  if (pWChar != nullptr)
  {
    wdUnicodeUtils::UtfInserter<char, wdHybridArray<char, BufferSize>> tempInserter(&m_Data);

    while (*pWChar != '\0')
    {
      // decode utf8 to utf32
      const wdUInt32 uiUtf32 = wdUnicodeUtils::DecodeWCharToUtf32(pWChar);

      // encode utf32 to wchar_t
      wdUnicodeUtils::EncodeUtf32ToUtf8(uiUtf32, tempInserter);
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}

#if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)

void wdStringUtf8::operator=(const Microsoft::WRL::Wrappers::HString& hstring)
{
  wdUInt32 len = 0;
  const wchar_t* raw = hstring.GetRawBuffer(&len);

  // delegate to wchar_t operator
  *this = raw;
}

void wdStringUtf8::operator=(const HSTRING& hstring)
{
  Microsoft::WRL::Wrappers::HString tmp;
  tmp.Attach(hstring);

  wdUInt32 len = 0;
  const wchar_t* raw = tmp.GetRawBuffer(&len);

  // delegate to wchar_t operator
  *this = raw;
}

#endif


// **************** wdStringUtf16 ****************

void wdStringUtf16::operator=(const char* szUtf8)
{
  WD_ASSERT_DEV(
    wdUnicodeUtils::IsValidUtf8(szUtf8), "Input Data is not a valid Utf8 string. Did you intend to use a Wide-String and forget the 'L' prefix?");

  m_Data.Clear();

  if (szUtf8 != nullptr)
  {
    // skip any Utf8 Byte Order Mark
    wdUnicodeUtils::SkipUtf8Bom(szUtf8);

    wdUnicodeUtils::UtfInserter<wdUInt16, wdHybridArray<wdUInt16, BufferSize>> tempInserter(&m_Data);

    while (*szUtf8 != '\0')
    {
      // decode utf8 to utf32
      const wdUInt32 uiUtf32 = wdUnicodeUtils::DecodeUtf8ToUtf32(szUtf8);

      // encode utf32 to wchar_t
      wdUnicodeUtils::EncodeUtf32ToUtf16(uiUtf32, tempInserter);
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}


void wdStringUtf16::operator=(const wdUInt16* pUtf16)
{
  m_Data.Clear();

  if (pUtf16 != nullptr)
  {
    // skip any Utf16 little endian Byte Order Mark
    wdUnicodeUtils::SkipUtf16BomLE(pUtf16);
    WD_ASSERT_DEV(!wdUnicodeUtils::SkipUtf16BomBE(pUtf16), "Utf-16 Big Endian is currently not supported.");

    while (*pUtf16 != '\0')
    {
      m_Data.PushBack(*pUtf16);
      ++pUtf16;
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}


void wdStringUtf16::operator=(const wdUInt32* pUtf32)
{
  m_Data.Clear();

  if (pUtf32 != nullptr)
  {
    wdUnicodeUtils::UtfInserter<wdUInt16, wdHybridArray<wdUInt16, BufferSize>> tempInserter(&m_Data);

    while (*pUtf32 != '\0')
    {
      // decode utf8 to utf32
      const wdUInt32 uiUtf32 = *pUtf32;
      ++pUtf32;

      // encode utf32 to wchar_t
      wdUnicodeUtils::EncodeUtf32ToUtf16(uiUtf32, tempInserter);
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}

void wdStringUtf16::operator=(const wchar_t* pWChar)
{
  m_Data.Clear();

  if (pWChar != nullptr)
  {
    wdUnicodeUtils::UtfInserter<wdUInt16, wdHybridArray<wdUInt16, BufferSize>> tempInserter(&m_Data);

    while (*pWChar != '\0')
    {
      // decode utf8 to utf32
      const wdUInt32 uiUtf32 = wdUnicodeUtils::DecodeWCharToUtf32(pWChar);

      // encode utf32 to wchar_t
      wdUnicodeUtils::EncodeUtf32ToUtf16(uiUtf32, tempInserter);
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}



// **************** wdStringUtf32 ****************

void wdStringUtf32::operator=(const char* szUtf8)
{
  WD_ASSERT_DEV(
    wdUnicodeUtils::IsValidUtf8(szUtf8), "Input Data is not a valid Utf8 string. Did you intend to use a Wide-String and forget the 'L' prefix?");

  m_Data.Clear();

  if (szUtf8 != nullptr)
  {
    // skip any Utf8 Byte Order Mark
    wdUnicodeUtils::SkipUtf8Bom(szUtf8);

    while (*szUtf8 != '\0')
    {
      // decode utf8 to utf32
      m_Data.PushBack(wdUnicodeUtils::DecodeUtf8ToUtf32(szUtf8));
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}


void wdStringUtf32::operator=(const wdUInt16* pUtf16)
{
  m_Data.Clear();

  if (pUtf16 != nullptr)
  {
    // skip any Utf16 little endian Byte Order Mark
    wdUnicodeUtils::SkipUtf16BomLE(pUtf16);
    WD_ASSERT_DEV(!wdUnicodeUtils::SkipUtf16BomBE(pUtf16), "Utf-16 Big Endian is currently not supported.");

    while (*pUtf16 != '\0')
    {
      // decode utf16 to utf32
      m_Data.PushBack(wdUnicodeUtils::DecodeUtf16ToUtf32(pUtf16));
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}


void wdStringUtf32::operator=(const wdUInt32* pUtf32)
{
  m_Data.Clear();

  if (pUtf32 != nullptr)
  {
    while (*pUtf32 != '\0')
    {
      m_Data.PushBack(*pUtf32);
      ++pUtf32;
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}

void wdStringUtf32::operator=(const wchar_t* pWChar)
{
  m_Data.Clear();

  if (pWChar != nullptr)
  {
    while (*pWChar != '\0')
    {
      // decode wchar_t to utf32
      m_Data.PushBack(wdUnicodeUtils::DecodeWCharToUtf32(pWChar));
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}

#if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)

wdStringHString::wdStringHString()
{
}

wdStringHString::wdStringHString(const char* szUtf8)
{
  *this = szUtf8;
}

wdStringHString::wdStringHString(const wdUInt16* szUtf16)
{
  *this = szUtf16;
}

wdStringHString::wdStringHString(const wdUInt32* szUtf32)
{
  *this = szUtf32;
}

wdStringHString::wdStringHString(const wchar_t* szWChar)
{
  *this = szWChar;
}

void wdStringHString::operator=(const char* szUtf8)
{
  m_Data.Set(wdStringWChar(szUtf8).GetData());
}

void wdStringHString::operator=(const wdUInt16* szUtf16)
{
  m_Data.Set(wdStringWChar(szUtf16).GetData());
}

void wdStringHString::operator=(const wdUInt32* szUtf32)
{
  m_Data.Set(wdStringWChar(szUtf32).GetData());
}

void wdStringHString::operator=(const wchar_t* szWChar)
{
  m_Data.Set(wdStringWChar(szWChar).GetData());
}

#endif


WD_STATICLINK_FILE(Foundation, Foundation_Strings_Implementation_StringConversion);

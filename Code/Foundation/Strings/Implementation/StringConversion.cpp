#include <Foundation/FoundationPCH.h>

#include <Foundation/Strings/StringConversion.h>

// **************** nsStringWChar ****************

void nsStringWChar::operator=(const nsUInt16* pUtf16)
{
  m_Data.Clear();

  if (pUtf16 != nullptr)
  {
    // skip any Utf16 little endian Byte Order Mark
    nsUnicodeUtils::SkipUtf16BomLE(pUtf16);
    NS_ASSERT_DEV(!nsUnicodeUtils::SkipUtf16BomBE(pUtf16), "Utf-16 Big Endian is currently not supported.");

    nsUnicodeUtils::UtfInserter<wchar_t, nsHybridArray<wchar_t, BufferSize>> tempInserter(&m_Data);

    while (*pUtf16 != '\0')
    {
      // decode utf8 to utf32
      const nsUInt32 uiUtf32 = nsUnicodeUtils::DecodeUtf16ToUtf32(pUtf16);

      // encode utf32 to wchar_t
      nsUnicodeUtils::EncodeUtf32ToWChar(uiUtf32, tempInserter);
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}

void nsStringWChar::operator=(const nsUInt32* pUtf32)
{
  m_Data.Clear();

  if (pUtf32 != nullptr)
  {
    nsUnicodeUtils::UtfInserter<wchar_t, nsHybridArray<wchar_t, BufferSize>> tempInserter(&m_Data);

    while (*pUtf32 != '\0')
    {
      // decode utf8 to utf32
      const nsUInt32 uiUtf32 = *pUtf32;
      ++pUtf32;

      // encode utf32 to wchar_t
      nsUnicodeUtils::EncodeUtf32ToWChar(uiUtf32, tempInserter);
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}

void nsStringWChar::operator=(const wchar_t* pWChar)
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

void nsStringWChar::operator=(nsStringView sUtf8)
{
  m_Data.Clear();

  if (!sUtf8.IsEmpty())
  {
    const char* szUtf8 = sUtf8.GetStartPointer();

    NS_ASSERT_DEV(nsUnicodeUtils::IsValidUtf8(szUtf8), "Input Data is not a valid Utf8 string. Did you intend to use a Wide-String and forget the 'L' prefix?");

    // skip any Utf8 Byte Order Mark
    nsUnicodeUtils::SkipUtf8Bom(szUtf8);

    nsUnicodeUtils::UtfInserter<wchar_t, nsHybridArray<wchar_t, BufferSize>> tempInserter(&m_Data);

    while (szUtf8 < sUtf8.GetEndPointer() && *szUtf8 != '\0')
    {
      // decode utf8 to utf32
      const nsUInt32 uiUtf32 = nsUnicodeUtils::DecodeUtf8ToUtf32(szUtf8);

      // encode utf32 to wchar_t
      nsUnicodeUtils::EncodeUtf32ToWChar(uiUtf32, tempInserter);
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}

// **************** nsStringUtf8 ****************

void nsStringUtf8::operator=(const char* szUtf8)
{
  NS_ASSERT_DEV(
    nsUnicodeUtils::IsValidUtf8(szUtf8), "Input Data is not a valid Utf8 string. Did you intend to use a Wide-String and forget the 'L' prefix?");

  m_Data.Clear();

  if (szUtf8 != nullptr)
  {
    // skip any Utf8 Byte Order Mark
    nsUnicodeUtils::SkipUtf8Bom(szUtf8);

    while (*szUtf8 != '\0')
    {
      m_Data.PushBack(*szUtf8);
      ++szUtf8;
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}


void nsStringUtf8::operator=(const nsUInt16* pUtf16)
{
  m_Data.Clear();

  if (pUtf16 != nullptr)
  {
    // skip any Utf16 little endian Byte Order Mark
    nsUnicodeUtils::SkipUtf16BomLE(pUtf16);
    NS_ASSERT_DEV(!nsUnicodeUtils::SkipUtf16BomBE(pUtf16), "Utf-16 Big Endian is currently not supported.");

    nsUnicodeUtils::UtfInserter<char, nsHybridArray<char, BufferSize>> tempInserter(&m_Data);

    while (*pUtf16 != '\0')
    {
      // decode utf8 to utf32
      const nsUInt32 uiUtf32 = nsUnicodeUtils::DecodeUtf16ToUtf32(pUtf16);

      // encode utf32 to wchar_t
      nsUnicodeUtils::EncodeUtf32ToUtf8(uiUtf32, tempInserter);
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}


void nsStringUtf8::operator=(const nsUInt32* pUtf32)
{
  m_Data.Clear();

  if (pUtf32 != nullptr)
  {
    nsUnicodeUtils::UtfInserter<char, nsHybridArray<char, BufferSize>> tempInserter(&m_Data);

    while (*pUtf32 != '\0')
    {
      // decode utf8 to utf32
      const nsUInt32 uiUtf32 = *pUtf32;
      ++pUtf32;

      // encode utf32 to wchar_t
      nsUnicodeUtils::EncodeUtf32ToUtf8(uiUtf32, tempInserter);
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}

void nsStringUtf8::operator=(const wchar_t* pWChar)
{
  m_Data.Clear();

  if (pWChar != nullptr)
  {
    nsUnicodeUtils::UtfInserter<char, nsHybridArray<char, BufferSize>> tempInserter(&m_Data);

    while (*pWChar != '\0')
    {
      // decode utf8 to utf32
      const nsUInt32 uiUtf32 = nsUnicodeUtils::DecodeWCharToUtf32(pWChar);

      // encode utf32 to wchar_t
      nsUnicodeUtils::EncodeUtf32ToUtf8(uiUtf32, tempInserter);
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}

#if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)

void nsStringUtf8::operator=(const Microsoft::WRL::Wrappers::HString& hstring)
{
  nsUInt32 len = 0;
  const wchar_t* raw = hstring.GetRawBuffer(&len);

  // delegate to wchar_t operator
  *this = raw;
}

void nsStringUtf8::operator=(const HSTRING& hstring)
{
  Microsoft::WRL::Wrappers::HString tmp;
  tmp.Attach(hstring);

  nsUInt32 len = 0;
  const wchar_t* raw = tmp.GetRawBuffer(&len);

  // delegate to wchar_t operator
  *this = raw;
}

#endif


// **************** nsStringUtf16 ****************

void nsStringUtf16::operator=(const char* szUtf8)
{
  NS_ASSERT_DEV(
    nsUnicodeUtils::IsValidUtf8(szUtf8), "Input Data is not a valid Utf8 string. Did you intend to use a Wide-String and forget the 'L' prefix?");

  m_Data.Clear();

  if (szUtf8 != nullptr)
  {
    // skip any Utf8 Byte Order Mark
    nsUnicodeUtils::SkipUtf8Bom(szUtf8);

    nsUnicodeUtils::UtfInserter<nsUInt16, nsHybridArray<nsUInt16, BufferSize>> tempInserter(&m_Data);

    while (*szUtf8 != '\0')
    {
      // decode utf8 to utf32
      const nsUInt32 uiUtf32 = nsUnicodeUtils::DecodeUtf8ToUtf32(szUtf8);

      // encode utf32 to wchar_t
      nsUnicodeUtils::EncodeUtf32ToUtf16(uiUtf32, tempInserter);
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}


void nsStringUtf16::operator=(const nsUInt16* pUtf16)
{
  m_Data.Clear();

  if (pUtf16 != nullptr)
  {
    // skip any Utf16 little endian Byte Order Mark
    nsUnicodeUtils::SkipUtf16BomLE(pUtf16);
    NS_ASSERT_DEV(!nsUnicodeUtils::SkipUtf16BomBE(pUtf16), "Utf-16 Big Endian is currently not supported.");

    while (*pUtf16 != '\0')
    {
      m_Data.PushBack(*pUtf16);
      ++pUtf16;
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}


void nsStringUtf16::operator=(const nsUInt32* pUtf32)
{
  m_Data.Clear();

  if (pUtf32 != nullptr)
  {
    nsUnicodeUtils::UtfInserter<nsUInt16, nsHybridArray<nsUInt16, BufferSize>> tempInserter(&m_Data);

    while (*pUtf32 != '\0')
    {
      // decode utf8 to utf32
      const nsUInt32 uiUtf32 = *pUtf32;
      ++pUtf32;

      // encode utf32 to wchar_t
      nsUnicodeUtils::EncodeUtf32ToUtf16(uiUtf32, tempInserter);
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}

void nsStringUtf16::operator=(const wchar_t* pWChar)
{
  m_Data.Clear();

  if (pWChar != nullptr)
  {
    nsUnicodeUtils::UtfInserter<nsUInt16, nsHybridArray<nsUInt16, BufferSize>> tempInserter(&m_Data);

    while (*pWChar != '\0')
    {
      // decode utf8 to utf32
      const nsUInt32 uiUtf32 = nsUnicodeUtils::DecodeWCharToUtf32(pWChar);

      // encode utf32 to wchar_t
      nsUnicodeUtils::EncodeUtf32ToUtf16(uiUtf32, tempInserter);
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}



// **************** nsStringUtf32 ****************

void nsStringUtf32::operator=(const char* szUtf8)
{
  NS_ASSERT_DEV(
    nsUnicodeUtils::IsValidUtf8(szUtf8), "Input Data is not a valid Utf8 string. Did you intend to use a Wide-String and forget the 'L' prefix?");

  m_Data.Clear();

  if (szUtf8 != nullptr)
  {
    // skip any Utf8 Byte Order Mark
    nsUnicodeUtils::SkipUtf8Bom(szUtf8);

    while (*szUtf8 != '\0')
    {
      // decode utf8 to utf32
      m_Data.PushBack(nsUnicodeUtils::DecodeUtf8ToUtf32(szUtf8));
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}


void nsStringUtf32::operator=(const nsUInt16* pUtf16)
{
  m_Data.Clear();

  if (pUtf16 != nullptr)
  {
    // skip any Utf16 little endian Byte Order Mark
    nsUnicodeUtils::SkipUtf16BomLE(pUtf16);
    NS_ASSERT_DEV(!nsUnicodeUtils::SkipUtf16BomBE(pUtf16), "Utf-16 Big Endian is currently not supported.");

    while (*pUtf16 != '\0')
    {
      // decode utf16 to utf32
      m_Data.PushBack(nsUnicodeUtils::DecodeUtf16ToUtf32(pUtf16));
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}


void nsStringUtf32::operator=(const nsUInt32* pUtf32)
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

void nsStringUtf32::operator=(const wchar_t* pWChar)
{
  m_Data.Clear();

  if (pWChar != nullptr)
  {
    while (*pWChar != '\0')
    {
      // decode wchar_t to utf32
      m_Data.PushBack(nsUnicodeUtils::DecodeWCharToUtf32(pWChar));
    }
  }

  // append terminator
  m_Data.PushBack('\0');
}

#if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)

nsStringHString::nsStringHString()
{
}

nsStringHString::nsStringHString(const char* szUtf8)
{
  *this = szUtf8;
}

nsStringHString::nsStringHString(const nsUInt16* szUtf16)
{
  *this = szUtf16;
}

nsStringHString::nsStringHString(const nsUInt32* szUtf32)
{
  *this = szUtf32;
}

nsStringHString::nsStringHString(const wchar_t* szWChar)
{
  *this = szWChar;
}

void nsStringHString::operator=(const char* szUtf8)
{
  m_Data.Set(nsStringWChar(szUtf8).GetData());
}

void nsStringHString::operator=(const nsUInt16* szUtf16)
{
  m_Data.Set(nsStringWChar(szUtf16).GetData());
}

void nsStringHString::operator=(const nsUInt32* szUtf32)
{
  m_Data.Set(nsStringWChar(szUtf32).GetData());
}

void nsStringHString::operator=(const wchar_t* szWChar)
{
  m_Data.Set(nsStringWChar(szWChar).GetData());
}

#endif

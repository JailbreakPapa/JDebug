#include <FoundationTest/FoundationTestPCH.h>

// NOTE: always save as Unicode UTF-8 with signature

#include <Foundation/Strings/String.h>

NS_CREATE_SIMPLE_TEST(Strings, UnicodeUtils)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsASCII")
  {
    // test all ASCII Characters
    for (nsUInt32 i = 0; i < 128; ++i)
      NS_TEST_BOOL(nsUnicodeUtils::IsASCII(i));

    for (nsUInt32 i = 128; i < 0xFFFFF; ++i)
      NS_TEST_BOOL(!nsUnicodeUtils::IsASCII(i));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsUtf8StartByte")
  {
    nsStringUtf8 s(L"äöü€");
    // ä
    NS_TEST_BOOL(nsUnicodeUtils::IsUtf8StartByte(s.GetData()[0]));
    NS_TEST_BOOL(!nsUnicodeUtils::IsUtf8StartByte(s.GetData()[1]));

    // ö
    NS_TEST_BOOL(nsUnicodeUtils::IsUtf8StartByte(s.GetData()[2]));
    NS_TEST_BOOL(!nsUnicodeUtils::IsUtf8StartByte(s.GetData()[3]));

    // ü
    NS_TEST_BOOL(nsUnicodeUtils::IsUtf8StartByte(s.GetData()[4]));
    NS_TEST_BOOL(!nsUnicodeUtils::IsUtf8StartByte(s.GetData()[5]));

    // €
    NS_TEST_BOOL(nsUnicodeUtils::IsUtf8StartByte(s.GetData()[6]));
    NS_TEST_BOOL(!nsUnicodeUtils::IsUtf8StartByte(s.GetData()[7]));
    NS_TEST_BOOL(!nsUnicodeUtils::IsUtf8StartByte(s.GetData()[8]));

    // \0
    NS_TEST_BOOL(nsUnicodeUtils::IsUtf8StartByte(s.GetData()[9]));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsUtf8ContinuationByte")
  {
    // all ASCII Characters are not continuation bytes
    for (char i = 0; i < 127; ++i)
    {
      NS_TEST_BOOL(!nsUnicodeUtils::IsUtf8ContinuationByte(i));
    }

    for (nsUInt32 i = 0; i < 255u; ++i)
    {
      const char uiContByte = static_cast<char>(0x80 | (i & 0x3f));
      const char uiNoContByte1 = static_cast<char>(i | 0x40);
      const char uiNoContByte2 = static_cast<char>(i | 0xC0);

      NS_TEST_BOOL(nsUnicodeUtils::IsUtf8ContinuationByte(uiContByte));
      NS_TEST_BOOL(!nsUnicodeUtils::IsUtf8ContinuationByte(uiNoContByte1));
      NS_TEST_BOOL(!nsUnicodeUtils::IsUtf8ContinuationByte(uiNoContByte2));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetUtf8SequenceLength")
  {
    // All ASCII characters are 1 byte in length
    for (char i = 0; i < 127; ++i)
    {
      NS_TEST_INT(nsUnicodeUtils::GetUtf8SequenceLength(i), 1);
    }

    {
      nsStringUtf8 s(L"ä");
      NS_TEST_INT(nsUnicodeUtils::GetUtf8SequenceLength(s.GetData()[0]), 2);
    }

    {
      nsStringUtf8 s(L"ß");
      NS_TEST_INT(nsUnicodeUtils::GetUtf8SequenceLength(s.GetData()[0]), 2);
    }

    {
      nsStringUtf8 s(L"€");
      NS_TEST_INT(nsUnicodeUtils::GetUtf8SequenceLength(s.GetData()[0]), 3);
    }

    {
      nsStringUtf8 s(L"з");
      NS_TEST_INT(nsUnicodeUtils::GetUtf8SequenceLength(s.GetData()[0]), 2);
    }

    {
      nsStringUtf8 s(L"г");
      NS_TEST_INT(nsUnicodeUtils::GetUtf8SequenceLength(s.GetData()[0]), 2);
    }

    {
      nsStringUtf8 s(L"ы");
      NS_TEST_INT(nsUnicodeUtils::GetUtf8SequenceLength(s.GetData()[0]), 2);
    }

    {
      nsUInt32 u[2] = {L'\u0B87', 0};
      nsStringUtf8 s(u);
      NS_TEST_INT(nsUnicodeUtils::GetUtf8SequenceLength(s.GetData()[0]), 3);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ConvertUtf8ToUtf32")
  {
    // Just wraps around 'utf8::peek_next'
    // I think we can assume that that works.
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetSizeForCharacterInUtf8")
  {
    // All ASCII characters are 1 byte in length
    for (nsUInt32 i = 0; i < 128; ++i)
      NS_TEST_INT(nsUnicodeUtils::GetSizeForCharacterInUtf8(i), 1);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Decode")
  {
    char utf8[] = {(char)0xc3, (char)0xb6, 0};
    nsUInt16 utf16[] = {0xf6, 0};
    wchar_t wchar[] = {L'ö', 0};

    char* szUtf8 = &utf8[0];
    nsUInt16* szUtf16 = &utf16[0];
    wchar_t* szWChar = &wchar[0];

    nsUInt32 uiUtf321 = nsUnicodeUtils::DecodeUtf8ToUtf32(szUtf8);
    nsUInt32 uiUtf322 = nsUnicodeUtils::DecodeUtf16ToUtf32(szUtf16);
    nsUInt32 uiUtf323 = nsUnicodeUtils::DecodeWCharToUtf32(szWChar);

    NS_TEST_INT(uiUtf321, uiUtf322);
    NS_TEST_INT(uiUtf321, uiUtf323);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Encode")
  {
    char utf8[4] = {0};
    nsUInt16 utf16[4] = {0};
    wchar_t wchar[4] = {0};

    char* szUtf8 = &utf8[0];
    nsUInt16* szUtf16 = &utf16[0];
    wchar_t* szWChar = &wchar[0];

    nsUnicodeUtils::EncodeUtf32ToUtf8(0xf6, szUtf8);
    nsUnicodeUtils::EncodeUtf32ToUtf16(0xf6, szUtf16);
    nsUnicodeUtils::EncodeUtf32ToWChar(0xf6, szWChar);

    NS_TEST_BOOL(utf8[0] == (char)0xc3 && utf8[1] == (char)0xb6);
    NS_TEST_BOOL(utf16[0] == 0xf6);
    NS_TEST_BOOL(wchar[0] == L'ö');
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MoveToNextUtf8")
  {
    nsStringUtf8 s(L"aböäß€de");

    NS_TEST_INT(s.GetElementCount(), 13);

    const char* sz = s.GetData();

    // test how far it skips ahead

    nsUnicodeUtils::MoveToNextUtf8(sz).AssertSuccess();
    NS_TEST_BOOL(sz == &s.GetData()[1]);

    nsUnicodeUtils::MoveToNextUtf8(sz).AssertSuccess();
    NS_TEST_BOOL(sz == &s.GetData()[2]);

    nsUnicodeUtils::MoveToNextUtf8(sz).AssertSuccess();
    NS_TEST_BOOL(sz == &s.GetData()[4]);

    nsUnicodeUtils::MoveToNextUtf8(sz).AssertSuccess();
    NS_TEST_BOOL(sz == &s.GetData()[6]);

    nsUnicodeUtils::MoveToNextUtf8(sz).AssertSuccess();
    NS_TEST_BOOL(sz == &s.GetData()[8]);

    nsUnicodeUtils::MoveToNextUtf8(sz).AssertSuccess();
    NS_TEST_BOOL(sz == &s.GetData()[11]);

    nsUnicodeUtils::MoveToNextUtf8(sz).AssertSuccess();
    NS_TEST_BOOL(sz == &s.GetData()[12]);

    sz = s.GetData();
    const char* szEnd = s.GetView().GetEndPointer();


    nsUnicodeUtils::MoveToNextUtf8(sz, szEnd).AssertSuccess();
    NS_TEST_BOOL(sz == &s.GetData()[1]);

    nsUnicodeUtils::MoveToNextUtf8(sz, szEnd).AssertSuccess();
    NS_TEST_BOOL(sz == &s.GetData()[2]);

    nsUnicodeUtils::MoveToNextUtf8(sz, szEnd).AssertSuccess();
    NS_TEST_BOOL(sz == &s.GetData()[4]);

    nsUnicodeUtils::MoveToNextUtf8(sz, szEnd).AssertSuccess();
    NS_TEST_BOOL(sz == &s.GetData()[6]);

    nsUnicodeUtils::MoveToNextUtf8(sz, szEnd).AssertSuccess();
    NS_TEST_BOOL(sz == &s.GetData()[8]);

    nsUnicodeUtils::MoveToNextUtf8(sz, szEnd).AssertSuccess();
    NS_TEST_BOOL(sz == &s.GetData()[11]);

    nsUnicodeUtils::MoveToNextUtf8(sz, szEnd).AssertSuccess();
    NS_TEST_BOOL(sz == &s.GetData()[12]);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MoveToPriorUtf8")
  {
    nsStringUtf8 s(L"aböäß€de");

    const char* sz = &s.GetData()[13];

    NS_TEST_INT(s.GetElementCount(), 13);

    // test how far it skips ahead

    nsUnicodeUtils::MoveToPriorUtf8(sz, s.GetData()).AssertSuccess();
    NS_TEST_BOOL(sz == &s.GetData()[12]);

    nsUnicodeUtils::MoveToPriorUtf8(sz, s.GetData()).AssertSuccess();
    NS_TEST_BOOL(sz == &s.GetData()[11]);

    nsUnicodeUtils::MoveToPriorUtf8(sz, s.GetData()).AssertSuccess();
    NS_TEST_BOOL(sz == &s.GetData()[8]);

    nsUnicodeUtils::MoveToPriorUtf8(sz, s.GetData()).AssertSuccess();
    NS_TEST_BOOL(sz == &s.GetData()[6]);

    nsUnicodeUtils::MoveToPriorUtf8(sz, s.GetData()).AssertSuccess();
    NS_TEST_BOOL(sz == &s.GetData()[4]);

    nsUnicodeUtils::MoveToPriorUtf8(sz, s.GetData()).AssertSuccess();
    NS_TEST_BOOL(sz == &s.GetData()[2]);

    nsUnicodeUtils::MoveToPriorUtf8(sz, s.GetData()).AssertSuccess();
    NS_TEST_BOOL(sz == &s.GetData()[1]);

    nsUnicodeUtils::MoveToPriorUtf8(sz, s.GetData()).AssertSuccess();
    NS_TEST_BOOL(sz == &s.GetData()[0]);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SkipUtf8Bom")
  {
    // C++ is really stupid, chars are signed, but Utf8 only works with unsigned values ... argh!

    char szWithBom[] = {(char)0xef, (char)0xbb, (char)0xbf, 'a'};
    char szNoBom[] = {'a'};
    const char* pString = szWithBom;

    NS_TEST_BOOL(nsUnicodeUtils::SkipUtf8Bom(pString) == true);
    NS_TEST_BOOL(pString == &szWithBom[3]);

    pString = szNoBom;

    NS_TEST_BOOL(nsUnicodeUtils::SkipUtf8Bom(pString) == false);
    NS_TEST_BOOL(pString == szNoBom);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SkipUtf16BomLE")
  {
    nsUInt16 szWithBom[] = {0xfeff, 'a'};
    nsUInt16 szNoBom[] = {'a'};

    const nsUInt16* pString = szWithBom;

    NS_TEST_BOOL(nsUnicodeUtils::SkipUtf16BomLE(pString) == true);
    NS_TEST_BOOL(pString == &szWithBom[1]);

    pString = szNoBom;

    NS_TEST_BOOL(nsUnicodeUtils::SkipUtf16BomLE(pString) == false);
    NS_TEST_BOOL(pString == szNoBom);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SkipUtf16BomBE")
  {
    nsUInt16 szWithBom[] = {0xfffe, 'a'};
    nsUInt16 szNoBom[] = {'a'};

    const nsUInt16* pString = szWithBom;

    NS_TEST_BOOL(nsUnicodeUtils::SkipUtf16BomBE(pString) == true);
    NS_TEST_BOOL(pString == &szWithBom[1]);

    pString = szNoBom;

    NS_TEST_BOOL(nsUnicodeUtils::SkipUtf16BomBE(pString) == false);
    NS_TEST_BOOL(pString == szNoBom);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsUtf16Surrogate")
  {
    nsUInt16 szNoSurrogate[] = {0x2AD7};
    nsUInt16 szSurrogate[] = {0xd83e};

    NS_TEST_BOOL(nsUnicodeUtils::IsUtf16Surrogate(szNoSurrogate) == false);
    NS_TEST_BOOL(nsUnicodeUtils::IsUtf16Surrogate(szSurrogate) == true);
  }
}

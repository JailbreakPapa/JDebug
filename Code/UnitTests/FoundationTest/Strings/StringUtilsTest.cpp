#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Strings/String.h>

NS_CREATE_SIMPLE_TEST_GROUP(Strings);

NS_CREATE_SIMPLE_TEST(Strings, StringUtils)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsNullOrEmpty")
  {
    NS_TEST_BOOL(nsStringUtils::IsNullOrEmpty((char*)nullptr) == true);
    NS_TEST_BOOL(nsStringUtils::IsNullOrEmpty("") == true);

    // all other characters are not empty
    for (nsUInt8 c = 1; c < 255; c++)
      NS_TEST_BOOL(nsStringUtils::IsNullOrEmpty(&c) == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetStringElementCount")
  {
    NS_TEST_INT(nsStringUtils::GetStringElementCount((char*)nullptr), 0);

    // Counts the Bytes
    NS_TEST_INT(nsStringUtils::GetStringElementCount(""), 0);
    NS_TEST_INT(nsStringUtils::GetStringElementCount("a"), 1);
    NS_TEST_INT(nsStringUtils::GetStringElementCount("ab"), 2);
    NS_TEST_INT(nsStringUtils::GetStringElementCount("abc"), 3);

    // Counts the number of wchar_t's
    NS_TEST_INT(nsStringUtils::GetStringElementCount(L""), 0);
    NS_TEST_INT(nsStringUtils::GetStringElementCount(L"a"), 1);
    NS_TEST_INT(nsStringUtils::GetStringElementCount(L"ab"), 2);
    NS_TEST_INT(nsStringUtils::GetStringElementCount(L"abc"), 3);

    // test with a sub-string
    const char* sz = "abc def ghi";
    NS_TEST_INT(nsStringUtils::GetStringElementCount(sz, sz + 0), 0);
    NS_TEST_INT(nsStringUtils::GetStringElementCount(sz, sz + 3), 3);
    NS_TEST_INT(nsStringUtils::GetStringElementCount(sz, sz + 6), 6);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "UpdateStringEnd")
  {
    const char* sz = "Test test";
    const char* szEnd = nsUnicodeUtils::GetMaxStringEnd<char>();

    nsStringUtils::UpdateStringEnd(sz, szEnd);
    NS_TEST_BOOL(szEnd == sz + nsStringUtils::GetStringElementCount(sz));

    nsStringUtils::UpdateStringEnd(sz, szEnd);
    NS_TEST_BOOL(szEnd == sz + nsStringUtils::GetStringElementCount(sz));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetCharacterCount")
  {
    NS_TEST_INT(nsStringUtils::GetCharacterCount(nullptr), 0);
    NS_TEST_INT(nsStringUtils::GetCharacterCount(""), 0);
    NS_TEST_INT(nsStringUtils::GetCharacterCount("a"), 1);
    NS_TEST_INT(nsStringUtils::GetCharacterCount("abc"), 3);

    nsStringUtf8 s(L"äöü"); // 6 Bytes

    NS_TEST_INT(nsStringUtils::GetStringElementCount(s.GetData()), 6);
    NS_TEST_INT(nsStringUtils::GetCharacterCount(s.GetData()), 3);

    // test with a sub-string
    const char* sz = "abc def ghi";
    NS_TEST_INT(nsStringUtils::GetCharacterCount(sz, sz + 0), 0);
    NS_TEST_INT(nsStringUtils::GetCharacterCount(sz, sz + 3), 3);
    NS_TEST_INT(nsStringUtils::GetCharacterCount(sz, sz + 6), 6);

    NS_TEST_INT(nsStringUtils::GetCharacterCount(s.GetData(), s.GetData() + 0), 0);
    NS_TEST_INT(nsStringUtils::GetCharacterCount(s.GetData(), s.GetData() + 2), 1);
    NS_TEST_INT(nsStringUtils::GetCharacterCount(s.GetData(), s.GetData() + 4), 2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetCharacterAndElementCount")
  {
    nsUInt32 uiCC, uiEC;

    nsStringUtils::GetCharacterAndElementCount(nullptr, uiCC, uiEC);
    NS_TEST_INT(uiCC, 0);
    NS_TEST_INT(uiEC, 0);

    nsStringUtils::GetCharacterAndElementCount("", uiCC, uiEC);
    NS_TEST_INT(uiCC, 0);
    NS_TEST_INT(uiEC, 0);

    nsStringUtils::GetCharacterAndElementCount("a", uiCC, uiEC);
    NS_TEST_INT(uiCC, 1);
    NS_TEST_INT(uiEC, 1);

    nsStringUtils::GetCharacterAndElementCount("abc", uiCC, uiEC);
    NS_TEST_INT(uiCC, 3);
    NS_TEST_INT(uiEC, 3);

    nsStringUtf8 s(L"äöü"); // 6 Bytes

    nsStringUtils::GetCharacterAndElementCount(s.GetData(), uiCC, uiEC);
    NS_TEST_INT(uiCC, 3);
    NS_TEST_INT(uiEC, 6);

    nsStringUtils::GetCharacterAndElementCount(s.GetData(), uiCC, uiEC, s.GetData() + 0);
    NS_TEST_INT(uiCC, 0);
    NS_TEST_INT(uiEC, 0);

    nsStringUtils::GetCharacterAndElementCount(s.GetData(), uiCC, uiEC, s.GetData() + 4);
    NS_TEST_INT(uiCC, 2);
    NS_TEST_INT(uiEC, 4);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Copy (full)")
  {
    char szDest[256] = "";

    // large enough
    NS_TEST_INT(nsStringUtils::Copy(szDest, 256, "Test ABC"), 8);
    NS_TEST_BOOL(nsStringUtils::IsEqual(szDest, "Test ABC"));

    // exactly fitting
    NS_TEST_INT(nsStringUtils::Copy(szDest, 13, "Humpf, humpf"), 12);
    NS_TEST_BOOL(nsStringUtils::IsEqual(szDest, "Humpf, humpf"));

    // too small
    NS_TEST_INT(nsStringUtils::Copy(szDest, 8, "Test ABC"), 7);
    NS_TEST_BOOL(nsStringUtils::IsEqual(szDest, "Test AB"));

    const char* szUTF8 = "ABC \xe6\x97\xa5\xd1\x88"; // contains 'ABC ' + two UTF-8 chars (first is three bytes, second is two bytes)

    // large enough
    NS_TEST_INT(nsStringUtils::Copy(szDest, 256, szUTF8), 9);
    NS_TEST_BOOL(nsStringUtils::IsEqual(szDest, szUTF8));

    // exactly fitting
    NS_TEST_INT(nsStringUtils::Copy(szDest, 10, szUTF8), 9);
    NS_TEST_BOOL(nsStringUtils::IsEqual(szDest, szUTF8));

    // These tests are disabled as previously valid behavior was now turned into an assert.
    // Comment them in to test the assert.
    // too small 1
    /*NS_TEST_INT(nsStringUtils::Copy(szDest, 9, szUTF8), 7);
    NS_TEST_BOOL(nsStringUtils::IsEqualN(szDest, szUTF8, 5)); // one character less

    // too small 2
    NS_TEST_INT(nsStringUtils::Copy(szDest, 7, szUTF8), 4);
    NS_TEST_BOOL(nsStringUtils::IsEqualN(szDest, szUTF8, 4)); // two characters less*/


    // copy only from a subset
    NS_TEST_INT(nsStringUtils::Copy(szDest, 256, szUTF8, szUTF8 + 7), 7);
    NS_TEST_BOOL(nsStringUtils::IsEqualN(szDest, szUTF8, 5)); // two characters less
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CopyN")
  {
    char szDest[256] = "";

    NS_TEST_INT(nsStringUtils::CopyN(szDest, 256, "Test ABC", 4), 4);
    NS_TEST_BOOL(nsStringUtils::IsEqual(szDest, "Test"));

    const char* szUTF8 = "ABC \xe6\x97\xa5\xd1\x88"; // contains 'ABC ' + two UTF-8 chars (first is three bytes, second is two bytes)

    NS_TEST_INT(nsStringUtils::CopyN(szDest, 256, szUTF8, 6), 9);
    NS_TEST_BOOL(nsStringUtils::IsEqualN(szDest, szUTF8, 6));

    NS_TEST_INT(nsStringUtils::CopyN(szDest, 256, szUTF8, 5), 7);
    NS_TEST_BOOL(nsStringUtils::IsEqualN(szDest, szUTF8, 5));

    NS_TEST_INT(nsStringUtils::CopyN(szDest, 256, szUTF8, 4), 4);
    NS_TEST_BOOL(nsStringUtils::IsEqualN(szDest, szUTF8, 4));

    NS_TEST_INT(nsStringUtils::CopyN(szDest, 256, szUTF8, 1), 1);
    NS_TEST_BOOL(nsStringUtils::IsEqualN(szDest, szUTF8, 1));

    NS_TEST_INT(nsStringUtils::CopyN(szDest, 256, szUTF8, 0), 0);
    NS_TEST_BOOL(nsStringUtils::IsEqual(szDest, ""));

    // copy only from a subset
    NS_TEST_INT(nsStringUtils::CopyN(szDest, 256, szUTF8, 6, szUTF8 + 7), 7);
    NS_TEST_BOOL(nsStringUtils::IsEqualN(szDest, szUTF8, 5));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ToUpperChar")
  {
    // this only tests the ASCII range
    for (nsInt32 i = 0; i < 128; ++i)
      NS_TEST_INT(nsStringUtils::ToUpperChar(i), toupper(i));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ToLowerChar")
  {
    // this only tests the ASCII range
    for (nsInt32 i = 0; i < 128; ++i)
      NS_TEST_INT(nsStringUtils::ToLowerChar(i), tolower(i));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ToUpperString")
  {
    nsStringUtf8 sL(L"abc öäü ß €");
    nsStringUtf8 sU(L"ABC ÖÄÜ ß €");

    char szCopy[256];
    nsStringUtils::Copy(szCopy, 256, sL.GetData());

    nsStringUtils::ToUpperString(szCopy);

    NS_TEST_BOOL(nsStringUtils::IsEqual(szCopy, sU.GetData()));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ToLowerString")
  {
    nsStringUtf8 sL(L"abc öäü ß €");
    nsStringUtf8 sU(L"ABC ÖÄÜ ß €");

    char szCopy[256];
    nsStringUtils::Copy(szCopy, 256, sU.GetData());

    nsStringUtils::ToLowerString(szCopy);

    NS_TEST_BOOL(nsStringUtils::IsEqual(szCopy, sL.GetData()));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CompareChars")
  {
    NS_TEST_BOOL(nsStringUtils::CompareChars('a', 'a') == 0); // make sure the order is right
    NS_TEST_BOOL(nsStringUtils::CompareChars('a', 'b') < 0);  // a smaller than b -> negative
    NS_TEST_BOOL(nsStringUtils::CompareChars('b', 'a') > 0);  // b bigger than a  -> positive
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CompareChars(utf8)")
  {
    NS_TEST_BOOL(nsStringUtils::CompareChars("a", "a") == 0); // make sure the order is right
    NS_TEST_BOOL(nsStringUtils::CompareChars("a", "b") < 0);  // a smaller than b -> negative
    NS_TEST_BOOL(nsStringUtils::CompareChars("b", "a") > 0);  // b bigger than a  -> positive
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CompareChars_NoCase")
  {
    NS_TEST_BOOL(nsStringUtils::CompareChars_NoCase('a', 'A') == 0);
    NS_TEST_BOOL(nsStringUtils::CompareChars_NoCase('a', 'B') < 0);
    NS_TEST_BOOL(nsStringUtils::CompareChars_NoCase('B', 'a') > 0);

    NS_TEST_BOOL(nsStringUtils::CompareChars_NoCase('A', 'a') == 0);
    NS_TEST_BOOL(nsStringUtils::CompareChars_NoCase('A', 'b') < 0);
    NS_TEST_BOOL(nsStringUtils::CompareChars_NoCase('b', 'A') > 0);

    NS_TEST_BOOL(nsStringUtils::CompareChars_NoCase(L'ä', L'Ä') == 0);
    NS_TEST_BOOL(nsStringUtils::CompareChars_NoCase(L'ä', L'Ö') < 0);
    NS_TEST_BOOL(nsStringUtils::CompareChars_NoCase(L'ö', L'Ä') > 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CompareChars_NoCase(utf8)")
  {
    NS_TEST_BOOL(nsStringUtils::CompareChars_NoCase("a", "A") == 0);
    NS_TEST_BOOL(nsStringUtils::CompareChars_NoCase("a", "B") < 0);
    NS_TEST_BOOL(nsStringUtils::CompareChars_NoCase("B", "a") > 0);

    NS_TEST_BOOL(nsStringUtils::CompareChars_NoCase("A", "a") == 0);
    NS_TEST_BOOL(nsStringUtils::CompareChars_NoCase("A", "b") < 0);
    NS_TEST_BOOL(nsStringUtils::CompareChars_NoCase("b", "A") > 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsEqual")
  {
    NS_TEST_BOOL(nsStringUtils::IsEqual(nullptr, nullptr) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqual(nullptr, "") == true);
    NS_TEST_BOOL(nsStringUtils::IsEqual("", nullptr) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqual("", "") == true);

    NS_TEST_BOOL(nsStringUtils::IsEqual("abc", "abc") == true);
    NS_TEST_BOOL(nsStringUtils::IsEqual("abc", "abcd") == false);
    NS_TEST_BOOL(nsStringUtils::IsEqual("abcd", "abc") == false);

    NS_TEST_BOOL(nsStringUtils::IsEqual("a", nullptr) == false);
    NS_TEST_BOOL(nsStringUtils::IsEqual(nullptr, "a") == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsEqualN")
  {
    NS_TEST_BOOL(nsStringUtils::IsEqualN(nullptr, nullptr, 1) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqualN(nullptr, "", 1) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqualN("", nullptr, 1) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqualN("", "", 1) == true);

    // as long as we compare 'nothing' the strings must be equal
    NS_TEST_BOOL(nsStringUtils::IsEqualN("abc", nullptr, 0) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqualN("abc", "", 0) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqualN(nullptr, "abc", 0) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqualN("", "abc", 0) == true);

    NS_TEST_BOOL(nsStringUtils::IsEqualN("abc", "abcdef", 1) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqualN("abc", "abcdef", 2) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqualN("abc", "abcdef", 3) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqualN("abc", "abcdef", 4) == false);

    NS_TEST_BOOL(nsStringUtils::IsEqualN("abcdef", "abc", 1) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqualN("abcdef", "abc", 2) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqualN("abcdef", "abc", 3) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqualN("abcdef", "abc", 4) == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsEqual_NoCase")
  {
    NS_TEST_BOOL(nsStringUtils::IsEqual_NoCase(nullptr, nullptr) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqual_NoCase(nullptr, "") == true);
    NS_TEST_BOOL(nsStringUtils::IsEqual_NoCase("", nullptr) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqual_NoCase("", "") == true);


    nsStringUtf8 sL(L"abc öäü ß €");
    nsStringUtf8 sU(L"ABC ÖÄÜ ß €");
    nsStringUtf8 sU2(L"ABC ÖÄÜ ß € ");

    NS_TEST_BOOL(nsStringUtils::IsEqual_NoCase(sL.GetData(), sU.GetData()) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqual_NoCase(sL.GetData(), sU2.GetData()) == false);
    NS_TEST_BOOL(nsStringUtils::IsEqual_NoCase(sU2.GetData(), sL.GetData()) == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsEqualN_NoCase")
  {
    NS_TEST_BOOL(nsStringUtils::IsEqualN_NoCase(nullptr, nullptr, 1) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqualN_NoCase(nullptr, "", 1) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqualN_NoCase("", nullptr, 1) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqualN_NoCase("", "", 1) == true);

    // as long as we compare 'nothing' the strings must be equal
    NS_TEST_BOOL(nsStringUtils::IsEqualN_NoCase("abc", nullptr, 0) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqualN_NoCase("abc", "", 0) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqualN_NoCase(nullptr, "abc", 0) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqualN_NoCase("", "abc", 0) == true);

    nsStringUtf8 sL(L"abc öäü ß €");
    nsStringUtf8 sU(L"ABC ÖÄÜ ß € moep");

    for (nsInt32 i = 0; i < 12; ++i)
      NS_TEST_BOOL(nsStringUtils::IsEqualN_NoCase(sL.GetData(), sU.GetData(), i) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqualN_NoCase(sL.GetData(), sU.GetData(), 12) == false);

    for (nsInt32 i = 0; i < 12; ++i)
      NS_TEST_BOOL(nsStringUtils::IsEqualN_NoCase(sU.GetData(), sL.GetData(), i) == true);
    NS_TEST_BOOL(nsStringUtils::IsEqualN_NoCase(sU.GetData(), sL.GetData(), 12) == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Compare")
  {
    NS_TEST_BOOL(nsStringUtils::Compare(nullptr, nullptr) == 0);
    NS_TEST_BOOL(nsStringUtils::Compare(nullptr, "") == 0);
    NS_TEST_BOOL(nsStringUtils::Compare("", nullptr) == 0);
    NS_TEST_BOOL(nsStringUtils::Compare("", "") == 0);

    NS_TEST_BOOL(nsStringUtils::Compare("abc", "abc") == 0);
    NS_TEST_BOOL(nsStringUtils::Compare("abc", "abcd") < 0);
    NS_TEST_BOOL(nsStringUtils::Compare("abcd", "abc") > 0);

    NS_TEST_BOOL(nsStringUtils::Compare("a", nullptr) > 0);
    NS_TEST_BOOL(nsStringUtils::Compare(nullptr, "a") < 0);

    // substring compare
    const char* sz = "abc def ghi bla";
    NS_TEST_BOOL(nsStringUtils::Compare(sz, "abc", sz + 3) == 0);
    NS_TEST_BOOL(nsStringUtils::Compare(sz, "abc def", sz + 7) == 0);
    NS_TEST_BOOL(nsStringUtils::Compare(sz, sz, sz + 7, sz + 7) == 0);
    NS_TEST_BOOL(nsStringUtils::Compare(sz, sz, sz + 7, sz + 6) > 0);
    NS_TEST_BOOL(nsStringUtils::Compare(sz, sz, sz + 7, sz + 8) < 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CompareN")
  {
    NS_TEST_BOOL(nsStringUtils::CompareN(nullptr, nullptr, 1) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN(nullptr, "", 1) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN("", nullptr, 1) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN("", "", 1) == 0);

    // as long as we compare 'nothing' the strings must be equal
    NS_TEST_BOOL(nsStringUtils::CompareN("abc", nullptr, 0) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN("abc", "", 0) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN(nullptr, "abc", 0) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN("", "abc", 0) == 0);

    NS_TEST_BOOL(nsStringUtils::CompareN("abc", "abcdef", 1) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN("abc", "abcdef", 2) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN("abc", "abcdef", 3) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN("abc", "abcdef", 4) < 0);

    NS_TEST_BOOL(nsStringUtils::CompareN("abcdef", "abc", 1) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN("abcdef", "abc", 2) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN("abcdef", "abc", 3) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN("abcdef", "abc", 4) > 0);

    // substring compare
    const char* sz = "abc def ghi bla";
    NS_TEST_BOOL(nsStringUtils::CompareN(sz, "abc", 10, sz + 3) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN(sz, "abc def", 10, sz + 7) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN(sz, sz, 10, sz + 7, sz + 7) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN(sz, sz, 10, sz + 7, sz + 6) > 0);
    NS_TEST_BOOL(nsStringUtils::CompareN(sz, sz, 10, sz + 7, sz + 8) < 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Compare_NoCase")
  {
    NS_TEST_BOOL(nsStringUtils::Compare_NoCase(nullptr, nullptr) == 0);
    NS_TEST_BOOL(nsStringUtils::Compare_NoCase(nullptr, "") == 0);
    NS_TEST_BOOL(nsStringUtils::Compare_NoCase("", nullptr) == 0);
    NS_TEST_BOOL(nsStringUtils::Compare_NoCase("", "") == 0);

    NS_TEST_BOOL(nsStringUtils::Compare_NoCase("abc", "aBc") == 0);
    NS_TEST_BOOL(nsStringUtils::Compare_NoCase("ABC", "abcd") < 0);
    NS_TEST_BOOL(nsStringUtils::Compare_NoCase("abcd", "ABC") > 0);

    NS_TEST_BOOL(nsStringUtils::Compare_NoCase("a", nullptr) > 0);
    NS_TEST_BOOL(nsStringUtils::Compare_NoCase(nullptr, "a") < 0);

    // substring compare
    const char* sz = "abc def ghi bla";
    NS_TEST_BOOL(nsStringUtils::Compare_NoCase(sz, "ABC", sz + 3) == 0);
    NS_TEST_BOOL(nsStringUtils::Compare_NoCase(sz, "ABC def", sz + 7) == 0);
    NS_TEST_BOOL(nsStringUtils::Compare_NoCase(sz, sz, sz + 7, sz + 7) == 0);
    NS_TEST_BOOL(nsStringUtils::Compare_NoCase(sz, sz, sz + 7, sz + 6) > 0);
    NS_TEST_BOOL(nsStringUtils::Compare_NoCase(sz, sz, sz + 7, sz + 8) < 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CompareN_NoCase")
  {
    NS_TEST_BOOL(nsStringUtils::CompareN_NoCase(nullptr, nullptr, 1) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN_NoCase(nullptr, "", 1) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN_NoCase("", nullptr, 1) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN_NoCase("", "", 1) == 0);

    // as long as we compare 'nothing' the strings must be equal
    NS_TEST_BOOL(nsStringUtils::CompareN_NoCase("abc", nullptr, 0) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN_NoCase("abc", "", 0) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN_NoCase(nullptr, "abc", 0) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN_NoCase("", "abc", 0) == 0);

    NS_TEST_BOOL(nsStringUtils::CompareN_NoCase("aBc", "abcdef", 1) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN_NoCase("aBc", "abcdef", 2) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN_NoCase("aBc", "abcdef", 3) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN_NoCase("aBc", "abcdef", 4) < 0);

    NS_TEST_BOOL(nsStringUtils::CompareN_NoCase("abcdef", "Abc", 1) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN_NoCase("abcdef", "Abc", 2) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN_NoCase("abcdef", "Abc", 3) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN_NoCase("abcdef", "Abc", 4) > 0);

    // substring compare
    const char* sz = "abc def ghi bla";
    NS_TEST_BOOL(nsStringUtils::CompareN_NoCase(sz, "ABC", 10, sz + 3) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN_NoCase(sz, "ABC def", 10, sz + 7) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN_NoCase(sz, sz, 10, sz + 7, sz + 7) == 0);
    NS_TEST_BOOL(nsStringUtils::CompareN_NoCase(sz, sz, 10, sz + 7, sz + 6) > 0);
    NS_TEST_BOOL(nsStringUtils::CompareN_NoCase(sz, sz, 10, sz + 7, sz + 8) < 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "snprintf")
  {
    // This function has been tested to death during its implementation.
    // That test-code would require several pages, if one would try to test it properly.
    // I am not going to do that here, I am quite confident the function works as expected with pure ASCII strings.
    // So I'm only testing a bit of Utf8 stuff.

    nsStringUtf8 s(L"Abc %s äöü ß %i %s %.4f");
    nsStringUtf8 s2(L"ÄÖÜ");

    char sz[256];
    nsStringUtils::snprintf(sz, 256, s.GetData(), "ASCII", 42, s2.GetData(), 23.31415);

    nsStringUtf8 sC(L"Abc ASCII äöü ß 42 ÄÖÜ 23.3142"); // notice the correct float rounding ;-)

    NS_TEST_STRING(sz, sC.GetData());


    // NaN and Infinity
    nsStringUtils::snprintf(sz, 256, "NaN Value: %.2f", nsMath::NaN<float>());
    NS_TEST_STRING(sz, "NaN Value: NaN");

    nsStringUtils::snprintf(sz, 256, "Inf Value: %.2f", +nsMath::Infinity<float>());
    NS_TEST_STRING(sz, "Inf Value: Infinity");

    nsStringUtils::snprintf(sz, 256, "Inf Value: %.2f", -nsMath::Infinity<float>());
    NS_TEST_STRING(sz, "Inf Value: -Infinity");

    nsStringUtils::snprintf(sz, 256, "NaN Value: %.2e", nsMath::NaN<float>());
    NS_TEST_STRING(sz, "NaN Value: NaN");

    nsStringUtils::snprintf(sz, 256, "Inf Value: %.2e", +nsMath::Infinity<float>());
    NS_TEST_STRING(sz, "Inf Value: Infinity");

    nsStringUtils::snprintf(sz, 256, "Inf Value: %.2e", -nsMath::Infinity<float>());
    NS_TEST_STRING(sz, "Inf Value: -Infinity");

    nsStringUtils::snprintf(sz, 256, "NaN Value: %+10.2f", nsMath::NaN<float>());
    NS_TEST_STRING(sz, "NaN Value:       +NaN");

    nsStringUtils::snprintf(sz, 256, "Inf Value: %+10.2f", +nsMath::Infinity<float>());
    NS_TEST_STRING(sz, "Inf Value:  +Infinity");

    nsStringUtils::snprintf(sz, 256, "Inf Value: %+10.2f", -nsMath::Infinity<float>());
    NS_TEST_STRING(sz, "Inf Value:  -Infinity");

    // extended stuff
    nsStringUtils::snprintf(sz, 256, "size: %zu", (size_t)12345678);
    NS_TEST_STRING(sz, "size: 12345678");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "StartsWith")
  {
    NS_TEST_BOOL(nsStringUtils::StartsWith(nullptr, nullptr) == true);
    NS_TEST_BOOL(nsStringUtils::StartsWith(nullptr, "") == true);
    NS_TEST_BOOL(nsStringUtils::StartsWith("", nullptr) == true);
    NS_TEST_BOOL(nsStringUtils::StartsWith("", "") == true);

    NS_TEST_BOOL(nsStringUtils::StartsWith("abc", nullptr) == true);
    NS_TEST_BOOL(nsStringUtils::StartsWith("abc", "") == true);
    NS_TEST_BOOL(nsStringUtils::StartsWith(nullptr, "abc") == false);
    NS_TEST_BOOL(nsStringUtils::StartsWith("", "abc") == false);

    NS_TEST_BOOL(nsStringUtils::StartsWith("abc", "abc") == true);
    NS_TEST_BOOL(nsStringUtils::StartsWith("abcdef", "abc") == true);
    NS_TEST_BOOL(nsStringUtils::StartsWith("abcdef", "Abc") == false);

    // substring test
    const char* sz = (const char*)u8"äbc def ghi";
    const nsUInt32 uiByteCount = nsStringUtils::GetStringElementCount(u8"äbc");

    NS_TEST_BOOL(nsStringUtils::StartsWith(sz, (const char*)u8"äbc", sz + uiByteCount) == true);
    NS_TEST_BOOL(nsStringUtils::StartsWith(sz, (const char*)u8"äbc", sz + uiByteCount - 1) == false);
    NS_TEST_BOOL(nsStringUtils::StartsWith(sz, (const char*)u8"äbc", sz + 0) == false);

    const char* sz2 = (const char*)u8"äbc def";
    NS_TEST_BOOL(nsStringUtils::StartsWith(sz, sz2, sz + uiByteCount, sz2 + uiByteCount) == true);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "StartsWith_NoCase")
  {
    nsStringUtf8 sL(L"äöü");
    nsStringUtf8 sU(L"ÄÖÜ");

    NS_TEST_BOOL(nsStringUtils::StartsWith_NoCase(nullptr, nullptr) == true);
    NS_TEST_BOOL(nsStringUtils::StartsWith_NoCase(nullptr, "") == true);
    NS_TEST_BOOL(nsStringUtils::StartsWith_NoCase("", nullptr) == true);
    NS_TEST_BOOL(nsStringUtils::StartsWith_NoCase("", "") == true);

    NS_TEST_BOOL(nsStringUtils::StartsWith_NoCase("abc", nullptr) == true);
    NS_TEST_BOOL(nsStringUtils::StartsWith_NoCase("abc", "") == true);
    NS_TEST_BOOL(nsStringUtils::StartsWith_NoCase(nullptr, "abc") == false);
    NS_TEST_BOOL(nsStringUtils::StartsWith_NoCase("", "abc") == false);

    NS_TEST_BOOL(nsStringUtils::StartsWith_NoCase("abc", "ABC") == true);
    NS_TEST_BOOL(nsStringUtils::StartsWith_NoCase("aBCdef", "abc") == true);
    NS_TEST_BOOL(nsStringUtils::StartsWith_NoCase("aBCdef", "bc") == false);

    NS_TEST_BOOL(nsStringUtils::StartsWith_NoCase(sL.GetData(), sU.GetData()) == true);

    // substring test
    const char* sz = (const char*)u8"äbc def ghi";
    const nsUInt32 uiByteCount = nsStringUtils::GetStringElementCount(u8"äbc");
    NS_TEST_BOOL(nsStringUtils::StartsWith_NoCase(sz, (const char*)u8"ÄBC", sz + uiByteCount) == true);
    NS_TEST_BOOL(nsStringUtils::StartsWith_NoCase(sz, (const char*)u8"ÄBC", sz + uiByteCount - 1) == false);
    NS_TEST_BOOL(nsStringUtils::StartsWith_NoCase(sz, (const char*)u8"ÄBC", sz + 0) == false);

    const char* sz2 = (const char*)u8"Äbc def";
    NS_TEST_BOOL(nsStringUtils::StartsWith_NoCase(sz, sz2, sz + uiByteCount, sz2 + uiByteCount) == true);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "EndsWith")
  {
    NS_TEST_BOOL(nsStringUtils::EndsWith(nullptr, nullptr) == true);
    NS_TEST_BOOL(nsStringUtils::EndsWith(nullptr, "") == true);
    NS_TEST_BOOL(nsStringUtils::EndsWith("", nullptr) == true);
    NS_TEST_BOOL(nsStringUtils::EndsWith("", "") == true);

    NS_TEST_BOOL(nsStringUtils::EndsWith("abc", nullptr) == true);
    NS_TEST_BOOL(nsStringUtils::EndsWith("abc", "") == true);
    NS_TEST_BOOL(nsStringUtils::EndsWith(nullptr, "abc") == false);
    NS_TEST_BOOL(nsStringUtils::EndsWith("", "abc") == false);

    NS_TEST_BOOL(nsStringUtils::EndsWith("abc", "abc") == true);
    NS_TEST_BOOL(nsStringUtils::EndsWith("abcdef", "def") == true);
    NS_TEST_BOOL(nsStringUtils::EndsWith("abcdef", "Def") == false);
    NS_TEST_BOOL(nsStringUtils::EndsWith("def", "abcdef") == false);

    // substring test
    const char* sz = "abc def ghi";
    NS_TEST_BOOL(nsStringUtils::EndsWith(sz, "abc", sz + 3) == true);
    NS_TEST_BOOL(nsStringUtils::EndsWith(sz, "def", sz + 7) == true);
    NS_TEST_BOOL(nsStringUtils::EndsWith(sz, "def", sz + 8) == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "EndsWith_NoCase")
  {
    nsStringUtf8 sL(L"äöü");
    nsStringUtf8 sU(L"ÄÖÜ");

    NS_TEST_BOOL(nsStringUtils::EndsWith_NoCase(nullptr, nullptr) == true);
    NS_TEST_BOOL(nsStringUtils::EndsWith_NoCase(nullptr, "") == true);
    NS_TEST_BOOL(nsStringUtils::EndsWith_NoCase("", nullptr) == true);
    NS_TEST_BOOL(nsStringUtils::EndsWith_NoCase("", "") == true);

    NS_TEST_BOOL(nsStringUtils::EndsWith_NoCase("abc", nullptr) == true);
    NS_TEST_BOOL(nsStringUtils::EndsWith_NoCase("abc", "") == true);
    NS_TEST_BOOL(nsStringUtils::EndsWith_NoCase(nullptr, "abc") == false);
    NS_TEST_BOOL(nsStringUtils::EndsWith_NoCase("", "abc") == false);

    NS_TEST_BOOL(nsStringUtils::EndsWith_NoCase("abc", "abc") == true);
    NS_TEST_BOOL(nsStringUtils::EndsWith_NoCase("abcdef", "def") == true);
    NS_TEST_BOOL(nsStringUtils::EndsWith_NoCase("abcdef", "Def") == true);

    NS_TEST_BOOL(nsStringUtils::EndsWith_NoCase("def", "abcdef") == false);

    NS_TEST_BOOL(nsStringUtils::EndsWith_NoCase(sL.GetData(), sU.GetData()) == true);

    // substring test
    const char* sz = "abc def ghi";
    NS_TEST_BOOL(nsStringUtils::EndsWith_NoCase(sz, "ABC", sz + 3) == true);
    NS_TEST_BOOL(nsStringUtils::EndsWith_NoCase(sz, "DEF", sz + 7) == true);
    NS_TEST_BOOL(nsStringUtils::EndsWith_NoCase(sz, "DEF", sz + 8) == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FindSubString")
  {
    nsStringUtf8 s(L"abc def ghi äöü jkl ßßß abc2 def2 ghi2 äöü2 ß");
    nsStringUtf8 s2(L"äöü");
    nsStringUtf8 s3(L"äöü2");

    const char* szABC = "abc";

    NS_TEST_BOOL(nsStringUtils::FindSubString(szABC, szABC) == szABC);
    NS_TEST_BOOL(nsStringUtils::FindSubString("abc", "") == nullptr);
    NS_TEST_BOOL(nsStringUtils::FindSubString("abc", nullptr) == nullptr);
    NS_TEST_BOOL(nsStringUtils::FindSubString(nullptr, "abc") == nullptr);
    NS_TEST_BOOL(nsStringUtils::FindSubString("", "abc") == nullptr);

    NS_TEST_BOOL(nsStringUtils::FindSubString(s.GetData(), "abc") == s.GetData());
    NS_TEST_BOOL(nsStringUtils::FindSubString(s.GetData(), "def") == &s.GetData()[4]);
    NS_TEST_BOOL(nsStringUtils::FindSubString(s.GetData(), "ghi") == &s.GetData()[8]);
    NS_TEST_BOOL(nsStringUtils::FindSubString(s.GetData(), s2.GetData()) == &s.GetData()[12]);

    NS_TEST_BOOL(nsStringUtils::FindSubString(s.GetData(), "abc2") == &s.GetData()[30]);
    NS_TEST_BOOL(nsStringUtils::FindSubString(s.GetData(), "def2") == &s.GetData()[35]);
    NS_TEST_BOOL(nsStringUtils::FindSubString(s.GetData(), "ghi2") == &s.GetData()[40]);
    NS_TEST_BOOL(nsStringUtils::FindSubString(s.GetData(), s3.GetData()) == &s.GetData()[45]);

    // substring test
    NS_TEST_BOOL(nsStringUtils::FindSubString(s.GetData(), "abc2", s.GetData() + 34) == &s.GetData()[30]);
    NS_TEST_BOOL(nsStringUtils::FindSubString(s.GetData(), "abc2", s.GetData() + 33) == nullptr);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FindSubString_NoCase")
  {
    nsStringUtf8 s(L"abc def ghi äöü jkl ßßß abc2 def2 ghi2 äöü2 ß");
    nsStringUtf8 s2(L"äÖü");
    nsStringUtf8 s3(L"ÄöÜ2");

    const char* szABC = "abc";

    NS_TEST_BOOL(nsStringUtils::FindSubString_NoCase(szABC, "aBc") == szABC);
    NS_TEST_BOOL(nsStringUtils::FindSubString_NoCase("abc", "") == nullptr);
    NS_TEST_BOOL(nsStringUtils::FindSubString_NoCase("abc", nullptr) == nullptr);
    NS_TEST_BOOL(nsStringUtils::FindSubString_NoCase(nullptr, "abc") == nullptr);
    NS_TEST_BOOL(nsStringUtils::FindSubString_NoCase("", "abc") == nullptr);

    NS_TEST_BOOL(nsStringUtils::FindSubString_NoCase(s.GetData(), "Abc") == s.GetData());
    NS_TEST_BOOL(nsStringUtils::FindSubString_NoCase(s.GetData(), "dEf") == &s.GetData()[4]);
    NS_TEST_BOOL(nsStringUtils::FindSubString_NoCase(s.GetData(), "ghI") == &s.GetData()[8]);
    NS_TEST_BOOL(nsStringUtils::FindSubString_NoCase(s.GetData(), s2.GetData()) == &s.GetData()[12]);

    NS_TEST_BOOL(nsStringUtils::FindSubString_NoCase(s.GetData(), "abC2") == &s.GetData()[30]);
    NS_TEST_BOOL(nsStringUtils::FindSubString_NoCase(s.GetData(), "dEf2") == &s.GetData()[35]);
    NS_TEST_BOOL(nsStringUtils::FindSubString_NoCase(s.GetData(), "Ghi2") == &s.GetData()[40]);
    NS_TEST_BOOL(nsStringUtils::FindSubString_NoCase(s.GetData(), s3.GetData()) == &s.GetData()[45]);

    // substring test
    NS_TEST_BOOL(nsStringUtils::FindSubString_NoCase(s.GetData(), "aBc2", s.GetData() + 34) == &s.GetData()[30]);
    NS_TEST_BOOL(nsStringUtils::FindSubString_NoCase(s.GetData(), "abC2", s.GetData() + 33) == nullptr);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FindLastSubString")
  {
    nsStringUtf8 s(L"abc def ghi äöü jkl ßßß abc2 def2 ghi2 äöü2 ß");
    nsStringUtf8 s2(L"äöü");
    nsStringUtf8 s3(L"äöü2");

    const char* szABC = "abc";

    NS_TEST_BOOL(nsStringUtils::FindLastSubString(szABC, szABC) == szABC);
    NS_TEST_BOOL(nsStringUtils::FindLastSubString("abc", "") == nullptr);
    NS_TEST_BOOL(nsStringUtils::FindLastSubString("abc", nullptr) == nullptr);
    NS_TEST_BOOL(nsStringUtils::FindLastSubString(nullptr, "abc") == nullptr);
    NS_TEST_BOOL(nsStringUtils::FindLastSubString("", "abc") == nullptr);

    NS_TEST_BOOL(nsStringUtils::FindLastSubString(s.GetData(), "abc") == &s.GetData()[30]);
    NS_TEST_BOOL(nsStringUtils::FindLastSubString(s.GetData(), "def") == &s.GetData()[35]);
    NS_TEST_BOOL(nsStringUtils::FindLastSubString(s.GetData(), "ghi") == &s.GetData()[40]);
    NS_TEST_BOOL(nsStringUtils::FindLastSubString(s.GetData(), s2.GetData()) == &s.GetData()[45]);

    // substring test
    NS_TEST_BOOL(nsStringUtils::FindLastSubString(s.GetData(), "abc", nullptr, s.GetData() + 33) == &s.GetData()[30]);
    NS_TEST_BOOL(nsStringUtils::FindLastSubString(s.GetData(), "abc", nullptr, s.GetData() + 32) == &s.GetData()[0]);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FindLastSubString_NoCase")
  {
    nsStringUtf8 s(L"abc def ghi äöü jkl ßßß abc2 def2 ghi2 äöü2 ß");
    nsStringUtf8 s2(L"äÖü");
    nsStringUtf8 s3(L"ÄöÜ2");

    const char* szABC = "abc";

    NS_TEST_BOOL(nsStringUtils::FindLastSubString_NoCase(szABC, "aBC") == szABC);
    NS_TEST_BOOL(nsStringUtils::FindLastSubString_NoCase("abc", "") == nullptr);
    NS_TEST_BOOL(nsStringUtils::FindLastSubString_NoCase("abc", nullptr) == nullptr);
    NS_TEST_BOOL(nsStringUtils::FindLastSubString_NoCase(nullptr, "abc") == nullptr);
    NS_TEST_BOOL(nsStringUtils::FindLastSubString_NoCase("", "abc") == nullptr);

    NS_TEST_BOOL(nsStringUtils::FindLastSubString_NoCase(s.GetData(), "Abc") == &s.GetData()[30]);
    NS_TEST_BOOL(nsStringUtils::FindLastSubString_NoCase(s.GetData(), "dEf") == &s.GetData()[35]);
    NS_TEST_BOOL(nsStringUtils::FindLastSubString_NoCase(s.GetData(), "ghI") == &s.GetData()[40]);
    NS_TEST_BOOL(nsStringUtils::FindLastSubString_NoCase(s.GetData(), s2.GetData()) == &s.GetData()[45]);

    // substring test
    NS_TEST_BOOL(nsStringUtils::FindLastSubString_NoCase(s.GetData(), "ABC", nullptr, s.GetData() + 33) == &s.GetData()[30]);
    NS_TEST_BOOL(nsStringUtils::FindLastSubString_NoCase(s.GetData(), "ABC", nullptr, s.GetData() + 32) == &s.GetData()[0]);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FindWholeWord")
  {
    nsStringUtf8 s(L"mompfhüßß ßßß öäü abcdef abc def");

    NS_TEST_BOOL(nsStringUtils::FindWholeWord(s.GetData(), "abc", nsStringUtils::IsWordDelimiter_English) == &s.GetData()[34]);
    NS_TEST_BOOL(nsStringUtils::FindWholeWord(s.GetData(), "def", nsStringUtils::IsWordDelimiter_English) == &s.GetData()[38]);
    NS_TEST_BOOL(nsStringUtils::FindWholeWord(s.GetData(), "mompfh", nsStringUtils::IsWordDelimiter_English) == &s.GetData()[0]); // ü is not english

    // substring test
    NS_TEST_BOOL(nsStringUtils::FindWholeWord(s.GetData(), "abc", nsStringUtils::IsWordDelimiter_English, s.GetData() + 37) == &s.GetData()[34]);
    NS_TEST_BOOL(nsStringUtils::FindWholeWord(s.GetData(), "abc", nsStringUtils::IsWordDelimiter_English, s.GetData() + 36) == nullptr);
    NS_TEST_BOOL(nsStringUtils::FindWholeWord(s.GetData(), "abc", nsStringUtils::IsWordDelimiter_English, s.GetData() + 30) == s.GetData() + 27);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FindWholeWord_NoCase")
  {
    nsStringUtf8 s(L"mompfhüßß ßßß öäü abcdef abc def");

    NS_TEST_BOOL(nsStringUtils::FindWholeWord_NoCase(s.GetData(), "ABC", nsStringUtils::IsWordDelimiter_English) == &s.GetData()[34]);
    NS_TEST_BOOL(nsStringUtils::FindWholeWord_NoCase(s.GetData(), "DEF", nsStringUtils::IsWordDelimiter_English) == &s.GetData()[38]);
    NS_TEST_BOOL(nsStringUtils::FindWholeWord_NoCase(s.GetData(), "momPFH", nsStringUtils::IsWordDelimiter_English) == &s.GetData()[0]);

    // substring test
    NS_TEST_BOOL(
      nsStringUtils::FindWholeWord_NoCase(s.GetData(), "ABC", nsStringUtils::IsWordDelimiter_English, s.GetData() + 37) == &s.GetData()[34]);
    NS_TEST_BOOL(nsStringUtils::FindWholeWord_NoCase(s.GetData(), "ABC", nsStringUtils::IsWordDelimiter_English, s.GetData() + 36) == nullptr);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FindUIntAtTheEnd")
  {
    nsUInt32 uiTestValue = 0;
    nsUInt32 uiCharactersFromStart = 0;

    NS_TEST_BOOL(nsStringUtils::FindUIntAtTheEnd(nullptr, uiTestValue, &uiCharactersFromStart).Failed());

    nsStringUtf8 noNumberAtTheEnd(L"ThisStringContainsNoNumberAtTheEnd");
    NS_TEST_BOOL(nsStringUtils::FindUIntAtTheEnd(noNumberAtTheEnd.GetData(), uiTestValue, &uiCharactersFromStart).Failed());

    nsStringUtf8 noNumberAtTheEnd2(L"ThisStringContainsNoNumberAtTheEndBut42InBetween");
    NS_TEST_BOOL(nsStringUtils::FindUIntAtTheEnd(noNumberAtTheEnd.GetData(), uiTestValue, &uiCharactersFromStart).Failed());

    nsStringUtf8 aNumberAtTheEnd(L"ThisStringContainsANumberAtTheEnd1");
    NS_TEST_BOOL(nsStringUtils::FindUIntAtTheEnd(aNumberAtTheEnd.GetData(), uiTestValue, &uiCharactersFromStart).Succeeded());
    NS_TEST_INT(uiTestValue, 1);
    NS_TEST_INT(uiCharactersFromStart, aNumberAtTheEnd.GetElementCount() - 1);

    nsStringUtf8 aZeroLeadingNumberAtTheEnd(L"ThisStringContainsANumberAtTheEnd011129");
    NS_TEST_BOOL(nsStringUtils::FindUIntAtTheEnd(aZeroLeadingNumberAtTheEnd.GetData(), uiTestValue, &uiCharactersFromStart).Succeeded());
    NS_TEST_INT(uiTestValue, 11129);
    NS_TEST_INT(uiCharactersFromStart, aZeroLeadingNumberAtTheEnd.GetElementCount() - 6);

    NS_TEST_BOOL(nsStringUtils::FindUIntAtTheEnd(aNumberAtTheEnd.GetData(), uiTestValue, nullptr).Succeeded());
    NS_TEST_INT(uiTestValue, 1);

    nsStringUtf8 twoNumbersInOneString(L"FirstANumber23AndThen42");
    NS_TEST_BOOL(nsStringUtils::FindUIntAtTheEnd(twoNumbersInOneString.GetData(), uiTestValue, &uiCharactersFromStart).Succeeded());
    NS_TEST_INT(uiTestValue, 42);

    nsStringUtf8 onlyANumber(L"55566553");
    NS_TEST_BOOL(nsStringUtils::FindUIntAtTheEnd(onlyANumber.GetData(), uiTestValue, &uiCharactersFromStart).Succeeded());
    NS_TEST_INT(uiTestValue, 55566553);
    NS_TEST_INT(uiCharactersFromStart, 0);
  }


  NS_TEST_BLOCK(nsTestBlock::Enabled, "SkipCharacters")
  {
    nsStringUtf8 s(L"mompf   hüßß ßßß öäü abcdef abc def");
    const char* szEmpty = "";

    NS_TEST_BOOL(nsStringUtils::SkipCharacters(s.GetData(), nsStringUtils::IsWhiteSpace, false) == &s.GetData()[0]);
    NS_TEST_BOOL(nsStringUtils::SkipCharacters(s.GetData(), nsStringUtils::IsWhiteSpace, true) == &s.GetData()[1]);
    NS_TEST_BOOL(nsStringUtils::SkipCharacters(&s.GetData()[5], nsStringUtils::IsWhiteSpace, false) == &s.GetData()[8]);
    NS_TEST_BOOL(nsStringUtils::SkipCharacters(&s.GetData()[5], nsStringUtils::IsWhiteSpace, true) == &s.GetData()[8]);
    NS_TEST_BOOL(nsStringUtils::SkipCharacters(szEmpty, nsStringUtils::IsWhiteSpace, false) == szEmpty);
    NS_TEST_BOOL(nsStringUtils::SkipCharacters(szEmpty, nsStringUtils::IsWhiteSpace, true) == szEmpty);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FindWordEnd")
  {
    nsStringUtf8 s(L"mompf   hüßß ßßß öäü abcdef abc def");
    const char* szEmpty = "";

    NS_TEST_BOOL(nsStringUtils::FindWordEnd(s.GetData(), nsStringUtils::IsWhiteSpace, true) == &s.GetData()[5]);
    NS_TEST_BOOL(nsStringUtils::FindWordEnd(s.GetData(), nsStringUtils::IsWhiteSpace, false) == &s.GetData()[5]);
    NS_TEST_BOOL(nsStringUtils::FindWordEnd(&s.GetData()[5], nsStringUtils::IsWhiteSpace, true) == &s.GetData()[6]);
    NS_TEST_BOOL(nsStringUtils::FindWordEnd(&s.GetData()[5], nsStringUtils::IsWhiteSpace, false) == &s.GetData()[5]);
    NS_TEST_BOOL(nsStringUtils::FindWordEnd(szEmpty, nsStringUtils::IsWhiteSpace, true) == szEmpty);
    NS_TEST_BOOL(nsStringUtils::FindWordEnd(szEmpty, nsStringUtils::IsWhiteSpace, false) == szEmpty);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsWhitespace")
  {
    NS_TEST_BOOL(nsStringUtils::IsWhiteSpace(' '));
    NS_TEST_BOOL(nsStringUtils::IsWhiteSpace('\t'));
    NS_TEST_BOOL(nsStringUtils::IsWhiteSpace('\n'));
    NS_TEST_BOOL(nsStringUtils::IsWhiteSpace('\r'));
    NS_TEST_BOOL(nsStringUtils::IsWhiteSpace('\v'));

    NS_TEST_BOOL(nsStringUtils::IsWhiteSpace('\0') == false);

    for (nsUInt32 i = 33; i < 256; ++i)
    {
      NS_TEST_BOOL(nsStringUtils::IsWhiteSpace(i) == false);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsDecimalDigit / IsHexDigit")
  {
    NS_TEST_BOOL(nsStringUtils::IsDecimalDigit('0'));
    NS_TEST_BOOL(nsStringUtils::IsDecimalDigit('4'));
    NS_TEST_BOOL(nsStringUtils::IsDecimalDigit('9'));
    NS_TEST_BOOL(!nsStringUtils::IsDecimalDigit('/'));
    NS_TEST_BOOL(!nsStringUtils::IsDecimalDigit('A'));

    NS_TEST_BOOL(nsStringUtils::IsHexDigit('0'));
    NS_TEST_BOOL(nsStringUtils::IsHexDigit('4'));
    NS_TEST_BOOL(nsStringUtils::IsHexDigit('9'));
    NS_TEST_BOOL(nsStringUtils::IsHexDigit('A'));
    NS_TEST_BOOL(nsStringUtils::IsHexDigit('E'));
    NS_TEST_BOOL(nsStringUtils::IsHexDigit('a'));
    NS_TEST_BOOL(nsStringUtils::IsHexDigit('f'));
    NS_TEST_BOOL(!nsStringUtils::IsHexDigit('g'));
    NS_TEST_BOOL(!nsStringUtils::IsHexDigit('/'));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsWordDelimiter_English / IsIdentifierDelimiter_C_Code")
  {
    for (nsUInt32 i = 0; i < 256; ++i)
    {
      const bool alpha = (i >= 'a' && i <= 'z');
      const bool alpha2 = (i >= 'A' && i <= 'Z');
      const bool num = (i >= '0' && i <= '9');
      const bool dash = i == '-';
      const bool underscore = i == '_';

      const bool bCode = alpha || alpha2 || num || underscore;
      const bool bWord = bCode || dash;


      NS_TEST_BOOL(nsStringUtils::IsWordDelimiter_English(i) == !bWord);
      NS_TEST_BOOL(nsStringUtils::IsIdentifierDelimiter_C_Code(i) == !bCode);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsValidIdentifierName")
  {
    NS_TEST_BOOL(!nsStringUtils::IsValidIdentifierName(""));
    NS_TEST_BOOL(!nsStringUtils::IsValidIdentifierName("1asdf"));
    NS_TEST_BOOL(!nsStringUtils::IsValidIdentifierName("as df"));
    NS_TEST_BOOL(!nsStringUtils::IsValidIdentifierName("asdf!"));

    NS_TEST_BOOL(nsStringUtils::IsValidIdentifierName("asdf1"));
    NS_TEST_BOOL(nsStringUtils::IsValidIdentifierName("_asdf"));
  }
}

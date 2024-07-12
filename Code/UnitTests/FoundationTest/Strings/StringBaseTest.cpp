#include <FoundationTest/FoundationTestPCH.h>

// NOTE: always save as Unicode UTF-8 with signature

#include <Foundation/Strings/String.h>

NS_CREATE_SIMPLE_TEST(Strings, StringBase)
{
  // These tests need not be very through, as nsStringBase only passes through to nsStringUtil
  // which has been tested elsewhere already.
  // Here it is only assured that nsStringBases passes its own pointers properly through,
  // such that the nsStringUtil functions are called correctly.

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsEmpty")
  {
    nsStringView it(nullptr);
    NS_TEST_BOOL(it.IsEmpty());

    nsStringView it2("");
    NS_TEST_BOOL(it2.IsEmpty());

    nsStringView it3(nullptr, nullptr);
    NS_TEST_BOOL(it3.IsEmpty());

    const char* sz = "abcdef";

    nsStringView it4(sz, sz);
    NS_TEST_BOOL(it4.IsEmpty());

    nsStringView it5(sz, sz + 1);
    NS_TEST_BOOL(!it5.IsEmpty());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "StartsWith")
  {
    const char* sz = "abcdef";
    nsStringView it(sz);

    NS_TEST_BOOL(it.StartsWith("abc"));
    NS_TEST_BOOL(it.StartsWith("abcdef"));
    NS_TEST_BOOL(it.StartsWith("")); // empty strings always return true

    nsStringView it2(sz + 3);

    NS_TEST_BOOL(it2.StartsWith("def"));
    NS_TEST_BOOL(it2.StartsWith(""));

    nsStringView it3(sz + 2, sz + 4);
    it3.SetStartPosition(sz + 3);

    NS_TEST_BOOL(it3.StartsWith("d"));
    NS_TEST_BOOL(!it3.StartsWith("de"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "StartsWith_NoCase")
  {
    const char* sz = "abcdef";
    nsStringView it(sz);

    NS_TEST_BOOL(it.StartsWith_NoCase("ABC"));
    NS_TEST_BOOL(it.StartsWith_NoCase("abcDEF"));
    NS_TEST_BOOL(it.StartsWith_NoCase("")); // empty strings always return true

    nsStringView it2(sz + 3);

    NS_TEST_BOOL(it2.StartsWith_NoCase("DEF"));
    NS_TEST_BOOL(it2.StartsWith_NoCase(""));

    nsStringView it3(sz + 2, sz + 4);
    it3.SetStartPosition(sz + 3);

    NS_TEST_BOOL(it3.StartsWith_NoCase("D"));
    NS_TEST_BOOL(!it3.StartsWith_NoCase("DE"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "EndsWith")
  {
    const char* sz = "abcdef";
    nsStringView it(sz);

    NS_TEST_BOOL(it.EndsWith("def"));
    NS_TEST_BOOL(it.EndsWith("abcdef"));
    NS_TEST_BOOL(it.EndsWith("")); // empty strings always return true

    nsStringView it2(sz + 3);

    NS_TEST_BOOL(it2.EndsWith("def"));
    NS_TEST_BOOL(it2.EndsWith(""));

    nsStringView it3(sz + 2, sz + 4);
    it3.SetStartPosition(sz + 3);

    NS_TEST_BOOL(it3.EndsWith("d"));
    NS_TEST_BOOL(!it3.EndsWith("cd"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "EndsWith_NoCase")
  {
    const char* sz = "ABCDEF";
    nsStringView it(sz);

    NS_TEST_BOOL(it.EndsWith_NoCase("def"));
    NS_TEST_BOOL(it.EndsWith_NoCase("abcdef"));
    NS_TEST_BOOL(it.EndsWith_NoCase("")); // empty strings always return true

    nsStringView it2(sz + 3);

    NS_TEST_BOOL(it2.EndsWith_NoCase("def"));
    NS_TEST_BOOL(it2.EndsWith_NoCase(""));

    nsStringView it3(sz + 2, sz + 4);
    it3.SetStartPosition(sz + 3);

    NS_TEST_BOOL(it3.EndsWith_NoCase("d"));
    NS_TEST_BOOL(!it3.EndsWith_NoCase("cd"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FindSubString")
  {
    const char* sz = "abcdef";
    nsStringView it(sz);

    NS_TEST_BOOL(it.FindSubString("abcdef") == sz);
    NS_TEST_BOOL(it.FindSubString("abc") == sz);
    NS_TEST_BOOL(it.FindSubString("def") == sz + 3);
    NS_TEST_BOOL(it.FindSubString("cd") == sz + 2);
    NS_TEST_BOOL(it.FindSubString("") == nullptr);
    NS_TEST_BOOL(it.FindSubString(nullptr) == nullptr);
    NS_TEST_BOOL(it.FindSubString("g") == nullptr);

    NS_TEST_BOOL(it.FindSubString("abcdef", sz) == sz);
    NS_TEST_BOOL(it.FindSubString("abcdef", sz + 1) == nullptr);
    NS_TEST_BOOL(it.FindSubString("def", sz + 2) == sz + 3);
    NS_TEST_BOOL(it.FindSubString("def", sz + 3) == sz + 3);
    NS_TEST_BOOL(it.FindSubString("def", sz + 4) == nullptr);
    NS_TEST_BOOL(it.FindSubString("", sz + 3) == nullptr);

    nsStringView it2(sz + 1, sz + 5);
    it2.SetStartPosition(sz + 2);

    NS_TEST_BOOL(it2.FindSubString("abcdef") == nullptr);
    NS_TEST_BOOL(it2.FindSubString("abc") == nullptr);
    NS_TEST_BOOL(it2.FindSubString("de") == sz + 3);
    NS_TEST_BOOL(it2.FindSubString("cd") == sz + 2);
    NS_TEST_BOOL(it2.FindSubString("") == nullptr);
    NS_TEST_BOOL(it2.FindSubString(nullptr) == nullptr);
    NS_TEST_BOOL(it2.FindSubString("g") == nullptr);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FindSubString_NoCase")
  {
    const char* sz = "ABCDEF";
    nsStringView it(sz);

    NS_TEST_BOOL(it.FindSubString_NoCase("abcdef") == sz);
    NS_TEST_BOOL(it.FindSubString_NoCase("abc") == sz);
    NS_TEST_BOOL(it.FindSubString_NoCase("def") == sz + 3);
    NS_TEST_BOOL(it.FindSubString_NoCase("cd") == sz + 2);
    NS_TEST_BOOL(it.FindSubString_NoCase("") == nullptr);
    NS_TEST_BOOL(it.FindSubString_NoCase(nullptr) == nullptr);
    NS_TEST_BOOL(it.FindSubString_NoCase("g") == nullptr);

    NS_TEST_BOOL(it.FindSubString_NoCase("abcdef", sz) == sz);
    NS_TEST_BOOL(it.FindSubString_NoCase("abcdef", sz + 1) == nullptr);
    NS_TEST_BOOL(it.FindSubString_NoCase("def", sz + 2) == sz + 3);
    NS_TEST_BOOL(it.FindSubString_NoCase("def", sz + 3) == sz + 3);
    NS_TEST_BOOL(it.FindSubString_NoCase("def", sz + 4) == nullptr);
    NS_TEST_BOOL(it.FindSubString_NoCase("", sz + 3) == nullptr);


    nsStringView it2(sz + 1, sz + 5);
    it2.SetStartPosition(sz + 2);

    NS_TEST_BOOL(it2.FindSubString_NoCase("abcdef") == nullptr);
    NS_TEST_BOOL(it2.FindSubString_NoCase("abc") == nullptr);
    NS_TEST_BOOL(it2.FindSubString_NoCase("de") == sz + 3);
    NS_TEST_BOOL(it2.FindSubString_NoCase("cd") == sz + 2);
    NS_TEST_BOOL(it2.FindSubString_NoCase("") == nullptr);
    NS_TEST_BOOL(it2.FindSubString_NoCase(nullptr) == nullptr);
    NS_TEST_BOOL(it2.FindSubString_NoCase("g") == nullptr);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FindLastSubString")
  {
    const char* sz = "abcdef";
    nsStringView it(sz);

    NS_TEST_BOOL(it.FindLastSubString("abcdef") == sz);
    NS_TEST_BOOL(it.FindLastSubString("abc") == sz);
    NS_TEST_BOOL(it.FindLastSubString("def") == sz + 3);
    NS_TEST_BOOL(it.FindLastSubString("cd") == sz + 2);
    NS_TEST_BOOL(it.FindLastSubString("") == nullptr);
    NS_TEST_BOOL(it.FindLastSubString(nullptr) == nullptr);
    NS_TEST_BOOL(it.FindLastSubString("g") == nullptr);

    nsStringView it2(sz + 1, sz + 5);
    it2.SetStartPosition(sz + 2);

    NS_TEST_BOOL(it2.FindLastSubString("abcdef") == nullptr);
    NS_TEST_BOOL(it2.FindLastSubString("abc") == nullptr);
    NS_TEST_BOOL(it2.FindLastSubString("de") == sz + 3);
    NS_TEST_BOOL(it2.FindLastSubString("cd") == sz + 2);
    NS_TEST_BOOL(it2.FindLastSubString("") == nullptr);
    NS_TEST_BOOL(it2.FindLastSubString(nullptr) == nullptr);
    NS_TEST_BOOL(it2.FindLastSubString("g") == nullptr);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FindLastSubString_NoCase")
  {
    const char* sz = "ABCDEF";
    nsStringView it(sz);

    NS_TEST_BOOL(it.FindLastSubString_NoCase("abcdef") == sz);
    NS_TEST_BOOL(it.FindLastSubString_NoCase("abc") == sz);
    NS_TEST_BOOL(it.FindLastSubString_NoCase("def") == sz + 3);
    NS_TEST_BOOL(it.FindLastSubString_NoCase("cd") == sz + 2);
    NS_TEST_BOOL(it.FindLastSubString_NoCase("") == nullptr);
    NS_TEST_BOOL(it.FindLastSubString_NoCase(nullptr) == nullptr);
    NS_TEST_BOOL(it.FindLastSubString_NoCase("g") == nullptr);

    nsStringView it2(sz + 1, sz + 5);
    it2.SetStartPosition(sz + 2);

    NS_TEST_BOOL(it2.FindLastSubString_NoCase("abcdef") == nullptr);
    NS_TEST_BOOL(it2.FindLastSubString_NoCase("abc") == nullptr);
    NS_TEST_BOOL(it2.FindLastSubString_NoCase("de") == sz + 3);
    NS_TEST_BOOL(it2.FindLastSubString_NoCase("cd") == sz + 2);
    NS_TEST_BOOL(it2.FindLastSubString_NoCase("") == nullptr);
    NS_TEST_BOOL(it2.FindLastSubString_NoCase(nullptr) == nullptr);
    NS_TEST_BOOL(it2.FindLastSubString_NoCase("g") == nullptr);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Compare")
  {
    const char* sz = "abcdef";
    nsStringView it(sz);

    NS_TEST_BOOL(it.Compare("abcdef") == 0);
    NS_TEST_BOOL(it.Compare("abcde") > 0);
    NS_TEST_BOOL(it.Compare("abcdefg") < 0);

    nsStringView it2(sz + 2, sz + 5);
    it2.SetStartPosition(sz + 3);

    NS_TEST_BOOL(it2.Compare("de") == 0);
    NS_TEST_BOOL(it2.Compare("def") < 0);
    NS_TEST_BOOL(it2.Compare("d") > 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Compare_NoCase")
  {
    const char* sz = "ABCDEF";
    nsStringView it(sz);

    NS_TEST_BOOL(it.Compare_NoCase("abcdef") == 0);
    NS_TEST_BOOL(it.Compare_NoCase("abcde") > 0);
    NS_TEST_BOOL(it.Compare_NoCase("abcdefg") < 0);

    nsStringView it2(sz + 2, sz + 5);
    it2.SetStartPosition(sz + 3);

    NS_TEST_BOOL(it2.Compare_NoCase("de") == 0);
    NS_TEST_BOOL(it2.Compare_NoCase("def") < 0);
    NS_TEST_BOOL(it2.Compare_NoCase("d") > 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CompareN")
  {
    const char* sz = "abcdef";
    nsStringView it(sz);

    NS_TEST_BOOL(it.CompareN("abc", 3) == 0);
    NS_TEST_BOOL(it.CompareN("abcde", 6) > 0);
    NS_TEST_BOOL(it.CompareN("abcg", 3) == 0);

    nsStringView it2(sz + 2, sz + 5);

    NS_TEST_BOOL(it2.CompareN("cd", 2) == 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CompareN_NoCase")
  {
    const char* sz = "ABCDEF";
    nsStringView it(sz);

    NS_TEST_BOOL(it.CompareN_NoCase("abc", 3) == 0);
    NS_TEST_BOOL(it.CompareN_NoCase("abcde", 6) > 0);
    NS_TEST_BOOL(it.CompareN_NoCase("abcg", 3) == 0);

    nsStringView it2(sz + 2, sz + 5);

    NS_TEST_BOOL(it2.CompareN_NoCase("cd", 2) == 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsEqual")
  {
    const char* sz = "abcdef";
    nsStringView it(sz);

    NS_TEST_BOOL(it.IsEqual("abcdef"));
    NS_TEST_BOOL(!it.IsEqual("abcde"));
    NS_TEST_BOOL(!it.IsEqual("abcdefg"));

    nsStringView it2(sz + 1, sz + 5);
    it2.SetStartPosition(sz + 2);

    NS_TEST_BOOL(it2.IsEqual("cde"));
    NS_TEST_BOOL(!it2.IsEqual("bcde"));
    NS_TEST_BOOL(!it2.IsEqual("cdef"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsEqual_NoCase")
  {
    const char* sz = "ABCDEF";
    nsStringView it(sz);

    NS_TEST_BOOL(it.IsEqual_NoCase("abcdef"));
    NS_TEST_BOOL(!it.IsEqual_NoCase("abcde"));
    NS_TEST_BOOL(!it.IsEqual_NoCase("abcdefg"));

    nsStringView it2(sz + 1, sz + 5);
    it2.SetStartPosition(sz + 2);

    NS_TEST_BOOL(it2.IsEqual_NoCase("cde"));
    NS_TEST_BOOL(!it2.IsEqual_NoCase("bcde"));
    NS_TEST_BOOL(!it2.IsEqual_NoCase("cdef"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsEqualN")
  {
    const char* sz = "abcdef";
    nsStringView it(sz);

    NS_TEST_BOOL(it.IsEqualN("abcGHI", 3));
    NS_TEST_BOOL(!it.IsEqualN("abcGHI", 4));

    nsStringView it2(sz + 1, sz + 5);
    it2.SetStartPosition(sz + 2);

    NS_TEST_BOOL(it2.IsEqualN("cdeZX", 3));
    NS_TEST_BOOL(!it2.IsEqualN("cdeZX", 4));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsEqualN_NoCase")
  {
    const char* sz = "ABCDEF";
    nsStringView it(sz);

    NS_TEST_BOOL(it.IsEqualN_NoCase("abcGHI", 3));
    NS_TEST_BOOL(!it.IsEqualN_NoCase("abcGHI", 4));

    nsStringView it2(sz + 1, sz + 5);
    it2.SetStartPosition(sz + 2);

    NS_TEST_BOOL(it2.IsEqualN_NoCase("cdeZX", 3));
    NS_TEST_BOOL(!it2.IsEqualN_NoCase("cdeZX", 4));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator==/!=")
  {
    const char* sz = "abcdef";
    const char* sz2 = "blabla";
    nsStringView it(sz);
    nsStringView it2(sz);
    nsStringView it3(sz2);

    NS_TEST_BOOL(it == sz);
    NS_TEST_BOOL(sz == it);
    NS_TEST_BOOL(it == "abcdef");
    NS_TEST_BOOL("abcdef" == it);
    NS_TEST_BOOL(it == it);
    NS_TEST_BOOL(it == it2);

    NS_TEST_BOOL(it != sz2);
    NS_TEST_BOOL(sz2 != it);
    NS_TEST_BOOL(it != "blabla");
    NS_TEST_BOOL("blabla" != it);
    NS_TEST_BOOL(it != it3);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "substring operator ==/!=/</>/<=/>=")
  {

    const char* sz1 = "aaabbbcccddd";
    const char* sz2 = "aaabbbdddeee";

    nsStringView it1(sz1 + 3, sz1 + 6);
    nsStringView it2(sz2 + 3, sz2 + 6);

    NS_TEST_BOOL(it1 == it1);
    NS_TEST_BOOL(it2 == it2);

    NS_TEST_BOOL(it1 == it2);
    NS_TEST_BOOL(!(it1 != it2));
    NS_TEST_BOOL(!(it1 < it2));
    NS_TEST_BOOL(!(it1 > it2));
    NS_TEST_BOOL(it1 <= it2);
    NS_TEST_BOOL(it1 >= it2);

    it1 = nsStringView(sz1 + 3, sz1 + 7);
    it2 = nsStringView(sz2 + 3, sz2 + 7);

    NS_TEST_BOOL(it1 == it1);
    NS_TEST_BOOL(it2 == it2);

    NS_TEST_BOOL(it1 != it2);
    NS_TEST_BOOL(!(it1 == it2));

    NS_TEST_BOOL(it1 < it2);
    NS_TEST_BOOL(!(it1 > it2));
    NS_TEST_BOOL(it1 <= it2);
    NS_TEST_BOOL(!(it1 >= it2));

    NS_TEST_BOOL(it2 > it1);
    NS_TEST_BOOL(!(it2 < it1));
    NS_TEST_BOOL(it2 >= it1);
    NS_TEST_BOOL(!(it2 <= it1));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator</>")
  {
    const char* sz = "abcdef";
    const char* sz2 = "abcdefg";
    nsStringView it(sz);
    nsStringView it2(sz2);

    NS_TEST_BOOL(it < sz2);
    NS_TEST_BOOL(sz < it2);
    NS_TEST_BOOL(it < it2);

    NS_TEST_BOOL(sz2 > it);
    NS_TEST_BOOL(it2 > sz);
    NS_TEST_BOOL(it2 > it);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator<=/>=")
  {
    {
      const char* sz = "abcdef";
      const char* sz2 = "abcdefg";
      nsStringView it(sz);
      nsStringView it2(sz2);

      NS_TEST_BOOL(it <= sz2);
      NS_TEST_BOOL(sz <= it2);
      NS_TEST_BOOL(it <= it2);

      NS_TEST_BOOL(sz2 >= it);
      NS_TEST_BOOL(it2 >= sz);
      NS_TEST_BOOL(it2 >= it);
    }

    {
      const char* sz = "abcdef";
      const char* sz2 = "abcdef";
      nsStringView it(sz);
      nsStringView it2(sz2);

      NS_TEST_BOOL(it <= sz2);
      NS_TEST_BOOL(sz <= it2);
      NS_TEST_BOOL(it <= it2);

      NS_TEST_BOOL(sz2 >= it);
      NS_TEST_BOOL(it2 >= sz);
      NS_TEST_BOOL(it2 >= it);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FindWholeWord")
  {
    nsStringUtf8 s(L"abc def mompfhüßß ßßß öäü abcdef abc def abc def");
    nsStringView it(s.GetData() + 8, s.GetData() + s.GetElementCount() - 8);
    nsStringView it2(s.GetData() + 8, s.GetData() + s.GetElementCount());

    NS_TEST_BOOL(it.FindWholeWord("abc", nsStringUtils::IsWordDelimiter_English) == &it.GetStartPointer()[34]);
    NS_TEST_BOOL(it.FindWholeWord("def", nsStringUtils::IsWordDelimiter_English) == &it.GetStartPointer()[38]);
    NS_TEST_BOOL(
      it.FindWholeWord("mompfh", nsStringUtils::IsWordDelimiter_English) == &it.GetStartPointer()[0]); // ü is not English (thus a delimiter)

    NS_TEST_BOOL(it.FindWholeWord("abc", nsStringUtils::IsWordDelimiter_English, it.GetStartPointer() + 34) == &it.GetStartPointer()[34]);
    NS_TEST_BOOL(it.FindWholeWord("abc", nsStringUtils::IsWordDelimiter_English, it.GetStartPointer() + 35) == nullptr);

    NS_TEST_BOOL(it2.FindWholeWord("abc", nsStringUtils::IsWordDelimiter_English, it.GetStartPointer() + 34) == &it.GetStartPointer()[34]);
    NS_TEST_BOOL(it2.FindWholeWord("abc", nsStringUtils::IsWordDelimiter_English, it.GetStartPointer() + 35) == &it.GetStartPointer()[42]);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FindWholeWord_NoCase")
  {
    nsStringUtf8 s(L"abc def mompfhüßß ßßß öäü abcdef abc def abc def");
    nsStringView it(s.GetData() + 8, s.GetData() + s.GetElementCount() - 8);
    nsStringView it2(s.GetData() + 8, s.GetData() + s.GetElementCount());

    NS_TEST_BOOL(it.FindWholeWord_NoCase("ABC", nsStringUtils::IsWordDelimiter_English) == &it.GetStartPointer()[34]);
    NS_TEST_BOOL(it.FindWholeWord_NoCase("DEF", nsStringUtils::IsWordDelimiter_English) == &it.GetStartPointer()[38]);
    NS_TEST_BOOL(it.FindWholeWord_NoCase("momPFH", nsStringUtils::IsWordDelimiter_English) == &it.GetStartPointer()[0]);

    NS_TEST_BOOL(it.FindWholeWord_NoCase("ABc", nsStringUtils::IsWordDelimiter_English, it.GetStartPointer() + 34) == &it.GetStartPointer()[34]);
    NS_TEST_BOOL(it.FindWholeWord_NoCase("ABc", nsStringUtils::IsWordDelimiter_English, it.GetStartPointer() + 35) == nullptr);

    NS_TEST_BOOL(it2.FindWholeWord_NoCase("ABc", nsStringUtils::IsWordDelimiter_English, it.GetStartPointer() + 34) == &it.GetStartPointer()[34]);
    NS_TEST_BOOL(it2.FindWholeWord_NoCase("ABc", nsStringUtils::IsWordDelimiter_English, it.GetStartPointer() + 35) == &it.GetStartPointer()[42]);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ComputeCharacterPosition")
  {
    const wchar_t* sz = L"mompfhüßß ßßß öäü abcdef abc def abc def";
    nsStringBuilder s(sz);

    NS_TEST_STRING(s.ComputeCharacterPosition(14), nsStringUtf8(L"öäü abcdef abc def abc def").GetData());
  }
}

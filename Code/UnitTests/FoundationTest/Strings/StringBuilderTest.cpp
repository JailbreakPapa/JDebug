#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Memory/CommonAllocators.h>
#include <Foundation/Strings/String.h>

// this file takes ages to compile in a Release build
// since we don't care for runtime performance, just disable all optimizations
#pragma optimize("", off)

NS_CREATE_SIMPLE_TEST(Strings, StringBuilder)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor(empty)")
  {
    nsStringBuilder s;

    NS_TEST_BOOL(s.IsEmpty());
    NS_TEST_INT(s.GetCharacterCount(), 0);
    NS_TEST_INT(s.GetElementCount(), 0);
    NS_TEST_BOOL(s == "");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor(Utf8)")
  {
    nsStringUtf8 sUtf8(L"abc äöü € def");
    nsStringBuilder s(sUtf8.GetData());

    NS_TEST_BOOL(s.GetData() != sUtf8.GetData());
    NS_TEST_BOOL(s == sUtf8.GetData());
    NS_TEST_INT(s.GetElementCount(), 18);
    NS_TEST_INT(s.GetCharacterCount(), 13);

    nsStringBuilder s2("test test");

    NS_TEST_BOOL(s2 == "test test");
    NS_TEST_INT(s2.GetElementCount(), 9);
    NS_TEST_INT(s2.GetCharacterCount(), 9);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor(wchar_t)")
  {
    nsStringUtf8 sUtf8(L"abc äöü € def");
    nsStringBuilder s(L"abc äöü € def");

    NS_TEST_BOOL(s == sUtf8.GetData());
    NS_TEST_INT(s.GetElementCount(), 18);
    NS_TEST_INT(s.GetCharacterCount(), 13);

    nsStringBuilder s2(L"test test");

    NS_TEST_BOOL(s2 == "test test");
    NS_TEST_INT(s2.GetElementCount(), 9);
    NS_TEST_INT(s2.GetCharacterCount(), 9);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor(copy)")
  {
    nsStringUtf8 sUtf8(L"abc äöü € def");
    nsStringBuilder s(L"abc äöü € def");
    nsStringBuilder s2(s);

    NS_TEST_BOOL(s2 == sUtf8.GetData());
    NS_TEST_INT(s2.GetElementCount(), 18);
    NS_TEST_INT(s2.GetCharacterCount(), 13);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor(StringView)")
  {
    nsStringUtf8 sUtf8(L"abc äöü € def");

    nsStringView it(sUtf8.GetData() + 2, sUtf8.GetData() + 8);

    nsStringBuilder s(it);

    NS_TEST_INT(s.GetElementCount(), 6);
    NS_TEST_INT(s.GetCharacterCount(), 4);
    NS_TEST_BOOL(s == nsStringUtf8(L"c äö").GetData());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor(multiple)")
  {
    nsStringUtf8 sUtf8(L"⺅⻩⽇⿕〄㈷㑧䆴ظؼݻ༺");
    nsStringUtf8 sUtf2(L"⺅⻩⽇⿕〄㈷㑧䆴ظؼݻ༺⺅⻩⽇⿕〄㈷㑧䆴ظؼݻ༺⺅⻩⽇⿕〄㈷㑧䆴ظؼݻ༺⺅⻩⽇⿕〄㈷㑧䆴ظؼݻ༺⺅⻩⽇⿕〄㈷㑧䆴ظؼݻ༺⺅⻩⽇⿕〄㈷㑧䆴ظؼݻ༺");

    nsStringBuilder sb(sUtf8.GetData(), sUtf8.GetData(), sUtf8.GetData(), sUtf8.GetData(), sUtf8.GetData(), sUtf8.GetData());

    NS_TEST_STRING(sb.GetData(), sUtf2.GetData());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator=(Utf8)")
  {
    nsStringUtf8 sUtf8(L"abc äöü € def");
    nsStringBuilder s("bla");
    s = sUtf8.GetData();

    NS_TEST_BOOL(s.GetData() != sUtf8.GetData());
    NS_TEST_BOOL(s == sUtf8.GetData());
    NS_TEST_INT(s.GetElementCount(), 18);
    NS_TEST_INT(s.GetCharacterCount(), 13);

    nsStringBuilder s2("bla");
    s2 = "test test";

    NS_TEST_BOOL(s2 == "test test");
    NS_TEST_INT(s2.GetElementCount(), 9);
    NS_TEST_INT(s2.GetCharacterCount(), 9);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator=(wchar_t)")
  {
    nsStringUtf8 sUtf8(L"abc äöü € def");
    nsStringBuilder s("bla");
    s = L"abc äöü € def";

    NS_TEST_BOOL(s == sUtf8.GetData());
    NS_TEST_INT(s.GetElementCount(), 18);
    NS_TEST_INT(s.GetCharacterCount(), 13);

    nsStringBuilder s2("bla");
    s2 = L"test test";

    NS_TEST_BOOL(s2 == "test test");
    NS_TEST_INT(s2.GetElementCount(), 9);
    NS_TEST_INT(s2.GetCharacterCount(), 9);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator=(copy)")
  {
    nsStringUtf8 sUtf8(L"abc äöü € def");
    nsStringBuilder s(L"abc äöü € def");
    nsStringBuilder s2;
    s2 = s;

    NS_TEST_BOOL(s2 == sUtf8.GetData());
    NS_TEST_INT(s2.GetElementCount(), 18);
    NS_TEST_INT(s2.GetCharacterCount(), 13);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator=(StringView)")
  {
    nsStringBuilder s("abcdefghi");
    nsStringView it(s.GetData() + 2, s.GetData() + 8);
    it.SetStartPosition(s.GetData() + 3);

    s = it;

    NS_TEST_BOOL(s == "defgh");
    NS_TEST_INT(s.GetElementCount(), 5);
    NS_TEST_INT(s.GetCharacterCount(), 5);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "convert to nsStringView")
  {
    nsStringBuilder s(L"aölsdföasld");
    nsStringBuilder tmp;

    nsStringView sv = s;

    NS_TEST_STRING(sv.GetData(tmp), nsStringUtf8(L"aölsdföasld").GetData());
    NS_TEST_BOOL(sv == nsStringUtf8(L"aölsdföasld").GetData());

    s = "abcdef";

    NS_TEST_STRING(sv.GetStartPointer(), "abcdef");
    NS_TEST_BOOL(sv == "abcdef");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Clear")
  {
    nsStringBuilder s(L"abc äöü € def");

    NS_TEST_BOOL(!s.IsEmpty());

    s.Clear();
    NS_TEST_BOOL(s.IsEmpty());
    NS_TEST_INT(s.GetElementCount(), 0);
    NS_TEST_INT(s.GetCharacterCount(), 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetElementCount / GetCharacterCount")
  {
    nsStringBuilder s(L"abc äöü € def");

    NS_TEST_INT(s.GetElementCount(), 18);
    NS_TEST_INT(s.GetCharacterCount(), 13);

    s = "abc";

    NS_TEST_INT(s.GetElementCount(), 3);
    NS_TEST_INT(s.GetCharacterCount(), 3);

    s = L"Hällo! I love €";

    NS_TEST_INT(s.GetElementCount(), 18);
    NS_TEST_INT(s.GetCharacterCount(), 15);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Append(single unicode char)")
  {
    nsStringUtf32 u32(L"äöüß");

    nsStringBuilder s("abc");
    NS_TEST_INT(s.GetCharacterCount(), 3);
    s.Append(u32.GetData()[0]);
    NS_TEST_INT(s.GetCharacterCount(), 4);

    NS_TEST_BOOL(s == nsStringUtf8(L"abcä").GetData());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Prepend(single unicode char)")
  {
    nsStringUtf32 u32(L"äöüß");

    nsStringBuilder s("abc");
    NS_TEST_INT(s.GetCharacterCount(), 3);
    s.Prepend(u32.GetData()[0]);
    NS_TEST_INT(s.GetCharacterCount(), 4);

    NS_TEST_BOOL(s == nsStringUtf8(L"äabc").GetData());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Append(char)")
  {
    nsStringBuilder s("abc");
    NS_TEST_INT(s.GetCharacterCount(), 3);
    s.Append("de", "fg", "hi", nsStringUtf8(L"öä").GetData(), "jk", nsStringUtf8(L"ü€").GetData());
    NS_TEST_INT(s.GetCharacterCount(), 15);

    NS_TEST_BOOL(s == nsStringUtf8(L"abcdefghiöäjkü€").GetData());

    s = "pups";
    s.Append(nullptr, "b", nullptr, "d", nullptr, nsStringUtf8(L"ü€").GetData());
    NS_TEST_BOOL(s == nsStringUtf8(L"pupsbdü€").GetData());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Append(wchar_t)")
  {
    nsStringBuilder s("abc");
    NS_TEST_INT(s.GetCharacterCount(), 3);
    s.Append(L"de", L"fg", L"hi", L"öä", L"jk", L"ü€");
    NS_TEST_INT(s.GetCharacterCount(), 15);

    NS_TEST_BOOL(s == nsStringUtf8(L"abcdefghiöäjkü€").GetData());

    s = "pups";
    s.Append(nullptr, L"b", nullptr, L"d", nullptr, L"ü€");
    NS_TEST_BOOL(s == nsStringUtf8(L"pupsbdü€").GetData());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Append(multiple)")
  {
    nsStringUtf8 sUtf8(L"⺅⻩⽇⿕〄㈷㑧䆴ظؼݻ༺");
    nsStringUtf8 sUtf2(L"Test⺅⻩⽇⿕〄㈷㑧䆴ظؼݻ༺⺅⻩⽇⿕〄㈷㑧䆴ظؼݻ༺⺅⻩⽇⿕〄㈷㑧䆴ظؼݻ༺⺅⻩⽇⿕〄㈷㑧䆴ظؼݻ༺⺅⻩⽇⿕〄㈷㑧䆴ظؼݻ༺Test2");

    nsStringBuilder sb("Test");
    sb.Append(sUtf8.GetData(), sUtf8.GetData(), sUtf8.GetData(), sUtf8.GetData(), sUtf8.GetData(), "Test2");

    NS_TEST_STRING(sb.GetData(), sUtf2.GetData());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Set(multiple)")
  {
    nsStringUtf8 sUtf8(L"⺅⻩⽇⿕〄㈷㑧䆴ظؼݻ༺");
    nsStringUtf8 sUtf2(L"⺅⻩⽇⿕〄㈷㑧䆴ظؼݻ༺⺅⻩⽇⿕〄㈷㑧䆴ظؼݻ༺⺅⻩⽇⿕〄㈷㑧䆴ظؼݻ༺⺅⻩⽇⿕〄㈷㑧䆴ظؼݻ༺⺅⻩⽇⿕〄㈷㑧䆴ظؼݻ༺Test2");

    nsStringBuilder sb("Test");
    sb.Set(sUtf8.GetData(), sUtf8.GetData(), sUtf8.GetData(), sUtf8.GetData(), sUtf8.GetData(), "Test2");

    NS_TEST_STRING(sb.GetData(), sUtf2.GetData());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AppendFormat")
  {
    nsStringBuilder s("abc");
    s.AppendFormat("Test{0}{1}{2}", 42, "foo", nsStringUtf8(L"bär").GetData());

    NS_TEST_BOOL(s == nsStringUtf8(L"abcTest42foobär").GetData());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Prepend(char)")
  {
    nsStringBuilder s("abc");
    s.Prepend("de", "fg", "hi", nsStringUtf8(L"öä").GetData(), "jk", nsStringUtf8(L"ü€").GetData());

    NS_TEST_BOOL(s == nsStringUtf8(L"defghiöäjkü€abc").GetData());

    s = "pups";
    s.Prepend(nullptr, "b", nullptr, "d", nullptr, nsStringUtf8(L"ü€").GetData());
    NS_TEST_BOOL(s == nsStringUtf8(L"bdü€pups").GetData());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Prepend(wchar_t)")
  {
    nsStringBuilder s("abc");
    s.Prepend(L"de", L"fg", L"hi", L"öä", L"jk", L"ü€");

    NS_TEST_BOOL(s == nsStringUtf8(L"defghiöäjkü€abc").GetData());

    s = "pups";
    s.Prepend(nullptr, L"b", nullptr, L"d", nullptr, L"ü€");
    NS_TEST_BOOL(s == nsStringUtf8(L"bdü€pups").GetData());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "PrependFormat")
  {
    nsStringBuilder s("abc");
    s.PrependFormat("Test{0}{1}{2}", 42, "foo", nsStringUtf8(L"bär").GetData());

    NS_TEST_BOOL(s == nsStringUtf8(L"Test42foobärabc").GetData());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Printf")
  {
    nsStringBuilder s("abc");
    s.SetPrintf("Test%i%s%s", 42, "foo", nsStringUtf8(L"bär").GetData());

    NS_TEST_BOOL(s == nsStringUtf8(L"Test42foobär").GetData());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetFormat")
  {
    nsStringBuilder s("abc");
    s.SetFormat("Test{0}{1}{2}", 42, "foo", nsStringUtf8(L"bär").GetData());
    NS_TEST_BOOL(s == nsStringUtf8(L"Test42foobär").GetData());

    s.SetFormat("%%процент{}%%", 100);
    NS_TEST_BOOL(s == nsStringUtf8(L"%процент100%").GetData());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ToUpper")
  {
    nsStringBuilder s(L"abcdefghijklmnopqrstuvwxyzäöü€ß");
    s.ToUpper();
    NS_TEST_BOOL(s == nsStringUtf8(L"ABCDEFGHIJKLMNOPQRSTUVWXYZÄÖÜ€ß").GetData());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ToLower")
  {
    nsStringBuilder s(L"ABCDEFGHIJKLMNOPQRSTUVWXYZÄÖÜ€ß");
    s.ToLower();
    NS_TEST_BOOL(s == nsStringUtf8(L"abcdefghijklmnopqrstuvwxyzäöü€ß").GetData());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Shrink")
  {
    nsStringBuilder s(L"abcdefghijklmnopqrstuvwxyzäöü€ß");
    s.Shrink(5, 3);

    NS_TEST_BOOL(s == nsStringUtf8(L"fghijklmnopqrstuvwxyzäö").GetData());

    s.Shrink(9, 7);
    NS_TEST_BOOL(s == nsStringUtf8(L"opqrstu").GetData());

    s.Shrink(3, 2);
    NS_TEST_BOOL(s == nsStringUtf8(L"rs").GetData());

    s.Shrink(1, 0);
    NS_TEST_BOOL(s == nsStringUtf8(L"s").GetData());

    s.Shrink(0, 0);
    NS_TEST_BOOL(s == nsStringUtf8(L"s").GetData());

    s.Shrink(0, 1);
    NS_TEST_BOOL(s == nsStringUtf8(L"").GetData());

    s.Shrink(10, 0);
    NS_TEST_BOOL(s == nsStringUtf8(L"").GetData());

    s.Shrink(0, 10);
    NS_TEST_BOOL(s == nsStringUtf8(L"").GetData());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Reserve")
  {
    nsHeapAllocator allocator("reserve test allocator");
    nsStringBuilder s(L"abcdefghijklmnopqrstuvwxyzäöü€ß", &allocator);
    nsUInt32 characterCountBefore = s.GetCharacterCount();

    s.Reserve(2048);

    NS_TEST_BOOL(s.GetCharacterCount() == characterCountBefore);

    nsUInt64 iNumAllocs = allocator.GetStats().m_uiNumAllocations;
    s.Append("blablablablablablablablablablablablablablablablablablablablablablablablablablablablablabla");
    NS_TEST_BOOL(iNumAllocs == allocator.GetStats().m_uiNumAllocations);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Convert to StringView")
  {
    nsStringBuilder s(L"abcdefghijklmnopqrstuvwxyzäöü€ß");
    nsStringView it = s;

    NS_TEST_BOOL(it.StartsWith(nsStringUtf8(L"abcdefghijklmnopqrstuvwxyzäöü€ß").GetData()));
    NS_TEST_BOOL(it.EndsWith(nsStringUtf8(L"abcdefghijklmnopqrstuvwxyzäöü€ß").GetData()));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ChangeCharacter")
  {
    nsStringBuilder s(L"abcdefghijklmnopqrstuvwxyzäöü€ß");

    nsStringUtf8 upr(L"ÄÖÜ€ßABCDEFGHIJKLMNOPQRSTUVWXYZ");
    nsStringView view(upr.GetData());

    for (auto it = begin(s); it.IsValid(); ++it, view.Shrink(1, 0))
    {
      s.ChangeCharacter(it, view.GetCharacter());

      NS_TEST_BOOL(it.GetCharacter() == view.GetCharacter()); // iterator reflects the changes
    }

    NS_TEST_BOOL(s == upr.GetData());
    NS_TEST_INT(s.GetCharacterCount(), 31);
    NS_TEST_INT(s.GetElementCount(), 37);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ReplaceSubString")
  {
    nsStringBuilder s(L"abcdefghijklmnopqrstuvwxyzäöü€ß");

    s.ReplaceSubString(s.GetData() + 3, s.GetData() + 7, "DEFG"); // equal length, equal num characters
    NS_TEST_BOOL(s == nsStringUtf8(L"abcDEFGhijklmnopqrstuvwxyzäöü€ß").GetData());
    NS_TEST_INT(s.GetCharacterCount(), 31);
    NS_TEST_INT(s.GetElementCount(), 37);

    s.ReplaceSubString(s.GetData() + 7, s.GetData() + 15, ""); // remove
    NS_TEST_BOOL(s == nsStringUtf8(L"abcDEFGpqrstuvwxyzäöü€ß").GetData());
    NS_TEST_INT(s.GetCharacterCount(), 23);
    NS_TEST_INT(s.GetElementCount(), 29);

    s.ReplaceSubString(s.GetData() + 17, s.GetData() + 22, "blablub"); // make longer
    NS_TEST_BOOL(s == nsStringUtf8(L"abcDEFGpqrstuvwxyblablubü€ß").GetData());
    NS_TEST_INT(s.GetCharacterCount(), 27);
    NS_TEST_INT(s.GetElementCount(), 31);

    s.ReplaceSubString(s.GetData() + 22, s.GetData() + 22, nsStringUtf8(L"määh!").GetData()); // insert
    NS_TEST_BOOL(s == nsStringUtf8(L"abcDEFGpqrstuvwxyblablmääh!ubü€ß").GetData());
    NS_TEST_INT(s.GetCharacterCount(), 32);
    NS_TEST_INT(s.GetElementCount(), 38);

    s.ReplaceSubString(s.GetData(), s.GetData() + 10, nullptr); // remove at front
    NS_TEST_BOOL(s == nsStringUtf8(L"stuvwxyblablmääh!ubü€ß").GetData());
    NS_TEST_INT(s.GetCharacterCount(), 22);
    NS_TEST_INT(s.GetElementCount(), 28);

    s.ReplaceSubString(s.GetData() + 18, s.GetData() + 28, nullptr); // remove at back
    NS_TEST_BOOL(s == nsStringUtf8(L"stuvwxyblablmääh").GetData());
    NS_TEST_INT(s.GetCharacterCount(), 16);
    NS_TEST_INT(s.GetElementCount(), 18);

    s.ReplaceSubString(s.GetData(), s.GetData() + 18, nullptr); // clear
    NS_TEST_BOOL(s == nsStringUtf8(L"").GetData());
    NS_TEST_INT(s.GetCharacterCount(), 0);
    NS_TEST_INT(s.GetElementCount(), 0);

    const char* szInsert = "abc def ghi";

    s.ReplaceSubString(s.GetData(), s.GetData(), nsStringView(szInsert, szInsert + 7)); // partial insert into empty
    NS_TEST_BOOL(s == nsStringUtf8(L"abc def").GetData());
    NS_TEST_INT(s.GetCharacterCount(), 7);
    NS_TEST_INT(s.GetElementCount(), 7);

    // insert very large block
    s = nsStringBuilder("a"); // hard reset to keep buffer small
    nsString insertString("omfg this string is so long it possibly won't never ever ever ever fit into the current buffer - this will "
                          "hopefully lead to a buffer resize :)"
                          "................................................................................................................"
                          "........................................................"
                          "................................................................................................................"
                          "........................................................"
                          "................................................................................................................"
                          "........................................................"
                          "................................................................................................................"
                          "........................................................"
                          "................................................................................................................"
                          "........................................................"
                          "................................................................................................................"
                          "........................................................");
    s.ReplaceSubString(s.GetData(), s.GetData() + s.GetElementCount(), insertString.GetData());
    NS_TEST_BOOL(s == insertString.GetData());
    NS_TEST_INT(s.GetCharacterCount(), insertString.GetCharacterCount());
    NS_TEST_INT(s.GetElementCount(), insertString.GetElementCount());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Insert")
  {
    nsStringBuilder s;

    s.Insert(s.GetData(), "test");
    NS_TEST_BOOL(s == "test");

    s.Insert(s.GetData() + 2, "TUT");
    NS_TEST_BOOL(s == "teTUTst");

    s.Insert(s.GetData(), "MOEP");
    NS_TEST_BOOL(s == "MOEPteTUTst");

    s.Insert(s.GetData() + s.GetElementCount(), "hompf");
    NS_TEST_BOOL(s == "MOEPteTUTsthompf");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Remove")
  {
    nsStringBuilder s("MOEPteTUTsthompf");

    s.Remove(s.GetData() + 11, s.GetData() + s.GetElementCount());
    NS_TEST_BOOL(s == "MOEPteTUTst");

    s.Remove(s.GetData(), s.GetData() + 4);
    NS_TEST_BOOL(s == "teTUTst");

    s.Remove(s.GetData() + 2, s.GetData() + 5);
    NS_TEST_BOOL(s == "test");

    s.Remove(s.GetData(), s.GetData() + s.GetElementCount());
    NS_TEST_BOOL(s == "");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ReplaceFirst")
  {
    nsStringBuilder s = "abc def abc def ghi abc ghi";

    s.ReplaceFirst("def", "BLOED");
    NS_TEST_BOOL(s == "abc BLOED abc def ghi abc ghi");

    s.ReplaceFirst("abc", "BLOED");
    NS_TEST_BOOL(s == "BLOED BLOED abc def ghi abc ghi");

    s.ReplaceFirst("abc", "BLOED", s.GetData() + 15);
    NS_TEST_BOOL(s == "BLOED BLOED abc def ghi BLOED ghi");

    s.ReplaceFirst("ghi", "LAANGWEILIG");
    NS_TEST_BOOL(s == "BLOED BLOED abc def LAANGWEILIG BLOED ghi");

    s.ReplaceFirst("ghi", "LAANGWEILIG");
    NS_TEST_BOOL(s == "BLOED BLOED abc def LAANGWEILIG BLOED LAANGWEILIG");

    s.ReplaceFirst("def", "OEDE");
    NS_TEST_BOOL(s == "BLOED BLOED abc OEDE LAANGWEILIG BLOED LAANGWEILIG");

    s.ReplaceFirst("abc", "BLOEDE");
    NS_TEST_BOOL(s == "BLOED BLOED BLOEDE OEDE LAANGWEILIG BLOED LAANGWEILIG");

    s.ReplaceFirst("BLOED BLOED BLOEDE OEDE LAANGWEILIG BLOED LAANGWEILIG", "weg");
    NS_TEST_BOOL(s == "weg");

    s.ReplaceFirst("weg", nullptr);
    NS_TEST_BOOL(s == "");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ReplaceLast")
  {
    nsStringBuilder s = "abc def abc def ghi abc ghi";

    s.ReplaceLast("abc", "ABC");
    NS_TEST_BOOL(s == "abc def abc def ghi ABC ghi");

    s.ReplaceLast("abc", "ABC");
    NS_TEST_BOOL(s == "abc def ABC def ghi ABC ghi");

    s.ReplaceLast("abc", "ABC");
    NS_TEST_BOOL(s == "ABC def ABC def ghi ABC ghi");

    s.ReplaceLast("ghi", "GHI", s.GetData() + 24);
    NS_TEST_BOOL(s == "ABC def ABC def GHI ABC ghi");

    s.ReplaceLast("i", "I");
    NS_TEST_BOOL(s == "ABC def ABC def GHI ABC ghI");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ReplaceAll")
  {
    nsStringBuilder s = "abc def abc def ghi abc ghi";

    s.ReplaceAll("abc", "TEST");
    NS_TEST_BOOL(s == "TEST def TEST def ghi TEST ghi");

    s.ReplaceAll("def", "def");
    NS_TEST_BOOL(s == "TEST def TEST def ghi TEST ghi");

    s.ReplaceAll("def", "defdef");
    NS_TEST_BOOL(s == "TEST defdef TEST defdef ghi TEST ghi");

    s.ReplaceAll("def", "defdef");
    NS_TEST_BOOL(s == "TEST defdefdefdef TEST defdefdefdef ghi TEST ghi");

    s.ReplaceAll("def", " ");
    NS_TEST_BOOL(s == "TEST      TEST      ghi TEST ghi");

    s.ReplaceAll(" ", "");
    NS_TEST_BOOL(s == "TESTTESTghiTESTghi");

    s.ReplaceAll("TEST", "a");
    NS_TEST_BOOL(s == "aaghiaghi");

    s.ReplaceAll("hi", "hihi");
    NS_TEST_BOOL(s == "aaghihiaghihi");

    s.ReplaceAll("ag", " ");
    NS_TEST_BOOL(s == "a hihi hihi");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ReplaceFirst_NoCase")
  {
    nsStringBuilder s = "abc def abc def ghi abc ghi";

    s.ReplaceFirst_NoCase("DEF", "BLOED");
    NS_TEST_BOOL(s == "abc BLOED abc def ghi abc ghi");

    s.ReplaceFirst_NoCase("ABC", "BLOED");
    NS_TEST_BOOL(s == "BLOED BLOED abc def ghi abc ghi");

    s.ReplaceFirst_NoCase("ABC", "BLOED", s.GetData() + 15);
    NS_TEST_BOOL(s == "BLOED BLOED abc def ghi BLOED ghi");

    s.ReplaceFirst_NoCase("GHI", "LAANGWEILIG");
    NS_TEST_BOOL(s == "BLOED BLOED abc def LAANGWEILIG BLOED ghi");

    s.ReplaceFirst_NoCase("GHI", "LAANGWEILIG");
    NS_TEST_BOOL(s == "BLOED BLOED abc def LAANGWEILIG BLOED LAANGWEILIG");

    s.ReplaceFirst_NoCase("DEF", "OEDE");
    NS_TEST_BOOL(s == "BLOED BLOED abc OEDE LAANGWEILIG BLOED LAANGWEILIG");

    s.ReplaceFirst_NoCase("ABC", "BLOEDE");
    NS_TEST_BOOL(s == "BLOED BLOED BLOEDE OEDE LAANGWEILIG BLOED LAANGWEILIG");

    s.ReplaceFirst_NoCase("BLOED BLOED BLOEDE OEDE LAANGWEILIG BLOED LAANGWEILIG", "weg");
    NS_TEST_BOOL(s == "weg");

    s.ReplaceFirst_NoCase("WEG", nullptr);
    NS_TEST_BOOL(s == "");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ReplaceLast_NoCase")
  {
    nsStringBuilder s = "abc def abc def ghi abc ghi";

    s.ReplaceLast_NoCase("abc", "ABC");
    NS_TEST_BOOL(s == "abc def abc def ghi ABC ghi");

    s.ReplaceLast_NoCase("aBc", "ABC");
    NS_TEST_BOOL(s == "abc def abc def ghi ABC ghi");

    s.ReplaceLast_NoCase("ABC", "ABC");
    NS_TEST_BOOL(s == "abc def abc def ghi ABC ghi");

    s.ReplaceLast_NoCase("GHI", "GHI", s.GetData() + 24);
    NS_TEST_BOOL(s == "abc def abc def GHI ABC ghi");

    s.ReplaceLast_NoCase("I", "I");
    NS_TEST_BOOL(s == "abc def abc def GHI ABC ghI");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ReplaceAll_NoCase")
  {
    nsStringBuilder s = "abc def abc def ghi abc ghi";

    s.ReplaceAll_NoCase("ABC", "TEST");
    NS_TEST_BOOL(s == "TEST def TEST def ghi TEST ghi");

    s.ReplaceAll_NoCase("DEF", "def");
    NS_TEST_BOOL(s == "TEST def TEST def ghi TEST ghi");

    s.ReplaceAll_NoCase("DEF", "defdef");
    NS_TEST_BOOL(s == "TEST defdef TEST defdef ghi TEST ghi");

    s.ReplaceAll_NoCase("DEF", "defdef");
    NS_TEST_BOOL(s == "TEST defdefdefdef TEST defdefdefdef ghi TEST ghi");

    s.ReplaceAll_NoCase("DEF", " ");
    NS_TEST_BOOL(s == "TEST      TEST      ghi TEST ghi");

    s.ReplaceAll_NoCase(" ", "");
    NS_TEST_BOOL(s == "TESTTESTghiTESTghi");

    s.ReplaceAll_NoCase("teST", "a");
    NS_TEST_BOOL(s == "aaghiaghi");

    s.ReplaceAll_NoCase("hI", "hihi");
    NS_TEST_BOOL(s == "aaghihiaghihi");

    s.ReplaceAll_NoCase("Ag", " ");
    NS_TEST_BOOL(s == "a hihi hihi");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ReplaceWholeWord")
  {
    nsStringBuilder s = "abcd abc abcd abc dabc abc";

    NS_TEST_BOOL(s.ReplaceWholeWord("abc", "def", nsStringUtils::IsWordDelimiter_English) != nullptr);
    NS_TEST_BOOL(s == "abcd def abcd abc dabc abc");

    NS_TEST_BOOL(s.ReplaceWholeWord("abc", "def", nsStringUtils::IsWordDelimiter_English) != nullptr);
    NS_TEST_BOOL(s == "abcd def abcd def dabc abc");

    NS_TEST_BOOL(s.ReplaceWholeWord("abc", "def", nsStringUtils::IsWordDelimiter_English) != nullptr);
    NS_TEST_BOOL(s == "abcd def abcd def dabc def");

    NS_TEST_BOOL(s.ReplaceWholeWord("abc", "def", nsStringUtils::IsWordDelimiter_English) == nullptr);
    NS_TEST_BOOL(s == "abcd def abcd def dabc def");

    NS_TEST_BOOL(s.ReplaceWholeWord("abcd", "def", nsStringUtils::IsWordDelimiter_English) != nullptr);
    NS_TEST_BOOL(s == "def def abcd def dabc def");

    NS_TEST_BOOL(s.ReplaceWholeWord("abcd", "def", nsStringUtils::IsWordDelimiter_English) != nullptr);
    NS_TEST_BOOL(s == "def def def def dabc def");

    NS_TEST_BOOL(s.ReplaceWholeWord("abcd", "def", nsStringUtils::IsWordDelimiter_English) == nullptr);
    NS_TEST_BOOL(s == "def def def def dabc def");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ReplaceWholeWord_NoCase")
  {
    nsStringBuilder s = "abcd abc abcd abc dabc abc";

    NS_TEST_BOOL(s.ReplaceWholeWord_NoCase("ABC", "def", nsStringUtils::IsWordDelimiter_English) != nullptr);
    NS_TEST_BOOL(s == "abcd def abcd abc dabc abc");

    NS_TEST_BOOL(s.ReplaceWholeWord_NoCase("ABC", "def", nsStringUtils::IsWordDelimiter_English) != nullptr);
    NS_TEST_BOOL(s == "abcd def abcd def dabc abc");

    NS_TEST_BOOL(s.ReplaceWholeWord_NoCase("ABC", "def", nsStringUtils::IsWordDelimiter_English) != nullptr);
    NS_TEST_BOOL(s == "abcd def abcd def dabc def");

    NS_TEST_BOOL(s.ReplaceWholeWord_NoCase("ABC", "def", nsStringUtils::IsWordDelimiter_English) == nullptr);
    NS_TEST_BOOL(s == "abcd def abcd def dabc def");

    NS_TEST_BOOL(s.ReplaceWholeWord_NoCase("ABCd", "def", nsStringUtils::IsWordDelimiter_English) != nullptr);
    NS_TEST_BOOL(s == "def def abcd def dabc def");

    NS_TEST_BOOL(s.ReplaceWholeWord_NoCase("aBCD", "def", nsStringUtils::IsWordDelimiter_English) != nullptr);
    NS_TEST_BOOL(s == "def def def def dabc def");

    NS_TEST_BOOL(s.ReplaceWholeWord_NoCase("ABcd", "def", nsStringUtils::IsWordDelimiter_English) == nullptr);
    NS_TEST_BOOL(s == "def def def def dabc def");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ReplaceWholeWordAll")
  {
    nsStringBuilder s = "abcd abc abcd abc dabc abc";

    NS_TEST_INT(s.ReplaceWholeWordAll("abc", "def", nsStringUtils::IsWordDelimiter_English), 3);
    NS_TEST_BOOL(s == "abcd def abcd def dabc def");

    NS_TEST_INT(s.ReplaceWholeWordAll("abc", "def", nsStringUtils::IsWordDelimiter_English), 0);
    NS_TEST_BOOL(s == "abcd def abcd def dabc def");

    NS_TEST_INT(s.ReplaceWholeWordAll("abcd", "def", nsStringUtils::IsWordDelimiter_English), 2);
    NS_TEST_BOOL(s == "def def def def dabc def");

    NS_TEST_INT(s.ReplaceWholeWordAll("abcd", "def", nsStringUtils::IsWordDelimiter_English), 0);
    NS_TEST_BOOL(s == "def def def def dabc def");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ReplaceWholeWordAll_NoCase")
  {
    nsStringBuilder s = "abcd abc abcd abc dabc abc";

    NS_TEST_INT(s.ReplaceWholeWordAll_NoCase("ABC", "def", nsStringUtils::IsWordDelimiter_English), 3);
    NS_TEST_BOOL(s == "abcd def abcd def dabc def");

    NS_TEST_INT(s.ReplaceWholeWordAll_NoCase("ABC", "def", nsStringUtils::IsWordDelimiter_English), 0);
    NS_TEST_BOOL(s == "abcd def abcd def dabc def");

    NS_TEST_INT(s.ReplaceWholeWordAll_NoCase("ABCd", "def", nsStringUtils::IsWordDelimiter_English), 2);
    NS_TEST_BOOL(s == "def def def def dabc def");

    NS_TEST_INT(s.ReplaceWholeWordAll_NoCase("ABCd", "def", nsStringUtils::IsWordDelimiter_English), 0);
    NS_TEST_BOOL(s == "def def def def dabc def");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "teset")
  {
    const char* sz = "abc def";
    nsStringView it(sz);

    nsStringBuilder s = it;
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Split")
  {
    nsStringBuilder s = "|abc,def<>ghi|,<>jkl|mno,pqr|stu";

    nsHybridArray<nsStringView, 32> SubStrings;

    s.Split(false, SubStrings, ",", "|", "<>");

    NS_TEST_INT(SubStrings.GetCount(), 7);
    NS_TEST_BOOL(SubStrings[0] == "abc");
    NS_TEST_BOOL(SubStrings[1] == "def");
    NS_TEST_BOOL(SubStrings[2] == "ghi");
    NS_TEST_BOOL(SubStrings[3] == "jkl");
    NS_TEST_BOOL(SubStrings[4] == "mno");
    NS_TEST_BOOL(SubStrings[5] == "pqr");
    NS_TEST_BOOL(SubStrings[6] == "stu");

    s.Split(true, SubStrings, ",", "|", "<>");

    NS_TEST_INT(SubStrings.GetCount(), 10);
    NS_TEST_BOOL(SubStrings[0] == "");
    NS_TEST_BOOL(SubStrings[1] == "abc");
    NS_TEST_BOOL(SubStrings[2] == "def");
    NS_TEST_BOOL(SubStrings[3] == "ghi");
    NS_TEST_BOOL(SubStrings[4] == "");
    NS_TEST_BOOL(SubStrings[5] == "");
    NS_TEST_BOOL(SubStrings[6] == "jkl");
    NS_TEST_BOOL(SubStrings[7] == "mno");
    NS_TEST_BOOL(SubStrings[8] == "pqr");
    NS_TEST_BOOL(SubStrings[9] == "stu");
  }


  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeCleanPath")
  {
    nsStringBuilder p;

    p = "C:\\temp/temp//tut";
    p.MakeCleanPath();
    NS_TEST_BOOL(p == "C:/temp/temp/tut");

    p = "\\temp/temp//tut\\\\";
    p.MakeCleanPath();
    NS_TEST_BOOL(p == "/temp/temp/tut/");

    p = "\\";
    p.MakeCleanPath();
    NS_TEST_BOOL(p == "/");

    p = "file";
    p.MakeCleanPath();
    NS_TEST_BOOL(p == "file");

    p = "C:\\temp/..//tut";
    p.MakeCleanPath();
    NS_TEST_BOOL(p == "C:/tut");

    p = "C:\\temp/..";
    p.MakeCleanPath();
    NS_TEST_BOOL(p == "C:/temp/..");

    p = "C:\\temp/..\\";
    p.MakeCleanPath();
    NS_TEST_BOOL(p == "C:/");

    p = "\\//temp/../bla\\\\blub///..\\temp//tut/tat/..\\\\..\\//ploep";
    p.MakeCleanPath();
    NS_TEST_BOOL(p == "//bla/temp/ploep");

    p = "a/b/c/../../../../e/f";
    p.MakeCleanPath();
    NS_TEST_BOOL(p == "../e/f");

    p = "/../../a/../../e/f";
    p.MakeCleanPath();
    NS_TEST_BOOL(p == "../../e/f");

    p = "/../../a/../../e/f/../";
    p.MakeCleanPath();
    NS_TEST_BOOL(p == "../../e/");

    p = "/../../a/../../e/f/..";
    p.MakeCleanPath();
    NS_TEST_BOOL(p == "../../e/f/..");

    p = "\\//temp/./bla\\\\blub///.\\temp//tut/tat/..\\.\\.\\//ploep";
    p.MakeCleanPath();
    NS_TEST_STRING(p.GetData(), "//temp/bla/blub/temp/tut/ploep");

    p = "./";
    p.MakeCleanPath();
    NS_TEST_STRING(p.GetData(), "");

    p = "/./././";
    p.MakeCleanPath();
    NS_TEST_STRING(p.GetData(), "/");

    p = "./.././";
    p.MakeCleanPath();
    NS_TEST_STRING(p.GetData(), "../");

    // more than two dots are invalid, so the should be kept as is
    p = "./..././abc/...\\def";
    p.MakeCleanPath();
    NS_TEST_STRING(p.GetData(), ".../abc/.../def");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "PathParentDirectory")
  {
    nsStringBuilder p;

    p = "C:\\temp/temp//tut";
    p.PathParentDirectory();
    NS_TEST_BOOL(p == "C:/temp/temp/");

    p = "C:\\temp/temp//tut\\\\";
    p.PathParentDirectory();
    NS_TEST_BOOL(p == "C:/temp/temp/");

    p = "file";
    p.PathParentDirectory();
    NS_TEST_BOOL(p == "");

    p = "/file";
    p.PathParentDirectory();
    NS_TEST_BOOL(p == "/");

    p = "C:\\temp/..//tut";
    p.PathParentDirectory();
    NS_TEST_BOOL(p == "C:/");

    p = "file";
    p.PathParentDirectory(3);
    NS_TEST_BOOL(p == "../../");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "AppendPath")
  {
    nsStringBuilder p;

    p = "this/is\\my//path";
    p.AppendPath("orly/nowai");
    NS_TEST_BOOL(p == "this/is\\my//path/orly/nowai");

    p = "this/is\\my//path///";
    p.AppendPath("orly/nowai");
    NS_TEST_BOOL(p == "this/is\\my//path///orly/nowai");

    p = "";
    p.AppendPath("orly/nowai");
    NS_TEST_BOOL(p == "orly/nowai");

    // It should be valid to append an absolute path to an empty string.
    {
#if NS_ENABLED(NS_PLATFORM_WINDOWS)
      const char* szAbsPath = "C:\\folder";
      const char* szAbsPathAppendResult = "C:\\folder/File.ext";
#elif NS_ENABLED(NS_PLATFORM_OSX) || NS_ENABLED(NS_PLATFORM_LINUX) || NS_ENABLED(NS_PLATFORM_ANDROID)
      const char* szAbsPath = "/folder";
      const char* szAbsPathAppendResult = "/folder/File.ext";
#else
#  error "An absolute path example must be defined for the 'AppendPath' test for each platform!"
#endif

      p = "";
      p.AppendPath(szAbsPath, "File.ext");
      NS_TEST_BOOL(p == szAbsPathAppendResult);
    }

    p = "bla";
    p.AppendPath("");
    NS_TEST_BOOL(p == "bla");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ChangeFileName")
  {
    nsStringBuilder p;

    p = "C:/test/test/tut.ext";
    p.ChangeFileName("bla");
    NS_TEST_BOOL(p == "C:/test/test/bla.ext");

    p = "test/test/tut/troet.toeff";
    p.ChangeFileName("toeff");
    NS_TEST_BOOL(p == "test/test/tut/toeff.toeff");

    p = "test/test/tut/murpf";
    p.ChangeFileName("toeff");
    NS_TEST_BOOL(p == "test/test/tut/toeff");

    p = "test/test/tut/murpf/";
    p.ChangeFileName("toeff");
    NS_TEST_BOOL(p == "test/test/tut/murpf/toeff"); // filename is EMPTY -> thus ADDS it

    p = "test/test/tut/murpf/.file";                // files that start with a dot are considered to be filenames with no extension
    p.ChangeFileName("toeff");
    NS_TEST_BOOL(p == "test/test/tut/murpf/toeff");

    p = "test/test/tut/murpf/.file.extension";
    p.ChangeFileName("toeff");
    NS_TEST_BOOL(p == "test/test/tut/murpf/toeff.extension");

    p = "test/test/tut/murpf/.extension/"; // folders that start with a dot ARE considered as folders, if the path ends with a slash
    p.ChangeFileName("toeff");
    NS_TEST_BOOL(p == "test/test/tut/murpf/.extension/toeff");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ChangeFileNameAndExtension")
  {
    nsStringBuilder p;

    p = "C:/test/test/tut.ext";
    p.ChangeFileNameAndExtension("bla.pups");
    NS_TEST_BOOL(p == "C:/test/test/bla.pups");

    p = "test/test/tut/troet.toeff";
    p.ChangeFileNameAndExtension("toeff");
    NS_TEST_BOOL(p == "test/test/tut/toeff");

    p = "test/test/tut/murpf";
    p.ChangeFileNameAndExtension("toeff.tut");
    NS_TEST_BOOL(p == "test/test/tut/toeff.tut");

    p = "test/test/tut/murpf/";
    p.ChangeFileNameAndExtension("toeff.blo");
    NS_TEST_BOOL(p == "test/test/tut/murpf/toeff.blo"); // filename is EMPTY -> thus ADDS it

    p = "test/test/tut/murpf/.extension";               // folders that start with a dot must be considered to be empty filenames with an extension
    p.ChangeFileNameAndExtension("toeff.ext");
    NS_TEST_BOOL(p == "test/test/tut/murpf/toeff.ext");

    p = "test/test/tut/murpf/.extension/"; // folders that start with a dot ARE considered as folders, if the path ends with a slash
    p.ChangeFileNameAndExtension("toeff");
    NS_TEST_BOOL(p == "test/test/tut/murpf/.extension/toeff");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ChangeFileExtension")
  {
    nsStringBuilder p;

    p = "C:/test/test/tut.ext";
    p.ChangeFileExtension("pups");
    NS_TEST_BOOL(p == "C:/test/test/tut.pups");

    p = "C:/test/test/tut";
    p.ChangeFileExtension("pups");
    NS_TEST_BOOL(p == "C:/test/test/tut.pups");

    p = "C:/test/test/tut.ext";
    p.ChangeFileExtension("");
    NS_TEST_BOOL(p == "C:/test/test/tut.");

    p = "C:/test/test/tut";
    p.ChangeFileExtension("");
    NS_TEST_BOOL(p == "C:/test/test/tut.");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "HasAnyExtension")
  {
    nsStringBuilder p = "This/Is\\My//Path.dot\\file.extension";
    NS_TEST_BOOL(p.HasAnyExtension());

    p = "This/Is\\My//Path.dot\\file_no_extension";
    NS_TEST_BOOL(!p.HasAnyExtension());
    NS_TEST_BOOL(!p.HasAnyExtension());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "HasExtension")
  {
    nsStringBuilder p;

    p = "This/Is\\My//Path.dot\\file.extension";
    NS_TEST_BOOL(p.HasExtension(".Extension"));

    p = "This/Is\\My//Path.dot\\file.ext";
    NS_TEST_BOOL(p.HasExtension("EXT"));

    p = "This/Is\\My//Path.dot\\file.ext";
    NS_TEST_BOOL(!p.HasExtension("NEXT"));

    p = "This/Is\\My//Path.dot\\file.extension";
    NS_TEST_BOOL(!p.HasExtension(".Ext"));

    p = "This/Is\\My//Path.dot\\file.extension";
    NS_TEST_BOOL(!p.HasExtension("sion"));

    p = "";
    NS_TEST_BOOL(!p.HasExtension("ext"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetFileExtension")
  {
    nsStringBuilder p;

    p = "This/Is\\My//Path.dot\\file.extension";
    NS_TEST_BOOL(p.GetFileExtension() == "extension");

    p = "This/Is\\My//Path.dot\\file";
    NS_TEST_BOOL(p.GetFileExtension() == "");

    p = "";
    NS_TEST_BOOL(p.GetFileExtension() == "");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetFileNameAndExtension")
  {
    nsStringBuilder p;

    p = "This/Is\\My//Path.dot\\file.extension";
    NS_TEST_BOOL(p.GetFileNameAndExtension() == "file.extension");

    p = "This/Is\\My//Path.dot\\.extension";
    NS_TEST_BOOL(p.GetFileNameAndExtension() == ".extension");

    p = "This/Is\\My//Path.dot\\file";
    NS_TEST_BOOL(p.GetFileNameAndExtension() == "file");

    p = "\\file";
    NS_TEST_BOOL(p.GetFileNameAndExtension() == "file");

    p = "";
    NS_TEST_BOOL(p.GetFileNameAndExtension() == "");

    p = "/";
    NS_TEST_BOOL(p.GetFileNameAndExtension() == "");

    p = "This/Is\\My//Path.dot\\";
    NS_TEST_BOOL(p.GetFileNameAndExtension() == "");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetFileName")
  {
    nsStringBuilder p;

    p = "This/Is\\My//Path.dot\\file.extension";
    NS_TEST_BOOL(p.GetFileName() == "file");

    p = "This/Is\\My//Path.dot\\file";
    NS_TEST_BOOL(p.GetFileName() == "file");

    p = "\\file";
    NS_TEST_BOOL(p.GetFileName() == "file");

    p = "";
    NS_TEST_BOOL(p.GetFileName() == "");

    p = "/";
    NS_TEST_BOOL(p.GetFileName() == "");

    p = "This/Is\\My//Path.dot\\";
    NS_TEST_BOOL(p.GetFileName() == "");

    p = "This/Is\\My//Path.dot\\.stupidfile";
    NS_TEST_BOOL(p.GetFileName() == ".stupidfile");

    p = "This/Is\\My//Path.dot\\.stupidfile.ext";
    NS_TEST_BOOL(p.GetFileName() == ".stupidfile");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetFileDirectory")
  {
    nsStringBuilder p;

    p = "This/Is\\My//Path.dot\\file.extension";
    NS_TEST_BOOL(p.GetFileDirectory() == "This/Is\\My//Path.dot\\");

    p = "This/Is\\My//Path.dot\\.extension";
    NS_TEST_BOOL(p.GetFileDirectory() == "This/Is\\My//Path.dot\\");

    p = "This/Is\\My//Path.dot\\file";
    NS_TEST_BOOL(p.GetFileDirectory() == "This/Is\\My//Path.dot\\");

    p = "\\file";
    NS_TEST_BOOL(p.GetFileDirectory() == "\\");

    p = "";
    NS_TEST_BOOL(p.GetFileDirectory() == "");

    p = "/";
    NS_TEST_BOOL(p.GetFileDirectory() == "/");

    p = "This/Is\\My//Path.dot\\";
    NS_TEST_BOOL(p.GetFileDirectory() == "This/Is\\My//Path.dot\\");

    p = "This";
    NS_TEST_BOOL(p.GetFileDirectory() == "");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsAbsolutePath / IsRelativePath / IsRootedPath")
  {
    nsStringBuilder p;

    p = "";
    NS_TEST_BOOL(!p.IsAbsolutePath());
    NS_TEST_BOOL(p.IsRelativePath());
    NS_TEST_BOOL(!p.IsRootedPath());

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
    p = "C:\\temp.stuff";
    NS_TEST_BOOL(p.IsAbsolutePath());
    NS_TEST_BOOL(!p.IsRelativePath());
    NS_TEST_BOOL(!p.IsRootedPath());

    p = "C:/temp.stuff";
    NS_TEST_BOOL(p.IsAbsolutePath());
    NS_TEST_BOOL(!p.IsRelativePath());
    NS_TEST_BOOL(!p.IsRootedPath());

    p = "\\\\myserver\\temp.stuff";
    NS_TEST_BOOL(p.IsAbsolutePath());
    NS_TEST_BOOL(!p.IsRelativePath());
    NS_TEST_BOOL(!p.IsRootedPath());

    p = "\\myserver\\temp.stuff";
    NS_TEST_BOOL(!p.IsAbsolutePath());
    NS_TEST_BOOL(!p.IsRelativePath()); // neither absolute nor relativ, just stupid
    NS_TEST_BOOL(!p.IsRootedPath());

    p = "temp.stuff";
    NS_TEST_BOOL(!p.IsAbsolutePath());
    NS_TEST_BOOL(p.IsRelativePath());
    NS_TEST_BOOL(!p.IsRootedPath());

    p = "/temp.stuff";
    NS_TEST_BOOL(!p.IsAbsolutePath());
    NS_TEST_BOOL(!p.IsRelativePath()); // bloed
    NS_TEST_BOOL(!p.IsRootedPath());

    p = "\\temp.stuff";
    NS_TEST_BOOL(!p.IsAbsolutePath());
    NS_TEST_BOOL(!p.IsRelativePath()); // bloed
    NS_TEST_BOOL(!p.IsRootedPath());

    p = "..\\temp.stuff";
    NS_TEST_BOOL(!p.IsAbsolutePath());
    NS_TEST_BOOL(p.IsRelativePath());
    NS_TEST_BOOL(!p.IsRootedPath());

    p = ".\\temp.stuff";
    NS_TEST_BOOL(!p.IsAbsolutePath());
    NS_TEST_BOOL(p.IsRelativePath());
    NS_TEST_BOOL(!p.IsRootedPath());

    p = ":MyDataDir\bla";
    NS_TEST_BOOL(!p.IsAbsolutePath());
    NS_TEST_BOOL(!p.IsRelativePath());
    NS_TEST_BOOL(p.IsRootedPath());

    p = ":\\MyDataDir\bla";
    NS_TEST_BOOL(!p.IsAbsolutePath());
    NS_TEST_BOOL(!p.IsRelativePath());
    NS_TEST_BOOL(p.IsRootedPath());

    p = ":/MyDataDir/bla";
    NS_TEST_BOOL(!p.IsAbsolutePath());
    NS_TEST_BOOL(!p.IsRelativePath());
    NS_TEST_BOOL(p.IsRootedPath());

#elif NS_ENABLED(NS_PLATFORM_OSX) || NS_ENABLED(NS_PLATFORM_LINUX) || NS_ENABLED(NS_PLATFORM_ANDROID)

    p = "C:\\temp.stuff";
    NS_TEST_BOOL(!p.IsAbsolutePath());
    NS_TEST_BOOL(p.IsRelativePath());
    NS_TEST_BOOL(!p.IsRootedPath());

    p = "temp.stuff";
    NS_TEST_BOOL(!p.IsAbsolutePath());
    NS_TEST_BOOL(p.IsRelativePath());
    NS_TEST_BOOL(!p.IsRootedPath());

    p = "/temp.stuff";
    NS_TEST_BOOL(p.IsAbsolutePath());
    NS_TEST_BOOL(!p.IsRelativePath());
    NS_TEST_BOOL(!p.IsRootedPath());

    p = "..\\temp.stuff";
    NS_TEST_BOOL(!p.IsAbsolutePath());
    NS_TEST_BOOL(p.IsRelativePath());
    NS_TEST_BOOL(!p.IsRootedPath());

    p = ".\\temp.stuff";
    NS_TEST_BOOL(!p.IsAbsolutePath());
    NS_TEST_BOOL(p.IsRelativePath());
    NS_TEST_BOOL(!p.IsRootedPath());

#else
#  error "Unknown platform."
#endif
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetRootedPathRootName")
  {
    nsStringBuilder p;

    p = ":root\\bla";
    NS_TEST_BOOL(p.GetRootedPathRootName() == "root");

    p = ":root/bla";
    NS_TEST_BOOL(p.GetRootedPathRootName() == "root");

    p = "://root/bla";
    NS_TEST_BOOL(p.GetRootedPathRootName() == "root");

    p = ":/\\/root\\/bla";
    NS_TEST_BOOL(p.GetRootedPathRootName() == "root");

    p = "://\\root";
    NS_TEST_BOOL(p.GetRootedPathRootName() == "root");

    p = ":";
    NS_TEST_BOOL(p.GetRootedPathRootName() == "");

    p = "";
    NS_TEST_BOOL(p.GetRootedPathRootName() == "");

    p = "noroot\\bla";
    NS_TEST_BOOL(p.GetRootedPathRootName() == "");

    p = "C:\\noroot/bla";
    NS_TEST_BOOL(p.GetRootedPathRootName() == "");

    p = "/noroot/bla";
    NS_TEST_BOOL(p.GetRootedPathRootName() == "");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsPathBelowFolder")
  {
    nsStringBuilder p;

    p = "a/b\\c//d\\\\e/f";
    NS_TEST_BOOL(!p.IsPathBelowFolder("/a/b\\c"));
    NS_TEST_BOOL(p.IsPathBelowFolder("a/b\\c"));
    NS_TEST_BOOL(p.IsPathBelowFolder("a/b\\c//"));
    NS_TEST_BOOL(p.IsPathBelowFolder("a/b\\c//d/\\e\\f")); // equal paths are considered 'below'
    NS_TEST_BOOL(!p.IsPathBelowFolder("a/b\\c//d/\\e\\f/g"));
    NS_TEST_BOOL(p.IsPathBelowFolder("a"));
    NS_TEST_BOOL(!p.IsPathBelowFolder("b"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakeRelativeTo")
  {
    nsStringBuilder p;

    p = (const char*)u8"ä/b\\c/d\\\\e/f/g";
    NS_TEST_BOOL(p.MakeRelativeTo((const char*)u8"ä\\b/c").Succeeded());
    NS_TEST_BOOL(p == "d/e/f/g");
    NS_TEST_BOOL(p.MakeRelativeTo((const char*)u8"ä\\b/c").Failed());
    NS_TEST_BOOL(p == "d/e/f/g");

    p = (const char*)u8"ä/b\\c//d\\\\e/f/g";
    NS_TEST_BOOL(p.MakeRelativeTo((const char*)u8"ä\\b/c").Succeeded());
    NS_TEST_BOOL(p == "d/e/f/g");
    NS_TEST_BOOL(p.MakeRelativeTo((const char*)u8"ä\\b/c").Failed());
    NS_TEST_BOOL(p == "d/e/f/g");

    p = (const char*)u8"ä/b\\c/d\\\\e/f/g";
    NS_TEST_BOOL(p.MakeRelativeTo((const char*)u8"ä\\b/c/").Succeeded());
    NS_TEST_BOOL(p == "d/e/f/g");
    NS_TEST_BOOL(p.MakeRelativeTo((const char*)u8"ä\\b/c/").Failed());
    NS_TEST_BOOL(p == "d/e/f/g");

    p = (const char*)u8"ä/b\\c//d\\\\e/f/g";
    NS_TEST_BOOL(p.MakeRelativeTo((const char*)u8"ä\\b/c/").Succeeded());
    NS_TEST_BOOL(p == "d/e/f/g");
    NS_TEST_BOOL(p.MakeRelativeTo((const char*)u8"ä\\b/c/").Failed());
    NS_TEST_BOOL(p == "d/e/f/g");

    p = (const char*)u8"ä/b\\c//d\\\\e/f/g";
    NS_TEST_BOOL(p.MakeRelativeTo((const char*)u8"ä\\b/c\\/d/\\e\\f/g").Succeeded());
    NS_TEST_BOOL(p == "");
    NS_TEST_BOOL(p.MakeRelativeTo((const char*)u8"ä\\b/c\\/d/\\e\\f/g").Failed());
    NS_TEST_BOOL(p == "");

    p = (const char*)u8"ä/b\\c//d\\\\e/f/g/";
    NS_TEST_BOOL(p.MakeRelativeTo((const char*)u8"ä\\b/c\\/d//e\\f/g\\h/i").Succeeded());
    NS_TEST_BOOL(p == "../../");
    NS_TEST_BOOL(p.MakeRelativeTo((const char*)u8"ä\\b/c\\/d//e\\f/g\\h/i").Failed());
    NS_TEST_BOOL(p == "../../");

    p = (const char*)u8"ä/b\\c//d\\\\e/f/g/j/k";
    NS_TEST_BOOL(p.MakeRelativeTo((const char*)u8"ä\\b/c\\/d//e\\f/g\\h/i").Succeeded());
    NS_TEST_BOOL(p == "../../j/k");
    NS_TEST_BOOL(p.MakeRelativeTo((const char*)u8"ä\\b/c\\/d//e\\f/g\\h/i").Failed());
    NS_TEST_BOOL(p == "../../j/k");

    p = (const char*)u8"ä/b\\c//d\\\\e/f/ge";
    NS_TEST_BOOL(p.MakeRelativeTo((const char*)u8"ä\\b/c//d/\\e\\f/g\\h/i").Succeeded());
    NS_TEST_BOOL(p == "../../../ge");
    NS_TEST_BOOL(p.MakeRelativeTo((const char*)u8"ä\\b/c//d/\\e\\f/g\\h/i").Failed());
    NS_TEST_BOOL(p == "../../../ge");

    p = (const char*)u8"ä/b\\c//d\\\\e/f/g.txt";
    NS_TEST_BOOL(p.MakeRelativeTo((const char*)u8"ä\\b/c//d//e\\f/g\\h/i").Succeeded());
    NS_TEST_BOOL(p == "../../../g.txt");
    NS_TEST_BOOL(p.MakeRelativeTo((const char*)u8"ä\\b/c//d//e\\f/g\\h/i").Failed());
    NS_TEST_BOOL(p == "../../../g.txt");

    p = (const char*)u8"ä/b\\c//d\\\\e/f/g";
    NS_TEST_BOOL(p.MakeRelativeTo((const char*)u8"ä\\b/c//d//e\\f/g\\h/i").Succeeded());
    NS_TEST_BOOL(p == "../../");
    NS_TEST_BOOL(p.MakeRelativeTo((const char*)u8"ä\\b/c//d//e\\f/g\\h/i").Failed());
    NS_TEST_BOOL(p == "../../");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakePathSeparatorsNative")
  {
    nsStringBuilder p;
    p = "This/is\\a/temp\\\\path//to/my///file";

    p.MakePathSeparatorsNative();

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
    NS_TEST_STRING(p.GetData(), "This\\is\\a\\temp\\path\\to\\my\\file");
#else
    NS_TEST_STRING(p.GetData(), "This/is/a/temp/path/to/my/file");
#endif
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ReadAll")
  {
    nsDefaultMemoryStreamStorage StreamStorage;

    nsMemoryStreamWriter MemoryWriter(&StreamStorage);
    nsMemoryStreamReader MemoryReader(&StreamStorage);

    const char* szText =
      "l;kjasdflkjdfasjlk asflkj asfljwe oiweq2390432 4 @#$ otrjk3l;2rlkhitoqhrn324:R l324h32kjr hnasfhsakfh234fas1440687873242321245";

    MemoryWriter.WriteBytes(szText, nsStringUtils::GetStringElementCount(szText)).IgnoreResult();

    nsStringBuilder s;
    s.ReadAll(MemoryReader);

    NS_TEST_BOOL(s == szText);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetSubString_FromTo")
  {
    nsStringBuilder sb = "basf";

    const char* sz = "abcdefghijklmnopqrstuvwxyz";

    sb.SetSubString_FromTo(sz + 5, sz + 13);
    NS_TEST_BOOL(sb == "fghijklm");

    sb.SetSubString_FromTo(sz + 17, sz + 30);
    NS_TEST_BOOL(sb == "rstuvwxyz");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetSubString_ElementCount")
  {
    nsStringBuilder sb = "basf";

    nsStringUtf8 sz(L"aäbcödefügh");

    sb.SetSubString_ElementCount(sz.GetData() + 5, 5);
    NS_TEST_BOOL(sb == nsStringUtf8(L"ödef").GetData());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetSubString_CharacterCount")
  {
    nsStringBuilder sb = "basf";

    nsStringUtf8 sz(L"aäbcödefgh");

    sb.SetSubString_CharacterCount(sz.GetData() + 5, 5);
    NS_TEST_BOOL(sb == nsStringUtf8(L"ödefg").GetData());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RemoveFileExtension")
  {
    nsStringBuilder sb = L"⺅⻩⽇⿕.〄㈷㑧䆴.ؼݻ༺.";

    sb.RemoveFileExtension();
    NS_TEST_STRING_UNICODE(sb.GetData(), nsStringUtf8(L"⺅⻩⽇⿕.〄㈷㑧䆴.ؼݻ༺").GetData());

    sb.RemoveFileExtension();
    NS_TEST_STRING_UNICODE(sb.GetData(), nsStringUtf8(L"⺅⻩⽇⿕.〄㈷㑧䆴").GetData());

    sb.RemoveFileExtension();
    NS_TEST_STRING_UNICODE(sb.GetData(), nsStringUtf8(L"⺅⻩⽇⿕").GetData());

    sb.RemoveFileExtension();
    NS_TEST_STRING_UNICODE(sb.GetData(), nsStringUtf8(L"⺅⻩⽇⿕").GetData());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Trim")
  {
    // Empty input
    nsStringBuilder sb = L"";
    sb.Trim(" \t");
    NS_TEST_STRING(sb.GetData(), nsStringUtf8(L"").GetData());
    sb.Trim(nullptr, " \t");
    NS_TEST_STRING(sb.GetData(), nsStringUtf8(L"").GetData());
    sb.Trim(" \t", nullptr);
    NS_TEST_STRING(sb.GetData(), nsStringUtf8(L"").GetData());

    // Clear all from one side
    auto sUnicode = L"私はクリストハさんです";
    sb = sUnicode;
    sb.Trim(nullptr, nsStringUtf8(sUnicode).GetData());
    NS_TEST_STRING(sb.GetData(), "");
    sb = sUnicode;
    sb.Trim(nsStringUtf8(sUnicode).GetData(), nullptr);
    NS_TEST_STRING(sb.GetData(), "");

    // Clear partial side
    sb = L"ですですですAにぱにぱにぱ";
    sb.Trim(nullptr, nsStringUtf8(L"にぱ").GetData());
    NS_TEST_STRING_UNICODE(sb.GetData(), nsStringUtf8(L"ですですですA").GetData());
    sb.Trim(nsStringUtf8(L"です").GetData(), nullptr);
    NS_TEST_STRING_UNICODE(sb.GetData(), nsStringUtf8(L"A").GetData());

    sb = L"ですですですAにぱにぱにぱ";
    sb.Trim(nsStringUtf8(L"ですにぱ").GetData());
    NS_TEST_STRING(sb.GetData(), nsStringUtf8(L"A").GetData());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "TrimWordStart")
  {
    nsStringBuilder sb;

    {
      sb = "<test>abc<test>";
      NS_TEST_BOOL(sb.TrimWordStart("<test>"));
      NS_TEST_STRING(sb, "abc<test>");
      NS_TEST_BOOL(sb.TrimWordStart("<test>") == false);
      NS_TEST_STRING(sb, "abc<test>");
    }

    {
      sb = "<test><tut><test><test><tut>abc<tut><test>";
      NS_TEST_BOOL(!sb.TrimWordStart("<tut>"));
      NS_TEST_BOOL(sb.TrimWordStart("<test>"));
      NS_TEST_BOOL(sb.TrimWordStart("<tut>"));
      NS_TEST_BOOL(sb.TrimWordStart("<test>"));
      NS_TEST_BOOL(sb.TrimWordStart("<test>"));
      NS_TEST_BOOL(sb.TrimWordStart("<tut>"));
      NS_TEST_STRING(sb, "abc<tut><test>");
      NS_TEST_BOOL(sb.TrimWordStart("<tut>") == false);
      NS_TEST_BOOL(sb.TrimWordStart("<test>") == false);
      NS_TEST_STRING(sb, "abc<tut><test>");
    }

    {
      sb = "<a><b><c><d><e><a><b><c><d><e>abc";

      while (sb.TrimWordStart("<a>") ||
             sb.TrimWordStart("<b>") ||
             sb.TrimWordStart("<c>") ||
             sb.TrimWordStart("<d>") ||
             sb.TrimWordStart("<e>"))
      {
      }

      NS_TEST_STRING(sb, "abc");
    }

    {
      sb = "<a><b><c><d><e><a><b><c><d><e>";

      while (sb.TrimWordStart("<a>") ||
             sb.TrimWordStart("<b>") ||
             sb.TrimWordStart("<c>") ||
             sb.TrimWordStart("<d>") ||
             sb.TrimWordStart("<e>"))
      {
      }

      NS_TEST_STRING(sb, "");
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "TrimWordEnd")
  {
    nsStringBuilder sb;

    {
      sb = "<test>abc<test>";
      NS_TEST_BOOL(sb.TrimWordEnd("<test>"));
      NS_TEST_STRING(sb, "<test>abc");
      NS_TEST_BOOL(sb.TrimWordEnd("<test>") == false);
      NS_TEST_STRING(sb, "<test>abc");
    }

    {
      sb = "<tut><test>abc<test><tut><test><test><tut>";
      NS_TEST_BOOL(sb.TrimWordEnd("<tut>"));
      NS_TEST_BOOL(sb.TrimWordEnd("<test>"));
      NS_TEST_BOOL(sb.TrimWordEnd("<test>"));
      NS_TEST_BOOL(sb.TrimWordEnd("<tut>"));
      NS_TEST_BOOL(sb.TrimWordEnd("<test>"));
      NS_TEST_STRING(sb, "<tut><test>abc");
      NS_TEST_BOOL(sb.TrimWordEnd("<tut>") == false);
      NS_TEST_BOOL(sb.TrimWordEnd("<test>") == false);
      NS_TEST_STRING(sb, "<tut><test>abc");
    }

    {
      sb = "abc<a><b><c><d><e><a><b><c><d><e>";

      while (sb.TrimWordEnd("<a>") ||
             sb.TrimWordEnd("<b>") ||
             sb.TrimWordEnd("<c>") ||
             sb.TrimWordEnd("<d>") ||
             sb.TrimWordEnd("<e>"))
      {
      }

      NS_TEST_STRING(sb, "abc");
    }

    {
      sb = "<a><b><c><d><e><a><b><c><d><e>";

      while (sb.TrimWordEnd("<a>") ||
             sb.TrimWordEnd("<b>") ||
             sb.TrimWordEnd("<c>") ||
             sb.TrimWordEnd("<d>") ||
             sb.TrimWordEnd("<e>"))
      {
      }

      NS_TEST_STRING(sb, "");
    }
  }
}

#pragma optimize("", on)

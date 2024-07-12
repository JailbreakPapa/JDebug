#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Containers/Deque.h>
#include <Foundation/Strings/String.h>

#include <string_view>

using namespace std;

const nsStringView gConstant1 = "gConstant1"_nssv;
const nsStringView gConstant2("gConstant2");
const std::string_view gConstant3 = "gConstant3"sv;

NS_CREATE_SIMPLE_TEST(Strings, StringView)
{
  nsStringBuilder tmp;

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor (simple)")
  {
    const char* sz = "abcdefghijklmnopqrstuvwxyz";

    nsStringView it(sz);

    NS_TEST_BOOL(it.GetStartPointer() == sz);
    NS_TEST_STRING(it.GetData(tmp), sz);
    NS_TEST_BOOL(it.GetEndPointer() == sz + 26);
    NS_TEST_INT(it.GetElementCount(), 26);

    nsStringView it2(sz + 15);

    NS_TEST_BOOL(it2.GetStartPointer() == &sz[15]);
    NS_TEST_STRING(it2.GetData(tmp), &sz[15]);
    NS_TEST_BOOL(it2.GetEndPointer() == sz + 26);
    NS_TEST_INT(it2.GetElementCount(), 11);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor (complex, YARLY!)")
  {
    const char* sz = "abcdefghijklmnopqrstuvwxyz";

    nsStringView it(sz + 3, sz + 17);
    it.SetStartPosition(sz + 5);

    NS_TEST_BOOL(it.GetStartPointer() == sz + 5);
    NS_TEST_STRING(it.GetData(tmp), "fghijklmnopq");
    NS_TEST_BOOL(it.GetEndPointer() == sz + 17);
    NS_TEST_INT(it.GetElementCount(), 12);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor constexpr")
  {
    constexpr nsStringView b = nsStringView("Hello World", 10);
    NS_TEST_INT(b.GetElementCount(), 10);
    NS_TEST_STRING(b.GetData(tmp), "Hello Worl");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "String literal")
  {
    constexpr nsStringView a = "Hello World"_nssv;
    NS_TEST_INT(a.GetElementCount(), 11);
    NS_TEST_STRING(a.GetData(tmp), "Hello World");

    nsStringView b = "Hello Worl"_nssv;
    NS_TEST_INT(b.GetElementCount(), 10);
    NS_TEST_STRING(b.GetData(tmp), "Hello Worl");

    // tests a special case in which the MSVC compiler would run into trouble
    NS_TEST_INT(gConstant1.GetElementCount(), 10);
    NS_TEST_STRING(gConstant1.GetData(tmp), "gConstant1");

    NS_TEST_INT(gConstant2.GetElementCount(), 10);
    NS_TEST_STRING(gConstant2.GetData(tmp), "gConstant2");

    NS_TEST_INT(gConstant3.size(), 10);
    NS_TEST_BOOL(gConstant3 == "gConstant3");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator++")
  {
    const char* sz = "abcdefghijklmnopqrstuvwxyz";
    nsStringView it(sz);

    for (nsInt32 i = 0; i < 26; ++i)
    {
      NS_TEST_INT(it.GetCharacter(), sz[i]);
      NS_TEST_BOOL(it.IsValid());
      it.Shrink(1, 0);
    }

    NS_TEST_BOOL(!it.IsValid());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator== / operator!=")
  {
    nsString s1(L"abcdefghiäöüß€");
    nsString s2(L"ghiäöüß€abdef");

    nsStringView it1 = s1.GetSubString(8, 4);
    nsStringView it2 = s2.GetSubString(2, 4);
    nsStringView it3 = s2.GetSubString(2, 5);

    NS_TEST_BOOL(it1 == it2);
    NS_TEST_BOOL(it1 != it3);

    NS_TEST_BOOL(it1 == nsString(L"iäöü").GetData());
    NS_TEST_BOOL(it2 == nsString(L"iäöü").GetData());
    NS_TEST_BOOL(it3 == nsString(L"iäöüß").GetData());

    s1 = "abcdefghijkl";
    s2 = "oghijklm";

    it1 = s1.GetSubString(6, 4);
    it2 = s2.GetSubString(1, 4);
    it3 = s2.GetSubString(1, 5);

    NS_TEST_BOOL(it1 == it2);
    NS_TEST_BOOL(it1 != it3);

    NS_TEST_BOOL(it1 == "ghij");
    NS_TEST_BOOL(it1 != "ghijk");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsEqual")
  {
    const char* sz = "abcdef";
    nsStringView it(sz);

    NS_TEST_BOOL(it.IsEqual(nsStringView("abcdef")));
    NS_TEST_BOOL(!it.IsEqual(nsStringView("abcde")));
    NS_TEST_BOOL(!it.IsEqual(nsStringView("abcdefg")));

    nsStringView it2(sz + 2, sz + 5);

    const char* szRhs = "Abcdef";
    nsStringView it3(szRhs + 2, szRhs + 5);
    NS_TEST_BOOL(it2.IsEqual(it3));
    it3 = nsStringView(szRhs + 1, szRhs + 5);
    NS_TEST_BOOL(!it2.IsEqual(it3));
    it3 = nsStringView(szRhs + 2, szRhs + 6);
    NS_TEST_BOOL(!it2.IsEqual(it3));
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

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator+=")
  {
    const char* sz = "abcdefghijklmnopqrstuvwxyz";
    nsStringView it(sz);

    for (nsInt32 i = 0; i < 26; i += 2)
    {
      NS_TEST_INT(it.GetCharacter(), sz[i]);
      NS_TEST_BOOL(it.IsValid());
      it.Shrink(2, 0);
    }

    NS_TEST_BOOL(!it.IsValid());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetCharacter")
  {
    nsStringUtf8 s(L"abcäöü€");
    nsStringView it = nsStringView(s.GetData());

    NS_TEST_INT(it.GetCharacter(), nsUnicodeUtils::ConvertUtf8ToUtf32(&s.GetData()[0]));
    it.Shrink(1, 0);
    NS_TEST_INT(it.GetCharacter(), nsUnicodeUtils::ConvertUtf8ToUtf32(&s.GetData()[1]));
    it.Shrink(1, 0);
    NS_TEST_INT(it.GetCharacter(), nsUnicodeUtils::ConvertUtf8ToUtf32(&s.GetData()[2]));
    it.Shrink(1, 0);
    NS_TEST_INT(it.GetCharacter(), nsUnicodeUtils::ConvertUtf8ToUtf32(&s.GetData()[3]));
    it.Shrink(1, 0);
    NS_TEST_INT(it.GetCharacter(), nsUnicodeUtils::ConvertUtf8ToUtf32(&s.GetData()[5]));
    it.Shrink(1, 0);
    NS_TEST_INT(it.GetCharacter(), nsUnicodeUtils::ConvertUtf8ToUtf32(&s.GetData()[7]));
    it.Shrink(1, 0);
    NS_TEST_INT(it.GetCharacter(), nsUnicodeUtils::ConvertUtf8ToUtf32(&s.GetData()[9]));
    it.Shrink(1, 0);
    NS_TEST_BOOL(!it.IsValid());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetElementCount")
  {
    nsStringUtf8 s(L"abcäöü€");
    nsStringView it = nsStringView(s.GetData());

    NS_TEST_INT(it.GetElementCount(), 12);
    it.Shrink(1, 0);
    NS_TEST_BOOL(it.IsValid());
    NS_TEST_INT(it.GetElementCount(), 11);
    it.Shrink(1, 0);
    NS_TEST_BOOL(it.IsValid());
    NS_TEST_INT(it.GetElementCount(), 10);
    it.Shrink(1, 0);
    NS_TEST_BOOL(it.IsValid());
    NS_TEST_INT(it.GetElementCount(), 9);
    it.Shrink(1, 0);
    NS_TEST_BOOL(it.IsValid());
    NS_TEST_INT(it.GetElementCount(), 7);
    it.Shrink(1, 0);
    NS_TEST_BOOL(it.IsValid());
    NS_TEST_INT(it.GetElementCount(), 5);
    it.Shrink(1, 0);
    NS_TEST_BOOL(it.IsValid());
    NS_TEST_INT(it.GetElementCount(), 3);
    it.Shrink(1, 0);
    NS_TEST_BOOL(!it.IsValid());
    NS_TEST_INT(it.GetElementCount(), 0);
    it.Shrink(1, 0);
    NS_TEST_BOOL(!it.IsValid());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetStartPosition")
  {
    const char* sz = "abcdefghijklmnopqrstuvwxyz";
    nsStringView it(sz);

    for (nsInt32 i = 0; i < 26; ++i)
    {
      it.SetStartPosition(sz + i);
      NS_TEST_BOOL(it.IsValid());
      NS_TEST_BOOL(it.StartsWith(&sz[i]));
    }

    NS_TEST_BOOL(it.IsValid());
    it.Shrink(1, 0);
    NS_TEST_BOOL(!it.IsValid());

    it = nsStringView(sz);
    for (nsInt32 i = 0; i < 26; ++i)
    {
      it.SetStartPosition(sz + i);
      NS_TEST_BOOL(it.IsValid());
      NS_TEST_BOOL(it.StartsWith(&sz[i]));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetStartPosition / GetEndPosition / GetData")
  {
    const char* sz = "abcdefghijklmnopqrstuvwxyz";
    nsStringView it(sz + 7, sz + 19);

    NS_TEST_BOOL(it.GetStartPointer() == sz + 7);
    NS_TEST_BOOL(it.GetEndPointer() == sz + 19);
    NS_TEST_STRING(it.GetData(tmp), "hijklmnopqrs");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Shrink")
  {
    nsStringUtf8 s(L"abcäöü€def");
    nsStringView it(s.GetData());

    NS_TEST_BOOL(it.GetStartPointer() == &s.GetData()[0]);
    NS_TEST_BOOL(it.GetEndPointer() == &s.GetData()[15]);
    NS_TEST_STRING(it.GetData(tmp), &s.GetData()[0]);
    NS_TEST_BOOL(it.IsValid());

    it.Shrink(1, 0);

    NS_TEST_BOOL(it.GetStartPointer() == &s.GetData()[1]);
    NS_TEST_BOOL(it.GetEndPointer() == &s.GetData()[15]);
    NS_TEST_STRING(it.GetData(tmp), &s.GetData()[1]);
    NS_TEST_BOOL(it.IsValid());

    it.Shrink(3, 0);

    NS_TEST_BOOL(it.GetStartPointer() == &s.GetData()[5]);
    NS_TEST_BOOL(it.GetEndPointer() == &s.GetData()[15]);
    NS_TEST_STRING(it.GetData(tmp), &s.GetData()[5]);
    NS_TEST_BOOL(it.IsValid());

    it.Shrink(0, 4);

    NS_TEST_BOOL(it.GetStartPointer() == &s.GetData()[5]);
    NS_TEST_BOOL(it.GetEndPointer() == &s.GetData()[9]);
    NS_TEST_STRING(it.GetData(tmp), (const char*)u8"öü");
    NS_TEST_BOOL(it.IsValid());

    it.Shrink(1, 1);

    NS_TEST_BOOL(it.GetStartPointer() == &s.GetData()[7]);
    NS_TEST_BOOL(it.GetEndPointer() == &s.GetData()[7]);
    NS_TEST_STRING(it.GetData(tmp), "");
    NS_TEST_BOOL(!it.IsValid());

    it.Shrink(10, 10);

    NS_TEST_BOOL(it.GetStartPointer() == &s.GetData()[7]);
    NS_TEST_BOOL(it.GetEndPointer() == &s.GetData()[7]);
    NS_TEST_STRING(it.GetData(tmp), "");
    NS_TEST_BOOL(!it.IsValid());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ChopAwayFirstCharacterUtf8")
  {
    nsStringUtf8 utf8(L"О, Господи!");
    nsStringView s(utf8.GetData());

    const char* szOrgStart = s.GetStartPointer();
    const char* szOrgEnd = s.GetEndPointer();

    while (!s.IsEmpty())
    {
      const nsUInt32 uiNumCharsBefore = nsStringUtils::GetCharacterCount(s.GetStartPointer(), s.GetEndPointer());
      s.ChopAwayFirstCharacterUtf8();
      const nsUInt32 uiNumCharsAfter = nsStringUtils::GetCharacterCount(s.GetStartPointer(), s.GetEndPointer());

      NS_TEST_INT(uiNumCharsBefore, uiNumCharsAfter + 1);
    }

    // this needs to be true, some code relies on the fact that the start pointer always moves forwards
    NS_TEST_BOOL(s.GetStartPointer() == szOrgEnd);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ChopAwayFirstCharacterAscii")
  {
    nsStringUtf8 utf8(L"Wosn Schmarrn");
    nsStringView s("");

    const char* szOrgStart = s.GetStartPointer();
    const char* szOrgEnd = s.GetEndPointer();

    while (!s.IsEmpty())
    {
      const nsUInt32 uiNumCharsBefore = s.GetElementCount();
      s.ChopAwayFirstCharacterAscii();
      const nsUInt32 uiNumCharsAfter = s.GetElementCount();

      NS_TEST_INT(uiNumCharsBefore, uiNumCharsAfter + 1);
    }

    // this needs to be true, some code relies on the fact that the start pointer always moves forwards
    NS_TEST_BOOL(s.GetStartPointer() == szOrgEnd);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Trim")
  {
    // Empty input
    nsStringUtf8 utf8(L"");
    nsStringView view(utf8.GetData());
    view.Trim(" \t");
    NS_TEST_BOOL(view.IsEqual(nsStringUtf8(L"").GetData()));
    view.Trim(nullptr, " \t");
    NS_TEST_BOOL(view.IsEqual(nsStringUtf8(L"").GetData()));
    view.Trim(" \t", nullptr);
    NS_TEST_BOOL(view.IsEqual(nsStringUtf8(L"").GetData()));

    // Clear all from one side
    nsStringUtf8 sUnicode(L"私はクリストハさんです");
    view = sUnicode.GetData();
    view.Trim(nullptr, sUnicode.GetData());
    NS_TEST_BOOL(view.IsEqual(""));
    view = sUnicode.GetData();
    view.Trim(sUnicode.GetData(), nullptr);
    NS_TEST_BOOL(view.IsEqual(""));

    // Clear partial side
    sUnicode = L"ですですですAにぱにぱにぱ";
    view = sUnicode.GetData();
    view.Trim(nullptr, nsStringUtf8(L"にぱ").GetData());
    sUnicode = L"ですですですA";
    NS_TEST_BOOL(view.IsEqual(sUnicode.GetData()));
    view.Trim(nsStringUtf8(L"です").GetData(), nullptr);
    NS_TEST_BOOL(view.IsEqual(nsStringUtf8(L"A").GetData()));

    sUnicode = L"ですですですAにぱにぱにぱ";
    view = sUnicode.GetData();
    view.Trim(nsStringUtf8(L"ですにぱ").GetData());
    NS_TEST_BOOL(view.IsEqual(nsStringUtf8(L"A").GetData()));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "TrimWordStart")
  {
    nsStringView sb;

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
    nsStringView sb;

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

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Split")
  {
    nsStringView s = "|abc,def<>ghi|,<>jkl|mno,pqr|stu";

    nsDeque<nsStringView> SubStrings;

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

  NS_TEST_BLOCK(nsTestBlock::Enabled, "HasAnyExtension")
  {
    nsStringView p = "This/Is\\My//Path.dot\\file.extension";
    NS_TEST_BOOL(p.HasAnyExtension());

    p = "This/Is\\My//Path.dot\\file_no_extension";
    NS_TEST_BOOL(!p.HasAnyExtension());
    NS_TEST_BOOL(!p.HasAnyExtension());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "HasExtension")
  {
    nsStringView p;

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
    nsStringView p;

    p = "This/Is\\My//Path.dot\\file.extension";
    NS_TEST_BOOL(p.GetFileExtension() == "extension");

    p = "This/Is\\My//Path.dot\\file";
    NS_TEST_BOOL(p.GetFileExtension() == "");

    p = "";
    NS_TEST_BOOL(p.GetFileExtension() == "");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetFileNameAndExtension")
  {
    nsStringView p;

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
    nsStringView p;

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

    p = "This/Is\\My//Path.dot\\.stupidfile.ext.";
    NS_TEST_BOOL(p.GetFileName() == ".stupidfile.ext.");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetFileDirectory")
  {
    nsStringView p;

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
    nsStringView p;

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
    nsStringView p;

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

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetSubString")
  {
    nsStringView s = u8"Пожалуйста, дай мне очень длинные Unicode-стринги!";

    NS_TEST_BOOL(s.GetElementCount() > nsStringUtils::GetCharacterCount(s.GetStartPointer(), s.GetEndPointer()));

    nsStringView w1 = s.GetSubString(0, 10);
    nsStringView w2 = s.GetSubString(12, 3);
    nsStringView w3 = s.GetSubString(20, 5);
    nsStringView w4 = s.GetSubString(34, 15);
    nsStringView w5 = s.GetSubString(34, 20);
    nsStringView w6 = s.GetSubString(100, 10);

    NS_TEST_BOOL(w1 == nsStringView(u8"Пожалуйста"));
    NS_TEST_BOOL(w2 == nsStringView(u8"дай"));
    NS_TEST_BOOL(w3 == nsStringView(u8"очень"));
    NS_TEST_BOOL(w4 == nsStringView(u8"Unicode-стринги"));
    NS_TEST_BOOL(w5 == nsStringView(u8"Unicode-стринги!"));
    NS_TEST_BOOL(!w6.IsValid());
    NS_TEST_BOOL(w6 == nsStringView(""));
  }
}

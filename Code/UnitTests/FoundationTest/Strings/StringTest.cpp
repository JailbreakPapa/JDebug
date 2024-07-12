#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Strings/String.h>

static nsString GetString(const char* szSz)
{
  nsString s;
  s = szSz;
  return s;
}

static nsStringBuilder GetStringBuilder(const char* szSz)
{
  nsStringBuilder s;

  for (nsUInt32 i = 0; i < 10; ++i)
    s.Append(szSz);

  return s;
}

NS_CREATE_SIMPLE_TEST(Strings, String)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
    nsString s1;
    NS_TEST_BOOL(s1 == "");

    nsString s2("abc");
    NS_TEST_BOOL(s2 == "abc");

    nsString s3(s2);
    NS_TEST_BOOL(s2 == s3);
    NS_TEST_BOOL(s3 == "abc");

    nsString s4(L"abc");
    NS_TEST_BOOL(s4 == "abc");

    nsStringView it = s4.GetFirst(2);
    nsString s5(it);
    NS_TEST_BOOL(s5 == "ab");

    nsStringBuilder strB("wobwob");
    nsString s6(strB);
    NS_TEST_BOOL(s6 == "wobwob");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator=")
  {
    nsString s2;
    s2 = "abc";
    NS_TEST_BOOL(s2 == "abc");

    nsString s3;
    s3 = s2;
    NS_TEST_BOOL(s2 == s3);
    NS_TEST_BOOL(s3 == "abc");

    nsString s4;
    s4 = L"abc";
    NS_TEST_BOOL(s4 == "abc");

    nsString s5(L"abcdefghijklm");
    nsStringView it(s5.GetData() + 2, s5.GetData() + 10);
    nsString s5b = it;
    NS_TEST_STRING(s5b, "cdefghij");

    nsString s6(L"aölsdföasld");
    nsStringBuilder strB("wobwob");
    s6 = strB;
    NS_TEST_BOOL(s6 == "wobwob");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "convert to nsStringView")
  {
    nsString s(L"aölsdföasld");
    nsStringBuilder tmp;

    nsStringView sv = s;

    NS_TEST_STRING(sv.GetData(tmp), nsStringUtf8(L"aölsdföasld").GetData());
    NS_TEST_BOOL(sv == nsStringUtf8(L"aölsdföasld").GetData());

    s = "abcdef";

    NS_TEST_STRING(sv.GetStartPointer(), "abcdef");
    NS_TEST_BOOL(sv == "abcdef");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Move constructor / operator")
  {
    nsString s1(GetString("move me"));
    NS_TEST_STRING(s1.GetData(), "move me");

    s1 = GetString("move move move move move move move move ");
    NS_TEST_STRING(s1.GetData(), "move move move move move move move move ");

    nsString s2(GetString("move move move move move move move move "));
    NS_TEST_STRING(s2.GetData(), "move move move move move move move move ");

    s2 = GetString("move me");
    NS_TEST_STRING(s2.GetData(), "move me");

    s1 = s2;
    NS_TEST_STRING(s1.GetData(), "move me");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Move constructor / operator (StringBuilder)")
  {
    const nsString s1(GetStringBuilder("move me"));
    const nsString s2(GetStringBuilder("move move move move move move move move "));

    nsString s3(GetStringBuilder("move me"));
    NS_TEST_BOOL(s3 == s1);

    s3 = GetStringBuilder("move move move move move move move move ");
    NS_TEST_BOOL(s3 == s2);

    nsString s4(GetStringBuilder("move move move move move move move move "));
    NS_TEST_BOOL(s4 == s2);

    s4 = GetStringBuilder("move me");
    NS_TEST_BOOL(s4 == s1);

    s3 = s4;
    NS_TEST_BOOL(s3 == s1);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Clear")
  {
    nsString s("abcdef");
    NS_TEST_BOOL(s == "abcdef");

    s.Clear();
    NS_TEST_BOOL(s.IsEmpty());
    NS_TEST_BOOL(s == "");
    NS_TEST_BOOL(s == nullptr);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetData")
  {
    const char* sz = "abcdef";

    nsString s(sz);
    NS_TEST_BOOL(s.GetData() != sz); // it should NOT be the exact same string
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetElementCount / GetCharacterCount")
  {
    nsString s(L"abcäöü€");

    NS_TEST_INT(s.GetElementCount(), 12);
    NS_TEST_INT(s.GetCharacterCount(), 7);

    s = "testtest";
    NS_TEST_INT(s.GetElementCount(), 8);
    NS_TEST_INT(s.GetCharacterCount(), 8);

    s.Clear();

    NS_TEST_INT(s.GetElementCount(), 0);
    NS_TEST_INT(s.GetCharacterCount(), 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Convert to nsStringView")
  {
    nsString s(L"abcäöü€def");

    nsStringView view = s;
    NS_TEST_BOOL(view.StartsWith("abc"));
    NS_TEST_BOOL(view.EndsWith("def"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetSubString")
  {
    nsString s(L"abcäöü€def");
    nsStringUtf8 s8(L"äöü€");

    nsStringView it = s.GetSubString(3, 4);
    NS_TEST_BOOL(it == s8.GetData());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetFirst")
  {
    nsString s(L"abcäöü€def");

    NS_TEST_BOOL(s.GetFirst(3) == "abc");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetLast")
  {
    nsString s(L"abcäöü€def");

    NS_TEST_BOOL(s.GetLast(3) == "def");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ReadAll")
  {
    nsDefaultMemoryStreamStorage StreamStorage;

    nsMemoryStreamWriter MemoryWriter(&StreamStorage);
    nsMemoryStreamReader MemoryReader(&StreamStorage);

    const char* szText =
      "l;kjasdflkjdfasjlk asflkj asfljwe oiweq2390432 4 @#$ otrjk3l;2rlkhitoqhrn324:R l324h32kjr hnasfhsakfh234fas1440687873242321245";

    MemoryWriter.WriteBytes(szText, nsStringUtils::GetStringElementCount(szText)).IgnoreResult();

    nsString s;
    s.ReadAll(MemoryReader);

    NS_TEST_BOOL(s == szText);
  }
}

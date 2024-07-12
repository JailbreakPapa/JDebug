#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Strings/HashedString.h>

NS_CREATE_SIMPLE_TEST(Strings, HashedString)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
    nsHashedString s;
    nsHashedString s2;

    s2.Assign("test"); // compile time hashing

    NS_TEST_INT(s.GetHash(), 0xef46db3751d8e999llu);
    NS_TEST_STRING(s.GetString().GetData(), "");
    NS_TEST_BOOL(s.GetString().IsEmpty());

    nsTempHashedString ts("test"); // compile time hashing
    NS_TEST_INT(ts.GetHash(), 0x4fdcca5ddb678139llu);

    nsStringBuilder sb = "test2";
    nsTempHashedString ts2(sb.GetData()); // runtime hashing
    NS_TEST_INT(ts2.GetHash(), 0x890e0a4c7111eb87llu);

    nsTempHashedString ts3(s2);
    NS_TEST_INT(ts3.GetHash(), 0x4fdcca5ddb678139llu);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Assign")
  {
    nsHashedString s;
    s.Assign("Test"); // compile time hashing

    NS_TEST_STRING(s.GetString().GetData(), "Test");
    NS_TEST_INT(s.GetHash(), 0xda83efc38a8922b4llu);

    nsStringBuilder sb = "test2";
    s.Assign(sb.GetData()); // runtime hashing
    NS_TEST_STRING(s.GetString().GetData(), "test2");
    NS_TEST_INT(s.GetHash(), 0x890e0a4c7111eb87llu);

    nsTempHashedString ts("dummy");
    ts = "test";       // compile time hashing
    NS_TEST_INT(ts.GetHash(), 0x4fdcca5ddb678139llu);

    ts = sb.GetData(); // runtime hashing
    NS_TEST_INT(ts.GetHash(), 0x890e0a4c7111eb87llu);

    s.Assign("");
    NS_TEST_INT(s.GetHash(), 0xef46db3751d8e999llu);
    NS_TEST_STRING(s.GetString().GetData(), "");
    NS_TEST_BOOL(s.GetString().IsEmpty());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "TempHashedString")
  {
    nsTempHashedString ts;
    nsHashedString hs;

    NS_TEST_INT(ts.GetHash(), hs.GetHash());

    NS_TEST_INT(ts.GetHash(), 0xef46db3751d8e999llu);

    ts = "Test";
    nsTempHashedString ts2 = ts;
    NS_TEST_INT(ts.GetHash(), 0xda83efc38a8922b4llu);

    ts = "";
    ts2.Clear();
    NS_TEST_INT(ts.GetHash(), 0xef46db3751d8e999llu);
    NS_TEST_INT(ts2.GetHash(), 0xef46db3751d8e999llu);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator== / operator!=")
  {
    nsHashedString s1, s2, s3, s4;
    s1.Assign("Test1");
    s2.Assign("Test2");
    s3.Assign("Test1");
    s4.Assign("Test2");

    nsTempHashedString t1("Test1");
    nsTempHashedString t2("Test2");

    NS_TEST_STRING(s1.GetString().GetData(), "Test1");
    NS_TEST_STRING(s2.GetString().GetData(), "Test2");
    NS_TEST_STRING(s3.GetString().GetData(), "Test1");
    NS_TEST_STRING(s4.GetString().GetData(), "Test2");

    NS_TEST_BOOL(s1 == s1);
    NS_TEST_BOOL(s2 == s2);
    NS_TEST_BOOL(s3 == s3);
    NS_TEST_BOOL(s4 == s4);
    NS_TEST_BOOL(t1 == t1);
    NS_TEST_BOOL(t2 == t2);

    NS_TEST_BOOL(s1 != s2);
    NS_TEST_BOOL(s1 == s3);
    NS_TEST_BOOL(s1 != s4);
    NS_TEST_BOOL(s1 == t1);
    NS_TEST_BOOL(s1 != t2);

    NS_TEST_BOOL(s2 != s3);
    NS_TEST_BOOL(s2 == s4);
    NS_TEST_BOOL(s2 != t1);
    NS_TEST_BOOL(s2 == t2);

    NS_TEST_BOOL(s3 != s4);
    NS_TEST_BOOL(s3 == t1);
    NS_TEST_BOOL(s3 != t2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Copying")
  {
    nsHashedString s1;
    s1.Assign("blaa");

    nsHashedString s2(s1);
    nsHashedString s3;
    s3 = s2;

    NS_TEST_BOOL(s1 == s2);
    NS_TEST_BOOL(s1 == s3);

    nsHashedString s4(std::move(s2));
    nsHashedString s5;
    s5 = std::move(s3);

    NS_TEST_BOOL(s1 == s4);
    NS_TEST_BOOL(s1 == s5);
    NS_TEST_BOOL(s1 != s2);
    NS_TEST_BOOL(s1 != s3);

    nsTempHashedString t1("blaa");

    nsTempHashedString t2(t1);
    nsTempHashedString t3("urg");
    t3 = t2;

    NS_TEST_BOOL(t1 == t2);
    NS_TEST_BOOL(t1 == t3);

    t3 = s1;
    NS_TEST_INT(t3.GetHash(), s1.GetHash());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator<")
  {
    nsHashedString s1, s2, s3;
    s1.Assign("blaa");
    s2.Assign("blub");
    s3.Assign("tut");

    nsMap<nsHashedString, nsInt32> m; // uses operator< internally
    m[s1] = 1;
    m[s2] = 2;
    m[s3] = 3;

    NS_TEST_INT(m[s1], 1);
    NS_TEST_INT(m[s2], 2);
    NS_TEST_INT(m[s3], 3);

    nsTempHashedString t1("blaa");
    nsTempHashedString t2("blub");
    nsTempHashedString t3("tut");

    NS_TEST_BOOL((s1 < s1) == (t1 < t1));
    NS_TEST_BOOL((s1 < s2) == (t1 < t2));
    NS_TEST_BOOL((s1 < s3) == (t1 < t3));

    NS_TEST_BOOL((s1 < s1) == (s1 < t1));
    NS_TEST_BOOL((s1 < s2) == (s1 < t2));
    NS_TEST_BOOL((s1 < s3) == (s1 < t3));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetString")
  {
    nsHashedString s1, s2, s3;
    s1.Assign("blaa");
    s2.Assign("blub");
    s3.Assign("tut");

    NS_TEST_STRING(s1.GetString().GetData(), "blaa");
    NS_TEST_STRING(s2.GetString().GetData(), "blub");
    NS_TEST_STRING(s3.GetString().GetData(), "tut");
  }

#if NS_ENABLED(NS_HASHED_STRING_REF_COUNTING)
  NS_TEST_BLOCK(nsTestBlock::Enabled, "ClearUnusedStrings")
  {
    nsHashedString::ClearUnusedStrings();

    {
      nsHashedString s1, s2, s3;
      s1.Assign("blaa");
      s2.Assign("blub");
      s3.Assign("tut");
    }

    NS_TEST_INT(nsHashedString::ClearUnusedStrings(), 3);
    NS_TEST_INT(nsHashedString::ClearUnusedStrings(), 0);
  }
#endif
}

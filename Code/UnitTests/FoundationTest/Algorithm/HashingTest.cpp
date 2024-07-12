#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Algorithm/HashHelperString.h>
#include <Foundation/Algorithm/HashStream.h>
#include <Foundation/Algorithm/HashableStruct.h>
#include <Foundation/Strings/HashedString.h>

NS_CREATE_SIMPLE_TEST_GROUP(Algorithm);

// Warning for overflow in compile time executed static_assert(nsHashingUtils::MurmurHash32...)
// Todo: Why is this not happening elsewhere?
#pragma warning(disable : 4307)

NS_CREATE_SIMPLE_TEST(Algorithm, Hashing)
{
  // check whether compile time hashing gives the same value as runtime hashing
  const char* szString = "This is a test string. 1234";
  const char* szStringLower = "this is a test string. 1234";
  const char* szString2 = "THiS iS A TESt sTrInG. 1234";
  nsStringBuilder sb = szString;

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Hashfunction")
  {
    nsUInt32 uiHashRT = nsHashingUtils::MurmurHash32String(sb.GetData());
    constexpr nsUInt32 uiHashCT = nsHashingUtils::MurmurHash32String("This is a test string. 1234");
    NS_TEST_INT(uiHashRT, 0xb999d6c4);
    NS_TEST_INT(uiHashRT, uiHashCT);

    // Static assert to ensure this is happening at compile time!
    static_assert(nsHashingUtils::MurmurHash32String("This is a test string. 1234") == static_cast<nsUInt32>(0xb999d6c4), "Error in compile time murmur hash calculation!");

    {
      // Test short inputs (< 16 characters) of xx hash at compile time
      nsUInt32 uixxHashRT = nsHashingUtils::xxHash32("Test string", 11, 0);
      nsUInt32 uixxHashCT = nsHashingUtils::xxHash32String("Test string", 0);
      NS_TEST_INT(uixxHashRT, uixxHashCT);
      static_assert(nsHashingUtils::xxHash32String("Test string") == 0x1b50ee03);

      // Test long inputs ( > 16 characters) of xx hash at compile time
      nsUInt32 uixxHashRTLong = nsHashingUtils::xxHash32String(sb.GetData());
      nsUInt32 uixxHashCTLong = nsHashingUtils::xxHash32String("This is a test string. 1234");
      NS_TEST_INT(uixxHashRTLong, uixxHashCTLong);
      static_assert(nsHashingUtils::xxHash32String("This is a test string. 1234") == 0xff35b049);
    }

    {
      // Test short inputs (< 32 characters) of xx hash 64 at compile time
      nsUInt64 uixxHash64RT = nsHashingUtils::xxHash64("Test string", 11, 0);
      nsUInt64 uixxHash64CT = nsHashingUtils::xxHash64String("Test string", 0);
      NS_TEST_INT(uixxHash64RT, uixxHash64CT);
      static_assert(nsHashingUtils::xxHash64String("Test string") == 0xcf0f91eece7c88feULL);

      // Test long inputs ( > 32 characters) of xx hash 64 at compile time
      nsUInt64 uixxHash64RTLong = nsHashingUtils::xxHash64String(nsStringView("This is a longer test string for 64-bit. 123456"));
      nsUInt64 uixxHash64CTLong = nsHashingUtils::xxHash64String("This is a longer test string for 64-bit. 123456");
      NS_TEST_INT(uixxHash64RTLong, uixxHash64CTLong);
      static_assert(nsHashingUtils::xxHash64String("This is a longer test string for 64-bit. 123456") == 0xb85d007925299bacULL);
    }

    {
      // Test short inputs (< 32 characters) of xx hash 64 at compile time
      nsUInt64 uixxHash64RT = nsHashingUtils::StringHash(nsStringView("Test string"));
      nsUInt64 uixxHash64CT = nsHashingUtils::StringHash("Test string");
      NS_TEST_INT(uixxHash64RT, uixxHash64CT);
      static_assert(nsHashingUtils::StringHash("Test string") == 0xcf0f91eece7c88feULL);

      // Test long inputs ( > 32 characters) of xx hash 64 at compile time
      nsUInt64 uixxHash64RTLong = nsHashingUtils::StringHash(nsStringView("This is a longer test string for 64-bit. 123456"));
      nsUInt64 uixxHash64CTLong = nsHashingUtils::StringHash("This is a longer test string for 64-bit. 123456");
      NS_TEST_INT(uixxHash64RTLong, uixxHash64CTLong);
      static_assert(nsHashingUtils::StringHash("This is a longer test string for 64-bit. 123456") == 0xb85d007925299bacULL);
    }

    // Check MurmurHash for unaligned inputs
    const char* alignmentTestString = "12345678_12345678__12345678___12345678";
    nsUInt32 uiHash1 = nsHashingUtils::MurmurHash32(alignmentTestString, 8);
    nsUInt32 uiHash2 = nsHashingUtils::MurmurHash32(alignmentTestString + 9, 8);
    nsUInt32 uiHash3 = nsHashingUtils::MurmurHash32(alignmentTestString + 19, 8);
    nsUInt32 uiHash4 = nsHashingUtils::MurmurHash32(alignmentTestString + 30, 8);
    NS_TEST_INT(uiHash1, uiHash2);
    NS_TEST_INT(uiHash1, uiHash3);
    NS_TEST_INT(uiHash1, uiHash4);

    // check 64bit hashes
    const nsUInt64 uiMurmurHash64 = nsHashingUtils::MurmurHash64(sb.GetData(), sb.GetElementCount());
    NS_TEST_INT(uiMurmurHash64, 0xf8ebc5e8cb110786);

    // Check MurmurHash64 for unaligned inputs
    nsUInt64 uiHash1_64 = nsHashingUtils::MurmurHash64(alignmentTestString, 8);
    nsUInt64 uiHash2_64 = nsHashingUtils::MurmurHash64(alignmentTestString + 9, 8);
    nsUInt64 uiHash3_64 = nsHashingUtils::MurmurHash64(alignmentTestString + 19, 8);
    nsUInt64 uiHash4_64 = nsHashingUtils::MurmurHash64(alignmentTestString + 30, 8);
    NS_TEST_INT(uiHash1_64, uiHash2_64);
    NS_TEST_INT(uiHash1_64, uiHash3_64);
    NS_TEST_INT(uiHash1_64, uiHash4_64);

    // test crc32
    const nsUInt32 uiCrc32 = nsHashingUtils::CRC32Hash(sb.GetData(), sb.GetElementCount());
    NS_TEST_INT(uiCrc32, 0x73b5e898);

    // Check crc32 for unaligned inputs
    uiHash1 = nsHashingUtils::CRC32Hash(alignmentTestString, 8);
    uiHash2 = nsHashingUtils::CRC32Hash(alignmentTestString + 9, 8);
    uiHash3 = nsHashingUtils::CRC32Hash(alignmentTestString + 19, 8);
    uiHash4 = nsHashingUtils::CRC32Hash(alignmentTestString + 30, 8);
    NS_TEST_INT(uiHash1, uiHash2);
    NS_TEST_INT(uiHash1, uiHash3);
    NS_TEST_INT(uiHash1, uiHash4);

    // 32 Bit xxHash
    const nsUInt32 uiXXHash32 = nsHashingUtils::xxHash32(sb.GetData(), sb.GetElementCount());
    NS_TEST_INT(uiXXHash32, 0xff35b049);

    // Check xxHash for unaligned inputs
    uiHash1 = nsHashingUtils::xxHash32(alignmentTestString, 8);
    uiHash2 = nsHashingUtils::xxHash32(alignmentTestString + 9, 8);
    uiHash3 = nsHashingUtils::xxHash32(alignmentTestString + 19, 8);
    uiHash4 = nsHashingUtils::xxHash32(alignmentTestString + 30, 8);
    NS_TEST_INT(uiHash1, uiHash2);
    NS_TEST_INT(uiHash1, uiHash3);
    NS_TEST_INT(uiHash1, uiHash4);

    // 64 Bit xxHash
    const nsUInt64 uiXXHash64 = nsHashingUtils::xxHash64(sb.GetData(), sb.GetElementCount());
    NS_TEST_INT(uiXXHash64, 0x141fb89c0bf32020);
    // Check xxHash64 for unaligned inputs
    uiHash1_64 = nsHashingUtils::xxHash64(alignmentTestString, 8);
    uiHash2_64 = nsHashingUtils::xxHash64(alignmentTestString + 9, 8);
    uiHash3_64 = nsHashingUtils::xxHash64(alignmentTestString + 19, 8);
    uiHash4_64 = nsHashingUtils::xxHash64(alignmentTestString + 30, 8);
    NS_TEST_INT(uiHash1_64, uiHash2_64);
    NS_TEST_INT(uiHash1_64, uiHash3_64);
    NS_TEST_INT(uiHash1_64, uiHash4_64);

    nsUInt32 uixxHash32RTEmpty = nsHashingUtils::xxHash32("", 0, 0);
    nsUInt32 uixxHash32CTEmpty = nsHashingUtils::xxHash32String("", 0);
    NS_TEST_BOOL(uixxHash32RTEmpty == uixxHash32CTEmpty);

    nsUInt64 uixxHash64RTEmpty = nsHashingUtils::xxHash64("", 0, 0);
    nsUInt64 uixxHash64CTEmpty = nsHashingUtils::xxHash64String("", 0);
    NS_TEST_BOOL(uixxHash64RTEmpty == uixxHash64CTEmpty);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "HashHelper")
  {
    nsUInt32 uiHash = nsHashHelper<nsStringBuilder>::Hash(sb);
    NS_TEST_INT(uiHash, 0x0bf32020);

    const char* szTest = "This is a test string. 1234";
    uiHash = nsHashHelper<const char*>::Hash(szTest);
    NS_TEST_INT(uiHash, 0x0bf32020);
    NS_TEST_BOOL(nsHashHelper<const char*>::Equal(szTest, sb.GetData()));

    nsHashedString hs;
    hs.Assign(szTest);
    uiHash = nsHashHelper<nsHashedString>::Hash(hs);
    NS_TEST_INT(uiHash, 0x0bf32020);

    nsTempHashedString ths(szTest);
    uiHash = nsHashHelper<nsHashedString>::Hash(ths);
    NS_TEST_INT(uiHash, 0x0bf32020);
    NS_TEST_BOOL(nsHashHelper<nsHashedString>::Equal(hs, ths));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "HashHelperString_NoCase")
  {
    const nsUInt32 uiHash = nsHashHelper<const char*>::Hash(szStringLower);
    NS_TEST_INT(uiHash, 0x19404167);
    NS_TEST_INT(uiHash, nsHashHelperString_NoCase::Hash(szString));
    NS_TEST_INT(uiHash, nsHashHelperString_NoCase::Hash(szStringLower));
    NS_TEST_INT(uiHash, nsHashHelperString_NoCase::Hash(szString2));
    NS_TEST_INT(uiHash, nsHashHelperString_NoCase::Hash(sb));
    nsStringBuilder sb2 = szString2;
    NS_TEST_INT(uiHash, nsHashHelperString_NoCase::Hash(sb2));
    nsString sL = szStringLower;
    nsString s1 = sb;
    nsString s2 = sb2;
    NS_TEST_INT(uiHash, nsHashHelperString_NoCase::Hash(s1));
    NS_TEST_INT(uiHash, nsHashHelperString_NoCase::Hash(s2));
    nsStringView svL = szStringLower;
    nsStringView sv1 = szString;
    nsStringView sv2 = szString2;
    NS_TEST_INT(uiHash, nsHashHelperString_NoCase::Hash(svL));
    NS_TEST_INT(uiHash, nsHashHelperString_NoCase::Hash(sv1));
    NS_TEST_INT(uiHash, nsHashHelperString_NoCase::Hash(sv2));

    NS_TEST_BOOL(nsHashHelperString_NoCase::Equal(sb, sb2));
    NS_TEST_BOOL(nsHashHelperString_NoCase::Equal(sb, szString2));
    NS_TEST_BOOL(nsHashHelperString_NoCase::Equal(sb, sv2));
    NS_TEST_BOOL(nsHashHelperString_NoCase::Equal(s1, sb2));
    NS_TEST_BOOL(nsHashHelperString_NoCase::Equal(s1, szString2));
    NS_TEST_BOOL(nsHashHelperString_NoCase::Equal(s1, sv2));
    NS_TEST_BOOL(nsHashHelperString_NoCase::Equal(sv1, sb2));
    NS_TEST_BOOL(nsHashHelperString_NoCase::Equal(sv1, szString2));
    NS_TEST_BOOL(nsHashHelperString_NoCase::Equal(sv1, sv2));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "HashStream32")
  {
    const char* szTest = "This is a test string. 1234";
    const char* szTestHalf1 = "This is a test";
    const char* szTestHalf2 = " string. 1234";

    auto test = [szTest, szTestHalf1, szTestHalf2](bool bFlush, nsUInt32* pHash)
    {
      nsHashStreamWriter32 writer1;
      writer1.WriteBytes(szTest, std::strlen(szTest)).IgnoreResult();
      if (bFlush)
      {
        writer1.Flush().IgnoreResult();
      }

      const nsUInt32 uiHash1 = writer1.GetHashValue();

      nsHashStreamWriter32 writer2;
      writer2.WriteBytes(szTestHalf1, std::strlen(szTestHalf1)).IgnoreResult();
      if (bFlush)
      {
        writer2.Flush().IgnoreResult();
      }

      writer2.WriteBytes(szTestHalf2, std::strlen(szTestHalf2)).IgnoreResult();
      if (bFlush)
      {
        writer2.Flush().IgnoreResult();
      }

      const nsUInt32 uiHash2 = writer2.GetHashValue();

      nsHashStreamWriter32 writer3;
      for (nsUInt64 i = 0; szTest[i] != 0; ++i)
      {
        writer3.WriteBytes(szTest + i, 1).IgnoreResult();

        if (bFlush)
        {
          writer3.Flush().IgnoreResult();
        }
      }
      const nsUInt32 uiHash3 = writer3.GetHashValue();

      NS_TEST_INT(uiHash1, uiHash2);
      NS_TEST_INT(uiHash1, uiHash3);

      *pHash = uiHash1;
    };

    nsUInt32 uiHash1 = 0, uiHash2 = 1;
    test(true, &uiHash1);
    test(false, &uiHash2);
    NS_TEST_INT(uiHash1, uiHash2);

    const nsUInt64 uiHash3 = nsHashingUtils::xxHash32(szTest, std::strlen(szTest));
    NS_TEST_INT(uiHash1, uiHash3);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "HashStream64")
  {
    const char* szTest = "This is a test string. 1234";
    const char* szTestHalf1 = "This is a test";
    const char* szTestHalf2 = " string. 1234";

    auto test = [szTest, szTestHalf1, szTestHalf2](bool bFlush, nsUInt64* pHash)
    {
      nsHashStreamWriter64 writer1;
      writer1.WriteBytes(szTest, std::strlen(szTest)).IgnoreResult();

      if (bFlush)
      {
        writer1.Flush().IgnoreResult();
      }

      const nsUInt64 uiHash1 = writer1.GetHashValue();

      nsHashStreamWriter64 writer2;
      writer2.WriteBytes(szTestHalf1, std::strlen(szTestHalf1)).IgnoreResult();
      if (bFlush)
        writer2.Flush().IgnoreResult();
      writer2.WriteBytes(szTestHalf2, std::strlen(szTestHalf2)).IgnoreResult();
      if (bFlush)
        writer2.Flush().IgnoreResult();

      const nsUInt64 uiHash2 = writer2.GetHashValue();

      nsHashStreamWriter64 writer3;
      for (nsUInt64 i = 0; szTest[i] != 0; ++i)
      {
        writer3.WriteBytes(szTest + i, 1).IgnoreResult();
        if (bFlush)
          writer3.Flush().IgnoreResult();
      }
      const nsUInt64 uiHash3 = writer3.GetHashValue();

      NS_TEST_INT(uiHash1, uiHash2);
      NS_TEST_INT(uiHash1, uiHash3);

      *pHash = uiHash1;
    };

    nsUInt64 uiHash1 = 0, uiHash2 = 1;
    test(true, &uiHash1);
    test(false, &uiHash2);
    NS_TEST_INT(uiHash1, uiHash2);

    const nsUInt64 uiHash3 = nsHashingUtils::xxHash64(szTest, std::strlen(szTest));
    NS_TEST_INT(uiHash1, uiHash3);
  }
}

struct SimpleHashableStruct : public nsHashableStruct<SimpleHashableStruct>
{
  nsUInt32 m_uiTestMember1;
  nsUInt8 m_uiTestMember2;
  nsUInt64 m_uiTestMember3;
};

struct SimpleStruct
{
  nsUInt32 m_uiTestMember1;
  nsUInt8 m_uiTestMember2;
  nsUInt64 m_uiTestMember3;
};

NS_CREATE_SIMPLE_TEST(Algorithm, HashableStruct)
{
  SimpleHashableStruct AutomaticInst;
  NS_TEST_INT(AutomaticInst.m_uiTestMember1, 0);
  NS_TEST_INT(AutomaticInst.m_uiTestMember2, 0);
  NS_TEST_INT(AutomaticInst.m_uiTestMember3, 0);

  SimpleStruct NonAutomaticInst;
  nsMemoryUtils::ZeroFill(&NonAutomaticInst, 1);

  NS_CHECK_AT_COMPILETIME(sizeof(AutomaticInst) == sizeof(NonAutomaticInst));

  NS_TEST_INT(nsMemoryUtils::Compare<nsUInt8>((nsUInt8*)&AutomaticInst, (nsUInt8*)&NonAutomaticInst, sizeof(AutomaticInst)), 0);

  AutomaticInst.m_uiTestMember2 = 0x42u;
  AutomaticInst.m_uiTestMember3 = 0x23u;

  nsUInt32 uiAutomaticHash = AutomaticInst.CalculateHash();

  NonAutomaticInst.m_uiTestMember2 = 0x42u;
  NonAutomaticInst.m_uiTestMember3 = 0x23u;

  nsUInt32 uiNonAutomaticHash = nsHashingUtils::xxHash32(&NonAutomaticInst, sizeof(NonAutomaticInst));

  NS_TEST_INT(uiAutomaticHash, uiNonAutomaticHash);

  AutomaticInst.m_uiTestMember1 = 0x5u;
  uiAutomaticHash = AutomaticInst.CalculateHash();

  NS_TEST_BOOL(uiAutomaticHash != uiNonAutomaticHash);
}

#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Containers/StaticArray.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/IO/Stream.h>

#include <Foundation/IO/StringDeduplicationContext.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Time/Stopwatch.h>

namespace
{
  struct SerializableStructWithMethods
  {

    NS_DECLARE_POD_TYPE();

    nsResult Serialize(nsStreamWriter& inout_stream) const
    {
      inout_stream << m_uiMember1;
      inout_stream << m_uiMember2;

      return NS_SUCCESS;
    }

    nsResult Deserialize(nsStreamReader& inout_stream)
    {
      inout_stream >> m_uiMember1;
      inout_stream >> m_uiMember2;

      return NS_SUCCESS;
    }

    nsInt32 m_uiMember1 = 0x42;
    nsInt32 m_uiMember2 = 0x23;
  };
} // namespace

NS_CREATE_SIMPLE_TEST(IO, StreamOperation)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Binary Stream Basic Operations (built-in types)")
  {
    nsDefaultMemoryStreamStorage StreamStorage(4096);

    // Create writer
    nsMemoryStreamWriter StreamWriter(&StreamStorage);

    StreamWriter << (nsUInt8)0x42;
    StreamWriter << (nsUInt16)0x4223;
    StreamWriter << (nsUInt32)0x42232342;
    StreamWriter << (nsUInt64)0x4223234242232342;
    StreamWriter << 42.0f;
    StreamWriter << 23.0;
    StreamWriter << (nsInt8)0x23;
    StreamWriter << (nsInt16)0x2342;
    StreamWriter << (nsInt32)0x23422342;
    StreamWriter << (nsInt64)0x2342234242232342;

    // Arrays
    {
      nsDynamicArray<nsUInt32> DynamicArray;
      DynamicArray.PushBack(42);
      DynamicArray.PushBack(23);
      DynamicArray.PushBack(13);
      DynamicArray.PushBack(5);
      DynamicArray.PushBack(0);

      StreamWriter.WriteArray(DynamicArray).IgnoreResult();
    }

    // Create reader
    nsMemoryStreamReader StreamReader(&StreamStorage);

    // Read back
    {
      nsUInt8 uiVal;
      StreamReader >> uiVal;
      NS_TEST_BOOL(uiVal == (nsUInt8)0x42);
    }
    {
      nsUInt16 uiVal;
      StreamReader >> uiVal;
      NS_TEST_BOOL(uiVal == (nsUInt16)0x4223);
    }
    {
      nsUInt32 uiVal;
      StreamReader >> uiVal;
      NS_TEST_BOOL(uiVal == (nsUInt32)0x42232342);
    }
    {
      nsUInt64 uiVal;
      StreamReader >> uiVal;
      NS_TEST_BOOL(uiVal == (nsUInt64)0x4223234242232342);
    }

    {
      float fVal;
      StreamReader >> fVal;
      NS_TEST_BOOL(fVal == 42.0f);
    }
    {
      double dVal;
      StreamReader >> dVal;
      NS_TEST_BOOL(dVal == 23.0f);
    }


    {
      nsInt8 iVal;
      StreamReader >> iVal;
      NS_TEST_BOOL(iVal == (nsInt8)0x23);
    }
    {
      nsInt16 iVal;
      StreamReader >> iVal;
      NS_TEST_BOOL(iVal == (nsInt16)0x2342);
    }
    {
      nsInt32 iVal;
      StreamReader >> iVal;
      NS_TEST_BOOL(iVal == (nsInt32)0x23422342);
    }
    {
      nsInt64 iVal;
      StreamReader >> iVal;
      NS_TEST_BOOL(iVal == (nsInt64)0x2342234242232342);
    }

    {
      nsDynamicArray<nsUInt32> ReadBackDynamicArray;

      // This element will be removed by the ReadArray function
      ReadBackDynamicArray.PushBack(0xAAu);

      StreamReader.ReadArray(ReadBackDynamicArray).IgnoreResult();

      NS_TEST_INT(ReadBackDynamicArray.GetCount(), 5);

      NS_TEST_INT(ReadBackDynamicArray[0], 42);
      NS_TEST_INT(ReadBackDynamicArray[1], 23);
      NS_TEST_INT(ReadBackDynamicArray[2], 13);
      NS_TEST_INT(ReadBackDynamicArray[3], 5);
      NS_TEST_INT(ReadBackDynamicArray[4], 0);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Binary Stream Arrays of Structs")
  {
    nsDefaultMemoryStreamStorage StreamStorage(4096);

    // Create writer
    nsMemoryStreamWriter StreamWriter(&StreamStorage);

    // Write out a couple of the structs
    {
      nsStaticArray<SerializableStructWithMethods, 16> WriteArray;
      WriteArray.ExpandAndGetRef().m_uiMember1 = 0x5;
      WriteArray.ExpandAndGetRef().m_uiMember1 = 0x6;

      StreamWriter.WriteArray(WriteArray).IgnoreResult();
    }

    // Read back in
    {
      // Create reader
      nsMemoryStreamReader StreamReader(&StreamStorage);

      // This intentionally uses a different array type for the read back
      // to verify that it is a) compatible and b) all arrays are somewhat tested
      nsHybridArray<SerializableStructWithMethods, 1> ReadArray;

      StreamReader.ReadArray(ReadArray).IgnoreResult();

      NS_TEST_INT(ReadArray.GetCount(), 2);

      NS_TEST_INT(ReadArray[0].m_uiMember1, 0x5);
      NS_TEST_INT(ReadArray[0].m_uiMember2, 0x23);

      NS_TEST_INT(ReadArray[1].m_uiMember1, 0x6);
      NS_TEST_INT(ReadArray[1].m_uiMember2, 0x23);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsSet Stream Operators")
  {
    nsDefaultMemoryStreamStorage StreamStorage(4096);

    // Create writer
    nsMemoryStreamWriter StreamWriter(&StreamStorage);

    nsSet<nsString> TestSet;
    TestSet.Insert("Hello");
    TestSet.Insert("World");
    TestSet.Insert("!");

    StreamWriter.WriteSet(TestSet).IgnoreResult();

    nsSet<nsString> TestSetReadBack;

    TestSetReadBack.Insert("Shouldn't be there after deserialization.");

    nsMemoryStreamReader StreamReader(&StreamStorage);

    StreamReader.ReadSet(TestSetReadBack).IgnoreResult();

    NS_TEST_INT(TestSetReadBack.GetCount(), 3);

    NS_TEST_BOOL(TestSetReadBack.Contains("Hello"));
    NS_TEST_BOOL(TestSetReadBack.Contains("!"));
    NS_TEST_BOOL(TestSetReadBack.Contains("World"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsMap Stream Operators")
  {
    nsDefaultMemoryStreamStorage StreamStorage(4096);

    // Create writer
    nsMemoryStreamWriter StreamWriter(&StreamStorage);

    nsMap<nsUInt64, nsString> TestMap;
    TestMap.Insert(42, "Hello");
    TestMap.Insert(23, "World");
    TestMap.Insert(5, "!");

    StreamWriter.WriteMap(TestMap).IgnoreResult();

    nsMap<nsUInt64, nsString> TestMapReadBack;

    TestMapReadBack.Insert(1, "Shouldn't be there after deserialization.");

    nsMemoryStreamReader StreamReader(&StreamStorage);

    StreamReader.ReadMap(TestMapReadBack).IgnoreResult();

    NS_TEST_INT(TestMapReadBack.GetCount(), 3);

    NS_TEST_BOOL(TestMapReadBack.Contains(42));
    NS_TEST_BOOL(TestMapReadBack.Contains(5));
    NS_TEST_BOOL(TestMapReadBack.Contains(23));

    NS_TEST_BOOL(TestMapReadBack.GetValue(42)->IsEqual("Hello"));
    NS_TEST_BOOL(TestMapReadBack.GetValue(5)->IsEqual("!"));
    NS_TEST_BOOL(TestMapReadBack.GetValue(23)->IsEqual("World"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsHashTable Stream Operators")
  {
    nsDefaultMemoryStreamStorage StreamStorage(4096);

    // Create writer
    nsMemoryStreamWriter StreamWriter(&StreamStorage);

    nsHashTable<nsUInt64, nsString> TestHashTable;
    TestHashTable.Insert(42, "Hello");
    TestHashTable.Insert(23, "World");
    TestHashTable.Insert(5, "!");

    StreamWriter.WriteHashTable(TestHashTable).IgnoreResult();

    nsMap<nsUInt64, nsString> TestHashTableReadBack;

    TestHashTableReadBack.Insert(1, "Shouldn't be there after deserialization.");

    nsMemoryStreamReader StreamReader(&StreamStorage);

    StreamReader.ReadMap(TestHashTableReadBack).IgnoreResult();

    NS_TEST_INT(TestHashTableReadBack.GetCount(), 3);

    NS_TEST_BOOL(TestHashTableReadBack.Contains(42));
    NS_TEST_BOOL(TestHashTableReadBack.Contains(5));
    NS_TEST_BOOL(TestHashTableReadBack.Contains(23));

    NS_TEST_BOOL(TestHashTableReadBack.GetValue(42)->IsEqual("Hello"));
    NS_TEST_BOOL(TestHashTableReadBack.GetValue(5)->IsEqual("!"));
    NS_TEST_BOOL(TestHashTableReadBack.GetValue(23)->IsEqual("World"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "String Deduplication")
  {
    nsDefaultMemoryStreamStorage StreamStorageNonDeduplicated(4096);
    nsDefaultMemoryStreamStorage StreamStorageDeduplicated(4096);

    nsHybridString<4> str1 = "Hello World";
    nsDynamicString str2 = "Hello World 2";
    nsStringBuilder str3 = "Hello Schlumpf";

    // Non deduplicated serialization
    {
      nsMemoryStreamWriter StreamWriter(&StreamStorageNonDeduplicated);

      StreamWriter << str1;
      StreamWriter << str2;
      StreamWriter << str1;
      StreamWriter << str3;
      StreamWriter << str1;
      StreamWriter << str2;
    }

    // Deduplicated serialization
    {
      nsMemoryStreamWriter StreamWriter(&StreamStorageDeduplicated);

      nsStringDeduplicationWriteContext StringDeduplicationContext(StreamWriter);
      auto& DeduplicationWriter = StringDeduplicationContext.Begin();

      DeduplicationWriter << str1;
      DeduplicationWriter << str2;
      DeduplicationWriter << str1;
      DeduplicationWriter << str3;
      DeduplicationWriter << str1;
      DeduplicationWriter << str2;

      StringDeduplicationContext.End().IgnoreResult();

      NS_TEST_INT(StringDeduplicationContext.GetUniqueStringCount(), 3);
    }

    NS_TEST_BOOL(StreamStorageDeduplicated.GetStorageSize64() < StreamStorageNonDeduplicated.GetStorageSize64());

    // Read the deduplicated strings back
    {
      nsMemoryStreamReader StreamReader(&StreamStorageDeduplicated);

      nsStringDeduplicationReadContext StringDeduplicationReadContext(StreamReader);

      nsHybridString<16> szRead0, szRead1, szRead2;
      nsStringBuilder szRead3, szRead4, szRead5;

      StreamReader >> szRead0;
      StreamReader >> szRead1;
      StreamReader >> szRead2;
      StreamReader >> szRead3;
      StreamReader >> szRead4;
      StreamReader >> szRead5;

      NS_TEST_STRING(szRead0, szRead2);
      NS_TEST_STRING(szRead0, szRead4);
      NS_TEST_STRING(szRead1, szRead5);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Array Serialization Performance (bytes)")
  {
    constexpr nsUInt32 uiCount = 1024 * 1024 * 10;

    nsContiguousMemoryStreamStorage storage(uiCount + 16);

    nsMemoryStreamWriter writer(&storage);
    nsMemoryStreamReader reader(&storage);

    nsDynamicArray<nsUInt8> DynamicArray;
    DynamicArray.SetCountUninitialized(uiCount);

    for (nsUInt32 i = 0; i < uiCount; ++i)
    {
      DynamicArray[i] = i & 0xFF;
    }

    {
      nsStopwatch sw;

      writer.WriteArray(DynamicArray).AssertSuccess();

      nsTime t = sw.GetRunningTotal();
      nsStringBuilder s;
      s.SetFormat("Write {} byte array: {}", nsArgFileSize(uiCount), t);
      nsTestFramework::Output(nsTestOutput::Details, s);
    }

    {
      nsStopwatch sw;

      reader.ReadArray(DynamicArray).IgnoreResult();

      nsTime t = sw.GetRunningTotal();
      nsStringBuilder s;
      s.SetFormat("Read {} byte array: {}", nsArgFileSize(uiCount), t);
      nsTestFramework::Output(nsTestOutput::Details, s);
    }

    for (nsUInt32 i = 0; i < uiCount; ++i)
    {
      NS_TEST_INT(DynamicArray[i], i & 0xFF);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Array Serialization Performance (nsVec3)")
  {
    constexpr nsUInt32 uiCount = 1024 * 1024 * 10;

    nsContiguousMemoryStreamStorage storage(uiCount * sizeof(nsVec3) + 16);

    nsMemoryStreamWriter writer(&storage);
    nsMemoryStreamReader reader(&storage);

    nsDynamicArray<nsVec3> DynamicArray;
    DynamicArray.SetCountUninitialized(uiCount);

    for (nsUInt32 i = 0; i < uiCount; ++i)
    {
      DynamicArray[i].Set(i, i + 1, i + 2);
    }

    {
      nsStopwatch sw;

      writer.WriteArray(DynamicArray).AssertSuccess();

      nsTime t = sw.GetRunningTotal();
      nsStringBuilder s;
      s.SetFormat("Write {} vec3 array: {}", nsArgFileSize(uiCount * sizeof(nsVec3)), t);
      nsTestFramework::Output(nsTestOutput::Details, s);
    }

    {
      nsStopwatch sw;

      reader.ReadArray(DynamicArray).AssertSuccess();

      nsTime t = sw.GetRunningTotal();
      nsStringBuilder s;
      s.SetFormat("Read {} vec3 array: {}", nsArgFileSize(uiCount * sizeof(nsVec3)), t);
      nsTestFramework::Output(nsTestOutput::Details, s);
    }

    for (nsUInt32 i = 0; i < uiCount; ++i)
    {
      NS_TEST_VEC3(DynamicArray[i], nsVec3(i, i + 1, i + 2), 0.01f);
    }
  }
}

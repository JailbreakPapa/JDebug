#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Algorithm/HashingUtils.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/IO/MemoryStream.h>

NS_CREATE_SIMPLE_TEST_GROUP(IO);

NS_CREATE_SIMPLE_TEST(IO, MemoryStream)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Memory Stream Reading / Writing")
  {
    nsDefaultMemoryStreamStorage StreamStorage;

    // Create reader
    nsMemoryStreamReader StreamReader(&StreamStorage);

    // Create writer
    nsMemoryStreamWriter StreamWriter(&StreamStorage);

    // Temp read pointer
    nsUInt8* pPointer = reinterpret_cast<nsUInt8*>(0x41); // Should crash when accessed

    // Try reading from an empty stream (should not crash, just return 0 bytes read)
    nsUInt64 uiBytesRead = StreamReader.ReadBytes(pPointer, 128);

    NS_TEST_BOOL(uiBytesRead == 0);


    // Now try writing data to the stream and reading it back
    nsUInt32 uiData[1024];
    for (nsUInt32 i = 0; i < 1024; i++)
      uiData[i] = rand();

    // Calculate the hash so we can reuse the array
    const nsUInt32 uiHashBeforeWriting = nsHashingUtils::xxHash32(uiData, sizeof(nsUInt32) * 1024);

    // Write the data
    NS_TEST_BOOL(StreamWriter.WriteBytes(reinterpret_cast<const nsUInt8*>(uiData), sizeof(nsUInt32) * 1024) == NS_SUCCESS);

    NS_TEST_BOOL(StreamWriter.GetByteCount64() == sizeof(nsUInt32) * 1024);
    NS_TEST_BOOL(StreamWriter.GetByteCount64() == StreamReader.GetByteCount64());
    NS_TEST_BOOL(StreamWriter.GetByteCount64() == StreamStorage.GetStorageSize64());


    // Clear the array for the read back
    nsMemoryUtils::ZeroFill(uiData, 1024);

    uiBytesRead = StreamReader.ReadBytes(reinterpret_cast<nsUInt8*>(uiData), sizeof(nsUInt32) * 1024);

    NS_TEST_BOOL(uiBytesRead == sizeof(nsUInt32) * 1024);

    const nsUInt32 uiHashAfterReading = nsHashingUtils::xxHash32(uiData, sizeof(nsUInt32) * 1024);

    NS_TEST_BOOL(uiHashAfterReading == uiHashBeforeWriting);

    // Modify data and test the Rewind() functionality of the writer
    uiData[0] = 0x42;
    uiData[1] = 0x23;

    const nsUInt32 uiHashOfModifiedData = nsHashingUtils::xxHash32(uiData, sizeof(nsUInt32) * 4); // Only test the first 4 elements now

    StreamWriter.SetWritePosition(0);

    StreamWriter.WriteBytes(uiData, sizeof(nsUInt32) * 4).IgnoreResult();

    // Clear the array for the read back
    nsMemoryUtils::ZeroFill(uiData, 4);

    // Test the rewind of the reader as well
    StreamReader.SetReadPosition(0);

    uiBytesRead = StreamReader.ReadBytes(uiData, sizeof(nsUInt32) * 4);

    NS_TEST_BOOL(uiBytesRead == sizeof(nsUInt32) * 4);

    const nsUInt32 uiHashAfterReadingOfModifiedData = nsHashingUtils::xxHash32(uiData, sizeof(nsUInt32) * 4);

    NS_TEST_BOOL(uiHashAfterReadingOfModifiedData == uiHashOfModifiedData);

    // Test skipping
    StreamReader.SetReadPosition(0);

    StreamReader.SkipBytes(sizeof(nsUInt32));

    nsUInt32 uiTemp;

    uiBytesRead = StreamReader.ReadBytes(&uiTemp, sizeof(nsUInt32));

    NS_TEST_BOOL(uiBytesRead == sizeof(nsUInt32));

    // We skipped over the first 0x42 element, so this should be 0x23
    NS_TEST_BOOL(uiTemp == 0x23);

    // Skip more bytes than available
    nsUInt64 uiBytesSkipped = StreamReader.SkipBytes(0xFFFFFFFFFF);

    NS_TEST_BOOL(uiBytesSkipped < 0xFFFFFFFFFF);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Raw Memory Stream Reading")
  {
    nsDynamicArray<nsUInt8> OrigStorage;
    OrigStorage.SetCountUninitialized(1000);

    for (nsUInt32 i = 0; i < 1000; ++i)
    {
      OrigStorage[i] = i % 256;
    }

    {
      nsRawMemoryStreamReader reader(OrigStorage);

      nsDynamicArray<nsUInt8> CopyStorage;
      CopyStorage.SetCountUninitialized(static_cast<nsUInt32>(reader.GetByteCount()));
      reader.ReadBytes(CopyStorage.GetData(), reader.GetByteCount());

      NS_TEST_BOOL(OrigStorage == CopyStorage);
    }

    {
      nsRawMemoryStreamReader reader(OrigStorage.GetData() + 510, 490);

      nsDynamicArray<nsUInt8> CopyStorage;
      CopyStorage.SetCountUninitialized(static_cast<nsUInt32>(reader.GetByteCount()));
      reader.ReadBytes(CopyStorage.GetData(), reader.GetByteCount());

      NS_TEST_BOOL(OrigStorage != CopyStorage);

      for (nsUInt32 i = 0; i < 490; ++i)
      {
        CopyStorage[i] = (i + 10) % 256;
      }
    }

    {
      nsRawMemoryStreamReader reader(OrigStorage.GetData(), 1000);
      reader.SkipBytes(510);

      nsDynamicArray<nsUInt8> CopyStorage;
      CopyStorage.SetCountUninitialized(490);
      reader.ReadBytes(CopyStorage.GetData(), 490);

      NS_TEST_BOOL(OrigStorage != CopyStorage);

      for (nsUInt32 i = 0; i < 490; ++i)
      {
        CopyStorage[i] = (i + 10) % 256;
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Raw Memory Stream Writing")
  {
    nsDynamicArray<nsUInt8> OrigStorage;
    OrigStorage.SetCountUninitialized(1000);

    nsRawMemoryStreamWriter writer0;
    NS_TEST_INT(writer0.GetNumWrittenBytes(), 0);
    NS_TEST_INT(writer0.GetStorageSize(), 0);

    nsRawMemoryStreamWriter writer(OrigStorage.GetData(), OrigStorage.GetCount());

    for (nsUInt32 i = 0; i < 1000; ++i)
    {
      writer << static_cast<nsUInt8>(i % 256);

      NS_TEST_INT(writer.GetNumWrittenBytes(), i + 1);
      NS_TEST_INT(writer.GetStorageSize(), 1000);
    }

    for (nsUInt32 i = 0; i < 1000; ++i)
    {
      NS_TEST_INT(OrigStorage[i], i % 256);
    }

    {
      nsRawMemoryStreamWriter writer2(OrigStorage);
      NS_TEST_INT(writer2.GetNumWrittenBytes(), 0);
      NS_TEST_INT(writer2.GetStorageSize(), 1000);
    }
  }
}

NS_CREATE_SIMPLE_TEST(IO, LargeMemoryStream)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Large Memory Stream Reading / Writing")
  {
    nsDefaultMemoryStreamStorage storage;
    nsMemoryStreamWriter writer(&storage);
    nsMemoryStreamReader reader(&storage);

    const nsUInt8 pattern[] = {11, 10, 27, 4, 14, 3, 21, 6};

    nsUInt64 uiSize = 0;
    constexpr nsUInt64 bytesToTest = 0x8000000llu; // tested with up to 8 GB, but that just takes too long

    // writes n gigabyte
    for (nsUInt32 n = 0; n < 8; ++n)
    {
      // writes one gigabyte
      for (nsUInt32 gb = 0; gb < 1024; ++gb)
      {
        // writes one megabyte
        for (nsUInt32 mb = 0; mb < 1024 * 1024 / NS_ARRAY_SIZE(pattern); ++mb)
        {
          writer.WriteBytes(pattern, NS_ARRAY_SIZE(pattern)).IgnoreResult();
          uiSize += NS_ARRAY_SIZE(pattern);

          if (uiSize == bytesToTest)
            goto check;
        }
      }
    }

  check:
    NS_TEST_BOOL(uiSize == bytesToTest);
    NS_TEST_BOOL(writer.GetWritePosition() == bytesToTest);
    uiSize = 0;

    // reads n gigabyte
    for (nsUInt32 n = 0; n < 8; ++n)
    {
      // reads one gigabyte
      for (nsUInt32 gb = 0; gb < 1024; ++gb)
      {
        // reads one megabyte
        for (nsUInt32 mb = 0; mb < 1024 * 1024 / NS_ARRAY_SIZE(pattern); ++mb)
        {
          nsUInt8 pattern2[NS_ARRAY_SIZE(pattern)];

          const nsUInt64 uiRead = reader.ReadBytes(pattern2, NS_ARRAY_SIZE(pattern));

          if (uiRead != NS_ARRAY_SIZE(pattern))
          {
            NS_TEST_BOOL(uiRead == 0);
            NS_TEST_BOOL(uiSize == bytesToTest);
            goto endTest;
          }

          uiSize += uiRead;

          if (nsMemoryUtils::RawByteCompare(pattern, pattern2, NS_ARRAY_SIZE(pattern)) != 0)
          {
            NS_TEST_BOOL_MSG(false, "Memory read comparison failed.");
            goto endTest;
          }
        }
      }
    }

  endTest:;
    NS_TEST_BOOL(reader.GetReadPosition() == bytesToTest);
  }
}

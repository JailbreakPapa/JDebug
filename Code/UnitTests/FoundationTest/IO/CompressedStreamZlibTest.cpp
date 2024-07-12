#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/IO/CompressedStreamZlib.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/IO/Stream.h>

#ifdef BUILDSYSTEM_ENABLE_ZLIB_SUPPORT

NS_CREATE_SIMPLE_TEST(IO, CompressedStreamZlib)
{
  nsDynamicArray<nsUInt32> TestData;

  // create the test data
  // a repetition of a counting sequence that is getting longer and longer, ie:
  // 0, 0,1, 0,1,2, 0,1,2,3, 0,1,2,3,4, ...
  {
    TestData.SetCountUninitialized(1024 * 1024 * 8);

    const nsUInt32 uiItems = TestData.GetCount();
    nsUInt32 uiStartPos = 0;

    for (nsUInt32 uiWrite = 1; uiWrite < uiItems; ++uiWrite)
    {
      uiWrite = nsMath::Min(uiWrite, uiItems - uiStartPos);

      if (uiWrite == 0)
        break;

      for (nsUInt32 i = 0; i < uiWrite; ++i)
      {
        TestData[uiStartPos + i] = i;
      }

      uiStartPos += uiWrite;
    }
  }


  nsDefaultMemoryStreamStorage StreamStorage;

  nsMemoryStreamWriter MemoryWriter(&StreamStorage);
  nsMemoryStreamReader MemoryReader(&StreamStorage);

  nsCompressedStreamReaderZlib CompressedReader(&MemoryReader);
  nsCompressedStreamWriterZlib CompressedWriter(&MemoryWriter);

  const float fExpectedCompressionRatio = 25.0f; // this is a guess that is based on the current input data and size

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Compress Data")
  {
    nsUInt32 uiWrite = 1;
    for (nsUInt32 i = 0; i < TestData.GetCount();)
    {
      uiWrite = nsMath::Min<nsUInt32>(uiWrite, TestData.GetCount() - i);

      NS_TEST_BOOL(CompressedWriter.WriteBytes(&TestData[i], sizeof(nsUInt32) * uiWrite) == NS_SUCCESS);

      i += uiWrite;
      uiWrite += 17; // try different sizes to write
    }

    // flush all data
    CompressedWriter.CloseStream().IgnoreResult();

    const nsUInt64 uiCompressed = CompressedWriter.GetCompressedSize();
    const nsUInt64 uiUncompressed = CompressedWriter.GetUncompressedSize();

    NS_TEST_INT(uiUncompressed, TestData.GetCount() * sizeof(nsUInt32));

    NS_TEST_BOOL((float)uiCompressed <= (float)uiUncompressed / fExpectedCompressionRatio);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Uncompress Data")
  {
    bool bSkip = false;
    nsUInt32 uiStartPos = 0;

    nsDynamicArray<nsUInt32> TestDataRead = TestData; // initialize with identical data, makes comparing the skipped parts easier

    // read the data in blocks that get larger and larger
    for (nsUInt32 iRead = 1; iRead < TestData.GetCount(); ++iRead)
    {
      nsUInt32 iToRead = nsMath::Min(iRead, TestData.GetCount() - uiStartPos);

      if (iToRead == 0)
        break;

      if (bSkip)
      {
        const nsUInt64 uiReadFromStream = CompressedReader.SkipBytes(sizeof(nsUInt32) * iToRead);
        NS_TEST_BOOL(uiReadFromStream == sizeof(nsUInt32) * iToRead);
      }
      else
      {
        // overwrite part we are going to read from the stream, to make sure it re-reads the correct data
        for (nsUInt32 i = 0; i < iToRead; ++i)
        {
          TestDataRead[uiStartPos + i] = 0;
        }

        const nsUInt64 uiReadFromStream = CompressedReader.ReadBytes(&TestDataRead[uiStartPos], sizeof(nsUInt32) * iToRead);
        NS_TEST_BOOL(uiReadFromStream == sizeof(nsUInt32) * iToRead);
      }

      bSkip = !bSkip;

      uiStartPos += iToRead;
    }

    NS_TEST_BOOL(TestData == TestDataRead);

    // test reading after the end of the stream
    for (nsUInt32 i = 0; i < 1000; ++i)
    {
      nsUInt32 uiTemp = 0;
      NS_TEST_BOOL(CompressedReader.ReadBytes(&uiTemp, sizeof(nsUInt32)) == 0);
    }
  }
}

#endif

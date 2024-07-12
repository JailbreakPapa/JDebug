#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/IO/FileSystem/DataDirTypeFolder.h>
#include <Foundation/IO/FileSystem/DeferredFileWriter.h>
#include <Foundation/IO/FileSystem/FileReader.h>

NS_CREATE_SIMPLE_TEST(IO, DeferredFileWriter)
{
  NS_TEST_BOOL(nsFileSystem::AddDataDirectory("", "", ":", nsFileSystem::AllowWrites) == NS_SUCCESS);

  const nsStringBuilder szOutputFolder = nsTestFramework::GetInstance()->GetAbsOutputPath();
  nsStringBuilder sOutputFolderResolved;
  nsFileSystem::ResolveSpecialDirectory(szOutputFolder, sOutputFolderResolved).IgnoreResult();

  nsStringBuilder sTempFile = sOutputFolderResolved;
  sTempFile.AppendPath("Temp.tmp");

  // make sure the file does not exist
  nsFileSystem::DeleteFile(sTempFile);
  NS_TEST_BOOL(!nsFileSystem::ExistsFile(sTempFile));

  NS_TEST_BLOCK(nsTestBlock::Enabled, "DeferredFileWriter")
  {
    nsDeferredFileWriter writer;
    writer.SetOutput(sTempFile);

    for (nsUInt64 i = 0; i < 1'000'000; ++i)
    {
      writer << i;
    }

    // does not exist yet
    NS_TEST_BOOL(!nsFileSystem::ExistsFile(sTempFile));
  }

  // now it exists
  NS_TEST_BOOL(nsFileSystem::ExistsFile(sTempFile));

  // check content is correct
  {
    nsFileReader reader;
    NS_TEST_BOOL(reader.Open(sTempFile).Succeeded());

    for (nsUInt64 i = 0; i < 1'000'000; ++i)
    {
      nsUInt64 v;
      reader >> v;
      NS_TEST_BOOL(v == i);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "DeferredFileWriter2")
  {
    nsDeferredFileWriter writer;
    writer.SetOutput(sTempFile);

    for (nsUInt64 i = 1; i < 100'000; ++i)
    {
      writer << i;
    }

    // does exist from earlier
    NS_TEST_BOOL(nsFileSystem::ExistsFile(sTempFile));

    // check content is as previous correct
    {
      nsFileReader reader;
      NS_TEST_BOOL(reader.Open(sTempFile).Succeeded());

      for (nsUInt64 i = 0; i < 1'000'000; ++i)
      {
        nsUInt64 v;
        reader >> v;
        NS_TEST_BOOL(v == i);
      }
    }
  }

  // exist but now was overwritten
  NS_TEST_BOOL(nsFileSystem::ExistsFile(sTempFile));

  // check content is as previous correct
  {
    nsFileReader reader;
    NS_TEST_BOOL(reader.Open(sTempFile).Succeeded());

    for (nsUInt64 i = 1; i < 100'000; ++i)
    {
      nsUInt64 v;
      reader >> v;
      NS_TEST_BOOL(v == i);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Discard")
  {
    nsStringBuilder sTempFile2 = sOutputFolderResolved;
    sTempFile2.AppendPath("Temp2.tmp");
    {
      nsDeferredFileWriter writer;
      writer.SetOutput(sTempFile2);
      writer << 10;
      writer.Discard();
    }
    NS_TEST_BOOL(!nsFileSystem::ExistsFile(sTempFile2));
  }

  nsFileSystem::DeleteFile(sTempFile);
  nsFileSystem::ClearAllDataDirectories();
}

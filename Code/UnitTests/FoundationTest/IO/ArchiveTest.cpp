#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/IO/Archive/Archive.h>
#include <Foundation/IO/Archive/DataDirTypeArchive.h>
#include <Foundation/IO/FileSystem/DataDirTypeFolder.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/System/Process.h>
#include <Foundation/Utilities/CommandLineUtils.h>

#if (NS_ENABLED(NS_SUPPORTS_FILE_ITERATORS) && NS_ENABLED(NS_SUPPORTS_FILE_STATS) && defined(BUILDSYSTEM_HAS_ARCHIVE_TOOL))

NS_CREATE_SIMPLE_TEST(IO, Archive)
{
  nsStringBuilder sOutputFolder = nsTestFramework::GetInstance()->GetAbsOutputPath();
  sOutputFolder.AppendPath("ArchiveTest");
  sOutputFolder.MakeCleanPath();

  // make sure it is empty
  nsOSFile::DeleteFolder(sOutputFolder).IgnoreResult();
  nsOSFile::CreateDirectoryStructure(sOutputFolder).IgnoreResult();

  if (!NS_TEST_BOOL(nsFileSystem::AddDataDirectory(sOutputFolder, "Clear", "output", nsFileSystem::AllowWrites).Succeeded()))
    return;

  const char* szTestData = "TestData";
  const char* szUnpackedData = "Unpacked";

  // write a couple of files for packaging
  const char* szFileList[] = {
    "File1.txt",
    "FolderA/File2.jpg",         // should get stored uncompressed
    "FolderB/File3.txt",
    "FolderA/FolderC/File4.zip", // should get stored uncompressed
    "FolderA/FolderD/File5.txt",
    "File6.txt",
  };

  const nsUInt32 uiMinFileSize = 1024 * 128;


  NS_TEST_BLOCK(nsTestBlock::Enabled, "Generate Data")
  {
    nsUInt64 uiValue = 0;

    nsStringBuilder fileName;

    for (nsUInt32 uiFileIdx = 0; uiFileIdx < NS_ARRAY_SIZE(szFileList); ++uiFileIdx)
    {
      fileName.Set(":output/", szTestData, "/", szFileList[uiFileIdx]);

      nsFileWriter file;
      if (!NS_TEST_BOOL(file.Open(fileName).Succeeded()))
        return;

      for (nsUInt32 i = 0; i < uiMinFileSize * uiFileIdx; ++i)
      {
        file << uiValue;
        ++uiValue;
      }
    }
  }

  const nsStringBuilder sArchiveFolder(sOutputFolder, "/", szTestData);
  const nsStringBuilder sUnpackFolder(sOutputFolder, "/", szUnpackedData);
  const nsStringBuilder sArchiveFile(sOutputFolder, "/", szTestData, ".nsArchive");

  nsStringBuilder pathToArchiveTool = nsCommandLineUtils::GetGlobalInstance()->GetParameter(0);
  pathToArchiveTool.PathParentDirectory();
  pathToArchiveTool.AppendPath("nsArchiveTool");
#  if NS_ENABLED(NS_PLATFORM_WINDOWS)
  pathToArchiveTool.Append(".exe");
#  endif
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Create a Package")
  {

    nsProcessOptions opt;
    opt.m_sProcess = pathToArchiveTool;
    opt.m_Arguments.PushBack(sArchiveFolder);

    nsInt32 iReturnValue = 1;

    nsProcess ArchiveToolProc;
    if (!NS_TEST_BOOL(ArchiveToolProc.Execute(opt, &iReturnValue).Succeeded()))
      return;

    NS_TEST_INT(iReturnValue, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Unpack the Package")
  {

    nsProcessOptions opt;
    opt.m_sProcess = pathToArchiveTool;
    opt.m_Arguments.PushBack("-unpack");
    opt.m_Arguments.PushBack(sArchiveFile);
    opt.m_Arguments.PushBack("-out");
    opt.m_Arguments.PushBack(sUnpackFolder);

    nsInt32 iReturnValue = 1;

    nsProcess ArchiveToolProc;
    if (!NS_TEST_BOOL(ArchiveToolProc.Execute(opt, &iReturnValue).Succeeded()))
      return;

    NS_TEST_INT(iReturnValue, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Compare unpacked data")
  {
    nsUInt64 uiValue = 0;

    nsStringBuilder sFileSrc;
    nsStringBuilder sFileDst;

    for (nsUInt32 uiFileIdx = 0; uiFileIdx < NS_ARRAY_SIZE(szFileList); ++uiFileIdx)
    {
      sFileSrc.Set(sOutputFolder, "/", szTestData, "/", szFileList[uiFileIdx]);
      sFileDst.Set(sOutputFolder, "/", szUnpackedData, "/", szFileList[uiFileIdx]);

      NS_TEST_FILES(sFileSrc, sFileDst, "Unpacked file should be identical");
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Mount as Data Dir")
  {
    if (!NS_TEST_BOOL(nsFileSystem::AddDataDirectory(sArchiveFile, "Clear", "archive", nsFileSystem::ReadOnly) == NS_SUCCESS))
      return;

    nsStringBuilder sFileSrc;
    nsStringBuilder sFileDst;

    // test opening multiple files in parallel and keeping them open
    nsFileReader readers[NS_ARRAY_SIZE(szFileList)];
    for (nsUInt32 uiFileIdx = 0; uiFileIdx < NS_ARRAY_SIZE(szFileList); ++uiFileIdx)
    {
      sFileDst.Set(":archive/", szFileList[uiFileIdx]);
      NS_TEST_BOOL(readers[uiFileIdx].Open(sFileDst).Succeeded());

      // advance the reader a bit
      NS_TEST_INT(readers[uiFileIdx].SkipBytes(uiMinFileSize * uiFileIdx), uiMinFileSize * uiFileIdx);
    }

    for (nsUInt32 uiFileIdx = 0; uiFileIdx < NS_ARRAY_SIZE(szFileList); ++uiFileIdx)
    {
      sFileSrc.Set(":output/", szTestData, "/", szFileList[uiFileIdx]);
      sFileDst.Set(":archive/", szFileList[uiFileIdx]);

      NS_TEST_FILES(sFileSrc, sFileDst, "Unpacked file should be identical");
    }

    // mount a second time
    if (!NS_TEST_BOOL(nsFileSystem::AddDataDirectory(sArchiveFile, "Clear", "archive2", nsFileSystem::ReadOnly) == NS_SUCCESS))
      return;
  }

  nsFileSystem::RemoveDataDirectoryGroup("Clear");
}

#endif

#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/IO/FileSystem/DataDirTypeFolder.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/FileSystem/FileWriter.h>

#if NS_ENABLED(NS_SUPPORTS_LONG_PATHS)
#  define LongPath                                                                                                                                   \
    "AVeryLongSubFolderPathNameThatShouldExceedThePathLengthLimitOnPlatformsLikeWindowsWhereOnly260CharactersAreAllowedOhNoesIStillNeedMoreThisIsNo" \
    "tLongEnoughAaaaaaaaaaaaaaahhhhStillTooShortAaaaaaaaaaaaaaaaaaaaaahImBoredNow"
#else
#  define LongPath "AShortPathBecaueThisPlatformDoesntSupportLongOnes"
#endif

NS_CREATE_SIMPLE_TEST(IO, FileSystem)
{
  nsStringBuilder sFileContent = "Lyrics to Taste The Cake:\n\
Turret: Who's there?\n\
Turret: Is anyone there?\n\
Turret: I see you.\n\
\n\
Chell rises from a stasis inside of a glass box\n\
She isn't greeted by faces,\n\
Only concrete and clocks.\n\
...";

  nsStringBuilder szOutputFolder = nsTestFramework::GetInstance()->GetAbsOutputPath();
  szOutputFolder.MakeCleanPath();

  nsStringBuilder sOutputFolderResolved;
  nsFileSystem::ResolveSpecialDirectory(szOutputFolder, sOutputFolderResolved).IgnoreResult();

  nsStringBuilder sOutputFolder1 = szOutputFolder;
  sOutputFolder1.AppendPath("IO", "SubFolder");
  nsStringBuilder sOutputFolder1Resolved;
  nsFileSystem::ResolveSpecialDirectory(sOutputFolder1, sOutputFolder1Resolved).IgnoreResult();

  nsStringBuilder sOutputFolder2 = szOutputFolder;
  sOutputFolder2.AppendPath("IO", "SubFolder2");
  nsStringBuilder sOutputFolder2Resolved;
  nsFileSystem::ResolveSpecialDirectory(sOutputFolder2, sOutputFolder2Resolved).IgnoreResult();

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Setup Data Dirs")
  {
    // adding the same factory three times would actually not make a difference
    nsFileSystem::RegisterDataDirectoryFactory(nsDataDirectory::FolderType::Factory);
    nsFileSystem::RegisterDataDirectoryFactory(nsDataDirectory::FolderType::Factory);
    nsFileSystem::RegisterDataDirectoryFactory(nsDataDirectory::FolderType::Factory);

    // nsFileSystem::ClearAllDataDirectoryFactories();

    nsFileSystem::RegisterDataDirectoryFactory(nsDataDirectory::FolderType::Factory);

    // for absolute paths
    NS_TEST_BOOL(nsFileSystem::AddDataDirectory("", "", ":", nsFileSystem::AllowWrites) == NS_SUCCESS);
    NS_TEST_BOOL(nsFileSystem::AddDataDirectory(szOutputFolder, "Clear", "output", nsFileSystem::AllowWrites) == NS_SUCCESS);

    nsStringBuilder sTempFile = sOutputFolder1Resolved;
    sTempFile.AppendPath(LongPath);
    sTempFile.AppendPath("Temp.tmp");

    nsFileWriter TempFile;
    NS_TEST_BOOL(TempFile.Open(sTempFile) == NS_SUCCESS);
    TempFile.Close();

    sTempFile = sOutputFolder2Resolved;
    sTempFile.AppendPath("Temp.tmp");

    NS_TEST_BOOL(TempFile.Open(sTempFile) == NS_SUCCESS);
    TempFile.Close();

    NS_TEST_BOOL(nsFileSystem::AddDataDirectory(sOutputFolder1, "Clear", "output1", nsFileSystem::AllowWrites) == NS_SUCCESS);
    NS_TEST_BOOL(nsFileSystem::AddDataDirectory(sOutputFolder2, "Clear") == NS_SUCCESS);

    NS_TEST_BOOL(nsFileSystem::AddDataDirectory(sOutputFolder2, "Remove", "output2", nsFileSystem::AllowWrites) == NS_SUCCESS);
    NS_TEST_BOOL(nsFileSystem::AddDataDirectory(sOutputFolder1, "Remove") == NS_SUCCESS);
    NS_TEST_BOOL(nsFileSystem::AddDataDirectory(sOutputFolder2, "Remove") == NS_SUCCESS);

    NS_TEST_INT(nsFileSystem::RemoveDataDirectoryGroup("Remove"), 3);

    NS_TEST_BOOL(nsFileSystem::AddDataDirectory(sOutputFolder2, "Remove", "output2", nsFileSystem::AllowWrites) == NS_SUCCESS);
    NS_TEST_BOOL(nsFileSystem::AddDataDirectory(sOutputFolder1, "Remove") == NS_SUCCESS);
    NS_TEST_BOOL(nsFileSystem::AddDataDirectory(sOutputFolder2, "Remove") == NS_SUCCESS);

    nsFileSystem::ClearAllDataDirectories();

    NS_TEST_INT(nsFileSystem::RemoveDataDirectoryGroup("Remove"), 0);
    NS_TEST_INT(nsFileSystem::RemoveDataDirectoryGroup("Clear"), 0);

    NS_TEST_BOOL(nsFileSystem::AddDataDirectory(sOutputFolder1, "", "output1", nsFileSystem::AllowWrites) == NS_SUCCESS);
    NS_TEST_BOOL(nsFileSystem::AddDataDirectory(sOutputFolder2) == NS_SUCCESS);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Add / Remove Data Dirs")
  {
    NS_TEST_BOOL(nsFileSystem::AddDataDirectory("", "xyz-rooted", "xyz", nsFileSystem::AllowWrites) == NS_SUCCESS);

    NS_TEST_BOOL(nsFileSystem::FindDataDirectoryWithRoot("xyz") != nullptr);

    NS_TEST_BOOL(nsFileSystem::RemoveDataDirectory("xyz") == true);

    NS_TEST_BOOL(nsFileSystem::FindDataDirectoryWithRoot("xyz") == nullptr);

    NS_TEST_BOOL(nsFileSystem::RemoveDataDirectory("xyz") == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Write File")
  {
    nsFileWriter FileOut;

    nsStringBuilder sAbs = sOutputFolder1Resolved;
    sAbs.AppendPath("FileSystemTest.txt");

    NS_TEST_BOOL(FileOut.Open(":output1/FileSystemTest.txt") == NS_SUCCESS);

    NS_TEST_STRING(FileOut.GetFilePathRelative(), "FileSystemTest.txt");
    NS_TEST_STRING(FileOut.GetFilePathAbsolute(), sAbs);

    NS_TEST_INT(FileOut.GetFileSize(), 0);

    NS_TEST_BOOL(FileOut.WriteBytes(sFileContent.GetData(), sFileContent.GetElementCount()) == NS_SUCCESS);

    FileOut.Flush().IgnoreResult();
    NS_TEST_INT(FileOut.GetFileSize(), sFileContent.GetElementCount());

    FileOut.Close();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Read File")
  {
    nsFileReader FileIn;

    nsStringBuilder sAbs = sOutputFolder1Resolved;
    sAbs.AppendPath("FileSystemTest.txt");

    NS_TEST_BOOL(FileIn.Open("FileSystemTest.txt") == NS_SUCCESS);

    NS_TEST_STRING(FileIn.GetFilePathRelative(), "FileSystemTest.txt");
    NS_TEST_STRING(FileIn.GetFilePathAbsolute(), sAbs);

    NS_TEST_INT(FileIn.GetFileSize(), sFileContent.GetElementCount());

    char szTemp[1024 * 2];
    NS_TEST_INT(FileIn.ReadBytes(szTemp, 1024 * 2), sFileContent.GetElementCount());

    NS_TEST_BOOL(nsMemoryUtils::IsEqual(szTemp, sFileContent.GetData(), sFileContent.GetElementCount()));

    FileIn.Close();
  }

#if NS_DISABLED(NS_PLATFORM_WINDOWS_UWP)

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Read File (Absolute Path)")
  {
    nsFileReader FileIn;

    nsStringBuilder sAbs = sOutputFolder1Resolved;
    sAbs.AppendPath("FileSystemTest.txt");

    NS_TEST_BOOL(FileIn.Open(sAbs) == NS_SUCCESS);

    NS_TEST_STRING(FileIn.GetFilePathRelative(), "FileSystemTest.txt");
    NS_TEST_STRING(FileIn.GetFilePathAbsolute(), sAbs);

    NS_TEST_INT(FileIn.GetFileSize(), sFileContent.GetElementCount());

    char szTemp[1024 * 2];
    NS_TEST_INT(FileIn.ReadBytes(szTemp, 1024 * 2), sFileContent.GetElementCount());

    NS_TEST_BOOL(nsMemoryUtils::IsEqual(szTemp, sFileContent.GetData(), sFileContent.GetElementCount()));

    FileIn.Close();
  }

#endif

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Delete File / Exists File")
  {
    {
      NS_TEST_BOOL(nsFileSystem::ExistsFile(":output1/FileSystemTest.txt"));
      nsFileSystem::DeleteFile(":output1/FileSystemTest.txt");
      NS_TEST_BOOL(!nsFileSystem::ExistsFile("FileSystemTest.txt"));

      nsFileReader FileIn;
      NS_TEST_BOOL(FileIn.Open("FileSystemTest.txt") == NS_FAILURE);
    }

    // very long path names
    {
      nsStringBuilder sTempFile = ":output1";
      sTempFile.AppendPath(LongPath);
      sTempFile.AppendPath("Temp.tmp");

      nsFileWriter TempFile;
      NS_TEST_BOOL(TempFile.Open(sTempFile) == NS_SUCCESS);
      TempFile.Close();

      NS_TEST_BOOL(nsFileSystem::ExistsFile(sTempFile));
      nsFileSystem::DeleteFile(sTempFile);
      NS_TEST_BOOL(!nsFileSystem::ExistsFile(sTempFile));

      nsFileReader FileIn;
      NS_TEST_BOOL(FileIn.Open("FileSystemTest.txt") == NS_FAILURE);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetFileStats")
  {
    const char* szPath = ":output1/" LongPath "/FileSystemTest.txt";

    // Create file
    {
      nsFileWriter FileOut;
      nsStringBuilder sAbs = sOutputFolder1Resolved;
      sAbs.AppendPath("FileSystemTest.txt");
      NS_TEST_BOOL(FileOut.Open(szPath) == NS_SUCCESS);
      FileOut.WriteBytes("Test", 4).IgnoreResult();
    }

    nsFileStats stat;

    NS_TEST_BOOL(nsFileSystem::GetFileStats(szPath, stat).Succeeded());

    NS_TEST_BOOL(!stat.m_bIsDirectory);
    NS_TEST_STRING(stat.m_sName, "FileSystemTest.txt");
    NS_TEST_INT(stat.m_uiFileSize, 4);

    nsFileSystem::DeleteFile(szPath);
    NS_TEST_BOOL(nsFileSystem::GetFileStats(szPath, stat).Failed());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ResolvePath")
  {
    nsStringBuilder sRel, sAbs;

    NS_TEST_BOOL(nsFileSystem::ResolvePath(":output1/FileSystemTest2.txt", &sAbs, &sRel) == NS_SUCCESS);

    nsStringBuilder sExpectedAbs = sOutputFolder1Resolved;
    sExpectedAbs.AppendPath("FileSystemTest2.txt");

    NS_TEST_STRING(sAbs, sExpectedAbs);
    NS_TEST_STRING(sRel, "FileSystemTest2.txt");

    // create a file in the second dir
    {
      NS_TEST_BOOL(nsFileSystem::AddDataDirectory(sOutputFolder2, "Remove", "output2", nsFileSystem::AllowWrites) == NS_SUCCESS);

      {
        nsFileWriter FileOut;
        NS_TEST_BOOL(FileOut.Open(":output2/FileSystemTest2.txt") == NS_SUCCESS);
      }

      NS_TEST_INT(nsFileSystem::RemoveDataDirectoryGroup("Remove"), 1);
    }

    // find the path to an existing file
    {
      NS_TEST_BOOL(nsFileSystem::ResolvePath("FileSystemTest2.txt", &sAbs, &sRel) == NS_SUCCESS);

      sExpectedAbs = sOutputFolder2Resolved;
      sExpectedAbs.AppendPath("FileSystemTest2.txt");

      NS_TEST_STRING(sAbs, sExpectedAbs);
      NS_TEST_STRING(sRel, "FileSystemTest2.txt");
    }

    // find where we would write the file to (ignoring existing files)
    {
      NS_TEST_BOOL(nsFileSystem::ResolvePath(":output1/FileSystemTest2.txt", &sAbs, &sRel) == NS_SUCCESS);

      sExpectedAbs = sOutputFolder1Resolved;
      sExpectedAbs.AppendPath("FileSystemTest2.txt");

      NS_TEST_STRING(sAbs, sExpectedAbs);
      NS_TEST_STRING(sRel, "FileSystemTest2.txt");
    }

    // find where we would write the file to (ignoring existing files)
    {
      NS_TEST_BOOL(nsFileSystem::ResolvePath(":output1/SubSub/FileSystemTest2.txt", &sAbs, &sRel) == NS_SUCCESS);

      sExpectedAbs = sOutputFolder1Resolved;
      sExpectedAbs.AppendPath("SubSub/FileSystemTest2.txt");

      NS_TEST_STRING(sAbs, sExpectedAbs);
      NS_TEST_STRING(sRel, "SubSub/FileSystemTest2.txt");
    }

    nsFileSystem::DeleteFile(":output1/FileSystemTest2.txt");
    nsFileSystem::DeleteFile(":output2/FileSystemTest2.txt");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FindFolderWithSubPath")
  {
    NS_TEST_BOOL(nsFileSystem::AddDataDirectory(szOutputFolder, "remove", "toplevel", nsFileSystem::AllowWrites) == NS_SUCCESS);
    NS_TEST_BOOL(nsFileSystem::AddDataDirectory(sOutputFolder2, "remove", "output2", nsFileSystem::AllowWrites) == NS_SUCCESS);

    nsStringBuilder StartPath;
    nsStringBuilder SubPath;
    nsStringBuilder result, expected;

    // make sure this exists
    {
      nsFileWriter FileOut;
      NS_TEST_BOOL(FileOut.Open(":output2/FileSystemTest2.txt") == NS_SUCCESS);
    }

    {
      StartPath.Set(sOutputFolder1Resolved, "/SubSub", "/Irrelevant");
      SubPath.Set("DoesNotExist");

      NS_TEST_BOOL(nsFileSystem::FindFolderWithSubPath(result, StartPath, SubPath).Failed());
    }

    {
      StartPath.Set(sOutputFolder1Resolved, "/SubSub", "/Irrelevant");
      SubPath.Set("SubFolder2");
      expected.Set(sOutputFolderResolved, "/IO/");

      NS_TEST_BOOL(nsFileSystem::FindFolderWithSubPath(result, StartPath, SubPath).Succeeded());
      NS_TEST_STRING(result, expected);
    }

    {
      StartPath.Set(sOutputFolder1Resolved, "/SubSub");
      SubPath.Set("IO/SubFolder2");
      expected.Set(sOutputFolderResolved, "/");

      NS_TEST_BOOL(nsFileSystem::FindFolderWithSubPath(result, StartPath, SubPath).Succeeded());
      NS_TEST_STRING(result, expected);
    }

    {
      StartPath.Set(sOutputFolder1Resolved, "/SubSub");
      SubPath.Set("IO/SubFolder2");
      expected.Set(sOutputFolderResolved, "/");

      NS_TEST_BOOL(nsFileSystem::FindFolderWithSubPath(result, StartPath, SubPath).Succeeded());
      NS_TEST_STRING(result, expected);
    }

    {
      StartPath.Set(sOutputFolder1Resolved, "/SubSub", "/Irrelevant");
      SubPath.Set("SubFolder2/FileSystemTest2.txt");
      expected.Set(sOutputFolderResolved, "/IO/");

      NS_TEST_BOOL(nsFileSystem::FindFolderWithSubPath(result, StartPath, SubPath).Succeeded());
      NS_TEST_STRING(result, expected);
    }

    {
      StartPath.Set(":toplevel/IO/SubFolder");
      SubPath.Set("IO/SubFolder2");
      expected.Set(":toplevel/");

      NS_TEST_BOOL(nsFileSystem::FindFolderWithSubPath(result, StartPath, SubPath).Succeeded());
      NS_TEST_STRING(result, expected);
    }

    nsFileSystem::DeleteFile(":output1/FileSystemTest2.txt");
    nsFileSystem::DeleteFile(":output2/FileSystemTest2.txt");

    nsFileSystem::RemoveDataDirectoryGroup("remove");
  }
}

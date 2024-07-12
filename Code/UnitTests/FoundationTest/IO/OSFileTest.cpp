#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/IO/OSFile.h>

NS_CREATE_SIMPLE_TEST(IO, OSFile)
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

  const nsUInt32 uiTextLen = sFileContent.GetElementCount();

  nsStringBuilder sOutputFile = nsTestFramework::GetInstance()->GetAbsOutputPath();
  sOutputFile.MakeCleanPath();
  sOutputFile.AppendPath("IO", "SubFolder");
  sOutputFile.AppendPath("OSFile_TestFile.txt");

  nsStringBuilder sOutputFile2 = nsTestFramework::GetInstance()->GetAbsOutputPath();
  sOutputFile2.MakeCleanPath();
  sOutputFile2.AppendPath("IO", "SubFolder2");
  sOutputFile2.AppendPath("OSFile_TestFileCopy.txt");

  nsStringBuilder sOutputFile3 = nsTestFramework::GetInstance()->GetAbsOutputPath();
  sOutputFile3.MakeCleanPath();
  sOutputFile3.AppendPath("IO", "SubFolder2", "SubSubFolder");
  sOutputFile3.AppendPath("RandomFile.txt");

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Write File")
  {
    nsOSFile f;
    NS_TEST_BOOL(f.Open(sOutputFile.GetData(), nsFileOpenMode::Write) == NS_SUCCESS);
    NS_TEST_BOOL(f.IsOpen());
    NS_TEST_INT(f.GetFilePosition(), 0);
    NS_TEST_INT(f.GetFileSize(), 0);

    for (nsUInt32 i = 0; i < uiTextLen; ++i)
    {
      NS_TEST_BOOL(f.Write(&sFileContent.GetData()[i], 1) == NS_SUCCESS);
      NS_TEST_INT(f.GetFilePosition(), i + 1);
      NS_TEST_INT(f.GetFileSize(), i + 1);
    }

    NS_TEST_INT(f.GetFilePosition(), uiTextLen);
    f.SetFilePosition(5, nsFileSeekMode::FromStart);
    NS_TEST_INT(f.GetFileSize(), uiTextLen);

    NS_TEST_INT(f.GetFilePosition(), 5);
    // f.Close(); // The file should be closed automatically
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Append File")
  {
    nsOSFile f;
    NS_TEST_BOOL(f.Open(sOutputFile.GetData(), nsFileOpenMode::Append) == NS_SUCCESS);
    NS_TEST_BOOL(f.IsOpen());
    NS_TEST_INT(f.GetFilePosition(), uiTextLen);
    NS_TEST_BOOL(f.Write(sFileContent.GetData(), uiTextLen) == NS_SUCCESS);
    NS_TEST_INT(f.GetFilePosition(), uiTextLen * 2);
    f.Close();
    NS_TEST_BOOL(!f.IsOpen());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Read File")
  {
    const nsUInt32 FS_MAX_PATH = 1024;
    char szTemp[FS_MAX_PATH];

    nsOSFile f;
    NS_TEST_BOOL(f.Open(sOutputFile.GetData(), nsFileOpenMode::Read) == NS_SUCCESS);
    NS_TEST_BOOL(f.IsOpen());
    NS_TEST_INT(f.GetFilePosition(), 0);

    NS_TEST_INT(f.Read(szTemp, FS_MAX_PATH), uiTextLen * 2);
    NS_TEST_INT(f.GetFilePosition(), uiTextLen * 2);

    NS_TEST_BOOL(nsMemoryUtils::IsEqual(szTemp, sFileContent.GetData(), uiTextLen));
    NS_TEST_BOOL(nsMemoryUtils::IsEqual(&szTemp[uiTextLen], sFileContent.GetData(), uiTextLen));

    f.Close();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Copy File")
  {
    nsOSFile::CopyFile(sOutputFile.GetData(), sOutputFile2.GetData()).IgnoreResult();

    nsOSFile f;
    NS_TEST_BOOL(f.Open(sOutputFile2.GetData(), nsFileOpenMode::Read) == NS_SUCCESS);

    const nsUInt32 FS_MAX_PATH = 1024;
    char szTemp[FS_MAX_PATH];

    NS_TEST_INT(f.Read(szTemp, FS_MAX_PATH), uiTextLen * 2);

    NS_TEST_BOOL(nsMemoryUtils::IsEqual(szTemp, sFileContent.GetData(), uiTextLen));
    NS_TEST_BOOL(nsMemoryUtils::IsEqual(&szTemp[uiTextLen], sFileContent.GetData(), uiTextLen));

    f.Close();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ReadAll")
  {
    nsOSFile f;
    NS_TEST_BOOL(f.Open(sOutputFile, nsFileOpenMode::Read) == NS_SUCCESS);

    nsDynamicArray<nsUInt8> fileContent;
    const nsUInt64 bytes = f.ReadAll(fileContent);

    NS_TEST_INT(bytes, uiTextLen * 2);

    NS_TEST_BOOL(nsMemoryUtils::IsEqual(fileContent.GetData(), (const nsUInt8*)sFileContent.GetData(), uiTextLen));

    f.Close();
  }

#if NS_ENABLED(NS_SUPPORTS_FILE_STATS)
  NS_TEST_BLOCK(nsTestBlock::Enabled, "File Stats")
  {
    nsFileStats s;

    nsStringBuilder dir = sOutputFile2.GetFileDirectory();

    NS_TEST_BOOL(nsOSFile::GetFileStats(sOutputFile2.GetData(), s) == NS_SUCCESS);
    // printf("%s Name: '%s' (%lli Bytes), Modified Time: %lli\n", s.m_bIsDirectory ? "Directory" : "File", s.m_sFileName.GetData(),
    // s.m_uiFileSize, s.m_LastModificationTime.GetInt64(nsSIUnitOfTime::Microsecond));

    NS_TEST_BOOL(nsOSFile::GetFileStats(dir.GetData(), s) == NS_SUCCESS);
    // printf("%s Name: '%s' (%lli Bytes), Modified Time: %lli\n", s.m_bIsDirectory ? "Directory" : "File", s.m_sFileName.GetData(),
    // s.m_uiFileSize, s.m_LastModificationTime.GetInt64(nsSIUnitOfTime::Microsecond));
  }

#  if (NS_ENABLED(NS_SUPPORTS_CASE_INSENSITIVE_PATHS) && NS_ENABLED(NS_SUPPORTS_UNRESTRICTED_FILE_ACCESS))
  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetFileCasing")
  {
    nsStringBuilder dir = sOutputFile2;
    dir.ToLower();

#    if NS_ENABLED(NS_PLATFORM_WINDOWS)
    // On Windows the drive letter will always be turned upper case by nsOSFile::GetFileCasing()
    // ensure that our input data ('ground truth') also uses an upper case drive letter
    auto driveLetterIterator = sOutputFile2.GetIteratorFront();
    const nsUInt32 uiDriveLetter = nsStringUtils::ToUpperChar(driveLetterIterator.GetCharacter());
    sOutputFile2.ChangeCharacter(driveLetterIterator, uiDriveLetter);
#    endif

    nsStringBuilder sCorrected;
    NS_TEST_BOOL(nsOSFile::GetFileCasing(dir.GetData(), sCorrected) == NS_SUCCESS);

    // On Windows the drive letter will always be made to upper case
    NS_TEST_STRING(sCorrected.GetData(), sOutputFile2.GetData());
  }
#  endif // NS_SUPPORTS_CASE_INSENSITIVE_PATHS && NS_SUPPORTS_UNRESTRICTED_FILE_ACCESS

#endif   // NS_SUPPORTS_FILE_STATS

#if NS_ENABLED(NS_SUPPORTS_FILE_ITERATORS)

  NS_TEST_BLOCK(nsTestBlock::Enabled, "File Iterator")
  {
    // It is not really possible to test this stuff (with a guaranteed result), as long as we do not have
    // a test data folder with deterministic content
    // Therefore I tested it manually, and leave the code in, such that it is at least a 'does it compile and link' test.

    nsStringBuilder sOutputFolder = nsOSFile::GetApplicationDirectory();
    sOutputFolder.AppendPath("*");

    nsStringBuilder sFullPath;

    nsUInt32 uiFolders = 0;
    nsUInt32 uiFiles = 0;

    bool bSkipFolder = true;

    nsFileSystemIterator it;
    for (it.StartSearch(sOutputFolder.GetData(), nsFileSystemIteratorFlags::ReportFilesAndFoldersRecursive); it.IsValid();)
    {
      sFullPath = it.GetCurrentPath();
      sFullPath.AppendPath(it.GetStats().m_sName.GetData());

      it.GetStats();
      it.GetCurrentPath();

      if (it.GetStats().m_bIsDirectory)
      {
        ++uiFolders;
        bSkipFolder = !bSkipFolder;

        if (bSkipFolder)
        {
          it.SkipFolder(); // replaces the 'Next' call
          continue;
        }
      }
      else
      {
        ++uiFiles;
      }

      it.Next();
    }

// The binary folder will only have subdirectories on windows desktop
#  if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)
    NS_TEST_BOOL(uiFolders > 0);
#  endif
    NS_TEST_BOOL(uiFiles > 0);
  }

#endif

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Delete File")
  {
    NS_TEST_BOOL(nsOSFile::DeleteFile(sOutputFile.GetData()) == NS_SUCCESS);
    NS_TEST_BOOL(nsOSFile::DeleteFile(sOutputFile.GetData()) == NS_SUCCESS);          // second time should still 'succeed'

    NS_TEST_BOOL(nsOSFile::DeleteFile(sOutputFile2.GetData()) == NS_SUCCESS);
    NS_TEST_BOOL(nsOSFile::DeleteFile(sOutputFile2.GetData()) == NS_SUCCESS);         // second time should still 'succeed'

    nsOSFile f;
    NS_TEST_BOOL(f.Open(sOutputFile.GetData(), nsFileOpenMode::Read) == NS_FAILURE);  // file should not exist anymore
    NS_TEST_BOOL(f.Open(sOutputFile2.GetData(), nsFileOpenMode::Read) == NS_FAILURE); // file should not exist anymore
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetCurrentWorkingDirectory")
  {
    nsStringBuilder cwd = nsOSFile::GetCurrentWorkingDirectory();

    NS_TEST_BOOL(!cwd.IsEmpty());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "MakePathAbsoluteWithCWD")
  {
    nsStringBuilder cwd = nsOSFile::GetCurrentWorkingDirectory();
    nsStringBuilder path = nsOSFile::MakePathAbsoluteWithCWD("sub/folder");

    NS_TEST_BOOL(path.StartsWith(cwd));
    NS_TEST_BOOL(path.EndsWith("/sub/folder"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExistsFile")
  {
    NS_TEST_BOOL(nsOSFile::ExistsFile(sOutputFile.GetData()) == false);
    NS_TEST_BOOL(nsOSFile::ExistsFile(sOutputFile2.GetData()) == false);

    {
      nsOSFile f;
      NS_TEST_BOOL(f.Open(sOutputFile.GetData(), nsFileOpenMode::Write) == NS_SUCCESS);
    }

    NS_TEST_BOOL(nsOSFile::ExistsFile(sOutputFile.GetData()) == true);
    NS_TEST_BOOL(nsOSFile::ExistsFile(sOutputFile2.GetData()) == false);

    {
      nsOSFile f;
      NS_TEST_BOOL(f.Open(sOutputFile2.GetData(), nsFileOpenMode::Write) == NS_SUCCESS);
    }

    NS_TEST_BOOL(nsOSFile::ExistsFile(sOutputFile.GetData()) == true);
    NS_TEST_BOOL(nsOSFile::ExistsFile(sOutputFile2.GetData()) == true);

    NS_TEST_BOOL(nsOSFile::DeleteFile(sOutputFile.GetData()) == NS_SUCCESS);
    NS_TEST_BOOL(nsOSFile::DeleteFile(sOutputFile2.GetData()) == NS_SUCCESS);

    NS_TEST_BOOL(nsOSFile::ExistsFile(sOutputFile.GetData()) == false);
    NS_TEST_BOOL(nsOSFile::ExistsFile(sOutputFile2.GetData()) == false);

    nsStringBuilder sOutputFolder = nsTestFramework::GetInstance()->GetAbsOutputPath();
    // We should not report folders as files
    NS_TEST_BOOL(nsOSFile::ExistsFile(sOutputFolder) == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExistsDirectory")
  {
    // files are not folders
    NS_TEST_BOOL(nsOSFile::ExistsDirectory(sOutputFile.GetData()) == false);
    NS_TEST_BOOL(nsOSFile::ExistsDirectory(sOutputFile2.GetData()) == false);

    nsStringBuilder sOutputFolder = nsTestFramework::GetInstance()->GetAbsOutputPath();
    NS_TEST_BOOL(nsOSFile::ExistsDirectory(sOutputFolder) == true);

    sOutputFile.AppendPath("IO");
    NS_TEST_BOOL(nsOSFile::ExistsDirectory(sOutputFolder) == true);

    sOutputFile.AppendPath("SubFolder");
    NS_TEST_BOOL(nsOSFile::ExistsDirectory(sOutputFolder) == true);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetApplicationDirectory")
  {
    nsStringView sAppDir = nsOSFile::GetApplicationDirectory();
    NS_TEST_BOOL(!sAppDir.IsEmpty());
  }

#if (NS_ENABLED(NS_SUPPORTS_FILE_ITERATORS) && NS_ENABLED(NS_SUPPORTS_FILE_STATS))

  NS_TEST_BLOCK(nsTestBlock::Enabled, "DeleteFolder")
  {
    {
      nsOSFile f;
      NS_TEST_BOOL(f.Open(sOutputFile3.GetData(), nsFileOpenMode::Write) == NS_SUCCESS);
    }

    nsStringBuilder SubFolder2 = nsTestFramework::GetInstance()->GetAbsOutputPath();
    SubFolder2.MakeCleanPath();
    SubFolder2.AppendPath("IO", "SubFolder2");

    NS_TEST_BOOL(nsOSFile::DeleteFolder(SubFolder2).Succeeded());
    NS_TEST_BOOL(!nsOSFile::ExistsDirectory(SubFolder2));
  }

#endif
}

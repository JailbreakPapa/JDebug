#include <ToolsFoundationTest/ToolsFoundationTestPCH.h>

#if NS_ENABLED(NS_SUPPORTS_DIRECTORY_WATCHER) && NS_ENABLED(NS_SUPPORTS_FILE_ITERATORS)

#  include <Foundation/Application/Config/FileSystemConfig.h>
#  include <Foundation/IO/FileSystem/DataDirTypeFolder.h>
#  include <Foundation/IO/FileSystem/FileReader.h>
#  include <Foundation/IO/FileSystem/FileSystem.h>
#  include <Foundation/IO/FileSystem/FileWriter.h>
#  include <Foundation/Threading/ThreadUtils.h>
#  include <ToolsFoundation/FileSystem/FileSystemModel.h>


NS_CREATE_SIMPLE_TEST_GROUP(FileSystem);

namespace
{
  nsResult nstCreateFile(nsStringView sPath)
  {
    nsFileWriter FileOut;
    NS_SUCCEED_OR_RETURN(FileOut.Open(sPath));
    NS_SUCCEED_OR_RETURN(FileOut.WriteString("Test"));
    FileOut.Close();
    return NS_SUCCESS;
  }
} // namespace

NS_CREATE_SIMPLE_TEST(FileSystem, DataDirPath)
{
  const nsStringView sFilePathView = "C:/Code/nsEngine/Data/Samples/Testing Chambers/Objects/Barrel.nsPrefab"_nssv;
  const nsStringView sDataDirView = "C:/Code/nsEngine/Data/Samples/Testing Chambers"_nssv;

  auto CheckIsValid = [&](const nsDataDirPath& path)
  {
    NS_TEST_BOOL(path.IsValid());
    nsStringView sAbs = path.GetAbsolutePath();
    NS_TEST_STRING(sAbs, sFilePathView);
    nsStringView sDD = path.GetDataDir();
    NS_TEST_STRING(sDD, sDataDirView);
    nsStringView sPR = path.GetDataDirParentRelativePath();
    NS_TEST_STRING(sPR, "Testing Chambers/Objects/Barrel.nsPrefab");
    nsStringView sR = path.GetDataDirRelativePath();
    NS_TEST_STRING(sR, "Objects/Barrel.nsPrefab");
  };

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Windows Path copy ctor")
  {
    nsHybridArray<nsString, 2> rootFolders;
    rootFolders.PushBack("C:/SomeOtherFolder/Folder");
    rootFolders.PushBack(sDataDirView);

    nsDataDirPath path(sFilePathView, rootFolders);
    CheckIsValid(path);
    nsUInt32 uiIndex = path.GetDataDirIndex();
    NS_TEST_INT(uiIndex, 1);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Linux Path move ctor")
  {
    nsString sFilePathView = "/Code/nsEngine/Data/Samples/Testing Chambers/Objects/Barrel.nsPrefab"_nssv;
    nsString sFilePath = sFilePathView;
    auto sDataDir = "/Code/nsEngine/Data/Samples/Testing Chambers"_nssv;
    nsHybridArray<nsString, 2> rootFolders;
    rootFolders.PushBack(sDataDir);
    rootFolders.PushBack("/SomeOtherFolder/Folder");

    const char* szRawStringPtr = sFilePath.GetData();
    nsDataDirPath path(std::move(sFilePath), rootFolders);
    NS_TEST_BOOL(path.IsValid());
    nsStringView sAbs = path.GetAbsolutePath();
    NS_TEST_STRING(sAbs, sFilePathView);
    NS_TEST_BOOL(szRawStringPtr == sAbs.GetStartPointer());
    nsStringView sDD = path.GetDataDir();
    NS_TEST_STRING(sDD, sDataDir);
    nsStringView sPR = path.GetDataDirParentRelativePath();
    NS_TEST_STRING(sPR, "Testing Chambers/Objects/Barrel.nsPrefab");
    nsStringView sR = path.GetDataDirRelativePath();
    NS_TEST_STRING(sR, "Objects/Barrel.nsPrefab");
    nsUInt32 uiIndex = path.GetDataDirIndex();
    NS_TEST_INT(uiIndex, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Path to DataDir Itself")
  {
    nsString sDataDirView = (const char*)u8"/Code/nsEngine/Data/Sämples/Testing Chämbers";
    nsHybridArray<nsString, 2> rootFolders;
    rootFolders.PushBack(sDataDirView);

    nsDataDirPath path(sDataDirView.GetView(), rootFolders);
    NS_TEST_BOOL(path.IsValid());
    nsStringView sAbs = path.GetAbsolutePath();
    NS_TEST_STRING(sAbs, sDataDirView);
    nsStringView sDD = path.GetDataDir();
    NS_TEST_STRING(sDD, sDataDirView);
    nsStringView sPR = path.GetDataDirParentRelativePath();
    NS_TEST_STRING(sPR, (const char*)u8"Testing Chämbers");
    nsStringView sR = path.GetDataDirRelativePath();
    NS_TEST_STRING(sR, "");
    nsUInt32 uiIndex = path.GetDataDirIndex();
    NS_TEST_INT(uiIndex, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Move")
  {
    nsHybridArray<nsString, 2> rootFolders;
    rootFolders.PushBack(sDataDirView);

    nsString sFilePath = sFilePathView;
    const char* szRawStringPtr = sFilePath.GetData();
    nsDataDirPath path(std::move(sFilePath), rootFolders);
    CheckIsValid(path);

    nsStringView sAbs = path.GetAbsolutePath();
    NS_TEST_BOOL(szRawStringPtr == sAbs.GetStartPointer());

    nsDataDirPath path2 = std::move(path);
    nsStringView sAbs2 = path2.GetAbsolutePath();
    NS_TEST_BOOL(szRawStringPtr == sAbs2.GetStartPointer());

    nsDataDirPath path3(std::move(path2));
    nsStringView sAbs3 = path3.GetAbsolutePath();
    NS_TEST_BOOL(szRawStringPtr == sAbs3.GetStartPointer());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Rebuild")
  {
    nsHybridArray<nsString, 2> rootFolders;
    rootFolders.PushBack(sDataDirView);

    nsDataDirPath path(sFilePathView, rootFolders);
    CheckIsValid(path);
    NS_TEST_INT(path.GetDataDirIndex(), 0);

    nsHybridArray<nsString, 2> newRootFolders;
    newRootFolders.PushBack(sDataDirView);
    newRootFolders.PushBack("C:/Some/Other/DataDir");

    path.UpdateDataDirInfos(newRootFolders);
    CheckIsValid(path);
    NS_TEST_INT(path.GetDataDirIndex(), 0);

    newRootFolders.InsertAt(0, "C:/Some/Other/DataDir2");
    path.UpdateDataDirInfos(newRootFolders);
    CheckIsValid(path);
    NS_TEST_INT(path.GetDataDirIndex(), 1);

    newRootFolders.RemoveAtAndCopy(0);
    path.UpdateDataDirInfos(newRootFolders);
    CheckIsValid(path);
    NS_TEST_INT(path.GetDataDirIndex(), 0);

    newRootFolders.RemoveAtAndCopy(0);
    path.UpdateDataDirInfos(newRootFolders);
    NS_TEST_BOOL(!path.IsValid());
    nsStringView sAbs = path.GetAbsolutePath();
    NS_TEST_STRING(sAbs, sFilePathView);
  }
}

NS_CREATE_SIMPLE_TEST(FileSystem, FileSystemModel)
{
  constexpr nsUInt32 WAIT_LOOPS = 1000;

  nsStringBuilder sOutputFolder = nsTestFramework::GetInstance()->GetAbsOutputPath();
  sOutputFolder.AppendPath("Model");
  sOutputFolder.MakeCleanPath();

  nsStringBuilder sOutputFolderResolved;
  nsFileSystem::ResolveSpecialDirectory(sOutputFolder, sOutputFolderResolved).IgnoreResult();

  nsHybridArray<nsString, 1> rootFolders;

  nsApplicationFileSystemConfig fsConfig;
  nsApplicationFileSystemConfig::DataDirConfig& dataDir = fsConfig.m_DataDirs.ExpandAndGetRef();
  dataDir.m_bWritable = true;
  dataDir.m_sDataDirSpecialPath = sOutputFolder;
  dataDir.m_sRootName = "output";
  rootFolders.PushBack(sOutputFolder);

  // Files
  nsHybridArray<nsFileChangedEvent, 2> fileEvents;
  nsHybridArray<nsTime, 2> fileEventTimestamps;
  nsMutex fileEventLock;
  auto fileEvent = [&](const nsFileChangedEvent& e)
  {
    NS_LOCK(fileEventLock);
    fileEvents.PushBack(e);
    fileEventTimestamps.PushBack(nsTime::Now());

    nsFileStatus stat;
    switch (e.m_Type)
    {
      case nsFileChangedEvent::Type::FileRemoved:
        NS_TEST_BOOL(nsFileSystemModel::GetSingleton()->FindFile(e.m_Path, stat).Failed());
        break;
      case nsFileChangedEvent::Type::FileAdded:
      case nsFileChangedEvent::Type::FileChanged:
      case nsFileChangedEvent::Type::DocumentLinked:
        NS_TEST_BOOL(nsFileSystemModel::GetSingleton()->FindFile(e.m_Path, stat).Succeeded());
        break;

      case nsFileChangedEvent::Type::ModelReset:
      default:
        break;
    }
  };
  nsEventSubscriptionID fileId = nsFileSystemModel::GetSingleton()->m_FileChangedEvents.AddEventHandler(fileEvent);

  // Folders
  nsHybridArray<nsFolderChangedEvent, 2> folderEvents;
  nsHybridArray<nsTime, 2> folderEventTimestamps;
  nsMutex folderEventLock;
  auto folderEvent = [&](const nsFolderChangedEvent& e)
  {
    NS_LOCK(folderEventLock);
    folderEvents.PushBack(e);
    folderEventTimestamps.PushBack(nsTime::Now());

    switch (e.m_Type)
    {
      case nsFolderChangedEvent::Type::FolderAdded:
        NS_TEST_BOOL(nsFileSystemModel::GetSingleton()->GetFolders()->Contains(e.m_Path));
        break;
      case nsFolderChangedEvent::Type::FolderRemoved:
        NS_TEST_BOOL(!nsFileSystemModel::GetSingleton()->GetFolders()->Contains(e.m_Path));
        break;
      case nsFolderChangedEvent::Type::ModelReset:
      default:
        break;
    }
  };
  nsEventSubscriptionID folderId = nsFileSystemModel::GetSingleton()->m_FolderChangedEvents.AddEventHandler(folderEvent);

  // Helper functions
  auto CompareFiles = [&](nsArrayPtr<nsFileChangedEvent> expected)
  {
    NS_LOCK(fileEventLock);
    if (NS_TEST_INT(expected.GetCount(), fileEvents.GetCount()))
    {
      for (size_t i = 0; i < expected.GetCount(); i++)
      {
        NS_TEST_INT((int)expected[i].m_Type, (int)fileEvents[i].m_Type);
        NS_TEST_STRING(expected[i].m_Path, fileEvents[i].m_Path);
        NS_TEST_BOOL(expected[i].m_Status.m_DocumentID == fileEvents[i].m_Status.m_DocumentID);
        // Ignore stats besudes GUID.
      }
    }
  };

  auto ClearFiles = [&]()
  {
    NS_LOCK(fileEventLock);
    fileEvents.Clear();
    fileEventTimestamps.Clear();
  };

  auto CompareFolders = [&](nsArrayPtr<nsFolderChangedEvent> expected)
  {
    NS_LOCK(folderEventLock);
    if (NS_TEST_INT(expected.GetCount(), folderEvents.GetCount()))
    {
      for (size_t i = 0; i < expected.GetCount(); i++)
      {
        NS_TEST_INT((int)expected[i].m_Type, (int)folderEvents[i].m_Type);
        NS_TEST_STRING(expected[i].m_Path, folderEvents[i].m_Path);
        // Ignore stats
      }
    }
  };

  auto ClearFolders = [&]()
  {
    NS_LOCK(folderEventLock);
    folderEvents.Clear();
    folderEventTimestamps.Clear();
  };

  auto MakePath = [&](nsStringView sPath)
  {
    return nsDataDirPath(sPath, rootFolders);
  };


  NS_TEST_BLOCK(nsTestBlock::Enabled, "Startup")
  {
    nsFileSystem::RegisterDataDirectoryFactory(nsDataDirectory::FolderType::Factory);

    NS_TEST_RESULT(nsOSFile::DeleteFolder(sOutputFolderResolved));
    NS_TEST_RESULT(nsFileSystem::CreateDirectoryStructure(sOutputFolderResolved));

    // for absolute paths
    NS_TEST_BOOL(nsFileSystem::AddDataDirectory("", "", ":", nsFileSystem::AllowWrites) == NS_SUCCESS);
    NS_TEST_BOOL(nsFileSystem::AddDataDirectory(sOutputFolder, "Clear", "output", nsFileSystem::AllowWrites) == NS_SUCCESS);

    nsFileSystemModel::GetSingleton()->Initialize(fsConfig, {}, {});

    nsFileChangedEvent expected[] = {nsFileChangedEvent({}, {}, nsFileChangedEvent::Type::ModelReset)};
    CompareFiles(nsMakeArrayPtr(expected));
    ClearFiles();

    nsFolderChangedEvent expected2[] = {nsFolderChangedEvent({}, nsFolderChangedEvent::Type::ModelReset)};
    CompareFolders(nsMakeArrayPtr(expected2));
    ClearFolders();

    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFiles()->GetCount(), 0);
    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFolders()->GetCount(), 1);

    auto it = nsFileSystemModel::GetSingleton()->GetFolders()->GetIterator();
    NS_TEST_STRING(it.Key(), sOutputFolder);
    NS_TEST_BOOL(it.Value() == nsFileStatus::Status::Valid);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Add file")
  {
    nsStringBuilder sFilePath(sOutputFolder);
    sFilePath.AppendPath("rootFile.txt");

    NS_TEST_RESULT(nstCreateFile(sFilePath));

    for (nsUInt32 i = 0; i < WAIT_LOOPS; i++)
    {
      nsFileSystemModel::GetSingleton()->MainThreadTick();
      nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));

      NS_LOCK(fileEventLock);
      if (fileEvents.GetCount() > 0)
        break;
    }

    nsFileChangedEvent expected[] = {nsFileChangedEvent(MakePath(sFilePath), {}, nsFileChangedEvent::Type::FileAdded)};
    CompareFiles(nsMakeArrayPtr(expected));
    ClearFiles();
    CompareFolders({});

    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFiles()->GetCount(), 1);
    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFolders()->GetCount(), 1);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "modify file")
  {
    nsStringBuilder sFilePath(sOutputFolder);
    sFilePath.AppendPath("rootFile.txt");

    {
#  if NS_ENABLED(NS_PLATFORM_LINUX)
      // EXT3 filesystem only support second resolution so we won't detect the modification if it is done within the same second.
      nsThreadUtils::Sleep(nsTime::MakeFromSeconds(1.0));
#  endif
      nsFileWriter FileOut;
      NS_TEST_RESULT(FileOut.Open(sFilePath));
      NS_TEST_RESULT(FileOut.WriteString("Test2"));
      NS_TEST_RESULT(FileOut.Flush());
      FileOut.Close();
    }

    for (nsUInt32 i = 0; i < WAIT_LOOPS; i++)
    {
      nsFileSystemModel::GetSingleton()->MainThreadTick();
      nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));

      NS_LOCK(fileEventLock);
      if (fileEvents.GetCount() > 0)
        break;
    }

    nsFileChangedEvent expected[] = {nsFileChangedEvent(MakePath(sFilePath), {}, nsFileChangedEvent::Type::FileChanged)};
    CompareFiles(nsMakeArrayPtr(expected));
    ClearFiles();
    CompareFolders({});

    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFiles()->GetCount(), 1);
    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFolders()->GetCount(), 1);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "rename file")
  {
    nsStringBuilder sFilePathOld(sOutputFolder);
    sFilePathOld.AppendPath("rootFile.txt");

    nsStringBuilder sFilePathNew(sOutputFolder);
    sFilePathNew.AppendPath("rootFile2.txt");

    NS_TEST_RESULT(nsOSFile::MoveFileOrDirectory(sFilePathOld, sFilePathNew));

    for (nsUInt32 i = 0; i < WAIT_LOOPS; i++)
    {
      nsFileSystemModel::GetSingleton()->MainThreadTick();
      nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));

      NS_LOCK(fileEventLock);
      if (fileEvents.GetCount() == 2)
        break;
    }

    nsFileChangedEvent expected[] = {
      nsFileChangedEvent(MakePath(sFilePathNew), {}, nsFileChangedEvent::Type::FileAdded),
      nsFileChangedEvent(MakePath(sFilePathOld), {}, nsFileChangedEvent::Type::FileRemoved)};
    CompareFiles(nsMakeArrayPtr(expected));
    ClearFiles();
    CompareFolders({});

    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFiles()->GetCount(), 1);
    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFolders()->GetCount(), 1);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Add folder")
  {
    nsStringBuilder sFolderPath(sOutputFolder);
    sFolderPath.AppendPath("Folder1");

    NS_TEST_RESULT(nsFileSystem::CreateDirectoryStructure(sFolderPath));

    for (nsUInt32 i = 0; i < WAIT_LOOPS; i++)
    {
      nsFileSystemModel::GetSingleton()->MainThreadTick();
      nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));

      NS_LOCK(folderEventLock);
      if (folderEvents.GetCount() > 0)
        break;
    }

    nsFolderChangedEvent expected[] = {nsFolderChangedEvent(MakePath(sFolderPath), nsFolderChangedEvent::Type::FolderAdded)};
    CompareFolders(nsMakeArrayPtr(expected));
    ClearFolders();
    CompareFiles({});

    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFiles()->GetCount(), 1);
    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFolders()->GetCount(), 2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "move file")
  {
    nsStringBuilder sFilePathOld(sOutputFolder);
    sFilePathOld.AppendPath("rootFile2.txt");

    nsStringBuilder sFilePathNew(sOutputFolder);
    sFilePathNew.AppendPath("Folder1", "rootFile2.txt");

    NS_TEST_RESULT(nsOSFile::MoveFileOrDirectory(sFilePathOld, sFilePathNew));

    for (nsUInt32 i = 0; i < WAIT_LOOPS; i++)
    {
      nsFileSystemModel::GetSingleton()->MainThreadTick();
      nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));

      NS_LOCK(fileEventLock);
      if (fileEvents.GetCount() == 2)
        break;
    }

    nsFileChangedEvent expected[] = {
      nsFileChangedEvent(MakePath(sFilePathNew), {}, nsFileChangedEvent::Type::FileAdded),
      nsFileChangedEvent(MakePath(sFilePathOld), {}, nsFileChangedEvent::Type::FileRemoved)};
    CompareFiles(nsMakeArrayPtr(expected));
    ClearFiles();
    CompareFolders({});

    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFiles()->GetCount(), 1);
    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFolders()->GetCount(), 2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "move folder")
  {
    nsStringBuilder sFolderPathOld(sOutputFolder);
    sFolderPathOld.AppendPath("Folder1");

    nsStringBuilder sFilePathOld(sOutputFolder);
    sFilePathOld.AppendPath("Folder1", "rootFile2.txt");

    nsStringBuilder sFolderPathNew(sOutputFolder);
    sFolderPathNew.AppendPath("Folder12");

    nsStringBuilder sFilePathNew(sOutputFolder);
    sFilePathNew.AppendPath("Folder12", "rootFile2.txt");

    NS_TEST_RESULT(nsOSFile::MoveFileOrDirectory(sFolderPathOld, sFolderPathNew));

    for (nsUInt32 i = 0; i < WAIT_LOOPS; i++)
    {
      nsFileSystemModel::GetSingleton()->MainThreadTick();
      nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));

      NS_LOCK(fileEventLock);
      NS_LOCK(folderEventLock);
      if (fileEvents.GetCount() == 2 && folderEvents.GetCount() == 2)
        break;
    }

    {
      nsFolderChangedEvent expected[] = {
        nsFolderChangedEvent(MakePath(sFolderPathNew), nsFolderChangedEvent::Type::FolderAdded),
        nsFolderChangedEvent(MakePath(sFolderPathOld), nsFolderChangedEvent::Type::FolderRemoved)};
      CompareFolders(nsMakeArrayPtr(expected));
    }

    {
      nsFileChangedEvent expected[] = {
        nsFileChangedEvent(MakePath(sFilePathNew), {}, nsFileChangedEvent::Type::FileAdded),
        nsFileChangedEvent(MakePath(sFilePathOld), {}, nsFileChangedEvent::Type::FileRemoved)};
      CompareFiles(nsMakeArrayPtr(expected));
    }
    {
      NS_LOCK(fileEventLock);
      NS_LOCK(folderEventLock);
      // Check folder added before file
      NS_TEST_BOOL(fileEventTimestamps[0] > folderEventTimestamps[0]);
      // Check file removed before folder
      NS_TEST_BOOL(fileEventTimestamps[1] < folderEventTimestamps[1]);
    }

    ClearFolders();
    ClearFiles();

    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFiles()->GetCount(), 1);
    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFolders()->GetCount(), 2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "HashFile")
  {
    nsStringBuilder sFilePathNew(sOutputFolder);
    sFilePathNew.AppendPath("Folder12", "rootFile2.txt");

    nsFileStatus status;
    NS_TEST_RESULT(nsFileSystemModel::GetSingleton()->HashFile(sFilePathNew, status));
    NS_TEST_INT((nsInt64)status.m_uiHash, (nsInt64)10983861097202158394u);
  }

  nsFileSystemModel::FilesMap referencedFiles;
  nsFileSystemModel::FoldersMap referencedFolders;

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Shutdown")
  {
    nsFileSystemModel::GetSingleton()->Deinitialize(&referencedFiles, &referencedFolders);
    NS_TEST_INT(referencedFiles.GetCount(), 1);
    NS_TEST_INT(referencedFolders.GetCount(), 2);

    nsFileChangedEvent expected[] = {nsFileChangedEvent({}, {}, nsFileChangedEvent::Type::ModelReset)};
    CompareFiles(nsMakeArrayPtr(expected));
    ClearFiles();

    nsFolderChangedEvent expected2[] = {nsFolderChangedEvent({}, nsFolderChangedEvent::Type::ModelReset)};
    CompareFolders(nsMakeArrayPtr(expected2));
    ClearFolders();
  }

  nsStringBuilder sOutputFolder2 = sOutputFolderResolved;
  sOutputFolder2.ChangeFileNameAndExtension("Model2");

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Startup Restore Model")
  {
    {
      // Add another data directory. This is now at the index of the old one, requiring the indices to be updated inside nsFileSystemModel::Initialize.
      NS_TEST_RESULT(nsOSFile::DeleteFolder(sOutputFolder2));
      NS_TEST_RESULT(nsFileSystem::CreateDirectoryStructure(sOutputFolder2));
      nsApplicationFileSystemConfig::DataDirConfig dataDir;
      dataDir.m_bWritable = true;
      dataDir.m_sDataDirSpecialPath = sOutputFolder2;
      dataDir.m_sRootName = "output2";

      rootFolders.InsertAt(0, sOutputFolder);
      fsConfig.m_DataDirs.InsertAt(0, dataDir);
    }

    nsFileSystemModel::GetSingleton()->Initialize(fsConfig, std::move(referencedFiles), std::move(referencedFolders));

    nsFileChangedEvent expected[] = {nsFileChangedEvent({}, {}, nsFileChangedEvent::Type::ModelReset)};
    CompareFiles(nsMakeArrayPtr(expected));
    ClearFiles();

    nsFolderChangedEvent expected2[] = {nsFolderChangedEvent({}, nsFolderChangedEvent::Type::ModelReset)};
    CompareFolders(nsMakeArrayPtr(expected2));
    ClearFolders();

    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFiles()->GetCount(), 1);
    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFolders()->GetCount(), 3);

    // Check that files have be remapped.
    for (auto it : *nsFileSystemModel::GetSingleton()->GetFiles())
    {
      NS_TEST_INT(it.Key().GetDataDirIndex(), 1);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetFiles")
  {
    nsStringBuilder sFilePathNew(sOutputFolder);
    sFilePathNew.AppendPath("Folder12", "rootFile2.txt");

    nsFileSystemModel::LockedFiles files = nsFileSystemModel::GetSingleton()->GetFiles();
    NS_TEST_INT(files->GetCount(), 1);
    auto it = files->GetIterator();
    NS_TEST_STRING(it.Key(), sFilePathNew);
    NS_TEST_BOOL(it.Value().m_LastModified.IsValid());
    NS_TEST_INT((nsInt64)it.Value().m_uiHash, (nsInt64)10983861097202158394u);
    NS_TEST_BOOL(it.Value().m_Status == nsFileStatus::Status::Valid);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetFolders")
  {
    nsStringBuilder sFolder(sOutputFolder);
    sFolder.AppendPath("Folder12");

    nsFileSystemModel::LockedFolders folders = nsFileSystemModel::GetSingleton()->GetFolders();
    NS_TEST_INT(folders->GetCount(), 3);
    auto it = folders->GetIterator();

    // nsMap is sorted so the order is fixed.
    NS_TEST_STRING(it.Key(), sOutputFolder);
    NS_TEST_BOOL(it.Value() == nsFileStatus::Status::Valid);

    it.Next();
    NS_TEST_STRING(it.Key(), sFolder);
    NS_TEST_BOOL(it.Value() == nsFileStatus::Status::Valid);

    it.Next();
    NS_TEST_STRING(it.Key(), sOutputFolder2);
    NS_TEST_BOOL(it.Value() == nsFileStatus::Status::Valid);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CheckFileSystem")
  {
    nsStringBuilder sFolderPath(sOutputFolder);
    sFolderPath.AppendPath("Folder12");

    nsStringBuilder sFilePath(sOutputFolder);
    sFilePath.AppendPath("Folder12", "rootFile2.txt");

    nsFileSystemModel::GetSingleton()->CheckFileSystem();

    // #TODO_ASSET This FileChanged should be removed once the model is fixed to no longer require firing this after restoring the model from cache. See comment in nsFileSystemModel::HandleSingleFile.
    nsFileChangedEvent expected[] = {
      nsFileChangedEvent(MakePath(sFilePath), {}, nsFileChangedEvent::Type::FileChanged),
      nsFileChangedEvent({}, {}, nsFileChangedEvent::Type::ModelReset)};
    CompareFiles(nsMakeArrayPtr(expected));
    ClearFiles();

    nsFolderChangedEvent expected2[] = {nsFolderChangedEvent({}, nsFolderChangedEvent::Type::ModelReset)};
    CompareFolders(nsMakeArrayPtr(expected2));
    ClearFolders();

    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFiles()->GetCount(), 1);
    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFolders()->GetCount(), 3);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "NotifyOfChange - File")
  {
    nsStringBuilder sFilePath(sOutputFolder);
    sFilePath.AppendPath("rootFile.txt");
    {
      NS_TEST_RESULT(nstCreateFile(sFilePath));
      nsFileSystemModel::GetSingleton()->NotifyOfChange(sFilePath);

      nsFileChangedEvent expected[] = {nsFileChangedEvent(MakePath(sFilePath), {}, nsFileChangedEvent::Type::FileAdded)};
      CompareFiles(nsMakeArrayPtr(expected));
      ClearFiles();
      CompareFolders({});
      NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFiles()->GetCount(), 2);
      NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFolders()->GetCount(), 3);
    }

    {
      NS_TEST_RESULT(nsOSFile::DeleteFile(sFilePath));
      nsFileSystemModel::GetSingleton()->NotifyOfChange(sFilePath);

      nsFileChangedEvent expected[] = {nsFileChangedEvent(MakePath(sFilePath), {}, nsFileChangedEvent::Type::FileRemoved)};
      CompareFiles(nsMakeArrayPtr(expected));
      ClearFiles();
      CompareFolders({});
      NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFiles()->GetCount(), 1);
      NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFolders()->GetCount(), 3);
    }

    for (size_t i = 0; i < 15; i++)
    {
      nsFileSystemModel::GetSingleton()->MainThreadTick();
      nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));
    }
    CompareFiles({});
    CompareFolders({});
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "NotifyOfChange - Folder")
  {
    nsStringBuilder sFolderPath(sOutputFolder);
    sFolderPath.AppendPath("AnotherFolder");
    {
      NS_TEST_RESULT(nsFileSystem::CreateDirectoryStructure(sFolderPath));
      nsFileSystemModel::GetSingleton()->NotifyOfChange(sFolderPath);

      CompareFiles({});
      ClearFiles();
      nsFolderChangedEvent expected[] = {nsFolderChangedEvent(MakePath(sFolderPath), nsFolderChangedEvent::Type::FolderAdded)};
      CompareFolders(nsMakeArrayPtr(expected));
      ClearFolders();
      NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFiles()->GetCount(), 1);
      NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFolders()->GetCount(), 4);
    }

    {
      NS_TEST_RESULT(nsOSFile::DeleteFolder(sFolderPath));
      nsFileSystemModel::GetSingleton()->NotifyOfChange(sFolderPath);

      CompareFiles({});
      ClearFiles();
      nsFolderChangedEvent expected[] = {nsFolderChangedEvent(MakePath(sFolderPath), nsFolderChangedEvent::Type::FolderRemoved)};
      CompareFolders(nsMakeArrayPtr(expected));
      ClearFolders();
      NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFiles()->GetCount(), 1);
      NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFolders()->GetCount(), 3);
    }
    for (size_t i = 0; i < 15; i++)
    {
      nsFileSystemModel::GetSingleton()->MainThreadTick();
      nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));
    }
    CompareFiles({});
    CompareFolders({});
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CheckFolder - File")
  {
    nsStringBuilder sFilePath(sOutputFolder);
    sFilePath.AppendPath("Folder12", "subFile.txt");
    {
      NS_TEST_RESULT(nstCreateFile(sFilePath));
      nsFileSystemModel::GetSingleton()->CheckFolder(sOutputFolder);

      nsFileChangedEvent expected[] = {nsFileChangedEvent(MakePath(sFilePath), {}, nsFileChangedEvent::Type::FileAdded)};
      CompareFiles(nsMakeArrayPtr(expected));
      ClearFiles();
      CompareFolders({});
      NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFiles()->GetCount(), 2);
      NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFolders()->GetCount(), 3);
    }

    {
      NS_TEST_RESULT(nsOSFile::DeleteFile(sFilePath));
      nsFileSystemModel::GetSingleton()->CheckFolder(sOutputFolder);

      nsFileChangedEvent expected[] = {nsFileChangedEvent(MakePath(sFilePath), {}, nsFileChangedEvent::Type::FileRemoved)};
      CompareFiles(nsMakeArrayPtr(expected));
      ClearFiles();
      CompareFolders({});
      NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFiles()->GetCount(), 1);
      NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFolders()->GetCount(), 3);
    }
    for (size_t i = 0; i < 15; i++)
    {
      nsFileSystemModel::GetSingleton()->MainThreadTick();
      nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));
    }
    CompareFiles({});
    CompareFolders({});
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CheckFolder - Folder")
  {
    nsStringBuilder sFolderPath(sOutputFolder);
    sFolderPath.AppendPath("YetAnotherFolder");
    nsStringBuilder sFolderSubPath(sOutputFolder);
    sFolderSubPath.AppendPath("YetAnotherFolder", "SubFolder");
    {
      NS_TEST_RESULT(nsFileSystem::CreateDirectoryStructure(sFolderSubPath));
      nsFileSystemModel::GetSingleton()->CheckFolder(sOutputFolder);

      CompareFiles({});
      ClearFiles();
      nsFolderChangedEvent expected[] = {
        nsFolderChangedEvent(MakePath(sFolderPath), nsFolderChangedEvent::Type::FolderAdded),
        nsFolderChangedEvent(MakePath(sFolderSubPath), nsFolderChangedEvent::Type::FolderAdded)};
      CompareFolders(nsMakeArrayPtr(expected));
      ClearFolders();
      NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFiles()->GetCount(), 1);
      NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFolders()->GetCount(), 5);
    }

    {
      NS_TEST_RESULT(nsOSFile::DeleteFolder(sFolderPath));
      nsFileSystemModel::GetSingleton()->CheckFolder(sOutputFolder);

      CompareFiles({});
      ClearFiles();
      nsFolderChangedEvent expected[] = {
        nsFolderChangedEvent(MakePath(sFolderSubPath), nsFolderChangedEvent::Type::FolderRemoved),
        nsFolderChangedEvent(MakePath(sFolderPath), nsFolderChangedEvent::Type::FolderRemoved)};
      CompareFolders(nsMakeArrayPtr(expected));
      ClearFolders();
      NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFiles()->GetCount(), 1);
      NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFolders()->GetCount(), 3);
    }
    for (size_t i = 0; i < 15; i++)
    {
      nsFileSystemModel::GetSingleton()->MainThreadTick();
      nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));
    }
    CompareFiles({});
    CompareFolders({});
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ReadDocument")
  {
    nsStringBuilder sFilePathNew(sOutputFolder);
    sFilePathNew.AppendPath("Folder12", "rootFile2.txt");

    nsUuid docGuid = nsUuid::MakeUuid();
    auto callback = [&](const nsFileStatus& status, nsStreamReader& ref_reader)
    {
      NS_TEST_INT((nsInt64)status.m_uiHash, (nsInt64)10983861097202158394u);
      nsFileSystemModel::GetSingleton()->LinkDocument(sFilePathNew, docGuid).IgnoreResult();
    };

    NS_TEST_RESULT(nsFileSystemModel::GetSingleton()->ReadDocument(sFilePathNew, callback));

    nsFileStatus stat;
    stat.m_DocumentID = docGuid;
    nsFileChangedEvent expected[] = {nsFileChangedEvent(MakePath(sFilePathNew), stat, nsFileChangedEvent::Type::DocumentLinked)};
    CompareFiles(nsMakeArrayPtr(expected));
    ClearFiles();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "LinkDocument")
  {
    nsStringBuilder sFilePathNew(sOutputFolder);
    sFilePathNew.AppendPath("Folder12", "rootFile2.txt");

    nsUuid guid = nsUuid::MakeUuid();
    nsUuid guid2 = nsUuid::MakeUuid();
    {
      NS_TEST_RESULT(nsFileSystemModel::GetSingleton()->LinkDocument(sFilePathNew, guid));
      NS_TEST_RESULT(nsFileSystemModel::GetSingleton()->LinkDocument(sFilePathNew, guid));
      NS_TEST_RESULT(nsFileSystemModel::GetSingleton()->LinkDocument(sFilePathNew, guid2));

      nsFileStatus stat;
      stat.m_DocumentID = guid;
      nsFileStatus stat2;
      stat2.m_DocumentID = guid2;

      nsFileChangedEvent expected[] = {
        nsFileChangedEvent(MakePath(sFilePathNew), stat, nsFileChangedEvent::Type::DocumentLinked),
        nsFileChangedEvent(MakePath(sFilePathNew), stat, nsFileChangedEvent::Type::DocumentUnlinked),
        nsFileChangedEvent(MakePath(sFilePathNew), stat2, nsFileChangedEvent::Type::DocumentLinked)};
      CompareFiles(nsMakeArrayPtr(expected));
      ClearFiles();
    }
    {
      NS_TEST_RESULT(nsFileSystemModel::GetSingleton()->UnlinkDocument(sFilePathNew));
      NS_TEST_RESULT(nsFileSystemModel::GetSingleton()->UnlinkDocument(sFilePathNew));

      nsFileStatus stat2;
      stat2.m_DocumentID = guid2;

      nsFileChangedEvent expected[] = {nsFileChangedEvent(MakePath(sFilePathNew), stat2, nsFileChangedEvent::Type::DocumentUnlinked)};
      CompareFiles(nsMakeArrayPtr(expected));
      ClearFiles();
    }
    for (size_t i = 0; i < 15; i++)
    {
      nsFileSystemModel::GetSingleton()->MainThreadTick();
      nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));
    }
    CompareFiles({});
    CompareFolders({});
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Change file casing")
  {
    nsStringBuilder sFilePathOld(sOutputFolder);
    sFilePathOld.AppendPath("Folder12", "rootFile2.txt");

    nsStringBuilder sFilePathNew(sOutputFolder);
    sFilePathNew.AppendPath("Folder12", "RootFile2.txt");

    NS_TEST_RESULT(nsOSFile::MoveFileOrDirectory(sFilePathOld, sFilePathNew));

    for (nsUInt32 i = 0; i < WAIT_LOOPS; i++)
    {
      nsFileSystemModel::GetSingleton()->MainThreadTick();
      nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));

      NS_LOCK(fileEventLock);
      if (fileEvents.GetCount() == 2)
        break;
    }

    nsFileChangedEvent expected[] = {
      nsFileChangedEvent(MakePath(sFilePathNew), {}, nsFileChangedEvent::Type::FileAdded),
      nsFileChangedEvent(MakePath(sFilePathOld), {}, nsFileChangedEvent::Type::FileRemoved)};
    CompareFiles(nsMakeArrayPtr(expected));
    ClearFiles();
    CompareFolders({});

    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFiles()->GetCount(), 1);
    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFolders()->GetCount(), 3);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Change folder casing")
  {
    nsStringBuilder sFolderPathOld(sOutputFolder);
    sFolderPathOld.AppendPath("Folder12");

    nsStringBuilder sFolderPathNew(sOutputFolder);
    sFolderPathNew.AppendPath("FOLDER12");

    NS_TEST_RESULT(nsOSFile::MoveFileOrDirectory(sFolderPathOld, sFolderPathNew));

    for (nsUInt32 i = 0; i < WAIT_LOOPS; i++)
    {
      nsFileSystemModel::GetSingleton()->MainThreadTick();
      nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));

      NS_LOCK(fileEventLock);
      if (fileEvents.GetCount() == 2 && folderEvents.GetCount() == 2)
        break;
    }

    {
      nsFolderChangedEvent expected[] = {
        nsFolderChangedEvent(MakePath(sFolderPathNew), nsFolderChangedEvent::Type::FolderAdded),
        nsFolderChangedEvent(MakePath(sFolderPathOld), nsFolderChangedEvent::Type::FolderRemoved)};
      CompareFolders(nsMakeArrayPtr(expected));
      ClearFolders();
    }

    {
      nsStringBuilder sFilePathOld(sOutputFolder);
      sFilePathOld.AppendPath("Folder12", "RootFile2.txt");
      nsStringBuilder sFilePathNew(sOutputFolder);
      sFilePathNew.AppendPath("FOLDER12", "RootFile2.txt");

      nsFileChangedEvent expected[] = {
        nsFileChangedEvent(MakePath(sFilePathNew), {}, nsFileChangedEvent::Type::FileAdded),
        nsFileChangedEvent(MakePath(sFilePathOld), {}, nsFileChangedEvent::Type::FileRemoved)};
      CompareFiles(nsMakeArrayPtr(expected));
      ClearFiles();
    }

    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFiles()->GetCount(), 1);
    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFolders()->GetCount(), 3);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "delete folder")
  {
    nsStringBuilder sFolderPath(sOutputFolder);
    sFolderPath.AppendPath("FOLDER12");

    nsStringBuilder sFilePath(sOutputFolder);
    sFilePath.AppendPath("FOLDER12", "RootFile2.txt");

    NS_TEST_RESULT(nsOSFile::DeleteFolder(sFolderPath));

    for (nsUInt32 i = 0; i < WAIT_LOOPS; i++)
    {
      nsFileSystemModel::GetSingleton()->MainThreadTick();
      nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));

      NS_LOCK(fileEventLock);
      NS_LOCK(folderEventLock);
      if (fileEvents.GetCount() == 1 && folderEvents.GetCount() == 1)
        break;
    }

    {
      nsFolderChangedEvent expected[] = {
        nsFolderChangedEvent(MakePath(sFolderPath), nsFolderChangedEvent::Type::FolderRemoved)};
      CompareFolders(nsMakeArrayPtr(expected));
    }

    {
      nsFileChangedEvent expected[] = {
        nsFileChangedEvent(MakePath(sFilePath), {}, nsFileChangedEvent::Type::FileRemoved)};
      CompareFiles(nsMakeArrayPtr(expected));
    }

    {
      NS_LOCK(fileEventLock);
      NS_LOCK(folderEventLock);
      // Check file removed before folder.
      NS_TEST_BOOL(fileEventTimestamps[0] < folderEventTimestamps[0]);
    }

    ClearFolders();
    ClearFiles();

    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFiles()->GetCount(), 0);
    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFolders()->GetCount(), 2);
  }

  referencedFiles = {};
  referencedFolders = {};

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Shutdown with cached files and folders")
  {
    {
      // Add a file to test data directories being removed.
      nsStringBuilder sFilePath(sOutputFolder);
      sFilePath.AppendPath("rootFile.txt");

      NS_TEST_RESULT(nstCreateFile(sFilePath));

      for (nsUInt32 i = 0; i < WAIT_LOOPS; i++)
      {
        nsFileSystemModel::GetSingleton()->MainThreadTick();
        nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));

        NS_LOCK(fileEventLock);
        if (fileEvents.GetCount() > 0)
          break;
      }

      nsFileChangedEvent expected[] = {nsFileChangedEvent(MakePath(sFilePath), {}, nsFileChangedEvent::Type::FileAdded)};
      CompareFiles(nsMakeArrayPtr(expected));
      ClearFiles();
      CompareFolders({});
    }

    nsFileSystemModel::GetSingleton()->Deinitialize(&referencedFiles, &referencedFolders);
    NS_TEST_INT(referencedFiles.GetCount(), 1);
    NS_TEST_INT(referencedFolders.GetCount(), 2);

    nsFileChangedEvent expected[] = {nsFileChangedEvent({}, {}, nsFileChangedEvent::Type::ModelReset)};
    CompareFiles(nsMakeArrayPtr(expected));
    ClearFiles();

    nsFolderChangedEvent expected2[] = {nsFolderChangedEvent({}, nsFolderChangedEvent::Type::ModelReset)};
    CompareFolders(nsMakeArrayPtr(expected2));
    ClearFolders();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Startup without data dirs")
  {
    fsConfig.m_DataDirs.Clear();

    nsFileSystemModel::GetSingleton()->Initialize(fsConfig, std::move(referencedFiles), std::move(referencedFolders));
    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFiles()->GetCount(), 0);
    NS_TEST_INT(nsFileSystemModel::GetSingleton()->GetFolders()->GetCount(), 0);

    nsFileChangedEvent expected[] = {nsFileChangedEvent({}, {}, nsFileChangedEvent::Type::ModelReset)};
    CompareFiles(nsMakeArrayPtr(expected));
    ClearFiles();

    nsFolderChangedEvent expected2[] = {nsFolderChangedEvent({}, nsFolderChangedEvent::Type::ModelReset)};
    CompareFolders(nsMakeArrayPtr(expected2));
    ClearFolders();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Final shutdown")
  {
    nsFileSystemModel::GetSingleton()->Deinitialize();
    nsFileChangedEvent expected[] = {nsFileChangedEvent({}, {}, nsFileChangedEvent::Type::ModelReset)};
    CompareFiles(nsMakeArrayPtr(expected));
    ClearFiles();

    nsFolderChangedEvent expected2[] = {nsFolderChangedEvent({}, nsFolderChangedEvent::Type::ModelReset)};
    CompareFolders(nsMakeArrayPtr(expected2));
    ClearFolders();

    nsFileSystemModel::GetSingleton()->m_FileChangedEvents.RemoveEventHandler(fileId);
    nsFileSystemModel::GetSingleton()->m_FolderChangedEvents.RemoveEventHandler(folderId);
  }
}

#endif

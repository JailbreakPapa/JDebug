#include <ToolsFoundation/ToolsFoundationDLL.h>

#if NS_ENABLED(NS_SUPPORTS_DIRECTORY_WATCHER) && NS_ENABLED(NS_SUPPORTS_FILE_ITERATORS)

#  include <ToolsFoundation/FileSystem/FileSystemModel.h>
#  include <ToolsFoundation/FileSystem/FileSystemWatcher.h>

#  include <Foundation/Algorithm/HashStream.h>
#  include <Foundation/Configuration/SubSystem.h>
#  include <Foundation/IO/FileSystem/FileReader.h>
#  include <Foundation/IO/FileSystem/FileSystem.h>
#  include <Foundation/IO/MemoryStream.h>
#  include <Foundation/IO/OSFile.h>
#  include <Foundation/Logging/Log.h>
#  include <Foundation/Time/Stopwatch.h>
#  include <Foundation/Utilities/Progress.h>

NS_IMPLEMENT_SINGLETON(nsFileSystemModel);

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(ToolsFoundation, FileSystemModel)

  BEGIN_SUBSYSTEM_DEPENDENCIES
  "Foundation"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    NS_DEFAULT_NEW(nsFileSystemModel);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsFileSystemModel* pDummy = nsFileSystemModel::GetSingleton();
    NS_DEFAULT_DELETE(pDummy);
  }

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

namespace
{
  thread_local nsHybridArray<nsFileChangedEvent, 2, nsStaticsAllocatorWrapper> g_PostponedFiles;
  thread_local bool g_bInFileBroadcast = false;
  thread_local nsHybridArray<nsFolderChangedEvent, 2, nsStaticsAllocatorWrapper> g_PostponedFolders;
  thread_local bool g_bInFolderBroadcast = false;
} // namespace

nsFolderChangedEvent::nsFolderChangedEvent(const nsDataDirPath& file, Type type)
  : m_Path(file)
  , m_Type(type)
{
}

nsFileChangedEvent::nsFileChangedEvent(const nsDataDirPath& file, nsFileStatus status, Type type)
  : m_Path(file)
  , m_Status(status)
  , m_Type(type)
{
}

bool nsFileSystemModel::IsSameFile(const nsStringView sAbsolutePathA, const nsStringView sAbsolutePathB)
{
#  if (NS_ENABLED(NS_SUPPORTS_CASE_INSENSITIVE_PATHS))
  return sAbsolutePathA.IsEqual_NoCase(sAbsolutePathB);
#  else
  return sAbsolutePathA.IsEqual(sAbsolutePathB);
#  endif
}

////////////////////////////////////////////////////////////////////////
// nsAssetFiles
////////////////////////////////////////////////////////////////////////

nsFileSystemModel::nsFileSystemModel()
  : m_SingletonRegistrar(this)
{
}

nsFileSystemModel::~nsFileSystemModel() = default;

void nsFileSystemModel::Initialize(const nsApplicationFileSystemConfig& fileSystemConfig, nsFileSystemModel::FilesMap&& referencedFiles, nsFileSystemModel::FoldersMap&& referencedFolders)
{
  {
    NS_PROFILE_SCOPE("Initialize");
    NS_LOCK(m_FilesMutex);
    m_FileSystemConfig = fileSystemConfig;

    m_ReferencedFiles = std::move(referencedFiles);
    m_ReferencedFolders = std::move(referencedFolders);

    nsStringBuilder sDataDirPath;
    m_DataDirRoots.Reserve(m_FileSystemConfig.m_DataDirs.GetCount());
    for (nsUInt32 i = 0; i < m_FileSystemConfig.m_DataDirs.GetCount(); ++i)
    {
      if (nsFileSystem::ResolveSpecialDirectory(m_FileSystemConfig.m_DataDirs[i].m_sDataDirSpecialPath, sDataDirPath).Failed())
      {
        nsLog::Error("Failed to resolve data directory named '{}' at '{}'", m_FileSystemConfig.m_DataDirs[i].m_sRootName, m_FileSystemConfig.m_DataDirs[i].m_sDataDirSpecialPath);
        m_DataDirRoots.PushBack({});
      }
      else
      {
        sDataDirPath.MakeCleanPath();
        sDataDirPath.Trim(nullptr, "/");

        m_DataDirRoots.PushBack(sDataDirPath);

        // The root should always be in the model so that every file's parent folder is present in the model.
        m_ReferencedFolders.FindOrAdd(nsDataDirPath(sDataDirPath, m_DataDirRoots, i)).Value() = nsFileStatus::Status::Valid;
      }
    }

    // Update data dir index and remove files no longer inside a data dir.
    for (auto it = m_ReferencedFiles.GetIterator(); it.IsValid();)
    {
      const bool bValid = it.Key().UpdateDataDirInfos(m_DataDirRoots, it.Key().GetDataDirIndex());
      if (!bValid)
      {
        it = m_ReferencedFiles.Remove(it);
      }
      else
      {
        ++it;
      }
    }
    for (auto it = m_ReferencedFolders.GetIterator(); it.IsValid();)
    {
      const bool bValid = it.Key().UpdateDataDirInfos(m_DataDirRoots, it.Key().GetDataDirIndex());
      if (!bValid)
      {
        it = m_ReferencedFolders.Remove(it);
      }
      else
      {
        ++it;
      }
    }

    m_pWatcher = NS_DEFAULT_NEW(nsFileSystemWatcher, m_FileSystemConfig);
    m_WatcherSubscription = m_pWatcher->m_Events.AddEventHandler(nsMakeDelegate(&nsFileSystemModel::OnAssetWatcherEvent, this));
    m_pWatcher->Initialize();
    m_bInitialized = true;
  }
  FireFileChangedEvent({}, {}, nsFileChangedEvent::Type::ModelReset);
  FireFolderChangedEvent({}, nsFolderChangedEvent::Type::ModelReset);
}


void nsFileSystemModel::Deinitialize(nsFileSystemModel::FilesMap* out_pReferencedFiles, nsFileSystemModel::FoldersMap* out_pReferencedFolders)
{
  {
    NS_LOCK(m_FilesMutex);
    NS_PROFILE_SCOPE("Deinitialize");
    m_pWatcher->m_Events.RemoveEventHandler(m_WatcherSubscription);
    m_pWatcher->Deinitialize();
    m_pWatcher.Clear();

    if (out_pReferencedFiles)
    {
      m_ReferencedFiles.Swap(*out_pReferencedFiles);
    }
    if (out_pReferencedFolders)
    {
      m_ReferencedFolders.Swap(*out_pReferencedFolders);
    }
    m_ReferencedFiles.Clear();
    m_ReferencedFolders.Clear();
    m_LockedFiles.Clear();
    m_FileSystemConfig = nsApplicationFileSystemConfig();
    m_DataDirRoots.Clear();
    m_bInitialized = false;
  }
  FireFileChangedEvent({}, {}, nsFileChangedEvent::Type::ModelReset);
  FireFolderChangedEvent({}, nsFolderChangedEvent::Type::ModelReset);
}

void nsFileSystemModel::MainThreadTick()
{
  if (m_pWatcher)
    m_pWatcher->MainThreadTick();
}

const nsFileSystemModel::LockedFiles nsFileSystemModel::GetFiles() const
{
  return LockedFiles(m_FilesMutex, &m_ReferencedFiles);
}


const nsFileSystemModel::LockedFolders nsFileSystemModel::GetFolders() const
{
  return LockedFolders(m_FilesMutex, &m_ReferencedFolders);
}

void nsFileSystemModel::NotifyOfChange(nsStringView sAbsolutePath)
{
  if (!m_bInitialized)
    return;

  NS_ASSERT_DEV(nsPathUtils::IsAbsolutePath(sAbsolutePath), "Only absolute paths are supported for directory iteration.");

  nsStringBuilder sPath(sAbsolutePath);
  sPath.MakeCleanPath();
  sPath.Trim(nullptr, "/");
  if (sPath.IsEmpty())
    return;
  nsDataDirPath folder(sPath, m_DataDirRoots);

  // We ignore any changes outside the model's data dirs.
  if (!folder.IsValid())
    return;

  HandleSingleFile(std::move(folder), true);
}

void nsFileSystemModel::CheckFileSystem()
{
  if (!m_bInitialized)
    return;

  NS_PROFILE_SCOPE("CheckFileSystem");

  nsUniquePtr<nsProgressRange> range = nullptr;
  if (nsThreadUtils::IsMainThread())
    range = NS_DEFAULT_NEW(nsProgressRange, "Check File-System for Assets", m_FileSystemConfig.m_DataDirs.GetCount(), false);

  {
    SetAllStatusUnknown();

    // check every data directory
    for (nsUInt32 i = 0; i < m_FileSystemConfig.m_DataDirs.GetCount(); i++)
    {
      auto& dd = m_FileSystemConfig.m_DataDirs[i];
      if (nsThreadUtils::IsMainThread())
        range->BeginNextStep(dd.m_sDataDirSpecialPath);
      if (!m_DataDirRoots[i].IsEmpty())
      {
        CheckFolder(m_DataDirRoots[i]);
      }
    }

    RemoveStaleFileInfos();
  }

  if (nsThreadUtils::IsMainThread())
  {
    range = nullptr;
  }

  FireFileChangedEvent({}, {}, nsFileChangedEvent::Type::ModelReset);
  FireFolderChangedEvent({}, nsFolderChangedEvent::Type::ModelReset);
}


nsResult nsFileSystemModel::FindFile(nsStringView sPath, nsFileStatus& out_stat) const
{
  if (!m_bInitialized)
    return NS_FAILURE;

  NS_LOCK(m_FilesMutex);
  nsFileSystemModel::FilesMap::ConstIterator it;
  if (nsPathUtils::IsAbsolutePath(sPath))
  {
    it = m_ReferencedFiles.Find(sPath);
  }
  else
  {
    // Data dir parent relative?
    for (const auto& dd : m_FileSystemConfig.m_DataDirs)
    {
      nsStringBuilder sDataDir;
      nsFileSystem::ResolveSpecialDirectory(dd.m_sDataDirSpecialPath, sDataDir).AssertSuccess();
      sDataDir.PathParentDirectory();
      sDataDir.AppendPath(sPath);
      it = m_ReferencedFiles.Find(sDataDir);
      if (it.IsValid())
        break;
    }

    if (!it.IsValid())
    {
      // Data dir relative?
      for (const auto& dd : m_FileSystemConfig.m_DataDirs)
      {
        nsStringBuilder sDataDir;
        nsFileSystem::ResolveSpecialDirectory(dd.m_sDataDirSpecialPath, sDataDir).AssertSuccess();
        sDataDir.AppendPath(sPath);
        it = m_ReferencedFiles.Find(sDataDir);
        if (it.IsValid())
          break;
      }
    }
  }

  if (it.IsValid())
  {
    out_stat = it.Value();
    return NS_SUCCESS;
  }
  return NS_FAILURE;
}


nsResult nsFileSystemModel::FindFile(nsDelegate<bool(const nsDataDirPath&, const nsFileStatus&)> visitor) const
{
  if (!m_bInitialized)
    return NS_FAILURE;

  NS_LOCK(m_FilesMutex);
  for (auto it = m_ReferencedFiles.GetIterator(); it.IsValid(); ++it)
  {
    if (visitor(it.Key(), it.Value()))
      return NS_SUCCESS;
  }
  return NS_FAILURE;
}


nsResult nsFileSystemModel::LinkDocument(nsStringView sAbsolutePath, const nsUuid& documentId)
{
  if (!m_bInitialized || !documentId.IsValid())
    return NS_FAILURE;

  nsDataDirPath filePath;
  nsFileStatus fileStatus;
  {
    NS_LOCK(m_FilesMutex);
    auto it = m_ReferencedFiles.Find(sAbsolutePath);
    if (it.IsValid())
    {
      // Store status before updates so we can fire the unlink if a guid was already set.
      fileStatus = it.Value();
      it.Value().m_DocumentID = documentId;
      filePath = it.Key();
    }
    else
    {
      return NS_FAILURE;
    }
  }

  if (fileStatus.m_DocumentID != documentId)
  {
    if (fileStatus.m_DocumentID.IsValid())
    {
      FireFileChangedEvent(filePath, fileStatus, nsFileChangedEvent::Type::DocumentUnlinked);
    }
    fileStatus.m_DocumentID = documentId;
    FireFileChangedEvent(std::move(filePath), fileStatus, nsFileChangedEvent::Type::DocumentLinked);
  }
  return NS_SUCCESS;
}

nsResult nsFileSystemModel::UnlinkDocument(nsStringView sAbsolutePath)
{
  if (!m_bInitialized)
    return NS_FAILURE;

  nsDataDirPath filePath;
  nsFileStatus fileStatus;
  bool bDocumentLinkChanged = false;
  {
    NS_LOCK(m_FilesMutex);
    auto it = m_ReferencedFiles.Find(sAbsolutePath);
    if (it.IsValid())
    {
      bDocumentLinkChanged = it.Value().m_DocumentID.IsValid();
      fileStatus = it.Value();
      it.Value().m_DocumentID = nsUuid::MakeInvalid();
      filePath = it.Key();
    }
    else
    {
      return NS_FAILURE;
    }
  }

  if (bDocumentLinkChanged)
  {
    FireFileChangedEvent(std::move(filePath), fileStatus, nsFileChangedEvent::Type::DocumentUnlinked);
  }
  return NS_SUCCESS;
}

nsResult nsFileSystemModel::HashFile(nsStringView sAbsolutePath, nsFileStatus& out_stat)
{
  if (!m_bInitialized)
    return NS_FAILURE;

  NS_ASSERT_DEV(nsPathUtils::IsAbsolutePath(sAbsolutePath), "Only absolute paths are supported for hashing.");

  nsStringBuilder sAbsolutePath2(sAbsolutePath);
  sAbsolutePath2.MakeCleanPath();
  sAbsolutePath2.Trim("", "/");
  if (sAbsolutePath2.IsEmpty())
    return NS_FAILURE;

  nsDataDirPath file(sAbsolutePath2, m_DataDirRoots);

  nsFileStats statDep;
  if (nsOSFile::GetFileStats(sAbsolutePath2, statDep).Failed())
  {
    nsLog::Error("Failed to hash file '{0}', retrieve stats failed", sAbsolutePath2);
    return NS_FAILURE;
  }

  // We ignore any changes outside the model's data dirs.
  if (file.IsValid())
  {
    {
      NS_LOCK(m_FilesMutex);
      auto it = m_ReferencedFiles.Find(sAbsolutePath2);
      if (it.IsValid())
      {
        out_stat = it.Value();
      }
    }

    // We can only hash files that are tracked.
    if (out_stat.m_Status == nsFileStatus::Status::Unknown)
    {
      out_stat = HandleSingleFile(file, statDep, false);
      if (out_stat.m_Status == nsFileStatus::Status::Unknown)
      {
        nsLog::Error("Failed to hash file '{0}', update failed", sAbsolutePath2);
        return NS_FAILURE;
      }
    }

    // if the file has been modified, make sure to get updated data
    if (!out_stat.m_LastModified.Compare(statDep.m_LastModificationTime, nsTimestamp::CompareMode::Identical) || out_stat.m_uiHash == 0)
    {
      FILESYSTEM_PROFILE(sAbsolutePath2);
      nsFileReader fileReader;
      if (fileReader.Open(sAbsolutePath2).Failed())
      {
        MarkFileLocked(sAbsolutePath2);
        nsLog::Error("Failed to hash file '{0}', open failed", sAbsolutePath2);
        return NS_FAILURE;
      }

      // We need to request the stats again wile while we have shared read access or we might trigger a race condition of writes to the file between the last stat call and the current file open.
      if (nsOSFile::GetFileStats(sAbsolutePath2, statDep).Failed())
      {
        nsLog::Error("Failed to hash file '{0}', retrieve stats failed", sAbsolutePath2);
        return NS_FAILURE;
      }
      out_stat.m_LastModified = statDep.m_LastModificationTime;
      out_stat.m_uiHash = nsFileSystemModel::HashFile(fileReader, nullptr);
      out_stat.m_Status = nsFileStatus::Status::Valid;

      // Update state. No need to compare timestamps we hold a lock on the file via the reader.
      NS_LOCK(m_FilesMutex);
      m_ReferencedFiles.Insert(file, out_stat);
    }
    return NS_SUCCESS;
  }
  else
  {
    {
      NS_LOCK(m_FilesMutex);
      auto it = m_TransiendFiles.Find(sAbsolutePath2);
      if (it.IsValid())
      {
        out_stat = it.Value();
      }
    }

    // if the file has been modified, make sure to get updated data
    if (!out_stat.m_LastModified.Compare(statDep.m_LastModificationTime, nsTimestamp::CompareMode::Identical) || out_stat.m_uiHash == 0)
    {
      FILESYSTEM_PROFILE(sAbsolutePath2);
      nsFileReader modifiedFile;
      if (modifiedFile.Open(sAbsolutePath2).Failed())
      {
        nsLog::Error("Failed to hash file '{0}', open failed", sAbsolutePath2);
        return NS_FAILURE;
      }

      // We need to request the stats again wile while we have shared read access or we might trigger a race condition of writes to the file between the last stat call and the current file open.
      if (nsOSFile::GetFileStats(sAbsolutePath2, statDep).Failed())
      {
        nsLog::Error("Failed to hash file '{0}', retrieve stats failed", sAbsolutePath2);
        return NS_FAILURE;
      }
      out_stat.m_LastModified = statDep.m_LastModificationTime;
      out_stat.m_uiHash = nsFileSystemModel::HashFile(modifiedFile, nullptr);
      out_stat.m_Status = nsFileStatus::Status::Valid;

      // Update state. No need to compare timestamps we hold a lock on the file via the reader.
      NS_LOCK(m_FilesMutex);
      m_TransiendFiles.Insert(sAbsolutePath2, out_stat);
    }
    return NS_SUCCESS;
  }
}


nsUInt64 nsFileSystemModel::HashFile(nsStreamReader& ref_inputStream, nsStreamWriter* pPassThroughStream)
{
  nsHashStreamWriter64 hsw;

  FILESYSTEM_PROFILE("HashFile");
  nsUInt8 cachedBytes[1024 * 10];

  while (true)
  {
    const nsUInt64 uiRead = ref_inputStream.ReadBytes(cachedBytes, NS_ARRAY_SIZE(cachedBytes));

    if (uiRead == 0)
      break;

    hsw.WriteBytes(cachedBytes, uiRead).AssertSuccess();

    if (pPassThroughStream != nullptr)
      pPassThroughStream->WriteBytes(cachedBytes, uiRead).AssertSuccess();
  }

  return hsw.GetHashValue();
}

nsResult nsFileSystemModel::ReadDocument(nsStringView sAbsolutePath, const nsDelegate<void(const nsFileStatus&, nsStreamReader&)>& callback)
{
  if (!m_bInitialized)
    return NS_FAILURE;

  nsStringBuilder sAbsolutePath2(sAbsolutePath);
  sAbsolutePath2.MakeCleanPath();
  sAbsolutePath2.Trim(nullptr, "/");

  // try to read the asset file
  nsFileReader file;
  if (file.Open(sAbsolutePath2) == NS_FAILURE)
  {
    MarkFileLocked(sAbsolutePath2);
    nsLog::Error("Failed to open file '{0}'", sAbsolutePath2);
    return NS_FAILURE;
  }

  // Get model state.
  nsFileStatus stat;
  {
    NS_LOCK(m_FilesMutex);
    auto it = m_ReferencedFiles.Find(sAbsolutePath2);
    if (!it.IsValid())
      return NS_FAILURE;

    stat = it.Value();
  }

  // Get current state.
  nsFileStats statDep;
  if (nsOSFile::GetFileStats(sAbsolutePath, statDep).Failed())
  {
    nsLog::Error("Failed to retrieve file stats '{0}'", sAbsolutePath);
    return NS_FAILURE;
  }

  nsDefaultMemoryStreamStorage storage;
  nsMemoryStreamReader MemReader(&storage);
  MemReader.SetDebugSourceInformation(sAbsolutePath);

  nsMemoryStreamWriter MemWriter(&storage);
  stat.m_LastModified = statDep.m_LastModificationTime;
  stat.m_Status = nsFileStatus::Status::Valid;
  stat.m_uiHash = nsFileSystemModel::HashFile(file, &MemWriter);

  if (callback.IsValid())
  {
    callback(stat, MemReader);
  }

  bool bFileChanged = false;
  {
    // Update state. No need to compare timestamps we hold a lock on the file via the reader.
    NS_LOCK(m_FilesMutex);
    auto it = m_ReferencedFiles.Find(sAbsolutePath2);
    if (it.IsValid())
    {
      bFileChanged = !it.Value().m_LastModified.Compare(stat.m_LastModified, nsTimestamp::CompareMode::Identical);
      it.Value() = stat;
    }
    else
    {
      NS_REPORT_FAILURE("A file was removed from the model while we had a lock on it.");
    }

    if (bFileChanged)
    {
      FireFileChangedEvent(it.Key(), stat, nsFileChangedEvent::Type::FileChanged);
    }
  }

  return NS_SUCCESS;
}

void nsFileSystemModel::SetAllStatusUnknown()
{
  NS_LOCK(m_FilesMutex);
  for (auto it = m_ReferencedFiles.GetIterator(); it.IsValid(); ++it)
  {
    it.Value().m_Status = nsFileStatus::Status::Unknown;
  }

  for (auto it = m_ReferencedFolders.GetIterator(); it.IsValid(); ++it)
  {
    it.Value() = nsFileStatus::Status::Unknown;
  }
}


void nsFileSystemModel::RemoveStaleFileInfos()
{
  nsSet<nsDataDirPath> unknownFiles;
  nsSet<nsDataDirPath> unknownFolders;
  {
    NS_LOCK(m_FilesMutex);
    for (auto it = m_ReferencedFiles.GetIterator(); it.IsValid(); ++it)
    {
      // search for files that existed previously but have not been found anymore recently
      if (it.Value().m_Status == nsFileStatus::Status::Unknown)
      {
        unknownFiles.Insert(it.Key());
      }
    }
    for (auto it = m_ReferencedFolders.GetIterator(); it.IsValid(); ++it)
    {
      // search for folders that existed previously but have not been found anymore recently
      if (it.Value() == nsFileStatus::Status::Unknown)
      {
        unknownFolders.Insert(it.Key());
      }
    }
  }

  for (const nsDataDirPath& file : unknownFiles)
  {
    HandleSingleFile(file, false);
  }
  for (const nsDataDirPath& folders : unknownFolders)
  {
    HandleSingleFile(folders, false);
  }
}


void nsFileSystemModel::CheckFolder(nsStringView sAbsolutePath)
{
  nsStringBuilder sAbsolutePath2 = sAbsolutePath;
  sAbsolutePath2.MakeCleanPath();
  NS_ASSERT_DEV(nsPathUtils::IsAbsolutePath(sAbsolutePath2), "Only absolute paths are supported for directory iteration.");
  sAbsolutePath2.Trim(nullptr, "/");

  if (sAbsolutePath2.IsEmpty())
    return;

  nsDataDirPath folder(sAbsolutePath2, m_DataDirRoots);

  // We ignore any changes outside the model's data dirs.
  if (!folder.IsValid())
    return;

  bool bExists = false;
  {
    NS_LOCK(m_FilesMutex);
    bExists = m_ReferencedFolders.Contains(folder);
  }
  if (!bExists)
  {
    // If the folder does not exist yet we call NotifyOfChange which handles add / removal recursively as well.
    NotifyOfChange(folder);
    return;
  }

  nsFileSystemIterator iterator;
  iterator.StartSearch(sAbsolutePath2, nsFileSystemIteratorFlags::ReportFilesAndFoldersRecursive);

  if (!iterator.IsValid())
    return;

  nsStringBuilder sPath;

  nsSet<nsString> visitedFiles;
  nsSet<nsString> visitedFolders;
  visitedFolders.Insert(sAbsolutePath2);

  for (; iterator.IsValid(); iterator.Next())
  {
    sPath = iterator.GetCurrentPath();
    sPath.AppendPath(iterator.GetStats().m_sName);
    sPath.MakeCleanPath();
    if (iterator.GetStats().m_bIsDirectory)
      visitedFolders.Insert(sPath);
    else
      visitedFiles.Insert(sPath);

    nsDataDirPath path(sPath, m_DataDirRoots, folder.GetDataDirIndex());
    HandleSingleFile(std::move(path), iterator.GetStats(), false);
  }

  nsDynamicArray<nsString> missingFiles;
  nsDynamicArray<nsString> missingFolders;

  {
    NS_LOCK(m_FilesMutex);

    // As we are using nsCompareDataDirPath, entries of different casing interleave but we are only interested in the ones with matching casing so we skip the rest.
    for (auto it = m_ReferencedFiles.LowerBound(sAbsolutePath2.GetView()); it.IsValid(); ++it)
    {
      if (nsPathUtils::IsSubPath(sAbsolutePath2, it.Key().GetAbsolutePath()) && !visitedFiles.Contains(it.Key().GetAbsolutePath()))
        missingFiles.PushBack(it.Key().GetAbsolutePath());
      if (!it.Key().GetAbsolutePath().StartsWith_NoCase(sAbsolutePath2))
        break;
    }

    for (auto it = m_ReferencedFolders.LowerBound(sAbsolutePath2.GetView()); it.IsValid(); ++it)
    {
      if (nsPathUtils::IsSubPath(sAbsolutePath2, it.Key().GetAbsolutePath()) && !visitedFolders.Contains(it.Key().GetAbsolutePath()))
        missingFolders.PushBack(it.Key().GetAbsolutePath());
      if (!it.Key().GetAbsolutePath().StartsWith_NoCase(sAbsolutePath2))
        break;
    }
  }

  for (nsString& sFile : missingFiles)
  {
    nsDataDirPath path(std::move(sFile), m_DataDirRoots, folder.GetDataDirIndex());
    HandleSingleFile(std::move(path), false);
  }

  // Delete sub-folders before parent folders.
  missingFolders.Sort([](const nsString& lhs, const nsString& rhs) -> bool
    { return nsStringUtils::Compare(lhs, rhs) > 0; });
  for (nsString& sFolder : missingFolders)
  {
    nsDataDirPath path(std::move(sFolder), m_DataDirRoots, folder.GetDataDirIndex());
    HandleSingleFile(std::move(path), false);
  }
}

void nsFileSystemModel::OnAssetWatcherEvent(const nsFileSystemWatcherEvent& e)
{
  switch (e.m_Type)
  {
    case nsFileSystemWatcherEvent::Type::FileAdded:
    case nsFileSystemWatcherEvent::Type::FileRemoved:
    case nsFileSystemWatcherEvent::Type::FileChanged:
    case nsFileSystemWatcherEvent::Type::DirectoryAdded:
    case nsFileSystemWatcherEvent::Type::DirectoryRemoved:
      NotifyOfChange(e.m_sPath);
      break;
  }
}

nsFileStatus nsFileSystemModel::HandleSingleFile(nsDataDirPath absolutePath, bool bRecurseIntoFolders)
{
  FILESYSTEM_PROFILE("HandleSingleFile");

  nsFileStats Stats;
  const nsResult statCheck = nsOSFile::GetFileStats(absolutePath, Stats);

#  if NS_ENABLED(NS_PLATFORM_WINDOWS)
  if (statCheck.Succeeded() && Stats.m_sName != nsPathUtils::GetFileNameAndExtension(absolutePath))
  {
    // Casing has changed.
    nsStringBuilder sCorrectCasingPath = absolutePath.GetAbsolutePath();
    sCorrectCasingPath.ChangeFileNameAndExtension(Stats.m_sName);
    nsDataDirPath correctCasingPath(sCorrectCasingPath.GetView(), m_DataDirRoots, absolutePath.GetDataDirIndex());
    // Add new casing
    nsFileStatus res = HandleSingleFile(std::move(correctCasingPath), Stats, bRecurseIntoFolders);
    // Remove old casing
    RemoveFileOrFolder(absolutePath, bRecurseIntoFolders);
    return res;
  }
#  endif

  if (statCheck.Failed())
  {
    RemoveFileOrFolder(absolutePath, bRecurseIntoFolders);
    return {};
  }

  return HandleSingleFile(std::move(absolutePath), Stats, bRecurseIntoFolders);
}


nsFileStatus nsFileSystemModel::HandleSingleFile(nsDataDirPath absolutePath, const nsFileStats& FileStat, bool bRecurseIntoFolders)
{
  FILESYSTEM_PROFILE("HandleSingleFile2");

  if (FileStat.m_bIsDirectory)
  {
    nsFileStatus status;
    status.m_Status = nsFileStatus::Status::Valid;

    bool bExisted = false;
    {
      NS_LOCK(m_FilesMutex);
      auto it = m_ReferencedFolders.FindOrAdd(absolutePath, &bExisted);
      it.Value() = nsFileStatus::Status::Valid;
    }

    if (!bExisted)
    {
      FireFolderChangedEvent(absolutePath, nsFolderChangedEvent::Type::FolderAdded);
      if (bRecurseIntoFolders)
        CheckFolder(absolutePath);
    }

    return status;
  }
  else
  {
    nsFileStatus status;
    bool bExisted = false;
    bool bFileChanged = false;
    {
      NS_LOCK(m_FilesMutex);
      auto it = m_ReferencedFiles.FindOrAdd(absolutePath, &bExisted);
      nsFileStatus& value = it.Value();
      bFileChanged = !value.m_LastModified.Compare(FileStat.m_LastModificationTime, nsTimestamp::CompareMode::Identical);
      if (bFileChanged)
      {
        value.m_uiHash = 0;
      }

      // If the state is unknown, we loaded it from the cache and need to fire FileChanged to update dependent systems.
      // #TODO_ASSET This behaviors should be changed once the asset cache is stored less lossy.
      bFileChanged |= value.m_Status == nsFileStatus::Status::Unknown;
      // mark the file as valid (i.e. we saw it on disk, so it hasn't been deleted or such)
      value.m_Status = nsFileStatus::Status::Valid;
      value.m_LastModified = FileStat.m_LastModificationTime;
      status = value;
    }

    if (!bExisted)
    {
      FireFileChangedEvent(absolutePath, status, nsFileChangedEvent::Type::FileAdded);
    }
    else if (bFileChanged)
    {
      FireFileChangedEvent(absolutePath, status, nsFileChangedEvent::Type::FileChanged);
    }
    return status;
  }
}

void nsFileSystemModel::RemoveFileOrFolder(const nsDataDirPath& absolutePath, bool bRecurseIntoFolders)
{
  nsFileStatus fileStatus;
  bool bFileExisted = false;
  bool bFolderExisted = false;
  {
    NS_LOCK(m_FilesMutex);
    if (auto it = m_ReferencedFiles.Find(absolutePath); it.IsValid())
    {
      bFileExisted = true;
      fileStatus = it.Value();
      m_ReferencedFiles.Remove(it);
    }
    if (auto it = m_ReferencedFolders.Find(absolutePath); it.IsValid())
    {
      bFolderExisted = true;
      m_ReferencedFolders.Remove(it);
    }
  }

  if (bFileExisted)
  {
    FireFileChangedEvent(absolutePath, fileStatus, nsFileChangedEvent::Type::FileRemoved);
  }

  if (bFolderExisted)
  {
    if (bRecurseIntoFolders)
    {
      nsSet<nsDataDirPath> previouslyKnownFiles;
      {
        FILESYSTEM_PROFILE("FindReferencedFiles");
        NS_LOCK(m_FilesMutex);
        auto itlowerBound = m_ReferencedFiles.LowerBound(absolutePath);
        while (itlowerBound.IsValid())
        {
          if (nsPathUtils::IsSubPath(absolutePath, itlowerBound.Key().GetAbsolutePath()))
          {
            previouslyKnownFiles.Insert(itlowerBound.Key());
          }
          // As we are using nsCompareDataDirPath, entries of different casing interleave but we are only interested in the ones with matching casing so we skip the rest.
          if (!itlowerBound.Key().GetAbsolutePath().StartsWith_NoCase(absolutePath.GetAbsolutePath()))
          {
            break;
          }
          ++itlowerBound;
        }
      }
      {
        FILESYSTEM_PROFILE("HandleRemovedFiles");
        for (const nsDataDirPath& file : previouslyKnownFiles)
        {
          RemoveFileOrFolder(file, false);
        }
      }
    }
    FireFolderChangedEvent(absolutePath, nsFolderChangedEvent::Type::FolderRemoved);
  }
}

void nsFileSystemModel::MarkFileLocked(nsStringView sAbsolutePath)
{
  NS_LOCK(m_FilesMutex);
  auto it = m_ReferencedFiles.Find(sAbsolutePath);
  if (it.IsValid())
  {
    it.Value().m_Status = nsFileStatus::Status::FileLocked;
    m_LockedFiles.Insert(sAbsolutePath);
  }
}

void nsFileSystemModel::FireFileChangedEvent(const nsDataDirPath& file, nsFileStatus fileStatus, nsFileChangedEvent::Type type)
{
  // We queue up all requests on a thread and only return once the list is empty. The reason for this is that:
  // A: We don't want to allow recursive event calling as it creates limbo states in the model and hard to debug bugs.
  // B: If a user calls NotifyOfChange, the function should only return if the event and any indirect events that were triggered by the event handlers have been processed.

  nsFileChangedEvent& e = g_PostponedFiles.ExpandAndGetRef();
  e.m_Path = file;
  e.m_Status = fileStatus;
  e.m_Type = type;

  if (g_bInFileBroadcast)
  {
    return;
  }

  g_bInFileBroadcast = true;
  NS_SCOPE_EXIT(g_bInFileBroadcast = false);

  for (nsUInt32 i = 0; i < g_PostponedFiles.GetCount(); i++)
  {
    // Need to make a copy as new elements can be added and the array resized during broadcast.
    nsFileChangedEvent tempEvent = std::move(g_PostponedFiles[i]);
    m_FileChangedEvents.Broadcast(tempEvent);
  }
  g_PostponedFiles.Clear();
}

void nsFileSystemModel::FireFolderChangedEvent(const nsDataDirPath& file, nsFolderChangedEvent::Type type)
{
  // See comment in FireFileChangedEvent.
  nsFolderChangedEvent& e = g_PostponedFolders.ExpandAndGetRef();
  e.m_Path = file;
  e.m_Type = type;

  if (g_bInFolderBroadcast)
  {
    return;
  }

  g_bInFolderBroadcast = true;
  NS_SCOPE_EXIT(g_bInFolderBroadcast = false);

  for (nsUInt32 i = 0; i < g_PostponedFolders.GetCount(); i++)
  {
    // Need to make a copy as new elements can be added and the array resized during broadcast.
    nsFolderChangedEvent tempEvent = std::move(g_PostponedFolders[i]);
    m_FolderChangedEvents.Broadcast(tempEvent);
  }
  g_PostponedFolders.Clear();
}

#endif

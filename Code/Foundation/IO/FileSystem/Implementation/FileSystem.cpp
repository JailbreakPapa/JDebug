#include <Foundation/FoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/Implementation/StringIterator.h>
#include <Foundation/Strings/StringView.h>

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(Foundation, FileSystem)

  ON_CORESYSTEMS_STARTUP
  {
    nsFileSystem::Startup();
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsFileSystem::Shutdown();
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

nsFileSystem::FileSystemData* nsFileSystem::s_pData = nullptr;
nsString nsFileSystem::s_sSdkRootDir;
nsMap<nsString, nsString> nsFileSystem::s_SpecialDirectories;


void nsFileSystem::RegisterDataDirectoryFactory(nsDataDirFactory factory, float fPriority /*= 0*/)
{
  NS_LOCK(s_pData->m_FsMutex);

  auto& data = s_pData->m_DataDirFactories.ExpandAndGetRef();
  data.m_Factory = factory;
  data.m_fPriority = fPriority;
}

nsEventSubscriptionID nsFileSystem::RegisterEventHandler(nsEvent<const FileEvent&>::Handler handler)
{
  NS_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  return s_pData->m_Event.AddEventHandler(handler);
}

void nsFileSystem::UnregisterEventHandler(nsEvent<const FileEvent&>::Handler handler)
{
  NS_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  s_pData->m_Event.RemoveEventHandler(handler);
}

void nsFileSystem::UnregisterEventHandler(nsEventSubscriptionID subscriptionId)
{
  NS_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  s_pData->m_Event.RemoveEventHandler(subscriptionId);
}

void nsFileSystem::CleanUpRootName(nsStringBuilder& sRoot)
{
  // this cleaning might actually make the root name empty
  // e.g. ":" becomes ""
  // which is intended to support passing through of absolute paths
  // ie. mounting the empty dir "" under the root ":" will allow to write directly to files using absolute paths

  while (sRoot.StartsWith(":"))
    sRoot.Shrink(1, 0);

  while (sRoot.EndsWith("/"))
    sRoot.Shrink(0, 1);

  sRoot.ToUpper();
}

nsResult nsFileSystem::AddDataDirectory(nsStringView sDataDirectory, nsStringView sGroup, nsStringView sRootName, DataDirUsage usage)
{
  NS_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");
  NS_ASSERT_DEV(usage != AllowWrites || !sRootName.IsEmpty(), "A data directory must have a non-empty, unique name to be mounted for write access");

  nsStringBuilder sPath = sDataDirectory;
  sPath.MakeCleanPath();

  if (!sPath.IsEmpty() && !sPath.EndsWith("/"))
    sPath.Append("/");

  nsStringBuilder sCleanRootName = sRootName;
  CleanUpRootName(sCleanRootName);

  NS_LOCK(s_pData->m_FsMutex);

  bool failed = false;
  if (FindDataDirectoryWithRoot(sCleanRootName) != nullptr)
  {
    nsLog::Error("A data directory with root name '{0}' already exists.", sCleanRootName);
    failed = true;
  }

  if (!failed)
  {
    s_pData->m_DataDirFactories.Sort([](const auto& a, const auto& b)
      { return a.m_fPriority < b.m_fPriority; });

    // use the factory that was added last as the one with the highest priority -> allows to override already added factories
    for (nsInt32 i = s_pData->m_DataDirFactories.GetCount() - 1; i >= 0; --i)
    {
      nsDataDirectoryType* pDataDir = s_pData->m_DataDirFactories[i].m_Factory(sPath, sGroup, sRootName, usage);

      if (pDataDir != nullptr)
      {
        DataDirectoryInfo dd;
        dd.m_Usage = usage;
        dd.m_pDataDirectory = pDataDir;
        dd.m_sRootName = sCleanRootName;
        dd.m_sGroup = sGroup;

        s_pData->m_DataDirectories.PushBack(dd);

        {
          // Broadcast that a data directory was added
          FileEvent fe;
          fe.m_EventType = FileEventType::AddDataDirectorySucceeded;
          fe.m_sFileOrDirectory = sPath;
          fe.m_sOther = sCleanRootName;
          fe.m_pDataDir = pDataDir;
          s_pData->m_Event.Broadcast(fe);
        }

        return NS_SUCCESS;
      }
    }
  }

  {
    // Broadcast that adding a data directory failed
    FileEvent fe;
    fe.m_EventType = FileEventType::AddDataDirectoryFailed;
    fe.m_sFileOrDirectory = sPath;
    fe.m_sOther = sCleanRootName;
    s_pData->m_Event.Broadcast(fe);
  }

  nsLog::Error("Adding Data Directory '{0}' failed.", nsArgSensitive(sDataDirectory, "Path"));
  return NS_FAILURE;
}


bool nsFileSystem::RemoveDataDirectory(nsStringView sRootName)
{
  nsStringBuilder sCleanRootName = sRootName;
  CleanUpRootName(sCleanRootName);

  NS_LOCK(s_pData->m_FsMutex);

  for (nsUInt32 i = 0; i < s_pData->m_DataDirectories.GetCount();)
  {
    const auto& directory = s_pData->m_DataDirectories[i];

    if (directory.m_sRootName == sCleanRootName)
    {
      {
        // Broadcast that a data directory is about to be removed
        FileEvent fe;
        fe.m_EventType = FileEventType::RemoveDataDirectory;
        fe.m_sFileOrDirectory = directory.m_pDataDirectory->GetDataDirectoryPath();
        fe.m_sOther = directory.m_sRootName;
        fe.m_pDataDir = directory.m_pDataDirectory;
        s_pData->m_Event.Broadcast(fe);
      }

      directory.m_pDataDirectory->RemoveDataDirectory();
      s_pData->m_DataDirectories.RemoveAtAndCopy(i);

      return true;
    }
    else
      ++i;
  }

  return false;
}

nsUInt32 nsFileSystem::RemoveDataDirectoryGroup(nsStringView sGroup)
{
  if (s_pData == nullptr)
    return 0;

  NS_LOCK(s_pData->m_FsMutex);

  nsUInt32 uiRemoved = 0;

  for (nsUInt32 i = 0; i < s_pData->m_DataDirectories.GetCount();)
  {
    if (s_pData->m_DataDirectories[i].m_sGroup == sGroup)
    {
      {
        // Broadcast that a data directory is about to be removed
        FileEvent fe;
        fe.m_EventType = FileEventType::RemoveDataDirectory;
        fe.m_sFileOrDirectory = s_pData->m_DataDirectories[i].m_pDataDirectory->GetDataDirectoryPath();
        fe.m_sOther = s_pData->m_DataDirectories[i].m_sRootName;
        fe.m_pDataDir = s_pData->m_DataDirectories[i].m_pDataDirectory;
        s_pData->m_Event.Broadcast(fe);
      }

      ++uiRemoved;

      s_pData->m_DataDirectories[i].m_pDataDirectory->RemoveDataDirectory();
      s_pData->m_DataDirectories.RemoveAtAndCopy(i);
    }
    else
      ++i;
  }

  return uiRemoved;
}

void nsFileSystem::ClearAllDataDirectories()
{
  NS_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  NS_LOCK(s_pData->m_FsMutex);

  for (nsInt32 i = s_pData->m_DataDirectories.GetCount() - 1; i >= 0; --i)
  {
    {
      // Broadcast that a data directory is about to be removed
      FileEvent fe;
      fe.m_EventType = FileEventType::RemoveDataDirectory;
      fe.m_sFileOrDirectory = s_pData->m_DataDirectories[i].m_pDataDirectory->GetDataDirectoryPath();
      fe.m_sOther = s_pData->m_DataDirectories[i].m_sRootName;
      fe.m_pDataDir = s_pData->m_DataDirectories[i].m_pDataDirectory;
      s_pData->m_Event.Broadcast(fe);
    }

    s_pData->m_DataDirectories[i].m_pDataDirectory->RemoveDataDirectory();
  }

  s_pData->m_DataDirectories.Clear();
}

const nsFileSystem::DataDirectoryInfo* nsFileSystem::FindDataDirectoryWithRoot(nsStringView sRootName)
{
  if (sRootName.IsEmpty())
    return nullptr;

  NS_LOCK(s_pData->m_FsMutex);

  for (const auto& dd : s_pData->m_DataDirectories)
  {
    if (dd.m_sRootName.IsEqual_NoCase(sRootName))
    {
      return &dd;
    }
  }

  return nullptr;
}

nsUInt32 nsFileSystem::GetNumDataDirectories()
{
  NS_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  return s_pData->m_DataDirectories.GetCount();
}

nsDataDirectoryType* nsFileSystem::GetDataDirectory(nsUInt32 uiDataDirIndex)
{
  NS_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  return s_pData->m_DataDirectories[uiDataDirIndex].m_pDataDirectory;
}

const nsFileSystem::DataDirectoryInfo& nsFileSystem::GetDataDirectoryInfo(nsUInt32 uiDataDirIndex)
{
  NS_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  return s_pData->m_DataDirectories[uiDataDirIndex];
}

nsStringView nsFileSystem::GetDataDirRelativePath(nsStringView sPath, nsUInt32 uiDataDir)
{
  NS_LOCK(s_pData->m_FsMutex);

  // if an absolute path is given, this will check whether the absolute path would fall into this data directory
  // if yes, the prefix path is removed and then only the relative path is given to the data directory type
  // otherwise the data directory would prepend its own path and thus create an invalid path to work with

  // first check the redirected directory
  const nsString128& sRedDirPath = s_pData->m_DataDirectories[uiDataDir].m_pDataDirectory->GetRedirectedDataDirectoryPath();

  if (!sRedDirPath.IsEmpty() && sPath.StartsWith_NoCase(sRedDirPath))
  {
    nsStringView sRelPath(sPath.GetStartPointer() + sRedDirPath.GetElementCount(), sPath.GetEndPointer());

    // if the relative path still starts with a path-separator, skip it
    if (nsPathUtils::IsPathSeparator(sRelPath.GetCharacter()))
    {
      sRelPath.ChopAwayFirstCharacterUtf8();
    }

    return sRelPath;
  }

  // then check the original mount path
  const nsString128& sDirPath = s_pData->m_DataDirectories[uiDataDir].m_pDataDirectory->GetDataDirectoryPath();

  // If the data dir is empty we return the paths as is or the code below would remove the '/' in front of an
  // absolute path.
  if (!sDirPath.IsEmpty() && sPath.StartsWith_NoCase(sDirPath))
  {
    nsStringView sRelPath(sPath.GetStartPointer() + sDirPath.GetElementCount(), sPath.GetEndPointer());

    // if the relative path still starts with a path-separator, skip it
    if (nsPathUtils::IsPathSeparator(sRelPath.GetCharacter()))
    {
      sRelPath.ChopAwayFirstCharacterUtf8();
    }

    return sRelPath;
  }

  return sPath;
}


nsFileSystem::DataDirectoryInfo* nsFileSystem::GetDataDirForRoot(const nsString& sRoot)
{
  NS_LOCK(s_pData->m_FsMutex);

  for (nsInt32 i = (nsInt32)s_pData->m_DataDirectories.GetCount() - 1; i >= 0; --i)
  {
    if (s_pData->m_DataDirectories[i].m_sRootName == sRoot)
      return &s_pData->m_DataDirectories[i];
  }

  return nullptr;
}


void nsFileSystem::DeleteFile(nsStringView sFile)
{
  NS_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  if (nsPathUtils::IsAbsolutePath(sFile))
  {
    nsOSFile::DeleteFile(sFile).IgnoreResult();
    return;
  }

  nsString sRootName;
  sFile = ExtractRootName(sFile, sRootName);

  NS_ASSERT_DEV(!sRootName.IsEmpty(), "Files can only be deleted with a rooted path name.");

  if (sRootName.IsEmpty())
    return;

  NS_LOCK(s_pData->m_FsMutex);

  for (nsInt32 i = (nsInt32)s_pData->m_DataDirectories.GetCount() - 1; i >= 0; --i)
  {
    // do not delete data from directories that are mounted as read only
    if (s_pData->m_DataDirectories[i].m_Usage != AllowWrites)
      continue;

    if (s_pData->m_DataDirectories[i].m_sRootName != sRootName)
      continue;

    nsStringView sRelPath = GetDataDirRelativePath(sFile, i);

    {
      // Broadcast that a file is about to be deleted
      // This can be used to check out files or mark them as deleted in a revision control system
      FileEvent fe;
      fe.m_EventType = FileEventType::DeleteFile;
      fe.m_sFileOrDirectory = sRelPath;
      fe.m_pDataDir = s_pData->m_DataDirectories[i].m_pDataDirectory;
      fe.m_sOther = sRootName;
      s_pData->m_Event.Broadcast(fe);
    }

    s_pData->m_DataDirectories[i].m_pDataDirectory->DeleteFile(sRelPath);
  }
}

bool nsFileSystem::ExistsFile(nsStringView sFile)
{
  NS_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  nsString sRootName;
  sFile = ExtractRootName(sFile, sRootName);

  const bool bOneSpecificDataDir = !sRootName.IsEmpty();

  NS_LOCK(s_pData->m_FsMutex);

  for (nsInt32 i = (nsInt32)s_pData->m_DataDirectories.GetCount() - 1; i >= 0; --i)
  {
    if (!sRootName.IsEmpty() && s_pData->m_DataDirectories[i].m_sRootName != sRootName)
      continue;

    nsStringView sRelPath = GetDataDirRelativePath(sFile, i);

    if (s_pData->m_DataDirectories[i].m_pDataDirectory->ExistsFile(sRelPath, bOneSpecificDataDir))
      return true;
  }

  return false;
}


nsResult nsFileSystem::GetFileStats(nsStringView sFileOrFolder, nsFileStats& out_stats)
{
  NS_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  NS_LOCK(s_pData->m_FsMutex);

  nsString sRootName;
  sFileOrFolder = ExtractRootName(sFileOrFolder, sRootName);

  const bool bOneSpecificDataDir = !sRootName.IsEmpty();

  for (nsInt32 i = (nsInt32)s_pData->m_DataDirectories.GetCount() - 1; i >= 0; --i)
  {
    if (!sRootName.IsEmpty() && s_pData->m_DataDirectories[i].m_sRootName != sRootName)
      continue;

    nsStringView sRelPath = GetDataDirRelativePath(sFileOrFolder, i);

    if (s_pData->m_DataDirectories[i].m_pDataDirectory->GetFileStats(sRelPath, bOneSpecificDataDir, out_stats).Succeeded())
      return NS_SUCCESS;
  }

  return NS_FAILURE;
}

nsStringView nsFileSystem::ExtractRootName(nsStringView sPath, nsString& rootName)
{
  nsStringView root, path;
  nsPathUtils::GetRootedPathParts(sPath, root, path);

  nsStringBuilder rootUpr = root;
  rootUpr.ToUpper();
  rootName = rootUpr;
  return path;
}

nsDataDirectoryReader* nsFileSystem::GetFileReader(nsStringView sFile, nsFileShareMode::Enum FileShareMode, bool bAllowFileEvents)
{
  NS_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  if (sFile.IsEmpty())
    return nullptr;

  NS_LOCK(s_pData->m_FsMutex);

  nsString sRootName;
  sFile = ExtractRootName(sFile, sRootName);

  // clean up the path to get rid of ".." etc.
  nsStringBuilder sPath = sFile;
  sPath.MakeCleanPath();

  const bool bOneSpecificDataDir = !sRootName.IsEmpty();

  // the last added data directory has the highest priority
  for (nsInt32 i = (nsInt32)s_pData->m_DataDirectories.GetCount() - 1; i >= 0; --i)
  {
    // if a root is used, ignore all directories that do not have the same root name
    if (bOneSpecificDataDir && s_pData->m_DataDirectories[i].m_sRootName != sRootName)
      continue;

    nsStringView sRelPath = GetDataDirRelativePath(sPath, i);

    if (bAllowFileEvents)
    {
      // Broadcast that we now try to open this file
      // Could be useful to check this file out before it is accessed
      FileEvent fe;
      fe.m_EventType = FileEventType::OpenFileAttempt;
      fe.m_sFileOrDirectory = sRelPath;
      fe.m_sOther = sRootName;
      fe.m_pDataDir = s_pData->m_DataDirectories[i].m_pDataDirectory;
      s_pData->m_Event.Broadcast(fe);
    }

    // Let the data directory try to open the file.
    nsDataDirectoryReader* pReader = s_pData->m_DataDirectories[i].m_pDataDirectory->OpenFileToRead(sRelPath, FileShareMode, bOneSpecificDataDir);

    if (bAllowFileEvents && pReader != nullptr)
    {
      // Broadcast that this file has been opened.
      FileEvent fe;
      fe.m_EventType = FileEventType::OpenFileSucceeded;
      fe.m_sFileOrDirectory = sRelPath;
      fe.m_sOther = sRootName;
      fe.m_pDataDir = s_pData->m_DataDirectories[i].m_pDataDirectory;
      s_pData->m_Event.Broadcast(fe);

      return pReader;
    }
  }

  if (bAllowFileEvents)
  {
    // Broadcast that opening this file failed.
    FileEvent fe;
    fe.m_EventType = FileEventType::OpenFileFailed;
    fe.m_sFileOrDirectory = sPath;
    s_pData->m_Event.Broadcast(fe);
  }

  return nullptr;
}

nsDataDirectoryWriter* nsFileSystem::GetFileWriter(nsStringView sFile, nsFileShareMode::Enum FileShareMode, bool bAllowFileEvents)
{
  NS_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  if (sFile.IsEmpty())
    return nullptr;

  NS_LOCK(s_pData->m_FsMutex);

  nsString sRootName;

  if (!nsPathUtils::IsAbsolutePath(sFile))
  {
    NS_ASSERT_DEV(sFile.StartsWith(":"),
      "Only native absolute paths or rooted paths (starting with a colon and then the data dir root name) are allowed for "
      "writing to files. This path is neither: '{0}'",
      sFile);
    sFile = ExtractRootName(sFile, sRootName);
  }

  // clean up the path to get rid of ".." etc.
  nsStringBuilder sPath = sFile;
  sPath.MakeCleanPath();

  // the last added data directory has the highest priority
  for (nsInt32 i = (nsInt32)s_pData->m_DataDirectories.GetCount() - 1; i >= 0; --i)
  {
    if (s_pData->m_DataDirectories[i].m_Usage != AllowWrites)
      continue;

    // ignore all directories that have not the category that is currently requested
    if (s_pData->m_DataDirectories[i].m_sRootName != sRootName)
      continue;

    nsStringView sRelPath = GetDataDirRelativePath(sPath, i);

    if (bAllowFileEvents)
    {
      // Broadcast that we now try to open this file
      // Could be useful to check this file out before it is accessed
      FileEvent fe;
      fe.m_EventType = FileEventType::CreateFileAttempt;
      fe.m_sFileOrDirectory = sRelPath;
      fe.m_sOther = sRootName;
      fe.m_pDataDir = s_pData->m_DataDirectories[i].m_pDataDirectory;
      s_pData->m_Event.Broadcast(fe);
    }

    nsDataDirectoryWriter* pWriter = s_pData->m_DataDirectories[i].m_pDataDirectory->OpenFileToWrite(sRelPath, FileShareMode);

    if (bAllowFileEvents && pWriter != nullptr)
    {
      // Broadcast that this file has been created.
      FileEvent fe;
      fe.m_EventType = FileEventType::CreateFileSucceeded;
      fe.m_sFileOrDirectory = sRelPath;
      fe.m_sOther = sRootName;
      fe.m_pDataDir = s_pData->m_DataDirectories[i].m_pDataDirectory;
      s_pData->m_Event.Broadcast(fe);

      return pWriter;
    }
  }

  if (bAllowFileEvents)
  {
    // Broadcast that creating this file failed.
    FileEvent fe;
    fe.m_EventType = FileEventType::CreateFileFailed;
    fe.m_sFileOrDirectory = sPath;
    s_pData->m_Event.Broadcast(fe);
  }

  return nullptr;
}

nsResult nsFileSystem::ResolvePath(nsStringView sPath, nsStringBuilder* out_pAbsolutePath, nsStringBuilder* out_pDataDirRelativePath, nsDataDirectoryType** out_pDataDir /*= nullptr*/)
{
  NS_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  NS_LOCK(s_pData->m_FsMutex);

  nsStringBuilder absPath, relPath;

  if (sPath.StartsWith(":"))
  {
    // writing is only allowed using rooted paths
    nsString sRootName;
    ExtractRootName(sPath, sRootName);

    DataDirectoryInfo* pDataDir = GetDataDirForRoot(sRootName);

    if (pDataDir == nullptr)
      return NS_FAILURE;

    if (out_pDataDir != nullptr)
      *out_pDataDir = pDataDir->m_pDataDirectory;

    relPath = sPath.GetShrunk(sRootName.GetCharacterCount() + 2);

    absPath = pDataDir->m_pDataDirectory->GetRedirectedDataDirectoryPath(); /// \todo We might also need the none-redirected path as an output
    absPath.AppendPath(relPath);
  }
  else if (nsPathUtils::IsAbsolutePath(sPath))
  {
    absPath = sPath;
    absPath.MakeCleanPath();

    for (nsUInt32 dd = s_pData->m_DataDirectories.GetCount(); dd > 0; --dd)
    {
      auto& dir = s_pData->m_DataDirectories[dd - 1];

      if (nsPathUtils::IsSubPath(dir.m_pDataDirectory->GetRedirectedDataDirectoryPath(), absPath))
      {
        if (out_pAbsolutePath)
          *out_pAbsolutePath = absPath;

        if (out_pDataDirRelativePath)
        {
          *out_pDataDirRelativePath = absPath;
          out_pDataDirRelativePath->MakeRelativeTo(dir.m_pDataDirectory->GetRedirectedDataDirectoryPath()).IgnoreResult();
        }

        if (out_pDataDir)
          *out_pDataDir = dir.m_pDataDirectory;

        return NS_SUCCESS;
      }
    }

    return NS_FAILURE;
  }
  else
  {
    // try to get a reader -> if we get one, the file does indeed exist
    nsDataDirectoryReader* pReader = nsFileSystem::GetFileReader(sPath, nsFileShareMode::SharedReads, true);

    if (!pReader)
      return NS_FAILURE;

    if (out_pDataDir != nullptr)
      *out_pDataDir = pReader->GetDataDirectory();

    relPath = pReader->GetFilePath();

    absPath = pReader->GetDataDirectory()->GetRedirectedDataDirectoryPath(); /// \todo We might also need the none-redirected path as an output
    absPath.AppendPath(relPath);

    pReader->Close();
  }

  if (out_pAbsolutePath)
    *out_pAbsolutePath = absPath;

  if (out_pDataDirRelativePath)
    *out_pDataDirRelativePath = relPath;

  return NS_SUCCESS;
}

nsResult nsFileSystem::FindFolderWithSubPath(nsStringBuilder& out_sResult, nsStringView sStartDirectory, nsStringView sSubPath, nsStringView sRedirectionFileName /*= nullptr*/)
{
  nsStringBuilder sStartDirAbs = sStartDirectory;
  sStartDirAbs.MakeCleanPath();

  // in this case the given path and the absolute path are different
  // but we want to return the same path format as is given
  // ie. if we get ":MyRoot\Bla" with "MyRoot" pointing to "C:\Game", then the result should be
  // ":MyRoot\blub", rather than "C:\Game\blub"
  if (sStartDirAbs.StartsWith(":"))
  {
    nsStringBuilder abs;
    if (ResolvePath(sStartDirAbs, &abs, nullptr).Failed())
    {
      out_sResult.Clear();
      return NS_FAILURE;
    }

    sStartDirAbs = abs;
  }

  out_sResult = sStartDirectory;
  out_sResult.MakeCleanPath();

  nsStringBuilder FullPath, sRedirection;

  while (!out_sResult.IsEmpty())
  {
    sRedirection.Clear();

    if (!sRedirectionFileName.IsEmpty())
    {
      FullPath = sStartDirAbs;
      FullPath.AppendPath(sRedirectionFileName);

      nsOSFile f;
      if (f.Open(FullPath, nsFileOpenMode::Read).Succeeded())
      {
        nsDataBuffer db;
        f.ReadAll(db);
        sRedirection.Set(nsStringView((const char*)db.GetData(), db.GetCount()));
      }
    }

    // first try with the redirection
    if (!sRedirection.IsEmpty())
    {
      FullPath = sStartDirAbs;
      FullPath.AppendPath(sRedirection);
      FullPath.AppendPath(sSubPath);
      FullPath.MakeCleanPath();

      if (nsOSFile::ExistsDirectory(FullPath) || nsOSFile::ExistsFile(FullPath))
      {
        out_sResult.AppendPath(sRedirection);
        out_sResult.MakeCleanPath();
        return NS_SUCCESS;
      }
    }

    // then try without the redirection
    FullPath = sStartDirAbs;
    FullPath.AppendPath(sSubPath);
    FullPath.MakeCleanPath();

    if (nsOSFile::ExistsDirectory(FullPath) || nsOSFile::ExistsFile(FullPath))
    {
      return NS_SUCCESS;
    }

    out_sResult.PathParentDirectory();
    sStartDirAbs.PathParentDirectory();
  }

  return NS_FAILURE;
}

bool nsFileSystem::ResolveAssetRedirection(nsStringView sPathOrAssetGuid, nsStringBuilder& out_sRedirection)
{
  NS_LOCK(s_pData->m_FsMutex);

  for (auto& dd : s_pData->m_DataDirectories)
  {
    if (dd.m_pDataDirectory->ResolveAssetRedirection(sPathOrAssetGuid, out_sRedirection))
      return true;
  }

  out_sRedirection = sPathOrAssetGuid;
  return false;
}

nsStringView nsFileSystem::MigrateFileLocation(nsStringView sOldLocation, nsStringView sNewLocation)
{
  nsStringBuilder sOldPathFull, sNewPathFull;

  if (ResolvePath(sOldLocation, &sOldPathFull, nullptr).Failed() || sOldPathFull.IsEmpty())
  {
    // if the old path could not be resolved, use the new path
    return sNewLocation;
  }

  ResolvePath(sNewLocation, &sNewPathFull, nullptr).AssertSuccess();

  if (!ExistsFile(sOldPathFull))
  {
    // old path doesn't exist -> use the new
    return sNewLocation;
  }

  // old path does exist -> deal with it

  if (ExistsFile(sNewPathFull))
  {
    // new path also exists -> delete the old one (in all data directories), use the new one
    DeleteFile(sOldLocation); // location, not full path
    return sNewLocation;
  }

  // new one doesn't exist -> try to move old to new
  if (nsOSFile::MoveFileOrDirectory(sOldPathFull, sNewPathFull).Failed())
  {
    // if the old location exists, but we can't move the file, return the old location to use
    return sOldLocation;
  }

  // deletes the file in the old location in ALL data directories,
  // so that they can't interfere with the new file in the future
  DeleteFile(sOldLocation); // location, not full path

  // if we successfully moved the file to the new location, use the new location
  return sNewLocation;
}

void nsFileSystem::ReloadAllExternalDataDirectoryConfigs()
{
  NS_LOG_BLOCK("ReloadAllExternalDataDirectoryConfigs");

  NS_LOCK(s_pData->m_FsMutex);

  for (auto& dd : s_pData->m_DataDirectories)
  {
    dd.m_pDataDirectory->ReloadExternalConfigs();
  }
}

void nsFileSystem::Startup()
{
  s_pData = NS_DEFAULT_NEW(FileSystemData);
}

void nsFileSystem::Shutdown()
{
  {
    NS_LOCK(s_pData->m_FsMutex);

    s_pData->m_DataDirFactories.Clear();

    ClearAllDataDirectories();
  }

  NS_DEFAULT_DELETE(s_pData);
}

nsResult nsFileSystem::DetectSdkRootDirectory(nsStringView sExpectedSubFolder /*= "Data/Base"*/)
{
  if (!s_sSdkRootDir.IsEmpty())
    return NS_SUCCESS;

  nsStringBuilder sdkRoot;

#if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
  // Probably this is what needs to be done on all mobile platforms as well
  sdkRoot = nsOSFile::GetApplicationDirectory();
#elif NS_ENABLED(NS_PLATFORM_ANDROID)
  sdkRoot = nsOSFile::GetApplicationDirectory();
#else
  if (nsFileSystem::FindFolderWithSubPath(sdkRoot, nsOSFile::GetApplicationDirectory(), sExpectedSubFolder, "nsSdkRoot.txt").Failed())
  {
    nsLog::Error("Could not find SDK root. Application dir is '{0}'. Searched for parent with '{1}' sub-folder.", nsOSFile::GetApplicationDirectory(), sExpectedSubFolder);
    return NS_FAILURE;
  }
#endif

  nsFileSystem::SetSdkRootDirectory(sdkRoot);
  return NS_SUCCESS;
}

void nsFileSystem::SetSdkRootDirectory(nsStringView sSdkDir)
{
  nsStringBuilder s = sSdkDir;
  s.MakeCleanPath();

  s_sSdkRootDir = s;
}

nsStringView nsFileSystem::GetSdkRootDirectory()
{
  NS_ASSERT_DEV(!s_sSdkRootDir.IsEmpty(), "The project directory has not been set through 'nsFileSystem::SetSdkRootDirectory'.");
  return s_sSdkRootDir;
}

void nsFileSystem::SetSpecialDirectory(nsStringView sName, nsStringView sReplacement)
{
  nsStringBuilder tmp = sName;
  tmp.ToLower();

  if (sReplacement.IsEmpty())
  {
    s_SpecialDirectories.Remove(tmp);
  }
  else
  {
    s_SpecialDirectories[tmp] = sReplacement;
  }
}

nsResult nsFileSystem::ResolveSpecialDirectory(nsStringView sDirectory, nsStringBuilder& out_sPath)
{
  if (sDirectory.IsEmpty() || !sDirectory.StartsWith(">"))
  {
    out_sPath = sDirectory;
    return NS_SUCCESS;
  }

  // skip the '>'
  sDirectory.ChopAwayFirstCharacterAscii();
  const char* szStart = sDirectory.GetStartPointer();

  const char* szEnd = sDirectory.FindSubString("/");

  if (szEnd == nullptr)
    szEnd = szStart + nsStringUtils::GetStringElementCount(szStart);

  nsStringBuilder sName;
  sName.SetSubString_FromTo(szStart, szEnd);
  sName.ToLower();

  const auto it = s_SpecialDirectories.Find(sName);
  if (it.IsValid())
  {
    out_sPath = it.Value();
    out_sPath.AppendPath(szEnd); // szEnd might be on \0 or a slash
    out_sPath.MakeCleanPath();
    return NS_SUCCESS;
  }

  if (sName == "sdk")
  {
    sDirectory.Shrink(3, 0);
    out_sPath = GetSdkRootDirectory();
    out_sPath.AppendPath(sDirectory);
    out_sPath.MakeCleanPath();
    return NS_SUCCESS;
  }

  if (sName == "user")
  {
    sDirectory.Shrink(4, 0);
    out_sPath = nsOSFile::GetUserDataFolder();
    out_sPath.AppendPath(sDirectory);
    out_sPath.MakeCleanPath();
    return NS_SUCCESS;
  }

  if (sName == "temp")
  {
    sDirectory.Shrink(4, 0);
    out_sPath = nsOSFile::GetTempDataFolder();
    out_sPath.AppendPath(sDirectory);
    out_sPath.MakeCleanPath();
    return NS_SUCCESS;
  }

  if (sName == "appdir")
  {
    sDirectory.Shrink(6, 0);
    out_sPath = nsOSFile::GetApplicationDirectory();
    out_sPath.AppendPath(sDirectory);
    out_sPath.MakeCleanPath();
    return NS_SUCCESS;
  }

  return NS_FAILURE;
}


nsMutex& nsFileSystem::GetMutex()
{
  NS_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");
  return s_pData->m_FsMutex;
}

#if NS_ENABLED(NS_SUPPORTS_FILE_ITERATORS)

void nsFileSystem::StartSearch(nsFileSystemIterator& ref_iterator, nsStringView sSearchTerm, nsBitflags<nsFileSystemIteratorFlags> flags /*= nsFileSystemIteratorFlags::Default*/)
{
  NS_LOCK(s_pData->m_FsMutex);

  nsHybridArray<nsString, 16> folders;
  nsStringBuilder sDdPath, sRelPath;

  if (sSearchTerm.IsRootedPath())
  {
    const nsStringView root = sSearchTerm.GetRootedPathRootName();

    const DataDirectoryInfo* pDataDir = FindDataDirectoryWithRoot(root);
    if (pDataDir == nullptr)
      return;

    sSearchTerm.SetStartPosition(root.GetEndPointer());

    if (!sSearchTerm.IsEmpty())
    {
      // root name should be followed by a slash
      sSearchTerm.ChopAwayFirstCharacterAscii();
    }

    folders.PushBack(pDataDir->m_pDataDirectory->GetRedirectedDataDirectoryPath().GetView());
  }
  else if (sSearchTerm.IsAbsolutePath())
  {
    for (nsUInt32 idx = s_pData->m_DataDirectories.GetCount(); idx > 0; --idx)
    {
      const auto& dd = s_pData->m_DataDirectories[idx - 1];

      sDdPath = dd.m_pDataDirectory->GetRedirectedDataDirectoryPath();

      sRelPath = sSearchTerm;

      if (!sDdPath.IsEmpty())
      {
        if (sRelPath.MakeRelativeTo(sDdPath).Failed())
          continue;

        // this would use "../" if necessary, which we don't want
        if (sRelPath.StartsWith(".."))
          continue;
      }

      sSearchTerm = sRelPath;

      folders.PushBack(sDdPath);
      break;
    }
  }
  else
  {
    for (nsUInt32 idx = s_pData->m_DataDirectories.GetCount(); idx > 0; --idx)
    {
      const auto& dd = s_pData->m_DataDirectories[idx - 1];

      sDdPath = dd.m_pDataDirectory->GetRedirectedDataDirectoryPath();

      folders.PushBack(sDdPath);
    }
  }

  ref_iterator.StartMultiFolderSearch(folders, sSearchTerm, flags);
}

#endif

nsResult nsFileSystem::CreateDirectoryStructure(nsStringView sPath)
{
  nsStringBuilder sRedir;
  NS_SUCCEED_OR_RETURN(ResolveSpecialDirectory(sPath, sRedir));

  if (sRedir.IsRootedPath())
  {
    nsFileSystem::ResolvePath(sRedir, &sRedir, nullptr).AssertSuccess();
  }

  return nsOSFile::CreateDirectoryStructure(sRedir);
}

NS_STATICLINK_FILE(Foundation, Foundation_IO_FileSystem_Implementation_FileSystem);

#include <Foundation/FoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/Implementation/StringIterator.h>
#include <Foundation/Strings/StringView.h>

// clang-format off
WD_BEGIN_SUBSYSTEM_DECLARATION(Foundation, FileSystem)

  ON_CORESYSTEMS_STARTUP
  {
    wdFileSystem::Startup();
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    wdFileSystem::Shutdown();
  }

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

wdFileSystem::FileSystemData* wdFileSystem::s_pData = nullptr;
wdString wdFileSystem::s_sSdkRootDir;
wdMap<wdString, wdString> wdFileSystem::s_SpecialDirectories;


void wdFileSystem::RegisterDataDirectoryFactory(wdDataDirFactory factory, float fPriority /*= 0*/)
{
  WD_LOCK(s_pData->m_FsMutex);

  auto& data = s_pData->m_DataDirFactories.ExpandAndGetRef();
  data.m_Factory = factory;
  data.m_fPriority = fPriority;
}

wdEventSubscriptionID wdFileSystem::RegisterEventHandler(wdEvent<const FileEvent&>::Handler handler)
{
  WD_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  return s_pData->m_Event.AddEventHandler(handler);
}

void wdFileSystem::UnregisterEventHandler(wdEvent<const FileEvent&>::Handler handler)
{
  WD_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  s_pData->m_Event.RemoveEventHandler(handler);
}

void wdFileSystem::UnregisterEventHandler(wdEventSubscriptionID subscriptionId)
{
  WD_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  s_pData->m_Event.RemoveEventHandler(subscriptionId);
}

void wdFileSystem::CleanUpRootName(wdStringBuilder& sRoot)
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

wdResult wdFileSystem::AddDataDirectory(wdStringView sDataDirectory, wdStringView sGroup, wdStringView sRootName, DataDirUsage usage)
{
  WD_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");
  WD_ASSERT_DEV(usage != AllowWrites || !sRootName.IsEmpty(), "A data directory must have a non-empty, unique name to be mounted for write access");

  wdStringBuilder sPath = sDataDirectory;
  sPath.MakeCleanPath();

  if (!sPath.IsEmpty() && !sPath.EndsWith("/"))
    sPath.Append("/");

  wdStringBuilder sCleanRootName = sRootName;
  CleanUpRootName(sCleanRootName);

  WD_LOCK(s_pData->m_FsMutex);

  bool failed = false;
  if (FindDataDirectoryWithRoot(sCleanRootName) != nullptr)
  {
    wdLog::Error("A data directory with root name '{0}' already exists.", sCleanRootName);
    failed = true;
  }

  if (!failed)
  {
    s_pData->m_DataDirFactories.Sort([](const auto& a, const auto& b) { return a.m_fPriority < b.m_fPriority; });

    // use the factory that was added last as the one with the highest priority -> allows to override already added factories
    for (wdInt32 i = s_pData->m_DataDirFactories.GetCount() - 1; i >= 0; --i)
    {
      wdDataDirectoryType* pDataDir = s_pData->m_DataDirFactories[i].m_Factory(sPath, sGroup, sRootName, usage);

      if (pDataDir != nullptr)
      {
        DataDirectory dd;
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

        return WD_SUCCESS;
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

  wdLog::Error("Adding Data Directory '{0}' failed.", wdArgSensitive(sDataDirectory, "Path"));
  return WD_FAILURE;
}


bool wdFileSystem::RemoveDataDirectory(wdStringView sRootName)
{
  wdStringBuilder sCleanRootName = sRootName;
  CleanUpRootName(sCleanRootName);

  WD_LOCK(s_pData->m_FsMutex);

  for (wdUInt32 i = 0; i < s_pData->m_DataDirectories.GetCount();)
  {
    if (s_pData->m_DataDirectories[i].m_sRootName == sCleanRootName)
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
      s_pData->m_DataDirectories.RemoveAtAndCopy(i);

      return true;
    }
    else
      ++i;
  }

  return false;
}

wdUInt32 wdFileSystem::RemoveDataDirectoryGroup(wdStringView sGroup)
{
  if (s_pData == nullptr)
    return 0;

  WD_LOCK(s_pData->m_FsMutex);

  wdUInt32 uiRemoved = 0;

  for (wdUInt32 i = 0; i < s_pData->m_DataDirectories.GetCount();)
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

void wdFileSystem::ClearAllDataDirectories()
{
  WD_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  WD_LOCK(s_pData->m_FsMutex);

  for (wdInt32 i = s_pData->m_DataDirectories.GetCount() - 1; i >= 0; --i)
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

wdDataDirectoryType* wdFileSystem::FindDataDirectoryWithRoot(wdStringView sRootName)
{
  if (sRootName.IsEmpty())
    return nullptr;

  WD_LOCK(s_pData->m_FsMutex);

  for (const auto& dd : s_pData->m_DataDirectories)
  {
    if (dd.m_sRootName.IsEqual_NoCase(sRootName))
    {
      return dd.m_pDataDirectory;
    }
  }

  return nullptr;
}

wdUInt32 wdFileSystem::GetNumDataDirectories()
{
  WD_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  return s_pData->m_DataDirectories.GetCount();
}

wdDataDirectoryType* wdFileSystem::GetDataDirectory(wdUInt32 uiDataDirIndex)
{
  WD_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  return s_pData->m_DataDirectories[uiDataDirIndex].m_pDataDirectory;
}

wdStringView wdFileSystem::GetDataDirRelativePath(wdStringView sPath, wdUInt32 uiDataDir)
{
  WD_LOCK(s_pData->m_FsMutex);

  // if an absolute path is given, this will check whether the absolute path would fall into this data directory
  // if yes, the prefix path is removed and then only the relative path is given to the data directory type
  // otherwise the data directory would prepend its own path and thus create an invalid path to work with

  // first check the redirected directory
  const wdString128& sRedDirPath = s_pData->m_DataDirectories[uiDataDir].m_pDataDirectory->GetRedirectedDataDirectoryPath();

  if (!sRedDirPath.IsEmpty() && sPath.StartsWith_NoCase(sRedDirPath))
  {
    wdStringView sRelPath(sPath.GetStartPointer() + sRedDirPath.GetElementCount(), sPath.GetEndPointer());

    // if the relative path still starts with a path-separator, skip it
    if (wdPathUtils::IsPathSeparator(sRelPath.GetCharacter()))
    {
      sRelPath.ChopAwayFirstCharacterUtf8();
    }

    return sRelPath;
  }

  // then check the original mount path
  const wdString128& sDirPath = s_pData->m_DataDirectories[uiDataDir].m_pDataDirectory->GetDataDirectoryPath();

  // If the data dir is empty we return the paths as is or the code below would remove the '/' in front of an
  // absolute path.
  if (!sDirPath.IsEmpty() && sPath.StartsWith_NoCase(sDirPath))
  {
    wdStringView sRelPath(sPath.GetStartPointer() + sDirPath.GetElementCount(), sPath.GetEndPointer());

    // if the relative path still starts with a path-separator, skip it
    if (wdPathUtils::IsPathSeparator(sRelPath.GetCharacter()))
    {
      sRelPath.ChopAwayFirstCharacterUtf8();
    }

    return sRelPath;
  }

  return sPath;
}


wdFileSystem::DataDirectory* wdFileSystem::GetDataDirForRoot(const wdString& sRoot)
{
  WD_LOCK(s_pData->m_FsMutex);

  for (wdInt32 i = (wdInt32)s_pData->m_DataDirectories.GetCount() - 1; i >= 0; --i)
  {
    if (s_pData->m_DataDirectories[i].m_sRootName == sRoot)
      return &s_pData->m_DataDirectories[i];
  }

  return nullptr;
}


void wdFileSystem::DeleteFile(wdStringView sFile)
{
  WD_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  if (wdPathUtils::IsAbsolutePath(sFile))
  {
    wdOSFile::DeleteFile(sFile).IgnoreResult();
    return;
  }

  wdString sRootName;
  sFile = ExtractRootName(sFile, sRootName);

  WD_ASSERT_DEV(!sRootName.IsEmpty(), "Files can only be deleted with a rooted path name.");

  if (sRootName.IsEmpty())
    return;

  WD_LOCK(s_pData->m_FsMutex);

  for (wdInt32 i = (wdInt32)s_pData->m_DataDirectories.GetCount() - 1; i >= 0; --i)
  {
    // do not delete data from directories that are mounted as read only
    if (s_pData->m_DataDirectories[i].m_Usage != AllowWrites)
      continue;

    if (s_pData->m_DataDirectories[i].m_sRootName != sRootName)
      continue;

    wdStringView sRelPath = GetDataDirRelativePath(sFile, i);

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

bool wdFileSystem::ExistsFile(wdStringView sFile)
{
  WD_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  wdString sRootName;
  sFile = ExtractRootName(sFile, sRootName);

  const bool bOneSpecificDataDir = !sRootName.IsEmpty();

  WD_LOCK(s_pData->m_FsMutex);

  for (wdInt32 i = (wdInt32)s_pData->m_DataDirectories.GetCount() - 1; i >= 0; --i)
  {
    if (!sRootName.IsEmpty() && s_pData->m_DataDirectories[i].m_sRootName != sRootName)
      continue;

    wdStringView sRelPath = GetDataDirRelativePath(sFile, i);

    if (s_pData->m_DataDirectories[i].m_pDataDirectory->ExistsFile(sRelPath, bOneSpecificDataDir))
      return true;
  }

  return false;
}


wdResult wdFileSystem::GetFileStats(wdStringView sFileOrFolder, wdFileStats& out_stats)
{
  WD_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  WD_LOCK(s_pData->m_FsMutex);

  wdString sRootName;
  sFileOrFolder = ExtractRootName(sFileOrFolder, sRootName);

  const bool bOneSpecificDataDir = !sRootName.IsEmpty();

  for (wdInt32 i = (wdInt32)s_pData->m_DataDirectories.GetCount() - 1; i >= 0; --i)
  {
    if (!sRootName.IsEmpty() && s_pData->m_DataDirectories[i].m_sRootName != sRootName)
      continue;

    wdStringView sRelPath = GetDataDirRelativePath(sFileOrFolder, i);

    if (s_pData->m_DataDirectories[i].m_pDataDirectory->GetFileStats(sRelPath, bOneSpecificDataDir, out_stats).Succeeded())
      return WD_SUCCESS;
  }

  return WD_FAILURE;
}

wdStringView wdFileSystem::ExtractRootName(wdStringView sPath, wdString& rootName)
{
  rootName.Clear();

  if (!sPath.StartsWith(":"))
    return sPath;

  wdStringBuilder sCur;
  const wdStringView view = sPath;
  wdStringIterator it = view.GetIteratorFront();
  ++it;

  while (it.IsValid() && (it.GetCharacter() != '/'))
  {
    sCur.Append(it.GetCharacter());
    ++it;
  }

  WD_ASSERT_DEV(it.IsValid(), "Cannot parse the path \"{0}\". The data-dir root name starts with a ':' but does not end with '/'.", sPath);

  sCur.ToUpper();
  rootName = sCur;
  ++it;

  return it.GetData(); // return the string after the data-dir filter declaration
}

wdDataDirectoryReader* wdFileSystem::GetFileReader(wdStringView sFile, wdFileShareMode::Enum FileShareMode, bool bAllowFileEvents)
{
  WD_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  if (sFile.IsEmpty())
    return nullptr;

  WD_LOCK(s_pData->m_FsMutex);

  wdString sRootName;
  sFile = ExtractRootName(sFile, sRootName);

  // clean up the path to get rid of ".." etc.
  wdStringBuilder sPath = sFile;
  sPath.MakeCleanPath();

  const bool bOneSpecificDataDir = !sRootName.IsEmpty();

  // the last added data directory has the highest priority
  for (wdInt32 i = (wdInt32)s_pData->m_DataDirectories.GetCount() - 1; i >= 0; --i)
  {
    // if a root is used, ignore all directories that do not have the same root name
    if (bOneSpecificDataDir && s_pData->m_DataDirectories[i].m_sRootName != sRootName)
      continue;

    wdStringView sRelPath = GetDataDirRelativePath(sPath, i);

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
    wdDataDirectoryReader* pReader = s_pData->m_DataDirectories[i].m_pDataDirectory->OpenFileToRead(sRelPath, FileShareMode, bOneSpecificDataDir);

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

wdDataDirectoryWriter* wdFileSystem::GetFileWriter(wdStringView sFile, wdFileShareMode::Enum FileShareMode, bool bAllowFileEvents)
{
  WD_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  if (sFile.IsEmpty())
    return nullptr;

  WD_LOCK(s_pData->m_FsMutex);

  wdString sRootName;

  if (!wdPathUtils::IsAbsolutePath(sFile))
  {
    WD_ASSERT_DEV(sFile.StartsWith(":"),
      "Only native absolute paths or rooted paths (starting with a colon and then the data dir root name) are allowed for "
      "writing to files. This path is neither: '{0}'",
      sFile);
    sFile = ExtractRootName(sFile, sRootName);
  }

  // clean up the path to get rid of ".." etc.
  wdStringBuilder sPath = sFile;
  sPath.MakeCleanPath();

  // the last added data directory has the highest priority
  for (wdInt32 i = (wdInt32)s_pData->m_DataDirectories.GetCount() - 1; i >= 0; --i)
  {
    if (s_pData->m_DataDirectories[i].m_Usage != AllowWrites)
      continue;

    // ignore all directories that have not the category that is currently requested
    if (s_pData->m_DataDirectories[i].m_sRootName != sRootName)
      continue;

    wdStringView sRelPath = GetDataDirRelativePath(sPath, i);

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

    wdDataDirectoryWriter* pWriter = s_pData->m_DataDirectories[i].m_pDataDirectory->OpenFileToWrite(sRelPath, FileShareMode);

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

wdResult wdFileSystem::ResolvePath(wdStringView sPath, wdStringBuilder* out_pAbsolutePath, wdStringBuilder* out_pDataDirRelativePath, wdDataDirectoryType** out_pDataDir /*= nullptr*/)
{
  WD_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");

  WD_LOCK(s_pData->m_FsMutex);

  wdStringBuilder absPath, relPath;

  if (sPath.StartsWith(":"))
  {
    // writing is only allowed using rooted paths
    wdString sRootName;
    ExtractRootName(sPath, sRootName);

    DataDirectory* pDataDir = GetDataDirForRoot(sRootName);

    if (pDataDir == nullptr)
      return WD_FAILURE;

    if (out_pDataDir != nullptr)
      *out_pDataDir = pDataDir->m_pDataDirectory;

    relPath = sPath.GetShrunk(sRootName.GetCharacterCount() + 2);

    absPath = pDataDir->m_pDataDirectory->GetRedirectedDataDirectoryPath(); /// \todo We might also need the none-redirected path as an output
    absPath.AppendPath(relPath);
  }
  else if (wdPathUtils::IsAbsolutePath(sPath))
  {
    absPath = sPath;
    absPath.MakeCleanPath();

    for (wdUInt32 dd = s_pData->m_DataDirectories.GetCount(); dd > 0; --dd)
    {
      auto& dir = s_pData->m_DataDirectories[dd - 1];

      if (wdPathUtils::IsSubPath(dir.m_pDataDirectory->GetRedirectedDataDirectoryPath(), absPath))
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

        return WD_SUCCESS;
      }
    }

    return WD_FAILURE;
  }
  else
  {
    // try to get a reader -> if we get one, the file does indeed exist
    wdDataDirectoryReader* pReader = wdFileSystem::GetFileReader(sPath, wdFileShareMode::SharedReads, true);

    if (!pReader)
      return WD_FAILURE;

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

  return WD_SUCCESS;
}

wdResult wdFileSystem::FindFolderWithSubPath(wdStringBuilder& out_sResult, wdStringView sStartDirectory, wdStringView sSubPath, wdStringView sRedirectionFileName /*= nullptr*/)
{
  wdStringBuilder sStartDirAbs = sStartDirectory;
  sStartDirAbs.MakeCleanPath();

  // in this case the given path and the absolute path are different
  // but we want to return the same path format as is given
  // ie. if we get ":MyRoot\Bla" with "MyRoot" pointing to "C:\Game", then the result should be
  // ":MyRoot\blub", rather than "C:\Game\blub"
  if (sStartDirAbs.StartsWith(":"))
  {
    wdStringBuilder abs;
    if (ResolvePath(sStartDirAbs, &abs, nullptr).Failed())
    {
      out_sResult.Clear();
      return WD_FAILURE;
    }

    sStartDirAbs = abs;
  }

  out_sResult = sStartDirectory;
  out_sResult.MakeCleanPath();

  wdStringBuilder FullPath, sRedirection;

  while (!out_sResult.IsEmpty())
  {
    sRedirection.Clear();

    if (!sRedirectionFileName.IsEmpty())
    {
      FullPath = sStartDirAbs;
      FullPath.AppendPath(sRedirectionFileName);

      wdOSFile f;
      if (f.Open(FullPath, wdFileOpenMode::Read).Succeeded())
      {
        wdDataBuffer db;
        f.ReadAll(db);
        sRedirection.Set(wdStringView((const char*)db.GetData(), db.GetCount()));
      }
    }

    // first try with the redirection
    if (!sRedirection.IsEmpty())
    {
      FullPath = sStartDirAbs;
      FullPath.AppendPath(sRedirection);
      FullPath.AppendPath(sSubPath);
      FullPath.MakeCleanPath();

      if (wdOSFile::ExistsDirectory(FullPath) || wdOSFile::ExistsFile(FullPath))
      {
        out_sResult.AppendPath(sRedirection);
        out_sResult.MakeCleanPath();
        return WD_SUCCESS;
      }
    }

    // then try without the redirection
    FullPath = sStartDirAbs;
    FullPath.AppendPath(sSubPath);
    FullPath.MakeCleanPath();

    if (wdOSFile::ExistsDirectory(FullPath) || wdOSFile::ExistsFile(FullPath))
    {
      return WD_SUCCESS;
    }

    out_sResult.PathParentDirectory();
    sStartDirAbs.PathParentDirectory();
  }

  return WD_FAILURE;
}

bool wdFileSystem::ResolveAssetRedirection(wdStringView sPathOrAssetGuid, wdStringBuilder& out_sRedirection)
{
  WD_LOCK(s_pData->m_FsMutex);

  for (auto& dd : s_pData->m_DataDirectories)
  {
    if (dd.m_pDataDirectory->ResolveAssetRedirection(sPathOrAssetGuid, out_sRedirection))
      return true;
  }

  out_sRedirection = sPathOrAssetGuid;
  return false;
}

wdStringView wdFileSystem::MigrateFileLocation(wdStringView sOldLocation, wdStringView sNewLocation)
{
  wdStringBuilder sOldPathFull, sNewPathFull;

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
  if (wdOSFile::MoveFileOrDirectory(sOldPathFull, sNewPathFull).Failed())
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

void wdFileSystem::ReloadAllExternalDataDirectoryConfigs()
{
  WD_LOG_BLOCK("ReloadAllExternalDataDirectoryConfigs");

  WD_LOCK(s_pData->m_FsMutex);

  for (auto& dd : s_pData->m_DataDirectories)
  {
    dd.m_pDataDirectory->ReloadExternalConfigs();
  }
}

void wdFileSystem::Startup()
{
  s_pData = WD_DEFAULT_NEW(FileSystemData);
}

void wdFileSystem::Shutdown()
{
  {
    WD_LOCK(s_pData->m_FsMutex);

    s_pData->m_DataDirFactories.Clear();

    ClearAllDataDirectories();
  }

  WD_DEFAULT_DELETE(s_pData);
}

wdResult wdFileSystem::DetectSdkRootDirectory(wdStringView sExpectedSubFolder /*= "Data/Base"*/)
{
  if (!s_sSdkRootDir.IsEmpty())
    return WD_SUCCESS;

  wdStringBuilder sdkRoot;

#if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)
  // Probably this is what needs to be done on all mobile platforms as well
  sdkRoot = wdOSFile::GetApplicationDirectory();
#elif WD_ENABLED(WD_PLATFORM_ANDROID)
  sdkRoot = wdOSFile::GetApplicationDirectory();
#else
  if (wdFileSystem::FindFolderWithSubPath(sdkRoot, wdOSFile::GetApplicationDirectory(), sExpectedSubFolder, "wdSdkRoot.txt").Failed())
  {
    wdLog::Error("Could not find SDK root. Application dir is '{0}'. Searched for parent with '{1}' sub-folder.", wdOSFile::GetApplicationDirectory(), sExpectedSubFolder);
    return WD_FAILURE;
  }
#endif

  wdFileSystem::SetSdkRootDirectory(sdkRoot);
  return WD_SUCCESS;
}

void wdFileSystem::SetSdkRootDirectory(wdStringView sSdkDir)
{
  wdStringBuilder s = sSdkDir;
  s.MakeCleanPath();

  s_sSdkRootDir = s;
}

const char* wdFileSystem::GetSdkRootDirectory()
{
  WD_ASSERT_DEV(!s_sSdkRootDir.IsEmpty(), "The project directory has not been set through 'wdFileSystem::SetSdkRootDirectory'.");
  return s_sSdkRootDir.GetData();
}

void wdFileSystem::SetSpecialDirectory(wdStringView sName, wdStringView sReplacement)
{
  wdStringBuilder tmp = sName;
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

wdResult wdFileSystem::ResolveSpecialDirectory(wdStringView sDirectory, wdStringBuilder& out_sPath)
{
  if (sDirectory.IsEmpty() || !sDirectory.StartsWith(">"))
  {
    out_sPath = sDirectory;
    return WD_SUCCESS;
  }

  // skip the '>'
  sDirectory.ChopAwayFirstCharacterAscii();
  const char* szStart = sDirectory.GetStartPointer();

  const char* szEnd = sDirectory.FindSubString("/");

  if (szEnd == nullptr)
    szEnd = szStart + wdStringUtils::GetStringElementCount(szStart);

  wdStringBuilder sName;
  sName.SetSubString_FromTo(szStart, szEnd);
  sName.ToLower();

  const auto it = s_SpecialDirectories.Find(sName);
  if (it.IsValid())
  {
    out_sPath = it.Value();
    out_sPath.AppendPath(szEnd); // szEnd might be on \0 or a slash
    out_sPath.MakeCleanPath();
    return WD_SUCCESS;
  }

  if (sName == "sdk")
  {
    sDirectory.Shrink(3, 0);
    out_sPath = GetSdkRootDirectory();
    out_sPath.AppendPath(sDirectory);
    out_sPath.MakeCleanPath();
    return WD_SUCCESS;
  }

  if (sName == "user")
  {
    sDirectory.Shrink(4, 0);
    out_sPath = wdOSFile::GetUserDataFolder();
    out_sPath.AppendPath(sDirectory);
    out_sPath.MakeCleanPath();
    return WD_SUCCESS;
  }

  if (sName == "temp")
  {
    sDirectory.Shrink(4, 0);
    out_sPath = wdOSFile::GetTempDataFolder();
    out_sPath.AppendPath(sDirectory);
    out_sPath.MakeCleanPath();
    return WD_SUCCESS;
  }

  if (sName == "appdir")
  {
    sDirectory.Shrink(6, 0);
    out_sPath = wdOSFile::GetApplicationDirectory();
    out_sPath.AppendPath(sDirectory);
    out_sPath.MakeCleanPath();
    return WD_SUCCESS;
  }

  return WD_FAILURE;
}


wdMutex& wdFileSystem::GetMutex()
{
  WD_ASSERT_DEV(s_pData != nullptr, "FileSystem is not initialized.");
  return s_pData->m_FsMutex;
}

#if WD_ENABLED(WD_SUPPORTS_FILE_ITERATORS)

void wdFileSystem::StartSearch(wdFileSystemIterator& ref_iterator, wdStringView sSearchTerm, wdBitflags<wdFileSystemIteratorFlags> flags /*= wdFileSystemIteratorFlags::Default*/)
{
  WD_LOCK(s_pData->m_FsMutex);

  wdHybridArray<wdString, 16> folders;
  wdStringBuilder sDdPath;

  for (const auto& dd : s_pData->m_DataDirectories)
  {
    sDdPath = dd.m_pDataDirectory->GetRedirectedDataDirectoryPath();

    if (ResolvePath(sDdPath, &sDdPath, nullptr).Failed())
      continue;

    if (sDdPath.IsEmpty() || !wdOSFile::ExistsDirectory(sDdPath))
      continue;


    folders.PushBack(sDdPath);
  }

  ref_iterator.StartMultiFolderSearch(folders, sSearchTerm, flags);
}

#endif

wdResult wdFileSystem::CreateDirectoryStructure(wdStringView sPath)
{
  wdStringBuilder sRedir;
  WD_SUCCEED_OR_RETURN(ResolveSpecialDirectory(sPath, sRedir));

  return wdOSFile::CreateDirectoryStructure(sRedir);
}

WD_STATICLINK_FILE(Foundation, Foundation_IO_FileSystem_Implementation_FileSystem);

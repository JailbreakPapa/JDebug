#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/OSFile.h>

wdString64 wdOSFile::s_sApplicationPath;
wdString64 wdOSFile::s_sUserDataPath;
wdString64 wdOSFile::s_sTempDataPath;
wdAtomicInteger32 wdOSFile::s_iFileCounter;
wdOSFile::Event wdOSFile::s_FileEvents;

wdFileStats::wdFileStats() = default;
wdFileStats::~wdFileStats() = default;

void wdFileStats::GetFullPath(wdStringBuilder& ref_sPath) const
{
  ref_sPath.Set(m_sParentPath, "/", m_sName);
  ref_sPath.MakeCleanPath();
}

wdOSFile::wdOSFile()
{
  m_FileMode = wdFileOpenMode::None;
  m_iFileID = s_iFileCounter.Increment();
}

wdOSFile::~wdOSFile()
{
  Close();
}

wdResult wdOSFile::Open(wdStringView sFile, wdFileOpenMode::Enum openMode, wdFileShareMode::Enum fileShareMode)
{
  m_iFileID = s_iFileCounter.Increment();

  WD_ASSERT_DEV(openMode >= wdFileOpenMode::Read && openMode <= wdFileOpenMode::Append, "Invalid Mode");
  WD_ASSERT_DEV(!IsOpen(), "The file has already been opened.");

  const wdTime t0 = wdTime::Now();

  m_sFileName = sFile;
  m_sFileName.MakeCleanPath();
  m_sFileName.MakePathSeparatorsNative();

  wdResult Res = WD_FAILURE;

  if (!m_sFileName.IsAbsolutePath())
    goto done;

  {
    wdStringBuilder sFolder = m_sFileName.GetFileDirectory();

    if (openMode == wdFileOpenMode::Write || openMode == wdFileOpenMode::Append)
    {
      WD_SUCCEED_OR_RETURN(CreateDirectoryStructure(sFolder.GetData()));
    }
  }

  if (InternalOpen(m_sFileName.GetData(), openMode, fileShareMode) == WD_SUCCESS)
  {
    m_FileMode = openMode;
    Res = WD_SUCCESS;
    goto done;
  }

  m_sFileName.Clear();
  m_FileMode = wdFileOpenMode::None;
  goto done;

done:
  const wdTime t1 = wdTime::Now();
  const wdTime tdiff = t1 - t0;

  EventData e;
  e.m_bSuccess = Res == WD_SUCCESS;
  e.m_Duration = tdiff;
  e.m_FileMode = openMode;
  e.m_iFileID = m_iFileID;
  e.m_sFile = m_sFileName;
  e.m_EventType = EventType::FileOpen;

  s_FileEvents.Broadcast(e);

  return Res;
}

bool wdOSFile::IsOpen() const
{
  return m_FileMode != wdFileOpenMode::None;
}

void wdOSFile::Close()
{
  if (!IsOpen())
    return;

  const wdTime t0 = wdTime::Now();

  InternalClose();

  const wdTime t1 = wdTime::Now();
  const wdTime tdiff = t1 - t0;

  EventData e;
  e.m_bSuccess = true;
  e.m_Duration = tdiff;
  e.m_iFileID = m_iFileID;
  e.m_sFile = m_sFileName;
  e.m_EventType = EventType::FileClose;

  s_FileEvents.Broadcast(e);

  m_sFileName.Clear();
  m_FileMode = wdFileOpenMode::None;
}

wdResult wdOSFile::Write(const void* pBuffer, wdUInt64 uiBytes)
{
  WD_ASSERT_DEV((m_FileMode == wdFileOpenMode::Write) || (m_FileMode == wdFileOpenMode::Append), "The file is not opened for writing.");
  WD_ASSERT_DEV(pBuffer != nullptr, "pBuffer must not be nullptr.");

  const wdTime t0 = wdTime::Now();

  const wdResult Res = InternalWrite(pBuffer, uiBytes);

  const wdTime t1 = wdTime::Now();
  const wdTime tdiff = t1 - t0;

  EventData e;
  e.m_bSuccess = Res == WD_SUCCESS;
  e.m_Duration = tdiff;
  e.m_iFileID = m_iFileID;
  e.m_sFile = m_sFileName;
  e.m_EventType = EventType::FileWrite;
  e.m_uiBytesAccessed = uiBytes;

  s_FileEvents.Broadcast(e);

  return Res;
}

wdUInt64 wdOSFile::Read(void* pBuffer, wdUInt64 uiBytes)
{
  WD_ASSERT_DEV(m_FileMode == wdFileOpenMode::Read, "The file is not opened for reading.");
  WD_ASSERT_DEV(pBuffer != nullptr, "pBuffer must not be nullptr.");

  const wdTime t0 = wdTime::Now();

  const wdUInt64 Res = InternalRead(pBuffer, uiBytes);

  const wdTime t1 = wdTime::Now();
  const wdTime tdiff = t1 - t0;

  EventData e;
  e.m_bSuccess = (Res == uiBytes);
  e.m_Duration = tdiff;
  e.m_iFileID = m_iFileID;
  e.m_sFile = m_sFileName;
  e.m_EventType = EventType::FileRead;
  e.m_uiBytesAccessed = Res;

  s_FileEvents.Broadcast(e);

  return Res;
}

wdUInt64 wdOSFile::ReadAll(wdDynamicArray<wdUInt8>& out_fileContent)
{
  WD_ASSERT_DEV(m_FileMode == wdFileOpenMode::Read, "The file is not opened for reading.");

  out_fileContent.Clear();
  out_fileContent.SetCountUninitialized((wdUInt32)GetFileSize());

  if (!out_fileContent.IsEmpty())
  {
    Read(out_fileContent.GetData(), out_fileContent.GetCount());
  }

  return out_fileContent.GetCount();
}

wdUInt64 wdOSFile::GetFilePosition() const
{
  WD_ASSERT_DEV(IsOpen(), "The file must be open to tell the file pointer position.");

  return InternalGetFilePosition();
}

void wdOSFile::SetFilePosition(wdInt64 iDistance, wdFileSeekMode::Enum pos) const
{
  WD_ASSERT_DEV(IsOpen(), "The file must be open to tell the file pointer position.");
  WD_ASSERT_DEV(m_FileMode != wdFileOpenMode::Append, "SetFilePosition is not possible on files that were opened for appending.");

  return InternalSetFilePosition(iDistance, pos);
}

wdUInt64 wdOSFile::GetFileSize() const
{
  WD_ASSERT_DEV(IsOpen(), "The file must be open to tell the file size.");

  const wdInt64 iCurPos = static_cast<wdInt64>(GetFilePosition());

  // to circumvent the 'append does not support SetFilePosition' assert, we use the internal function directly
  InternalSetFilePosition(0, wdFileSeekMode::FromEnd);

  const wdUInt64 uiCurSize = static_cast<wdInt64>(GetFilePosition());

  // to circumvent the 'append does not support SetFilePosition' assert, we use the internal function directly
  InternalSetFilePosition(iCurPos, wdFileSeekMode::FromStart);

  return uiCurSize;
}

const wdString wdOSFile::MakePathAbsoluteWithCWD(wdStringView sPath)
{
  wdStringBuilder tmp = sPath;
  tmp.MakeCleanPath();

  if (tmp.IsRelativePath())
  {
    tmp.PrependFormat("{}/", GetCurrentWorkingDirectory());
    tmp.MakeCleanPath();
  }

  return tmp;
}

bool wdOSFile::ExistsFile(wdStringView sFile)
{
  const wdTime t0 = wdTime::Now();

  wdStringBuilder s(sFile);
  s.MakeCleanPath();
  s.MakePathSeparatorsNative();

  const bool bRes = InternalExistsFile(s);

  const wdTime t1 = wdTime::Now();
  const wdTime tdiff = t1 - t0;


  EventData e;
  e.m_bSuccess = bRes;
  e.m_Duration = tdiff;
  e.m_iFileID = s_iFileCounter.Increment();
  e.m_sFile = s;
  e.m_EventType = EventType::FileExists;

  s_FileEvents.Broadcast(e);

  return bRes;
}

bool wdOSFile::ExistsDirectory(wdStringView sDirectory)
{
  const wdTime t0 = wdTime::Now();

  wdStringBuilder s(sDirectory);
  s.MakeCleanPath();
  s.MakePathSeparatorsNative();

  WD_ASSERT_DEV(s.IsAbsolutePath(), "Path must be absolute");

  const bool bRes = InternalExistsDirectory(s);

  const wdTime t1 = wdTime::Now();
  const wdTime tdiff = t1 - t0;


  EventData e;
  e.m_bSuccess = bRes;
  e.m_Duration = tdiff;
  e.m_iFileID = s_iFileCounter.Increment();
  e.m_sFile = s;
  e.m_EventType = EventType::DirectoryExists;

  s_FileEvents.Broadcast(e);

  return bRes;
}

wdResult wdOSFile::DeleteFile(wdStringView sFile)
{
  const wdTime t0 = wdTime::Now();

  wdStringBuilder s(sFile);
  s.MakeCleanPath();
  s.MakePathSeparatorsNative();

  const wdResult Res = InternalDeleteFile(s.GetData());

  const wdTime t1 = wdTime::Now();
  const wdTime tdiff = t1 - t0;

  EventData e;
  e.m_bSuccess = Res == WD_SUCCESS;
  e.m_Duration = tdiff;
  e.m_iFileID = s_iFileCounter.Increment();
  e.m_sFile = sFile;
  e.m_EventType = EventType::FileDelete;

  s_FileEvents.Broadcast(e);

  return Res;
}

wdResult wdOSFile::CreateDirectoryStructure(wdStringView sDirectory)
{
  const wdTime t0 = wdTime::Now();

  wdStringBuilder s(sDirectory);
  s.MakeCleanPath();
  s.MakePathSeparatorsNative();

  WD_ASSERT_DEV(s.IsAbsolutePath(), "The path '{0}' is not absolute.", s);

  wdStringBuilder sCurPath;

  auto it = s.GetIteratorFront();

  wdResult Res = WD_SUCCESS;

  while (it.IsValid())
  {
    while ((it.GetCharacter() != '\0') && (!wdPathUtils::IsPathSeparator(it.GetCharacter())))
    {
      sCurPath.Append(it.GetCharacter());
      ++it;
    }

    sCurPath.Append(it.GetCharacter());
    ++it;

    if (InternalCreateDirectory(sCurPath.GetData()) == WD_FAILURE)
    {
      Res = WD_FAILURE;
      break;
    }
  }

  const wdTime t1 = wdTime::Now();
  const wdTime tdiff = t1 - t0;

  EventData e;
  e.m_bSuccess = Res == WD_SUCCESS;
  e.m_Duration = tdiff;
  e.m_iFileID = s_iFileCounter.Increment();
  e.m_sFile = sDirectory;
  e.m_EventType = EventType::MakeDir;

  s_FileEvents.Broadcast(e);

  return Res;
}

wdResult wdOSFile::MoveFileOrDirectory(wdStringView sDirectoryFrom, wdStringView sDirectoryTo)
{
  wdStringBuilder sFrom(sDirectoryFrom);
  sFrom.MakeCleanPath();
  sFrom.MakePathSeparatorsNative();

  wdStringBuilder sTo(sDirectoryTo);
  sTo.MakeCleanPath();
  sTo.MakePathSeparatorsNative();

  return InternalMoveFileOrDirectory(sFrom, sTo);
}

wdResult wdOSFile::CopyFile(wdStringView sSource, wdStringView sDestination)
{
  const wdTime t0 = wdTime::Now();

  wdOSFile SrcFile, DstFile;

  wdResult Res = WD_FAILURE;

  if (SrcFile.Open(sSource, wdFileOpenMode::Read) == WD_FAILURE)
    goto done;

  DstFile.m_bRetryOnSharingViolation = false;
  if (DstFile.Open(sDestination, wdFileOpenMode::Write) == WD_FAILURE)
    goto done;

  {
    const wdUInt32 uiTempSize = 1024 * 1024 * 8; // 8 MB

    // can't allocate that much data on the stack
    wdDynamicArray<wdUInt8> TempBuffer;
    TempBuffer.SetCountUninitialized(uiTempSize);

    while (true)
    {
      const wdUInt64 uiRead = SrcFile.Read(&TempBuffer[0], uiTempSize);

      if (uiRead == 0)
        break;

      if (DstFile.Write(&TempBuffer[0], uiRead) == WD_FAILURE)
        goto done;
    }
  }

  Res = WD_SUCCESS;

done:

  const wdTime t1 = wdTime::Now();
  const wdTime tdiff = t1 - t0;

  EventData e;
  e.m_bSuccess = Res == WD_SUCCESS;
  e.m_Duration = tdiff;
  e.m_iFileID = s_iFileCounter.Increment();
  e.m_sFile = sSource;
  e.m_sFile2 = sDestination;
  e.m_EventType = EventType::FileCopy;

  s_FileEvents.Broadcast(e);

  return Res;
}

#if WD_ENABLED(WD_SUPPORTS_FILE_STATS)

wdResult wdOSFile::GetFileStats(wdStringView sFileOrFolder, wdFileStats& out_stats)
{
  const wdTime t0 = wdTime::Now();

  wdStringBuilder s = sFileOrFolder;
  s.MakeCleanPath();
  s.MakePathSeparatorsNative();

  WD_ASSERT_DEV(s.IsAbsolutePath(), "The path '{0}' is not absolute.", s);

  const wdResult Res = InternalGetFileStats(s.GetData(), out_stats);

  const wdTime t1 = wdTime::Now();
  const wdTime tdiff = t1 - t0;

  EventData e;
  e.m_bSuccess = Res == WD_SUCCESS;
  e.m_Duration = tdiff;
  e.m_iFileID = s_iFileCounter.Increment();
  e.m_sFile = sFileOrFolder;
  e.m_EventType = EventType::FileStat;

  s_FileEvents.Broadcast(e);

  return Res;
}

#  if WD_ENABLED(WD_SUPPORTS_CASE_INSENSITIVE_PATHS) && WD_ENABLED(WD_SUPPORTS_UNRESTRICTED_FILE_ACCESS)
wdResult wdOSFile::GetFileCasing(wdStringView sFileOrFolder, wdStringBuilder& out_sCorrectSpelling)
{
  /// \todo We should implement this also on wdFileSystem, to be able to support stats through virtual filesystems

  const wdTime t0 = wdTime::Now();

  wdStringBuilder s(sFileOrFolder);
  s.MakeCleanPath();
  s.MakePathSeparatorsNative();

  WD_ASSERT_DEV(s.IsAbsolutePath(), "The path '{0}' is not absolute.", s);

  wdStringBuilder sCurPath;

  auto it = s.GetIteratorFront();

  out_sCorrectSpelling.Clear();

  wdResult Res = WD_SUCCESS;

  while (it.IsValid())
  {
    while ((it.GetCharacter() != '\0') && (!wdPathUtils::IsPathSeparator(it.GetCharacter())))
    {
      sCurPath.Append(it.GetCharacter());
      ++it;
    }

    if (!sCurPath.IsEmpty())
    {
      wdFileStats stats;
      if (GetFileStats(sCurPath.GetData(), stats) == WD_FAILURE)
      {
        Res = WD_FAILURE;
        break;
      }

      out_sCorrectSpelling.AppendPath(stats.m_sName);
    }
    sCurPath.Append(it.GetCharacter());
    ++it;
  }

  const wdTime t1 = wdTime::Now();
  const wdTime tdiff = t1 - t0;

  EventData e;
  e.m_bSuccess = Res == WD_SUCCESS;
  e.m_Duration = tdiff;
  e.m_iFileID = s_iFileCounter.Increment();
  e.m_sFile = sFileOrFolder;
  e.m_EventType = EventType::FileCasing;

  s_FileEvents.Broadcast(e);

  return Res;
}

#  endif // WD_SUPPORTS_CASE_INSENSITIVE_PATHS && WD_SUPPORTS_UNRESTRICTED_FILE_ACCESS

#endif // WD_SUPPORTS_FILE_STATS

#if WD_ENABLED(WD_SUPPORTS_FILE_ITERATORS) && WD_ENABLED(WD_SUPPORTS_FILE_STATS)

void wdOSFile::GatherAllItemsInFolder(wdDynamicArray<wdFileStats>& out_itemList, wdStringView sFolder, wdBitflags<wdFileSystemIteratorFlags> flags /*= wdFileSystemIteratorFlags::All*/)
{
  out_itemList.Clear();

  wdFileSystemIterator iterator;
  iterator.StartSearch(sFolder, flags);

  if (!iterator.IsValid())
    return;

  out_itemList.Reserve(128);

  while (iterator.IsValid())
  {
    out_itemList.PushBack(iterator.GetStats());

    iterator.Next();
  }
}

wdResult wdOSFile::CopyFolder(wdStringView sSourceFolder, wdStringView sDestinationFolder, wdDynamicArray<wdString>* out_pFilesCopied /*= nullptr*/)
{
  wdDynamicArray<wdFileStats> items;
  GatherAllItemsInFolder(items, sSourceFolder);

  wdStringBuilder srcPath;
  wdStringBuilder dstPath;
  wdStringBuilder relPath;

  for (const auto& item : items)
  {
    srcPath = item.m_sParentPath;
    srcPath.AppendPath(item.m_sName);

    relPath = srcPath;

    if (relPath.MakeRelativeTo(sSourceFolder).Failed())
      return WD_FAILURE; // unexpected to ever fail, but don't want to assert on it

    dstPath = sDestinationFolder;
    dstPath.AppendPath(relPath);

    if (item.m_bIsDirectory)
    {
      if (wdOSFile::CreateDirectoryStructure(dstPath).Failed())
        return WD_FAILURE;
    }
    else
    {
      if (wdOSFile::CopyFile(srcPath, dstPath).Failed())
        return WD_FAILURE;

      if (out_pFilesCopied)
      {
        out_pFilesCopied->PushBack(dstPath);
      }
    }

    // TODO: make sure to remove read-only flags of copied files ?
  }

  return WD_SUCCESS;
}

wdResult wdOSFile::DeleteFolder(wdStringView sFolder)
{
  wdDynamicArray<wdFileStats> items;
  GatherAllItemsInFolder(items, sFolder);

  wdStringBuilder fullPath;

  for (const auto& item : items)
  {
    if (item.m_bIsDirectory)
      continue;

    fullPath = item.m_sParentPath;
    fullPath.AppendPath(item.m_sName);

    if (wdOSFile::DeleteFile(fullPath).Failed())
      return WD_FAILURE;
  }

  for (wdUInt32 i = items.GetCount(); i > 0; --i)
  {
    const auto& item = items[i - 1];

    if (!item.m_bIsDirectory)
      continue;

    fullPath = item.m_sParentPath;
    fullPath.AppendPath(item.m_sName);

    if (wdOSFile::InternalDeleteDirectory(fullPath).Failed())
      return WD_FAILURE;
  }

  if (wdOSFile::InternalDeleteDirectory(sFolder).Failed())
    return WD_FAILURE;

  return WD_SUCCESS;
}

#endif // WD_ENABLED(WD_SUPPORTS_FILE_ITERATORS) && WD_ENABLED(WD_SUPPORTS_FILE_STATS)

#if WD_ENABLED(WD_SUPPORTS_FILE_ITERATORS)

void wdFileSystemIterator::StartMultiFolderSearch(wdArrayPtr<wdString> startFolders, wdStringView sSearchTerm, wdBitflags<wdFileSystemIteratorFlags> flags /*= wdFileSystemIteratorFlags::Default*/)
{
  if (startFolders.IsEmpty())
    return;

  m_sMultiSearchTerm = sSearchTerm;
  m_Flags = flags;
  m_uiCurrentStartFolder = 0;
  m_StartFolders = startFolders;

  wdStringBuilder search = startFolders[m_uiCurrentStartFolder];
  search.AppendPath(sSearchTerm);

  StartSearch(search, m_Flags);

  if (!IsValid())
  {
    Next();
  }
}

void wdFileSystemIterator::Next()
{
  while (true)
  {
    const wdInt32 res = InternalNext();

    if (res == 1) // success
    {
      return;
    }
    else if (res == 0) // failure
    {
      ++m_uiCurrentStartFolder;

      if (m_uiCurrentStartFolder < m_StartFolders.GetCount())
      {
        wdStringBuilder search = m_StartFolders[m_uiCurrentStartFolder];
        search.AppendPath(m_sMultiSearchTerm);

        StartSearch(search, m_Flags);
      }
      else
      {
        return;
      }

      if (IsValid())
      {
        return;
      }
    }
    else
    {
      // call InternalNext() again
    }
  }
}

void wdFileSystemIterator::SkipFolder()
{
  WD_ASSERT_DEBUG(m_Flags.IsSet(wdFileSystemIteratorFlags::Recursive), "SkipFolder has no meaning when the iterator is not set to be recursive.");
  WD_ASSERT_DEBUG(m_CurFile.m_bIsDirectory, "SkipFolder can only be called when the current object is a folder.");

  m_Flags.Remove(wdFileSystemIteratorFlags::Recursive);

  Next();

  m_Flags.Add(wdFileSystemIteratorFlags::Recursive);
}

#endif


#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/IO/Implementation/Win/OSFile_win.h>

// For UWP we're currently using a mix of WinRT functions and posix.
#  if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)
#    include <Foundation/IO/Implementation/Posix/OSFile_posix.h>
#  endif
#elif WD_ENABLED(WD_USE_POSIX_FILE_API)
#  include <Foundation/IO/Implementation/Posix/OSFile_posix.h>
#else
#  error "Unknown Platform."
#endif

WD_STATICLINK_FILE(Foundation, Foundation_IO_Implementation_OSFile);

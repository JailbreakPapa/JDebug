#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/OSFile.h>

nsString64 nsOSFile::s_sApplicationPath;
nsString64 nsOSFile::s_sUserDataPath;
nsString64 nsOSFile::s_sTempDataPath;
nsString64 nsOSFile::s_sUserDocumentsPath;
nsAtomicInteger32 nsOSFile::s_iFileCounter;

nsOSFile::Event nsOSFile::s_FileEvents;

nsFileStats::nsFileStats() = default;
nsFileStats::~nsFileStats() = default;

void nsFileStats::GetFullPath(nsStringBuilder& ref_sPath) const
{
  ref_sPath.Set(m_sParentPath, "/", m_sName);
  ref_sPath.MakeCleanPath();
}

nsOSFile::nsOSFile()
{
  m_FileMode = nsFileOpenMode::None;
  m_iFileID = s_iFileCounter.Increment();
}

nsOSFile::~nsOSFile()
{
  Close();
}

nsResult nsOSFile::Open(nsStringView sFile, nsFileOpenMode::Enum openMode, nsFileShareMode::Enum fileShareMode)
{
  m_iFileID = s_iFileCounter.Increment();

  NS_ASSERT_DEV(openMode >= nsFileOpenMode::Read && openMode <= nsFileOpenMode::Append, "Invalid Mode");
  NS_ASSERT_DEV(!IsOpen(), "The file has already been opened.");

  const nsTime t0 = nsTime::Now();

  m_sFileName = sFile;
  m_sFileName.MakeCleanPath();
  m_sFileName.MakePathSeparatorsNative();

  nsResult Res = NS_FAILURE;

  if (!m_sFileName.IsAbsolutePath())
    goto done;

  {
    nsStringBuilder sFolder = m_sFileName.GetFileDirectory();

    if (openMode == nsFileOpenMode::Write || openMode == nsFileOpenMode::Append)
    {
      NS_SUCCEED_OR_RETURN(CreateDirectoryStructure(sFolder.GetData()));
    }
  }

  if (InternalOpen(m_sFileName.GetData(), openMode, fileShareMode) == NS_SUCCESS)
  {
    m_FileMode = openMode;
    Res = NS_SUCCESS;
    goto done;
  }

  m_sFileName.Clear();
  m_FileMode = nsFileOpenMode::None;
  goto done;

done:
  const nsTime t1 = nsTime::Now();
  const nsTime tdiff = t1 - t0;

  EventData e;
  e.m_bSuccess = Res == NS_SUCCESS;
  e.m_Duration = tdiff;
  e.m_FileMode = openMode;
  e.m_iFileID = m_iFileID;
  e.m_sFile = m_sFileName;
  e.m_EventType = EventType::FileOpen;

  s_FileEvents.Broadcast(e);

  return Res;
}

bool nsOSFile::IsOpen() const
{
  return m_FileMode != nsFileOpenMode::None;
}

void nsOSFile::Close()
{
  if (!IsOpen())
    return;

  const nsTime t0 = nsTime::Now();

  InternalClose();

  const nsTime t1 = nsTime::Now();
  const nsTime tdiff = t1 - t0;

  EventData e;
  e.m_bSuccess = true;
  e.m_Duration = tdiff;
  e.m_iFileID = m_iFileID;
  e.m_sFile = m_sFileName;
  e.m_EventType = EventType::FileClose;

  s_FileEvents.Broadcast(e);

  m_sFileName.Clear();
  m_FileMode = nsFileOpenMode::None;
}

nsResult nsOSFile::Write(const void* pBuffer, nsUInt64 uiBytes)
{
  NS_ASSERT_DEV((m_FileMode == nsFileOpenMode::Write) || (m_FileMode == nsFileOpenMode::Append), "The file is not opened for writing.");
  NS_ASSERT_DEV(pBuffer != nullptr, "pBuffer must not be nullptr.");

  const nsTime t0 = nsTime::Now();

  const nsResult Res = InternalWrite(pBuffer, uiBytes);

  const nsTime t1 = nsTime::Now();
  const nsTime tdiff = t1 - t0;

  EventData e;
  e.m_bSuccess = Res == NS_SUCCESS;
  e.m_Duration = tdiff;
  e.m_iFileID = m_iFileID;
  e.m_sFile = m_sFileName;
  e.m_EventType = EventType::FileWrite;
  e.m_uiBytesAccessed = uiBytes;

  s_FileEvents.Broadcast(e);

  return Res;
}

nsUInt64 nsOSFile::Read(void* pBuffer, nsUInt64 uiBytes)
{
  NS_ASSERT_DEV(m_FileMode == nsFileOpenMode::Read, "The file is not opened for reading.");
  NS_ASSERT_DEV(pBuffer != nullptr, "pBuffer must not be nullptr.");

  const nsTime t0 = nsTime::Now();

  const nsUInt64 Res = InternalRead(pBuffer, uiBytes);

  const nsTime t1 = nsTime::Now();
  const nsTime tdiff = t1 - t0;

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

nsUInt64 nsOSFile::ReadAll(nsDynamicArray<nsUInt8>& out_fileContent)
{
  NS_ASSERT_DEV(m_FileMode == nsFileOpenMode::Read, "The file is not opened for reading.");

  out_fileContent.Clear();
  out_fileContent.SetCountUninitialized((nsUInt32)GetFileSize());

  if (!out_fileContent.IsEmpty())
  {
    Read(out_fileContent.GetData(), out_fileContent.GetCount());
  }

  return out_fileContent.GetCount();
}

nsUInt64 nsOSFile::GetFilePosition() const
{
  NS_ASSERT_DEV(IsOpen(), "The file must be open to tell the file pointer position.");

  return InternalGetFilePosition();
}

void nsOSFile::SetFilePosition(nsInt64 iDistance, nsFileSeekMode::Enum pos) const
{
  NS_ASSERT_DEV(IsOpen(), "The file must be open to tell the file pointer position.");
  NS_ASSERT_DEV(m_FileMode != nsFileOpenMode::Append, "SetFilePosition is not possible on files that were opened for appending.");

  return InternalSetFilePosition(iDistance, pos);
}

nsUInt64 nsOSFile::GetFileSize() const
{
  NS_ASSERT_DEV(IsOpen(), "The file must be open to tell the file size.");

  const nsInt64 iCurPos = static_cast<nsInt64>(GetFilePosition());

  // to circumvent the 'append does not support SetFilePosition' assert, we use the internal function directly
  InternalSetFilePosition(0, nsFileSeekMode::FromEnd);

  const nsUInt64 uiCurSize = static_cast<nsInt64>(GetFilePosition());

  // to circumvent the 'append does not support SetFilePosition' assert, we use the internal function directly
  InternalSetFilePosition(iCurPos, nsFileSeekMode::FromStart);

  return uiCurSize;
}

const nsString nsOSFile::MakePathAbsoluteWithCWD(nsStringView sPath)
{
  nsStringBuilder tmp = sPath;
  tmp.MakeCleanPath();

  if (tmp.IsRelativePath())
  {
    tmp.PrependFormat("{}/", GetCurrentWorkingDirectory());
    tmp.MakeCleanPath();
  }

  return tmp;
}

bool nsOSFile::ExistsFile(nsStringView sFile)
{
  const nsTime t0 = nsTime::Now();

  nsStringBuilder s(sFile);
  s.MakeCleanPath();
  s.MakePathSeparatorsNative();

  const bool bRes = InternalExistsFile(s);

  const nsTime t1 = nsTime::Now();
  const nsTime tdiff = t1 - t0;


  EventData e;
  e.m_bSuccess = bRes;
  e.m_Duration = tdiff;
  e.m_iFileID = s_iFileCounter.Increment();
  e.m_sFile = s;
  e.m_EventType = EventType::FileExists;

  s_FileEvents.Broadcast(e);

  return bRes;
}

bool nsOSFile::ExistsDirectory(nsStringView sDirectory)
{
  const nsTime t0 = nsTime::Now();

  nsStringBuilder s(sDirectory);
  s.MakeCleanPath();
  s.MakePathSeparatorsNative();

  NS_ASSERT_DEV(s.IsAbsolutePath(), "Path must be absolute");

  const bool bRes = InternalExistsDirectory(s);

  const nsTime t1 = nsTime::Now();
  const nsTime tdiff = t1 - t0;


  EventData e;
  e.m_bSuccess = bRes;
  e.m_Duration = tdiff;
  e.m_iFileID = s_iFileCounter.Increment();
  e.m_sFile = s;
  e.m_EventType = EventType::DirectoryExists;

  s_FileEvents.Broadcast(e);

  return bRes;
}

void nsOSFile::FindFreeFilename(nsStringBuilder& inout_sPath, nsStringView sSuffix /*= {}*/)
{
  NS_ASSERT_DEV(!inout_sPath.IsEmpty() && inout_sPath.IsAbsolutePath(), "Invalid input path.");

  if (!nsOSFile::ExistsFile(inout_sPath))
    return;

  const nsString orgName = inout_sPath.GetFileName();

  nsStringBuilder newName;

  for (nsUInt32 i = 1; i < 100000; ++i)
  {
    newName.SetFormat("{}{}{}", orgName, sSuffix, i);

    inout_sPath.ChangeFileName(newName);
    if (!nsOSFile::ExistsFile(inout_sPath))
      return;
  }

  NS_REPORT_FAILURE("Something went wrong.");
}

nsResult nsOSFile::DeleteFile(nsStringView sFile)
{
  const nsTime t0 = nsTime::Now();

  nsStringBuilder s(sFile);
  s.MakeCleanPath();
  s.MakePathSeparatorsNative();

  const nsResult Res = InternalDeleteFile(s.GetData());

  const nsTime t1 = nsTime::Now();
  const nsTime tdiff = t1 - t0;

  EventData e;
  e.m_bSuccess = Res == NS_SUCCESS;
  e.m_Duration = tdiff;
  e.m_iFileID = s_iFileCounter.Increment();
  e.m_sFile = sFile;
  e.m_EventType = EventType::FileDelete;

  s_FileEvents.Broadcast(e);

  return Res;
}

nsStringView nsOSFile::GetApplicationDirectory()
{
  if (s_sApplicationPath.IsEmpty())
  {
    // s_sApplicationPath is filled out and cached by GetApplicationPath(), so call that first, if necessary
    GetApplicationPath();
  }

  NS_ASSERT_ALWAYS(!s_sApplicationPath.IsEmpty(), "Invalid application directory");
  return s_sApplicationPath.GetFileDirectory();
}

nsResult nsOSFile::CreateDirectoryStructure(nsStringView sDirectory)
{
  const nsTime t0 = nsTime::Now();

  nsStringBuilder s(sDirectory);
  s.MakeCleanPath();
  s.MakePathSeparatorsNative();

  NS_ASSERT_DEV(s.IsAbsolutePath(), "The path '{0}' is not absolute.", s);

  nsStringBuilder sCurPath;

  auto it = s.GetIteratorFront();

  nsResult Res = NS_SUCCESS;

  while (it.IsValid())
  {
    while ((it.GetCharacter() != '\0') && (!nsPathUtils::IsPathSeparator(it.GetCharacter())))
    {
      sCurPath.Append(it.GetCharacter());
      ++it;
    }

    sCurPath.Append(it.GetCharacter());
    ++it;

    if (InternalCreateDirectory(sCurPath.GetData()) == NS_FAILURE)
    {
      Res = NS_FAILURE;
      break;
    }
  }

  const nsTime t1 = nsTime::Now();
  const nsTime tdiff = t1 - t0;

  EventData e;
  e.m_bSuccess = Res == NS_SUCCESS;
  e.m_Duration = tdiff;
  e.m_iFileID = s_iFileCounter.Increment();
  e.m_sFile = sDirectory;
  e.m_EventType = EventType::MakeDir;

  s_FileEvents.Broadcast(e);

  return Res;
}

nsResult nsOSFile::MoveFileOrDirectory(nsStringView sDirectoryFrom, nsStringView sDirectoryTo)
{
  nsStringBuilder sFrom(sDirectoryFrom);
  sFrom.MakeCleanPath();
  sFrom.MakePathSeparatorsNative();

  nsStringBuilder sTo(sDirectoryTo);
  sTo.MakeCleanPath();
  sTo.MakePathSeparatorsNative();

  return InternalMoveFileOrDirectory(sFrom, sTo);
}

nsResult nsOSFile::CopyFile(nsStringView sSource, nsStringView sDestination)
{
  const nsTime t0 = nsTime::Now();

  nsOSFile SrcFile, DstFile;

  nsResult Res = NS_FAILURE;

  if (SrcFile.Open(sSource, nsFileOpenMode::Read) == NS_FAILURE)
    goto done;

  DstFile.m_bRetryOnSharingViolation = false;
  if (DstFile.Open(sDestination, nsFileOpenMode::Write) == NS_FAILURE)
    goto done;

  {
    const nsUInt32 uiTempSize = 1024 * 1024 * 8; // 8 MB

    // can't allocate that much data on the stack
    nsDynamicArray<nsUInt8> TempBuffer;
    TempBuffer.SetCountUninitialized(uiTempSize);

    while (true)
    {
      const nsUInt64 uiRead = SrcFile.Read(&TempBuffer[0], uiTempSize);

      if (uiRead == 0)
        break;

      if (DstFile.Write(&TempBuffer[0], uiRead) == NS_FAILURE)
        goto done;
    }
  }

  Res = NS_SUCCESS;

done:

  const nsTime t1 = nsTime::Now();
  const nsTime tdiff = t1 - t0;

  EventData e;
  e.m_bSuccess = Res == NS_SUCCESS;
  e.m_Duration = tdiff;
  e.m_iFileID = s_iFileCounter.Increment();
  e.m_sFile = sSource;
  e.m_sFile2 = sDestination;
  e.m_EventType = EventType::FileCopy;

  s_FileEvents.Broadcast(e);

  return Res;
}

#if NS_ENABLED(NS_SUPPORTS_FILE_STATS)

nsResult nsOSFile::GetFileStats(nsStringView sFileOrFolder, nsFileStats& out_stats)
{
  const nsTime t0 = nsTime::Now();

  nsStringBuilder s = sFileOrFolder;
  s.MakeCleanPath();
  s.MakePathSeparatorsNative();

  NS_ASSERT_DEV(s.IsAbsolutePath(), "The path '{0}' is not absolute.", s);

  const nsResult Res = InternalGetFileStats(s.GetData(), out_stats);

  const nsTime t1 = nsTime::Now();
  const nsTime tdiff = t1 - t0;

  EventData e;
  e.m_bSuccess = Res == NS_SUCCESS;
  e.m_Duration = tdiff;
  e.m_iFileID = s_iFileCounter.Increment();
  e.m_sFile = sFileOrFolder;
  e.m_EventType = EventType::FileStat;

  s_FileEvents.Broadcast(e);

  return Res;
}

#  if NS_ENABLED(NS_SUPPORTS_CASE_INSENSITIVE_PATHS) && NS_ENABLED(NS_SUPPORTS_UNRESTRICTED_FILE_ACCESS)
nsResult nsOSFile::GetFileCasing(nsStringView sFileOrFolder, nsStringBuilder& out_sCorrectSpelling)
{
  /// \todo We should implement this also on nsFileSystem, to be able to support stats through virtual filesystems

  const nsTime t0 = nsTime::Now();

  nsStringBuilder s(sFileOrFolder);
  s.MakeCleanPath();
  s.MakePathSeparatorsNative();

  NS_ASSERT_DEV(s.IsAbsolutePath(), "The path '{0}' is not absolute.", s);

  nsStringBuilder sCurPath;

  auto it = s.GetIteratorFront();

  out_sCorrectSpelling.Clear();

  nsResult Res = NS_SUCCESS;

  while (it.IsValid())
  {
    while ((it.GetCharacter() != '\0') && (!nsPathUtils::IsPathSeparator(it.GetCharacter())))
    {
      sCurPath.Append(it.GetCharacter());
      ++it;
    }

    if (!sCurPath.IsEmpty())
    {
      nsFileStats stats;
      if (GetFileStats(sCurPath.GetData(), stats) == NS_FAILURE)
      {
        Res = NS_FAILURE;
        break;
      }

      out_sCorrectSpelling.AppendPath(stats.m_sName);
    }
    sCurPath.Append(it.GetCharacter());
    ++it;
  }

  const nsTime t1 = nsTime::Now();
  const nsTime tdiff = t1 - t0;

  EventData e;
  e.m_bSuccess = Res == NS_SUCCESS;
  e.m_Duration = tdiff;
  e.m_iFileID = s_iFileCounter.Increment();
  e.m_sFile = sFileOrFolder;
  e.m_EventType = EventType::FileCasing;

  s_FileEvents.Broadcast(e);

  return Res;
}

#  endif // NS_SUPPORTS_CASE_INSENSITIVE_PATHS && NS_SUPPORTS_UNRESTRICTED_FILE_ACCESS

#endif   // NS_SUPPORTS_FILE_STATS

#if NS_ENABLED(NS_SUPPORTS_FILE_ITERATORS) && NS_ENABLED(NS_SUPPORTS_FILE_STATS)

void nsOSFile::GatherAllItemsInFolder(nsDynamicArray<nsFileStats>& out_itemList, nsStringView sFolder, nsBitflags<nsFileSystemIteratorFlags> flags /*= nsFileSystemIteratorFlags::All*/)
{
  out_itemList.Clear();

  nsFileSystemIterator iterator;
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

nsResult nsOSFile::CopyFolder(nsStringView sSourceFolder, nsStringView sDestinationFolder, nsDynamicArray<nsString>* out_pFilesCopied /*= nullptr*/)
{
  nsDynamicArray<nsFileStats> items;
  GatherAllItemsInFolder(items, sSourceFolder);

  nsStringBuilder srcPath;
  nsStringBuilder dstPath;
  nsStringBuilder relPath;

  for (const auto& item : items)
  {
    srcPath = item.m_sParentPath;
    srcPath.AppendPath(item.m_sName);

    relPath = srcPath;

    if (relPath.MakeRelativeTo(sSourceFolder).Failed())
      return NS_FAILURE; // unexpected to ever fail, but don't want to assert on it

    dstPath = sDestinationFolder;
    dstPath.AppendPath(relPath);

    if (item.m_bIsDirectory)
    {
      if (nsOSFile::CreateDirectoryStructure(dstPath).Failed())
        return NS_FAILURE;
    }
    else
    {
      if (nsOSFile::CopyFile(srcPath, dstPath).Failed())
        return NS_FAILURE;

      if (out_pFilesCopied)
      {
        out_pFilesCopied->PushBack(dstPath);
      }
    }

    // TODO: make sure to remove read-only flags of copied files ?
  }

  return NS_SUCCESS;
}

nsResult nsOSFile::DeleteFolder(nsStringView sFolder)
{
  nsDynamicArray<nsFileStats> items;
  GatherAllItemsInFolder(items, sFolder);

  nsStringBuilder fullPath;

  for (const auto& item : items)
  {
    if (item.m_bIsDirectory)
      continue;

    fullPath = item.m_sParentPath;
    fullPath.AppendPath(item.m_sName);

    if (nsOSFile::DeleteFile(fullPath).Failed())
      return NS_FAILURE;
  }

  for (nsUInt32 i = items.GetCount(); i > 0; --i)
  {
    const auto& item = items[i - 1];

    if (!item.m_bIsDirectory)
      continue;

    fullPath = item.m_sParentPath;
    fullPath.AppendPath(item.m_sName);

    if (nsOSFile::InternalDeleteDirectory(fullPath).Failed())
      return NS_FAILURE;
  }

  if (nsOSFile::InternalDeleteDirectory(sFolder).Failed())
    return NS_FAILURE;

  return NS_SUCCESS;
}

#endif // NS_ENABLED(NS_SUPPORTS_FILE_ITERATORS) && NS_ENABLED(NS_SUPPORTS_FILE_STATS)

#if NS_ENABLED(NS_SUPPORTS_FILE_ITERATORS)

void nsFileSystemIterator::StartMultiFolderSearch(nsArrayPtr<nsString> startFolders, nsStringView sSearchTerm, nsBitflags<nsFileSystemIteratorFlags> flags /*= nsFileSystemIteratorFlags::Default*/)
{
  if (startFolders.IsEmpty())
    return;

  m_sMultiSearchTerm = sSearchTerm;
  m_Flags = flags;
  m_uiCurrentStartFolder = 0;
  m_StartFolders = startFolders;

  nsStringBuilder search = startFolders[m_uiCurrentStartFolder];
  search.AppendPath(sSearchTerm);

  StartSearch(search, m_Flags);

  if (!IsValid())
  {
    Next();
  }
}

void nsFileSystemIterator::Next()
{
  while (true)
  {
    const nsInt32 res = InternalNext();

    if (res == 1) // success
    {
      return;
    }
    else if (res == 0) // failure
    {
      ++m_uiCurrentStartFolder;

      if (m_uiCurrentStartFolder < m_StartFolders.GetCount())
      {
        nsStringBuilder search = m_StartFolders[m_uiCurrentStartFolder];
        search.AppendPath(m_sMultiSearchTerm);

        if (search.IsAbsolutePath())
        {
          StartSearch(search, m_Flags);
        }
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

void nsFileSystemIterator::SkipFolder()
{
  NS_ASSERT_DEBUG(m_Flags.IsSet(nsFileSystemIteratorFlags::Recursive), "SkipFolder has no meaning when the iterator is not set to be recursive.");
  NS_ASSERT_DEBUG(m_CurFile.m_bIsDirectory, "SkipFolder can only be called when the current object is a folder.");

  m_Flags.Remove(nsFileSystemIteratorFlags::Recursive);

  Next();

  m_Flags.Add(nsFileSystemIteratorFlags::Recursive);
}

#endif

#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS)

#  include <Foundation/IO/OSFile.h>
#  include <Foundation/Logging/Log.h>
#  include <Foundation/Platform/Win/DosDevicePath_Win.h>
#  include <Foundation/Strings/StringConversion.h>
#  include <Foundation/Threading/ThreadUtils.h>

// Defined in Timestamp_win.h
nsInt64 FileTimeToEpoch(FILETIME fileTime);

static nsUInt64 HighLowToUInt64(nsUInt32 uiHigh32, nsUInt32 uiLow32)
{
  nsUInt64 uiHigh64 = uiHigh32;
  nsUInt64 uiLow64 = uiLow32;

  return (uiHigh64 << 32) | uiLow64;
}

#  if NS_DISABLED(NS_USE_POSIX_FILE_API)

#    include <Shlobj.h>

nsResult nsOSFile::InternalOpen(nsStringView sFile, nsFileOpenMode::Enum OpenMode, nsFileShareMode::Enum FileShareMode)
{
  const nsTime sleepTime = nsTime::MakeFromMilliseconds(20);
  nsInt32 iRetries = 20;

  if (FileShareMode == nsFileShareMode::Default)
  {
    // when 'default' share mode is requested, use 'share reads' when opening a file for reading
    // and use 'exclusive' when opening a file for writing

    if (OpenMode == nsFileOpenMode::Read)
    {
      FileShareMode = nsFileShareMode::SharedReads;
    }
    else
    {
      FileShareMode = nsFileShareMode::Exclusive;
    }
  }

  DWORD dwSharedMode = 0; // exclusive access
  if (FileShareMode == nsFileShareMode::SharedReads)
  {
    dwSharedMode = FILE_SHARE_READ;
  }

  while (iRetries > 0)
  {
    SetLastError(ERROR_SUCCESS);
    DWORD error = 0;

    switch (OpenMode)
    {
      case nsFileOpenMode::Read:
        m_FileData.m_pFileHandle = CreateFileW(nsDosDevicePath(sFile), GENERIC_READ, dwSharedMode, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        break;

      case nsFileOpenMode::Write:
        m_FileData.m_pFileHandle = CreateFileW(nsDosDevicePath(sFile), GENERIC_WRITE, dwSharedMode, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        break;

      case nsFileOpenMode::Append:
        m_FileData.m_pFileHandle = CreateFileW(nsDosDevicePath(sFile), FILE_APPEND_DATA, dwSharedMode, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

        // in append mode we need to set the file pointer to the end explicitly, otherwise GetFilePosition might return 0 the first time
        if ((m_FileData.m_pFileHandle != nullptr) && (m_FileData.m_pFileHandle != INVALID_HANDLE_VALUE))
          InternalSetFilePosition(0, nsFileSeekMode::FromEnd);

        break;

        NS_DEFAULT_CASE_NOT_IMPLEMENTED
    }

    const nsResult res = ((m_FileData.m_pFileHandle != nullptr) && (m_FileData.m_pFileHandle != INVALID_HANDLE_VALUE)) ? NS_SUCCESS : NS_FAILURE;

    if (res.Failed())
    {
      if (nsOSFile::ExistsDirectory(sFile))
      {
        // trying to 'open' a directory fails with little useful error codes such as 'access denied'
        return NS_FAILURE;
      }

      error = GetLastError();

      // file does not exist
      if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND)
        return res;
      // badly formed path, happens when two absolute paths are concatenated
      if (error == ERROR_INVALID_NAME)
        return res;

      if (error == ERROR_SHARING_VIOLATION
          // these two situations happen when the nsInspector is connected
          // for some reason, the networking blocks file reading (when run on the same machine)
          // retrying fixes the problem, but can introduce very long stalls
          || error == WSAEWOULDBLOCK || error == ERROR_SUCCESS)
      {
        if (m_bRetryOnSharingViolation)
        {
          --iRetries;
          nsThreadUtils::Sleep(sleepTime);
          continue; // try again
        }
        else
        {
          return res;
        }
      }

      // anything else, print an error (for now)
      nsLog::Error("CreateFile failed with error {0}", nsArgErrorCode(error));
    }

    return res;
  }

  return NS_FAILURE;
}

void nsOSFile::InternalClose()
{
  CloseHandle(m_FileData.m_pFileHandle);
  m_FileData.m_pFileHandle = INVALID_HANDLE_VALUE;
}

nsResult nsOSFile::InternalWrite(const void* pBuffer, nsUInt64 uiBytes)
{
  const nsUInt32 uiBatchBytes = 1024 * 1024 * 1024; // 1 GB

  // first write out all the data in 1GB batches
  while (uiBytes > uiBatchBytes)
  {
    DWORD uiBytesWritten = 0;
    if ((!WriteFile(m_FileData.m_pFileHandle, pBuffer, uiBatchBytes, &uiBytesWritten, nullptr)) || (uiBytesWritten != uiBatchBytes))
      return NS_FAILURE;

    uiBytes -= uiBatchBytes;
    pBuffer = nsMemoryUtils::AddByteOffset(pBuffer, uiBatchBytes);
  }

  if (uiBytes > 0)
  {
    const nsUInt32 uiBytes32 = static_cast<nsUInt32>(uiBytes);

    DWORD uiBytesWritten = 0;
    if ((!WriteFile(m_FileData.m_pFileHandle, pBuffer, uiBytes32, &uiBytesWritten, nullptr)) || (uiBytesWritten != uiBytes32))
      return NS_FAILURE;
  }

  return NS_SUCCESS;
}

nsUInt64 nsOSFile::InternalRead(void* pBuffer, nsUInt64 uiBytes)
{
  nsUInt64 uiBytesRead = 0;

  const nsUInt32 uiBatchBytes = 1024 * 1024 * 1024; // 1 GB

  // first write out all the data in 1GB batches
  while (uiBytes > uiBatchBytes)
  {
    DWORD uiBytesReadThisTime = 0;
    if (!ReadFile(m_FileData.m_pFileHandle, pBuffer, uiBatchBytes, &uiBytesReadThisTime, nullptr))
      return uiBytesRead + uiBytesReadThisTime;

    uiBytesRead += uiBytesReadThisTime;

    if (uiBytesReadThisTime != uiBatchBytes)
      return uiBytesRead;

    uiBytes -= uiBatchBytes;
    pBuffer = nsMemoryUtils::AddByteOffset(pBuffer, uiBatchBytes);
  }

  if (uiBytes > 0)
  {
    const nsUInt32 uiBytes32 = static_cast<nsUInt32>(uiBytes);

    DWORD uiBytesReadThisTime = 0;
    if (!ReadFile(m_FileData.m_pFileHandle, pBuffer, uiBytes32, &uiBytesReadThisTime, nullptr))
      return uiBytesRead + uiBytesReadThisTime;

    uiBytesRead += uiBytesReadThisTime;
  }

  return uiBytesRead;
}

nsUInt64 nsOSFile::InternalGetFilePosition() const
{
  long int uiHigh32 = 0;
  nsUInt32 uiLow32 = SetFilePointer(m_FileData.m_pFileHandle, 0, &uiHigh32, FILE_CURRENT);

  return HighLowToUInt64(uiHigh32, uiLow32);
}

void nsOSFile::InternalSetFilePosition(nsInt64 iDistance, nsFileSeekMode::Enum Pos) const
{
  LARGE_INTEGER pos;
  LARGE_INTEGER newpos;
  pos.QuadPart = static_cast<LONGLONG>(iDistance);

  switch (Pos)
  {
    case nsFileSeekMode::FromStart:
      NS_VERIFY(SetFilePointerEx(m_FileData.m_pFileHandle, pos, &newpos, FILE_BEGIN), "Seek Failed.");
      break;
    case nsFileSeekMode::FromEnd:
      NS_VERIFY(SetFilePointerEx(m_FileData.m_pFileHandle, pos, &newpos, FILE_END), "Seek Failed.");
      break;
    case nsFileSeekMode::FromCurrent:
      NS_VERIFY(SetFilePointerEx(m_FileData.m_pFileHandle, pos, &newpos, FILE_CURRENT), "Seek Failed.");
      break;
  }
}

bool nsOSFile::InternalExistsFile(nsStringView sFile)
{
  const DWORD dwAttrib = GetFileAttributesW(nsDosDevicePath(sFile).GetData());

  return ((dwAttrib != INVALID_FILE_ATTRIBUTES) && ((dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == 0));
}

bool nsOSFile::InternalExistsDirectory(nsStringView sDirectory)
{
  const DWORD dwAttrib = GetFileAttributesW(nsDosDevicePath(sDirectory));

  return ((dwAttrib != INVALID_FILE_ATTRIBUTES) && ((dwAttrib & FILE_ATTRIBUTE_DIRECTORY) != 0));
}

nsResult nsOSFile::InternalDeleteFile(nsStringView sFile)
{
  if (DeleteFileW(nsDosDevicePath(sFile)) == FALSE)
  {
    DWORD error = GetLastError();
    if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND)
      return NS_SUCCESS;

    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

nsResult nsOSFile::InternalDeleteDirectory(nsStringView sDirectory)
{
  if (RemoveDirectoryW(nsDosDevicePath(sDirectory)) == FALSE)
  {
    DWORD error = GetLastError();
    if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND)
      return NS_SUCCESS;

    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

nsResult nsOSFile::InternalCreateDirectory(nsStringView sDirectory)
{
  // handle drive letters as always successful
  if (nsStringUtils::GetCharacterCount(sDirectory.GetStartPointer(), sDirectory.GetEndPointer()) <= 3) // 'C:\'
    return NS_SUCCESS;

  if (CreateDirectoryW(nsDosDevicePath(sDirectory), nullptr) == FALSE)
  {
    const DWORD uiError = GetLastError();
    if (uiError == ERROR_ALREADY_EXISTS)
      return NS_SUCCESS;

    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

nsResult nsOSFile::InternalMoveFileOrDirectory(nsStringView sDirectoryFrom, nsStringView sDirectoryTo)
{
  if (MoveFileW(nsDosDevicePath(sDirectoryFrom), nsDosDevicePath(sDirectoryTo)) == 0)
  {
    return NS_FAILURE;
  }
  return NS_SUCCESS;
}

#  endif // not NS_USE_POSIX_FILE_API

nsResult nsOSFile::InternalGetFileStats(nsStringView sFileOrFolder, nsFileStats& out_Stats)
{
  nsStringBuilder s = sFileOrFolder;

  // FindFirstFile does not like paths that end with a separator, so remove them all
  s.Trim(nullptr, "/\\");

  // handle the case that this query is done on the 'device part' of a path
  if (s.GetCharacterCount() <= 2) // 'C:', 'D:', 'E' etc.
  {
    s.ToUpper();

    out_Stats.m_uiFileSize = 0;
    out_Stats.m_bIsDirectory = true;
    out_Stats.m_sParentPath.Clear();
    out_Stats.m_sName = s;
    out_Stats.m_LastModificationTime = nsTimestamp::MakeInvalid();
    return NS_SUCCESS;
  }

  WIN32_FIND_DATAW data;
  HANDLE hSearch = FindFirstFileW(nsDosDevicePath(s), &data);

  if ((hSearch == nullptr) || (hSearch == INVALID_HANDLE_VALUE))
    return NS_FAILURE;

  out_Stats.m_uiFileSize = HighLowToUInt64(data.nFileSizeHigh, data.nFileSizeLow);
  out_Stats.m_bIsDirectory = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
  out_Stats.m_sParentPath = sFileOrFolder;
  out_Stats.m_sParentPath.PathParentDirectory();
  out_Stats.m_sName = data.cFileName;
  out_Stats.m_LastModificationTime = nsTimestamp::MakeFromInt(FileTimeToEpoch(data.ftLastWriteTime), nsSIUnitOfTime::Microsecond);

  FindClose(hSearch);
  return NS_SUCCESS;
}

#  if NS_ENABLED(NS_SUPPORTS_FILE_ITERATORS)

nsFileSystemIterator::nsFileSystemIterator() = default;

nsFileSystemIterator::~nsFileSystemIterator()
{
  while (!m_Data.m_Handles.IsEmpty())
  {
    FindClose(m_Data.m_Handles.PeekBack());
    m_Data.m_Handles.PopBack();
  }
}

bool nsFileSystemIterator::IsValid() const
{
  return !m_Data.m_Handles.IsEmpty();
}

void nsFileSystemIterator::StartSearch(nsStringView sSearchStart, nsBitflags<nsFileSystemIteratorFlags> flags /*= nsFileSystemIteratorFlags::All*/)
{
  NS_ASSERT_DEV(m_Data.m_Handles.IsEmpty(), "Cannot start another search.");

  m_sSearchTerm = sSearchStart;

  nsStringBuilder sSearch = sSearchStart;
  sSearch.MakeCleanPath();

  // same as just passing in the folder path, so remove this
  if (sSearch.EndsWith("/*"))
    sSearch.Shrink(0, 2);

  // The Windows documentation disallows trailing (back)slashes.
  sSearch.Trim(nullptr, "/");

  // Since the use of wildcard-ed file names will disable recursion, we ensure both are not used simultaneously.
  const bool bHasWildcard = sSearch.FindLastSubString("*") || sSearch.FindLastSubString("?");
  NS_ASSERT_DEV(flags.IsSet(nsFileSystemIteratorFlags::Recursive) == false || bHasWildcard == false, "Recursive file iteration does not support wildcards. Either don't use recursion, or filter the filenames manually.");

  m_sCurPath = sSearch.GetFileDirectory();

  NS_ASSERT_DEV(sSearch.IsAbsolutePath(), "The path '{0}' is not absolute.", m_sCurPath);

  m_Flags = flags;

  WIN32_FIND_DATAW data;
  HANDLE hSearch = FindFirstFileW(nsDosDevicePath(sSearch), &data);

  if ((hSearch == nullptr) || (hSearch == INVALID_HANDLE_VALUE))
    return;

  m_CurFile.m_uiFileSize = HighLowToUInt64(data.nFileSizeHigh, data.nFileSizeLow);
  m_CurFile.m_bIsDirectory = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
  m_CurFile.m_sParentPath = m_sCurPath;
  m_CurFile.m_sName = data.cFileName;
  m_CurFile.m_LastModificationTime = nsTimestamp::MakeFromInt(FileTimeToEpoch(data.ftLastWriteTime), nsSIUnitOfTime::Microsecond);

  m_Data.m_Handles.PushBack(hSearch);

  if (nsOSFile::ExistsDirectory(sSearch))
  {
    // when calling FindFirstFileW with a path to a folder (e.g. "C:/test") it will report "test" as the very first item
    // which is typically NOT what one wants, instead you want items INSIDE that folder to be reported
    // this is especially annoying when 'Recursion' is disabled, as "C:/test" would result in "C:/test" being reported
    // but no items inside it
    // therefore, when the start search points to a directory, we enable recursion for one call to 'Next', thus enter
    // the directory, and then switch it back again; all following calls to 'Next' will then iterate through the sub directory

    const bool bRecursive = m_Flags.IsSet(nsFileSystemIteratorFlags::Recursive);
    m_Flags.Add(nsFileSystemIteratorFlags::Recursive);

    Next();

    m_Flags.AddOrRemove(nsFileSystemIteratorFlags::Recursive, bRecursive);
    return;
  }

  if ((m_CurFile.m_sName == "..") || (m_CurFile.m_sName == "."))
  {
    Next(); // will search for the next file or folder that is not ".." or "." ; might return false though
    return;
  }

  if (m_CurFile.m_bIsDirectory)
  {
    if (!m_Flags.IsSet(nsFileSystemIteratorFlags::ReportFolders))
    {
      Next();
      return;
    }
  }
  else
  {
    if (!m_Flags.IsSet(nsFileSystemIteratorFlags::ReportFiles))
    {
      Next();
      return;
    }
  }
}

nsInt32 nsFileSystemIterator::InternalNext()
{
  constexpr nsInt32 ReturnFailure = 0;
  constexpr nsInt32 ReturnSuccess = 1;
  constexpr nsInt32 ReturnCallInternalNext = 2;

  if (m_Data.m_Handles.IsEmpty())
    return ReturnFailure;

  if (m_Flags.IsSet(nsFileSystemIteratorFlags::Recursive) && m_CurFile.m_bIsDirectory && (m_CurFile.m_sName != "..") && (m_CurFile.m_sName != "."))
  {
    m_sCurPath.AppendPath(m_CurFile.m_sName);

    nsStringBuilder sNewSearch = m_sCurPath;
    sNewSearch.AppendPath("*");

    WIN32_FIND_DATAW data;
    HANDLE hSearch = FindFirstFileW(nsDosDevicePath(sNewSearch), &data);

    if ((hSearch != nullptr) && (hSearch != INVALID_HANDLE_VALUE))
    {
      m_CurFile.m_uiFileSize = HighLowToUInt64(data.nFileSizeHigh, data.nFileSizeLow);
      m_CurFile.m_bIsDirectory = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
      m_CurFile.m_sParentPath = m_sCurPath;
      m_CurFile.m_sName = data.cFileName;
      m_CurFile.m_LastModificationTime = nsTimestamp::MakeFromInt(FileTimeToEpoch(data.ftLastWriteTime), nsSIUnitOfTime::Microsecond);

      m_Data.m_Handles.PushBack(hSearch);

      if ((m_CurFile.m_sName == "..") || (m_CurFile.m_sName == "."))
        return ReturnCallInternalNext; // will search for the next file or folder that is not ".." or "." ; might return false though

      if (m_CurFile.m_bIsDirectory)
      {
        if (!m_Flags.IsSet(nsFileSystemIteratorFlags::ReportFolders))
          return ReturnCallInternalNext;
      }
      else
      {
        if (!m_Flags.IsSet(nsFileSystemIteratorFlags::ReportFiles))
          return ReturnCallInternalNext;
      }

      return ReturnSuccess;
    }

    // if the recursion did not work, just iterate in this folder further
  }

  WIN32_FIND_DATAW data;
  if (!FindNextFileW(m_Data.m_Handles.PeekBack(), &data))
  {
    // nothing found in this directory anymore
    FindClose(m_Data.m_Handles.PeekBack());
    m_Data.m_Handles.PopBack();

    if (m_Data.m_Handles.IsEmpty())
      return ReturnFailure;

    m_sCurPath.PathParentDirectory();
    if (m_sCurPath.EndsWith("/"))
    {
      m_sCurPath.Shrink(0, 1); // Remove trailing /
    }

    return ReturnCallInternalNext;
  }

  m_CurFile.m_uiFileSize = HighLowToUInt64(data.nFileSizeHigh, data.nFileSizeLow);
  m_CurFile.m_bIsDirectory = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
  m_CurFile.m_sParentPath = m_sCurPath;
  m_CurFile.m_sName = data.cFileName;
  m_CurFile.m_LastModificationTime = nsTimestamp::MakeFromInt(FileTimeToEpoch(data.ftLastWriteTime), nsSIUnitOfTime::Microsecond);

  if ((m_CurFile.m_sName == "..") || (m_CurFile.m_sName == "."))
    return ReturnCallInternalNext;

  if (m_CurFile.m_bIsDirectory)
  {
    if (!m_Flags.IsSet(nsFileSystemIteratorFlags::ReportFolders))
      return ReturnCallInternalNext;
  }
  else
  {
    if (!m_Flags.IsSet(nsFileSystemIteratorFlags::ReportFiles))
      return ReturnCallInternalNext;
  }

  return ReturnSuccess;
}

#  endif

nsStringView nsOSFile::GetApplicationPath()
{
  if (s_sApplicationPath.IsEmpty())
  {
    nsUInt32 uiRequiredLength = 512;
    nsHybridArray<wchar_t, 1024> tmp;

    while (true)
    {
      tmp.SetCountUninitialized(uiRequiredLength);

      // reset last error code
      SetLastError(ERROR_SUCCESS);

      const nsUInt32 uiLength = GetModuleFileNameW(nullptr, tmp.GetData(), tmp.GetCount() - 1);
      const DWORD error = GetLastError();

      if (error == ERROR_SUCCESS)
      {
        tmp[uiLength] = L'\0';
        break;
      }

      if (error == ERROR_INSUFFICIENT_BUFFER)
      {
        uiRequiredLength += 512;
        continue;
      }

      NS_REPORT_FAILURE("GetModuleFileNameW failed: {0}", nsArgErrorCode(error));
    }

    s_sApplicationPath = nsStringUtf8(tmp.GetData()).GetData();
  }

  return s_sApplicationPath;
}

#  if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
#    include <Foundation/Basics/Platform/uwp/UWPUtils.h>
#    include <windows.storage.h>
#  endif

nsString nsOSFile::GetUserDataFolder(nsStringView sSubFolder)
{
  if (s_sUserDataPath.IsEmpty())
  {
#  if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
    ComPtr<ABI::Windows::Storage::IApplicationDataStatics> appDataStatics;
    if (SUCCEEDED(ABI::Windows::Foundation::GetActivationFactory(HStringReference(RuntimeClass_Windows_Storage_ApplicationData).Get(), &appDataStatics)))
    {
      ComPtr<ABI::Windows::Storage::IApplicationData> applicationData;
      if (SUCCEEDED(appDataStatics->get_Current(&applicationData)))
      {
        ComPtr<ABI::Windows::Storage::IStorageFolder> applicationDataLocal;
        if (SUCCEEDED(applicationData->get_LocalFolder(&applicationDataLocal)))
        {
          ComPtr<ABI::Windows::Storage::IStorageItem> localFolderItem;
          if (SUCCEEDED(applicationDataLocal.As(&localFolderItem)))
          {
            HSTRING path;
            localFolderItem->get_Path(&path);
            s_sUserDataPath = nsStringUtf8(path).GetData();
          }
        }
      }
    }
#  else
    wchar_t* pPath = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DEFAULT, nullptr, &pPath)))
    {
      s_sUserDataPath = nsStringWChar(pPath);
    }

    if (pPath != nullptr)
    {
      CoTaskMemFree(pPath);
    }
#  endif
  }

  nsStringBuilder s = s_sUserDataPath;
  s.AppendPath(sSubFolder);
  s.MakeCleanPath();
  return s;
}

nsString nsOSFile::GetTempDataFolder(nsStringView sSubFolder /*= nullptr*/)
{
  nsStringBuilder s;

  if (s_sTempDataPath.IsEmpty())
  {
#  if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
    ComPtr<ABI::Windows::Storage::IApplicationDataStatics> appDataStatics;
    if (SUCCEEDED(ABI::Windows::Foundation::GetActivationFactory(HStringReference(RuntimeClass_Windows_Storage_ApplicationData).Get(), &appDataStatics)))
    {
      ComPtr<ABI::Windows::Storage::IApplicationData> applicationData;
      if (SUCCEEDED(appDataStatics->get_Current(&applicationData)))
      {
        ComPtr<ABI::Windows::Storage::IStorageFolder> applicationTempData;
        if (SUCCEEDED(applicationData->get_TemporaryFolder(&applicationTempData)))
        {
          ComPtr<ABI::Windows::Storage::IStorageItem> tempFolderItem;
          if (SUCCEEDED(applicationTempData.As(&tempFolderItem)))
          {
            HSTRING path;
            tempFolderItem->get_Path(&path);
            s_sTempDataPath = nsStringUtf8(path).GetData();
          }
        }
      }
    }
#  else
    wchar_t* pPath = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT, nullptr, &pPath)))
    {
      s = nsStringWChar(pPath);
      s.AppendPath("Temp");
      s_sTempDataPath = s;
    }

    if (pPath != nullptr)
    {
      CoTaskMemFree(pPath);
    }
#  endif
  }

  s = s_sTempDataPath;
  s.AppendPath(sSubFolder);
  s.MakeCleanPath();
  return s;
}

nsString nsOSFile::GetUserDocumentsFolder(nsStringView sSubFolder /*= {}*/)
{
  if (s_sUserDocumentsPath.IsEmpty())
  {
#  if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
    NS_ASSERT_NOT_IMPLEMENTED;
#  else
    wchar_t* pPath = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_PublicDocuments, KF_FLAG_DEFAULT, nullptr, &pPath)))
    {
      s_sUserDocumentsPath = nsStringWChar(pPath);
    }

    if (pPath != nullptr)
    {
      CoTaskMemFree(pPath);
    }
#  endif
  }

  nsStringBuilder s = s_sUserDocumentsPath;
  s.AppendPath(sSubFolder);
  s.MakeCleanPath();
  return s;
}

const nsString nsOSFile::GetCurrentWorkingDirectory()
{
  const nsUInt32 uiRequiredLength = GetCurrentDirectoryW(0, nullptr);

  nsHybridArray<wchar_t, 1024> tmp;
  tmp.SetCountUninitialized(uiRequiredLength + 16);

  if (GetCurrentDirectoryW(tmp.GetCount() - 1, tmp.GetData()) == 0)
  {
    NS_REPORT_FAILURE("GetCurrentDirectoryW failed: {}", nsArgErrorCode(GetLastError()));
    return nsString();
  }

  tmp[uiRequiredLength] = L'\0';

  nsStringBuilder clean = nsStringUtf8(tmp.GetData()).GetData();
  clean.MakeCleanPath();

  return clean;
}

#endif

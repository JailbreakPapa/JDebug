#pragma once

#include <Foundation/FoundationPCH.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <Foundation/Logging/Log.h>
#include <Foundation/Threading/ThreadUtils.h>
#include <Foundation/Utilities/CommandLineUtils.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <direct.h>
#  define WD_USE_OLD_POSIX_FUNCTIONS WD_ON
#else
#  include <dirent.h>
#  include <fnmatch.h>
#  include <pwd.h>
#  include <sys/file.h>
#  include <sys/types.h>
#  include <unistd.h>
#  define WD_USE_OLD_POSIX_FUNCTIONS WD_OFF
#endif

#if WD_ENABLED(WD_PLATFORM_OSX)
#  include <CoreFoundation/CoreFoundation.h>
#endif

#if WD_ENABLED(WD_PLATFORM_ANDROID)
#  include <Foundation/Basics/Platform/Android/AndroidJni.h>
#  include <Foundation/Basics/Platform/Android/AndroidUtils.h>
#  include <android_native_app_glue.h>
#endif

#ifndef PATH_MAX
#  define PATH_MAX 1024
#endif

wdResult wdOSFile::InternalOpen(wdStringView sFile, wdFileOpenMode::Enum OpenMode, wdFileShareMode::Enum FileShareMode)
{
  wdStringBuilder sFileCopy = sFile;
  const char* szFile = sFileCopy;

#if WD_DISABLED(WD_PLATFORM_WINDOWS_UWP) // UWP does not support these functions
  int fd = -1;
  switch (OpenMode)
  {
    // O_CLOEXEC = don't forward to child processes
    case wdFileOpenMode::Read:
      fd = open(szFile, O_RDONLY | O_CLOEXEC);
      break;
    case wdFileOpenMode::Write:
    case wdFileOpenMode::Append:
      fd = open(szFile, O_CREAT | O_WRONLY | O_CLOEXEC, 0644);
      break;
    default:
      break;
  }

  if (FileShareMode == wdFileShareMode::Default)
  {
    if (OpenMode == wdFileOpenMode::Read)
    {
      FileShareMode = wdFileShareMode::SharedReads;
    }
    else
    {
      FileShareMode = wdFileShareMode::Exclusive;
    }
  }

  if (fd == -1)
  {
    return WD_FAILURE;
  }

  const int iSharedMode = (FileShareMode == wdFileShareMode::Exclusive) ? LOCK_EX : LOCK_SH;
  const wdTime sleepTime = wdTime::Milliseconds(20);
  wdInt32 iRetries = m_bRetryOnSharingViolation ? 20 : 1;

  while (flock(fd, iSharedMode | LOCK_NB /* do not block */) != 0)
  {
    int errorCode = errno;
    iRetries--;
    if (iRetries == 0 || errorCode != EWOULDBLOCK)
    {
      // error, could not get a lock
      wdLog::Error("Failed to get a {} lock for file {}, error {}", (FileShareMode == wdFileShareMode::Exclusive) ? "Exculsive" : "Shared", szFile, errno);
      close(fd);
      return WD_FAILURE;
    }
    wdThreadUtils::Sleep(sleepTime);
  }

  switch (OpenMode)
  {
    case wdFileOpenMode::Read:
      m_FileData.m_pFileHandle = fdopen(fd, "rb");
      break;
    case wdFileOpenMode::Write:
      if (ftruncate(fd, 0) < 0)
      {
        close(fd);
        return WD_FAILURE;
      }
      m_FileData.m_pFileHandle = fdopen(fd, "wb");
      break;
    case wdFileOpenMode::Append:
      m_FileData.m_pFileHandle = fdopen(fd, "ab");

      // in append mode we need to set the file pointer to the end explicitly, otherwise GetFilePosition might return 0 the first time
      if (m_FileData.m_pFileHandle != nullptr)
        InternalSetFilePosition(0, wdFileSeekMode::FromEnd);

      break;
    default:
      break;
  }

  if (m_FileData.m_pFileHandle == nullptr)
  {
    close(fd);
  }

#else

  switch (OpenMode)
  {
    case wdFileOpenMode::Read:
      m_FileData.m_pFileHandle = fopen(szFile, "rb");
      break;
    case wdFileOpenMode::Write:
      m_FileData.m_pFileHandle = fopen(szFile, "wb");
      break;
    case wdFileOpenMode::Append:
      m_FileData.m_pFileHandle = fopen(szFile, "ab");

      // in append mode we need to set the file pointer to the end explicitly, otherwise GetFilePosition might return 0 the first time
      if (m_FileData.m_pFileHandle != nullptr)
        InternalSetFilePosition(0, wdFileSeekMode::FromEnd);

      break;
    default:
      break;
  }
#endif

  if (m_FileData.m_pFileHandle == nullptr)
  {
    return WD_FAILURE;
  }

  // lock will be released automatically when the file is closed
  return WD_SUCCESS;
}

void wdOSFile::InternalClose()
{
  fclose(m_FileData.m_pFileHandle);
}

wdResult wdOSFile::InternalWrite(const void* pBuffer, wdUInt64 uiBytes)
{
  const wdUInt32 uiBatchBytes = 1024 * 1024 * 1024; // 1 GB

  // first write out all the data in 1GB batches
  while (uiBytes > uiBatchBytes)
  {
    if (fwrite(pBuffer, 1, uiBatchBytes, m_FileData.m_pFileHandle) != uiBatchBytes)
    {
      wdLog::Error("fwrite 1GB failed for '{}'", m_sFileName);
      return WD_FAILURE;
    }

    uiBytes -= uiBatchBytes;
    pBuffer = wdMemoryUtils::AddByteOffset(pBuffer, uiBatchBytes);
  }

  if (uiBytes > 0)
  {
    const wdUInt32 uiBytes32 = static_cast<wdUInt32>(uiBytes);

    if (fwrite(pBuffer, 1, uiBytes32, m_FileData.m_pFileHandle) != uiBytes)
    {
      wdLog::Error("fwrite failed for '{}'", m_sFileName);
      return WD_FAILURE;
    }
  }

  return WD_SUCCESS;
}

wdUInt64 wdOSFile::InternalRead(void* pBuffer, wdUInt64 uiBytes)
{
  wdUInt64 uiBytesRead = 0;

  const wdUInt32 uiBatchBytes = 1024 * 1024 * 1024; // 1 GB

  // first write out all the data in 1GB batches
  while (uiBytes > uiBatchBytes)
  {
    const wdUInt64 uiReadThisTime = fread(pBuffer, 1, uiBatchBytes, m_FileData.m_pFileHandle);
    uiBytesRead += uiReadThisTime;

    if (uiReadThisTime != uiBatchBytes)
      return uiBytesRead;

    uiBytes -= uiBatchBytes;
    pBuffer = wdMemoryUtils::AddByteOffset(pBuffer, uiBatchBytes);
  }

  if (uiBytes > 0)
  {
    const wdUInt32 uiBytes32 = static_cast<wdUInt32>(uiBytes);

    uiBytesRead += fread(pBuffer, 1, uiBytes32, m_FileData.m_pFileHandle);
  }

  return uiBytesRead;
}

wdUInt64 wdOSFile::InternalGetFilePosition() const
{
#if WD_ENABLED(WD_USE_OLD_POSIX_FUNCTIONS)
  return static_cast<wdUInt64>(ftell(m_FileData.m_pFileHandle));
#else
  return static_cast<wdUInt64>(ftello(m_FileData.m_pFileHandle));
#endif
}

void wdOSFile::InternalSetFilePosition(wdInt64 iDistance, wdFileSeekMode::Enum Pos) const
{
#if WD_ENABLED(WD_USE_OLD_POSIX_FUNCTIONS)
  switch (Pos)
  {
    case wdFileSeekMode::FromStart:
      WD_VERIFY(fseek(m_FileData.m_pFileHandle, (long)iDistance, SEEK_SET) == 0, "Seek Failed");
      break;
    case wdFileSeekMode::FromEnd:
      WD_VERIFY(fseek(m_FileData.m_pFileHandle, (long)iDistance, SEEK_END) == 0, "Seek Failed");
      break;
    case wdFileSeekMode::FromCurrent:
      WD_VERIFY(fseek(m_FileData.m_pFileHandle, (long)iDistance, SEEK_CUR) == 0, "Seek Failed");
      break;
  }
#else
  switch (Pos)
  {
    case wdFileSeekMode::FromStart:
      WD_VERIFY(fseeko(m_FileData.m_pFileHandle, iDistance, SEEK_SET) == 0, "Seek Failed");
      break;
    case wdFileSeekMode::FromEnd:
      WD_VERIFY(fseeko(m_FileData.m_pFileHandle, iDistance, SEEK_END) == 0, "Seek Failed");
      break;
    case wdFileSeekMode::FromCurrent:
      WD_VERIFY(fseeko(m_FileData.m_pFileHandle, iDistance, SEEK_CUR) == 0, "Seek Failed");
      break;
  }
#endif
}

bool wdOSFile::InternalExistsFile(wdStringView sFile)
{
  FILE* pFile = fopen(wdString(sFile), "r");

  if (pFile == nullptr)
    return false;

  fclose(pFile);
  return true;
}

// this might not be defined on Windows
#ifndef S_ISDIR
#  define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)
#endif

bool wdOSFile::InternalExistsDirectory(wdStringView sDirectory)
{
  struct stat sb;
  return (stat(wdString(sDirectory), &sb) == 0 && S_ISDIR(sb.st_mode));
}

wdResult wdOSFile::InternalDeleteFile(wdStringView sFile)
{
#if WD_ENABLED(WD_PLATFORM_WINDOWS)
  int iRes = _unlink(wdString(sFile));
#else
  int iRes = unlink(wdString(sFile));
#endif

  if (iRes == 0 || (iRes == -1 && errno == ENOENT))
    return WD_SUCCESS;

  return WD_FAILURE;
}

wdResult wdOSFile::InternalDeleteDirectory(wdStringView sDirectory)
{
#if WD_ENABLED(WD_PLATFORM_WINDOWS)
  int iRes = _rmdir(wdString(sDirectory));
#else
  int iRes = rmdir(wdString(sDirectory));
#endif

  if (iRes == 0 || (iRes == -1 && errno == ENOENT))
    return WD_SUCCESS;

  return WD_FAILURE;
}

wdResult wdOSFile::InternalCreateDirectory(wdStringView sDirectory)
{
  // handle drive letters as always successful
  if (wdStringUtils::GetCharacterCount(sDirectory.GetStartPointer(), sDirectory.GetEndPointer()) <= 1) // '/'
    return WD_SUCCESS;

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
  int iRes = _mkdir(wdString(sDirectory));
#else
  int iRes = mkdir(wdString(sDirectory), 0777);
#endif

  if (iRes == 0 || (iRes == -1 && errno == EEXIST))
    return WD_SUCCESS;

  // If we were not allowed to access the folder but it alreay exists, we treat the operation as successful.
  // Note that this is espcially relevant for calls to wdOSFile::CreateDirectoryStructure where we may call mkdir on top level directories that are
  // not accessible.
  if (errno == EACCES && InternalExistsDirectory(sDirectory))
    return WD_SUCCESS;

  return WD_FAILURE;
}

wdResult wdOSFile::InternalMoveFileOrDirectory(wdStringView sDirectoryFrom, wdStringView sDirectoryTo)
{
  if (rename(wdString(sDirectoryFrom), wdString(sDirectoryTo)) != 0)
  {
    return WD_FAILURE;
  }
  return WD_SUCCESS;
}

#if WD_ENABLED(WD_SUPPORTS_FILE_STATS) && WD_DISABLED(WD_PLATFORM_WINDOWS_UWP)
wdResult wdOSFile::InternalGetFileStats(wdStringView sFileOrFolder, wdFileStats& out_Stats)
{
  struct stat tempStat;
  int iRes = stat(wdString(sFileOrFolder), &tempStat);

  if (iRes != 0)
    return WD_FAILURE;

  out_Stats.m_bIsDirectory = S_ISDIR(tempStat.st_mode);
  out_Stats.m_uiFileSize = tempStat.st_size;
  out_Stats.m_sParentPath = sFileOrFolder;
  out_Stats.m_sParentPath.PathParentDirectory();
  out_Stats.m_sName = wdPathUtils::GetFileNameAndExtension(sFileOrFolder); // no OS support, so just pass it through
  out_Stats.m_LastModificationTime.SetInt64(tempStat.st_mtime, wdSIUnitOfTime::Second);

  return WD_SUCCESS;
}
#endif

#if WD_DISABLED(WD_PLATFORM_WINDOWS_UWP)

const char* wdOSFile::GetApplicationDirectory()
{
  static wdString256 s_Path;

  if (s_Path.IsEmpty())
  {
#  if WD_ENABLED(WD_PLATFORM_OSX)

    CFBundleRef appBundle = CFBundleGetMainBundle();
    CFURLRef bundleURL = CFBundleCopyBundleURL(appBundle);
    CFStringRef bundlePath = CFURLCopyFileSystemPath(bundleURL, kCFURLPOSIXPathStyle);

    if (bundlePath != nullptr)
    {
      CFIndex length = CFStringGetLength(bundlePath);
      CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;

      wdArrayPtr<char> temp = WD_DEFAULT_NEW_ARRAY(char, maxSize);

      if (CFStringGetCString(bundlePath, temp.GetPtr(), maxSize, kCFStringEncodingUTF8))
      {
        s_Path = temp.GetPtr();
      }

      WD_DEFAULT_DELETE_ARRAY(temp);
    }

    CFRelease(bundlePath);
    CFRelease(bundleURL);
    CFRelease(appBundle);
#  elif WD_ENABLED(WD_PLATFORM_ANDROID)
    {
      wdJniAttachment attachment;

      wdJniString packagePath = attachment.GetActivity().Call<wdJniString>("getPackageCodePath");
      // By convention, android requires assets to be placed in the 'Assets' folder
      // inside the apk thus we use that as our SDK root.
      wdStringBuilder sTemp = packagePath.GetData();
      sTemp.AppendPath("Assets");
      s_Path = sTemp;
    }
#  else
    char result[PATH_MAX];
    ssize_t length = readlink("/proc/self/exe", result, PATH_MAX);
    wdStringBuilder path(wdStringView(result, result + length));
    s_Path = path.GetFileDirectory();
#  endif
  }

  return s_Path.GetData();
}

wdString wdOSFile::GetUserDataFolder(wdStringView sSubFolder)
{
  if (s_sUserDataPath.IsEmpty())
  {
#  if WD_ENABLED(WD_PLATFORM_ANDROID)
    android_app* app = wdAndroidUtils::GetAndroidApp();
    s_sUserDataPath = app->activity->internalDataPath;
#  else
    s_sUserDataPath = getenv("HOME");

    if (s_sUserDataPath.IsEmpty())
      s_sUserDataPath = getpwuid(getuid())->pw_dir;
#  endif
  }

  wdStringBuilder s = s_sUserDataPath;
  s.AppendPath(sSubFolder);
  s.MakeCleanPath();
  return s;
}

wdString wdOSFile::GetTempDataFolder(wdStringView sSubFolder)
{
  if (s_sTempDataPath.IsEmpty())
  {
#  if WD_ENABLED(WD_PLATFORM_ANDROID)
    wdJniAttachment attachment;

    wdJniObject cacheDir = attachment.GetActivity().Call<wdJniObject>("getCacheDir");
    wdJniString path = cacheDir.Call<wdJniString>("getPath");
    s_sTempDataPath = path.GetData();
#  else
    s_sTempDataPath = GetUserDataFolder(".cache").GetData();
#  endif
  }

  wdStringBuilder s = s_sTempDataPath;
  s.AppendPath(sSubFolder);
  s.MakeCleanPath();
  return s;
}

const wdString wdOSFile::GetCurrentWorkingDirectory()
{
  char tmp[PATH_MAX];

  wdStringBuilder clean = getcwd(tmp, WD_ARRAY_SIZE(tmp));
  clean.MakeCleanPath();

  return clean;
}

#  if WD_ENABLED(WD_SUPPORTS_FILE_ITERATORS)

wdFileSystemIterator::wdFileSystemIterator() = default;

wdFileSystemIterator::~wdFileSystemIterator()
{
  while (!m_Data.m_Handles.IsEmpty())
  {
    closedir((DIR*)m_Data.m_Handles.PeekBack());
    m_Data.m_Handles.PopBack();
  }
}

bool wdFileSystemIterator::IsValid() const
{
  return !m_Data.m_Handles.IsEmpty();
}

namespace
{
  wdResult UpdateCurrentFile(wdFileStats& curFile, const wdStringBuilder& curPath, DIR* hSearch, const wdString& wildcardSearch)
  {
    struct dirent* hCurrentFile = readdir(hSearch);
    if (hCurrentFile == nullptr)
      return WD_FAILURE;

    if (!wildcardSearch.IsEmpty())
    {
      while (fnmatch(wildcardSearch.GetData(), hCurrentFile->d_name, FNM_NOESCAPE) != 0)
      {
        hCurrentFile = readdir(hSearch);
        if (hCurrentFile == nullptr)
          return WD_FAILURE;
      }
    }

    wdStringBuilder absFileName = curPath;
    absFileName.AppendPath(hCurrentFile->d_name);

    struct stat fileStat = {};
    stat(absFileName.GetData(), &fileStat);

    curFile.m_uiFileSize = fileStat.st_size;
    curFile.m_bIsDirectory = hCurrentFile->d_type == DT_DIR;
    curFile.m_sParentPath = curPath;
    curFile.m_sName = hCurrentFile->d_name;
    curFile.m_LastModificationTime.SetInt64(fileStat.st_mtime, wdSIUnitOfTime::Second);

    return WD_SUCCESS;
  }
} // namespace

void wdFileSystemIterator::StartSearch(wdStringView sSearchTerm, wdBitflags<wdFileSystemIteratorFlags> flags /*= wdFileSystemIteratorFlags::All*/)
{
  WD_ASSERT_DEV(m_Data.m_Handles.IsEmpty(), "Cannot start another search.");

  m_sSearchTerm = sSearchTerm;

  wdStringBuilder sSearch = sSearchTerm;
  sSearch.MakeCleanPath();

  // same as just passing in the folder path, so remove this
  if (sSearch.EndsWith("/*"))
    sSearch.Shrink(0, 2);

  // Remove a trailing slash if any
  sSearch.Trim(nullptr, "/");

  // Since the use of wildcard-ed file names will disable recursion, we ensure both are not used simultaneously.
  const bool bHasWildcard = sSearch.FindLastSubString("*") || sSearch.FindLastSubString("?");
  if (flags.IsSet(wdFileSystemIteratorFlags::Recursive) == true && bHasWildcard == true)
  {
    WD_ASSERT_DEV(false, "Recursive file iteration does not support wildcards. Either don't use recursion, or filter the filenames manually.");
    return;
  }

  if (bHasWildcard)
  {
    m_Data.m_wildcardSearch = sSearch.GetFileNameAndExtension();
    m_sCurPath = sSearch.GetFileDirectory();
  }
  else
  {
    m_Data.m_wildcardSearch.Clear();
    m_sCurPath = sSearch;
  }

  WD_ASSERT_DEV(m_sCurPath.IsAbsolutePath(), "The path '{0}' is not absolute.", m_sCurPath);

  m_Flags = flags;

  DIR* hSearch = opendir(m_sCurPath.GetData());

  if (hSearch == nullptr)
    return;

  if (UpdateCurrentFile(m_CurFile, m_sCurPath, hSearch, m_Data.m_wildcardSearch).Failed())
  {
    return;
  }

  m_Data.m_Handles.PushBack(hSearch);

  if ((m_CurFile.m_sName == "..") || (m_CurFile.m_sName == "."))
  {
    Next(); // will search for the next file or folder that is not ".." or "." ; might return false though
    return;
  }

  if (m_CurFile.m_bIsDirectory)
  {
    if (!m_Flags.IsSet(wdFileSystemIteratorFlags::ReportFolders))
    {
      Next();
      return;
    }
  }
  else
  {
    if (!m_Flags.IsSet(wdFileSystemIteratorFlags::ReportFiles))
    {
      Next();
      return;
    }
  }
}

wdInt32 wdFileSystemIterator::InternalNext()
{
  constexpr wdInt32 CallInternalNext = 2;

  if (m_Data.m_Handles.IsEmpty())
    return WD_FAILURE;

  if (m_Flags.IsSet(wdFileSystemIteratorFlags::Recursive) && m_CurFile.m_bIsDirectory && (m_CurFile.m_sName != "..") && (m_CurFile.m_sName != "."))
  {
    m_sCurPath.AppendPath(m_CurFile.m_sName.GetData());

    DIR* hSearch = opendir(m_sCurPath.GetData());

    if (hSearch != nullptr && UpdateCurrentFile(m_CurFile, m_sCurPath, hSearch, m_Data.m_wildcardSearch).Succeeded())
    {
      m_Data.m_Handles.PushBack(hSearch);

      if ((m_CurFile.m_sName == "..") || (m_CurFile.m_sName == "."))
        return CallInternalNext; // will search for the next file or folder that is not ".." or "." ; might return false though

      if (m_CurFile.m_bIsDirectory)
      {
        if (!m_Flags.IsSet(wdFileSystemIteratorFlags::ReportFolders))
          return CallInternalNext;
      }
      else
      {
        if (!m_Flags.IsSet(wdFileSystemIteratorFlags::ReportFiles))
          return CallInternalNext;
      }

      return WD_SUCCESS;
    }

    // if the recursion did not work, just iterate in this folder further
  }

  if (UpdateCurrentFile(m_CurFile, m_sCurPath, (DIR*)m_Data.m_Handles.PeekBack(), m_Data.m_wildcardSearch).Failed())
  {
    // nothing found in this directory anymore
    closedir((DIR*)m_Data.m_Handles.PeekBack());
    m_Data.m_Handles.PopBack();

    if (m_Data.m_Handles.IsEmpty())
      return WD_FAILURE;

    m_sCurPath.PathParentDirectory();
    if (m_sCurPath.GetElementCount() > 1 && m_sCurPath.EndsWith("/"))
    {
      m_sCurPath.Shrink(0, 1);
    }

    return CallInternalNext;
  }

  if ((m_CurFile.m_sName == "..") || (m_CurFile.m_sName == "."))
    return CallInternalNext;

  if (m_CurFile.m_bIsDirectory)
  {
    if (!m_Flags.IsSet(wdFileSystemIteratorFlags::ReportFolders))
      return CallInternalNext;
  }
  else
  {
    if (!m_Flags.IsSet(wdFileSystemIteratorFlags::ReportFiles))
      return CallInternalNext;
  }

  return WD_SUCCESS;
}

#  endif

#endif // WD_DISABLED(WD_PLATFORM_WINDOWS_UWP)

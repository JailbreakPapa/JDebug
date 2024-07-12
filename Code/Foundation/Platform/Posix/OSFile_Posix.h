#include <Foundation/FoundationPCH.h>
NS_FOUNDATION_INTERNAL_HEADER

#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Threading/ThreadUtils.h>
#include <Foundation/Utilities/CommandLineUtils.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
#  include <direct.h>
#  define NS_USE_OLD_POSIX_FUNCTIONS NS_ON
#else
#  include <dirent.h>
#  include <fnmatch.h>
#  include <pwd.h>
#  include <sys/file.h>
#  include <sys/types.h>
#  include <unistd.h>
#  define NS_USE_OLD_POSIX_FUNCTIONS NS_OFF
#endif

#if NS_ENABLED(NS_PLATFORM_OSX)
#  include <CoreFoundation/CoreFoundation.h>
#endif

#if NS_ENABLED(NS_PLATFORM_ANDROID)
#  include <Foundation/Basics/Platform/Android/AndroidJni.h>
#  include <Foundation/Basics/Platform/Android/AndroidUtils.h>
#  include <android_native_app_glue.h>
#endif

#ifndef PATH_MAX
#  define PATH_MAX 1024
#endif

nsResult nsOSFile::InternalOpen(nsStringView sFile, nsFileOpenMode::Enum OpenMode, nsFileShareMode::Enum FileShareMode)
{
  nsStringBuilder sFileCopy = sFile;
  const char* szFile = sFileCopy;

#if NS_DISABLED(NS_PLATFORM_WINDOWS_UWP) // UWP does not support these functions
  int fd = -1;
  switch (OpenMode)
  {
    // O_CLOEXEC = don't forward to child processes
    case nsFileOpenMode::Read:
      fd = open(szFile, O_RDONLY | O_CLOEXEC);
      break;
    case nsFileOpenMode::Write:
    case nsFileOpenMode::Append:
      fd = open(szFile, O_CREAT | O_WRONLY | O_CLOEXEC, 0644);
      break;
    default:
      break;
  }

  if (FileShareMode == nsFileShareMode::Default)
  {
    if (OpenMode == nsFileOpenMode::Read)
    {
      FileShareMode = nsFileShareMode::SharedReads;
    }
    else
    {
      FileShareMode = nsFileShareMode::Exclusive;
    }
  }

  if (fd == -1)
  {
    return NS_FAILURE;
  }

  const int iSharedMode = (FileShareMode == nsFileShareMode::Exclusive) ? LOCK_EX : LOCK_SH;
  const nsTime sleepTime = nsTime::MakeFromMilliseconds(20);
  nsInt32 iRetries = m_bRetryOnSharingViolation ? 20 : 1;

  while (flock(fd, iSharedMode | LOCK_NB /* do not block */) != 0)
  {
    int errorCode = errno;
    iRetries--;
    if (iRetries == 0 || errorCode != EWOULDBLOCK)
    {
      // error, could not get a lock
      nsLog::Error("Failed to get a {} lock for file {}, error {}", (FileShareMode == nsFileShareMode::Exclusive) ? "Exculsive" : "Shared", szFile, errno);
      close(fd);
      return NS_FAILURE;
    }
    nsThreadUtils::Sleep(sleepTime);
  }

  switch (OpenMode)
  {
    case nsFileOpenMode::Read:
      m_FileData.m_pFileHandle = fdopen(fd, "rb");
      break;
    case nsFileOpenMode::Write:
      if (ftruncate(fd, 0) < 0)
      {
        close(fd);
        return NS_FAILURE;
      }
      m_FileData.m_pFileHandle = fdopen(fd, "wb");
      break;
    case nsFileOpenMode::Append:
      m_FileData.m_pFileHandle = fdopen(fd, "ab");

      // in append mode we need to set the file pointer to the end explicitly, otherwise GetFilePosition might return 0 the first time
      if (m_FileData.m_pFileHandle != nullptr)
        InternalSetFilePosition(0, nsFileSeekMode::FromEnd);

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
    case nsFileOpenMode::Read:
      m_FileData.m_pFileHandle = fopen(szFile, "rb");
      break;
    case nsFileOpenMode::Write:
      m_FileData.m_pFileHandle = fopen(szFile, "wb");
      break;
    case nsFileOpenMode::Append:
      m_FileData.m_pFileHandle = fopen(szFile, "ab");

      // in append mode we need to set the file pointer to the end explicitly, otherwise GetFilePosition might return 0 the first time
      if (m_FileData.m_pFileHandle != nullptr)
        InternalSetFilePosition(0, nsFileSeekMode::FromEnd);

      break;
    default:
      break;
  }
#endif

  if (m_FileData.m_pFileHandle == nullptr)
  {
    return NS_FAILURE;
  }

  // lock will be released automatically when the file is closed
  return NS_SUCCESS;
}

void nsOSFile::InternalClose()
{
  fclose(m_FileData.m_pFileHandle);
}

nsResult nsOSFile::InternalWrite(const void* pBuffer, nsUInt64 uiBytes)
{
  const nsUInt32 uiBatchBytes = 1024 * 1024 * 1024; // 1 GB

  // first write out all the data in 1GB batches
  while (uiBytes > uiBatchBytes)
  {
    if (fwrite(pBuffer, 1, uiBatchBytes, m_FileData.m_pFileHandle) != uiBatchBytes)
    {
      nsLog::Error("fwrite 1GB failed for '{}'", m_sFileName);
      return NS_FAILURE;
    }

    uiBytes -= uiBatchBytes;
    pBuffer = nsMemoryUtils::AddByteOffset(pBuffer, uiBatchBytes);
  }

  if (uiBytes > 0)
  {
    const nsUInt32 uiBytes32 = static_cast<nsUInt32>(uiBytes);

    if (fwrite(pBuffer, 1, uiBytes32, m_FileData.m_pFileHandle) != uiBytes)
    {
      nsLog::Error("fwrite failed for '{}'", m_sFileName);
      return NS_FAILURE;
    }
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
    const nsUInt64 uiReadThisTime = fread(pBuffer, 1, uiBatchBytes, m_FileData.m_pFileHandle);
    uiBytesRead += uiReadThisTime;

    if (uiReadThisTime != uiBatchBytes)
      return uiBytesRead;

    uiBytes -= uiBatchBytes;
    pBuffer = nsMemoryUtils::AddByteOffset(pBuffer, uiBatchBytes);
  }

  if (uiBytes > 0)
  {
    const nsUInt32 uiBytes32 = static_cast<nsUInt32>(uiBytes);

    uiBytesRead += fread(pBuffer, 1, uiBytes32, m_FileData.m_pFileHandle);
  }

  return uiBytesRead;
}

nsUInt64 nsOSFile::InternalGetFilePosition() const
{
#if NS_ENABLED(NS_USE_OLD_POSIX_FUNCTIONS)
  return static_cast<nsUInt64>(ftell(m_FileData.m_pFileHandle));
#else
  return static_cast<nsUInt64>(ftello(m_FileData.m_pFileHandle));
#endif
}

void nsOSFile::InternalSetFilePosition(nsInt64 iDistance, nsFileSeekMode::Enum Pos) const
{
#if NS_ENABLED(NS_USE_OLD_POSIX_FUNCTIONS)
  switch (Pos)
  {
    case nsFileSeekMode::FromStart:
      NS_VERIFY(fseek(m_FileData.m_pFileHandle, (long)iDistance, SEEK_SET) == 0, "Seek Failed");
      break;
    case nsFileSeekMode::FromEnd:
      NS_VERIFY(fseek(m_FileData.m_pFileHandle, (long)iDistance, SEEK_END) == 0, "Seek Failed");
      break;
    case nsFileSeekMode::FromCurrent:
      NS_VERIFY(fseek(m_FileData.m_pFileHandle, (long)iDistance, SEEK_CUR) == 0, "Seek Failed");
      break;
  }
#else
  switch (Pos)
  {
    case nsFileSeekMode::FromStart:
      NS_VERIFY(fseeko(m_FileData.m_pFileHandle, iDistance, SEEK_SET) == 0, "Seek Failed");
      break;
    case nsFileSeekMode::FromEnd:
      NS_VERIFY(fseeko(m_FileData.m_pFileHandle, iDistance, SEEK_END) == 0, "Seek Failed");
      break;
    case nsFileSeekMode::FromCurrent:
      NS_VERIFY(fseeko(m_FileData.m_pFileHandle, iDistance, SEEK_CUR) == 0, "Seek Failed");
      break;
  }
#endif
}

// this might not be defined on Windows
#ifndef S_ISDIR
#  define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

bool nsOSFile::InternalExistsFile(nsStringView sFile)
{
  struct stat sb;
  return (stat(nsString(sFile), &sb) == 0 && !S_ISDIR(sb.st_mode));
}

bool nsOSFile::InternalExistsDirectory(nsStringView sDirectory)
{
  struct stat sb;
  return (stat(nsString(sDirectory), &sb) == 0 && S_ISDIR(sb.st_mode));
}

nsResult nsOSFile::InternalDeleteFile(nsStringView sFile)
{
#if NS_ENABLED(NS_PLATFORM_WINDOWS)
  int iRes = _unlink(nsString(sFile));
#else
  int iRes = unlink(nsString(sFile));
#endif

  if (iRes == 0 || (iRes == -1 && errno == ENOENT))
    return NS_SUCCESS;

  return NS_FAILURE;
}

nsResult nsOSFile::InternalDeleteDirectory(nsStringView sDirectory)
{
#if NS_ENABLED(NS_PLATFORM_WINDOWS)
  int iRes = _rmdir(nsString(sDirectory));
#else
  int iRes = rmdir(nsString(sDirectory));
#endif

  if (iRes == 0 || (iRes == -1 && errno == ENOENT))
    return NS_SUCCESS;

  return NS_FAILURE;
}

nsResult nsOSFile::InternalCreateDirectory(nsStringView sDirectory)
{
  // handle drive letters as always successful
  if (nsStringUtils::GetCharacterCount(sDirectory.GetStartPointer(), sDirectory.GetEndPointer()) <= 1) // '/'
    return NS_SUCCESS;

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
  int iRes = _mkdir(nsString(sDirectory));
#else
  int iRes = mkdir(nsString(sDirectory), 0777);
#endif

  if (iRes == 0 || (iRes == -1 && errno == EEXIST))
    return NS_SUCCESS;

  // If we were not allowed to access the folder but it alreay exists, we treat the operation as successful.
  // Note that this is espcially relevant for calls to nsOSFile::CreateDirectoryStructure where we may call mkdir on top level directories that are
  // not accessible.
  if (errno == EACCES && InternalExistsDirectory(sDirectory))
    return NS_SUCCESS;

  return NS_FAILURE;
}

nsResult nsOSFile::InternalMoveFileOrDirectory(nsStringView sDirectoryFrom, nsStringView sDirectoryTo)
{
  if (rename(nsString(sDirectoryFrom), nsString(sDirectoryTo)) != 0)
  {
    return NS_FAILURE;
  }
  return NS_SUCCESS;
}

#if NS_ENABLED(NS_SUPPORTS_FILE_STATS) && NS_DISABLED(NS_PLATFORM_WINDOWS_UWP)
nsResult nsOSFile::InternalGetFileStats(nsStringView sFileOrFolder, nsFileStats& out_Stats)
{
  struct stat tempStat;
  int iRes = stat(nsString(sFileOrFolder), &tempStat);

  if (iRes != 0)
    return NS_FAILURE;

  out_Stats.m_bIsDirectory = S_ISDIR(tempStat.st_mode);
  out_Stats.m_uiFileSize = tempStat.st_size;
  out_Stats.m_sParentPath = sFileOrFolder;
  out_Stats.m_sParentPath.PathParentDirectory();
  out_Stats.m_sName = nsPathUtils::GetFileNameAndExtension(sFileOrFolder); // no OS support, so just pass it through
  out_Stats.m_LastModificationTime = nsTimestamp::MakeFromInt(tempStat.st_mtime, nsSIUnitOfTime::Second);

  return NS_SUCCESS;
}
#endif

#if NS_DISABLED(NS_PLATFORM_WINDOWS_UWP)

nsStringView nsOSFile::GetApplicationPath()
{
  if (s_sApplicationPath.IsEmpty())
  {
#  if NS_ENABLED(NS_PLATFORM_OSX)

    CFBundleRef appBundle = CFBundleGetMainBundle();
    CFURLRef bundleURL = CFBundleCopyBundleURL(appBundle);
    CFStringRef bundlePath = CFURLCopyFileSystemPath(bundleURL, kCFURLPOSIXPathStyle);

    if (bundlePath != nullptr)
    {
      CFIndex length = CFStringGetLength(bundlePath);
      CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;

      nsArrayPtr<char> temp = NS_DEFAULT_NEW_ARRAY(char, static_cast<nsUInt32>(maxSize));

      if (CFStringGetCString(bundlePath, temp.GetPtr(), maxSize, kCFStringEncodingUTF8))
      {
        s_sApplicationPath = temp.GetPtr();
      }

      NS_DEFAULT_DELETE_ARRAY(temp);
    }

    CFRelease(bundlePath);
    CFRelease(bundleURL);
    CFRelease(appBundle);
#  elif NS_ENABLED(NS_PLATFORM_ANDROID)
    {
      nsJniAttachment attachment;

      nsJniString packagePath = attachment.GetActivity().Call<nsJniString>("getPackageCodePath");
      // By convention, android requires assets to be placed in the 'Assets' folder
      // inside the apk thus we use that as our SDK root.
      nsStringBuilder sTemp = packagePath.GetData();
      sTemp.AppendPath("Assets/nsDummyBin");
      s_sApplicationPath = sTemp;
    }
#  else
    char result[PATH_MAX];
    ssize_t length = readlink("/proc/self/exe", result, PATH_MAX);
    s_sApplicationPath = nsStringView(result, result + length);
#  endif
  }

  return s_sApplicationPath;
}

nsString nsOSFile::GetUserDataFolder(nsStringView sSubFolder)
{
  if (s_sUserDataPath.IsEmpty())
  {
#  if NS_ENABLED(NS_PLATFORM_ANDROID)
    android_app* app = nsAndroidUtils::GetAndroidApp();
    s_sUserDataPath = app->activity->internalDataPath;
#  else
    s_sUserDataPath = getenv("HOME");

    if (s_sUserDataPath.IsEmpty())
      s_sUserDataPath = getpwuid(getuid())->pw_dir;
#  endif
  }

  nsStringBuilder s = s_sUserDataPath;
  s.AppendPath(sSubFolder);
  s.MakeCleanPath();
  return s;
}

nsString nsOSFile::GetTempDataFolder(nsStringView sSubFolder)
{
  if (s_sTempDataPath.IsEmpty())
  {
#  if NS_ENABLED(NS_PLATFORM_ANDROID)
    nsJniAttachment attachment;

    nsJniObject cacheDir = attachment.GetActivity().Call<nsJniObject>("getCacheDir");
    nsJniString path = cacheDir.Call<nsJniString>("getPath");
    s_sTempDataPath = path.GetData();
#  else
    s_sTempDataPath = GetUserDataFolder(".cache").GetData();
#  endif
  }

  nsStringBuilder s = s_sTempDataPath;
  s.AppendPath(sSubFolder);
  s.MakeCleanPath();
  return s;
}

nsString nsOSFile::GetUserDocumentsFolder(nsStringView sSubFolder)
{
  if (s_sUserDocumentsPath.IsEmpty())
  {
#  if NS_ENABLED(NS_PLATFORM_ANDROID)
    NS_ASSERT_NOT_IMPLEMENTED;
#  else
    s_sUserDataPath = getenv("HOME");

    if (s_sUserDataPath.IsEmpty())
      s_sUserDataPath = getpwuid(getuid())->pw_dir;
#  endif
  }

  nsStringBuilder s = s_sUserDocumentsPath;
  s.AppendPath(sSubFolder);
  s.MakeCleanPath();
  return s;
}

const nsString nsOSFile::GetCurrentWorkingDirectory()
{
  char tmp[PATH_MAX];

  nsStringBuilder clean = getcwd(tmp, NS_ARRAY_SIZE(tmp));
  clean.MakeCleanPath();

  return clean;
}

#  if NS_ENABLED(NS_SUPPORTS_FILE_ITERATORS)

nsFileSystemIterator::nsFileSystemIterator() = default;

nsFileSystemIterator::~nsFileSystemIterator()
{
  while (!m_Data.m_Handles.IsEmpty())
  {
    closedir((DIR*)m_Data.m_Handles.PeekBack());
    m_Data.m_Handles.PopBack();
  }
}

bool nsFileSystemIterator::IsValid() const
{
  return !m_Data.m_Handles.IsEmpty();
}

namespace
{
  nsResult UpdateCurrentFile(nsFileStats& curFile, const nsStringBuilder& curPath, DIR* hSearch, const nsString& wildcardSearch)
  {
    struct dirent* hCurrentFile = readdir(hSearch);
    if (hCurrentFile == nullptr)
      return NS_FAILURE;

    if (!wildcardSearch.IsEmpty())
    {
      while (fnmatch(wildcardSearch.GetData(), hCurrentFile->d_name, FNM_NOESCAPE) != 0)
      {
        hCurrentFile = readdir(hSearch);
        if (hCurrentFile == nullptr)
          return NS_FAILURE;
      }
    }

    nsStringBuilder absFileName = curPath;
    absFileName.AppendPath(hCurrentFile->d_name);

    struct stat fileStat = {};
    stat(absFileName.GetData(), &fileStat);

    curFile.m_uiFileSize = fileStat.st_size;
    curFile.m_bIsDirectory = hCurrentFile->d_type == DT_DIR;
    curFile.m_sParentPath = curPath;
    curFile.m_sName = hCurrentFile->d_name;
    curFile.m_LastModificationTime = nsTimestamp::MakeFromInt(fileStat.st_mtime, nsSIUnitOfTime::Second);

    return NS_SUCCESS;
  }
} // namespace

void nsFileSystemIterator::StartSearch(nsStringView sSearchTerm, nsBitflags<nsFileSystemIteratorFlags> flags /*= nsFileSystemIteratorFlags::All*/)
{
  NS_ASSERT_DEV(m_Data.m_Handles.IsEmpty(), "Cannot start another search.");

  m_sSearchTerm = sSearchTerm;

  nsStringBuilder sSearch = sSearchTerm;
  sSearch.MakeCleanPath();

  // same as just passing in the folder path, so remove this
  if (sSearch.EndsWith("/*"))
    sSearch.Shrink(0, 2);

  // Remove a trailing slash if any
  sSearch.Trim(nullptr, "/");

  // Since the use of wildcard-ed file names will disable recursion, we ensure both are not used simultaneously.
  const bool bHasWildcard = sSearch.FindLastSubString("*") || sSearch.FindLastSubString("?");
  if (flags.IsSet(nsFileSystemIteratorFlags::Recursive) == true && bHasWildcard == true)
  {
    NS_ASSERT_DEV(false, "Recursive file iteration does not support wildcards. Either don't use recursion, or filter the filenames manually.");
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

  NS_ASSERT_DEV(m_sCurPath.IsAbsolutePath(), "The path '{0}' is not absolute.", m_sCurPath);

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
  constexpr nsInt32 CallInternalNext = 2;

  if (m_Data.m_Handles.IsEmpty())
    return NS_FAILURE;

  if (m_Flags.IsSet(nsFileSystemIteratorFlags::Recursive) && m_CurFile.m_bIsDirectory && (m_CurFile.m_sName != "..") && (m_CurFile.m_sName != "."))
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
        if (!m_Flags.IsSet(nsFileSystemIteratorFlags::ReportFolders))
          return CallInternalNext;
      }
      else
      {
        if (!m_Flags.IsSet(nsFileSystemIteratorFlags::ReportFiles))
          return CallInternalNext;
      }

      return NS_SUCCESS;
    }

    // if the recursion did not work, just iterate in this folder further
  }

  if (UpdateCurrentFile(m_CurFile, m_sCurPath, (DIR*)m_Data.m_Handles.PeekBack(), m_Data.m_wildcardSearch).Failed())
  {
    // nothing found in this directory anymore
    closedir((DIR*)m_Data.m_Handles.PeekBack());
    m_Data.m_Handles.PopBack();

    if (m_Data.m_Handles.IsEmpty())
      return NS_FAILURE;

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
    if (!m_Flags.IsSet(nsFileSystemIteratorFlags::ReportFolders))
      return CallInternalNext;
  }
  else
  {
    if (!m_Flags.IsSet(nsFileSystemIteratorFlags::ReportFiles))
      return CallInternalNext;
  }

  return NS_SUCCESS;
}

#  endif

#endif // NS_DISABLED(NS_PLATFORM_WINDOWS_UWP)

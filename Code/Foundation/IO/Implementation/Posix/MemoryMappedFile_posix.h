#include <Foundation/FoundationPCH.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <Foundation/IO/MemoryMappedFile.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/PathUtils.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#if WD_ENABLED(WD_PLATFORM_LINUX)
#  include <linux/version.h>
#endif

#if WD_ENABLED(WD_PLATFORM_OSX)
#  include <unistd.h>
#endif

struct wdMemoryMappedFileImpl
{
  wdMemoryMappedFile::Mode m_Mode = wdMemoryMappedFile::Mode::None;
  void* m_pMappedFilePtr = nullptr;
  wdUInt64 m_uiFileSize = 0;
  int m_hFile = -1;
  wdString m_sSharedMemoryName;

  ~wdMemoryMappedFileImpl()
  {
    if (m_pMappedFilePtr != nullptr)
    {
      munmap(m_pMappedFilePtr, m_uiFileSize);
      m_pMappedFilePtr = nullptr;
    }
    if (m_hFile != -1)
    {
      close(m_hFile);
      m_hFile = -1;
    }
#if WD_ENABLED(WD_PLATFORM_ANDROID)
    // shm_open / shm_unlink deprecated.
    // There is an alternative in ASharedMemory_create but that is only
    // available in API 26 upwards.
#else
    if (!m_sSharedMemoryName.IsEmpty())
    {
      shm_unlink(m_sSharedMemoryName);
      m_sSharedMemoryName.Clear();
    }
#endif
    m_uiFileSize = 0;
  }
};

wdMemoryMappedFile::wdMemoryMappedFile()
{
  m_pImpl = WD_DEFAULT_NEW(wdMemoryMappedFileImpl);
}

wdMemoryMappedFile::~wdMemoryMappedFile()
{
  Close();
}

#if WD_ENABLED(WD_SUPPORTS_MEMORY_MAPPED_FILE)
wdResult wdMemoryMappedFile::Open(wdStringView sAbsolutePath, Mode mode)
{
  WD_ASSERT_DEV(mode != Mode::None, "Invalid mode to open the memory mapped file");
  WD_ASSERT_DEV(wdPathUtils::IsAbsolutePath(sAbsolutePath), "wdMemoryMappedFile::Open() can only be used with absolute file paths");

  WD_LOG_BLOCK("MemoryMapFile", sAbsolutePath);

  const wdStringBuilder sPath = sAbsolutePath;

  Close();

  m_pImpl->m_Mode = mode;

  int access = O_RDONLY;
  int prot = PROT_READ;
  int flags = MAP_PRIVATE;
  if (mode == Mode::ReadWrite)
  {
    access = O_RDWR;
    prot |= PROT_WRITE;
    flags = MAP_SHARED;
  }
#  if WD_ENABLED(WD_PLATFORM_LINUX)
#    if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 22)
  flags |= MAP_POPULATE;
#    endif
#  endif
  m_pImpl->m_hFile = open(sPath, access | O_CLOEXEC, 0);
  if (m_pImpl->m_hFile == -1)
  {
    wdLog::Error("Could not open file for memory mapping - {}", strerror(errno));
    Close();
    return WD_FAILURE;
  }
  struct stat sb;
  if (stat(sPath, &sb) == -1 || sb.st_size == 0)
  {
    wdLog::Error("File for memory mapping is empty - {}", strerror(errno));
    Close();
    return WD_FAILURE;
  }
  m_pImpl->m_uiFileSize = sb.st_size;

  m_pImpl->m_pMappedFilePtr = mmap(nullptr, m_pImpl->m_uiFileSize, prot, flags, m_pImpl->m_hFile, 0);
  if (m_pImpl->m_pMappedFilePtr == nullptr)
  {
    wdLog::Error("Could not create memory mapping of file - {}", strerror(errno));
    Close();
    return WD_FAILURE;
  }

  return WD_SUCCESS;
}
#endif

#if WD_ENABLED(WD_SUPPORTS_SHARED_MEMORY)
wdResult wdMemoryMappedFile::OpenShared(wdStringView sSharedName, wdUInt64 uiSize, Mode mode)
{
  WD_ASSERT_DEV(mode != Mode::None, "Invalid mode to open the memory mapped file");
  WD_ASSERT_DEV(uiSize > 0, "wdMemoryMappedFile::OpenShared() needs a valid file size to map");

  const wdStringBuilder sName = sSharedName;

  WD_LOG_BLOCK("MemoryMapFile", sName);

  Close();

  m_pImpl->m_Mode = mode;

  int prot = PROT_READ;
  int oflag = O_RDONLY;
  int flags = MAP_SHARED;
#  if WD_ENABLED(WD_PLATFORM_LINUX)
#    if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 22)
  flags |= MAP_POPULATE;
#    endif
#  endif

  if (mode == Mode::ReadWrite)
  {
    oflag = O_RDWR;
    prot |= PROT_WRITE;
  }
  oflag |= O_CREAT;

  m_pImpl->m_hFile = shm_open(sName, oflag, 0666);
  if (m_pImpl->m_hFile == -1)
  {
    wdLog::Error("Could not open shared memory mapping - {}", strerror(errno));
    Close();
    return WD_FAILURE;
  }
  m_pImpl->m_sSharedMemoryName = sName;

  if (ftruncate(m_pImpl->m_hFile, uiSize) == -1)
  {
    wdLog::Error("Could not open shared memory mapping - {}", strerror(errno));
    Close();
    return WD_FAILURE;
  }
  m_pImpl->m_uiFileSize = uiSize;

  m_pImpl->m_pMappedFilePtr = mmap(nullptr, m_pImpl->m_uiFileSize, prot, flags, m_pImpl->m_hFile, 0);
  if (m_pImpl->m_pMappedFilePtr == nullptr)
  {
    wdLog::Error("Could not create memory mapping of file - {}", strerror(errno));
    Close();
    return WD_FAILURE;
  }
  return WD_SUCCESS;
}
#endif

void wdMemoryMappedFile::Close()
{
  m_pImpl = WD_DEFAULT_NEW(wdMemoryMappedFileImpl);
}

wdMemoryMappedFile::Mode wdMemoryMappedFile::GetMode() const
{
  return m_pImpl->m_Mode;
}

const void* wdMemoryMappedFile::GetReadPointer(wdUInt64 uiOffset /*= 0*/, OffsetBase base /*= OffsetBase::Start*/) const
{
  WD_ASSERT_DEBUG(m_pImpl->m_Mode >= Mode::ReadOnly, "File must be opened with read access before accessing it for reading.");
  WD_ASSERT_DEBUG(uiOffset <= m_pImpl->m_uiFileSize, "Read offset must be smaller than mapped file size");

  if (base == OffsetBase::Start)
  {
    return wdMemoryUtils::AddByteOffset(m_pImpl->m_pMappedFilePtr, uiOffset);
  }
  else
  {
    return wdMemoryUtils::AddByteOffset(m_pImpl->m_pMappedFilePtr, m_pImpl->m_uiFileSize - uiOffset);
  }
}

void* wdMemoryMappedFile::GetWritePointer(wdUInt64 uiOffset /*= 0*/, OffsetBase base /*= OffsetBase::Start*/)
{
  WD_ASSERT_DEBUG(m_pImpl->m_Mode >= Mode::ReadWrite, "File must be opened with read/write access before accessing it for writing.");
  WD_ASSERT_DEBUG(uiOffset <= m_pImpl->m_uiFileSize, "Read offset must be smaller than mapped file size");

  if (base == OffsetBase::Start)
  {
    return wdMemoryUtils::AddByteOffset(m_pImpl->m_pMappedFilePtr, uiOffset);
  }
  else
  {
    return wdMemoryUtils::AddByteOffset(m_pImpl->m_pMappedFilePtr, m_pImpl->m_uiFileSize - uiOffset);
  }
}

wdUInt64 wdMemoryMappedFile::GetFileSize() const
{
  return m_pImpl->m_uiFileSize;
}

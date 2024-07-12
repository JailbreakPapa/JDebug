#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)

#  include <Foundation/IO/MemoryMappedFile.h>
#  include <Foundation/Logging/Log.h>
#  include <Foundation/Platform/Win/DosDevicePath_Win.h>
#  include <Foundation/Strings/PathUtils.h>
#  include <Foundation/Strings/StringConversion.h>

struct nsMemoryMappedFileImpl
{
  nsMemoryMappedFile::Mode m_Mode = nsMemoryMappedFile::Mode::None;
  void* m_pMappedFilePtr = nullptr;
  nsUInt64 m_uiFileSize = 0;
  HANDLE m_hFile = INVALID_HANDLE_VALUE;
  HANDLE m_hMapping = INVALID_HANDLE_VALUE;

  ~nsMemoryMappedFileImpl()
  {
    if (m_pMappedFilePtr != nullptr)
    {
      UnmapViewOfFile(m_pMappedFilePtr);
      m_pMappedFilePtr = nullptr;
    }

    if (m_hMapping != INVALID_HANDLE_VALUE)
    {
      CloseHandle(m_hMapping);
      m_hMapping = INVALID_HANDLE_VALUE;
    }

    if (m_hFile != INVALID_HANDLE_VALUE)
    {
      CloseHandle(m_hFile);
      m_hFile = INVALID_HANDLE_VALUE;
    }
  }
};

nsMemoryMappedFile::nsMemoryMappedFile()
{
  m_pImpl = NS_DEFAULT_NEW(nsMemoryMappedFileImpl);
}

nsMemoryMappedFile::~nsMemoryMappedFile()
{
  Close();
}

nsResult nsMemoryMappedFile::Open(nsStringView sAbsolutePath, Mode mode)
{
  NS_ASSERT_DEV(mode != Mode::None, "Invalid mode to open the memory mapped file");
  NS_ASSERT_DEV(nsPathUtils::IsAbsolutePath(sAbsolutePath), "nsMemoryMappedFile::Open() can only be used with absolute file paths");

  NS_LOG_BLOCK("MemoryMapFile", sAbsolutePath);


  Close();

  m_pImpl->m_Mode = mode;

  DWORD access = GENERIC_READ;

  if (mode == Mode::ReadWrite)
  {
    access |= GENERIC_WRITE;
  }

  m_pImpl->m_hFile = CreateFileW(nsDosDevicePath(sAbsolutePath), access, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

  DWORD errorCode = GetLastError();

  if (m_pImpl->m_hFile == nullptr || m_pImpl->m_hFile == INVALID_HANDLE_VALUE)
  {
    nsLog::Error("Could not open file for memory mapping - {}", nsArgErrorCode(errorCode));
    Close();
    return NS_FAILURE;
  }

  if (GetFileSizeEx(m_pImpl->m_hFile, reinterpret_cast<LARGE_INTEGER*>(&m_pImpl->m_uiFileSize)) == FALSE || m_pImpl->m_uiFileSize == 0)
  {
    nsLog::Error("File for memory mapping is empty");
    Close();
    return NS_FAILURE;
  }

  m_pImpl->m_hMapping = CreateFileMappingW(m_pImpl->m_hFile, nullptr, m_pImpl->m_Mode == Mode::ReadOnly ? PAGE_READONLY : PAGE_READWRITE, 0, 0, nullptr);

  if (m_pImpl->m_hMapping == nullptr || m_pImpl->m_hMapping == INVALID_HANDLE_VALUE)
  {
    errorCode = GetLastError();

    nsLog::Error("Could not create memory mapping of file - {}", nsArgErrorCode(errorCode));
    Close();
    return NS_FAILURE;
  }

  m_pImpl->m_pMappedFilePtr = MapViewOfFile(m_pImpl->m_hMapping, mode == Mode::ReadOnly ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, 0);

  if (m_pImpl->m_pMappedFilePtr == nullptr)
  {
    errorCode = GetLastError();

    nsLog::Error("Could not create memory mapping view of file - {}", nsArgErrorCode(errorCode));
    Close();
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

nsResult nsMemoryMappedFile::OpenShared(nsStringView sSharedName, nsUInt64 uiSize, Mode mode)
{
  NS_ASSERT_DEV(mode != Mode::None, "Invalid mode to open the memory mapped file");
  NS_ASSERT_DEV(uiSize > 0, "nsMemoryMappedFile::OpenShared() needs a valid file size to map");

  NS_LOG_BLOCK("MemoryMapFile", sSharedName);

  Close();

  m_pImpl->m_Mode = mode;

  DWORD errorCode = 0;
  DWORD sizeHigh = static_cast<DWORD>((uiSize >> 32) & 0xFFFFFFFFu);
  DWORD sizeLow = static_cast<DWORD>(uiSize & 0xFFFFFFFFu);

  m_pImpl->m_hMapping = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, m_pImpl->m_Mode == Mode::ReadOnly ? PAGE_READONLY : PAGE_READWRITE, sizeHigh,
    sizeLow, nsStringWChar(sSharedName).GetData());

  if (m_pImpl->m_hMapping == nullptr || m_pImpl->m_hMapping == INVALID_HANDLE_VALUE)
  {
    errorCode = GetLastError();

    nsLog::Error("Could not create memory mapping of file - {}", nsArgErrorCode(errorCode));
    Close();
    return NS_FAILURE;
  }

  m_pImpl->m_pMappedFilePtr = MapViewOfFile(m_pImpl->m_hMapping, mode == Mode::ReadOnly ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, 0);

  if (m_pImpl->m_pMappedFilePtr == nullptr)
  {
    errorCode = GetLastError();

    nsLog::Error("Could not create memory mapping view of file - {}", nsArgErrorCode(errorCode));
    Close();
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

void nsMemoryMappedFile::Close()
{
  m_pImpl = NS_DEFAULT_NEW(nsMemoryMappedFileImpl);
}

nsMemoryMappedFile::Mode nsMemoryMappedFile::GetMode() const
{
  return m_pImpl->m_Mode;
}

const void* nsMemoryMappedFile::GetReadPointer(nsUInt64 uiOffset /*= 0*/, OffsetBase base /*= OffsetBase::Start*/) const
{
  NS_ASSERT_DEBUG(m_pImpl->m_Mode >= Mode::ReadOnly, "File must be opened with read access before accessing it for reading.");
  NS_ASSERT_DEBUG(uiOffset <= m_pImpl->m_uiFileSize, "Read offset must be smaller than mapped file size");

  if (base == OffsetBase::Start)
  {
    return nsMemoryUtils::AddByteOffset(m_pImpl->m_pMappedFilePtr, static_cast<std::ptrdiff_t>(uiOffset));
  }
  else
  {
    return nsMemoryUtils::AddByteOffset(m_pImpl->m_pMappedFilePtr, static_cast<std::ptrdiff_t>(m_pImpl->m_uiFileSize - uiOffset));
  }
}

void* nsMemoryMappedFile::GetWritePointer(nsUInt64 uiOffset /*= 0*/, OffsetBase base /*= OffsetBase::Start*/)
{
  NS_ASSERT_DEBUG(m_pImpl->m_Mode >= Mode::ReadWrite, "File must be opened with read/write access before accessing it for writing.");
  NS_ASSERT_DEBUG(uiOffset <= m_pImpl->m_uiFileSize, "Read offset must be smaller than mapped file size");

  if (base == OffsetBase::Start)
  {
    return nsMemoryUtils::AddByteOffset(m_pImpl->m_pMappedFilePtr, static_cast<std::ptrdiff_t>(uiOffset));
  }
  else
  {
    return nsMemoryUtils::AddByteOffset(m_pImpl->m_pMappedFilePtr, static_cast<std::ptrdiff_t>(m_pImpl->m_uiFileSize - uiOffset));
  }
}

nsUInt64 nsMemoryMappedFile::GetFileSize() const
{
  return m_pImpl->m_uiFileSize;
}

#endif

#include <Foundation/IO/Implementation/Win/DosDevicePath_win.h>
#include <Foundation/IO/MemoryMappedFile.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/PathUtils.h>
#include <Foundation/Strings/StringConversion.h>

struct wdMemoryMappedFileImpl
{
  wdMemoryMappedFile::Mode m_Mode = wdMemoryMappedFile::Mode::None;
  void* m_pMappedFilePtr = nullptr;
  wdUInt64 m_uiFileSize = 0;
  HANDLE m_hFile = INVALID_HANDLE_VALUE;
  HANDLE m_hMapping = INVALID_HANDLE_VALUE;

  ~wdMemoryMappedFileImpl()
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

wdMemoryMappedFile::wdMemoryMappedFile()
{
  m_pImpl = WD_DEFAULT_NEW(wdMemoryMappedFileImpl);
}

wdMemoryMappedFile::~wdMemoryMappedFile()
{
  Close();
}

wdResult wdMemoryMappedFile::Open(wdStringView sAbsolutePath, Mode mode)
{
  WD_ASSERT_DEV(mode != Mode::None, "Invalid mode to open the memory mapped file");
  WD_ASSERT_DEV(wdPathUtils::IsAbsolutePath(sAbsolutePath), "wdMemoryMappedFile::Open() can only be used with absolute file paths");

  WD_LOG_BLOCK("MemoryMapFile", sAbsolutePath);


  Close();

  m_pImpl->m_Mode = mode;

  DWORD access = GENERIC_READ;

  if (mode == Mode::ReadWrite)
  {
    access |= GENERIC_WRITE;
  }

  m_pImpl->m_hFile = CreateFileW(wdDosDevicePath(sAbsolutePath), access, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

  DWORD errorCode = GetLastError();

  if (m_pImpl->m_hFile == nullptr || m_pImpl->m_hFile == INVALID_HANDLE_VALUE)
  {
    wdLog::Error("Could not open file for memory mapping - {}", wdArgErrorCode(errorCode));
    Close();
    return WD_FAILURE;
  }

  if (GetFileSizeEx(m_pImpl->m_hFile, reinterpret_cast<LARGE_INTEGER*>(&m_pImpl->m_uiFileSize)) == FALSE || m_pImpl->m_uiFileSize == 0)
  {
    wdLog::Error("File for memory mapping is empty");
    Close();
    return WD_FAILURE;
  }

  m_pImpl->m_hMapping = CreateFileMappingW(m_pImpl->m_hFile, nullptr, m_pImpl->m_Mode == Mode::ReadOnly ? PAGE_READONLY : PAGE_READWRITE, 0, 0, nullptr);

  if (m_pImpl->m_hMapping == nullptr || m_pImpl->m_hMapping == INVALID_HANDLE_VALUE)
  {
    errorCode = GetLastError();

    wdLog::Error("Could not create memory mapping of file - {}", wdArgErrorCode(errorCode));
    Close();
    return WD_FAILURE;
  }

  m_pImpl->m_pMappedFilePtr = MapViewOfFile(m_pImpl->m_hMapping, mode == Mode::ReadOnly ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, 0);

  if (m_pImpl->m_pMappedFilePtr == nullptr)
  {
    errorCode = GetLastError();

    wdLog::Error("Could not create memory mapping view of file - {}", wdArgErrorCode(errorCode));
    Close();
    return WD_FAILURE;
  }

  return WD_SUCCESS;
}

wdResult wdMemoryMappedFile::OpenShared(wdStringView sSharedName, wdUInt64 uiSize, Mode mode)
{
  WD_ASSERT_DEV(mode != Mode::None, "Invalid mode to open the memory mapped file");
  WD_ASSERT_DEV(uiSize > 0, "wdMemoryMappedFile::OpenShared() needs a valid file size to map");

  WD_LOG_BLOCK("MemoryMapFile", sSharedName);

  Close();

  m_pImpl->m_Mode = mode;

  DWORD errorCode = 0;
  DWORD sizeHigh = static_cast<DWORD>((uiSize >> 32) & 0xFFFFFFFFu);
  DWORD sizeLow = static_cast<DWORD>(uiSize & 0xFFFFFFFFu);

  m_pImpl->m_hMapping = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, m_pImpl->m_Mode == Mode::ReadOnly ? PAGE_READONLY : PAGE_READWRITE, sizeHigh,
    sizeLow, wdStringWChar(sSharedName).GetData());

  if (m_pImpl->m_hMapping == nullptr || m_pImpl->m_hMapping == INVALID_HANDLE_VALUE)
  {
    errorCode = GetLastError();

    wdLog::Error("Could not create memory mapping of file - {}", wdArgErrorCode(errorCode));
    Close();
    return WD_FAILURE;
  }

  m_pImpl->m_pMappedFilePtr = MapViewOfFile(m_pImpl->m_hMapping, mode == Mode::ReadOnly ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, 0);

  if (m_pImpl->m_pMappedFilePtr == nullptr)
  {
    errorCode = GetLastError();

    wdLog::Error("Could not create memory mapping view of file - {}", wdArgErrorCode(errorCode));
    Close();
    return WD_FAILURE;
  }

  return WD_SUCCESS;
}

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
    return wdMemoryUtils::AddByteOffset(m_pImpl->m_pMappedFilePtr, static_cast<ptrdiff_t>(uiOffset));
  }
  else
  {
    return wdMemoryUtils::AddByteOffset(m_pImpl->m_pMappedFilePtr, static_cast<ptrdiff_t>(m_pImpl->m_uiFileSize - uiOffset));
  }
}

void* wdMemoryMappedFile::GetWritePointer(wdUInt64 uiOffset /*= 0*/, OffsetBase base /*= OffsetBase::Start*/)
{
  WD_ASSERT_DEBUG(m_pImpl->m_Mode >= Mode::ReadWrite, "File must be opened with read/write access before accessing it for writing.");
  WD_ASSERT_DEBUG(uiOffset <= m_pImpl->m_uiFileSize, "Read offset must be smaller than mapped file size");

  if (base == OffsetBase::Start)
  {
    return wdMemoryUtils::AddByteOffset(m_pImpl->m_pMappedFilePtr, static_cast<ptrdiff_t>(uiOffset));
  }
  else
  {
    return wdMemoryUtils::AddByteOffset(m_pImpl->m_pMappedFilePtr, static_cast<ptrdiff_t>(m_pImpl->m_uiFileSize - uiOffset));
  }
}

wdUInt64 wdMemoryMappedFile::GetFileSize() const
{
  return m_pImpl->m_uiFileSize;
}

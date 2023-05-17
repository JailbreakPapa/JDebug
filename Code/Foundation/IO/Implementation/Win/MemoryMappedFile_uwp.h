#include <Foundation/IO/MemoryMappedFile.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/PathUtils.h>

///#TODO: Implement. Under some restrictions, UWP supports
/// CreateFileMappingFromApp, OpenFileMappingFromApp, MapViewOfFileFromApp
/// Needs adding codeGeneration capability to the app manifest.

struct wdMemoryMappedFileImpl
{
  wdMemoryMappedFile::Mode m_Mode = wdMemoryMappedFile::Mode::None;
  void* m_pMappedFilePtr = nullptr;
  wdUInt64 m_uiFileSize = 0;

  ~wdMemoryMappedFileImpl() {}
};

wdMemoryMappedFile::wdMemoryMappedFile()
{
  m_pImpl = WD_DEFAULT_NEW(wdMemoryMappedFileImpl);
}

wdMemoryMappedFile::~wdMemoryMappedFile()
{
  Close();
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
  return m_pImpl->m_pMappedFilePtr;
}

void* wdMemoryMappedFile::GetWritePointer(wdUInt64 uiOffset /*= 0*/, OffsetBase base /*= OffsetBase::Start*/)
{
  WD_ASSERT_DEBUG(m_pImpl->m_Mode >= Mode::ReadWrite, "File must be opened with read/write access before accessing it for writing.");
  return m_pImpl->m_pMappedFilePtr;
}

wdUInt64 wdMemoryMappedFile::GetFileSize() const
{
  return m_pImpl->m_uiFileSize;
}

#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/FileSystem/FileWriter.h>

wdResult wdFileWriter::Open(wdStringView sFile, wdUInt32 uiCacheSize /*= 1024 * 1024*/, wdFileShareMode::Enum fileShareMode /*= wdFileShareMode::Exclusive*/, bool bAllowFileEvents /*= true*/)
{
  uiCacheSize = wdMath::Clamp<wdUInt32>(uiCacheSize, 1024, 1024 * 1024 * 32);

  m_pDataDirWriter = GetFileWriter(sFile, fileShareMode, bAllowFileEvents);

  if (!m_pDataDirWriter)
    return WD_FAILURE;

  m_Cache.SetCountUninitialized(uiCacheSize);

  m_uiCacheWritePosition = 0;

  return WD_SUCCESS;
}

void wdFileWriter::Close()
{
  if (!m_pDataDirWriter)
    return;

  Flush().IgnoreResult();

  m_pDataDirWriter->Close();
  m_pDataDirWriter = nullptr;
}

wdResult wdFileWriter::Flush()
{
  const wdResult res = m_pDataDirWriter->Write(&m_Cache[0], m_uiCacheWritePosition);
  m_uiCacheWritePosition = 0;

  return res;
}

wdResult wdFileWriter::WriteBytes(const void* pWriteBuffer, wdUInt64 uiBytesToWrite)
{
  WD_ASSERT_DEV(m_pDataDirWriter != nullptr, "The file has not been opened (successfully).");

  if (uiBytesToWrite > m_Cache.GetCount())
  {
    // if there is more incoming data than what our cache can hold, there is no point in storing a copy
    // instead we can just pass the entire data through right away

    if (m_uiCacheWritePosition > 0)
    {
      WD_SUCCEED_OR_RETURN(Flush());
    }

    return m_pDataDirWriter->Write(pWriteBuffer, uiBytesToWrite);
  }
  else
  {
    wdUInt8* pBuffer = (wdUInt8*)pWriteBuffer;

    while (uiBytesToWrite > 0)
    {
      // determine chunk size to be written
      wdUInt64 uiChunkSize = uiBytesToWrite;

      const wdUInt64 uiRemainingCache = m_Cache.GetCount() - m_uiCacheWritePosition;

      if (uiRemainingCache < uiBytesToWrite)
        uiChunkSize = uiRemainingCache;

      // copy memory
      wdMemoryUtils::Copy(&m_Cache[(wdUInt32)m_uiCacheWritePosition], pBuffer, (wdUInt32)uiChunkSize);

      pBuffer += uiChunkSize;
      m_uiCacheWritePosition += uiChunkSize;
      uiBytesToWrite -= uiChunkSize;

      // if the cache is full or nearly full, flush it to disk
      if (m_uiCacheWritePosition + 32 >= m_Cache.GetCount())
      {
        if (Flush() == WD_FAILURE)
          return WD_FAILURE;
      }
    }

    return WD_SUCCESS;
  }
}



WD_STATICLINK_FILE(Foundation, Foundation_IO_FileSystem_Implementation_FileWriter);

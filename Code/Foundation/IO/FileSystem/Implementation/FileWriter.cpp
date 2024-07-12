#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/FileSystem/FileWriter.h>

nsResult nsFileWriter::Open(nsStringView sFile, nsUInt32 uiCacheSize /*= 1024 * 1024*/, nsFileShareMode::Enum fileShareMode /*= nsFileShareMode::Exclusive*/, bool bAllowFileEvents /*= true*/)
{
  uiCacheSize = nsMath::Clamp<nsUInt32>(uiCacheSize, 1024, 1024 * 1024 * 32);

  m_pDataDirWriter = GetFileWriter(sFile, fileShareMode, bAllowFileEvents);

  if (!m_pDataDirWriter)
    return NS_FAILURE;

  m_Cache.SetCountUninitialized(uiCacheSize);

  m_uiCacheWritePosition = 0;

  return NS_SUCCESS;
}

void nsFileWriter::Close()
{
  if (!m_pDataDirWriter)
    return;

  Flush().IgnoreResult();

  m_pDataDirWriter->Close();
  m_pDataDirWriter = nullptr;
}

nsResult nsFileWriter::Flush()
{
  const nsResult res = m_pDataDirWriter->Write(&m_Cache[0], m_uiCacheWritePosition);
  m_uiCacheWritePosition = 0;

  return res;
}

nsResult nsFileWriter::WriteBytes(const void* pWriteBuffer, nsUInt64 uiBytesToWrite)
{
  NS_ASSERT_DEV(m_pDataDirWriter != nullptr, "The file has not been opened (successfully).");

  if (uiBytesToWrite > m_Cache.GetCount())
  {
    // if there is more incoming data than what our cache can hold, there is no point in storing a copy
    // instead we can just pass the entire data through right away

    if (m_uiCacheWritePosition > 0)
    {
      NS_SUCCEED_OR_RETURN(Flush());
    }

    return m_pDataDirWriter->Write(pWriteBuffer, uiBytesToWrite);
  }
  else
  {
    nsUInt8* pBuffer = (nsUInt8*)pWriteBuffer;

    while (uiBytesToWrite > 0)
    {
      // determine chunk size to be written
      nsUInt64 uiChunkSize = uiBytesToWrite;

      const nsUInt64 uiRemainingCache = m_Cache.GetCount() - m_uiCacheWritePosition;

      if (uiRemainingCache < uiBytesToWrite)
        uiChunkSize = uiRemainingCache;

      // copy memory
      nsMemoryUtils::Copy(&m_Cache[(nsUInt32)m_uiCacheWritePosition], pBuffer, (nsUInt32)uiChunkSize);

      pBuffer += uiChunkSize;
      m_uiCacheWritePosition += uiChunkSize;
      uiBytesToWrite -= uiChunkSize;

      // if the cache is full or nearly full, flush it to disk
      if (m_uiCacheWritePosition + 32 >= m_Cache.GetCount())
      {
        if (Flush() == NS_FAILURE)
          return NS_FAILURE;
      }
    }

    return NS_SUCCESS;
  }
}

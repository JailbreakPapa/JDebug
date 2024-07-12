#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/FileSystem/FileReader.h>

nsResult nsFileReader::Open(nsStringView sFile, nsUInt32 uiCacheSize /*= 1024 * 64*/,
  nsFileShareMode::Enum fileShareMode /*= nsFileShareMode::SharedReads*/, bool bAllowFileEvents /*= true*/)
{
  NS_ASSERT_DEV(m_pDataDirReader == nullptr, "The file reader is already open. (File: '{0}')", sFile);

  uiCacheSize = nsMath::Min<nsUInt32>(uiCacheSize, 1024 * 1024 * 32);

  m_pDataDirReader = GetFileReader(sFile, fileShareMode, bAllowFileEvents);

  if (!m_pDataDirReader)
    return NS_FAILURE;

  m_Cache.SetCountUninitialized(uiCacheSize);

  m_uiCacheReadPosition = 0;
  m_uiBytesCached = 0;
  m_bEOF = false;

  return NS_SUCCESS;
}

void nsFileReader::Close()
{
  if (m_pDataDirReader)
    m_pDataDirReader->Close();

  m_pDataDirReader = nullptr;
  m_bEOF = true;
}

nsUInt64 nsFileReader::SkipBytes(nsUInt64 uiBytesToSkip)
{
  NS_ASSERT_DEV(m_pDataDirReader != nullptr, "The file has not been opened (successfully).");
  if (m_bEOF)
    return 0;

  nsUInt64 uiSkipPosition = 0; // how much was skipped, yet

  // if any data is still in the cache, skip that first
  {
    const nsUInt64 uiCachedBytesLeft = m_uiBytesCached - m_uiCacheReadPosition;
    const nsUInt64 uiBytesSkippedFromCache = nsMath::Min(uiCachedBytesLeft, uiBytesToSkip);
    uiSkipPosition += uiBytesSkippedFromCache;
    m_uiCacheReadPosition += uiBytesSkippedFromCache;
    uiBytesToSkip -= uiBytesSkippedFromCache;
  }

  // skip bytes on disk
  const nsUInt64 uiBytesMeantToSkipFromDisk = uiBytesToSkip;
  const nsUInt64 uiBytesSkippedFromDisk = m_pDataDirReader->Skip(uiBytesToSkip);
  uiSkipPosition += uiBytesSkippedFromDisk;
  uiBytesToSkip -= uiBytesSkippedFromDisk;

  // mark end of file if suitable
  const bool endOfCacheReached = m_uiCacheReadPosition == m_uiBytesCached;
  const bool endOfDiskDataReached = uiBytesSkippedFromDisk < uiBytesMeantToSkipFromDisk;
  if (endOfCacheReached && endOfDiskDataReached)
  {
    m_bEOF = true;
  }

  return uiSkipPosition;
}

nsUInt64 nsFileReader::ReadBytes(void* pReadBuffer, nsUInt64 uiBytesToRead)
{
  NS_ASSERT_DEV(m_pDataDirReader != nullptr, "The file has not been opened (successfully).");
  if (m_bEOF)
    return 0;

  nsUInt64 uiBufferPosition = 0; // how much was read, yet
  nsUInt8* pBuffer = (nsUInt8*)pReadBuffer;

  if (uiBytesToRead > m_Cache.GetCount())
  {
    // if any data is still in the cache, use that first
    const nsUInt64 uiCachedBytesLeft = m_uiBytesCached - m_uiCacheReadPosition;
    if (uiCachedBytesLeft > 0)
    {
      nsMemoryUtils::Copy(&pBuffer[uiBufferPosition], &m_Cache[(nsUInt32)m_uiCacheReadPosition], (nsUInt32)uiCachedBytesLeft);
      uiBufferPosition += uiCachedBytesLeft;
      m_uiCacheReadPosition += uiCachedBytesLeft;
      uiBytesToRead -= uiCachedBytesLeft;
    }

    // read remaining data from disk
    nsUInt64 uiBytesReadFromDisk = 0;
    if (uiBytesToRead > 0)
    {
      uiBytesReadFromDisk = m_pDataDirReader->Read(&pBuffer[uiBufferPosition], uiBytesToRead);
      uiBufferPosition += uiBytesReadFromDisk;
    }

    // mark eof if we're already there
    if (uiBytesReadFromDisk == 0)
    {
      m_bEOF = true;
      return uiBufferPosition;
    }
  }
  else
  {
    while (uiBytesToRead > 0)
    {
      // determine the chunk size to read
      nsUInt64 uiChunkSize = uiBytesToRead;

      const nsUInt64 uiCachedBytesLeft = m_uiBytesCached - m_uiCacheReadPosition;
      if (uiCachedBytesLeft < uiBytesToRead)
      {
        uiChunkSize = uiCachedBytesLeft;
      }

      // copy data into the buffer
      // uiChunkSize can never be larger than the cache size, which is limited to 32 Bit
      if (uiChunkSize > 0)
      {
        nsMemoryUtils::Copy(&pBuffer[uiBufferPosition], &m_Cache[(nsUInt32)m_uiCacheReadPosition], (nsUInt32)uiChunkSize);

        // store how much was read and how much is still left to read
        uiBufferPosition += uiChunkSize;
        m_uiCacheReadPosition += uiChunkSize;
        uiBytesToRead -= uiChunkSize;
      }

      // if the cache is depleted, refill it
      // this will even be triggered if EXACTLY the amount of available bytes was read
      if (m_uiCacheReadPosition >= m_uiBytesCached)
      {
        m_uiBytesCached = m_pDataDirReader->Read(&m_Cache[0], m_Cache.GetCount());
        m_uiCacheReadPosition = 0;

        // if nothing else could be read from the file, return the number of bytes that have been read
        if (m_uiBytesCached == 0)
        {
          // if absolutely nothing could be read, we reached the end of the file (and we actually returned everything else,
          // so the file was really read to the end).
          m_bEOF = true;
          return uiBufferPosition;
        }
      }
    }
  }

  // return how much was read
  return uiBufferPosition;
}

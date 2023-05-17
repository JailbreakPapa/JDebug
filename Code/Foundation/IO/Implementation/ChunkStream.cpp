#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/ChunkStream.h>

wdChunkStreamWriter::wdChunkStreamWriter(wdStreamWriter& inout_stream)
  : m_Stream(inout_stream)
{
  m_bWritingFile = false;
  m_bWritingChunk = false;
}

void wdChunkStreamWriter::BeginStream(wdUInt16 uiVersion)
{
  WD_ASSERT_DEV(!m_bWritingFile, "Already writing the file.");
  WD_ASSERT_DEV(uiVersion > 0, "The version number must be larger than 0");

  m_bWritingFile = true;

  const char* szTag = "BGNCHNK2";
  m_Stream.WriteBytes(szTag, 8).IgnoreResult();
  m_Stream.WriteBytes(&uiVersion, 2).IgnoreResult();
}

void wdChunkStreamWriter::EndStream()
{
  WD_ASSERT_DEV(m_bWritingFile, "Not writing to the file.");
  WD_ASSERT_DEV(!m_bWritingChunk, "A chunk is still open for writing: '{0}'", m_sChunkName);

  m_bWritingFile = false;

  const char* szTag = "END CHNK";
  m_Stream.WriteBytes(szTag, 8).IgnoreResult();
}

void wdChunkStreamWriter::BeginChunk(wdStringView sName, wdUInt32 uiVersion)
{
  WD_ASSERT_DEV(m_bWritingFile, "Not writing to the file.");
  WD_ASSERT_DEV(!m_bWritingChunk, "A chunk is already open for writing: '{0}'", m_sChunkName);

  m_sChunkName = sName;

  const char* szTag = "NXT CHNK";
  m_Stream.WriteBytes(szTag, 8).IgnoreResult();

  m_Stream << m_sChunkName;
  m_Stream << uiVersion;

  m_bWritingChunk = true;
}


void wdChunkStreamWriter::EndChunk()
{
  WD_ASSERT_DEV(m_bWritingFile, "Not writing to the file.");
  WD_ASSERT_DEV(m_bWritingChunk, "No chunk is currently open.");

  m_bWritingChunk = false;

  const wdUInt32 uiStorageSize = m_Storage.GetCount();
  m_Stream << uiStorageSize;
  /// \todo Write Chunk CRC

  for (wdUInt32 i = 0; i < uiStorageSize;)
  {
    const wdUInt32 uiRange = m_Storage.GetContiguousRange(i);

    WD_ASSERT_DEBUG(uiRange > 0, "Invalid contiguous range");

    m_Stream.WriteBytes(&m_Storage[i], uiRange).IgnoreResult();
    i += uiRange;
  }

  m_Storage.Clear();
}

wdResult wdChunkStreamWriter::WriteBytes(const void* pWriteBuffer, wdUInt64 uiBytesToWrite)
{
  WD_ASSERT_DEV(m_bWritingChunk, "No chunk is currently written to");

  const wdUInt8* pBytes = (const wdUInt8*)pWriteBuffer;

  for (wdUInt64 i = 0; i < uiBytesToWrite; ++i)
    m_Storage.PushBack(pBytes[i]);

  return WD_SUCCESS;
}



wdChunkStreamReader::wdChunkStreamReader(wdStreamReader& inout_stream)
  : m_Stream(inout_stream)
{
  m_ChunkInfo.m_bValid = false;
  m_EndChunkFileMode = EndChunkFileMode::JustClose;
}

wdUInt64 wdChunkStreamReader::ReadBytes(void* pReadBuffer, wdUInt64 uiBytesToRead)
{
  WD_ASSERT_DEV(m_ChunkInfo.m_bValid, "No valid chunk available.");

  uiBytesToRead = wdMath::Min<wdUInt64>(uiBytesToRead, m_ChunkInfo.m_uiUnreadChunkBytes);
  m_ChunkInfo.m_uiUnreadChunkBytes -= (wdUInt32)uiBytesToRead;

  return m_Stream.ReadBytes(pReadBuffer, uiBytesToRead);
}

wdUInt16 wdChunkStreamReader::BeginStream()
{
  m_ChunkInfo.m_bValid = false;

  char szTag[9];
  m_Stream.ReadBytes(szTag, 8);
  szTag[8] = '\0';

  wdUInt16 uiVersion = 0;

  if (wdStringUtils::IsEqual(szTag, "BGNCHNK2"))
  {
    m_Stream.ReadBytes(&uiVersion, 2);
  }
  else
  {
    // "BGN CHNK" is the old chunk identifier, before a version number was written
    WD_ASSERT_DEV(wdStringUtils::IsEqual(szTag, "BGN CHNK"), "Not a valid chunk file.");
  }

  TryReadChunkHeader();
  return uiVersion;
}

void wdChunkStreamReader::EndStream()
{
  if (m_EndChunkFileMode == EndChunkFileMode::SkipToEnd)
  {
    while (m_ChunkInfo.m_bValid)
      NextChunk();
  }
}

void wdChunkStreamReader::TryReadChunkHeader()
{
  m_ChunkInfo.m_bValid = false;

  char szTag[9];
  m_Stream.ReadBytes(szTag, 8);
  szTag[8] = '\0';

  if (wdStringUtils::IsEqual(szTag, "END CHNK"))
    return;

  if (wdStringUtils::IsEqual(szTag, "NXT CHNK"))
  {
    m_Stream >> m_ChunkInfo.m_sChunkName;
    m_Stream >> m_ChunkInfo.m_uiChunkVersion;
    m_Stream >> m_ChunkInfo.m_uiChunkBytes;
    m_ChunkInfo.m_uiUnreadChunkBytes = m_ChunkInfo.m_uiChunkBytes;

    m_ChunkInfo.m_bValid = true;

    return;
  }

  WD_REPORT_FAILURE("Invalid chunk file, tag is '{0}'", szTag);
}

void wdChunkStreamReader::NextChunk()
{
  if (!m_ChunkInfo.m_bValid)
    return;

  const wdUInt64 uiToSkip = m_ChunkInfo.m_uiUnreadChunkBytes;
  const wdUInt64 uiSkipped = SkipBytes(uiToSkip);
  WD_VERIFY(uiSkipped == uiToSkip, "Corrupt chunk '{0}' (version {1}), tried to skip {2} bytes, could only read {3} bytes", m_ChunkInfo.m_sChunkName, m_ChunkInfo.m_uiChunkVersion, uiToSkip, uiSkipped);

  TryReadChunkHeader();
}



WD_STATICLINK_FILE(Foundation, Foundation_IO_Implementation_ChunkStream);

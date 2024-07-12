#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/ChunkStream.h>

nsChunkStreamWriter::nsChunkStreamWriter(nsStreamWriter& inout_stream)
  : m_Stream(inout_stream)
{
  m_bWritingFile = false;
  m_bWritingChunk = false;
}

void nsChunkStreamWriter::BeginStream(nsUInt16 uiVersion)
{
  NS_ASSERT_DEV(!m_bWritingFile, "Already writing the file.");
  NS_ASSERT_DEV(uiVersion > 0, "The version number must be larger than 0");

  m_bWritingFile = true;

  const char* szTag = "BGNCHNK2";
  m_Stream.WriteBytes(szTag, 8).IgnoreResult();
  m_Stream.WriteBytes(&uiVersion, 2).IgnoreResult();
}

void nsChunkStreamWriter::EndStream()
{
  NS_ASSERT_DEV(m_bWritingFile, "Not writing to the file.");
  NS_ASSERT_DEV(!m_bWritingChunk, "A chunk is still open for writing: '{0}'", m_sChunkName);

  m_bWritingFile = false;

  const char* szTag = "END CHNK";
  m_Stream.WriteBytes(szTag, 8).IgnoreResult();
}

void nsChunkStreamWriter::BeginChunk(nsStringView sName, nsUInt32 uiVersion)
{
  NS_ASSERT_DEV(m_bWritingFile, "Not writing to the file.");
  NS_ASSERT_DEV(!m_bWritingChunk, "A chunk is already open for writing: '{0}'", m_sChunkName);

  m_sChunkName = sName;

  const char* szTag = "NXT CHNK";
  m_Stream.WriteBytes(szTag, 8).IgnoreResult();

  m_Stream << m_sChunkName;
  m_Stream << uiVersion;

  m_bWritingChunk = true;
}


void nsChunkStreamWriter::EndChunk()
{
  NS_ASSERT_DEV(m_bWritingFile, "Not writing to the file.");
  NS_ASSERT_DEV(m_bWritingChunk, "No chunk is currently open.");

  m_bWritingChunk = false;

  const nsUInt32 uiStorageSize = m_Storage.GetCount();
  m_Stream << uiStorageSize;
  /// \todo Write Chunk CRC

  for (nsUInt32 i = 0; i < uiStorageSize;)
  {
    const nsUInt32 uiRange = m_Storage.GetContiguousRange(i);

    NS_ASSERT_DEBUG(uiRange > 0, "Invalid contiguous range");

    m_Stream.WriteBytes(&m_Storage[i], uiRange).IgnoreResult();
    i += uiRange;
  }

  m_Storage.Clear();
}

nsResult nsChunkStreamWriter::WriteBytes(const void* pWriteBuffer, nsUInt64 uiBytesToWrite)
{
  NS_ASSERT_DEV(m_bWritingChunk, "No chunk is currently written to");

  const nsUInt8* pBytes = (const nsUInt8*)pWriteBuffer;

  for (nsUInt64 i = 0; i < uiBytesToWrite; ++i)
    m_Storage.PushBack(pBytes[i]);

  return NS_SUCCESS;
}



nsChunkStreamReader::nsChunkStreamReader(nsStreamReader& inout_stream)
  : m_Stream(inout_stream)
{
  m_ChunkInfo.m_bValid = false;
  m_EndChunkFileMode = EndChunkFileMode::JustClose;
}

nsUInt64 nsChunkStreamReader::ReadBytes(void* pReadBuffer, nsUInt64 uiBytesToRead)
{
  NS_ASSERT_DEV(m_ChunkInfo.m_bValid, "No valid chunk available.");

  uiBytesToRead = nsMath::Min<nsUInt64>(uiBytesToRead, m_ChunkInfo.m_uiUnreadChunkBytes);
  m_ChunkInfo.m_uiUnreadChunkBytes -= (nsUInt32)uiBytesToRead;

  return m_Stream.ReadBytes(pReadBuffer, uiBytesToRead);
}

nsUInt16 nsChunkStreamReader::BeginStream()
{
  m_ChunkInfo.m_bValid = false;

  char szTag[9];
  m_Stream.ReadBytes(szTag, 8);
  szTag[8] = '\0';

  nsUInt16 uiVersion = 0;

  if (nsStringUtils::IsEqual(szTag, "BGNCHNK2"))
  {
    m_Stream.ReadBytes(&uiVersion, 2);
  }
  else
  {
    // "BGN CHNK" is the old chunk identifier, before a version number was written
    NS_ASSERT_DEV(nsStringUtils::IsEqual(szTag, "BGN CHNK"), "Not a valid chunk file.");
  }

  TryReadChunkHeader();
  return uiVersion;
}

void nsChunkStreamReader::EndStream()
{
  if (m_EndChunkFileMode == EndChunkFileMode::SkipToEnd)
  {
    while (m_ChunkInfo.m_bValid)
      NextChunk();
  }
}

void nsChunkStreamReader::TryReadChunkHeader()
{
  m_ChunkInfo.m_bValid = false;

  char szTag[9];
  m_Stream.ReadBytes(szTag, 8);
  szTag[8] = '\0';

  if (nsStringUtils::IsEqual(szTag, "END CHNK"))
    return;

  if (nsStringUtils::IsEqual(szTag, "NXT CHNK"))
  {
    m_Stream >> m_ChunkInfo.m_sChunkName;
    m_Stream >> m_ChunkInfo.m_uiChunkVersion;
    m_Stream >> m_ChunkInfo.m_uiChunkBytes;
    m_ChunkInfo.m_uiUnreadChunkBytes = m_ChunkInfo.m_uiChunkBytes;

    m_ChunkInfo.m_bValid = true;

    return;
  }

  NS_REPORT_FAILURE("Invalid chunk file, tag is '{0}'", szTag);
}

void nsChunkStreamReader::NextChunk()
{
  if (!m_ChunkInfo.m_bValid)
    return;

  const nsUInt64 uiToSkip = m_ChunkInfo.m_uiUnreadChunkBytes;
  const nsUInt64 uiSkipped = SkipBytes(uiToSkip);
  NS_VERIFY(uiSkipped == uiToSkip, "Corrupt chunk '{0}' (version {1}), tried to skip {2} bytes, could only read {3} bytes", m_ChunkInfo.m_sChunkName, m_ChunkInfo.m_uiChunkVersion, uiToSkip, uiSkipped);

  TryReadChunkHeader();
}

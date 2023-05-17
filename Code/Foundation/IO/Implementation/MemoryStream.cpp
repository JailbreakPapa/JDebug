#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/MemoryStream.h>

// Reader implementation

wdMemoryStreamReader::wdMemoryStreamReader(const wdMemoryStreamStorageInterface* pStreamStorage)
  : m_pStreamStorage(pStreamStorage)
{
}

wdMemoryStreamReader::~wdMemoryStreamReader() {}

wdUInt64 wdMemoryStreamReader::ReadBytes(void* pReadBuffer, wdUInt64 uiBytesToRead)
{
  WD_ASSERT_RELEASE(m_pStreamStorage != nullptr, "The memory stream reader needs a valid memory storage object!");

  const wdUInt64 uiBytes = wdMath::Min<wdUInt64>(uiBytesToRead, m_pStreamStorage->GetStorageSize64() - m_uiReadPosition);

  if (uiBytes == 0)
    return 0;

  if (pReadBuffer)
  {
    wdUInt64 uiBytesLeft = uiBytes;

    while (uiBytesLeft > 0)
    {
      wdArrayPtr<const wdUInt8> data = m_pStreamStorage->GetContiguousMemoryRange(m_uiReadPosition);

      WD_ASSERT_DEV(!data.IsEmpty(), "MemoryStreamStorage returned an empty contiguous memory block.");

      const wdUInt64 toRead = wdMath::Min<wdUInt64>(data.GetCount(), uiBytesLeft);

      wdMemoryUtils::Copy(static_cast<wdUInt8*>(pReadBuffer), data.GetPtr(), static_cast<size_t>(toRead)); // Down-cast to size_t for 32-bit.

      pReadBuffer = wdMemoryUtils::AddByteOffset(pReadBuffer, static_cast<size_t>(toRead)); // Down-cast to size_t for 32-bit.

      m_uiReadPosition += toRead;
      uiBytesLeft -= toRead;
    }
  }
  else
  {
    m_uiReadPosition += uiBytes;
  }

  return uiBytes;
}

wdUInt64 wdMemoryStreamReader::SkipBytes(wdUInt64 uiBytesToSkip)
{
  WD_ASSERT_RELEASE(m_pStreamStorage != nullptr, "The memory stream reader needs a valid memory storage object!");

  const wdUInt64 uiBytes = wdMath::Min<wdUInt64>(uiBytesToSkip, m_pStreamStorage->GetStorageSize64() - m_uiReadPosition);

  m_uiReadPosition += uiBytes;

  return uiBytes;
}

void wdMemoryStreamReader::SetReadPosition(wdUInt64 uiReadPosition)
{
  WD_ASSERT_RELEASE(uiReadPosition <= GetByteCount64(), "Read position must be between 0 and GetByteCount()!");
  m_uiReadPosition = uiReadPosition;
}

wdUInt32 wdMemoryStreamReader::GetByteCount32() const
{
  WD_ASSERT_RELEASE(m_pStreamStorage != nullptr, "The memory stream reader needs a valid memory storage object!");

  return m_pStreamStorage->GetStorageSize32();
}

wdUInt64 wdMemoryStreamReader::GetByteCount64() const
{
  WD_ASSERT_RELEASE(m_pStreamStorage != nullptr, "The memory stream reader needs a valid memory storage object!");

  return m_pStreamStorage->GetStorageSize64();
}

void wdMemoryStreamReader::SetDebugSourceInformation(wdStringView sDebugSourceInformation)
{
  m_sDebugSourceInformation = sDebugSourceInformation;
}

//////////////////////////////////////////////////////////////////////////

// Writer implementation
wdMemoryStreamWriter::wdMemoryStreamWriter(wdMemoryStreamStorageInterface* pStreamStorage)
  : m_pStreamStorage(pStreamStorage)
  , m_uiWritePosition(0)
{
}

wdMemoryStreamWriter::~wdMemoryStreamWriter() = default;

wdResult wdMemoryStreamWriter::WriteBytes(const void* pWriteBuffer, wdUInt64 uiBytesToWrite)
{
  WD_ASSERT_DEV(m_pStreamStorage != nullptr, "The memory stream writer needs a valid memory storage object!");

  if (uiBytesToWrite == 0)
    return WD_SUCCESS;

  WD_ASSERT_DEBUG(pWriteBuffer != nullptr, "No valid buffer containing data given!");

  // Reserve the memory in the storage object, grow size if appending data (don't shrink)
  m_pStreamStorage->SetInternalSize(wdMath::Max(m_pStreamStorage->GetStorageSize64(), m_uiWritePosition + uiBytesToWrite));

  {
    wdUInt64 uiBytesLeft = uiBytesToWrite;

    while (uiBytesLeft > 0)
    {
      wdArrayPtr<wdUInt8> data = m_pStreamStorage->GetContiguousMemoryRange(m_uiWritePosition);

      WD_ASSERT_DEV(!data.IsEmpty(), "MemoryStreamStorage returned an empty contiguous memory block.");

      const wdUInt64 toWrite = wdMath::Min<wdUInt64>(data.GetCount(), uiBytesLeft);

      wdMemoryUtils::Copy(data.GetPtr(), static_cast<const wdUInt8*>(pWriteBuffer), static_cast<size_t>(toWrite)); // Down-cast to size_t for 32-bit.

      pWriteBuffer = wdMemoryUtils::AddByteOffset(pWriteBuffer, static_cast<size_t>(toWrite)); // Down-cast to size_t for 32-bit.

      m_uiWritePosition += toWrite;
      uiBytesLeft -= toWrite;
    }
  }

  return WD_SUCCESS;
}

void wdMemoryStreamWriter::SetWritePosition(wdUInt64 uiWritePosition)
{
  WD_ASSERT_RELEASE(m_pStreamStorage != nullptr, "The memory stream writer needs a valid memory storage object!");

  WD_ASSERT_RELEASE(uiWritePosition <= GetByteCount64(), "Write position must be between 0 and GetByteCount()!");
  m_uiWritePosition = uiWritePosition;
}

wdUInt32 wdMemoryStreamWriter::GetByteCount32() const
{
  WD_ASSERT_DEV(m_uiWritePosition <= 0xFFFFFFFFllu, "Use GetByteCount64 instead of GetByteCount32");
  return (wdUInt32)m_uiWritePosition;
}

wdUInt64 wdMemoryStreamWriter::GetByteCount64() const
{
  return m_uiWritePosition;
}

//////////////////////////////////////////////////////////////////////////

wdMemoryStreamStorageInterface::wdMemoryStreamStorageInterface() = default;
wdMemoryStreamStorageInterface::~wdMemoryStreamStorageInterface() = default;

void wdMemoryStreamStorageInterface::ReadAll(wdStreamReader& inout_stream, wdUInt64 uiMaxBytes /*= 0xFFFFFFFFFFFFFFFFllu*/)
{
  Clear();
  wdMemoryStreamWriter w(this);

  wdUInt8 uiTemp[1024 * 8];

  while (uiMaxBytes > 0)
  {
    const wdUInt64 uiToRead = wdMath::Min<wdUInt64>(uiMaxBytes, WD_ARRAY_SIZE(uiTemp));

    const wdUInt64 uiRead = inout_stream.ReadBytes(uiTemp, uiToRead);
    uiMaxBytes -= uiRead;

    w.WriteBytes(uiTemp, uiRead).IgnoreResult();

    if (uiRead < uiToRead)
      break;
  }
}

//////////////////////////////////////////////////////////////////////////


wdRawMemoryStreamReader::wdRawMemoryStreamReader() = default;

wdRawMemoryStreamReader::wdRawMemoryStreamReader(const void* pData, wdUInt64 uiDataSize)
{
  Reset(pData, uiDataSize);
}

wdRawMemoryStreamReader::~wdRawMemoryStreamReader() = default;

void wdRawMemoryStreamReader::Reset(const void* pData, wdUInt64 uiDataSize)
{
  m_pRawMemory = static_cast<const wdUInt8*>(pData);
  m_uiChunkSize = uiDataSize;
  m_uiReadPosition = 0;
}

wdUInt64 wdRawMemoryStreamReader::ReadBytes(void* pReadBuffer, wdUInt64 uiBytesToRead)
{
  const wdUInt64 uiBytes = wdMath::Min<wdUInt64>(uiBytesToRead, m_uiChunkSize - m_uiReadPosition);

  if (uiBytes == 0)
    return 0;

  if (pReadBuffer)
  {
    wdMemoryUtils::Copy(static_cast<wdUInt8*>(pReadBuffer), &m_pRawMemory[m_uiReadPosition], static_cast<size_t>(uiBytes));
  }

  m_uiReadPosition += uiBytes;

  return uiBytes;
}

wdUInt64 wdRawMemoryStreamReader::SkipBytes(wdUInt64 uiBytesToSkip)
{
  const wdUInt64 uiBytes = wdMath::Min<wdUInt64>(uiBytesToSkip, m_uiChunkSize - m_uiReadPosition);

  m_uiReadPosition += uiBytes;

  return uiBytes;
}

void wdRawMemoryStreamReader::SetReadPosition(wdUInt64 uiReadPosition)
{
  WD_ASSERT_RELEASE(uiReadPosition < GetByteCount(), "Read position must be between 0 and GetByteCount()!");
  m_uiReadPosition = uiReadPosition;
}

wdUInt64 wdRawMemoryStreamReader::GetByteCount() const
{
  return m_uiChunkSize;
}

void wdRawMemoryStreamReader::SetDebugSourceInformation(wdStringView sDebugSourceInformation)
{
  m_sDebugSourceInformation = sDebugSourceInformation;
}

//////////////////////////////////////////////////////////////////////////


wdRawMemoryStreamWriter::wdRawMemoryStreamWriter() = default;

wdRawMemoryStreamWriter::wdRawMemoryStreamWriter(void* pData, wdUInt64 uiDataSize)
{
  Reset(pData, uiDataSize);
}

wdRawMemoryStreamWriter::~wdRawMemoryStreamWriter() = default;

void wdRawMemoryStreamWriter::Reset(void* pData, wdUInt64 uiDataSize)
{
  WD_ASSERT_DEV(pData != nullptr, "Invalid memory stream storage");

  m_pRawMemory = static_cast<wdUInt8*>(pData);
  m_uiChunkSize = uiDataSize;
  m_uiWritePosition = 0;
}

wdResult wdRawMemoryStreamWriter::WriteBytes(const void* pWriteBuffer, wdUInt64 uiBytesToWrite)
{
  const wdUInt64 uiBytes = wdMath::Min<wdUInt64>(uiBytesToWrite, m_uiChunkSize - m_uiWritePosition);

  wdMemoryUtils::Copy(&m_pRawMemory[m_uiWritePosition], static_cast<const wdUInt8*>(pWriteBuffer), static_cast<size_t>(uiBytes));

  m_uiWritePosition += uiBytes;

  if (uiBytes < uiBytesToWrite)
    return WD_FAILURE;

  return WD_SUCCESS;
}

wdUInt64 wdRawMemoryStreamWriter::GetStorageSize() const
{
  return m_uiChunkSize;
}

wdUInt64 wdRawMemoryStreamWriter::GetNumWrittenBytes() const
{
  return m_uiWritePosition;
}

void wdRawMemoryStreamWriter::SetDebugSourceInformation(wdStringView sDebugSourceInformation)
{
  m_sDebugSourceInformation = sDebugSourceInformation;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

wdDefaultMemoryStreamStorage::wdDefaultMemoryStreamStorage(wdUInt32 uiInitialCapacity, wdAllocatorBase* pAllocator)
  : m_Chunks(pAllocator)
{
  Reserve(uiInitialCapacity);
}

wdDefaultMemoryStreamStorage::~wdDefaultMemoryStreamStorage()
{
  Clear();
}

void wdDefaultMemoryStreamStorage::Reserve(wdUInt64 uiBytes)
{
  if (m_Chunks.IsEmpty())
  {
    auto& chunk = m_Chunks.ExpandAndGetRef();
    chunk.m_Bytes = wdByteArrayPtr(m_InplaceMemory);
    chunk.m_uiStartOffset = 0;
    m_uiCapacity = m_Chunks[0].m_Bytes.GetCount();
  }

  while (m_uiCapacity < uiBytes)
  {
    AddChunk(static_cast<wdUInt32>(wdMath::Min<wdUInt64>(uiBytes - m_uiCapacity, wdMath::MaxValue<wdUInt32>())));
  }
}

wdUInt64 wdDefaultMemoryStreamStorage::GetStorageSize64() const
{
  return m_uiInternalSize;
}

void wdDefaultMemoryStreamStorage::Clear()
{
  m_uiInternalSize = 0;
  m_uiLastByteAccessed = 0;
  m_uiLastChunkAccessed = 0;
  Compact();
}

void wdDefaultMemoryStreamStorage::Compact()
{
  // skip chunk 0, because that's where our inplace storage is used
  while (m_Chunks.GetCount() > 1)
  {
    auto& chunk = m_Chunks.PeekBack();

    if (m_uiInternalSize > m_uiCapacity - chunk.m_Bytes.GetCount())
      break;

    m_uiCapacity -= chunk.m_Bytes.GetCount();

    wdUInt8* pData = chunk.m_Bytes.GetPtr();
    WD_DELETE_RAW_BUFFER(m_Chunks.GetAllocator(), pData);

    m_Chunks.PopBack();
  }
}

wdUInt64 wdDefaultMemoryStreamStorage::GetHeapMemoryUsage() const
{
  return m_Chunks.GetHeapMemoryUsage() + m_uiCapacity - m_Chunks[0].m_Bytes.GetCount();
}

wdResult wdDefaultMemoryStreamStorage::CopyToStream(wdStreamWriter& inout_stream) const
{
  wdUInt64 uiBytesLeft = m_uiInternalSize;
  wdUInt64 uiReadPosition = 0;

  while (uiBytesLeft > 0)
  {
    wdArrayPtr<const wdUInt8> data = GetContiguousMemoryRange(uiReadPosition);

    WD_ASSERT_DEV(!data.IsEmpty(), "MemoryStreamStorage returned an empty contiguous memory block.");

    WD_SUCCEED_OR_RETURN(inout_stream.WriteBytes(data.GetPtr(), data.GetCount()));

    uiReadPosition += data.GetCount();
    uiBytesLeft -= data.GetCount();
  }

  return WD_SUCCESS;
}

wdArrayPtr<const wdUInt8> wdDefaultMemoryStreamStorage::GetContiguousMemoryRange(wdUInt64 uiStartByte) const
{
  if (uiStartByte >= m_uiInternalSize)
    return {};

  // remember the last access (byte offset) and in which chunk that ended up, to speed up this lookup
  // if a read comes in that's not AFTER the previous one, just reset to the start

  if (uiStartByte < m_uiLastByteAccessed)
  {
    m_uiLastChunkAccessed = 0;
  }

  m_uiLastByteAccessed = uiStartByte;

  for (; m_uiLastChunkAccessed < m_Chunks.GetCount(); ++m_uiLastChunkAccessed)
  {
    const auto& chunk = m_Chunks[m_uiLastChunkAccessed];

    if (uiStartByte < chunk.m_uiStartOffset + chunk.m_Bytes.GetCount())
    {
      const wdUInt64 uiStartByteRel = uiStartByte - chunk.m_uiStartOffset;    // start offset into the chunk
      const wdUInt64 uiMaxLenRel = chunk.m_Bytes.GetCount() - uiStartByteRel; // max number of bytes to use from this chunk
      const wdUInt64 uiMaxRangeRel = m_uiInternalSize - uiStartByte;          // the 'stored data' might be less than the capacity of the chunk

      return {chunk.m_Bytes.GetPtr() + uiStartByteRel, static_cast<wdUInt32>(wdMath::Min<wdUInt64>(uiMaxRangeRel, uiMaxLenRel))};
    }
  }

  return {};
}

wdArrayPtr<wdUInt8> wdDefaultMemoryStreamStorage::GetContiguousMemoryRange(wdUInt64 uiStartByte)
{
  wdArrayPtr<const wdUInt8> constData = const_cast<const wdDefaultMemoryStreamStorage*>(this)->GetContiguousMemoryRange(uiStartByte);
  return {const_cast<wdUInt8*>(constData.GetPtr()), constData.GetCount()};
}

void wdDefaultMemoryStreamStorage::SetInternalSize(wdUInt64 uiSize)
{
  Reserve(uiSize);

  m_uiInternalSize = uiSize;
}

void wdDefaultMemoryStreamStorage::AddChunk(wdUInt32 uiMinimumSize)
{
  auto& chunk = m_Chunks.ExpandAndGetRef();

  wdUInt32 uiSize = 0;

  if (m_Chunks.GetCount() < 4)
  {
    uiSize = 1024 * 4; // 4 KB
  }
  else if (m_Chunks.GetCount() < 8)
  {
    uiSize = 1024 * 64; // 64 KB
  }
  else if (m_Chunks.GetCount() < 16)
  {
    uiSize = 1024 * 1024 * 4; // 4 MB
  }
  else
  {
    uiSize = 1024 * 1024 * 64; // 64 MB
  }

  uiSize = wdMath::Max(uiSize, uiMinimumSize);

  const auto& prevChunk = m_Chunks[m_Chunks.GetCount() - 2];

  chunk.m_Bytes = wdArrayPtr<wdUInt8>(WD_NEW_RAW_BUFFER(m_Chunks.GetAllocator(), wdUInt8, uiSize), uiSize);
  chunk.m_uiStartOffset = prevChunk.m_uiStartOffset + prevChunk.m_Bytes.GetCount();
  m_uiCapacity += chunk.m_Bytes.GetCount();
}


WD_STATICLINK_FILE(Foundation, Foundation_IO_Implementation_MemoryStream);

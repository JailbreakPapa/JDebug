#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/MemoryStream.h>

// Reader implementation

nsMemoryStreamReader::nsMemoryStreamReader(const nsMemoryStreamStorageInterface* pStreamStorage)
  : m_pStreamStorage(pStreamStorage)
{
}

nsMemoryStreamReader::~nsMemoryStreamReader() = default;

nsUInt64 nsMemoryStreamReader::ReadBytes(void* pReadBuffer, nsUInt64 uiBytesToRead)
{
  NS_ASSERT_RELEASE(m_pStreamStorage != nullptr, "The memory stream reader needs a valid memory storage object!");

  const nsUInt64 uiBytes = nsMath::Min<nsUInt64>(uiBytesToRead, m_pStreamStorage->GetStorageSize64() - m_uiReadPosition);

  if (uiBytes == 0)
    return 0;

  if (pReadBuffer)
  {
    nsUInt64 uiBytesLeft = uiBytes;

    while (uiBytesLeft > 0)
    {
      nsArrayPtr<const nsUInt8> data = m_pStreamStorage->GetContiguousMemoryRange(m_uiReadPosition);

      NS_ASSERT_DEV(!data.IsEmpty(), "MemoryStreamStorage returned an empty contiguous memory block.");

      const nsUInt64 toRead = nsMath::Min<nsUInt64>(data.GetCount(), uiBytesLeft);

      nsMemoryUtils::Copy(static_cast<nsUInt8*>(pReadBuffer), data.GetPtr(), static_cast<size_t>(toRead)); // Down-cast to size_t for 32-bit.

      pReadBuffer = nsMemoryUtils::AddByteOffset(pReadBuffer, static_cast<size_t>(toRead));                // Down-cast to size_t for 32-bit.

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

nsUInt64 nsMemoryStreamReader::SkipBytes(nsUInt64 uiBytesToSkip)
{
  NS_ASSERT_RELEASE(m_pStreamStorage != nullptr, "The memory stream reader needs a valid memory storage object!");

  const nsUInt64 uiBytes = nsMath::Min<nsUInt64>(uiBytesToSkip, m_pStreamStorage->GetStorageSize64() - m_uiReadPosition);

  m_uiReadPosition += uiBytes;

  return uiBytes;
}

void nsMemoryStreamReader::SetReadPosition(nsUInt64 uiReadPosition)
{
  NS_ASSERT_RELEASE(uiReadPosition <= GetByteCount64(), "Read position must be between 0 and GetByteCount()!");
  m_uiReadPosition = uiReadPosition;
}

nsUInt32 nsMemoryStreamReader::GetByteCount32() const
{
  NS_ASSERT_RELEASE(m_pStreamStorage != nullptr, "The memory stream reader needs a valid memory storage object!");

  return m_pStreamStorage->GetStorageSize32();
}

nsUInt64 nsMemoryStreamReader::GetByteCount64() const
{
  NS_ASSERT_RELEASE(m_pStreamStorage != nullptr, "The memory stream reader needs a valid memory storage object!");

  return m_pStreamStorage->GetStorageSize64();
}

void nsMemoryStreamReader::SetDebugSourceInformation(nsStringView sDebugSourceInformation)
{
  m_sDebugSourceInformation = sDebugSourceInformation;
}

//////////////////////////////////////////////////////////////////////////

// Writer implementation
nsMemoryStreamWriter::nsMemoryStreamWriter(nsMemoryStreamStorageInterface* pStreamStorage)
  : m_pStreamStorage(pStreamStorage)

{
}

nsMemoryStreamWriter::~nsMemoryStreamWriter() = default;

nsResult nsMemoryStreamWriter::WriteBytes(const void* pWriteBuffer, nsUInt64 uiBytesToWrite)
{
  NS_ASSERT_DEV(m_pStreamStorage != nullptr, "The memory stream writer needs a valid memory storage object!");

  if (uiBytesToWrite == 0)
    return NS_SUCCESS;

  NS_ASSERT_DEBUG(pWriteBuffer != nullptr, "No valid buffer containing data given!");

  // Reserve the memory in the storage object, grow size if appending data (don't shrink)
  m_pStreamStorage->SetInternalSize(nsMath::Max(m_pStreamStorage->GetStorageSize64(), m_uiWritePosition + uiBytesToWrite));

  {
    nsUInt64 uiBytesLeft = uiBytesToWrite;

    while (uiBytesLeft > 0)
    {
      nsArrayPtr<nsUInt8> data = m_pStreamStorage->GetContiguousMemoryRange(m_uiWritePosition);

      NS_ASSERT_DEV(!data.IsEmpty(), "MemoryStreamStorage returned an empty contiguous memory block.");

      const nsUInt64 toWrite = nsMath::Min<nsUInt64>(data.GetCount(), uiBytesLeft);

      nsMemoryUtils::Copy(data.GetPtr(), static_cast<const nsUInt8*>(pWriteBuffer), static_cast<size_t>(toWrite)); // Down-cast to size_t for 32-bit.

      pWriteBuffer = nsMemoryUtils::AddByteOffset(pWriteBuffer, static_cast<size_t>(toWrite));                     // Down-cast to size_t for 32-bit.

      m_uiWritePosition += toWrite;
      uiBytesLeft -= toWrite;
    }
  }

  return NS_SUCCESS;
}

void nsMemoryStreamWriter::SetWritePosition(nsUInt64 uiWritePosition)
{
  NS_ASSERT_RELEASE(m_pStreamStorage != nullptr, "The memory stream writer needs a valid memory storage object!");

  NS_ASSERT_RELEASE(uiWritePosition <= GetByteCount64(), "Write position must be between 0 and GetByteCount()!");
  m_uiWritePosition = uiWritePosition;
}

nsUInt32 nsMemoryStreamWriter::GetByteCount32() const
{
  NS_ASSERT_DEV(m_uiWritePosition <= 0xFFFFFFFFllu, "Use GetByteCount64 instead of GetByteCount32");
  return (nsUInt32)m_uiWritePosition;
}

nsUInt64 nsMemoryStreamWriter::GetByteCount64() const
{
  return m_uiWritePosition;
}

//////////////////////////////////////////////////////////////////////////

nsMemoryStreamStorageInterface::nsMemoryStreamStorageInterface() = default;
nsMemoryStreamStorageInterface::~nsMemoryStreamStorageInterface() = default;

void nsMemoryStreamStorageInterface::ReadAll(nsStreamReader& inout_stream, nsUInt64 uiMaxBytes /*= 0xFFFFFFFFFFFFFFFFllu*/)
{
  Clear();
  nsMemoryStreamWriter w(this);

  nsUInt8 uiTemp[1024 * 8];

  while (uiMaxBytes > 0)
  {
    const nsUInt64 uiToRead = nsMath::Min<nsUInt64>(uiMaxBytes, NS_ARRAY_SIZE(uiTemp));

    const nsUInt64 uiRead = inout_stream.ReadBytes(uiTemp, uiToRead);
    uiMaxBytes -= uiRead;

    w.WriteBytes(uiTemp, uiRead).IgnoreResult();

    if (uiRead < uiToRead)
      break;
  }
}

//////////////////////////////////////////////////////////////////////////


nsRawMemoryStreamReader::nsRawMemoryStreamReader() = default;

nsRawMemoryStreamReader::nsRawMemoryStreamReader(const void* pData, nsUInt64 uiDataSize)
{
  Reset(pData, uiDataSize);
}

nsRawMemoryStreamReader::~nsRawMemoryStreamReader() = default;

void nsRawMemoryStreamReader::Reset(const void* pData, nsUInt64 uiDataSize)
{
  m_pRawMemory = static_cast<const nsUInt8*>(pData);
  m_uiChunkSize = uiDataSize;
  m_uiReadPosition = 0;
}

nsUInt64 nsRawMemoryStreamReader::ReadBytes(void* pReadBuffer, nsUInt64 uiBytesToRead)
{
  const nsUInt64 uiBytes = nsMath::Min<nsUInt64>(uiBytesToRead, m_uiChunkSize - m_uiReadPosition);

  if (uiBytes == 0)
    return 0;

  if (pReadBuffer)
  {
    nsMemoryUtils::Copy(static_cast<nsUInt8*>(pReadBuffer), &m_pRawMemory[m_uiReadPosition], static_cast<size_t>(uiBytes));
  }

  m_uiReadPosition += uiBytes;

  return uiBytes;
}

nsUInt64 nsRawMemoryStreamReader::SkipBytes(nsUInt64 uiBytesToSkip)
{
  const nsUInt64 uiBytes = nsMath::Min<nsUInt64>(uiBytesToSkip, m_uiChunkSize - m_uiReadPosition);

  m_uiReadPosition += uiBytes;

  return uiBytes;
}

void nsRawMemoryStreamReader::SetReadPosition(nsUInt64 uiReadPosition)
{
  NS_ASSERT_RELEASE(uiReadPosition < GetByteCount(), "Read position must be between 0 and GetByteCount()!");
  m_uiReadPosition = uiReadPosition;
}

nsUInt64 nsRawMemoryStreamReader::GetByteCount() const
{
  return m_uiChunkSize;
}

void nsRawMemoryStreamReader::SetDebugSourceInformation(nsStringView sDebugSourceInformation)
{
  m_sDebugSourceInformation = sDebugSourceInformation;
}

//////////////////////////////////////////////////////////////////////////


nsRawMemoryStreamWriter::nsRawMemoryStreamWriter() = default;

nsRawMemoryStreamWriter::nsRawMemoryStreamWriter(void* pData, nsUInt64 uiDataSize)
{
  Reset(pData, uiDataSize);
}

nsRawMemoryStreamWriter::~nsRawMemoryStreamWriter() = default;

void nsRawMemoryStreamWriter::Reset(void* pData, nsUInt64 uiDataSize)
{
  NS_ASSERT_DEV(pData != nullptr, "Invalid memory stream storage");

  m_pRawMemory = static_cast<nsUInt8*>(pData);
  m_uiChunkSize = uiDataSize;
  m_uiWritePosition = 0;
}

nsResult nsRawMemoryStreamWriter::WriteBytes(const void* pWriteBuffer, nsUInt64 uiBytesToWrite)
{
  const nsUInt64 uiBytes = nsMath::Min<nsUInt64>(uiBytesToWrite, m_uiChunkSize - m_uiWritePosition);

  nsMemoryUtils::Copy(&m_pRawMemory[m_uiWritePosition], static_cast<const nsUInt8*>(pWriteBuffer), static_cast<size_t>(uiBytes));

  m_uiWritePosition += uiBytes;

  if (uiBytes < uiBytesToWrite)
    return NS_FAILURE;

  return NS_SUCCESS;
}

nsUInt64 nsRawMemoryStreamWriter::GetStorageSize() const
{
  return m_uiChunkSize;
}

nsUInt64 nsRawMemoryStreamWriter::GetNumWrittenBytes() const
{
  return m_uiWritePosition;
}

void nsRawMemoryStreamWriter::SetDebugSourceInformation(nsStringView sDebugSourceInformation)
{
  m_sDebugSourceInformation = sDebugSourceInformation;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

nsDefaultMemoryStreamStorage::nsDefaultMemoryStreamStorage(nsUInt32 uiInitialCapacity, nsAllocator* pAllocator)
  : m_Chunks(pAllocator)
{
  Reserve(uiInitialCapacity);
}

nsDefaultMemoryStreamStorage::~nsDefaultMemoryStreamStorage()
{
  Clear();
}

void nsDefaultMemoryStreamStorage::Reserve(nsUInt64 uiBytes)
{
  if (m_Chunks.IsEmpty())
  {
    auto& chunk = m_Chunks.ExpandAndGetRef();
    chunk.m_Bytes = nsByteArrayPtr(m_InplaceMemory);
    chunk.m_uiStartOffset = 0;
    m_uiCapacity = m_Chunks[0].m_Bytes.GetCount();
  }

  while (m_uiCapacity < uiBytes)
  {
    AddChunk(static_cast<nsUInt32>(nsMath::Min<nsUInt64>(uiBytes - m_uiCapacity, nsMath::MaxValue<nsUInt32>())));
  }
}

nsUInt64 nsDefaultMemoryStreamStorage::GetStorageSize64() const
{
  return m_uiInternalSize;
}

void nsDefaultMemoryStreamStorage::Clear()
{
  m_uiInternalSize = 0;
  m_uiLastByteAccessed = 0;
  m_uiLastChunkAccessed = 0;
  Compact();
}

void nsDefaultMemoryStreamStorage::Compact()
{
  // skip chunk 0, because that's where our inplace storage is used
  while (m_Chunks.GetCount() > 1)
  {
    auto& chunk = m_Chunks.PeekBack();

    if (m_uiInternalSize > m_uiCapacity - chunk.m_Bytes.GetCount())
      break;

    m_uiCapacity -= chunk.m_Bytes.GetCount();

    nsUInt8* pData = chunk.m_Bytes.GetPtr();
    NS_DELETE_RAW_BUFFER(m_Chunks.GetAllocator(), pData);

    m_Chunks.PopBack();
  }
}

nsUInt64 nsDefaultMemoryStreamStorage::GetHeapMemoryUsage() const
{
  return m_Chunks.GetHeapMemoryUsage() + m_uiCapacity - m_Chunks[0].m_Bytes.GetCount();
}

nsResult nsDefaultMemoryStreamStorage::CopyToStream(nsStreamWriter& inout_stream) const
{
  nsUInt64 uiBytesLeft = m_uiInternalSize;
  nsUInt64 uiReadPosition = 0;

  while (uiBytesLeft > 0)
  {
    nsArrayPtr<const nsUInt8> data = GetContiguousMemoryRange(uiReadPosition);

    NS_ASSERT_DEV(!data.IsEmpty(), "MemoryStreamStorage returned an empty contiguous memory block.");

    NS_SUCCEED_OR_RETURN(inout_stream.WriteBytes(data.GetPtr(), data.GetCount()));

    uiReadPosition += data.GetCount();
    uiBytesLeft -= data.GetCount();
  }

  return NS_SUCCESS;
}

nsArrayPtr<const nsUInt8> nsDefaultMemoryStreamStorage::GetContiguousMemoryRange(nsUInt64 uiStartByte) const
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
      const nsUInt64 uiStartByteRel = uiStartByte - chunk.m_uiStartOffset;    // start offset into the chunk
      const nsUInt64 uiMaxLenRel = chunk.m_Bytes.GetCount() - uiStartByteRel; // max number of bytes to use from this chunk
      const nsUInt64 uiMaxRangeRel = m_uiInternalSize - uiStartByte;          // the 'stored data' might be less than the capacity of the chunk

      return {chunk.m_Bytes.GetPtr() + uiStartByteRel, static_cast<nsUInt32>(nsMath::Min<nsUInt64>(uiMaxRangeRel, uiMaxLenRel))};
    }
  }

  return {};
}

nsArrayPtr<nsUInt8> nsDefaultMemoryStreamStorage::GetContiguousMemoryRange(nsUInt64 uiStartByte)
{
  nsArrayPtr<const nsUInt8> constData = const_cast<const nsDefaultMemoryStreamStorage*>(this)->GetContiguousMemoryRange(uiStartByte);
  return {const_cast<nsUInt8*>(constData.GetPtr()), constData.GetCount()};
}

void nsDefaultMemoryStreamStorage::SetInternalSize(nsUInt64 uiSize)
{
  Reserve(uiSize);

  m_uiInternalSize = uiSize;
}

void nsDefaultMemoryStreamStorage::AddChunk(nsUInt32 uiMinimumSize)
{
  auto& chunk = m_Chunks.ExpandAndGetRef();

  nsUInt32 uiSize = 0;

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

  uiSize = nsMath::Max(uiSize, uiMinimumSize);

  const auto& prevChunk = m_Chunks[m_Chunks.GetCount() - 2];

  chunk.m_Bytes = nsArrayPtr<nsUInt8>(NS_NEW_RAW_BUFFER(m_Chunks.GetAllocator(), nsUInt8, uiSize), uiSize);
  chunk.m_uiStartOffset = prevChunk.m_uiStartOffset + prevChunk.m_Bytes.GetCount();
  m_uiCapacity += chunk.m_Bytes.GetCount();
}

#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/CompressedStreamZlib.h>
#include <Foundation/Math/Math.h>

#ifdef BUILDSYSTEM_ENABLE_ZLIB_SUPPORT

#  include <zlib/zlib.h>

static voidpf zLibAlloc OF((voidpf opaque, uInt items, uInt size))
{
  return NS_DEFAULT_NEW_RAW_BUFFER(nsUInt8, nsMath::SafeConvertToSizeT(nsMath::SafeMultiply64(items, size)));
}

static void zLibFree OF((voidpf opaque, voidpf address))
{
  nsUInt8* pData = (nsUInt8*)address;
  NS_DEFAULT_DELETE_RAW_BUFFER(pData);
}

NS_DEFINE_AS_POD_TYPE(z_stream_s);

nsCompressedStreamReaderZip::nsCompressedStreamReaderZip() = default;

nsCompressedStreamReaderZip::~nsCompressedStreamReaderZip()
{
  NS_VERIFY(inflateEnd(m_pZLibStream) == Z_OK, "Deinitializing the zlib stream failed: '{0}'", m_pZLibStream->msg);
  NS_DEFAULT_DELETE(m_pZLibStream);
}

void nsCompressedStreamReaderZip::SetInputStream(nsStreamReader* pInputStream, nsUInt64 uiInputSize)
{
  if (m_pZLibStream)
  {
    NS_VERIFY(inflateEnd(m_pZLibStream) == Z_OK, "Deinitializing the zlib stream failed: '{0}'", m_pZLibStream->msg);
    NS_DEFAULT_DELETE(m_pZLibStream);
  }

  m_CompressedCache.SetCountUninitialized(1024 * 4);
  m_bReachedEnd = false;
  m_pInputStream = pInputStream;
  m_uiRemainingInputSize = uiInputSize;

  {
    m_pZLibStream = NS_DEFAULT_NEW(z_stream_s);
    nsMemoryUtils::ZeroFill(m_pZLibStream, 1);

    m_pZLibStream->opaque = nullptr;
    m_pZLibStream->zalloc = zLibAlloc;
    m_pZLibStream->zfree = zLibFree;

    NS_VERIFY(inflateInit2(m_pZLibStream, -MAX_WBITS) == Z_OK, "Initializing the zip stream for decompression failed: '{0}'", m_pZLibStream->msg);
  }
}

nsUInt64 nsCompressedStreamReaderZip::ReadBytes(void* pReadBuffer, nsUInt64 uiBytesToRead)
{
  if (uiBytesToRead == 0 || m_bReachedEnd)
    return 0;

  // Implement the 'skip n bytes' feature with a temp cache
  if (pReadBuffer == nullptr)
  {
    nsUInt64 uiBytesRead = 0;
    nsUInt8 uiTemp[1024];

    while (uiBytesToRead > 0)
    {
      const nsUInt32 uiToRead = nsMath::Min<nsUInt32>(static_cast<nsUInt32>(uiBytesToRead), 1024);

      const nsUInt64 uiGotBytes = ReadBytes(uiTemp, uiToRead);

      uiBytesRead += uiGotBytes;
      uiBytesToRead -= uiGotBytes;

      if (uiGotBytes == 0) // prevent an endless loop
        break;
    }

    return uiBytesRead;
  }


  m_pZLibStream->next_out = static_cast<Bytef*>(pReadBuffer);
  m_pZLibStream->avail_out = static_cast<nsUInt32>(uiBytesToRead);
  m_pZLibStream->total_out = 0;

  while (m_pZLibStream->avail_out > 0)
  {
    // if our input buffer is empty, we need to read more into our cache
    if (m_pZLibStream->avail_in == 0 && m_uiRemainingInputSize > 0)
    {
      nsUInt64 uiReadAmount = m_CompressedCache.GetCount();
      if (m_uiRemainingInputSize < uiReadAmount)
      {
        uiReadAmount = m_uiRemainingInputSize;
      }
      if (uiReadAmount == 0)
      {
        m_bReachedEnd = true;
        return m_pZLibStream->total_out;
      }

      NS_VERIFY(m_pInputStream->ReadBytes(m_CompressedCache.GetData(), sizeof(nsUInt8) * uiReadAmount) == sizeof(nsUInt8) * uiReadAmount, "Reading the compressed chunk of size {0} from the input stream failed.", uiReadAmount);
      m_pZLibStream->avail_in = static_cast<uInt>(uiReadAmount);
      m_pZLibStream->next_in = m_CompressedCache.GetData();
      m_uiRemainingInputSize -= uiReadAmount;
    }

    const int iRet = inflate(m_pZLibStream, Z_SYNC_FLUSH);
    NS_ASSERT_DEV(iRet == Z_OK || iRet == Z_STREAM_END, "Decompressing the stream failed: '{0}'", m_pZLibStream->msg);

    if (iRet == Z_STREAM_END)
    {
      m_bReachedEnd = true;
      NS_ASSERT_DEV(m_pZLibStream->avail_in == 0, "The input buffer should be depleted, but {0} bytes are still there.", m_pZLibStream->avail_in);
      return m_pZLibStream->total_out;
    }
  }

  return m_pZLibStream->total_out;
}


//////////////////////////////////////////////////////////////////////////


nsCompressedStreamReaderZlib::nsCompressedStreamReaderZlib(nsStreamReader* pInputStream)
  : m_pInputStream(pInputStream)
{
  m_CompressedCache.SetCountUninitialized(1024 * 4);
}

nsCompressedStreamReaderZlib::~nsCompressedStreamReaderZlib()
{
  NS_VERIFY(inflateEnd(m_pZLibStream) == Z_OK, "Deinitializing the zlib stream failed: '{0}'", m_pZLibStream->msg);

  NS_DEFAULT_DELETE(m_pZLibStream);
}

nsUInt64 nsCompressedStreamReaderZlib::ReadBytes(void* pReadBuffer, nsUInt64 uiBytesToRead)
{
  if (uiBytesToRead == 0 || m_bReachedEnd)
    return 0;

  // if we have not read from the stream before, initialize everything
  if (m_pZLibStream == nullptr)
  {
    m_pZLibStream = NS_DEFAULT_NEW(z_stream_s);
    nsMemoryUtils::ZeroFill(m_pZLibStream, 1);

    m_pZLibStream->opaque = nullptr;
    m_pZLibStream->zalloc = zLibAlloc;
    m_pZLibStream->zfree = zLibFree;

    NS_VERIFY(inflateInit(m_pZLibStream) == Z_OK, "Initializing the zlib stream for decompression failed: '{0}'", m_pZLibStream->msg);
  }

  // Implement the 'skip n bytes' feature with a temp cache
  if (pReadBuffer == nullptr)
  {
    nsUInt64 uiBytesRead = 0;
    nsUInt8 uiTemp[1024];

    while (uiBytesToRead > 0)
    {
      const nsUInt32 uiToRead = nsMath::Min<nsUInt32>(static_cast<nsUInt32>(uiBytesToRead), 1024);

      const nsUInt64 uiGotBytes = ReadBytes(uiTemp, uiToRead);

      uiBytesRead += uiGotBytes;
      uiBytesToRead -= uiGotBytes;

      if (uiGotBytes == 0) // prevent an endless loop
        break;
    }

    return uiBytesRead;
  }


  m_pZLibStream->next_out = static_cast<Bytef*>(pReadBuffer);
  m_pZLibStream->avail_out = static_cast<nsUInt32>(uiBytesToRead);
  m_pZLibStream->total_out = 0;

  while (m_pZLibStream->avail_out > 0)
  {
    // if our input buffer is empty, we need to read more into our cache
    if (m_pZLibStream->avail_in == 0)
    {
      nsUInt16 uiCompressedSize = 0;
      NS_VERIFY(m_pInputStream->ReadBytes(&uiCompressedSize, sizeof(nsUInt16)) == sizeof(nsUInt16), "Reading the compressed chunk size from the input stream failed.");

      m_pZLibStream->avail_in = uiCompressedSize;
      m_pZLibStream->next_in = m_CompressedCache.GetData();

      if (uiCompressedSize > 0)
      {
        NS_VERIFY(m_pInputStream->ReadBytes(m_CompressedCache.GetData(), sizeof(nsUInt8) * uiCompressedSize) == sizeof(nsUInt8) * uiCompressedSize, "Reading the compressed chunk of size {0} from the input stream failed.", uiCompressedSize);
      }
    }

    // if the input buffer is still empty, there was no more data to read (we reached the zero-terminator)
    if (m_pZLibStream->avail_in == 0)
    {
      // in this case there is also no output that can be generated anymore
      m_bReachedEnd = true;
      return m_pZLibStream->total_out;
    }

    const int iRet = inflate(m_pZLibStream, Z_NO_FLUSH);
    NS_ASSERT_DEV(iRet == Z_OK || iRet == Z_STREAM_END, "Decompressing the stream failed: '{0}'", m_pZLibStream->msg);

    if (iRet == Z_STREAM_END)
    {
      m_bReachedEnd = true;

      // if we have reached the end, we have not yet read the zero-terminator
      // do this now, so that data that comes after the compressed stream can be read properly

      nsUInt16 uiTerminator = 0;
      NS_VERIFY(m_pInputStream->ReadBytes(&uiTerminator, sizeof(nsUInt16)) == sizeof(nsUInt16), "Reading the compressed stream terminator failed.");

      NS_ASSERT_DEV(uiTerminator == 0, "Unexpected Stream Terminator: {0}", uiTerminator);
      NS_ASSERT_DEV(m_pZLibStream->avail_in == 0, "The input buffer should be depleted, but {0} bytes are still there.", m_pZLibStream->avail_in);
      return m_pZLibStream->total_out;
    }
  }

  return m_pZLibStream->total_out;
}


nsCompressedStreamWriterZlib::nsCompressedStreamWriterZlib(nsStreamWriter* pOutputStream, Compression ratio)
  : m_pOutputStream(pOutputStream)
{
  m_CompressedCache.SetCountUninitialized(1024 * 4);

  m_pZLibStream = NS_DEFAULT_NEW(z_stream_s);

  nsMemoryUtils::ZeroFill(m_pZLibStream, 1);

  m_pZLibStream->opaque = nullptr;
  m_pZLibStream->zalloc = zLibAlloc;
  m_pZLibStream->zfree = zLibFree;
  m_pZLibStream->next_out = m_CompressedCache.GetData();
  m_pZLibStream->avail_out = m_CompressedCache.GetCount();
  m_pZLibStream->total_out = 0;

  NS_VERIFY(deflateInit(m_pZLibStream, ratio) == Z_OK, "Initializing the zlib stream for compression failed: '{0}'", m_pZLibStream->msg);
}

nsCompressedStreamWriterZlib::~nsCompressedStreamWriterZlib()
{
  CloseStream().IgnoreResult();
}

nsResult nsCompressedStreamWriterZlib::CloseStream()
{
  if (m_pZLibStream == nullptr)
    return NS_SUCCESS;

  nsInt32 iRes = Z_OK;
  while (iRes == Z_OK)
  {
    if (m_pZLibStream->avail_out == 0)
    {
      if (Flush() == NS_FAILURE)
        return NS_FAILURE;
    }

    iRes = deflate(m_pZLibStream, Z_FINISH);
    NS_ASSERT_DEV(iRes == Z_STREAM_END || iRes == Z_OK, "Finishing the stream failed: '{0}'", m_pZLibStream->msg);
  }

  // one more flush to write out the last chunk
  if (Flush() == NS_FAILURE)
    return NS_FAILURE;

  // write a zero-terminator
  const nsUInt16 uiTerminator = 0;
  if (m_pOutputStream->WriteBytes(&uiTerminator, sizeof(nsUInt16)) == NS_FAILURE)
    return NS_FAILURE;

  NS_VERIFY(deflateEnd(m_pZLibStream) == Z_OK, "Deinitializing the zlib compression stream failed: '{0}'", m_pZLibStream->msg);
  NS_DEFAULT_DELETE(m_pZLibStream);

  return NS_SUCCESS;
}

nsResult nsCompressedStreamWriterZlib::Flush()
{
  if (m_pZLibStream == nullptr)
    return NS_SUCCESS;

  const nsUInt16 uiUsedCache = static_cast<nsUInt16>(m_pZLibStream->total_out);

  if (uiUsedCache == 0)
    return NS_SUCCESS;

  if (m_pOutputStream->WriteBytes(&uiUsedCache, sizeof(nsUInt16)) == NS_FAILURE)
    return NS_FAILURE;

  if (m_pOutputStream->WriteBytes(m_CompressedCache.GetData(), sizeof(nsUInt8) * uiUsedCache) == NS_FAILURE)
    return NS_FAILURE;

  m_uiCompressedSize += uiUsedCache;

  m_pZLibStream->total_out = 0;
  m_pZLibStream->next_out = m_CompressedCache.GetData();
  m_pZLibStream->avail_out = m_CompressedCache.GetCount();

  return NS_SUCCESS;
}

nsResult nsCompressedStreamWriterZlib::WriteBytes(const void* pWriteBuffer, nsUInt64 uiBytesToWrite)
{
  NS_ASSERT_DEV(m_pZLibStream != nullptr, "The stream is already closed, you cannot write more data to it.");

  m_uiUncompressedSize += uiBytesToWrite;

  m_pZLibStream->next_in = static_cast<Bytef*>(const_cast<void*>(pWriteBuffer)); // C libraries suck at type safety
  m_pZLibStream->avail_in = static_cast<nsUInt32>(uiBytesToWrite);
  m_pZLibStream->total_in = 0;

  while (m_pZLibStream->avail_in > 0)
  {
    if (m_pZLibStream->avail_out == 0)
    {
      if (Flush() == NS_FAILURE)
        return NS_FAILURE;
    }

    NS_VERIFY(deflate(m_pZLibStream, Z_NO_FLUSH) == Z_OK, "Compressing the zlib stream failed: '{0}'", m_pZLibStream->msg);
  }

  return NS_SUCCESS;
}

#endif // BUILDSYSTEM_ENABLE_ZLIB_SUPPORT

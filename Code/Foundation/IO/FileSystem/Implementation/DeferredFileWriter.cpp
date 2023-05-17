#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/FileSystem/DeferredFileWriter.h>
#include <Foundation/IO/FileSystem/FileReader.h>

wdDeferredFileWriter::wdDeferredFileWriter()
  : m_Writer(&m_Storage)
{
}

void wdDeferredFileWriter::SetOutput(wdStringView sFileToWriteTo, bool bOnlyWriteIfDifferent)
{
  m_bOnlyWriteIfDifferent = bOnlyWriteIfDifferent;
  m_sOutputFile = sFileToWriteTo;
}

wdResult wdDeferredFileWriter::WriteBytes(const void* pWriteBuffer, wdUInt64 uiBytesToWrite)
{
  WD_ASSERT_DEBUG(!m_sOutputFile.IsEmpty(), "Output file has not been configured");

  return m_Writer.WriteBytes(pWriteBuffer, uiBytesToWrite);
}

wdResult wdDeferredFileWriter::Close()
{
  if (m_bAlreadyClosed)
    return WD_SUCCESS;

  if (m_sOutputFile.IsEmpty())
    return WD_FAILURE;

  m_bAlreadyClosed = true;

  if (m_bOnlyWriteIfDifferent)
  {
    wdFileReader fileIn;
    if (fileIn.Open(m_sOutputFile).Succeeded() && fileIn.GetFileSize() == m_Storage.GetStorageSize64())
    {
      wdUInt8 tmp1[1024 * 4];
      wdUInt8 tmp2[1024 * 4];

      wdUInt64 readLeft = m_Storage.GetStorageSize64();

      wdMemoryStreamReader storageReader(&m_Storage);

      while (true)
      {
        const wdUInt64 readBytes1 = fileIn.ReadBytes(tmp1, WD_ARRAY_SIZE(tmp1));
        const wdUInt64 readBytes2 = storageReader.ReadBytes(tmp2, WD_ARRAY_SIZE(tmp2));

        if (readBytes1 != readBytes2)
          goto write_data;

        if (readBytes1 == 0)
          break;

        if (wdMemoryUtils::RawByteCompare(tmp1, tmp2, wdMath::SafeConvertToSizeT(readBytes1)) != 0)
          goto write_data;
      }

      // content is already the same as what we would write -> skip the write (do not modify file write date)
      return WD_SUCCESS;
    }
  }

write_data:
  wdFileWriter file;
  WD_SUCCEED_OR_RETURN(file.Open(m_sOutputFile, 0)); // use the minimum cache size, we want to pass data directly through to disk

  m_sOutputFile.Clear();
  return m_Storage.CopyToStream(file);
}

void wdDeferredFileWriter::Discard()
{
  m_sOutputFile.Clear();
}

WD_STATICLINK_FILE(Foundation, Foundation_IO_FileSystem_Implementation_DeferredFileWriter);

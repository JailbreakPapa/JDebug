#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/FileSystem/DeferredFileWriter.h>
#include <Foundation/IO/FileSystem/FileReader.h>

nsDeferredFileWriter::nsDeferredFileWriter()
  : m_Writer(&m_Storage)
{
}

void nsDeferredFileWriter::SetOutput(nsStringView sFileToWriteTo, bool bOnlyWriteIfDifferent)
{
  m_bOnlyWriteIfDifferent = bOnlyWriteIfDifferent;
  m_sOutputFile = sFileToWriteTo;
}

nsResult nsDeferredFileWriter::WriteBytes(const void* pWriteBuffer, nsUInt64 uiBytesToWrite)
{
  NS_ASSERT_DEBUG(!m_sOutputFile.IsEmpty(), "Output file has not been configured");

  return m_Writer.WriteBytes(pWriteBuffer, uiBytesToWrite);
}

nsResult nsDeferredFileWriter::Close(bool* out_pWasWrittenTo /*= nullptr*/)
{
  if (out_pWasWrittenTo)
  {
    *out_pWasWrittenTo = false;
  }

  if (m_bAlreadyClosed)
    return NS_SUCCESS;

  if (m_sOutputFile.IsEmpty())
    return NS_FAILURE;

  m_bAlreadyClosed = true;

  if (m_bOnlyWriteIfDifferent)
  {
    nsFileReader fileIn;
    if (fileIn.Open(m_sOutputFile).Succeeded() && fileIn.GetFileSize() == m_Storage.GetStorageSize64())
    {
      nsUInt8 tmp1[1024 * 4];
      nsUInt8 tmp2[1024 * 4];

      nsMemoryStreamReader storageReader(&m_Storage);

      while (true)
      {
        const nsUInt64 readBytes1 = fileIn.ReadBytes(tmp1, NS_ARRAY_SIZE(tmp1));
        const nsUInt64 readBytes2 = storageReader.ReadBytes(tmp2, NS_ARRAY_SIZE(tmp2));

        if (readBytes1 != readBytes2)
          goto write_data;

        if (readBytes1 == 0)
          break;

        if (nsMemoryUtils::RawByteCompare(tmp1, tmp2, nsMath::SafeConvertToSizeT(readBytes1)) != 0)
          goto write_data;
      }

      // content is already the same as what we would write -> skip the write (do not modify file write date)
      return NS_SUCCESS;
    }
  }

write_data:
  nsFileWriter file;
  NS_SUCCEED_OR_RETURN(file.Open(m_sOutputFile, 0)); // use the minimum cache size, we want to pass data directly through to disk

  if (out_pWasWrittenTo)
  {
    *out_pWasWrittenTo = true;
  }

  m_sOutputFile.Clear();
  return m_Storage.CopyToStream(file);
}

void nsDeferredFileWriter::Discard()
{
  m_sOutputFile.Clear();
}

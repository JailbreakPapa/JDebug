#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/Archive/ArchiveReader.h>
#include <Foundation/IO/Archive/ArchiveUtils.h>

#include <Foundation/IO/Archive/ArchiveUtils.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Types/Types.h>

#include <Foundation/Logging/Log.h>

nsResult nsArchiveReader::OpenArchive(nsStringView sPath)
{
#if NS_ENABLED(NS_SUPPORTS_MEMORY_MAPPED_FILE)
  NS_LOG_BLOCK("OpenArchive", sPath);

  NS_SUCCEED_OR_RETURN(m_MemFile.Open(sPath, nsMemoryMappedFile::Mode::ReadOnly));
  m_uiMemFileSize = m_MemFile.GetFileSize();

  // validate the archive
  {
    nsRawMemoryStreamReader reader(m_MemFile.GetReadPointer(), m_MemFile.GetFileSize());

    nsStringView extension = nsPathUtils::GetFileExtension(sPath);

    if (nsArchiveUtils::IsAcceptedArchiveFileExtensions(extension))
    {
      NS_SUCCEED_OR_RETURN(nsArchiveUtils::ReadHeader(reader, m_uiArchiveVersion));

      m_pDataStart = m_MemFile.GetReadPointer(nsArchiveUtils::ArchiveHeaderSize, nsMemoryMappedFile::OffsetBase::Start);

      NS_SUCCEED_OR_RETURN(nsArchiveUtils::ExtractTOC(m_MemFile, m_ArchiveTOC, m_uiArchiveVersion));
    }
#  ifdef BUILDSYSTEM_ENABLE_ZLIB_SUPPORT
    else if (extension == "zip" || extension == "apk")
    {
      NS_SUCCEED_OR_RETURN(nsArchiveUtils::ReadZipHeader(reader, m_uiArchiveVersion));
      if (m_uiArchiveVersion != 0)
      {
        nsLog::Error("Unknown zip version '{}'", m_uiArchiveVersion);
        return NS_FAILURE;
      }
      m_pDataStart = m_MemFile.GetReadPointer(0, nsMemoryMappedFile::OffsetBase::Start);

      if (nsArchiveUtils::ExtractZipTOC(m_MemFile, m_ArchiveTOC).Failed())
      {
        nsLog::Error("Failed to deserialize zip TOC");
        return NS_FAILURE;
      }
    }
#  endif
    else
    {
      nsLog::Error("Unknown archive file extension '{}'", extension);
      return NS_FAILURE;
    }
  }

  // validate the entries
  {
    const nsUInt32 uiMaxPathString = m_ArchiveTOC.m_AllPathStrings.GetCount();
    const nsUInt64 uiValidSize = m_uiMemFileSize - uiMaxPathString;

    for (const auto& e : m_ArchiveTOC.m_Entries)
    {
      if (e.m_uiDataStartOffset + e.m_uiStoredDataSize > uiValidSize)
      {
        nsLog::Error("Archive is corrupt. Invalid entry data range.");
        return NS_FAILURE;
      }

      if (e.m_uiUncompressedDataSize < e.m_uiStoredDataSize)
      {
        nsLog::Error("Archive is corrupt. Invalid compression info.");
        return NS_FAILURE;
      }

      if (e.m_uiPathStringOffset >= uiMaxPathString)
      {
        nsLog::Error("Archive is corrupt. Invalid entry path-string offset.");
        return NS_FAILURE;
      }
    }
  }

  return NS_SUCCESS;
#else
  NS_REPORT_FAILURE("Memory mapped files are unsupported on this platform.");
  return NS_FAILURE;
#endif
}

const nsArchiveTOC& nsArchiveReader::GetArchiveTOC()
{
  return m_ArchiveTOC;
}

nsResult nsArchiveReader::ExtractAllFiles(nsStringView sTargetFolder) const
{
  NS_LOG_BLOCK("ExtractAllFiles", sTargetFolder);

  const nsUInt32 numEntries = m_ArchiveTOC.m_Entries.GetCount();

  for (nsUInt32 e = 0; e < numEntries; ++e)
  {
    const char* szPath = reinterpret_cast<const char*>(&m_ArchiveTOC.m_AllPathStrings[m_ArchiveTOC.m_Entries[e].m_uiPathStringOffset]);

    if (!ExtractNextFileCallback(e + 1, numEntries, szPath))
      return NS_FAILURE;

    NS_SUCCEED_OR_RETURN(ExtractFile(e, sTargetFolder));
  }

  return NS_SUCCESS;
}

void nsArchiveReader::ConfigureRawMemoryStreamReader(nsUInt32 uiEntryIdx, nsRawMemoryStreamReader& ref_memReader) const
{
  nsArchiveUtils::ConfigureRawMemoryStreamReader(m_ArchiveTOC.m_Entries[uiEntryIdx], m_pDataStart, ref_memReader);
}

nsUniquePtr<nsStreamReader> nsArchiveReader::CreateEntryReader(nsUInt32 uiEntryIdx) const
{
  return nsArchiveUtils::CreateEntryReader(m_ArchiveTOC.m_Entries[uiEntryIdx], m_pDataStart);
}

nsResult nsArchiveReader::ExtractFile(nsUInt32 uiEntryIdx, nsStringView sTargetFolder) const
{
  nsStringView sFilePath = m_ArchiveTOC.GetEntryPathString(uiEntryIdx);
  const nsUInt64 uiMaxSize = m_ArchiveTOC.m_Entries[uiEntryIdx].m_uiUncompressedDataSize;

  nsUniquePtr<nsStreamReader> pReader = CreateEntryReader(uiEntryIdx);

  nsStringBuilder sOutputFile = sTargetFolder;
  sOutputFile.AppendPath(sFilePath);

  nsFileWriter file;
  NS_SUCCEED_OR_RETURN(file.Open(sOutputFile));

  nsUInt8 uiTemp[1024 * 8];

  nsUInt64 uiRead = 0;
  nsUInt64 uiReadTotal = 0;
  while (true)
  {
    uiRead = pReader->ReadBytes(uiTemp, NS_ARRAY_SIZE(uiTemp));

    if (uiRead == 0)
      break;

    NS_SUCCEED_OR_RETURN(file.WriteBytes(uiTemp, uiRead));

    uiReadTotal += uiRead;

    if (!ExtractFileProgressCallback(uiReadTotal, uiMaxSize))
      return NS_FAILURE;
  }

  NS_ASSERT_DEV(uiReadTotal == uiMaxSize, "Failed to read entire file");

  return NS_SUCCESS;
}

bool nsArchiveReader::ExtractNextFileCallback(nsUInt32 uiCurEntry, nsUInt32 uiMaxEntries, nsStringView sSourceFile) const
{
  return true;
}

bool nsArchiveReader::ExtractFileProgressCallback(nsUInt64 bytesWritten, nsUInt64 bytesTotal) const
{
  return true;
}

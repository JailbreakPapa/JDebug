#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/Archive/ArchiveReader.h>
#include <Foundation/IO/Archive/ArchiveUtils.h>

#include <Foundation/IO/Archive/ArchiveUtils.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Types/Types.h>

#include <Foundation/Logging/Log.h>

wdResult wdArchiveReader::OpenArchive(wdStringView sPath)
{
#if WD_ENABLED(WD_SUPPORTS_MEMORY_MAPPED_FILE)
  WD_LOG_BLOCK("OpenArchive", sPath);

  WD_SUCCEED_OR_RETURN(m_MemFile.Open(sPath, wdMemoryMappedFile::Mode::ReadOnly));
  m_uiMemFileSize = m_MemFile.GetFileSize();

  // validate the archive
  {
    wdRawMemoryStreamReader reader(m_MemFile.GetReadPointer(), m_MemFile.GetFileSize());

    wdStringView extension = wdPathUtils::GetFileExtension(sPath);

    if (wdArchiveUtils::IsAcceptedArchiveFileExtensions(extension))
    {
      WD_SUCCEED_OR_RETURN(wdArchiveUtils::ReadHeader(reader, m_uiArchiveVersion));

      m_pDataStart = m_MemFile.GetReadPointer(16, wdMemoryMappedFile::OffsetBase::Start);

      WD_SUCCEED_OR_RETURN(wdArchiveUtils::ExtractTOC(m_MemFile, m_ArchiveTOC, m_uiArchiveVersion));
    }

    else
    {
      wdLog::Error("Unknown archive file extension '{}'", extension);
      return WD_FAILURE;
    }
  }

  // validate the entries
  {
    const wdUInt32 uiMaxPathString = m_ArchiveTOC.m_AllPathStrings.GetCount();
    const wdUInt64 uiValidSize = m_uiMemFileSize - uiMaxPathString;

    for (const auto& e : m_ArchiveTOC.m_Entries)
    {
      if (e.m_uiDataStartOffset + e.m_uiStoredDataSize > uiValidSize)
      {
        wdLog::Error("Archive is corrupt. Invalid entry data range.");
        return WD_FAILURE;
      }

      if (e.m_uiUncompressedDataSize < e.m_uiStoredDataSize)
      {
        wdLog::Error("Archive is corrupt. Invalid compression info.");
        return WD_FAILURE;
      }

      if (e.m_uiPathStringOffset >= uiMaxPathString)
      {
        wdLog::Error("Archive is corrupt. Invalid entry path-string offset.");
        return WD_FAILURE;
      }
    }
  }

  return WD_SUCCESS;
#else
  WD_REPORT_FAILURE("Memory mapped files are unsupported on this platform.");
  return WD_FAILURE;
#endif
}

const wdArchiveTOC& wdArchiveReader::GetArchiveTOC()
{
  return m_ArchiveTOC;
}

wdResult wdArchiveReader::ExtractAllFiles(wdStringView sTargetFolder) const
{
  WD_LOG_BLOCK("ExtractAllFiles", sTargetFolder);

  const wdUInt32 numEntries = m_ArchiveTOC.m_Entries.GetCount();

  for (wdUInt32 e = 0; e < numEntries; ++e)
  {
    const char* szPath = reinterpret_cast<const char*>(&m_ArchiveTOC.m_AllPathStrings[m_ArchiveTOC.m_Entries[e].m_uiPathStringOffset]);

    if (!ExtractNextFileCallback(e + 1, numEntries, szPath))
      return WD_FAILURE;

    WD_SUCCEED_OR_RETURN(ExtractFile(e, sTargetFolder));
  }

  return WD_SUCCESS;
}

void wdArchiveReader::ConfigureRawMemoryStreamReader(wdUInt32 uiEntryIdx, wdRawMemoryStreamReader& ref_memReader) const
{
  wdArchiveUtils::ConfigureRawMemoryStreamReader(m_ArchiveTOC.m_Entries[uiEntryIdx], m_pDataStart, ref_memReader);
}

wdUniquePtr<wdStreamReader> wdArchiveReader::CreateEntryReader(wdUInt32 uiEntryIdx) const
{
  return wdArchiveUtils::CreateEntryReader(m_ArchiveTOC.m_Entries[uiEntryIdx], m_pDataStart);
}

wdResult wdArchiveReader::ExtractFile(wdUInt32 uiEntryIdx, wdStringView sTargetFolder) const
{
  const char* szFilePath = m_ArchiveTOC.GetEntryPathString(uiEntryIdx);
  const wdUInt64 uiMaxSize = m_ArchiveTOC.m_Entries[uiEntryIdx].m_uiUncompressedDataSize;

  wdUniquePtr<wdStreamReader> pReader = CreateEntryReader(uiEntryIdx);

  wdStringBuilder sOutputFile = sTargetFolder;
  sOutputFile.AppendPath(szFilePath);

  wdFileWriter file;
  WD_SUCCEED_OR_RETURN(file.Open(sOutputFile));

  wdUInt8 uiTemp[1024 * 8];

  wdUInt64 uiRead = 0;
  wdUInt64 uiReadTotal = 0;
  while (true)
  {
    uiRead = pReader->ReadBytes(uiTemp, WD_ARRAY_SIZE(uiTemp));

    if (uiRead == 0)
      break;

    WD_SUCCEED_OR_RETURN(file.WriteBytes(uiTemp, uiRead));

    uiReadTotal += uiRead;

    if (!ExtractFileProgressCallback(uiReadTotal, uiMaxSize))
      return WD_FAILURE;
  }

  WD_ASSERT_DEV(uiReadTotal == uiMaxSize, "Failed to read entire file");

  return WD_SUCCESS;
}

bool wdArchiveReader::ExtractNextFileCallback(wdUInt32 uiCurEntry, wdUInt32 uiMaxEntries, wdStringView sSourceFile) const
{
  return true;
}

bool wdArchiveReader::ExtractFileProgressCallback(wdUInt64 bytesWritten, wdUInt64 bytesTotal) const
{
  return true;
}


WD_STATICLINK_FILE(Foundation, Foundation_IO_Archive_Implementation_ArchiveReader);

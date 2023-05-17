#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/Archive/ArchiveBuilder.h>
#include <Foundation/IO/Archive/ArchiveUtils.h>
#include <Foundation/IO/CompressedStreamZstd.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Time/Stopwatch.h>

void wdArchiveBuilder::AddFolder(wdStringView sAbsFolderPath, wdArchiveCompressionMode defaultMode /*= wdArchiveCompressionMode::Uncompressed*/, InclusionCallback callback /*= InclusionCallback()*/)
{
#if WD_ENABLED(WD_SUPPORTS_FILE_ITERATORS)
  wdFileSystemIterator fileIt;

  wdStringBuilder sBasePath = sAbsFolderPath;
  sBasePath.MakeCleanPath();

  wdStringBuilder fullPath;
  wdStringBuilder relPath;

  for (fileIt.StartSearch(sBasePath, wdFileSystemIteratorFlags::ReportFilesRecursive); fileIt.IsValid(); fileIt.Next())
  {
    const auto& stat = fileIt.GetStats();

    stat.GetFullPath(fullPath);
    relPath = fullPath;

    if (relPath.MakeRelativeTo(sBasePath).Succeeded())
    {
      wdArchiveCompressionMode compression = defaultMode;
      wdInt32 iCompressionLevel = 0;

      if (callback.IsValid())
      {
        switch (callback(fullPath))
        {
          case InclusionMode::Exclude:
            continue;

          case InclusionMode::Uncompressed:
            compression = wdArchiveCompressionMode::Uncompressed;
            break;

          case InclusionMode::Compress_zstd_fastest:
            compression = wdArchiveCompressionMode::Compressed_zstd;
            iCompressionLevel = wdCompressedStreamWriterZstd::Compression::Fastest;
            break;
          case InclusionMode::Compress_zstd_fast:
            compression = wdArchiveCompressionMode::Compressed_zstd;
            iCompressionLevel = wdCompressedStreamWriterZstd::Compression::Fast;
            break;
          case InclusionMode::Compress_zstd_average:
            compression = wdArchiveCompressionMode::Compressed_zstd;
            iCompressionLevel = wdCompressedStreamWriterZstd::Compression::Average;
            break;
          case InclusionMode::Compress_zstd_high:
            compression = wdArchiveCompressionMode::Compressed_zstd;
            iCompressionLevel = wdCompressedStreamWriterZstd::Compression::High;
            break;
          case InclusionMode::Compress_zstd_highest:
            compression = wdArchiveCompressionMode::Compressed_zstd;
            iCompressionLevel = wdCompressedStreamWriterZstd::Compression::Highest;
            break;
        }
      }

      auto& e = m_Entries.ExpandAndGetRef();
      e.m_sAbsSourcePath = fullPath;
      e.m_sRelTargetPath = relPath;
      e.m_CompressionMode = compression;
      e.m_iCompressionLevel = iCompressionLevel;
    }
  }

#else
  WD_ASSERT_NOT_IMPLEMENTED;
#endif
}

wdResult wdArchiveBuilder::WriteArchive(wdStringView sFile) const
{
  WD_LOG_BLOCK("WriteArchive", sFile);

  wdFileWriter file;
  if (file.Open(sFile, 1024 * 1024 * 16).Failed())
  {
    wdLog::Error("Could not open file for writing archive to: '{}'", sFile);
    return WD_FAILURE;
  }

  return WriteArchive(file);
}

wdResult wdArchiveBuilder::WriteArchive(wdStreamWriter& inout_stream) const
{
  WD_SUCCEED_OR_RETURN(wdArchiveUtils::WriteHeader(inout_stream));

  wdArchiveTOC toc;

  wdStringBuilder sHashablePath;

  wdUInt64 uiStreamSize = 0;
  const wdUInt32 uiNumEntries = m_Entries.GetCount();

  wdStopwatch sw;

  for (wdUInt32 i = 0; i < uiNumEntries; ++i)
  {
    const SourceEntry& e = m_Entries[i];

    const wdUInt32 uiPathStringOffset = toc.m_AllPathStrings.GetCount();
    toc.m_AllPathStrings.PushBackRange(wdArrayPtr<const wdUInt8>(reinterpret_cast<const wdUInt8*>(e.m_sRelTargetPath.GetData()), e.m_sRelTargetPath.GetElementCount() + 1));

    sHashablePath = e.m_sRelTargetPath;
    sHashablePath.ToLower();

    toc.m_PathToEntryIndex[wdArchiveStoredString(wdHashingUtils::StringHash(sHashablePath), uiPathStringOffset)] = toc.m_Entries.GetCount();

    if (!WriteNextFileCallback(i + 1, uiNumEntries, e.m_sAbsSourcePath))
      return WD_FAILURE;

    wdArchiveEntry& tocEntry = toc.m_Entries.ExpandAndGetRef();

    WD_SUCCEED_OR_RETURN(wdArchiveUtils::WriteEntryOptimal(inout_stream, e.m_sAbsSourcePath, uiPathStringOffset, e.m_CompressionMode, e.m_iCompressionLevel, tocEntry, uiStreamSize, wdMakeDelegate(&wdArchiveBuilder::WriteFileProgressCallback, this)));

    WriteFileResultCallback(i + 1, uiNumEntries, e.m_sAbsSourcePath, tocEntry.m_uiUncompressedDataSize, tocEntry.m_uiStoredDataSize, sw.Checkpoint());
  }

  WD_SUCCEED_OR_RETURN(wdArchiveUtils::AppendTOC(inout_stream, toc));

  return WD_SUCCESS;
}

bool wdArchiveBuilder::WriteNextFileCallback(wdUInt32 uiCurEntry, wdUInt32 uiMaxEntries, wdStringView sSourceFile) const
{
  return true;
}

bool wdArchiveBuilder::WriteFileProgressCallback(wdUInt64 bytesWritten, wdUInt64 bytesTotal) const
{
  return true;
}

WD_STATICLINK_FILE(Foundation, Foundation_IO_Archive_Implementation_ArchiveBuilder);

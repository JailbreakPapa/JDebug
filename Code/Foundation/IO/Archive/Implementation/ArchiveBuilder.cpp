#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/Archive/ArchiveBuilder.h>
#include <Foundation/IO/Archive/ArchiveUtils.h>
#include <Foundation/IO/CompressedStreamZstd.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Time/Stopwatch.h>

void nsArchiveBuilder::AddFolder(nsStringView sAbsFolderPath, nsArchiveCompressionMode defaultMode /*= nsArchiveCompressionMode::Uncompressed*/, InclusionCallback callback /*= InclusionCallback()*/)
{
#if NS_ENABLED(NS_SUPPORTS_FILE_ITERATORS)
  nsFileSystemIterator fileIt;

  nsStringBuilder sBasePath = sAbsFolderPath;
  sBasePath.MakeCleanPath();

  nsStringBuilder fullPath;
  nsStringBuilder relPath;

  for (fileIt.StartSearch(sBasePath, nsFileSystemIteratorFlags::ReportFilesRecursive); fileIt.IsValid(); fileIt.Next())
  {
    const auto& stat = fileIt.GetStats();

    stat.GetFullPath(fullPath);
    relPath = fullPath;

    if (relPath.MakeRelativeTo(sBasePath).Succeeded())
    {
      nsArchiveCompressionMode compression = defaultMode;
      nsInt32 iCompressionLevel = 0;

      if (callback.IsValid())
      {
        switch (callback(fullPath))
        {
          case InclusionMode::Exclude:
            continue;

          case InclusionMode::Uncompressed:
            compression = nsArchiveCompressionMode::Uncompressed;
            break;

#  ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
          case InclusionMode::Compress_zstd_fastest:
            compression = nsArchiveCompressionMode::Compressed_zstd;
            iCompressionLevel = static_cast<nsInt32>(nsCompressedStreamWriterZstd::Compression::Fastest);
            break;
          case InclusionMode::Compress_zstd_fast:
            compression = nsArchiveCompressionMode::Compressed_zstd;
            iCompressionLevel = static_cast<nsInt32>(nsCompressedStreamWriterZstd::Compression::Fast);
            break;
          case InclusionMode::Compress_zstd_average:
            compression = nsArchiveCompressionMode::Compressed_zstd;
            iCompressionLevel = static_cast<nsInt32>(nsCompressedStreamWriterZstd::Compression::Average);
            break;
          case InclusionMode::Compress_zstd_high:
            compression = nsArchiveCompressionMode::Compressed_zstd;
            iCompressionLevel = static_cast<nsInt32>(nsCompressedStreamWriterZstd::Compression::High);
            break;
          case InclusionMode::Compress_zstd_highest:
            compression = nsArchiveCompressionMode::Compressed_zstd;
            iCompressionLevel = static_cast<nsInt32>(nsCompressedStreamWriterZstd::Compression::Highest);
            break;
#  endif
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
  NS_ASSERT_NOT_IMPLEMENTED;
#endif
}

nsResult nsArchiveBuilder::WriteArchive(nsStringView sFile) const
{
  NS_LOG_BLOCK("WriteArchive", sFile);

  nsFileWriter file;
  if (file.Open(sFile, 1024 * 1024 * 16).Failed())
  {
    nsLog::Error("Could not open file for writing archive to: '{}'", sFile);
    return NS_FAILURE;
  }

  return WriteArchive(file);
}

nsResult nsArchiveBuilder::WriteArchive(nsStreamWriter& inout_stream) const
{
  NS_SUCCEED_OR_RETURN(nsArchiveUtils::WriteHeader(inout_stream));

  nsArchiveTOC toc;

  nsStringBuilder sHashablePath;

  nsUInt64 uiStreamSize = 0;
  const nsUInt32 uiNumEntries = m_Entries.GetCount();

  nsStopwatch sw;

  for (nsUInt32 i = 0; i < uiNumEntries; ++i)
  {
    const SourceEntry& e = m_Entries[i];

    const nsUInt32 uiPathStringOffset = toc.AddPathString(e.m_sRelTargetPath);

    sHashablePath = e.m_sRelTargetPath;
    sHashablePath.ToLower();

    toc.m_PathToEntryIndex[nsArchiveStoredString(nsHashingUtils::StringHash(sHashablePath), uiPathStringOffset)] = toc.m_Entries.GetCount();

    if (!WriteNextFileCallback(i + 1, uiNumEntries, e.m_sAbsSourcePath))
      return NS_FAILURE;

    nsArchiveEntry& tocEntry = toc.m_Entries.ExpandAndGetRef();

    NS_SUCCEED_OR_RETURN(nsArchiveUtils::WriteEntryOptimal(inout_stream, e.m_sAbsSourcePath, uiPathStringOffset, e.m_CompressionMode, e.m_iCompressionLevel, tocEntry, uiStreamSize, nsMakeDelegate(&nsArchiveBuilder::WriteFileProgressCallback, this)));

    WriteFileResultCallback(i + 1, uiNumEntries, e.m_sAbsSourcePath, tocEntry.m_uiUncompressedDataSize, tocEntry.m_uiStoredDataSize, sw.Checkpoint());
  }

  NS_SUCCEED_OR_RETURN(nsArchiveUtils::AppendTOC(inout_stream, toc));

  return NS_SUCCESS;
}

bool nsArchiveBuilder::WriteNextFileCallback(nsUInt32 uiCurEntry, nsUInt32 uiMaxEntries, nsStringView sSourceFile) const
{
  return true;
}

bool nsArchiveBuilder::WriteFileProgressCallback(nsUInt64 bytesWritten, nsUInt64 bytesTotal) const
{
  return true;
}

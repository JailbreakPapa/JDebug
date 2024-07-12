#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/Archive/ArchiveUtils.h>

#include <Foundation/Algorithm/HashStream.h>
#include <Foundation/IO/CompressedStreamZlib.h>
#include <Foundation/IO/CompressedStreamZstd.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/MemoryMappedFile.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Logging/Log.h>

nsHybridArray<nsString, 4, nsStaticsAllocatorWrapper>& nsArchiveUtils::GetAcceptedArchiveFileExtensions()
{
  static nsHybridArray<nsString, 4, nsStaticsAllocatorWrapper> extensions;

  if (extensions.IsEmpty())
  {
    extensions.PushBack("nsArchive");
  }

  return extensions;
}

bool nsArchiveUtils::IsAcceptedArchiveFileExtensions(nsStringView sExtension)
{
  for (const auto& ext : GetAcceptedArchiveFileExtensions())
  {
    if (sExtension.IsEqual_NoCase(ext.GetView()))
      return true;
  }

  return false;
}

nsResult nsArchiveUtils::WriteHeader(nsStreamWriter& inout_stream)
{
  static_assert(16 == ArchiveHeaderSize);

  const char* szTag = "NSARCHIVE";
  NS_SUCCEED_OR_RETURN(inout_stream.WriteBytes(szTag, 10));

  const nsUInt8 uiArchiveVersion = 4;

  // Version 2: Added end-of-file marker for file corruption (cutoff) detection
  // Version 3: HashedStrings changed from MurmurHash to xxHash
  // Version 4: use 64 Bit string hashes
  inout_stream << uiArchiveVersion;

  const nsUInt8 uiPadding[5] = {0, 0, 0, 0, 0};
  NS_SUCCEED_OR_RETURN(inout_stream.WriteBytes(uiPadding, 5));

  return NS_SUCCESS;
}

nsResult nsArchiveUtils::ReadHeader(nsStreamReader& inout_stream, nsUInt8& out_uiVersion)
{
  static_assert(16 == ArchiveHeaderSize);

  char szTag[10];
  if (inout_stream.ReadBytes(szTag, 10) != 10 || !nsStringUtils::IsEqual(szTag, "NSARCHIVE"))
  {
    nsLog::Error("Invalid or corrupted archive. Archive-marker not found.");
    return NS_FAILURE;
  }

  out_uiVersion = 0;
  inout_stream >> out_uiVersion;

  if (out_uiVersion != 1 && out_uiVersion != 2 && out_uiVersion != 3 && out_uiVersion != 4)
  {
    nsLog::Error("Unsupported archive version '{}'.", out_uiVersion);
    return NS_FAILURE;
  }

  nsUInt8 uiPadding[5] = {255, 255, 255, 255, 255};
  if (inout_stream.ReadBytes(uiPadding, 5) != 5)
  {
    nsLog::Error("Invalid or corrupted archive. Missing header data.");
    return NS_FAILURE;
  }

  const nsUInt8 uiZeroPadding[5] = {0, 0, 0, 0, 0};

  if (nsMemoryUtils::Compare<nsUInt8>(uiPadding, uiZeroPadding, 5) != 0)
  {
    nsLog::Error("Invalid or corrupted archive. Unexpected header data.");
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

nsResult nsArchiveUtils::WriteEntryPreprocessed(nsStreamWriter& inout_stream, nsConstByteArrayPtr entryData, nsUInt32 uiPathStringOffset, nsArchiveCompressionMode compression, nsUInt32 uiUncompressedEntryDataSize, nsArchiveEntry& ref_tocEntry, nsUInt64& inout_uiCurrentStreamPosition)
{
  NS_SUCCEED_OR_RETURN(inout_stream.WriteBytes(entryData.GetPtr(), entryData.GetCount()));

  ref_tocEntry.m_uiPathStringOffset = uiPathStringOffset;
  ref_tocEntry.m_uiDataStartOffset = inout_uiCurrentStreamPosition;
  ref_tocEntry.m_uiUncompressedDataSize = uiUncompressedEntryDataSize;
  ref_tocEntry.m_uiStoredDataSize = entryData.GetCount();
  ref_tocEntry.m_CompressionMode = compression;

  inout_uiCurrentStreamPosition += entryData.GetCount();

  return NS_SUCCESS;
}

nsResult nsArchiveUtils::WriteEntry(
  nsStreamWriter& inout_stream, nsStringView sAbsSourcePath, nsUInt32 uiPathStringOffset, nsArchiveCompressionMode compression,
  nsInt32 iCompressionLevel, nsArchiveEntry& inout_tocEntry, nsUInt64& inout_uiCurrentStreamPosition, FileWriteProgressCallback progress /*= FileWriteProgressCallback()*/)
{
  nsFileReader file;
  NS_SUCCEED_OR_RETURN(file.Open(sAbsSourcePath, 1024 * 1024));

  const nsUInt64 uiMaxBytes = file.GetFileSize();

  constexpr nsUInt32 uiMaxNumWorkerThreads = 12u;

  nsUInt8 buf[1024 * 32];

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
  nsUInt32 uiWorkerThreadCount;
  if (uiMaxBytes > nsMath::MaxValue<nsUInt32>())
  {
    uiWorkerThreadCount = uiMaxNumWorkerThreads;
  }
  else
  {
    constexpr nsUInt32 uiBytesPerThread = 1024u * 1024u;
    uiWorkerThreadCount = nsMath::Clamp((nsUInt32)floor(uiMaxBytes / uiBytesPerThread), 1u, uiMaxNumWorkerThreads);
  }
#endif

  inout_tocEntry.m_uiPathStringOffset = uiPathStringOffset;
  inout_tocEntry.m_uiDataStartOffset = inout_uiCurrentStreamPosition;
  inout_tocEntry.m_uiUncompressedDataSize = 0;

  nsStreamWriter* pWriter = &inout_stream;

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
  nsCompressedStreamWriterZstd zstdWriter;
#endif

  switch (compression)
  {
    case nsArchiveCompressionMode::Uncompressed:
      break;

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
    case nsArchiveCompressionMode::Compressed_zstd:
    {
      zstdWriter.SetOutputStream(&inout_stream, uiWorkerThreadCount, (nsCompressedStreamWriterZstd::Compression)iCompressionLevel);
      pWriter = &zstdWriter;
    }
    break;
#endif

    default:
      compression = nsArchiveCompressionMode::Uncompressed;
      break;
  }

  inout_tocEntry.m_CompressionMode = compression;

  nsUInt64 uiRead = 0;
  while (true)
  {
    uiRead = file.ReadBytes(buf, NS_ARRAY_SIZE(buf));

    if (uiRead == 0)
      break;

    inout_tocEntry.m_uiUncompressedDataSize += uiRead;

    if (progress.IsValid())
    {
      if (!progress(inout_tocEntry.m_uiUncompressedDataSize, uiMaxBytes))
        return NS_FAILURE;
    }

    NS_SUCCEED_OR_RETURN(pWriter->WriteBytes(buf, uiRead));
  }


  switch (compression)
  {
#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
    case nsArchiveCompressionMode::Compressed_zstd:
      NS_SUCCEED_OR_RETURN(zstdWriter.FinishCompressedStream());
      inout_tocEntry.m_uiStoredDataSize = zstdWriter.GetWrittenBytes();
      break;
#endif

    case nsArchiveCompressionMode::Uncompressed:
    default:
      inout_tocEntry.m_uiStoredDataSize = inout_tocEntry.m_uiUncompressedDataSize;
      break;
  }

  inout_uiCurrentStreamPosition += inout_tocEntry.m_uiStoredDataSize;

  return NS_SUCCESS;
}

nsResult nsArchiveUtils::WriteEntryOptimal(nsStreamWriter& inout_stream, nsStringView sAbsSourcePath, nsUInt32 uiPathStringOffset, nsArchiveCompressionMode compression, nsInt32 iCompressionLevel, nsArchiveEntry& ref_tocEntry, nsUInt64& inout_uiCurrentStreamPosition, FileWriteProgressCallback progress /*= FileWriteProgressCallback()*/)
{
  if (compression == nsArchiveCompressionMode::Uncompressed)
  {
    return WriteEntry(inout_stream, sAbsSourcePath, uiPathStringOffset, nsArchiveCompressionMode::Uncompressed, iCompressionLevel, ref_tocEntry, inout_uiCurrentStreamPosition, progress);
  }
  else
  {
    nsDefaultMemoryStreamStorage storage;
    nsMemoryStreamWriter writer(&storage);

    nsUInt64 streamPos = inout_uiCurrentStreamPosition;
    NS_SUCCEED_OR_RETURN(WriteEntry(writer, sAbsSourcePath, uiPathStringOffset, compression, iCompressionLevel, ref_tocEntry, streamPos, progress));

    if (ref_tocEntry.m_uiStoredDataSize * 12 >= ref_tocEntry.m_uiUncompressedDataSize * 10)
    {
      // less than 20% size saving -> go uncompressed
      return WriteEntry(inout_stream, sAbsSourcePath, uiPathStringOffset, nsArchiveCompressionMode::Uncompressed, iCompressionLevel, ref_tocEntry, inout_uiCurrentStreamPosition, progress);
    }
    else
    {
      auto res = storage.CopyToStream(inout_stream);
      inout_uiCurrentStreamPosition = streamPos;

      return res;
    }
  }
}

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT

class nsCompressedStreamReaderZstdWithSource : public nsCompressedStreamReaderZstd
{
public:
  nsRawMemoryStreamReader m_Source;
};

#endif

#ifdef BUILDSYSTEM_ENABLE_ZLIB_SUPPORT

class nsCompressedStreamReaderZipWithSource : public nsCompressedStreamReaderZip
{
public:
  nsRawMemoryStreamReader m_Source;
};

#endif

nsUniquePtr<nsStreamReader> nsArchiveUtils::CreateEntryReader(const nsArchiveEntry& entry, const void* pStartOfArchiveData)
{
  nsUniquePtr<nsStreamReader> reader;

  switch (entry.m_CompressionMode)
  {
    case nsArchiveCompressionMode::Uncompressed:
    {
      reader = NS_DEFAULT_NEW(nsRawMemoryStreamReader);
      nsRawMemoryStreamReader* pRawReader = static_cast<nsRawMemoryStreamReader*>(reader.Borrow());
      ConfigureRawMemoryStreamReader(entry, pStartOfArchiveData, *pRawReader);
      break;
    }

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
    case nsArchiveCompressionMode::Compressed_zstd:
    {
      reader = NS_DEFAULT_NEW(nsCompressedStreamReaderZstdWithSource);
      nsCompressedStreamReaderZstdWithSource* pRawReader = static_cast<nsCompressedStreamReaderZstdWithSource*>(reader.Borrow());
      ConfigureRawMemoryStreamReader(entry, pStartOfArchiveData, pRawReader->m_Source);
      pRawReader->SetInputStream(&pRawReader->m_Source);
      break;
    }
#endif
#ifdef BUILDSYSTEM_ENABLE_ZLIB_SUPPORT
    case nsArchiveCompressionMode::Compressed_zip:
    {
      reader = NS_DEFAULT_NEW(nsCompressedStreamReaderZipWithSource);
      nsCompressedStreamReaderZipWithSource* pRawReader = static_cast<nsCompressedStreamReaderZipWithSource*>(reader.Borrow());
      ConfigureRawMemoryStreamReader(entry, pStartOfArchiveData, pRawReader->m_Source);
      pRawReader->SetInputStream(&pRawReader->m_Source, entry.m_uiStoredDataSize);
      break;
    }
#endif

    default:
      NS_REPORT_FAILURE("Archive entry compression mode '{}' is not supported by nsArchiveReader", (int)entry.m_CompressionMode);
      break;
  }

  return std::move(reader);
}

void nsArchiveUtils::ConfigureRawMemoryStreamReader(const nsArchiveEntry& entry, const void* pStartOfArchiveData, nsRawMemoryStreamReader& ref_memReader)
{
  ref_memReader.Reset(nsMemoryUtils::AddByteOffset(pStartOfArchiveData, static_cast<std::ptrdiff_t>(entry.m_uiDataStartOffset)), entry.m_uiStoredDataSize);
}

static const char* szEndMarker = "NSARCHIVE-END";

static nsUInt32 GetEndMarkerSize(nsUInt8 uiFileVersion)
{
  if (uiFileVersion == 1)
    return 0;

  return 14;
}

static nsUInt32 GetTocMetaSize(nsUInt8 uiFileVersion)
{
  if (uiFileVersion == 1)
    return sizeof(nsUInt32); // TOC size

  return sizeof(nsUInt32) /* TOC size */ + sizeof(nsUInt64) /* TOC hash */;
}

struct TocMetaData
{
  nsUInt32 m_uiSize = 0;
  nsUInt64 m_uiHash = 0;
};

nsResult nsArchiveUtils::AppendTOC(nsStreamWriter& inout_stream, const nsArchiveTOC& toc)
{
  nsDefaultMemoryStreamStorage storage;
  nsMemoryStreamWriter writer(&storage);

  NS_SUCCEED_OR_RETURN(toc.Serialize(writer));

  NS_SUCCEED_OR_RETURN(storage.CopyToStream(inout_stream));

  TocMetaData tocMeta;

  nsHashStreamWriter64 hashStream(tocMeta.m_uiSize);
  NS_SUCCEED_OR_RETURN(storage.CopyToStream(hashStream));

  // Added in file version 2: hash of the TOC
  tocMeta.m_uiSize = storage.GetStorageSize32();
  tocMeta.m_uiHash = hashStream.GetHashValue();

  // append the TOC meta data
  inout_stream << tocMeta.m_uiSize;
  inout_stream << tocMeta.m_uiHash;

  // write an 'end' marker
  return inout_stream.WriteBytes(szEndMarker, 14);
}

static nsResult VerifyEndMarker(nsUInt64 uiArchiveDataSize, const void* pArchiveDataBuffer, nsUInt8 uiArchiveVersion)
{
  const nsUInt32 uiEndMarkerSize = GetEndMarkerSize(uiArchiveVersion);

  if (uiEndMarkerSize == 0)
  {
    return NS_SUCCESS;
  }

  if (uiEndMarkerSize > uiArchiveDataSize)
  {
    nsLog::Error("Archive is too small. End-marker not found.");
    return NS_FAILURE;
  }

  const void* pStart = nsMemoryUtils::AddByteOffset(pArchiveDataBuffer, uiArchiveDataSize - uiEndMarkerSize);

  nsRawMemoryStreamReader reader(pStart, uiEndMarkerSize);

  char szMarker[32] = "";
  if (reader.ReadBytes(szMarker, uiEndMarkerSize) != uiEndMarkerSize || !nsStringUtils::IsEqual(szMarker, szEndMarker))
  {
    nsLog::Error("Archive is corrupt or cut off. End-marker not found.");
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

nsResult nsArchiveUtils::ExtractTOCMeta(nsUInt64 uiArchiveEndingDataSize, const void* pArchiveEndingDataBuffer, TOCMeta& ref_tocMeta, nsUInt8 uiArchiveVersion)
{
  NS_SUCCEED_OR_RETURN(VerifyEndMarker(uiArchiveEndingDataSize, pArchiveEndingDataBuffer, uiArchiveVersion));

  const nsUInt32 uiEndMarkerSize = GetEndMarkerSize(uiArchiveVersion);
  const nsUInt32 uiTocMetaSize = GetTocMetaSize(uiArchiveVersion);

  nsUInt32 uiTocSize = 0;
  nsUInt64 uiExpectedTocHash = 0;

  // read the TOC meta data
  {
    NS_ASSERT_DEV(uiEndMarkerSize + uiTocMetaSize <= ArchiveTOCMetaMaxFooterSize, "");

    if (uiEndMarkerSize + uiTocMetaSize > uiArchiveEndingDataSize)
    {
      nsLog::Error("Unable to extract Archive TOC. File size too small: {0}", nsArgFileSize(uiArchiveEndingDataSize));
      return NS_FAILURE;
    }

    const void* pTocMetaStart = nsMemoryUtils::AddByteOffset(pArchiveEndingDataBuffer, uiArchiveEndingDataSize - uiEndMarkerSize - uiTocMetaSize);

    nsRawMemoryStreamReader tocMetaReader(pTocMetaStart, uiTocMetaSize);

    tocMetaReader >> uiTocSize;

    if (uiTocSize > 1024 * 1024 * 1024) // 1GB of TOC is enough for ~16M entries...
    {
      nsLog::Error("Archive TOC is probably corrupted. Unreasonable TOC size: {0}", nsArgFileSize(uiTocSize));
      return NS_FAILURE;
    }

    if (uiArchiveVersion >= 2)
    {
      tocMetaReader >> uiExpectedTocHash;
    }
  }

  // output the result
  {
    ref_tocMeta = TOCMeta();
    ref_tocMeta.m_uiTocSize = uiTocSize;
    ref_tocMeta.m_uiExpectedTocHash = uiExpectedTocHash;
    ref_tocMeta.m_uiTocOffsetFromArchiveEnd = uiTocSize + uiTocMetaSize + uiEndMarkerSize;
  }

  return NS_SUCCESS;
}

nsResult nsArchiveUtils::ExtractTOCMeta(const nsMemoryMappedFile& memFile, TOCMeta& ref_tocMeta, nsUInt8 uiArchiveVersion)
{
  return ExtractTOCMeta(memFile.GetFileSize(), memFile.GetReadPointer(), ref_tocMeta, uiArchiveVersion);
}

nsResult nsArchiveUtils::ExtractTOC(nsUInt64 uiArchiveEndingDataSize, const void* pArchiveEndingDataBuffer, nsArchiveTOC& ref_toc, nsUInt8 uiArchiveVersion)
{
  // get toc meta
  TOCMeta tocMeta;
  if (ExtractTOCMeta(uiArchiveEndingDataSize, pArchiveEndingDataBuffer, tocMeta, uiArchiveVersion).Failed())
  {
    return NS_FAILURE;
  }

  // verify meta is valid
  if (tocMeta.m_uiTocOffsetFromArchiveEnd > uiArchiveEndingDataSize)
  {
    nsLog::Error("Archive TOC offset is corrupted.");
    return NS_FAILURE;
  }

  // get toc data ptr
  const void* pTocStart = nsMemoryUtils::AddByteOffset(pArchiveEndingDataBuffer, uiArchiveEndingDataSize - tocMeta.m_uiTocOffsetFromArchiveEnd);

  // validate the TOC hash
  if (uiArchiveVersion >= 2)
  {
    const nsUInt64 uiActualTocHash = nsHashingUtils::xxHash64(pTocStart, tocMeta.m_uiTocSize);
    if (tocMeta.m_uiExpectedTocHash != uiActualTocHash)
    {
      nsLog::Error("Archive TOC is corrupted. Hashes do not match.");
      return NS_FAILURE;
    }
  }

  // read the actual TOC data
  {
    nsRawMemoryStreamReader tocReader(pTocStart, tocMeta.m_uiTocSize);

    if (ref_toc.Deserialize(tocReader, uiArchiveVersion).Failed())
    {
      nsLog::Error("Failed to deserialize nsArchive TOC");
      return NS_FAILURE;
    }
  }

  return NS_SUCCESS;
}

nsResult nsArchiveUtils::ExtractTOC(const nsMemoryMappedFile& memFile, nsArchiveTOC& ref_toc, nsUInt8 uiArchiveVersion)
{
  return ExtractTOC(memFile.GetFileSize(), memFile.GetReadPointer(), ref_toc, uiArchiveVersion);
}

namespace ZipFormat
{
  constexpr nsUInt32 EndOfCDMagicSignature = 0x06054b50;
  constexpr nsUInt32 EndOfCDHeaderLength = 22;

  constexpr nsUInt32 MaxCommentLength = 65535;
  constexpr nsUInt64 MaxEndOfCDSearchLength = MaxCommentLength + EndOfCDHeaderLength;

  constexpr nsUInt32 LocalFileMagicSignature = 0x04034b50;
  constexpr nsUInt32 LocalFileHeaderLength = 30;

  constexpr nsUInt32 CDFileMagicSignature = 0x02014b50;
  constexpr nsUInt32 CDFileHeaderLength = 46;

  enum CompressionType
  {
    Uncompressed = 0,
    Deflate = 8,

  };

  struct EndOfCDHeader
  {
    nsUInt32 signature;
    nsUInt16 diskNumber;
    nsUInt16 diskWithCD;
    nsUInt16 diskEntries;
    nsUInt16 totalEntries;
    nsUInt32 cdSize;
    nsUInt32 cdOffset;
    nsUInt16 commentLength;
  };

  nsStreamReader& operator>>(nsStreamReader& inout_stream, EndOfCDHeader& ref_value)
  {
    inout_stream >> ref_value.signature >> ref_value.diskNumber >> ref_value.diskWithCD >> ref_value.diskEntries >> ref_value.totalEntries >> ref_value.cdSize;
    inout_stream >> ref_value.cdOffset >> ref_value.commentLength;
    NS_ASSERT_DEBUG(ref_value.signature == EndOfCDMagicSignature, "ZIP: Corrupt end of central directory header.");
    return inout_stream;
  }

  struct CDFileHeader
  {
    nsUInt32 signature;
    nsUInt16 version;
    nsUInt16 versionNeeded;
    nsUInt16 flags;
    nsUInt16 compression;
    nsUInt16 modTime;
    nsUInt16 modDate;
    nsUInt32 crc32;
    nsUInt32 compressedSize;
    nsUInt32 uncompressedSize;
    nsUInt16 fileNameLength;
    nsUInt16 extraFieldLength;
    nsUInt16 fileCommentLength;
    nsUInt16 diskNumStart;
    nsUInt16 internalAttr;
    nsUInt32 externalAttr;
    nsUInt32 offsetLocalHeader;
  };

  nsStreamReader& operator>>(nsStreamReader& inout_stream, CDFileHeader& ref_value)
  {
    inout_stream >> ref_value.signature >> ref_value.version >> ref_value.versionNeeded >> ref_value.flags >> ref_value.compression >> ref_value.modTime >> ref_value.modDate;
    inout_stream >> ref_value.crc32 >> ref_value.compressedSize >> ref_value.uncompressedSize >> ref_value.fileNameLength >> ref_value.extraFieldLength;
    inout_stream >> ref_value.fileCommentLength >> ref_value.diskNumStart >> ref_value.internalAttr >> ref_value.externalAttr >> ref_value.offsetLocalHeader;
    NS_IGNORE_UNUSED(CDFileMagicSignature);
    NS_ASSERT_DEBUG(ref_value.signature == CDFileMagicSignature, "ZIP: Corrupt central directory file entry header.");
    return inout_stream;
  }

  struct LocalFileHeader
  {
    nsUInt32 signature;
    nsUInt16 version;
    nsUInt16 flags;
    nsUInt16 compression;
    nsUInt16 modTime;
    nsUInt16 modDate;
    nsUInt32 crc32;
    nsUInt32 compressedSize;
    nsUInt32 uncompressedSize;
    nsUInt16 fileNameLength;
    nsUInt16 extraFieldLength;
  };

  nsStreamReader& operator>>(nsStreamReader& inout_stream, LocalFileHeader& ref_value)
  {
    inout_stream >> ref_value.signature >> ref_value.version >> ref_value.flags >> ref_value.compression >> ref_value.modTime >> ref_value.modDate >> ref_value.crc32;
    inout_stream >> ref_value.compressedSize >> ref_value.uncompressedSize >> ref_value.fileNameLength >> ref_value.extraFieldLength;
    NS_ASSERT_DEBUG(ref_value.signature == LocalFileMagicSignature, "ZIP: Corrupt local file entry header.");
    return inout_stream;
  }
}; // namespace ZipFormat

nsResult nsArchiveUtils::ReadZipHeader(nsStreamReader& inout_stream, nsUInt8& out_uiVersion)
{
  using namespace ZipFormat;

  nsUInt32 header;
  inout_stream >> header;
  if (header == LocalFileMagicSignature)
  {
    out_uiVersion = 0;
    return NS_SUCCESS;
  }
  return NS_SUCCESS;
}

nsResult nsArchiveUtils::ExtractZipTOC(const nsMemoryMappedFile& memFile, nsArchiveTOC& ref_toc)
{
  using namespace ZipFormat;

  const nsUInt8* pEndOfCDStart = nullptr;
  {
    // Find End of CD signature by searching from the end of the file.
    // As a comment can come after it we have to potentially walk max comment length backwards.
    const nsUInt64 SearchEnd = memFile.GetFileSize() - nsMath::Min(MaxEndOfCDSearchLength, memFile.GetFileSize());
    const nsUInt8* pSearchEnd = static_cast<const nsUInt8*>(memFile.GetReadPointer(SearchEnd, nsMemoryMappedFile::OffsetBase::End));
    const nsUInt8* pSearchStart = static_cast<const nsUInt8*>(memFile.GetReadPointer(EndOfCDHeaderLength, nsMemoryMappedFile::OffsetBase::End));
    while (pSearchStart >= pSearchEnd)
    {
      if (*reinterpret_cast<const nsUInt32*>(pSearchStart) == EndOfCDMagicSignature)
      {
        pEndOfCDStart = pSearchStart;
        break;
      }
      pSearchStart--;
    }
    if (pEndOfCDStart == nullptr)
      return NS_FAILURE;
  }

  nsRawMemoryStreamReader tocReader(pEndOfCDStart, EndOfCDHeaderLength);
  EndOfCDHeader ecdHeader;
  tocReader >> ecdHeader;

  ref_toc.m_Entries.Reserve(ecdHeader.diskEntries);
  ref_toc.m_PathToEntryIndex.Reserve(ecdHeader.diskEntries);

  nsStringBuilder sLowerCaseHash;
  nsUInt64 uiEntryOffset = 0;
  for (nsUInt16 uiEntry = 0; uiEntry < ecdHeader.diskEntries; ++uiEntry)
  {
    // First, read the current file's header from the central directory
    const void* pCdfStart = memFile.GetReadPointer(ecdHeader.cdOffset + uiEntryOffset, nsMemoryMappedFile::OffsetBase::Start);
    nsRawMemoryStreamReader cdfReader(pCdfStart, ecdHeader.cdSize - uiEntryOffset);
    CDFileHeader cdfHeader;
    cdfReader >> cdfHeader;

    if (cdfHeader.compression == CompressionType::Uncompressed || cdfHeader.compression == CompressionType::Deflate)
    {
      auto& entry = ref_toc.m_Entries.ExpandAndGetRef();
      entry.m_uiUncompressedDataSize = cdfHeader.uncompressedSize;
      entry.m_uiStoredDataSize = cdfHeader.compressedSize;
      entry.m_uiPathStringOffset = ref_toc.m_AllPathStrings.GetCount();
      entry.m_CompressionMode = cdfHeader.compression == CompressionType::Uncompressed ? nsArchiveCompressionMode::Uncompressed : nsArchiveCompressionMode::Compressed_zip;

      auto nameBuffer = nsArrayPtr<const nsUInt8>(static_cast<const nsUInt8*>(pCdfStart) + CDFileHeaderLength, cdfHeader.fileNameLength);
      ref_toc.m_AllPathStrings.PushBackRange(nameBuffer);
      ref_toc.m_AllPathStrings.PushBack(0);
      const char* szName = reinterpret_cast<const char*>(ref_toc.m_AllPathStrings.GetData() + entry.m_uiPathStringOffset);
      sLowerCaseHash = szName;
      sLowerCaseHash.ToLower();
      ref_toc.m_PathToEntryIndex.Insert(nsArchiveStoredString(nsHashingUtils::StringHash(sLowerCaseHash), entry.m_uiPathStringOffset), ref_toc.m_Entries.GetCount() - 1);

      // Compute data stream start location. We need to skip past the local (and redundant) file header to find it.
      const void* pLfStart = memFile.GetReadPointer(cdfHeader.offsetLocalHeader, nsMemoryMappedFile::OffsetBase::Start);
      nsRawMemoryStreamReader lfReader(pLfStart, memFile.GetFileSize() - cdfHeader.offsetLocalHeader);
      LocalFileHeader lfHeader;
      lfReader >> lfHeader;
      entry.m_uiDataStartOffset = cdfHeader.offsetLocalHeader + LocalFileHeaderLength + lfHeader.fileNameLength + lfHeader.extraFieldLength;
    }
    // Compute next file header location.
    uiEntryOffset += CDFileHeaderLength + cdfHeader.fileNameLength + cdfHeader.extraFieldLength + cdfHeader.fileCommentLength;
  }

  return NS_SUCCESS;
}

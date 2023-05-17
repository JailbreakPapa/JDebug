#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/Archive/ArchiveUtils.h>

#include <Foundation/Algorithm/HashStream.h>
#include <Foundation/IO/CompressedStreamZstd.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/MemoryMappedFile.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Logging/Log.h>

wdHybridArray<wdString, 4, wdStaticAllocatorWrapper>& wdArchiveUtils::GetAcceptedArchiveFileExtensions()
{
  static wdHybridArray<wdString, 4, wdStaticAllocatorWrapper> extensions;

  if (extensions.IsEmpty())
  {
    extensions.PushBack("wdArchive");
  }

  return extensions;
}

bool wdArchiveUtils::IsAcceptedArchiveFileExtensions(wdStringView sExtension)
{
  for (const auto& ext : GetAcceptedArchiveFileExtensions())
  {
    if (sExtension.IsEqual_NoCase(ext.GetView()))
      return true;
  }

  return false;
}

wdResult wdArchiveUtils::WriteHeader(wdStreamWriter& inout_stream)
{
  const char* szTag = "WDARCHIVE";
  WD_SUCCEED_OR_RETURN(inout_stream.WriteBytes(szTag, 10));

  const wdUInt8 uiArchiveVersion = 4;

  // Version 2: Added end-of-file marker for file corruption (cutoff) detection
  // Version 3: HashedStrings changed from MurmurHash to xxHash
  // Version 4: use 64 Bit string hashes
  inout_stream << uiArchiveVersion;

  const wdUInt8 uiPadding[5] = {0, 0, 0, 0, 0};
  WD_SUCCEED_OR_RETURN(inout_stream.WriteBytes(uiPadding, 5));

  return WD_SUCCESS;
}

wdResult wdArchiveUtils::ReadHeader(wdStreamReader& inout_stream, wdUInt8& out_uiVersion)
{
  char szTag[10];
  if (inout_stream.ReadBytes(szTag, 10) != 10 || !wdStringUtils::IsEqual(szTag, "WDARCHIVE"))
  {
    wdLog::Error("Invalid or corrupted archive. Archive-marker not found.");
    return WD_FAILURE;
  }

  out_uiVersion = 0;
  inout_stream >> out_uiVersion;

  if (out_uiVersion != 1 && out_uiVersion != 2 && out_uiVersion != 3 && out_uiVersion != 4)
  {
    wdLog::Error("Unsupported archive version '{}'.", out_uiVersion);
    return WD_FAILURE;
  }

  wdUInt8 uiPadding[5] = {255, 255, 255, 255, 255};
  if (inout_stream.ReadBytes(uiPadding, 5) != 5)
  {
    wdLog::Error("Invalid or corrupted archive. Missing header data.");
    return WD_FAILURE;
  }

  const wdUInt8 uiZeroPadding[5] = {0, 0, 0, 0, 0};

  if (wdMemoryUtils::Compare<wdUInt8>(uiPadding, uiZeroPadding, 5) != 0)
  {
    wdLog::Error("Invalid or corrupted archive. Unexpected header data.");
    return WD_FAILURE;
  }

  return WD_SUCCESS;
}

wdResult wdArchiveUtils::WriteEntry(
  wdStreamWriter& inout_stream, wdStringView sAbsSourcePath, wdUInt32 uiPathStringOffset, wdArchiveCompressionMode compression,
  wdInt32 iCompressionLevel, wdArchiveEntry& inout_tocEntry, wdUInt64& inout_uiCurrentStreamPosition, FileWriteProgressCallback progress /*= FileWriteProgressCallback()*/)
{
  wdFileReader file;
  WD_SUCCEED_OR_RETURN(file.Open(sAbsSourcePath, 1024 * 1024));

  const wdUInt64 uiMaxBytes = file.GetFileSize();

  wdUInt8 uiTemp[1024 * 8];

  inout_tocEntry.m_uiPathStringOffset = uiPathStringOffset;
  inout_tocEntry.m_uiDataStartOffset = inout_uiCurrentStreamPosition;
  inout_tocEntry.m_uiUncompressedDataSize = 0;

  wdStreamWriter* pWriter = &inout_stream;

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
  wdCompressedStreamWriterZstd zstdWriter;
#endif

  switch (compression)
  {
    case wdArchiveCompressionMode::Uncompressed:
      break;

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
    case wdArchiveCompressionMode::Compressed_zstd:
      zstdWriter.SetOutputStream(&inout_stream, (wdCompressedStreamWriterZstd::Compression)iCompressionLevel);
      pWriter = &zstdWriter;
      break;
#endif

    default:
      compression = wdArchiveCompressionMode::Uncompressed;
      break;
  }

  inout_tocEntry.m_CompressionMode = compression;

  wdUInt64 uiRead = 0;
  while (true)
  {
    uiRead = file.ReadBytes(uiTemp, WD_ARRAY_SIZE(uiTemp));

    if (uiRead == 0)
      break;

    inout_tocEntry.m_uiUncompressedDataSize += uiRead;

    if (progress.IsValid())
    {
      if (!progress(inout_tocEntry.m_uiUncompressedDataSize, uiMaxBytes))
        return WD_FAILURE;
    }

    WD_SUCCEED_OR_RETURN(pWriter->WriteBytes(uiTemp, uiRead));
  }


  switch (compression)
  {
#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
    case wdArchiveCompressionMode::Compressed_zstd:
      WD_SUCCEED_OR_RETURN(zstdWriter.FinishCompressedStream());
      inout_tocEntry.m_uiStoredDataSize = zstdWriter.GetWrittenBytes();
      break;
#endif

    case wdArchiveCompressionMode::Uncompressed:
    default:
      inout_tocEntry.m_uiStoredDataSize = inout_tocEntry.m_uiUncompressedDataSize;
      break;
  }

  inout_uiCurrentStreamPosition += inout_tocEntry.m_uiStoredDataSize;

  return WD_SUCCESS;
}

wdResult wdArchiveUtils::WriteEntryOptimal(wdStreamWriter& inout_stream, wdStringView sAbsSourcePath, wdUInt32 uiPathStringOffset, wdArchiveCompressionMode compression, wdInt32 iCompressionLevel, wdArchiveEntry& ref_tocEntry, wdUInt64& inout_uiCurrentStreamPosition, FileWriteProgressCallback progress /*= FileWriteProgressCallback()*/)
{
  if (compression == wdArchiveCompressionMode::Uncompressed)
  {
    return WriteEntry(inout_stream, sAbsSourcePath, uiPathStringOffset, wdArchiveCompressionMode::Uncompressed, iCompressionLevel, ref_tocEntry, inout_uiCurrentStreamPosition, progress);
  }
  else
  {
    wdDefaultMemoryStreamStorage storage;
    wdMemoryStreamWriter writer(&storage);

    wdUInt64 streamPos = inout_uiCurrentStreamPosition;
    WD_SUCCEED_OR_RETURN(WriteEntry(writer, sAbsSourcePath, uiPathStringOffset, compression, iCompressionLevel, ref_tocEntry, streamPos, progress));

    if (ref_tocEntry.m_uiStoredDataSize * 12 >= ref_tocEntry.m_uiUncompressedDataSize * 10)
    {
      // less than 20% size saving -> go uncompressed
      return WriteEntry(inout_stream, sAbsSourcePath, uiPathStringOffset, wdArchiveCompressionMode::Uncompressed, iCompressionLevel, ref_tocEntry, inout_uiCurrentStreamPosition, progress);
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

class wdCompressedStreamReaderZstdWithSource : public wdCompressedStreamReaderZstd
{
public:
  wdRawMemoryStreamReader m_Source;
};

#endif


wdUniquePtr<wdStreamReader> wdArchiveUtils::CreateEntryReader(const wdArchiveEntry& entry, const void* pStartOfArchiveData)
{
  wdUniquePtr<wdStreamReader> reader;

  switch (entry.m_CompressionMode)
  {
    case wdArchiveCompressionMode::Uncompressed:
    {
      reader = WD_DEFAULT_NEW(wdRawMemoryStreamReader);
      wdRawMemoryStreamReader* pRawReader = static_cast<wdRawMemoryStreamReader*>(reader.Borrow());
      ConfigureRawMemoryStreamReader(entry, pStartOfArchiveData, *pRawReader);
      break;
    }

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
    case wdArchiveCompressionMode::Compressed_zstd:
    {
      reader = WD_DEFAULT_NEW(wdCompressedStreamReaderZstdWithSource);
      wdCompressedStreamReaderZstdWithSource* pRawReader = static_cast<wdCompressedStreamReaderZstdWithSource*>(reader.Borrow());
      ConfigureRawMemoryStreamReader(entry, pStartOfArchiveData, pRawReader->m_Source);
      pRawReader->SetInputStream(&pRawReader->m_Source);
      break;
    }
#endif

    default:
      WD_REPORT_FAILURE("Archive entry compression mode '{}' is not supported by wdArchiveReader", (int)entry.m_CompressionMode);
      break;
  }

  return std::move(reader);
}

void wdArchiveUtils::ConfigureRawMemoryStreamReader(const wdArchiveEntry& entry, const void* pStartOfArchiveData, wdRawMemoryStreamReader& ref_memReader)
{
  ref_memReader.Reset(wdMemoryUtils::AddByteOffset(pStartOfArchiveData, static_cast<ptrdiff_t>(entry.m_uiDataStartOffset)), entry.m_uiStoredDataSize);
}

static const char* szEndMarker = "WDARCHIVE-END";

static wdUInt32 GetEndMarkerSize(wdUInt8 uiFileVersion)
{
  if (uiFileVersion == 1)
    return 0;

  return 14;
}

static wdUInt32 GetTocMetaSize(wdUInt8 uiFileVersion)
{
  if (uiFileVersion == 1)
    return sizeof(wdUInt32); // TOC size

  return sizeof(wdUInt32) /* TOC size */ + sizeof(wdUInt64) /* TOC hash */;
}

struct TocMetaData
{
  wdUInt32 m_uiSize = 0;
  wdUInt64 m_uiHash = 0;
};

wdResult wdArchiveUtils::AppendTOC(wdStreamWriter& inout_stream, const wdArchiveTOC& toc)
{
  wdDefaultMemoryStreamStorage storage;
  wdMemoryStreamWriter writer(&storage);

  WD_SUCCEED_OR_RETURN(toc.Serialize(writer));

  WD_SUCCEED_OR_RETURN(storage.CopyToStream(inout_stream));

  TocMetaData tocMeta;

  wdHashStreamWriter64 hashStream(tocMeta.m_uiSize);
  WD_SUCCEED_OR_RETURN(storage.CopyToStream(hashStream));

  // Added in file version 2: hash of the TOC
  tocMeta.m_uiSize = storage.GetStorageSize32();
  tocMeta.m_uiHash = hashStream.GetHashValue();

  // append the TOC meta data
  inout_stream << tocMeta.m_uiSize;
  inout_stream << tocMeta.m_uiHash;

  // write an 'end' marker
  return inout_stream.WriteBytes(szEndMarker, 14);
}

static wdResult VerifyEndMarker(wdMemoryMappedFile& ref_memFile, wdUInt8 uiArchiveVersion)
{
  const wdUInt32 uiEndMarkerSize = GetEndMarkerSize(uiArchiveVersion);

  if (uiEndMarkerSize == 0)
    return WD_SUCCESS;

  const void* pStart = ref_memFile.GetReadPointer(uiEndMarkerSize, wdMemoryMappedFile::OffsetBase::End);

  wdRawMemoryStreamReader reader(pStart, uiEndMarkerSize);

  char szMarker[32] = "";
  if (reader.ReadBytes(szMarker, uiEndMarkerSize) != uiEndMarkerSize || !wdStringUtils::IsEqual(szMarker, szEndMarker))
  {
    wdLog::Error("Archive is corrupt or cut off. End-marker not found.");
    return WD_FAILURE;
  }

  return WD_SUCCESS;
}

wdResult wdArchiveUtils::ExtractTOC(wdMemoryMappedFile& ref_memFile, wdArchiveTOC& ref_toc, wdUInt8 uiArchiveVersion)
{
  WD_SUCCEED_OR_RETURN(VerifyEndMarker(ref_memFile, uiArchiveVersion));

  const wdUInt32 uiEndMarkerSize = GetEndMarkerSize(uiArchiveVersion);
  const wdUInt32 uiTocMetaSize = GetTocMetaSize(uiArchiveVersion);

  wdUInt32 uiTocSize = 0;
  wdUInt64 uiExpectedTocHash = 0;

  // read the TOC meta data
  {
    const void* pTocMetaStart = ref_memFile.GetReadPointer(uiEndMarkerSize + uiTocMetaSize, wdMemoryMappedFile::OffsetBase::End);

    wdRawMemoryStreamReader tocMetaReader(pTocMetaStart, uiTocMetaSize);

    tocMetaReader >> uiTocSize;

    if (uiTocSize > 1024 * 1024 * 1024) // 1GB of TOC is enough for ~16M entries...
    {
      wdLog::Error("Archive TOC is probably corrupted. Unreasonable TOC size: {0}", wdArgFileSize(uiTocSize));
      return WD_FAILURE;
    }

    if (uiArchiveVersion >= 2)
    {
      tocMetaReader >> uiExpectedTocHash;
    }
  }

  const void* pTocStart = ref_memFile.GetReadPointer(uiTocSize + uiTocMetaSize + uiEndMarkerSize, wdMemoryMappedFile::OffsetBase::End);

  // validate the TOC hash
  if (uiArchiveVersion >= 2)
  {
    const wdUInt64 uiActualTocHash = wdHashingUtils::xxHash64(pTocStart, uiTocSize);
    if (uiExpectedTocHash != uiActualTocHash)
    {
      wdLog::Error("Archive TOC is corrupted. Hashes do not match.");
      return WD_FAILURE;
    }
  }

  // read the actual TOC data
  {
    wdRawMemoryStreamReader tocReader(pTocStart, uiTocSize);

    if (ref_toc.Deserialize(tocReader, uiArchiveVersion).Failed())
    {
      wdLog::Error("Failed to deserialize wdArchive TOC");
      return WD_FAILURE;
    }
  }

  return WD_SUCCESS;
}

namespace ZipFormat
{
  constexpr wdUInt32 EndOfCDMagicSignature = 0x06054b50;
  constexpr wdUInt32 EndOfCDHeaderLength = 22;

  constexpr wdUInt32 MaxCommentLength = 65535;
  constexpr wdUInt64 MaxEndOfCDSearchLength = MaxCommentLength + EndOfCDHeaderLength;

  constexpr wdUInt32 LocalFileMagicSignature = 0x04034b50;
  constexpr wdUInt32 LocalFileHeaderLength = 30;

  constexpr wdUInt32 CDFileMagicSignature = 0x02014b50;
  constexpr wdUInt32 CDFileHeaderLength = 46;

  enum CompressionType
  {
    Uncompressed = 0,
    Deflate = 8,

  };

  struct EndOfCDHeader
  {
    wdUInt32 signature;
    wdUInt16 diskNumber;
    wdUInt16 diskWithCD;
    wdUInt16 diskEntries;
    wdUInt16 totalEntries;
    wdUInt32 cdSize;
    wdUInt32 cdOffset;
    wdUInt16 commentLength;
  };

  wdStreamReader& operator>>(wdStreamReader& inout_stream, EndOfCDHeader& ref_value)
  {
    inout_stream >> ref_value.signature >> ref_value.diskNumber >> ref_value.diskWithCD >> ref_value.diskEntries >> ref_value.totalEntries >> ref_value.cdSize;
    inout_stream >> ref_value.cdOffset >> ref_value.commentLength;
    WD_ASSERT_DEBUG(ref_value.signature == EndOfCDMagicSignature, "ZIP: Corrupt end of central directory header.");
    return inout_stream;
  }

  struct CDFileHeader
  {
    wdUInt32 signature;
    wdUInt16 version;
    wdUInt16 versionNeeded;
    wdUInt16 flags;
    wdUInt16 compression;
    wdUInt16 modTime;
    wdUInt16 modDate;
    wdUInt32 crc32;
    wdUInt32 compressedSize;
    wdUInt32 uncompressedSize;
    wdUInt16 fileNameLength;
    wdUInt16 extraFieldLength;
    wdUInt16 fileCommentLength;
    wdUInt16 diskNumStart;
    wdUInt16 internalAttr;
    wdUInt32 externalAttr;
    wdUInt32 offsetLocalHeader;
  };

  wdStreamReader& operator>>(wdStreamReader& inout_stream, CDFileHeader& ref_value)
  {
    inout_stream >> ref_value.signature >> ref_value.version >> ref_value.versionNeeded >> ref_value.flags >> ref_value.compression >> ref_value.modTime >> ref_value.modDate;
    inout_stream >> ref_value.crc32 >> ref_value.compressedSize >> ref_value.uncompressedSize >> ref_value.fileNameLength >> ref_value.extraFieldLength;
    inout_stream >> ref_value.fileCommentLength >> ref_value.diskNumStart >> ref_value.internalAttr >> ref_value.externalAttr >> ref_value.offsetLocalHeader;
    WD_ASSERT_DEBUG(ref_value.signature == CDFileMagicSignature, "ZIP: Corrupt central directory file entry header.");
    return inout_stream;
  }

  struct LocalFileHeader
  {
    wdUInt32 signature;
    wdUInt16 version;
    wdUInt16 flags;
    wdUInt16 compression;
    wdUInt16 modTime;
    wdUInt16 modDate;
    wdUInt32 crc32;
    wdUInt32 compressedSize;
    wdUInt32 uncompressedSize;
    wdUInt16 fileNameLength;
    wdUInt16 extraFieldLength;
  };

  wdStreamReader& operator>>(wdStreamReader& inout_stream, LocalFileHeader& ref_value)
  {
    inout_stream >> ref_value.signature >> ref_value.version >> ref_value.flags >> ref_value.compression >> ref_value.modTime >> ref_value.modDate >> ref_value.crc32;
    inout_stream >> ref_value.compressedSize >> ref_value.uncompressedSize >> ref_value.fileNameLength >> ref_value.extraFieldLength;
    WD_ASSERT_DEBUG(ref_value.signature == LocalFileMagicSignature, "ZIP: Corrupt local file entry header.");
    return inout_stream;
  }
}; // namespace ZipFormat

wdResult wdArchiveUtils::ReadZipHeader(wdStreamReader& inout_stream, wdUInt8& out_uiVersion)
{
  using namespace ZipFormat;

  wdUInt32 header;
  inout_stream >> header;
  if (header == LocalFileMagicSignature)
  {
    out_uiVersion = 0;
    return WD_SUCCESS;
  }
  return WD_SUCCESS;
}

wdResult wdArchiveUtils::ExtractZipTOC(wdMemoryMappedFile& ref_memFile, wdArchiveTOC& ref_toc)
{
  using namespace ZipFormat;

  const wdUInt8* pEndOfCDStart = nullptr;
  {
    // Find End of CD signature by searching from the end of the file.
    // As a comment can come after it we have to potentially walk max comment length backwards.
    const wdUInt64 SearchEnd = ref_memFile.GetFileSize() - wdMath::Min(MaxEndOfCDSearchLength, ref_memFile.GetFileSize());
    const wdUInt8* pSearchEnd = static_cast<const wdUInt8*>(ref_memFile.GetReadPointer(SearchEnd, wdMemoryMappedFile::OffsetBase::End));
    const wdUInt8* pSearchStart = static_cast<const wdUInt8*>(ref_memFile.GetReadPointer(EndOfCDHeaderLength, wdMemoryMappedFile::OffsetBase::End));
    while (pSearchStart >= pSearchEnd)
    {
      if (*reinterpret_cast<const wdUInt32*>(pSearchStart) == EndOfCDMagicSignature)
      {
        pEndOfCDStart = pSearchStart;
        break;
      }
      pSearchStart--;
    }
    if (pEndOfCDStart == nullptr)
      return WD_FAILURE;
  }

  wdRawMemoryStreamReader tocReader(pEndOfCDStart, EndOfCDHeaderLength);
  EndOfCDHeader ecdHeader;
  tocReader >> ecdHeader;

  ref_toc.m_Entries.Reserve(ecdHeader.diskEntries);
  ref_toc.m_PathToEntryIndex.Reserve(ecdHeader.diskEntries);

  wdStringBuilder sLowerCaseHash;
  wdUInt64 uiEntryOffset = 0;
  for (wdUInt16 uiEntry = 0; uiEntry < ecdHeader.diskEntries; ++uiEntry)
  {
    // First, read the current file's header from the central directory
    const void* pCdfStart = ref_memFile.GetReadPointer(ecdHeader.cdOffset + uiEntryOffset, wdMemoryMappedFile::OffsetBase::Start);
    wdRawMemoryStreamReader cdfReader(pCdfStart, ecdHeader.cdSize - uiEntryOffset);
    CDFileHeader cdfHeader;
    cdfReader >> cdfHeader;

    if (cdfHeader.compression == CompressionType::Uncompressed || cdfHeader.compression == CompressionType::Deflate)
    {
      auto& entry = ref_toc.m_Entries.ExpandAndGetRef();
      entry.m_uiUncompressedDataSize = cdfHeader.uncompressedSize;
      entry.m_uiStoredDataSize = cdfHeader.compressedSize;
      entry.m_uiPathStringOffset = ref_toc.m_AllPathStrings.GetCount();
      entry.m_CompressionMode = cdfHeader.compression == CompressionType::Uncompressed ? wdArchiveCompressionMode::Uncompressed : wdArchiveCompressionMode::Compressed_zip;

      auto nameBuffer = wdArrayPtr<const wdUInt8>(static_cast<const wdUInt8*>(pCdfStart) + CDFileHeaderLength, cdfHeader.fileNameLength);
      ref_toc.m_AllPathStrings.PushBackRange(nameBuffer);
      ref_toc.m_AllPathStrings.PushBack(0);
      const char* szName = reinterpret_cast<const char*>(ref_toc.m_AllPathStrings.GetData() + entry.m_uiPathStringOffset);
      sLowerCaseHash = szName;
      sLowerCaseHash.ToLower();
      ref_toc.m_PathToEntryIndex.Insert(wdArchiveStoredString(wdHashingUtils::StringHash(sLowerCaseHash), entry.m_uiPathStringOffset), ref_toc.m_Entries.GetCount() - 1);

      // Compute data stream start location. We need to skip past the local (and redundant) file header to find it.
      const void* pLfStart = ref_memFile.GetReadPointer(cdfHeader.offsetLocalHeader, wdMemoryMappedFile::OffsetBase::Start);
      wdRawMemoryStreamReader lfReader(pLfStart, ref_memFile.GetFileSize() - cdfHeader.offsetLocalHeader);
      LocalFileHeader lfHeader;
      lfReader >> lfHeader;
      entry.m_uiDataStartOffset = cdfHeader.offsetLocalHeader + LocalFileHeaderLength + lfHeader.fileNameLength + lfHeader.extraFieldLength;
    }
    // Compute next file header location.
    uiEntryOffset += CDFileHeaderLength + cdfHeader.fileNameLength + cdfHeader.extraFieldLength + cdfHeader.fileCommentLength;
  }

  return WD_SUCCESS;
}


WD_STATICLINK_FILE(Foundation, Foundation_IO_Archive_Implementation_ArchiveUtils);

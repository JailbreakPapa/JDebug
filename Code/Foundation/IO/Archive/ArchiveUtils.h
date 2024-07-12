#pragma once

#include <Foundation/IO/Archive/Archive.h>
#include <Foundation/Types/Delegate.h>
#include <Foundation/Types/UniquePtr.h>

class nsStreamReader;
class nsStreamWriter;
class nsMemoryMappedFile;
class nsArchiveTOC;
class nsArchiveEntry;
class nsRawMemoryStreamReader;

/// \brief Utilities for working with nsArchive files
namespace nsArchiveUtils
{
  using FileWriteProgressCallback = nsDelegate<bool(nsUInt64, nsUInt64)>;
  constexpr nsUInt32 ArchiveHeaderSize = 16;
  constexpr nsUInt32 ArchiveTOCMetaMaxFooterSize = 14 + 12; //< note that it's the MAX size, i.e. toc meta can be smaller

  struct TOCMeta
  {
    nsUInt32 m_uiTocSize = 0;
    nsUInt64 m_uiExpectedTocHash = 0;
    nsUInt32 m_uiTocOffsetFromArchiveEnd = 0;
  };

  /// \brief Returns a modifiable array of file extensions that the engine considers to be valid nsArchive file extensions.
  ///
  /// By default it always contains 'nsArchive'.
  /// Add or overwrite the values, if you want custom file extensions to be handled as nsArchives.
  NS_FOUNDATION_DLL nsHybridArray<nsString, 4, nsStaticsAllocatorWrapper>& GetAcceptedArchiveFileExtensions();

  /// \brief Checks case insensitive, whether the given extension is in the list of GetAcceptedArchiveFileExtensions().
  NS_FOUNDATION_DLL bool IsAcceptedArchiveFileExtensions(nsStringView sExtension);

  /// \brief Writes the header that identifies the nsArchive file and version to the stream
  NS_FOUNDATION_DLL nsResult WriteHeader(nsStreamWriter& inout_stream);

  /// \brief Reads the nsArchive header. Returns success and the version, if the stream is a valid nsArchive file.
  NS_FOUNDATION_DLL nsResult ReadHeader(nsStreamReader& inout_stream, nsUInt8& out_uiVersion);

  /// \brief Writes the archive TOC to the stream. This must be the last thing in the stream, if ExtractTOC() is supposed to work.
  NS_FOUNDATION_DLL nsResult AppendTOC(nsStreamWriter& inout_stream, const nsArchiveTOC& toc);

  /// \brief Deserializes the TOC meta from archive ending. Assumes the TOC is the very last data in the file.
  NS_FOUNDATION_DLL nsResult ExtractTOCMeta(nsUInt64 uiArchiveEndingDataSize, const void* pArchiveEndingDataBuffer, TOCMeta& ref_tocMeta, nsUInt8 uiArchiveVersion);

  /// \brief Deserializes the TOC meta from the memory mapped file. Assumes the TOC is the very last data in the file.
  NS_FOUNDATION_DLL nsResult ExtractTOCMeta(const nsMemoryMappedFile& memFile, TOCMeta& ref_tocMeta, nsUInt8 uiArchiveVersion);

  /// \brief Deserializes the TOC from from archive ending. Assumes the TOC is the very last data in the file and reads it from the back.
  NS_FOUNDATION_DLL nsResult ExtractTOC(nsUInt64 uiArchiveEndingDataSize, const void* pArchiveEndingDataBuffer, nsArchiveTOC& ref_toc, nsUInt8 uiArchiveVersion);

  /// \brief Deserializes the TOC from the memory mapped file. Assumes the TOC is the very last data in the file and reads it from the back.
  NS_FOUNDATION_DLL nsResult ExtractTOC(const nsMemoryMappedFile& memFile, nsArchiveTOC& ref_toc, nsUInt8 uiArchiveVersion);

  /// \brief Writes a single file entry to an nsArchive stream with the given compression level.
  ///
  /// Appends information to the TOC for finding the data in the stream. Reads and updates inout_uiCurrentStreamPosition with the data byte
  /// offset. The progress callback is executed for every couple of KB of data that were written.
  NS_FOUNDATION_DLL nsResult WriteEntry(nsStreamWriter& inout_stream, nsStringView sAbsSourcePath, nsUInt32 uiPathStringOffset,
    nsArchiveCompressionMode compression, nsInt32 iCompressionLevel, nsArchiveEntry& ref_tocEntry, nsUInt64& inout_uiCurrentStreamPosition,
    FileWriteProgressCallback progress = FileWriteProgressCallback());

  /// \brief Writes a single file entry to an nsArchive stream with the given compression level.
  ///
  /// Appends information to the TOC for finding the data in the stream. Reads and updates inout_uiCurrentStreamPosition with the data byte
  /// offset. Compression parameter indicate compression that the entry data already have applied.
  NS_FOUNDATION_DLL nsResult WriteEntryPreprocessed(nsStreamWriter& inout_stream, nsConstByteArrayPtr entryData, nsUInt32 uiPathStringOffset,
    nsArchiveCompressionMode compression, nsUInt32 uiUncompressedEntryDataSize, nsArchiveEntry& ref_tocEntry, nsUInt64& inout_uiCurrentStreamPosition);

  /// \brief Similar to WriteEntry, but if compression is enabled, checks that compression makes enough of a difference.
  /// If compression does not reduce file size enough, the file is stored uncompressed instead.
  NS_FOUNDATION_DLL nsResult WriteEntryOptimal(nsStreamWriter& inout_stream, nsStringView sAbsSourcePath, nsUInt32 uiPathStringOffset,
    nsArchiveCompressionMode compression, nsInt32 iCompressionLevel, nsArchiveEntry& ref_tocEntry, nsUInt64& inout_uiCurrentStreamPosition,
    FileWriteProgressCallback progress = FileWriteProgressCallback());

  /// \brief Configures \a memReader as a view into the data stored for \a entry in the archive file.
  ///
  /// The raw memory stream may be compressed or uncompressed. This only creates a view for the stored data, it does not interpret it.
  NS_FOUNDATION_DLL void ConfigureRawMemoryStreamReader(
    const nsArchiveEntry& entry, const void* pStartOfArchiveData, nsRawMemoryStreamReader& ref_memReader);

  /// \brief Creates a new stream reader which allows to read the uncompressed data for the given archive entry.
  ///
  /// Under the hood it may create different types of stream readers to uncompress or decode the data.
  NS_FOUNDATION_DLL nsUniquePtr<nsStreamReader> CreateEntryReader(const nsArchiveEntry& entry, const void* pStartOfArchiveData);

  NS_FOUNDATION_DLL nsResult ReadZipHeader(nsStreamReader& inout_stream, nsUInt8& out_uiVersion);
  NS_FOUNDATION_DLL nsResult ExtractZipTOC(const nsMemoryMappedFile& memFile, nsArchiveTOC& ref_toc);


} // namespace nsArchiveUtils

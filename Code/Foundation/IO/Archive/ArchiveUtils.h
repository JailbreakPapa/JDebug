#pragma once

#include <Foundation/IO/Archive/Archive.h>
#include <Foundation/Types/Delegate.h>
#include <Foundation/Types/UniquePtr.h>

class wdStreamReader;
class wdStreamWriter;
class wdMemoryMappedFile;
class wdArchiveTOC;
class wdArchiveEntry;
class wdRawMemoryStreamReader;

/// \brief Utilities for working with wdArchive files
namespace wdArchiveUtils
{
  typedef wdDelegate<bool(wdUInt64, wdUInt64)> FileWriteProgressCallback;

  /// \brief Returns a modifiable array of file extensions that the engine considers to be valid wdArchive file extensions.
  ///
  /// By default it always contains 'wdArchive'.
  /// Add or overwrite the values, if you want custom file extensions to be handled as wdArchives.
  WD_FOUNDATION_DLL wdHybridArray<wdString, 4, wdStaticAllocatorWrapper>& GetAcceptedArchiveFileExtensions();

  /// \brief Checks case insensitive, whether the given extension is in the list of GetAcceptedArchiveFileExtensions().
  WD_FOUNDATION_DLL bool IsAcceptedArchiveFileExtensions(wdStringView sExtension);

  /// \brief Writes the header that identifies the wdArchive file and version to the stream
  WD_FOUNDATION_DLL wdResult WriteHeader(wdStreamWriter& inout_stream);

  /// \brief Reads the wdArchive header. Returns success and the version, if the stream is a valid wdArchive file.
  WD_FOUNDATION_DLL wdResult ReadHeader(wdStreamReader& inout_stream, wdUInt8& out_uiVersion);

  /// \brief Writes the archive TOC to the stream. This must be the last thing in the stream, if ExtractTOC() is supposed to work.
  WD_FOUNDATION_DLL wdResult AppendTOC(wdStreamWriter& inout_stream, const wdArchiveTOC& toc);

  /// \brief Deserializes the TOC from the memory mapped file. Assumes the TOC is the very last data in the file and reads it from the back.
  WD_FOUNDATION_DLL wdResult ExtractTOC(wdMemoryMappedFile& ref_memFile, wdArchiveTOC& ref_toc, wdUInt8 uiArchiveVersion);

  /// \brief Writes a single file entry to an wdArchive stream with the given compression level.
  ///
  /// Appends information to the TOC for finding the data in the stream. Reads and updates inout_uiCurrentStreamPosition with the data byte
  /// offset. The progress callback is executed for every couple of KB of data that were written.
  WD_FOUNDATION_DLL wdResult WriteEntry(wdStreamWriter& inout_stream, wdStringView sAbsSourcePath, wdUInt32 uiPathStringOffset,
    wdArchiveCompressionMode compression, wdInt32 iCompressionLevel, wdArchiveEntry& ref_tocEntry, wdUInt64& inout_uiCurrentStreamPosition,
    FileWriteProgressCallback progress = FileWriteProgressCallback());

  /// \brief Similar to WriteEntry, but if compression is enabled, checks that compression makes enough of a difference.
  /// If compression does not reduce file size enough, the file is stored uncompressed instead.
  WD_FOUNDATION_DLL wdResult WriteEntryOptimal(wdStreamWriter& inout_stream, wdStringView sAbsSourcePath, wdUInt32 uiPathStringOffset,
    wdArchiveCompressionMode compression, wdInt32 iCompressionLevel, wdArchiveEntry& ref_tocEntry, wdUInt64& inout_uiCurrentStreamPosition,
    FileWriteProgressCallback progress = FileWriteProgressCallback());

  /// \brief Configures \a memReader as a view into the data stored for \a entry in the archive file.
  ///
  /// The raw memory stream may be compressed or uncompressed. This only creates a view for the stored data, it does not interpret it.
  WD_FOUNDATION_DLL void ConfigureRawMemoryStreamReader(
    const wdArchiveEntry& entry, const void* pStartOfArchiveData, wdRawMemoryStreamReader& ref_memReader);

  /// \brief Creates a new stream reader which allows to read the uncompressed data for the given archive entry.
  ///
  /// Under the hood it may create different types of stream readers to uncompress or decode the data.
  WD_FOUNDATION_DLL wdUniquePtr<wdStreamReader> CreateEntryReader(const wdArchiveEntry& entry, const void* pStartOfArchiveData);

  WD_FOUNDATION_DLL wdResult ReadZipHeader(wdStreamReader& inout_stream, wdUInt8& out_uiVersion);
  WD_FOUNDATION_DLL wdResult ExtractZipTOC(wdMemoryMappedFile& ref_memFile, wdArchiveTOC& ref_toc);


} // namespace wdArchiveUtils

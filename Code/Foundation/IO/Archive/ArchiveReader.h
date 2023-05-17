#pragma once

#include <Foundation/IO/Archive/Archive.h>
#include <Foundation/IO/MemoryMappedFile.h>
#include <Foundation/Types/UniquePtr.h>

class wdRawMemoryStreamReader;
class wdStreamReader;

/// \brief A utility class for reading from wdArchive files
class WD_FOUNDATION_DLL wdArchiveReader
{
public:
  /// \brief Opens the given file and validates that it is a valid archive file.
  wdResult OpenArchive(wdStringView sPath);

  /// \brief Returns the table-of-contents for the previously opened archive.
  const wdArchiveTOC& GetArchiveTOC();

  /// \brief Extracts the given entry to the target folder.
  ///
  /// Calls ExtractFileProgressCallback() to report progress.
  wdResult ExtractFile(wdUInt32 uiEntryIdx, wdStringView sTargetFolder) const;

  /// \brief Extracts all files to the target folder.
  ///
  /// Calls ExtractNextFileCallback() for every file that is being extracted.
  wdResult ExtractAllFiles(wdStringView sTargetFolder) const;

  /// \brief Sets up \a memReader for reading the raw (potentially compressed) data that is stored for the given entry in the archive.
  void ConfigureRawMemoryStreamReader(wdUInt32 uiEntryIdx, wdRawMemoryStreamReader& ref_memReader) const;

  /// \brief Creates a reader that will decompress the given file entry.
  wdUniquePtr<wdStreamReader> CreateEntryReader(wdUInt32 uiEntryIdx) const;

protected:
  /// \brief Called by ExtractAllFiles() for progress reporting. Return false to abort.
  virtual bool ExtractNextFileCallback(wdUInt32 uiCurEntry, wdUInt32 uiMaxEntries, wdStringView sSourceFile) const;

  /// \brief Called by ExtractFile() for progress reporting. Return false to abort.
  virtual bool ExtractFileProgressCallback(wdUInt64 bytesWritten, wdUInt64 bytesTotal) const;

  wdMemoryMappedFile m_MemFile;
  wdArchiveTOC m_ArchiveTOC;
  wdUInt8 m_uiArchiveVersion = 0;
  const void* m_pDataStart = nullptr;
  wdUInt64 m_uiMemFileSize = 0;
};

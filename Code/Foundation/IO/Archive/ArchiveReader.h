#pragma once

#include <Foundation/IO/Archive/Archive.h>
#include <Foundation/IO/MemoryMappedFile.h>
#include <Foundation/Types/UniquePtr.h>

class nsRawMemoryStreamReader;
class nsStreamReader;

/// \brief A utility class for reading from nsArchive files
class NS_FOUNDATION_DLL nsArchiveReader
{
public:
  /// \brief Opens the given file and validates that it is a valid archive file.
  nsResult OpenArchive(nsStringView sPath);

  /// \brief Returns the table-of-contents for the previously opened archive.
  const nsArchiveTOC& GetArchiveTOC();

  /// \brief Extracts the given entry to the target folder.
  ///
  /// Calls ExtractFileProgressCallback() to report progress.
  nsResult ExtractFile(nsUInt32 uiEntryIdx, nsStringView sTargetFolder) const;

  /// \brief Extracts all files to the target folder.
  ///
  /// Calls ExtractNextFileCallback() for every file that is being extracted.
  nsResult ExtractAllFiles(nsStringView sTargetFolder) const;

  /// \brief Sets up \a memReader for reading the raw (potentially compressed) data that is stored for the given entry in the archive.
  void ConfigureRawMemoryStreamReader(nsUInt32 uiEntryIdx, nsRawMemoryStreamReader& ref_memReader) const;

  /// \brief Creates a reader that will decompress the given file entry.
  nsUniquePtr<nsStreamReader> CreateEntryReader(nsUInt32 uiEntryIdx) const;

protected:
  /// \brief Called by ExtractAllFiles() for progress reporting. Return false to abort.
  virtual bool ExtractNextFileCallback(nsUInt32 uiCurEntry, nsUInt32 uiMaxEntries, nsStringView sSourceFile) const;

  /// \brief Called by ExtractFile() for progress reporting. Return false to abort.
  virtual bool ExtractFileProgressCallback(nsUInt64 bytesWritten, nsUInt64 bytesTotal) const;

  nsMemoryMappedFile m_MemFile;
  nsArchiveTOC m_ArchiveTOC;
  nsUInt8 m_uiArchiveVersion = 0;
  const void* m_pDataStart = nullptr;
  nsUInt64 m_uiMemFileSize = 0;
};

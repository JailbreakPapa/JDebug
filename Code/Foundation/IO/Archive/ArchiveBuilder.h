#pragma once

#include <Foundation/IO/Archive/Archive.h>

#include <Foundation/Containers/Deque.h>
#include <Foundation/Types/Delegate.h>

/// \brief Utility class to build an nsArchive file from files/folders on disk
///
/// All functionality for writing an nsArchive file is available through nsArchiveUtils.
class NS_FOUNDATION_DLL nsArchiveBuilder
{
public:
  struct SourceEntry
  {
    nsString m_sAbsSourcePath; ///< The source file to read
    nsString m_sRelTargetPath; ///< Under which relative path to store it in the nsArchive
    nsArchiveCompressionMode m_CompressionMode = nsArchiveCompressionMode::Uncompressed;
    nsInt32 m_iCompressionLevel = 0;
  };

  // all the source files from disk that should be put into the nsArchive
  nsDeque<SourceEntry> m_Entries;

  enum class InclusionMode
  {
    Exclude,               ///< Do not add this file to the archive
    Uncompressed,          ///< Add the file to the archive, but do not even try to compress it
    Compress_zstd_fastest, ///< Add the file and try out compression. If compression does not help, the file will end up uncompressed in the archive.
    Compress_zstd_fast,    ///< Add the file and try out compression. If compression does not help, the file will end up uncompressed in the archive.
    Compress_zstd_average, ///< Add the file and try out compression. If compression does not help, the file will end up uncompressed in the archive.
    Compress_zstd_high,    ///< Add the file and try out compression. If compression does not help, the file will end up uncompressed in the archive.
    Compress_zstd_highest, ///< Add the file and try out compression. If compression does not help, the file will end up uncompressed in the archive.
  };

  /// \brief Custom decider whether to include a file into the archive
  using InclusionCallback = nsDelegate<InclusionMode(nsStringView)>;

  /// \brief Iterates over all files in a folder and adds them to m_Entries for later.
  ///
  /// The callback can be used to exclude certain files or to deactivate compression on them.
  /// \note If no callback is given, the default is to store all files uncompressed!
  void AddFolder(nsStringView sAbsFolderPath, nsArchiveCompressionMode defaultMode = nsArchiveCompressionMode::Uncompressed, InclusionCallback callback = InclusionCallback());

  /// \brief Overwrites the given file with the archive
  nsResult WriteArchive(nsStringView sFile) const;

  /// \brief Writes the previously gathered files to the file stream
  nsResult WriteArchive(nsStreamWriter& inout_stream) const;

protected:
  /// Override this to get a callback when the next file is being written to the output. Return 'true' to continue, 'false' to cancel the entire archive generation.
  virtual bool WriteNextFileCallback(nsUInt32 uiCurEntry, nsUInt32 uiMaxEntries, nsStringView sSourceFile) const;
  /// Override this to get a progress report for writing a single file to the output
  virtual bool WriteFileProgressCallback(nsUInt64 bytesWritten, nsUInt64 bytesTotal) const;
  /// Override this to get a callback after a file has been processed. Gets additional information about the compression result and duration.
  virtual void WriteFileResultCallback(nsUInt32 uiCurEntry, nsUInt32 uiMaxEntries, nsStringView sSourceFile, nsUInt64 uiSourceSize, nsUInt64 uiStoredSize, nsTime duration) const {}
};

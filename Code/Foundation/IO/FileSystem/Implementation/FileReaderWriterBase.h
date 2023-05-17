#pragma once

#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/FileSystem/Implementation/DataDirType.h>
#include <Foundation/IO/Stream.h>

/// The base class for all file readers.
/// Provides access to wdFileSystem::GetFileReader, which is necessary to get access to the streams that
/// wdDataDirectoryType's provide.
/// Derive from this class if you want to implement different policies on how to read files.
/// E.g. the default reader (wdFileReader) implements a buffered read policy (using an internal cache).
class WD_FOUNDATION_DLL wdFileReaderBase : public wdStreamReader
{
  WD_DISALLOW_COPY_AND_ASSIGN(wdFileReaderBase);

public:
  wdFileReaderBase() { m_pDataDirReader = nullptr; }

  /// Returns the absolute path with which the file was opened (including the prefix of the data directory).
  wdString128 GetFilePathAbsolute() const
  {
    wdStringBuilder sAbs = m_pDataDirReader->GetDataDirectory()->GetRedirectedDataDirectoryPath();
    sAbs.AppendPath(m_pDataDirReader->GetFilePath().GetView());
    return sAbs;
  }

  /// Returns the relative path of the file within its data directory (excluding the prefix of the data directory).
  wdString128 GetFilePathRelative() const { return m_pDataDirReader->GetFilePath(); }

  /// Returns the wdDataDirectoryType over which this file has been opened.
  wdDataDirectoryType* GetDataDirectory() const { return m_pDataDirReader->GetDataDirectory(); }

  /// Returns true, if the file is currently open.
  bool IsOpen() const { return m_pDataDirReader != nullptr; }

  /// \brief Returns the current total size of the file.
  wdUInt64 GetFileSize() const { return m_pDataDirReader->GetFileSize(); }

protected:
  wdDataDirectoryReader* GetFileReader(wdStringView sFile, wdFileShareMode::Enum FileShareMode, bool bAllowFileEvents)
  {
    return wdFileSystem::GetFileReader(sFile, FileShareMode, bAllowFileEvents);
  }

  wdDataDirectoryReader* m_pDataDirReader;
};


/// The base class for all file writers.
/// Provides access to wdFileSystem::GetFileWriter, which is necessary to get access to the streams that
/// wdDataDirectoryType's provide.
/// Derive from this class if you want to implement different policies on how to write files.
/// E.g. the default writer (wdFileWriter) implements a buffered write policy (using an internal cache).
class WD_FOUNDATION_DLL wdFileWriterBase : public wdStreamWriter
{
  WD_DISALLOW_COPY_AND_ASSIGN(wdFileWriterBase);

public:
  wdFileWriterBase() { m_pDataDirWriter = nullptr; }

  /// Returns the absolute path with which the file was opened (including the prefix of the data directory).
  wdString128 GetFilePathAbsolute() const
  {
    wdStringBuilder sAbs = m_pDataDirWriter->GetDataDirectory()->GetRedirectedDataDirectoryPath();
    sAbs.AppendPath(m_pDataDirWriter->GetFilePath().GetView());
    return sAbs;
  }

  /// Returns the relative path of the file within its data directory (excluding the prefix of the data directory).
  wdString128 GetFilePathRelative() const { return m_pDataDirWriter->GetFilePath(); }

  /// Returns the wdDataDirectoryType over which this file has been opened.
  wdDataDirectoryType* GetDataDirectory() const { return m_pDataDirWriter->GetDataDirectory(); }

  /// Returns true, if the file is currently open.
  bool IsOpen() const { return m_pDataDirWriter != nullptr; }

  /// \brief Returns the current total size of the file.
  wdUInt64 GetFileSize() const { return m_pDataDirWriter->GetFileSize(); } // [tested]

protected:
  wdDataDirectoryWriter* GetFileWriter(wdStringView sFile, wdFileShareMode::Enum FileShareMode, bool bAllowFileEvents)
  {
    return wdFileSystem::GetFileWriter(sFile, FileShareMode, bAllowFileEvents);
  }

  wdDataDirectoryWriter* m_pDataDirWriter;
};

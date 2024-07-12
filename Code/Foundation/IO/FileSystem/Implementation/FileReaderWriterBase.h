#pragma once

#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/FileSystem/Implementation/DataDirType.h>
#include <Foundation/IO/Stream.h>

/// The base class for all file readers.
/// Provides access to nsFileSystem::GetFileReader, which is necessary to get access to the streams that
/// nsDataDirectoryType's provide.
/// Derive from this class if you want to implement different policies on how to read files.
/// E.g. the default reader (nsFileReader) implements a buffered read policy (using an internal cache).
class NS_FOUNDATION_DLL nsFileReaderBase : public nsStreamReader
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsFileReaderBase);

public:
  nsFileReaderBase() { m_pDataDirReader = nullptr; }

  /// Returns the absolute path with which the file was opened (including the prefix of the data directory).
  nsString128 GetFilePathAbsolute() const
  {
    nsStringBuilder sAbs = m_pDataDirReader->GetDataDirectory()->GetRedirectedDataDirectoryPath();
    sAbs.AppendPath(m_pDataDirReader->GetFilePath().GetView());
    return sAbs;
  }

  /// Returns the relative path of the file within its data directory (excluding the prefix of the data directory).
  nsString128 GetFilePathRelative() const { return m_pDataDirReader->GetFilePath(); }

  /// Returns the nsDataDirectoryType over which this file has been opened.
  nsDataDirectoryType* GetDataDirectory() const { return m_pDataDirReader->GetDataDirectory(); }

  /// Returns true, if the file is currently open.
  bool IsOpen() const { return m_pDataDirReader != nullptr; }

  /// \brief Returns the current total size of the file.
  nsUInt64 GetFileSize() const { return m_pDataDirReader->GetFileSize(); }

protected:
  nsDataDirectoryReader* GetFileReader(nsStringView sFile, nsFileShareMode::Enum FileShareMode, bool bAllowFileEvents)
  {
    return nsFileSystem::GetFileReader(sFile, FileShareMode, bAllowFileEvents);
  }

  nsDataDirectoryReader* m_pDataDirReader;
};


/// The base class for all file writers.
/// Provides access to nsFileSystem::GetFileWriter, which is necessary to get access to the streams that
/// nsDataDirectoryType's provide.
/// Derive from this class if you want to implement different policies on how to write files.
/// E.g. the default writer (nsFileWriter) implements a buffered write policy (using an internal cache).
class NS_FOUNDATION_DLL nsFileWriterBase : public nsStreamWriter
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsFileWriterBase);

public:
  nsFileWriterBase() { m_pDataDirWriter = nullptr; }

  /// Returns the absolute path with which the file was opened (including the prefix of the data directory).
  nsString128 GetFilePathAbsolute() const
  {
    nsStringBuilder sAbs = m_pDataDirWriter->GetDataDirectory()->GetRedirectedDataDirectoryPath();
    sAbs.AppendPath(m_pDataDirWriter->GetFilePath().GetView());
    return sAbs;
  }

  /// Returns the relative path of the file within its data directory (excluding the prefix of the data directory).
  nsString128 GetFilePathRelative() const { return m_pDataDirWriter->GetFilePath(); }

  /// Returns the nsDataDirectoryType over which this file has been opened.
  nsDataDirectoryType* GetDataDirectory() const { return m_pDataDirWriter->GetDataDirectory(); }

  /// Returns true, if the file is currently open.
  bool IsOpen() const { return m_pDataDirWriter != nullptr; }

  /// \brief Returns the current total size of the file.
  nsUInt64 GetFileSize() const { return m_pDataDirWriter->GetFileSize(); } // [tested]

protected:
  nsDataDirectoryWriter* GetFileWriter(nsStringView sFile, nsFileShareMode::Enum FileShareMode, bool bAllowFileEvents)
  {
    return nsFileSystem::GetFileWriter(sFile, FileShareMode, bAllowFileEvents);
  }

  nsDataDirectoryWriter* m_pDataDirWriter;
};

#pragma once

#include <Foundation/Basics.h>
#include <Foundation/IO/FileEnums.h>
#include <Foundation/Strings/String.h>

class wdDataDirectoryReaderWriterBase;
class wdDataDirectoryReader;
class wdDataDirectoryWriter;
struct wdFileStats;

/// \brief The base class for all data directory types.
///
/// There are different data directory types, such as a simple folder, a ZIP file or some kind of library
/// (e.g. image files from procedural data). Even a HTTP server that actually transmits files over a network
/// can provided by implementing it as a data directory type.
/// Data directories are added through wdFileSystem, which uses factories to decide which wdDataDirectoryType
/// to use for handling which data directory.
class WD_FOUNDATION_DLL wdDataDirectoryType
{
  WD_DISALLOW_COPY_AND_ASSIGN(wdDataDirectoryType);

public:
  wdDataDirectoryType() = default;
  virtual ~wdDataDirectoryType() = default;

  /// \brief Returns the absolute path to the data directory.
  const wdString128& GetDataDirectoryPath() const { return m_sDataDirectoryPath; }

  /// \brief By default this is the same as GetDataDirectoryPath(), but derived implementations may use a different location where they
  /// actually get the files from.
  virtual const wdString128& GetRedirectedDataDirectoryPath() const { return GetDataDirectoryPath(); }

  /// \brief Some data directory types may use external configuration files (e.g. asset lookup tables)
  ///        that may get updated, while the directory is mounted. This function allows each directory type to implement
  ///        reloading and reapplying of configurations, without dismounting and remounting the data directory.
  virtual void ReloadExternalConfigs(){};

protected:
  friend class wdFileSystem;

  /// \brief Tries to setup the data directory. Can fail, if the type is incorrect (e.g. a ZIP file data directory type cannot handle a
  /// simple folder and vice versa)
  wdResult InitializeDataDirectory(wdStringView sDataDirPath);

  /// \brief Must be implemented to create a wdDataDirectoryReader for accessing the given file. Returns nullptr if the file could not be
  /// opened.
  ///
  /// \param szFile is given as a path relative to the data directory's path.
  /// So unless the data directory path is empty, this will never be an absolute path.
  /// If a rooted path was given, the root name is also removed and only the relative part is passed along.
  /// \param bSpecificallyThisDataDir This is true when the original path specified to open the file through exactly this data directory,
  /// by using a rooted path.
  /// If an absolute path is used, which incidentally matches the prefix of this data directory, bSpecificallyThisDataDir is NOT set to
  /// true, as there might be other data directories that also match.
  virtual wdDataDirectoryReader* OpenFileToRead(wdStringView sFile, wdFileShareMode::Enum FileShareMode, bool bSpecificallyThisDataDir) = 0;

  /// \brief Must be implemented to create a wdDataDirectoryWriter for accessing the given file. Returns nullptr if the file could not be
  /// opened.
  ///
  /// If it always returns nullptr (default) the data directory is read-only (at least through this type).
  virtual wdDataDirectoryWriter* OpenFileToWrite(wdStringView sFile, wdFileShareMode::Enum FileShareMode) { return nullptr; }

  /// \brief This function is called by the filesystem when a data directory is removed.
  ///
  /// It should delete itself using the proper allocator.
  virtual void RemoveDataDirectory() = 0;

  /// \brief If a Data Directory Type supports it, this function will remove the given file from it.
  virtual void DeleteFile(wdStringView sFile) {}

  /// \brief This function checks whether the given file exists in this data directory.
  ///
  /// The default implementation simply calls wdOSFile::ExistsFile
  /// An optimized implementation might look this information up in some hash-map.
  virtual bool ExistsFile(wdStringView sFile, bool bOneSpecificDataDir);

  /// \brief Upon success returns the wdFileStats for a file in this data directory.
  virtual wdResult GetFileStats(wdStringView sFileOrFolder, bool bOneSpecificDataDir, wdFileStats& out_Stats) = 0;

  /// \brief If this data directory knows how to redirect the given path, it should do so and return true.
  /// Called by wdFileSystem::ResolveAssetRedirection
  virtual bool ResolveAssetRedirection(wdStringView sPathOrAssetGuid, wdStringBuilder& out_sRedirection) { return false; }

protected:
  friend class wdDataDirectoryReaderWriterBase;

  /// \brief This is automatically called whenever a wdDataDirectoryReaderWriterBase that was opened by this type is being closed.
  ///
  /// It allows the wdDataDirectoryType to return the reader/writer to a pool of reusable objects, or to destroy it
  /// using the proper allocator.
  virtual void OnReaderWriterClose(wdDataDirectoryReaderWriterBase* pClosed) {}

  /// \brief This function should only be used by a Factory (which should be a static function in the respective wdDataDirectoryType).
  ///
  /// It is used to initialize the data directory. If this wdDataDirectoryType cannot handle the given type,
  /// it must return WD_FAILURE and the Factory needs to clean it up properly.
  virtual wdResult InternalInitializeDataDirectory(wdStringView sDirectory) = 0;

  /// \brief Derived classes can use 'GetDataDirectoryPath' to access this data.
  wdString128 m_sDataDirectoryPath;
};



/// \brief This is the base class for all data directory readers/writers.
///
/// Different data directory types (ZIP file, simple folder, etc.) use different reader/writer types.
class WD_FOUNDATION_DLL wdDataDirectoryReaderWriterBase
{
  WD_DISALLOW_COPY_AND_ASSIGN(wdDataDirectoryReaderWriterBase);

public:
  /// \brief The derived class should pass along whether it is a reader or writer.
  wdDataDirectoryReaderWriterBase(wdInt32 iDataDirUserData, bool bIsReader);

  virtual ~wdDataDirectoryReaderWriterBase() = default;

  /// \brief Used by wdDataDirectoryType's to try to open the given file. They need to pass along their own pointer.
  wdResult Open(wdStringView sFile, wdDataDirectoryType* pOwnerDataDirectory, wdFileShareMode::Enum fileShareMode);

  /// \brief Closes this data stream.
  void Close();

  /// \brief Returns the relative path of this file within the owner data directory.
  const wdString128& GetFilePath() const;

  /// \brief Returns the pointer to the data directory, which created this reader/writer.
  wdDataDirectoryType* GetDataDirectory() const;

  /// \brief Returns true if this is a reader stream, false if it is a writer stream.
  bool IsReader() const { return m_bIsReader; }

  /// \brief Returns the current total size of the file.
  virtual wdUInt64 GetFileSize() const = 0;

  wdInt32 GetDataDirUserData() const { return m_iDataDirUserData; }

protected:
  /// \brief This function must be implemented by the derived class.
  virtual wdResult InternalOpen(wdFileShareMode::Enum FileShareMode) = 0;

  /// \brief This function must be implemented by the derived class.
  virtual void InternalClose() = 0;

  bool m_bIsReader;
  wdInt32 m_iDataDirUserData = 0;
  wdDataDirectoryType* m_pDataDirectory;
  wdString128 m_sFilePath;
};

/// \brief A base class for readers that handle reading from a (virtual) file inside a data directory.
///
/// Different data directory types (ZIP file, simple folder, etc.) use different reader/writer types.
class WD_FOUNDATION_DLL wdDataDirectoryReader : public wdDataDirectoryReaderWriterBase
{
  WD_DISALLOW_COPY_AND_ASSIGN(wdDataDirectoryReader);

public:
  wdDataDirectoryReader(wdInt32 iDataDirUserData)
    : wdDataDirectoryReaderWriterBase(iDataDirUserData, true)
  {
  }

  virtual wdUInt64 Read(void* pBuffer, wdUInt64 uiBytes) = 0;
};

/// \brief A base class for writers that handle writing to a (virtual) file inside a data directory.
///
/// Different data directory types (ZIP file, simple folder, etc.) use different reader/writer types.
class WD_FOUNDATION_DLL wdDataDirectoryWriter : public wdDataDirectoryReaderWriterBase
{
  WD_DISALLOW_COPY_AND_ASSIGN(wdDataDirectoryWriter);

public:
  wdDataDirectoryWriter(wdInt32 iDataDirUserData)
    : wdDataDirectoryReaderWriterBase(iDataDirUserData, false)
  {
  }

  virtual wdResult Write(const void* pBuffer, wdUInt64 uiBytes) = 0;
};


#include <Foundation/IO/FileSystem/Implementation/DataDirType_inl.h>

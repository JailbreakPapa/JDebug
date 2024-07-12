#pragma once

#include <Foundation/Basics.h>
#include <Foundation/IO/FileEnums.h>
#include <Foundation/Strings/String.h>

class nsDataDirectoryReaderWriterBase;
class nsDataDirectoryReader;
class nsDataDirectoryWriter;
struct nsFileStats;

/// \brief The base class for all data directory types.
///
/// There are different data directory types, such as a simple folder, a ZIP file or some kind of library
/// (e.g. image files from procedural data). Even a HTTP server that actually transmits files over a network
/// can provided by implementing it as a data directory type.
/// Data directories are added through nsFileSystem, which uses factories to decide which nsDataDirectoryType
/// to use for handling which data directory.
class NS_FOUNDATION_DLL nsDataDirectoryType
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsDataDirectoryType);

public:
  nsDataDirectoryType() = default;
  virtual ~nsDataDirectoryType() = default;

  /// \brief Returns the absolute path to the data directory.
  const nsString128& GetDataDirectoryPath() const { return m_sDataDirectoryPath; }

  /// \brief By default this is the same as GetDataDirectoryPath(), but derived implementations may use a different location where they
  /// actually get the files from.
  virtual const nsString128& GetRedirectedDataDirectoryPath() const { return GetDataDirectoryPath(); }

  /// \brief Some data directory types may use external configuration files (e.g. asset lookup tables)
  ///        that may get updated, while the directory is mounted. This function allows each directory type to implement
  ///        reloading and reapplying of configurations, without dismounting and remounting the data directory.
  virtual void ReloadExternalConfigs() {};

protected:
  friend class nsFileSystem;

  /// \brief Tries to setup the data directory. Can fail, if the type is incorrect (e.g. a ZIP file data directory type cannot handle a
  /// simple folder and vice versa)
  nsResult InitializeDataDirectory(nsStringView sDataDirPath);

  /// \brief Must be implemented to create a nsDataDirectoryReader for accessing the given file. Returns nullptr if the file could not be
  /// opened.
  ///
  /// \param szFile is given as a path relative to the data directory's path.
  /// So unless the data directory path is empty, this will never be an absolute path.
  /// If a rooted path was given, the root name is also removed and only the relative part is passed along.
  /// \param bSpecificallyThisDataDir This is true when the original path specified to open the file through exactly this data directory,
  /// by using a rooted path.
  /// If an absolute path is used, which incidentally matches the prefix of this data directory, bSpecificallyThisDataDir is NOT set to
  /// true, as there might be other data directories that also match.
  virtual nsDataDirectoryReader* OpenFileToRead(nsStringView sFile, nsFileShareMode::Enum FileShareMode, bool bSpecificallyThisDataDir) = 0;

  /// \brief Must be implemented to create a nsDataDirectoryWriter for accessing the given file. Returns nullptr if the file could not be
  /// opened.
  ///
  /// If it always returns nullptr (default) the data directory is read-only (at least through this type).
  virtual nsDataDirectoryWriter* OpenFileToWrite(nsStringView sFile, nsFileShareMode::Enum FileShareMode) { return nullptr; }

  /// \brief This function is called by the filesystem when a data directory is removed.
  ///
  /// It should delete itself using the proper allocator.
  virtual void RemoveDataDirectory() = 0;

  /// \brief If a Data Directory Type supports it, this function will remove the given file from it.
  virtual void DeleteFile(nsStringView sFile) {}

  /// \brief This function checks whether the given file exists in this data directory.
  ///
  /// The default implementation simply calls nsOSFile::ExistsFile
  /// An optimized implementation might look this information up in some hash-map.
  virtual bool ExistsFile(nsStringView sFile, bool bOneSpecificDataDir);

  /// \brief Upon success returns the nsFileStats for a file in this data directory.
  virtual nsResult GetFileStats(nsStringView sFileOrFolder, bool bOneSpecificDataDir, nsFileStats& out_Stats) = 0;

  /// \brief If this data directory knows how to redirect the given path, it should do so and return true.
  /// Called by nsFileSystem::ResolveAssetRedirection
  virtual bool ResolveAssetRedirection(nsStringView sPathOrAssetGuid, nsStringBuilder& out_sRedirection) { return false; }

protected:
  friend class nsDataDirectoryReaderWriterBase;

  /// \brief This is automatically called whenever a nsDataDirectoryReaderWriterBase that was opened by this type is being closed.
  ///
  /// It allows the nsDataDirectoryType to return the reader/writer to a pool of reusable objects, or to destroy it
  /// using the proper allocator.
  virtual void OnReaderWriterClose(nsDataDirectoryReaderWriterBase* pClosed) {}

  /// \brief This function should only be used by a Factory (which should be a static function in the respective nsDataDirectoryType).
  ///
  /// It is used to initialize the data directory. If this nsDataDirectoryType cannot handle the given type,
  /// it must return NS_FAILURE and the Factory needs to clean it up properly.
  virtual nsResult InternalInitializeDataDirectory(nsStringView sDirectory) = 0;

  /// \brief Derived classes can use 'GetDataDirectoryPath' to access this data.
  nsString128 m_sDataDirectoryPath;
};



/// \brief This is the base class for all data directory readers/writers.
///
/// Different data directory types (ZIP file, simple folder, etc.) use different reader/writer types.
class NS_FOUNDATION_DLL nsDataDirectoryReaderWriterBase
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsDataDirectoryReaderWriterBase);

public:
  /// \brief The derived class should pass along whether it is a reader or writer.
  nsDataDirectoryReaderWriterBase(nsInt32 iDataDirUserData, bool bIsReader);

  virtual ~nsDataDirectoryReaderWriterBase() = default;

  /// \brief Used by nsDataDirectoryType's to try to open the given file. They need to pass along their own pointer.
  nsResult Open(nsStringView sFile, nsDataDirectoryType* pOwnerDataDirectory, nsFileShareMode::Enum fileShareMode);

  /// \brief Closes this data stream.
  void Close();

  /// \brief Returns the relative path of this file within the owner data directory.
  const nsString128& GetFilePath() const;

  /// \brief Returns the pointer to the data directory, which created this reader/writer.
  nsDataDirectoryType* GetDataDirectory() const;

  /// \brief Returns true if this is a reader stream, false if it is a writer stream.
  bool IsReader() const { return m_bIsReader; }

  /// \brief Returns the current total size of the file.
  virtual nsUInt64 GetFileSize() const = 0;

  nsInt32 GetDataDirUserData() const { return m_iDataDirUserData; }

protected:
  /// \brief This function must be implemented by the derived class.
  virtual nsResult InternalOpen(nsFileShareMode::Enum FileShareMode) = 0;

  /// \brief This function must be implemented by the derived class.
  virtual void InternalClose() = 0;

  bool m_bIsReader;
  nsInt32 m_iDataDirUserData = 0;
  nsDataDirectoryType* m_pDataDirectory;
  nsString128 m_sFilePath;
};

/// \brief A base class for readers that handle reading from a (virtual) file inside a data directory.
///
/// Different data directory types (ZIP file, simple folder, etc.) use different reader/writer types.
class NS_FOUNDATION_DLL nsDataDirectoryReader : public nsDataDirectoryReaderWriterBase
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsDataDirectoryReader);

public:
  nsDataDirectoryReader(nsInt32 iDataDirUserData)
    : nsDataDirectoryReaderWriterBase(iDataDirUserData, true)
  {
  }

  virtual nsUInt64 Read(void* pBuffer, nsUInt64 uiBytes) = 0;

  /// \brief Helper method to skip a number of bytes (implementations of the directory reader may implement this more efficiently for example)
  virtual nsUInt64 Skip(nsUInt64 uiBytes)
  {
    nsUInt8 uiTempBuffer[1024];

    nsUInt64 uiBytesSkipped = 0;

    while (uiBytesSkipped < uiBytes)
    {
      nsUInt64 uiBytesToRead = nsMath::Min<nsUInt64>(uiBytes - uiBytesSkipped, 1024);

      nsUInt64 uiBytesRead = Read(uiTempBuffer, uiBytesToRead);

      uiBytesSkipped += uiBytesRead;

      // Terminate early if the stream didn't read as many bytes as we requested (EOF for example)
      if (uiBytesRead < uiBytesToRead)
        break;
    }

    return uiBytesSkipped;
  }
};

/// \brief A base class for writers that handle writing to a (virtual) file inside a data directory.
///
/// Different data directory types (ZIP file, simple folder, etc.) use different reader/writer types.
class NS_FOUNDATION_DLL nsDataDirectoryWriter : public nsDataDirectoryReaderWriterBase
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsDataDirectoryWriter);

public:
  nsDataDirectoryWriter(nsInt32 iDataDirUserData)
    : nsDataDirectoryReaderWriterBase(iDataDirUserData, false)
  {
  }

  virtual nsResult Write(const void* pBuffer, nsUInt64 uiBytes) = 0;
};


#include <Foundation/IO/FileSystem/Implementation/DataDirType_inl.h>

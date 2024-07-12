#pragma once

#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/FileSystem/Implementation/DataDirType.h>
#include <Foundation/IO/OSFile.h>

namespace nsDataDirectory
{
  class FolderReader;
  class FolderWriter;

  /// \brief A data directory type to handle access to ordinary files.
  ///
  /// Register the 'Factory' function at nsFileSystem to allow it to mount local directories.
  class NS_FOUNDATION_DLL FolderType : public nsDataDirectoryType
  {
  public:
    ~FolderType();

    /// \brief The factory that can be registered at nsFileSystem to create data directories of this type.
    static nsDataDirectoryType* Factory(nsStringView sDataDirectory, nsStringView sGroup, nsStringView sRootName, nsFileSystem::DataDirUsage usage);

    /// A 'redirection file' is an optional file located inside a data directory that lists which file access is redirected to which other
    /// file lookup. Each redirection is one line in the file (terminated by a \n). Each line consists of the 'key' string, a semicolon and
    /// a 'value' string. No unnecessary whitespace is allowed. When a file that matches 'key' is accessed through a mounted data directory,
    /// the file access will be replaced by 'value' (plus s_sRedirectionPrefix) 'key' may be anything (e.g. a GUID string), 'value' should
    /// be a valid relative path into the SAME data directory. The redirection file can be used to implement an asset lookup, where assets
    /// are identified by GUIDs and need to be mapped to the actual asset file.
    static nsString s_sRedirectionFile;

    /// If a redirection file is used AND the redirection lookup was successful, s_sRedirectionPrefix is prepended to the redirected file
    /// access.
    static nsString s_sRedirectionPrefix;

    /// \brief When s_sRedirectionFile and s_sRedirectionPrefix are used to enable file redirection, this will reload those config files.
    virtual void ReloadExternalConfigs() override;

    virtual const nsString128& GetRedirectedDataDirectoryPath() const override { return m_sRedirectedDataDirPath; }

  protected:
    // The implementations of the abstract functions.

    virtual nsDataDirectoryReader* OpenFileToRead(nsStringView sFile, nsFileShareMode::Enum FileShareMode, bool bSpecificallyThisDataDir) override;

    virtual bool ResolveAssetRedirection(nsStringView sPathOrAssetGuid, nsStringBuilder& out_sRedirection) override;
    virtual nsDataDirectoryWriter* OpenFileToWrite(nsStringView sFile, nsFileShareMode::Enum FileShareMode) override;
    virtual void RemoveDataDirectory() override;
    virtual void DeleteFile(nsStringView sFile) override;
    virtual bool ExistsFile(nsStringView sFile, bool bOneSpecificDataDir) override;
    virtual nsResult GetFileStats(nsStringView sFileOrFolder, bool bOneSpecificDataDir, nsFileStats& out_Stats) override;
    virtual FolderReader* CreateFolderReader() const;
    virtual FolderWriter* CreateFolderWriter() const;

    /// \brief Called by 'nsDataDirectoryType_Folder::Factory'
    virtual nsResult InternalInitializeDataDirectory(nsStringView sDirectory) override;

    /// \brief Marks the given reader/writer as reusable.
    virtual void OnReaderWriterClose(nsDataDirectoryReaderWriterBase* pClosed) override;

    void LoadRedirectionFile();

    mutable nsMutex m_ReaderWriterMutex; ///< Locks m_Readers / m_Writers as well as the m_bIsInUse flag of each reader / writer.
    nsHybridArray<nsDataDirectory::FolderReader*, 4> m_Readers;
    nsHybridArray<nsDataDirectory::FolderWriter*, 4> m_Writers;

    mutable nsMutex m_RedirectionMutex;
    nsMap<nsString, nsString> m_FileRedirection;
    nsString128 m_sRedirectedDataDirPath;
  };


  /// \brief Handles reading from ordinary files.
  class NS_FOUNDATION_DLL FolderReader : public nsDataDirectoryReader
  {
    NS_DISALLOW_COPY_AND_ASSIGN(FolderReader);

  public:
    FolderReader(nsInt32 iDataDirUserData)
      : nsDataDirectoryReader(iDataDirUserData)
    {
      m_bIsInUse = false;
    }

    virtual nsUInt64 Skip(nsUInt64 uiBytes) override;
    virtual nsUInt64 Read(void* pBuffer, nsUInt64 uiBytes) override;
    virtual nsUInt64 GetFileSize() const override;

  protected:
    virtual nsResult InternalOpen(nsFileShareMode::Enum FileShareMode) override;
    virtual void InternalClose() override;

    friend class FolderType;

    bool m_bIsInUse;
    nsOSFile m_File;
  };

  /// \brief Handles writing to ordinary files.
  class NS_FOUNDATION_DLL FolderWriter : public nsDataDirectoryWriter
  {
    NS_DISALLOW_COPY_AND_ASSIGN(FolderWriter);

  public:
    FolderWriter(nsInt32 iDataDirUserData = 0)
      : nsDataDirectoryWriter(iDataDirUserData)
    {
      m_bIsInUse = false;
    }

    virtual nsResult Write(const void* pBuffer, nsUInt64 uiBytes) override;
    virtual nsUInt64 GetFileSize() const override;

  protected:
    virtual nsResult InternalOpen(nsFileShareMode::Enum FileShareMode) override;
    virtual void InternalClose() override;

    friend class FolderType;

    bool m_bIsInUse;
    nsOSFile m_File;
  };
} // namespace nsDataDirectory

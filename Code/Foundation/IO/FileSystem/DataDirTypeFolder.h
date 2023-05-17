#pragma once

#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/FileSystem/Implementation/DataDirType.h>
#include <Foundation/IO/OSFile.h>

namespace wdDataDirectory
{
  class FolderReader;
  class FolderWriter;

  /// \brief A data directory type to handle access to ordinary files.
  ///
  /// Register the 'Factory' function at wdFileSystem to allow it to mount local directories.
  class WD_FOUNDATION_DLL FolderType : public wdDataDirectoryType
  {
  public:
    ~FolderType();

    /// \brief The factory that can be registered at wdFileSystem to create data directories of this type.
    static wdDataDirectoryType* Factory(wdStringView sDataDirectory, wdStringView sGroup, wdStringView sRootName, wdFileSystem::DataDirUsage usage);

    /// A 'redirection file' is an optional file located inside a data directory that lists which file access is redirected to which other
    /// file lookup. Each redirection is one line in the file (terminated by a \n). Each line consists of the 'key' string, a semicolon and
    /// a 'value' string. No unnecessary whitespace is allowed. When a file that matches 'key' is accessed through a mounted data directory,
    /// the file access will be replaced by 'value' (plus s_sRedirectionPrefix) 'key' may be anything (e.g. a GUID string), 'value' should
    /// be a valid relative path into the SAME data directory. The redirection file can be used to implement an asset lookup, where assets
    /// are identified by GUIDs and need to be mapped to the actual asset file.
    static wdString s_sRedirectionFile;

    /// If a redirection file is used AND the redirection lookup was successful, s_sRedirectionPrefix is prepended to the redirected file
    /// access.
    static wdString s_sRedirectionPrefix;

    /// \brief When s_sRedirectionFile and s_sRedirectionPrefix are used to enable file redirection, this will reload those config files.
    virtual void ReloadExternalConfigs() override;

    virtual const wdString128& GetRedirectedDataDirectoryPath() const override { return m_sRedirectedDataDirPath; }

  protected:
    // The implementations of the abstract functions.

    virtual wdDataDirectoryReader* OpenFileToRead(wdStringView sFile, wdFileShareMode::Enum FileShareMode, bool bSpecificallyThisDataDir) override;

    virtual bool ResolveAssetRedirection(wdStringView sPathOrAssetGuid, wdStringBuilder& out_sRedirection) override;
    virtual wdDataDirectoryWriter* OpenFileToWrite(wdStringView sFile, wdFileShareMode::Enum FileShareMode) override;
    virtual void RemoveDataDirectory() override;
    virtual void DeleteFile(wdStringView sFile) override;
    virtual bool ExistsFile(wdStringView sFile, bool bOneSpecificDataDir) override;
    virtual wdResult GetFileStats(wdStringView sFileOrFolder, bool bOneSpecificDataDir, wdFileStats& out_Stats) override;
    virtual FolderReader* CreateFolderReader() const;
    virtual FolderWriter* CreateFolderWriter() const;

    /// \brief Called by 'wdDataDirectoryType_Folder::Factory'
    virtual wdResult InternalInitializeDataDirectory(wdStringView sDirectory) override;

    /// \brief Marks the given reader/writer as reusable.
    virtual void OnReaderWriterClose(wdDataDirectoryReaderWriterBase* pClosed) override;

    void LoadRedirectionFile();

    mutable wdMutex m_ReaderWriterMutex; ///< Locks m_Readers / m_Writers as well as the m_bIsInUse flag of each reader / writer.
    wdHybridArray<wdDataDirectory::FolderReader*, 4> m_Readers;
    wdHybridArray<wdDataDirectory::FolderWriter*, 4> m_Writers;

    mutable wdMutex m_RedirectionMutex;
    wdMap<wdString, wdString> m_FileRedirection;
    wdString128 m_sRedirectedDataDirPath;
  };


  /// \brief Handles reading from ordinary files.
  class WD_FOUNDATION_DLL FolderReader : public wdDataDirectoryReader
  {
    WD_DISALLOW_COPY_AND_ASSIGN(FolderReader);

  public:
    FolderReader(wdInt32 iDataDirUserData)
      : wdDataDirectoryReader(iDataDirUserData)
    {
      m_bIsInUse = false;
    }

    virtual wdUInt64 Read(void* pBuffer, wdUInt64 uiBytes) override;
    virtual wdUInt64 GetFileSize() const override;

  protected:
    virtual wdResult InternalOpen(wdFileShareMode::Enum FileShareMode) override;
    virtual void InternalClose() override;

    friend class FolderType;

    bool m_bIsInUse;
    wdOSFile m_File;
  };

  /// \brief Handles writing to ordinary files.
  class WD_FOUNDATION_DLL FolderWriter : public wdDataDirectoryWriter
  {
    WD_DISALLOW_COPY_AND_ASSIGN(FolderWriter);

  public:
    FolderWriter(wdInt32 iDataDirUserData = 0)
      : wdDataDirectoryWriter(iDataDirUserData)
    {
      m_bIsInUse = false;
    }

    virtual wdResult Write(const void* pBuffer, wdUInt64 uiBytes) override;
    virtual wdUInt64 GetFileSize() const override;

  protected:
    virtual wdResult InternalOpen(wdFileShareMode::Enum FileShareMode) override;
    virtual void InternalClose() override;

    friend class FolderType;

    bool m_bIsInUse;
    wdOSFile m_File;
  };
} // namespace wdDataDirectory

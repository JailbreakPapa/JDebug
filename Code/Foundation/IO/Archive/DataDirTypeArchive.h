#pragma once

#include <Foundation/IO/Archive/ArchiveReader.h>
#include <Foundation/IO/CompressedStreamZstd.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/FileSystem/Implementation/DataDirType.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Time/Timestamp.h>

class wdArchiveEntry;

namespace wdDataDirectory
{
  class ArchiveReaderUncompressed;
  class ArchiveReaderZstd;
  class ArchiveReaderZip;

  class WD_FOUNDATION_DLL ArchiveType : public wdDataDirectoryType
  {
  public:
    ArchiveType();
    ~ArchiveType();

    static wdDataDirectoryType* Factory(wdStringView sDataDirectory, wdStringView sGroup, wdStringView sRootName, wdFileSystem::DataDirUsage usage);

    virtual const wdString128& GetRedirectedDataDirectoryPath() const override { return m_sRedirectedDataDirPath; }

  protected:
    virtual wdDataDirectoryReader* OpenFileToRead(wdStringView sFile, wdFileShareMode::Enum FileShareMode, bool bSpecificallyThisDataDir) override;

    virtual void RemoveDataDirectory() override;

    virtual bool ExistsFile(wdStringView sFile, bool bOneSpecificDataDir) override;

    virtual wdResult GetFileStats(wdStringView sFileOrFolder, bool bOneSpecificDataDir, wdFileStats& out_Stats) override;

    virtual wdResult InternalInitializeDataDirectory(wdStringView sDirectory) override;

    virtual void OnReaderWriterClose(wdDataDirectoryReaderWriterBase* pClosed) override;

    wdString128 m_sRedirectedDataDirPath;
    wdString32 m_sArchiveSubFolder;
    wdTimestamp m_LastModificationTime;
    wdArchiveReader m_ArchiveReader;

    wdMutex m_ReaderMutex;
    wdHybridArray<wdUniquePtr<ArchiveReaderUncompressed>, 4> m_ReadersUncompressed;
    wdHybridArray<ArchiveReaderUncompressed*, 4> m_FreeReadersUncompressed;

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
    wdHybridArray<wdUniquePtr<ArchiveReaderZstd>, 4> m_ReadersZstd;
    wdHybridArray<ArchiveReaderZstd*, 4> m_FreeReadersZstd;
#endif
  };

  class WD_FOUNDATION_DLL ArchiveReaderUncompressed : public wdDataDirectoryReader
  {
    WD_DISALLOW_COPY_AND_ASSIGN(ArchiveReaderUncompressed);

  public:
    ArchiveReaderUncompressed(wdInt32 iDataDirUserData);
    ~ArchiveReaderUncompressed();

    virtual wdUInt64 Read(void* pBuffer, wdUInt64 uiBytes) override;
    virtual wdUInt64 GetFileSize() const override;

  protected:
    virtual wdResult InternalOpen(wdFileShareMode::Enum FileShareMode) override;
    virtual void InternalClose() override;

    friend class ArchiveType;

    wdUInt64 m_uiUncompressedSize = 0;
    wdUInt64 m_uiCompressedSize = 0;
    wdRawMemoryStreamReader m_MemStreamReader;
  };

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
  class WD_FOUNDATION_DLL ArchiveReaderZstd : public ArchiveReaderUncompressed
  {
    WD_DISALLOW_COPY_AND_ASSIGN(ArchiveReaderZstd);

  public:
    ArchiveReaderZstd(wdInt32 iDataDirUserData);
    ~ArchiveReaderZstd();

    virtual wdUInt64 Read(void* pBuffer, wdUInt64 uiBytes) override;

  protected:
    virtual wdResult InternalOpen(wdFileShareMode::Enum FileShareMode) override;

    friend class ArchiveType;

    wdCompressedStreamReaderZstd m_CompressedStreamReader;
  };
#endif


} // namespace wdDataDirectory

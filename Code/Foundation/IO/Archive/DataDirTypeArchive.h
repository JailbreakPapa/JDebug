#pragma once

#include <Foundation/IO/Archive/ArchiveReader.h>
#include <Foundation/IO/CompressedStreamZlib.h>
#include <Foundation/IO/CompressedStreamZstd.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/FileSystem/Implementation/DataDirType.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Time/Timestamp.h>

class nsArchiveEntry;

namespace nsDataDirectory
{
  class ArchiveReaderUncompressed;
  class ArchiveReaderZstd;
  class ArchiveReaderZip;

  class NS_FOUNDATION_DLL ArchiveType : public nsDataDirectoryType
  {
  public:
    ArchiveType();
    ~ArchiveType();

    static nsDataDirectoryType* Factory(nsStringView sDataDirectory, nsStringView sGroup, nsStringView sRootName, nsFileSystem::DataDirUsage usage);

    virtual const nsString128& GetRedirectedDataDirectoryPath() const override { return m_sRedirectedDataDirPath; }

  protected:
    virtual nsDataDirectoryReader* OpenFileToRead(nsStringView sFile, nsFileShareMode::Enum FileShareMode, bool bSpecificallyThisDataDir) override;

    virtual void RemoveDataDirectory() override;

    virtual bool ExistsFile(nsStringView sFile, bool bOneSpecificDataDir) override;

    virtual nsResult GetFileStats(nsStringView sFileOrFolder, bool bOneSpecificDataDir, nsFileStats& out_Stats) override;

    virtual nsResult InternalInitializeDataDirectory(nsStringView sDirectory) override;

    virtual void OnReaderWriterClose(nsDataDirectoryReaderWriterBase* pClosed) override;

    nsString128 m_sRedirectedDataDirPath;
    nsString32 m_sArchiveSubFolder;
    nsTimestamp m_LastModificationTime;
    nsArchiveReader m_ArchiveReader;

    nsMutex m_ReaderMutex;
    nsHybridArray<nsUniquePtr<ArchiveReaderUncompressed>, 4> m_ReadersUncompressed;
    nsHybridArray<ArchiveReaderUncompressed*, 4> m_FreeReadersUncompressed;

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
    nsHybridArray<nsUniquePtr<ArchiveReaderZstd>, 4> m_ReadersZstd;
    nsHybridArray<ArchiveReaderZstd*, 4> m_FreeReadersZstd;
#endif
#ifdef BUILDSYSTEM_ENABLE_ZLIB_SUPPORT
    nsHybridArray<nsUniquePtr<ArchiveReaderZip>, 4> m_ReadersZip;
    nsHybridArray<ArchiveReaderZip*, 4> m_FreeReadersZip;
#endif
  };

  class NS_FOUNDATION_DLL ArchiveReaderCommon : public nsDataDirectoryReader
  {
    NS_DISALLOW_COPY_AND_ASSIGN(ArchiveReaderCommon);

  public:
    ArchiveReaderCommon(nsInt32 iDataDirUserData);

    virtual nsUInt64 GetFileSize() const override;

  protected:
    friend class ArchiveType;

    nsUInt64 m_uiUncompressedSize = 0;
    nsUInt64 m_uiCompressedSize = 0;
    nsRawMemoryStreamReader m_MemStreamReader;
  };

  class NS_FOUNDATION_DLL ArchiveReaderUncompressed : public ArchiveReaderCommon
  {
    NS_DISALLOW_COPY_AND_ASSIGN(ArchiveReaderUncompressed);

  public:
    ArchiveReaderUncompressed(nsInt32 iDataDirUserData);

    virtual nsUInt64 Skip(nsUInt64 uiBytes) override;
    virtual nsUInt64 Read(void* pBuffer, nsUInt64 uiBytes) override;

  protected:
    virtual nsResult InternalOpen(nsFileShareMode::Enum FileShareMode) override;
    virtual void InternalClose() override;
  };

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
  class NS_FOUNDATION_DLL ArchiveReaderZstd : public ArchiveReaderCommon
  {
    NS_DISALLOW_COPY_AND_ASSIGN(ArchiveReaderZstd);

  public:
    ArchiveReaderZstd(nsInt32 iDataDirUserData);

    virtual nsUInt64 Read(void* pBuffer, nsUInt64 uiBytes) override;

  protected:
    virtual nsResult InternalOpen(nsFileShareMode::Enum FileShareMode) override;
    virtual void InternalClose() override;

    nsCompressedStreamReaderZstd m_CompressedStreamReader;
  };
#endif

#ifdef BUILDSYSTEM_ENABLE_ZLIB_SUPPORT
  /// \brief Allows reading of zip / apk containers.
  /// Needed to allow Android to read data from the apk.
  class NS_FOUNDATION_DLL ArchiveReaderZip : public ArchiveReaderUncompressed
  {
    NS_DISALLOW_COPY_AND_ASSIGN(ArchiveReaderZip);

  public:
    ArchiveReaderZip(nsInt32 iDataDirUserData);
    ~ArchiveReaderZip();

    virtual nsUInt64 Read(void* pBuffer, nsUInt64 uiBytes) override;

  protected:
    virtual nsResult InternalOpen(nsFileShareMode::Enum FileShareMode) override;

    friend class ArchiveType;

    nsCompressedStreamReaderZip m_CompressedStreamReader;
  };
#endif
} // namespace nsDataDirectory

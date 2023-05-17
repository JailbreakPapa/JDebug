#include <Foundation/FoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/Archive/ArchiveUtils.h>
#include <Foundation/IO/Archive/DataDirTypeArchive.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>

// clang-format off
WD_BEGIN_SUBSYSTEM_DECLARATION(Foundation, ArchiveDataDirectory)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "FileSystem", "FolderDataDirectory"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
  wdFileSystem::RegisterDataDirectoryFactory(wdDataDirectory::ArchiveType::Factory);
  }

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

wdDataDirectory::ArchiveType::ArchiveType() = default;
wdDataDirectory::ArchiveType::~ArchiveType() = default;

wdDataDirectoryType* wdDataDirectory::ArchiveType::Factory(wdStringView sDataDirectory, wdStringView sGroup, wdStringView sRootName, wdFileSystem::DataDirUsage usage)
{
  ArchiveType* pDataDir = WD_DEFAULT_NEW(ArchiveType);

  if (pDataDir->InitializeDataDirectory(sDataDirectory) == WD_SUCCESS)
    return pDataDir;

  WD_DEFAULT_DELETE(pDataDir);
  return nullptr;
}

wdDataDirectoryReader* wdDataDirectory::ArchiveType::OpenFileToRead(wdStringView sFile, wdFileShareMode::Enum FileShareMode, bool bSpecificallyThisDataDir)
{
  const wdArchiveTOC& toc = m_ArchiveReader.GetArchiveTOC();
  wdStringBuilder sArchivePath = m_sArchiveSubFolder;
  sArchivePath.AppendPath(sFile);

  const wdUInt32 uiEntryIndex = toc.FindEntry(sArchivePath);

  if (uiEntryIndex == wdInvalidIndex)
    return nullptr;

  const wdArchiveEntry* pEntry = &toc.m_Entries[uiEntryIndex];

  ArchiveReaderUncompressed* pReader = nullptr;

  {
    WD_LOCK(m_ReaderMutex);

    switch (pEntry->m_CompressionMode)
    {
      case wdArchiveCompressionMode::Uncompressed:
      {
        if (!m_FreeReadersUncompressed.IsEmpty())
        {
          pReader = m_FreeReadersUncompressed.PeekBack();
          m_FreeReadersUncompressed.PopBack();
        }
        else
        {
          m_ReadersUncompressed.PushBack(WD_DEFAULT_NEW(ArchiveReaderUncompressed, 0));
          pReader = m_ReadersUncompressed.PeekBack().Borrow();
        }
        break;
      }

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
      case wdArchiveCompressionMode::Compressed_zstd:
      {
        if (!m_FreeReadersZstd.IsEmpty())
        {
          pReader = m_FreeReadersZstd.PeekBack();
          m_FreeReadersZstd.PopBack();
        }
        else
        {
          m_ReadersZstd.PushBack(WD_DEFAULT_NEW(ArchiveReaderZstd, 1));
          pReader = m_ReadersZstd.PeekBack().Borrow();
        }
        break;
      }
#endif

      default:
        WD_REPORT_FAILURE("Compression mode {} is unknown (or not compiled in)", (wdUInt8)pEntry->m_CompressionMode);
        return nullptr;
    }
  }

  pReader->m_uiUncompressedSize = pEntry->m_uiUncompressedDataSize;
  pReader->m_uiCompressedSize = pEntry->m_uiStoredDataSize;

  m_ArchiveReader.ConfigureRawMemoryStreamReader(uiEntryIndex, pReader->m_MemStreamReader);

  if (pReader->Open(sArchivePath, this, FileShareMode).Failed())
  {
    WD_DEFAULT_DELETE(pReader);
    return nullptr;
  }

  return pReader;
}

void wdDataDirectory::ArchiveType::RemoveDataDirectory()
{
  ArchiveType* pThis = this;
  WD_DEFAULT_DELETE(pThis);
}

bool wdDataDirectory::ArchiveType::ExistsFile(wdStringView sFile, bool bOneSpecificDataDir)
{
  wdStringBuilder sArchivePath = m_sArchiveSubFolder;
  sArchivePath.AppendPath(sFile);
  return m_ArchiveReader.GetArchiveTOC().FindEntry(sArchivePath) != wdInvalidIndex;
}

wdResult wdDataDirectory::ArchiveType::GetFileStats(wdStringView sFileOrFolder, bool bOneSpecificDataDir, wdFileStats& out_Stats)
{
  const wdArchiveTOC& toc = m_ArchiveReader.GetArchiveTOC();
  wdStringBuilder sArchivePath = m_sArchiveSubFolder;
  sArchivePath.AppendPath(sFileOrFolder);
  const wdUInt32 uiEntryIndex = toc.FindEntry(sArchivePath);

  if (uiEntryIndex == wdInvalidIndex)
    return WD_FAILURE;

  const wdArchiveEntry* pEntry = &toc.m_Entries[uiEntryIndex];

  const char* szPath = toc.GetEntryPathString(uiEntryIndex);

  out_Stats.m_bIsDirectory = false;
  out_Stats.m_LastModificationTime = m_LastModificationTime;
  out_Stats.m_uiFileSize = pEntry->m_uiUncompressedDataSize;
  out_Stats.m_sParentPath = szPath;
  out_Stats.m_sParentPath.PathParentDirectory();
  out_Stats.m_sName = wdPathUtils::GetFileNameAndExtension(szPath);

  return WD_SUCCESS;
}

wdResult wdDataDirectory::ArchiveType::InternalInitializeDataDirectory(wdStringView sDirectory)
{
  wdStringBuilder sRedirected;
  WD_SUCCEED_OR_RETURN(wdFileSystem::ResolveSpecialDirectory(sDirectory, sRedirected));

  sRedirected.MakeCleanPath();
  // remove trailing slashes
  sRedirected.Trim("", "/");
  m_sRedirectedDataDirPath = sRedirected;

  bool bSupported = false;
  wdStringBuilder sArchivePath;

  wdHybridArray<wdString, 4, wdStaticAllocatorWrapper> extensions = wdArchiveUtils::GetAcceptedArchiveFileExtensions();


  for (const auto& ext : extensions)
  {
    const wdUInt32 uiLength = ext.GetElementCount();
    if (sRedirected.HasExtension(ext))
    {
      sArchivePath = sRedirected;
      m_sArchiveSubFolder = "";
      bSupported = true;
      goto endloop;
    }
    const char* szFound = nullptr;
    do
    {
      szFound = sRedirected.FindLastSubString_NoCase(ext, szFound);
      if (szFound != nullptr && szFound[uiLength] == '/')
      {
        sArchivePath = wdStringView(sRedirected.GetData(), szFound + uiLength);
        m_sArchiveSubFolder = szFound + uiLength + 1;
        bSupported = true;
        goto endloop;
      }

    } while (szFound != nullptr);
  }
endloop:
  if (!bSupported)
    return WD_FAILURE;

  wdFileStats stats;
  if (wdOSFile::GetFileStats(sArchivePath, stats).Failed())
    return WD_FAILURE;

  WD_LOG_BLOCK("wdArchiveDataDir", sDirectory);

  m_LastModificationTime = stats.m_LastModificationTime;

  WD_SUCCEED_OR_RETURN(m_ArchiveReader.OpenArchive(sArchivePath));

  ReloadExternalConfigs();

  return WD_SUCCESS;
}

void wdDataDirectory::ArchiveType::OnReaderWriterClose(wdDataDirectoryReaderWriterBase* pClosed)
{
  WD_LOCK(m_ReaderMutex);

  if (pClosed->GetDataDirUserData() == 0)
  {
    m_FreeReadersUncompressed.PushBack(static_cast<ArchiveReaderUncompressed*>(pClosed));
    return;
  }

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
  if (pClosed->GetDataDirUserData() == 1)
  {
    m_FreeReadersZstd.PushBack(static_cast<ArchiveReaderZstd*>(pClosed));
    return;
  }
#endif


  WD_ASSERT_NOT_IMPLEMENTED;
}

//////////////////////////////////////////////////////////////////////////

wdDataDirectory::ArchiveReaderUncompressed::ArchiveReaderUncompressed(wdInt32 iDataDirUserData)
  : wdDataDirectoryReader(iDataDirUserData)
{
}

wdDataDirectory::ArchiveReaderUncompressed::~ArchiveReaderUncompressed() = default;

wdUInt64 wdDataDirectory::ArchiveReaderUncompressed::Read(void* pBuffer, wdUInt64 uiBytes)
{
  return m_MemStreamReader.ReadBytes(pBuffer, uiBytes);
}

wdUInt64 wdDataDirectory::ArchiveReaderUncompressed::GetFileSize() const
{
  return m_uiUncompressedSize;
}

wdResult wdDataDirectory::ArchiveReaderUncompressed::InternalOpen(wdFileShareMode::Enum FileShareMode)
{
  WD_ASSERT_DEBUG(FileShareMode != wdFileShareMode::Exclusive, "Archives only support shared reading of files. Exclusive access cannot be guaranteed.");

  // nothing to do
  return WD_SUCCESS;
}

void wdDataDirectory::ArchiveReaderUncompressed::InternalClose()
{
  // nothing to do
}

//////////////////////////////////////////////////////////////////////////

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT

wdDataDirectory::ArchiveReaderZstd::ArchiveReaderZstd(wdInt32 iDataDirUserData)
  : ArchiveReaderUncompressed(iDataDirUserData)
{
}

wdDataDirectory::ArchiveReaderZstd::~ArchiveReaderZstd() = default;

wdUInt64 wdDataDirectory::ArchiveReaderZstd::Read(void* pBuffer, wdUInt64 uiBytes)
{
  return m_CompressedStreamReader.ReadBytes(pBuffer, uiBytes);
}

wdResult wdDataDirectory::ArchiveReaderZstd::InternalOpen(wdFileShareMode::Enum FileShareMode)
{
  WD_ASSERT_DEBUG(FileShareMode != wdFileShareMode::Exclusive, "Archives only support shared reading of files. Exclusive access cannot be guaranteed.");

  m_CompressedStreamReader.SetInputStream(&m_MemStreamReader);
  return WD_SUCCESS;
}

#endif

//////////////////////////////////////////////////////////////////////////



WD_STATICLINK_FILE(Foundation, Foundation_IO_Archive_Implementation_DataDirTypeArchive);

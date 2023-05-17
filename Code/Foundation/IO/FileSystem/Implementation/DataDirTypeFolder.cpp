#include <Foundation/FoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/FileSystem/DataDirTypeFolder.h>
#include <Foundation/Logging/Log.h>

// clang-format off
WD_BEGIN_SUBSYSTEM_DECLARATION(Foundation, FolderDataDirectory)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "FileSystem"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    wdFileSystem::RegisterDataDirectoryFactory(wdDataDirectory::FolderType::Factory);
  }

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

namespace wdDataDirectory
{
  wdString FolderType::s_sRedirectionFile;
  wdString FolderType::s_sRedirectionPrefix;

  wdResult FolderReader::InternalOpen(wdFileShareMode::Enum FileShareMode)
  {
    wdStringBuilder sPath = ((wdDataDirectory::FolderType*)GetDataDirectory())->GetRedirectedDataDirectoryPath();
    sPath.AppendPath(GetFilePath());

    return m_File.Open(sPath.GetData(), wdFileOpenMode::Read, FileShareMode);
  }

  void FolderReader::InternalClose()
  {
    m_File.Close();
  }

  wdUInt64 FolderReader::Read(void* pBuffer, wdUInt64 uiBytes)
  {
    return m_File.Read(pBuffer, uiBytes);
  }

  wdUInt64 FolderReader::GetFileSize() const
  {
    return m_File.GetFileSize();
  }

  wdResult FolderWriter::InternalOpen(wdFileShareMode::Enum FileShareMode)
  {
    wdStringBuilder sPath = ((wdDataDirectory::FolderType*)GetDataDirectory())->GetRedirectedDataDirectoryPath();
    sPath.AppendPath(GetFilePath());

    return m_File.Open(sPath.GetData(), wdFileOpenMode::Write, FileShareMode);
  }

  void FolderWriter::InternalClose()
  {
    m_File.Close();
  }

  wdResult FolderWriter::Write(const void* pBuffer, wdUInt64 uiBytes)
  {
    return m_File.Write(pBuffer, uiBytes);
  }

  wdUInt64 FolderWriter::GetFileSize() const
  {
    return m_File.GetFileSize();
  }

  wdDataDirectoryType* FolderType::Factory(wdStringView sDataDirectory, wdStringView sGroup, wdStringView sRootName, wdFileSystem::DataDirUsage usage)
  {
    FolderType* pDataDir = WD_DEFAULT_NEW(FolderType);

    if (pDataDir->InitializeDataDirectory(sDataDirectory) == WD_SUCCESS)
      return pDataDir;

    WD_DEFAULT_DELETE(pDataDir);
    return nullptr;
  }

  void FolderType::RemoveDataDirectory()
  {
    {
      WD_LOCK(m_ReaderWriterMutex);
      for (wdUInt32 i = 0; i < m_Readers.GetCount(); ++i)
      {
        WD_ASSERT_DEV(!m_Readers[i]->m_bIsInUse, "Cannot remove a data directory while there are still files open in it.");
      }

      for (wdUInt32 i = 0; i < m_Writers.GetCount(); ++i)
      {
        WD_ASSERT_DEV(!m_Writers[i]->m_bIsInUse, "Cannot remove a data directory while there are still files open in it.");
      }
    }
    FolderType* pThis = this;
    WD_DEFAULT_DELETE(pThis);
  }

  void FolderType::DeleteFile(wdStringView sFile)
  {
    wdStringBuilder sPath = GetRedirectedDataDirectoryPath();
    sPath.AppendPath(sFile);

    wdOSFile::DeleteFile(sPath.GetData()).IgnoreResult();
  }

  FolderType::~FolderType()
  {
    WD_LOCK(m_ReaderWriterMutex);
    for (wdUInt32 i = 0; i < m_Readers.GetCount(); ++i)
      WD_DEFAULT_DELETE(m_Readers[i]);

    for (wdUInt32 i = 0; i < m_Writers.GetCount(); ++i)
      WD_DEFAULT_DELETE(m_Writers[i]);
  }

  void FolderType::ReloadExternalConfigs()
  {
    LoadRedirectionFile();
  }

  void FolderType::LoadRedirectionFile()
  {
    WD_LOCK(m_RedirectionMutex);
    m_FileRedirection.Clear();

    if (!s_sRedirectionFile.IsEmpty())
    {
      wdStringBuilder sRedirectionFile(GetRedirectedDataDirectoryPath(), "/", s_sRedirectionFile);
      sRedirectionFile.MakeCleanPath();

      WD_LOG_BLOCK("LoadRedirectionFile", sRedirectionFile.GetData());

      wdOSFile file;
      if (file.Open(sRedirectionFile, wdFileOpenMode::Read).Succeeded())
      {
        wdHybridArray<char, 1024 * 10> content;
        char uiTemp[4096];

        wdUInt64 uiRead = 0;

        do
        {
          uiRead = file.Read(uiTemp, WD_ARRAY_SIZE(uiTemp));
          content.PushBackRange(wdArrayPtr<char>(uiTemp, (wdUInt32)uiRead));
        } while (uiRead == WD_ARRAY_SIZE(uiTemp));

        content.PushBack(0); // make sure the string is terminated

        const char* szLineStart = content.GetData();
        const char* szSeparator = nullptr;
        const char* szLineEnd = nullptr;

        wdStringBuilder sFileToRedirect, sRedirection;

        while (true)
        {
          szSeparator = wdStringUtils::FindSubString(szLineStart, ";");
          szLineEnd = wdStringUtils::FindSubString(szSeparator, "\n");

          if (szLineStart == nullptr || szSeparator == nullptr || szLineEnd == nullptr)
            break;

          sFileToRedirect.SetSubString_FromTo(szLineStart, szSeparator);
          sRedirection.SetSubString_FromTo(szSeparator + 1, szLineEnd);

          m_FileRedirection[sFileToRedirect] = sRedirection;

          szLineStart = szLineEnd + 1;
        }

        // wdLog::Debug("Redirection file contains {0} entries", m_FileRedirection.GetCount());
      }
      // else
      // wdLog::Debug("No Redirection file found in: '{0}'", sRedirectionFile);
    }
  }


  bool FolderType::ExistsFile(wdStringView sFile, bool bOneSpecificDataDir)
  {
    wdStringBuilder sRedirectedAsset;
    ResolveAssetRedirection(sFile, sRedirectedAsset);

    wdStringBuilder sPath = GetRedirectedDataDirectoryPath();
    sPath.AppendPath(sRedirectedAsset);
    return wdOSFile::ExistsFile(sPath);
  }

  wdResult FolderType::GetFileStats(wdStringView sFileOrFolder, bool bOneSpecificDataDir, wdFileStats& out_Stats)
  {
    wdStringBuilder sRedirectedAsset;
    ResolveAssetRedirection(sFileOrFolder, sRedirectedAsset);

    wdStringBuilder sPath = GetRedirectedDataDirectoryPath();

    if (wdPathUtils::IsAbsolutePath(sRedirectedAsset))
    {
      if (!sRedirectedAsset.StartsWith_NoCase(sPath))
        return WD_FAILURE;

      sPath.Clear();
    }

    sPath.AppendPath(sRedirectedAsset);

    if (!wdPathUtils::IsAbsolutePath(sPath))
      return WD_FAILURE;

    return wdOSFile::GetFileStats(sPath, out_Stats);
  }

  wdResult FolderType::InternalInitializeDataDirectory(wdStringView sDirectory)
  {
    // allow to set the 'empty' directory to handle all absolute paths
    if (sDirectory.IsEmpty())
      return WD_SUCCESS;

    wdStringBuilder sRedirected;
    if (wdFileSystem::ResolveSpecialDirectory(sDirectory, sRedirected).Succeeded())
    {
      m_sRedirectedDataDirPath = sRedirected;
    }
    else
    {
      m_sRedirectedDataDirPath = sDirectory;
    }

    if (!wdOSFile::ExistsDirectory(m_sRedirectedDataDirPath))
      return WD_FAILURE;

    ReloadExternalConfigs();

    return WD_SUCCESS;
  }

  void FolderType::OnReaderWriterClose(wdDataDirectoryReaderWriterBase* pClosed)
  {
    WD_LOCK(m_ReaderWriterMutex);
    if (pClosed->IsReader())
    {
      FolderReader* pReader = (FolderReader*)pClosed;
      pReader->m_bIsInUse = false;
    }
    else
    {
      FolderWriter* pWriter = (FolderWriter*)pClosed;
      pWriter->m_bIsInUse = false;
    }
  }

  wdDataDirectory::FolderReader* FolderType::CreateFolderReader() const
  {
    return WD_DEFAULT_NEW(FolderReader, 0);
  }

  wdDataDirectory::FolderWriter* FolderType::CreateFolderWriter() const
  {
    return WD_DEFAULT_NEW(FolderWriter, 0);
  }

  wdDataDirectoryReader* FolderType::OpenFileToRead(wdStringView sFile, wdFileShareMode::Enum FileShareMode, bool bSpecificallyThisDataDir)
  {
    wdStringBuilder sFileToOpen;
    ResolveAssetRedirection(sFile, sFileToOpen);

    // we know that these files cannot be opened, so don't even try
    if (wdConversionUtils::IsStringUuid(sFileToOpen))
      return nullptr;

    FolderReader* pReader = nullptr;
    {
      WD_LOCK(m_ReaderWriterMutex);
      for (wdUInt32 i = 0; i < m_Readers.GetCount(); ++i)
      {
        if (!m_Readers[i]->m_bIsInUse)
          pReader = m_Readers[i];
      }

      if (pReader == nullptr)
      {
        m_Readers.PushBack(CreateFolderReader());
        pReader = m_Readers.PeekBack();
      }
      pReader->m_bIsInUse = true;
    }

    // if opening the file fails, the reader's m_bIsInUse needs to be reset.
    if (pReader->Open(sFileToOpen, this, FileShareMode) == WD_FAILURE)
    {
      WD_LOCK(m_ReaderWriterMutex);
      pReader->m_bIsInUse = false;
      return nullptr;
    }

    // if it succeeds, we return the reader
    return pReader;
  }


  bool FolderType::ResolveAssetRedirection(wdStringView sFile, wdStringBuilder& out_sRedirection)
  {
    WD_LOCK(m_RedirectionMutex);
    // Check if we know about a file redirection for this
    auto it = m_FileRedirection.Find(sFile);

    // if available, open the file that is mentioned in the redirection file instead
    if (it.IsValid())
    {

      if (it.Value().StartsWith("?"))
      {
        // ? is an option to tell the system to skip the redirection prefix and use the path as is
        out_sRedirection = &it.Value().GetData()[1];
      }
      else
      {
        out_sRedirection.Set(s_sRedirectionPrefix, it.Value());
      }
      return true;
    }
    else
    {
      out_sRedirection = sFile;
      return false;
    }
  }

  wdDataDirectoryWriter* FolderType::OpenFileToWrite(wdStringView sFile, wdFileShareMode::Enum FileShareMode)
  {
    FolderWriter* pWriter = nullptr;

    {
      WD_LOCK(m_ReaderWriterMutex);
      for (wdUInt32 i = 0; i < m_Writers.GetCount(); ++i)
      {
        if (!m_Writers[i]->m_bIsInUse)
          pWriter = m_Writers[i];
      }

      if (pWriter == nullptr)
      {
        m_Writers.PushBack(CreateFolderWriter());
        pWriter = m_Writers.PeekBack();
      }
      pWriter->m_bIsInUse = true;
    }
    // if opening the file fails, the writer's m_bIsInUse needs to be reset.
    if (pWriter->Open(sFile, this, FileShareMode) == WD_FAILURE)
    {
      WD_LOCK(m_ReaderWriterMutex);
      pWriter->m_bIsInUse = false;
      return nullptr;
    }

    // if it succeeds, we return the reader
    return pWriter;
  }
} // namespace wdDataDirectory



WD_STATICLINK_FILE(Foundation, Foundation_IO_FileSystem_Implementation_DataDirTypeFolder);

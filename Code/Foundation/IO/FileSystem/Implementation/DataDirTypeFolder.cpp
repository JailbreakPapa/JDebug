#include <Foundation/FoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/FileSystem/DataDirTypeFolder.h>
#include <Foundation/Logging/Log.h>

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(Foundation, FolderDataDirectory)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "FileSystem"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    nsFileSystem::RegisterDataDirectoryFactory(nsDataDirectory::FolderType::Factory);
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

namespace nsDataDirectory
{
  nsString FolderType::s_sRedirectionFile;
  nsString FolderType::s_sRedirectionPrefix;

  nsResult FolderReader::InternalOpen(nsFileShareMode::Enum FileShareMode)
  {
    nsStringBuilder sPath = ((nsDataDirectory::FolderType*)GetDataDirectory())->GetRedirectedDataDirectoryPath();
    sPath.AppendPath(GetFilePath());

    return m_File.Open(sPath.GetData(), nsFileOpenMode::Read, FileShareMode);
  }

  void FolderReader::InternalClose()
  {
    m_File.Close();
  }

  nsUInt64 FolderReader::Skip(nsUInt64 uiBytes)
  {
    if (uiBytes == 0)
    {
      return 0;
    }

    const nsUInt64 fileSize = m_File.GetFileSize();
    const nsUInt64 origFilePosition = m_File.GetFilePosition();
    NS_ASSERT_DEBUG(origFilePosition <= fileSize, "");

    const nsUInt64 newFilePosition = nsMath::Min(fileSize, origFilePosition + uiBytes);
    m_File.SetFilePosition(newFilePosition, nsFileSeekMode::FromStart);
    NS_ASSERT_DEBUG(newFilePosition == m_File.GetFilePosition(), "");

    NS_ASSERT_DEBUG(newFilePosition >= origFilePosition, "");
    return newFilePosition - origFilePosition;
  }

  nsUInt64 FolderReader::Read(void* pBuffer, nsUInt64 uiBytes)
  {
    return m_File.Read(pBuffer, uiBytes);
  }

  nsUInt64 FolderReader::GetFileSize() const
  {
    return m_File.GetFileSize();
  }

  nsResult FolderWriter::InternalOpen(nsFileShareMode::Enum FileShareMode)
  {
    nsStringBuilder sPath = ((nsDataDirectory::FolderType*)GetDataDirectory())->GetRedirectedDataDirectoryPath();
    sPath.AppendPath(GetFilePath());

    return m_File.Open(sPath.GetData(), nsFileOpenMode::Write, FileShareMode);
  }

  void FolderWriter::InternalClose()
  {
    m_File.Close();
  }

  nsResult FolderWriter::Write(const void* pBuffer, nsUInt64 uiBytes)
  {
    return m_File.Write(pBuffer, uiBytes);
  }

  nsUInt64 FolderWriter::GetFileSize() const
  {
    return m_File.GetFileSize();
  }

  nsDataDirectoryType* FolderType::Factory(nsStringView sDataDirectory, nsStringView sGroup, nsStringView sRootName, nsFileSystem::DataDirUsage usage)
  {
    FolderType* pDataDir = NS_DEFAULT_NEW(FolderType);

    if (pDataDir->InitializeDataDirectory(sDataDirectory) == NS_SUCCESS)
      return pDataDir;

    NS_DEFAULT_DELETE(pDataDir);
    return nullptr;
  }

  void FolderType::RemoveDataDirectory()
  {
    {
      NS_LOCK(m_ReaderWriterMutex);
      for (nsUInt32 i = 0; i < m_Readers.GetCount(); ++i)
      {
        NS_ASSERT_DEV(!m_Readers[i]->m_bIsInUse, "Cannot remove a data directory while there are still files open in it.");
      }

      for (nsUInt32 i = 0; i < m_Writers.GetCount(); ++i)
      {
        NS_ASSERT_DEV(!m_Writers[i]->m_bIsInUse, "Cannot remove a data directory while there are still files open in it.");
      }
    }
    FolderType* pThis = this;
    NS_DEFAULT_DELETE(pThis);
  }

  void FolderType::DeleteFile(nsStringView sFile)
  {
    nsStringBuilder sPath = GetRedirectedDataDirectoryPath();
    sPath.AppendPath(sFile);

    nsOSFile::DeleteFile(sPath.GetData()).IgnoreResult();
  }

  FolderType::~FolderType()
  {
    NS_LOCK(m_ReaderWriterMutex);
    for (nsUInt32 i = 0; i < m_Readers.GetCount(); ++i)
      NS_DEFAULT_DELETE(m_Readers[i]);

    for (nsUInt32 i = 0; i < m_Writers.GetCount(); ++i)
      NS_DEFAULT_DELETE(m_Writers[i]);
  }

  void FolderType::ReloadExternalConfigs()
  {
    LoadRedirectionFile();
  }

  void FolderType::LoadRedirectionFile()
  {
    NS_LOCK(m_RedirectionMutex);
    m_FileRedirection.Clear();

    if (!s_sRedirectionFile.IsEmpty())
    {
      nsStringBuilder sRedirectionFile(GetRedirectedDataDirectoryPath(), "/", s_sRedirectionFile);
      sRedirectionFile.MakeCleanPath();

      NS_LOG_BLOCK("LoadRedirectionFile", sRedirectionFile.GetData());

      nsOSFile file;
      if (file.Open(sRedirectionFile, nsFileOpenMode::Read).Succeeded())
      {
        nsHybridArray<char, 1024 * 10> content;
        char uiTemp[4096];

        nsUInt64 uiRead = 0;

        do
        {
          uiRead = file.Read(uiTemp, NS_ARRAY_SIZE(uiTemp));
          content.PushBackRange(nsArrayPtr<char>(uiTemp, (nsUInt32)uiRead));
        } while (uiRead == NS_ARRAY_SIZE(uiTemp));

        content.PushBack(0); // make sure the string is terminated

        const char* szLineStart = content.GetData();
        const char* szSeparator = nullptr;
        const char* szLineEnd = nullptr;

        nsStringBuilder sFileToRedirect, sRedirection;

        while (true)
        {
          szSeparator = nsStringUtils::FindSubString(szLineStart, ";");
          szLineEnd = nsStringUtils::FindSubString(szSeparator, "\n");

          if (szLineStart == nullptr || szSeparator == nullptr || szLineEnd == nullptr)
            break;

          sFileToRedirect.SetSubString_FromTo(szLineStart, szSeparator);
          sRedirection.SetSubString_FromTo(szSeparator + 1, szLineEnd);

          m_FileRedirection[sFileToRedirect] = sRedirection;

          szLineStart = szLineEnd + 1;
        }

        // nsLog::Debug("Redirection file contains {0} entries", m_FileRedirection.GetCount());
      }
      // else
      // nsLog::Debug("No Redirection file found in: '{0}'", sRedirectionFile);
    }
  }


  bool FolderType::ExistsFile(nsStringView sFile, bool bOneSpecificDataDir)
  {
    nsStringBuilder sRedirectedAsset;
    ResolveAssetRedirection(sFile, sRedirectedAsset);

    nsStringBuilder sPath = GetRedirectedDataDirectoryPath();
    sPath.AppendPath(sRedirectedAsset);
    return nsOSFile::ExistsFile(sPath);
  }

  nsResult FolderType::GetFileStats(nsStringView sFileOrFolder, bool bOneSpecificDataDir, nsFileStats& out_Stats)
  {
    nsStringBuilder sRedirectedAsset;
    ResolveAssetRedirection(sFileOrFolder, sRedirectedAsset);

    nsStringBuilder sPath = GetRedirectedDataDirectoryPath();

    if (nsPathUtils::IsAbsolutePath(sRedirectedAsset))
    {
      if (!sRedirectedAsset.StartsWith_NoCase(sPath))
        return NS_FAILURE;

      sPath.Clear();
    }

    sPath.AppendPath(sRedirectedAsset);

    if (!nsPathUtils::IsAbsolutePath(sPath))
      return NS_FAILURE;

#if NS_ENABLED(NS_SUPPORTS_FILE_STATS)
    return nsOSFile::GetFileStats(sPath, out_Stats);
#else
    return NS_FAILURE;
#endif
  }

  nsResult FolderType::InternalInitializeDataDirectory(nsStringView sDirectory)
  {
    // allow to set the 'empty' directory to handle all absolute paths
    if (sDirectory.IsEmpty())
      return NS_SUCCESS;

    nsStringBuilder sRedirected;
    if (nsFileSystem::ResolveSpecialDirectory(sDirectory, sRedirected).Succeeded())
    {
      m_sRedirectedDataDirPath = sRedirected;
    }
    else
    {
      m_sRedirectedDataDirPath = sDirectory;
    }

    if (!nsOSFile::ExistsDirectory(m_sRedirectedDataDirPath))
      return NS_FAILURE;

    ReloadExternalConfigs();

    return NS_SUCCESS;
  }

  void FolderType::OnReaderWriterClose(nsDataDirectoryReaderWriterBase* pClosed)
  {
    NS_LOCK(m_ReaderWriterMutex);
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

  nsDataDirectory::FolderReader* FolderType::CreateFolderReader() const
  {
    return NS_DEFAULT_NEW(FolderReader, 0);
  }

  nsDataDirectory::FolderWriter* FolderType::CreateFolderWriter() const
  {
    return NS_DEFAULT_NEW(FolderWriter, 0);
  }

  nsDataDirectoryReader* FolderType::OpenFileToRead(nsStringView sFile, nsFileShareMode::Enum FileShareMode, bool bSpecificallyThisDataDir)
  {
    nsStringBuilder sFileToOpen;
    ResolveAssetRedirection(sFile, sFileToOpen);

    // we know that these files cannot be opened, so don't even try
    if (nsConversionUtils::IsStringUuid(sFileToOpen))
      return nullptr;

    FolderReader* pReader = nullptr;
    {
      NS_LOCK(m_ReaderWriterMutex);
      for (nsUInt32 i = 0; i < m_Readers.GetCount(); ++i)
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
    if (pReader->Open(sFileToOpen, this, FileShareMode) == NS_FAILURE)
    {
      NS_LOCK(m_ReaderWriterMutex);
      pReader->m_bIsInUse = false;
      return nullptr;
    }

    // if it succeeds, we return the reader
    return pReader;
  }


  bool FolderType::ResolveAssetRedirection(nsStringView sFile, nsStringBuilder& out_sRedirection)
  {
    NS_LOCK(m_RedirectionMutex);
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

  nsDataDirectoryWriter* FolderType::OpenFileToWrite(nsStringView sFile, nsFileShareMode::Enum FileShareMode)
  {
    FolderWriter* pWriter = nullptr;

    {
      NS_LOCK(m_ReaderWriterMutex);
      for (nsUInt32 i = 0; i < m_Writers.GetCount(); ++i)
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
    if (pWriter->Open(sFile, this, FileShareMode) == NS_FAILURE)
    {
      NS_LOCK(m_ReaderWriterMutex);
      pWriter->m_bIsInUse = false;
      return nullptr;
    }

    // if it succeeds, we return the reader
    return pWriter;
  }
} // namespace nsDataDirectory



NS_STATICLINK_FILE(Foundation, Foundation_IO_FileSystem_Implementation_DataDirTypeFolder);

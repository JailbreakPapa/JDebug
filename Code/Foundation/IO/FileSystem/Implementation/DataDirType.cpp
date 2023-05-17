#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/OSFile.h>

wdResult wdDataDirectoryType::InitializeDataDirectory(wdStringView sDataDirPath)
{
  wdStringBuilder sPath = sDataDirPath;
  sPath.MakeCleanPath();

  WD_ASSERT_DEV(sPath.IsEmpty() || sPath.EndsWith("/"), "Data directory path must end with a slash.");

  m_sDataDirectoryPath = sPath;

  return InternalInitializeDataDirectory(m_sDataDirectoryPath.GetData());
}

bool wdDataDirectoryType::ExistsFile(wdStringView sFile, bool bOneSpecificDataDir)
{
  wdStringBuilder sRedirectedAsset;
  ResolveAssetRedirection(sFile, sRedirectedAsset);

  wdStringBuilder sPath = GetRedirectedDataDirectoryPath();
  sPath.AppendPath(sRedirectedAsset);
  return wdOSFile::ExistsFile(sPath);
}

void wdDataDirectoryReaderWriterBase::Close()
{
  InternalClose();

  wdFileSystem::FileEvent fe;
  fe.m_EventType = wdFileSystem::FileEventType::CloseFile;
  fe.m_sFileOrDirectory = GetFilePath();
  fe.m_pDataDir = m_pDataDirectory;
  wdFileSystem::s_pData->m_Event.Broadcast(fe);

  m_pDataDirectory->OnReaderWriterClose(this);
}



WD_STATICLINK_FILE(Foundation, Foundation_IO_FileSystem_Implementation_DataDirType);

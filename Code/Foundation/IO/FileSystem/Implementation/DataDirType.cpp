#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/IO/OSFile.h>

nsResult nsDataDirectoryType::InitializeDataDirectory(nsStringView sDataDirPath)
{
  nsStringBuilder sPath = sDataDirPath;
  sPath.MakeCleanPath();

  NS_ASSERT_DEV(sPath.IsEmpty() || sPath.EndsWith("/"), "Data directory path must end with a slash.");

  m_sDataDirectoryPath = sPath;

  return InternalInitializeDataDirectory(m_sDataDirectoryPath.GetData());
}

bool nsDataDirectoryType::ExistsFile(nsStringView sFile, bool bOneSpecificDataDir)
{
  nsStringBuilder sRedirectedAsset;
  ResolveAssetRedirection(sFile, sRedirectedAsset);

  nsStringBuilder sPath = GetRedirectedDataDirectoryPath();
  sPath.AppendPath(sRedirectedAsset);
  return nsOSFile::ExistsFile(sPath);
}

void nsDataDirectoryReaderWriterBase::Close()
{
  InternalClose();

  nsFileSystem::FileEvent fe;
  fe.m_EventType = nsFileSystem::FileEventType::CloseFile;
  fe.m_sFileOrDirectory = GetFilePath();
  fe.m_pDataDir = m_pDataDirectory;
  nsFileSystem::s_pData->m_Event.Broadcast(fe);

  m_pDataDirectory->OnReaderWriterClose(this);
}

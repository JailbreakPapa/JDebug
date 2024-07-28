#include <Core/CorePCH.h>

#include <Core/ResourceManager/Resource.h>
#include <Core/ResourceManager/ResourceTypeLoader.h>
#include <Foundation/Containers/Blob.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Profiling/Profiling.h>

struct FileResourceLoadData
{
  nsBlob m_Storage;
  nsRawMemoryStreamReader m_Reader;
};

nsResourceLoadData nsResourceLoaderFromFile::OpenDataStream(const nsResource* pResource)
{
  NS_PROFILE_SCOPE("ReadResourceFile");

  nsResourceLoadData res;

  nsFileReader File;
  if (File.Open(pResource->GetResourceID().GetData()).Failed())
    return res;

  res.m_sResourceDescription = File.GetFilePathRelative().GetData();

#if NS_ENABLED(NS_SUPPORTS_FILE_STATS)
  nsFileStats stat;
  if (nsFileSystem::GetFileStats(pResource->GetResourceID(), stat).Succeeded())
  {
    res.m_LoadedFileModificationDate = stat.m_LastModificationTime;
  }

#endif

  FileResourceLoadData* pData = NS_DEFAULT_NEW(FileResourceLoadData);

  const nsUInt64 uiFileSize = File.GetFileSize();

  const nsUInt64 uiBlobCapacity = uiFileSize + File.GetFilePathAbsolute().GetElementCount() + 8; // +8 for the string overhead
  pData->m_Storage.SetCountUninitialized(uiBlobCapacity);

  nsUInt8* pBlobPtr = pData->m_Storage.GetBlobPtr<nsUInt8>().GetPtr();

  nsRawMemoryStreamWriter w(pBlobPtr, uiBlobCapacity);

  // write the absolute path to the read file into the memory stream
  w << File.GetFilePathAbsolute();

  const nsUInt64 uiOffset = w.GetNumWrittenBytes();

  File.ReadBytes(pBlobPtr + uiOffset, uiFileSize);

  pData->m_Reader.Reset(pBlobPtr, w.GetNumWrittenBytes() + uiFileSize);
  res.m_pDataStream = &pData->m_Reader;
  res.m_pCustomLoaderData = pData;

  return res;
}

void nsResourceLoaderFromFile::CloseDataStream(const nsResource* pResource, const nsResourceLoadData& loaderData)
{
  FileResourceLoadData* pData = static_cast<FileResourceLoadData*>(loaderData.m_pCustomLoaderData);

  NS_DEFAULT_DELETE(pData);
}

bool nsResourceLoaderFromFile::IsResourceOutdated(const nsResource* pResource) const
{
  // if we cannot find the target file, there is no point in trying to reload it -> claim it's up to date
  if (nsFileSystem::ResolvePath(pResource->GetResourceID(), nullptr, nullptr).Failed())
    return false;

#if NS_ENABLED(NS_SUPPORTS_FILE_STATS)

  if (pResource->GetLoadedFileModificationTime().IsValid())
  {
    nsFileStats stat;
    if (nsFileSystem::GetFileStats(pResource->GetResourceID(), stat).Failed())
      return false;

    return !stat.m_LastModificationTime.Compare(pResource->GetLoadedFileModificationTime(), nsTimestamp::CompareMode::FileTimeEqual);
  }

#endif

  return true;
}

//////////////////////////////////////////////////////////////////////////

nsResourceLoadData nsResourceLoaderFromMemory::OpenDataStream(const nsResource* pResource)
{
  m_Reader.SetStorage(&m_CustomData);
  m_Reader.SetReadPosition(0);

  nsResourceLoadData res;

  res.m_sResourceDescription = m_sResourceDescription;
  res.m_LoadedFileModificationDate = m_ModificationTimestamp;
  res.m_pDataStream = &m_Reader;
  res.m_pCustomLoaderData = nullptr;

  return res;
}

void nsResourceLoaderFromMemory::CloseDataStream(const nsResource* pResource, const nsResourceLoadData& loaderData)
{
  m_Reader.SetStorage(nullptr);
}

bool nsResourceLoaderFromMemory::IsResourceOutdated(const nsResource* pResource) const
{
  if (pResource->GetLoadedFileModificationTime().IsValid() && m_ModificationTimestamp.IsValid())
  {
    if (!m_ModificationTimestamp.Compare(pResource->GetLoadedFileModificationTime(), nsTimestamp::CompareMode::FileTimeEqual))
      return true;

    return false;
  }

  return true;
}

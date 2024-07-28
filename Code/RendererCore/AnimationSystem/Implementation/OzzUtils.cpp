#include <RendererCore/RendererCorePCH.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <RendererCore/AnimationSystem/Implementation/OzzUtils.h>
#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/base/io/archive.h>

nsOzzArchiveData::nsOzzArchiveData() = default;
nsOzzArchiveData::~nsOzzArchiveData() = default;

nsResult nsOzzArchiveData::FetchRegularFile(const char* szFile)
{
  nsFileReader file;
  NS_SUCCEED_OR_RETURN(file.Open(szFile));

  m_Storage.Clear();
  m_Storage.Reserve(file.GetFileSize());
  m_Storage.ReadAll(file);

  return NS_SUCCESS;
}

nsResult nsOzzArchiveData::FetchEmbeddedArchive(nsStreamReader& inout_stream)
{
  char szTag[8] = "";

  inout_stream.ReadBytes(szTag, 8);
  szTag[7] = '\0';

  if (!nsStringUtils::IsEqual(szTag, "nsOzzAr"))
    return NS_FAILURE;

  /*const nsTypeVersion version =*/inout_stream.ReadVersion(1);

  nsUInt64 uiArchiveSize = 0;
  inout_stream >> uiArchiveSize;

  m_Storage.Clear();
  m_Storage.Reserve(uiArchiveSize);
  m_Storage.ReadAll(inout_stream, uiArchiveSize);

  if (m_Storage.GetStorageSize64() != uiArchiveSize)
    return NS_FAILURE;

  return NS_SUCCESS;
}

nsResult nsOzzArchiveData::StoreEmbeddedArchive(nsStreamWriter& inout_stream) const
{
  const char szTag[8] = "nsOzzAr";

  NS_SUCCEED_OR_RETURN(inout_stream.WriteBytes(szTag, 8));

  inout_stream.WriteVersion(1);

  const nsUInt64 uiArchiveSize = m_Storage.GetStorageSize64();

  inout_stream << uiArchiveSize;

  return m_Storage.CopyToStream(inout_stream);
}

nsOzzStreamReader::nsOzzStreamReader(const nsOzzArchiveData& data)
  : m_Reader(&data.m_Storage)
{
}

bool nsOzzStreamReader::opened() const
{
  return true;
}

size_t nsOzzStreamReader::Read(void* pBuffer, size_t uiSize)
{
  return static_cast<size_t>(m_Reader.ReadBytes(pBuffer, uiSize));
}

size_t nsOzzStreamReader::Write(const void* pBuffer, size_t uiSize)
{
  NS_ASSERT_NOT_IMPLEMENTED;
  return 0;
}

int nsOzzStreamReader::Seek(int iOffset, Origin origin)
{
  switch (origin)
  {
    case ozz::io::Stream::kCurrent:
      m_Reader.SetReadPosition(m_Reader.GetReadPosition() + iOffset);
      break;
    case ozz::io::Stream::kEnd:
      m_Reader.SetReadPosition(m_Reader.GetByteCount64() - iOffset);
      break;
    case ozz::io::Stream::kSet:
      m_Reader.SetReadPosition(iOffset);
      break;

      NS_DEFAULT_CASE_NOT_IMPLEMENTED;
  }

  return 0;
}

int nsOzzStreamReader::Tell() const
{
  return static_cast<int>(m_Reader.GetReadPosition());
}

size_t nsOzzStreamReader::Size() const
{
  return static_cast<size_t>(m_Reader.GetByteCount64());
}

nsOzzStreamWriter::nsOzzStreamWriter(nsOzzArchiveData& ref_data)
  : m_Writer(&ref_data.m_Storage)
{
}

bool nsOzzStreamWriter::opened() const
{
  return true;
}

size_t nsOzzStreamWriter::Read(void* pBuffer, size_t uiSize)
{
  NS_ASSERT_NOT_IMPLEMENTED;
  return 0;
}

size_t nsOzzStreamWriter::Write(const void* pBuffer, size_t uiSize)
{
  if (m_Writer.WriteBytes(pBuffer, uiSize).Failed())
    return 0;

  return uiSize;
}

int nsOzzStreamWriter::Seek(int iOffset, Origin origin)
{
  switch (origin)
  {
    case ozz::io::Stream::kCurrent:
      m_Writer.SetWritePosition(m_Writer.GetWritePosition() + iOffset);
      break;
    case ozz::io::Stream::kEnd:
      m_Writer.SetWritePosition(m_Writer.GetByteCount64() - iOffset);
      break;
    case ozz::io::Stream::kSet:
      m_Writer.SetWritePosition(iOffset);
      break;

      NS_DEFAULT_CASE_NOT_IMPLEMENTED;
  }

  return 0;
}

int nsOzzStreamWriter::Tell() const
{
  return static_cast<int>(m_Writer.GetWritePosition());
}

size_t nsOzzStreamWriter::Size() const
{
  return static_cast<size_t>(m_Writer.GetByteCount64());
}

void nsOzzUtils::CopyAnimation(ozz::animation::Animation* pDst, const ozz::animation::Animation* pSrc)
{
  nsOzzArchiveData ozzArchiveData;

  // store in ozz archive
  {
    nsOzzStreamWriter ozzWriter(ozzArchiveData);
    ozz::io::OArchive ozzArchive(&ozzWriter);

    ozzArchive << *pSrc;
  }

  // read it from archive again
  {
    nsOzzStreamReader ozzReader(ozzArchiveData);
    ozz::io::IArchive ozzArchive(&ozzReader);

    ozzArchive >> *pDst;
  }
}

NS_RENDERERCORE_DLL void nsOzzUtils::CopySkeleton(ozz::animation::Skeleton* pDst, const ozz::animation::Skeleton* pSrc)
{
  nsOzzArchiveData ozzArchiveData;

  // store in ozz archive
  {
    nsOzzStreamWriter ozzWriter(ozzArchiveData);
    ozz::io::OArchive ozzArchive(&ozzWriter);

    ozzArchive << *pSrc;
  }

  // read it from archive again
  {
    nsOzzStreamReader ozzReader(ozzArchiveData);
    ozz::io::IArchive ozzArchive(&ozzReader);

    ozzArchive >> *pDst;
  }
}

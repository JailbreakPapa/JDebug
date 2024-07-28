#pragma once

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/IO/Stream.h>
#include <RendererCore/RendererCoreDLL.h>
#include <ozz/base/io/stream.h>

namespace ozz::animation
{
  class Skeleton;
  class Animation;
}; // namespace ozz::animation

/// \brief Stores or gather the data for an ozz file, for random access operations (seek / tell).
///
/// Since ozz::io::Stream requires seek/tell functionality, it cannot be implemented with basic nsStreamReader / nsStreamWriter.
/// Instead, we must have the entire ozz archive data in memory, to be able to jump around arbitrarily.
class NS_RENDERERCORE_DLL nsOzzArchiveData
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsOzzArchiveData);

public:
  nsOzzArchiveData();
  ~nsOzzArchiveData();

  nsResult FetchRegularFile(const char* szFile);
  nsResult FetchEmbeddedArchive(nsStreamReader& inout_stream);
  nsResult StoreEmbeddedArchive(nsStreamWriter& inout_stream) const;

  nsDefaultMemoryStreamStorage m_Storage;
};

/// \brief Implements the ozz::io::Stream interface for reading. The data has to be present in an nsOzzArchiveData object.
///
/// The class is implemented inline and not DLL exported because ozz is only available as a static library.
class NS_RENDERERCORE_DLL nsOzzStreamReader : public ozz::io::Stream
{
public:
  nsOzzStreamReader(const nsOzzArchiveData& data);

  virtual bool opened() const override;

  virtual size_t Read(void* pBuffer, size_t uiSize) override;

  virtual size_t Write(const void* pBuffer, size_t uiSize) override;

  virtual int Seek(int iOffset, Origin origin) override;

  virtual int Tell() const override;

  virtual size_t Size() const override;

private:
  nsMemoryStreamReader m_Reader;
};

/// \brief Implements the ozz::io::Stream interface for writing. The data is gathered in an nsOzzArchiveData object.
///
/// The class is implemented inline and not DLL exported because ozz is only available as a static library.
class NS_RENDERERCORE_DLL nsOzzStreamWriter : public ozz::io::Stream
{
public:
  nsOzzStreamWriter(nsOzzArchiveData& ref_data);

  virtual bool opened() const override;

  virtual size_t Read(void* pBuffer, size_t uiSize) override;

  virtual size_t Write(const void* pBuffer, size_t uiSize) override;

  virtual int Seek(int iOffset, Origin origin) override;

  virtual int Tell() const override;

  virtual size_t Size() const override;

private:
  nsMemoryStreamWriter m_Writer;
};

namespace nsOzzUtils
{
  NS_RENDERERCORE_DLL void CopyAnimation(ozz::animation::Animation* pDst, const ozz::animation::Animation* pSrc);
  NS_RENDERERCORE_DLL void CopySkeleton(ozz::animation::Skeleton* pDst, const ozz::animation::Skeleton* pSrc);
} // namespace nsOzzUtils

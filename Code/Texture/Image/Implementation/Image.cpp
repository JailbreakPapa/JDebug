#include <Texture/TexturePCH.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/Profiling/Profiling.h>
#include <Texture/Image/Formats/ImageFileFormat.h>
#include <Texture/Image/Image.h>
#include <Texture/Image/ImageConversion.h>

nsImageView::nsImageView()
{
  Clear();
}

nsImageView::nsImageView(const nsImageHeader& header, nsConstByteBlobPtr imageData)
{
  ResetAndViewExternalStorage(header, imageData);
}

void nsImageView::Clear()
{
  nsImageHeader::Clear();
  m_SubImageOffsets.Clear();
  m_DataPtr.Clear();
}

bool nsImageView::IsValid() const
{
  return !m_DataPtr.IsEmpty();
}

void nsImageView::ResetAndViewExternalStorage(const nsImageHeader& header, nsConstByteBlobPtr imageData)
{
  static_cast<nsImageHeader&>(*this) = header;

  nsUInt64 dataSize = ComputeLayout();

  NS_IGNORE_UNUSED(dataSize);
  NS_ASSERT_DEV(imageData.GetCount() == dataSize, "Provided image storage ({} bytes) doesn't match required data size ({} bytes)",
    imageData.GetCount(), dataSize);

  // Const cast is safe here as we will only perform non-const access if this is an nsImage which owns mutable access to the storage
  m_DataPtr = nsBlobPtr<nsUInt8>(const_cast<nsUInt8*>(static_cast<const nsUInt8*>(imageData.GetPtr())), imageData.GetCount());
}

nsResult nsImageView::SaveTo(nsStringView sFileName) const
{
  NS_LOG_BLOCK("Writing Image", sFileName);

  if (m_Format == nsImageFormat::UNKNOWN)
  {
    nsLog::Error("Cannot write image '{0}' - image data is invalid or empty", sFileName);
    return NS_FAILURE;
  }

  nsFileWriter writer;
  if (writer.Open(sFileName) == NS_FAILURE)
  {
    nsLog::Error("Failed to open image file '{0}'", sFileName);
    return NS_FAILURE;
  }

  nsStringView it = nsPathUtils::GetFileExtension(sFileName);

  if (nsImageFileFormat* pFormat = nsImageFileFormat::GetWriterFormat(it.GetStartPointer()))
  {
    if (pFormat->WriteImage(writer, *this, it.GetStartPointer()) != NS_SUCCESS)
    {
      nsLog::Error("Failed to write image file '{0}'", sFileName);
      return NS_FAILURE;
    }

    return NS_SUCCESS;
  }

  nsLog::Error("No known image file format for extension '{0}'", it);
  return NS_FAILURE;
}

const nsImageHeader& nsImageView::GetHeader() const
{
  return *this;
}

nsImageView nsImageView::GetRowView(
  nsUInt32 uiMipLevel /*= 0*/, nsUInt32 uiFace /*= 0*/, nsUInt32 uiArrayIndex /*= 0*/, nsUInt32 y /*= 0*/, nsUInt32 z /*= 0*/, nsUInt32 uiPlaneIndex /*= 0*/) const
{
  nsImageHeader header;
  header.SetNumMipLevels(1);
  header.SetNumFaces(1);
  header.SetNumArrayIndices(1);

  // Scale dimensions relative to the block size of the subformat
  nsImageFormat::Enum subFormat = nsImageFormat::GetPlaneSubFormat(m_Format, uiPlaneIndex);
  header.SetWidth(GetWidth(uiMipLevel) * nsImageFormat::GetBlockWidth(subFormat) / nsImageFormat::GetBlockWidth(m_Format, uiPlaneIndex));
  header.SetHeight(nsImageFormat::GetBlockHeight(m_Format, 0) * nsImageFormat::GetBlockHeight(subFormat) / nsImageFormat::GetBlockHeight(m_Format, uiPlaneIndex));
  header.SetDepth(nsImageFormat::GetBlockDepth(subFormat) / nsImageFormat::GetBlockDepth(m_Format, uiPlaneIndex));
  header.SetImageFormat(nsImageFormat::GetPlaneSubFormat(m_Format, uiPlaneIndex));

  nsUInt64 offset = 0;

  offset += GetSubImageOffset(uiMipLevel, uiFace, uiArrayIndex, uiPlaneIndex);
  offset += z * GetDepthPitch(uiMipLevel, uiPlaneIndex);
  offset += y * GetRowPitch(uiMipLevel, uiPlaneIndex);

  nsBlobPtr<const nsUInt8> dataSlice = m_DataPtr.GetSubArray(offset, GetRowPitch(uiMipLevel, uiPlaneIndex));
  return nsImageView(header, nsConstByteBlobPtr(dataSlice.GetPtr(), dataSlice.GetCount()));
}

void nsImageView::ReinterpretAs(nsImageFormat::Enum format)
{
  NS_ASSERT_DEBUG(
    nsImageFormat::IsCompressed(format) == nsImageFormat::IsCompressed(GetImageFormat()), "Cannot reinterpret compressed and non-compressed formats");

  NS_ASSERT_DEBUG(nsImageFormat::GetBitsPerPixel(GetImageFormat()) == nsImageFormat::GetBitsPerPixel(format),
    "Cannot reinterpret between formats of different sizes");

  SetImageFormat(format);
}

nsUInt64 nsImageView::ComputeLayout()
{
  m_SubImageOffsets.Clear();
  m_SubImageOffsets.Reserve(m_uiNumMipLevels * m_uiNumFaces * m_uiNumArrayIndices * GetPlaneCount());

  nsUInt64 uiDataSize = 0;

  for (nsUInt32 uiArrayIndex = 0; uiArrayIndex < m_uiNumArrayIndices; uiArrayIndex++)
  {
    for (nsUInt32 uiFace = 0; uiFace < m_uiNumFaces; uiFace++)
    {
      for (nsUInt32 uiMipLevel = 0; uiMipLevel < m_uiNumMipLevels; uiMipLevel++)
      {
        for (nsUInt32 uiPlaneIndex = 0; uiPlaneIndex < GetPlaneCount(); uiPlaneIndex++)
        {
          m_SubImageOffsets.PushBack(uiDataSize);

          uiDataSize += GetDepthPitch(uiMipLevel, uiPlaneIndex) * GetDepth(uiMipLevel);
        }
      }
    }
  }

  // Push back total size as a marker
  m_SubImageOffsets.PushBack(uiDataSize);

  return uiDataSize;
}

void nsImageView::ValidateSubImageIndices(nsUInt32 uiMipLevel, nsUInt32 uiFace, nsUInt32 uiArrayIndex, nsUInt32 uiPlaneIndex) const
{
  NS_ASSERT_DEV(uiMipLevel < m_uiNumMipLevels, "Invalid mip level");
  NS_ASSERT_DEV(uiFace < m_uiNumFaces, "Invalid uiFace");
  NS_ASSERT_DEV(uiArrayIndex < m_uiNumArrayIndices, "Invalid array slice");
  NS_ASSERT_DEV(uiPlaneIndex < GetPlaneCount(), "Invalid plane index");
}

const nsUInt64& nsImageView::GetSubImageOffset(nsUInt32 uiMipLevel, nsUInt32 uiFace, nsUInt32 uiArrayIndex, nsUInt32 uiPlaneIndex) const
{
  ValidateSubImageIndices(uiMipLevel, uiFace, uiArrayIndex, uiPlaneIndex);
  return m_SubImageOffsets[uiPlaneIndex + GetPlaneCount() * (uiMipLevel + m_uiNumMipLevels * (uiFace + m_uiNumFaces * uiArrayIndex))];
}

nsImage::nsImage()
{
  Clear();
}

nsImage::nsImage(const nsImageHeader& header)
{
  ResetAndAlloc(header);
}

nsImage::nsImage(const nsImageHeader& header, nsByteBlobPtr externalData)
{
  ResetAndUseExternalStorage(header, externalData);
}

nsImage::nsImage(nsImage&& other)
{
  ResetAndMove(std::move(other));
}

nsImage::nsImage(const nsImageView& other)
{
  ResetAndCopy(other);
}

void nsImage::operator=(nsImage&& rhs)
{
  ResetAndMove(std::move(rhs));
}

void nsImage::Clear()
{
  m_InternalStorage.Clear();

  nsImageView::Clear();
}

void nsImage::ResetAndAlloc(const nsImageHeader& header)
{
  const nsUInt64 requiredSize = header.ComputeDataSize();

  // it is debatable whether this function should reuse external storage, at all
  // however, it is especially dangerous to rely on the external storage being big enough, since many functions just take an nsImage as a
  // destination parameter and expect it to behave correctly when any of the Reset functions is called on it; it is not intuitive, that
  // Reset may fail due to how the image was previously reset

  // therefore, if external storage is insufficient, fall back to internal storage

  if (!UsesExternalStorage() || m_DataPtr.GetCount() < requiredSize)
  {
    m_InternalStorage.SetCountUninitialized(requiredSize);
    m_DataPtr = m_InternalStorage.GetBlobPtr<nsUInt8>();
  }

  nsImageView::ResetAndViewExternalStorage(header, nsConstByteBlobPtr(m_DataPtr.GetPtr(), m_DataPtr.GetCount()));
}

void nsImage::ResetAndUseExternalStorage(const nsImageHeader& header, nsByteBlobPtr externalData)
{
  m_InternalStorage.Clear();

  nsImageView::ResetAndViewExternalStorage(header, externalData);
}

void nsImage::ResetAndMove(nsImage&& other)
{
  static_cast<nsImageHeader&>(*this) = other.GetHeader();

  if (other.UsesExternalStorage())
  {
    m_InternalStorage.Clear();
    m_SubImageOffsets = std::move(other.m_SubImageOffsets);
    m_DataPtr = other.m_DataPtr;
    other.Clear();
  }
  else
  {
    m_InternalStorage = std::move(other.m_InternalStorage);
    m_SubImageOffsets = std::move(other.m_SubImageOffsets);
    m_DataPtr = m_InternalStorage.GetBlobPtr<nsUInt8>();
    other.Clear();
  }
}

void nsImage::ResetAndCopy(const nsImageView& other)
{
  ResetAndAlloc(other.GetHeader());

  memcpy(GetBlobPtr<nsUInt8>().GetPtr(), other.GetBlobPtr<nsUInt8>().GetPtr(), static_cast<size_t>(other.GetBlobPtr<nsUInt8>().GetCount()));
}

nsResult nsImage::LoadFrom(nsStringView sFileName)
{
  NS_LOG_BLOCK("Loading Image", sFileName);

  NS_PROFILE_SCOPE(nsPathUtils::GetFileNameAndExtension(sFileName).GetStartPointer());

  nsFileReader reader;
  if (reader.Open(sFileName) == NS_FAILURE)
  {
    nsLog::Warning("Failed to open image file '{0}'", nsArgSensitive(sFileName, "File"));
    return NS_FAILURE;
  }

  nsStringView it = nsPathUtils::GetFileExtension(sFileName);

  if (nsImageFileFormat* pFormat = nsImageFileFormat::GetReaderFormat(it.GetStartPointer()))
  {
    if (pFormat->ReadImage(reader, *this, it.GetStartPointer()) != NS_SUCCESS)
    {
      nsLog::Warning("Failed to read image file '{0}'", nsArgSensitive(sFileName, "File"));
      return NS_FAILURE;
    }

    return NS_SUCCESS;
  }

  nsLog::Warning("No known image file format for extension '{0}'", it);

  return NS_FAILURE;
}

nsResult nsImage::Convert(nsImageFormat::Enum targetFormat)
{
  return nsImageConversion::Convert(*this, *this, targetFormat);
}

nsImageView nsImageView::GetSubImageView(nsUInt32 uiMipLevel /*= 0*/, nsUInt32 uiFace /*= 0*/, nsUInt32 uiArrayIndex /*= 0*/) const
{
  nsImageHeader header;
  header.SetNumMipLevels(1);
  header.SetNumFaces(1);
  header.SetNumArrayIndices(1);
  header.SetWidth(GetWidth(uiMipLevel));
  header.SetHeight(GetHeight(uiMipLevel));
  header.SetDepth(GetDepth(uiMipLevel));
  header.SetImageFormat(m_Format);

  const nsUInt64& offset = GetSubImageOffset(uiMipLevel, uiFace, uiArrayIndex, 0);
  nsUInt64 size = *(&offset + GetPlaneCount()) - offset;

  nsBlobPtr<const nsUInt8> subView = m_DataPtr.GetSubArray(offset, size);

  return nsImageView(header, nsConstByteBlobPtr(subView.GetPtr(), subView.GetCount()));
}

nsImage nsImage::GetSubImageView(nsUInt32 uiMipLevel /*= 0*/, nsUInt32 uiFace /*= 0*/, nsUInt32 uiArrayIndex /*= 0*/)
{
  nsImageView constView = nsImageView::GetSubImageView(uiMipLevel, uiFace, uiArrayIndex);

  // Create an nsImage attached to the view. Const cast is safe here since we own the storage.
  return nsImage(
    constView.GetHeader(), nsByteBlobPtr(const_cast<nsUInt8*>(constView.GetBlobPtr<nsUInt8>().GetPtr()), constView.GetBlobPtr<nsUInt8>().GetCount()));
}

nsImageView nsImageView::GetPlaneView(nsUInt32 uiMipLevel /*= 0*/, nsUInt32 uiFace /*= 0*/, nsUInt32 uiArrayIndex /*= 0*/, nsUInt32 uiPlaneIndex /*= 0*/) const
{
  nsImageHeader header;
  header.SetNumMipLevels(1);
  header.SetNumFaces(1);
  header.SetNumArrayIndices(1);

  // Scale dimensions relative to the block size of the first plane which determines the "nominal" width, height and depth
  nsImageFormat::Enum subFormat = nsImageFormat::GetPlaneSubFormat(m_Format, uiPlaneIndex);
  header.SetWidth(GetWidth(uiMipLevel) * nsImageFormat::GetBlockWidth(subFormat) / nsImageFormat::GetBlockWidth(m_Format, uiPlaneIndex));
  header.SetHeight(GetHeight(uiMipLevel) * nsImageFormat::GetBlockHeight(subFormat) / nsImageFormat::GetBlockHeight(m_Format, uiPlaneIndex));
  header.SetDepth(GetDepth(uiMipLevel) * nsImageFormat::GetBlockDepth(subFormat) / nsImageFormat::GetBlockDepth(m_Format, uiPlaneIndex));
  header.SetImageFormat(subFormat);

  const nsUInt64& offset = GetSubImageOffset(uiMipLevel, uiFace, uiArrayIndex, uiPlaneIndex);
  nsUInt64 size = *(&offset + 1) - offset;

  nsBlobPtr<const nsUInt8> subView = m_DataPtr.GetSubArray(offset, size);

  return nsImageView(header, nsConstByteBlobPtr(subView.GetPtr(), subView.GetCount()));
}

nsImage nsImage::GetPlaneView(nsUInt32 uiMipLevel /* = 0 */, nsUInt32 uiFace /* = 0 */, nsUInt32 uiArrayIndex /* = 0 */, nsUInt32 uiPlaneIndex /* = 0 */)
{
  nsImageView constView = nsImageView::GetPlaneView(uiMipLevel, uiFace, uiArrayIndex, uiPlaneIndex);

  // Create an nsImage attached to the view. Const cast is safe here since we own the storage.
  return nsImage(
    constView.GetHeader(), nsByteBlobPtr(const_cast<nsUInt8*>(constView.GetBlobPtr<nsUInt8>().GetPtr()), constView.GetBlobPtr<nsUInt8>().GetCount()));
}

nsImage nsImage::GetSliceView(nsUInt32 uiMipLevel /*= 0*/, nsUInt32 uiFace /*= 0*/, nsUInt32 uiArrayIndex /*= 0*/, nsUInt32 z /*= 0*/, nsUInt32 uiPlaneIndex /*= 0*/)
{
  nsImageView constView = nsImageView::GetSliceView(uiMipLevel, uiFace, uiArrayIndex, z, uiPlaneIndex);

  // Create an nsImage attached to the view. Const cast is safe here since we own the storage.
  return nsImage(
    constView.GetHeader(), nsByteBlobPtr(const_cast<nsUInt8*>(constView.GetBlobPtr<nsUInt8>().GetPtr()), constView.GetBlobPtr<nsUInt8>().GetCount()));
}

nsImageView nsImageView::GetSliceView(nsUInt32 uiMipLevel /*= 0*/, nsUInt32 uiFace /*= 0*/, nsUInt32 uiArrayIndex /*= 0*/, nsUInt32 z /*= 0*/, nsUInt32 uiPlaneIndex /*= 0*/) const
{
  nsImageHeader header;
  header.SetNumMipLevels(1);
  header.SetNumFaces(1);
  header.SetNumArrayIndices(1);

  // Scale dimensions relative to the block size of the first plane which determines the "nominal" width, height and depth
  nsImageFormat::Enum subFormat = nsImageFormat::GetPlaneSubFormat(m_Format, uiPlaneIndex);
  header.SetWidth(GetWidth(uiMipLevel) * nsImageFormat::GetBlockWidth(subFormat) / nsImageFormat::GetBlockWidth(m_Format, uiPlaneIndex));
  header.SetHeight(GetHeight(uiMipLevel) * nsImageFormat::GetBlockHeight(subFormat) / nsImageFormat::GetBlockHeight(m_Format, uiPlaneIndex));
  header.SetDepth(nsImageFormat::GetBlockDepth(subFormat) / nsImageFormat::GetBlockDepth(m_Format, uiPlaneIndex));
  header.SetImageFormat(subFormat);

  nsUInt64 offset = GetSubImageOffset(uiMipLevel, uiFace, uiArrayIndex, uiPlaneIndex) + z * GetDepthPitch(uiMipLevel, uiPlaneIndex);
  nsUInt64 size = GetDepthPitch(uiMipLevel, uiPlaneIndex);

  nsBlobPtr<const nsUInt8> subView = m_DataPtr.GetSubArray(offset, size);

  return nsImageView(header, nsConstByteBlobPtr(subView.GetPtr(), subView.GetCount()));
}

bool nsImage::UsesExternalStorage() const
{
  return m_InternalStorage.GetBlobPtr<nsUInt8>() != m_DataPtr;
}

#pragma once

template <typename T>
struct nsImageSizeofHelper
{
  static constexpr size_t Size = sizeof(T);
};

template <>
struct nsImageSizeofHelper<void>
{
  static constexpr size_t Size = 1;
};

template <>
struct nsImageSizeofHelper<const void>
{
  static constexpr size_t Size = 1;
};

template <typename T>
nsBlobPtr<const T> nsImageView::GetBlobPtr() const
{
  for (nsUInt32 uiPlaneIndex = 0; uiPlaneIndex < GetPlaneCount(); ++uiPlaneIndex)
  {
    ValidateDataTypeAccessor<T>(uiPlaneIndex);
  }
  return nsBlobPtr<const T>(reinterpret_cast<T*>(static_cast<nsUInt8*>(m_DataPtr.GetPtr())), m_DataPtr.GetCount() / nsImageSizeofHelper<T>::Size);
}

inline nsConstByteBlobPtr nsImageView::GetByteBlobPtr() const
{
  for (nsUInt32 uiPlaneIndex = 0; uiPlaneIndex < GetPlaneCount(); ++uiPlaneIndex)
  {
    ValidateDataTypeAccessor<nsUInt8>(uiPlaneIndex);
  }
  return nsConstByteBlobPtr(static_cast<nsUInt8*>(m_DataPtr.GetPtr()), m_DataPtr.GetCount());
}

template <typename T>
nsBlobPtr<T> nsImage::GetBlobPtr()
{
  nsBlobPtr<const T> constPtr = nsImageView::GetBlobPtr<T>();

  return nsBlobPtr<T>(const_cast<T*>(static_cast<const T*>(constPtr.GetPtr())), constPtr.GetCount());
}

inline nsByteBlobPtr nsImage::GetByteBlobPtr()
{
  nsConstByteBlobPtr constPtr = nsImageView::GetByteBlobPtr();

  return nsByteBlobPtr(const_cast<nsUInt8*>(constPtr.GetPtr()), constPtr.GetCount());
}

template <typename T>
const T* nsImageView::GetPixelPointer(nsUInt32 uiMipLevel /*= 0*/, nsUInt32 uiFace /*= 0*/, nsUInt32 uiArrayIndex /*= 0*/, nsUInt32 x /*= 0*/,
  nsUInt32 y /*= 0*/, nsUInt32 z /*= 0*/, nsUInt32 uiPlaneIndex /*= 0*/) const
{
  ValidateDataTypeAccessor<T>(uiPlaneIndex);
  NS_ASSERT_DEV(x < GetNumBlocksX(uiMipLevel, uiPlaneIndex), "Invalid x coordinate");
  NS_ASSERT_DEV(y < GetNumBlocksY(uiMipLevel, uiPlaneIndex), "Invalid y coordinate");
  NS_ASSERT_DEV(z < GetNumBlocksZ(uiMipLevel, uiPlaneIndex), "Invalid z coordinate");

  nsUInt64 offset = GetSubImageOffset(uiMipLevel, uiFace, uiArrayIndex, uiPlaneIndex) +
                    z * GetDepthPitch(uiMipLevel, uiPlaneIndex) +
                    y * GetRowPitch(uiMipLevel, uiPlaneIndex) +
                    x * nsImageFormat::GetBitsPerBlock(m_Format, uiPlaneIndex) / 8;
  return reinterpret_cast<const T*>(&m_DataPtr[offset]);
}

template <typename T>
T* nsImage::GetPixelPointer(
  nsUInt32 uiMipLevel /*= 0*/, nsUInt32 uiFace /*= 0*/, nsUInt32 uiArrayIndex /*= 0*/, nsUInt32 x /*= 0*/, nsUInt32 y /*= 0*/, nsUInt32 z /*= 0*/, nsUInt32 uiPlaneIndex /*= 0*/)
{
  return const_cast<T*>(nsImageView::GetPixelPointer<T>(uiMipLevel, uiFace, uiArrayIndex, x, y, z, uiPlaneIndex));
}


template <typename T>
void nsImageView::ValidateDataTypeAccessor(nsUInt32 uiPlaneIndex) const
{
  nsUInt32 bytesPerBlock = nsImageFormat::GetBitsPerBlock(GetImageFormat(), uiPlaneIndex) / 8;
  NS_IGNORE_UNUSED(bytesPerBlock);
  NS_ASSERT_DEV(bytesPerBlock % nsImageSizeofHelper<T>::Size == 0, "Accessor type is not suitable for interpreting contained data");
}

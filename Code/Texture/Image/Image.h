#pragma once

#include <Foundation/Containers/Blob.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Logging/Log.h>

#include <Texture/Image/Formats/ImageFileFormat.h>
#include <Texture/Image/ImageHeader.h>

/// \brief A class referencing image data and holding metadata about the image.
class NS_TEXTURE_DLL nsImageView : protected nsImageHeader
{
public:
  /// \brief Constructs an empty image view.
  nsImageView();

  /// \brief Constructs an image view with the given header and image data.
  nsImageView(const nsImageHeader& header, nsConstByteBlobPtr imageData);

  /// \brief Constructs an empty image view.
  void Clear();

  /// \brief Returns false if the image view does not reference any data yet.
  bool IsValid() const;

  /// \brief Constructs an image view with the given header and image data.
  void ResetAndViewExternalStorage(const nsImageHeader& header, nsConstByteBlobPtr imageData);

  /// \brief Convenience function to save the image to the given file.
  nsResult SaveTo(nsStringView sFileName) const;

  /// \brief Returns the header this image was constructed from.
  const nsImageHeader& GetHeader() const;

  /// \brief Returns a view to the entire data contained in this image.
  template <typename T>
  nsBlobPtr<const T> GetBlobPtr() const;

  nsConstByteBlobPtr GetByteBlobPtr() const;

  /// \brief Returns a view to the given sub-image.
  nsImageView GetSubImageView(nsUInt32 uiMipLevel = 0, nsUInt32 uiFace = 0, nsUInt32 uiArrayIndex = 0) const;

  /// \brief Returns a view to a sub-plane.
  nsImageView GetPlaneView(nsUInt32 uiMipLevel = 0, nsUInt32 uiFace = 0, nsUInt32 uiArrayIndex = 0, nsUInt32 uiPlaneIndex = 0) const;

  /// \brief Returns a view to z slice of the image.
  nsImageView GetSliceView(nsUInt32 uiMipLevel = 0, nsUInt32 uiFace = 0, nsUInt32 uiArrayIndex = 0, nsUInt32 z = 0, nsUInt32 uiPlaneIndex = 0) const;

  /// \brief Returns a view to a row of pixels resp. blocks.
  nsImageView GetRowView(nsUInt32 uiMipLevel = 0, nsUInt32 uiFace = 0, nsUInt32 uiArrayIndex = 0, nsUInt32 y = 0, nsUInt32 z = 0, nsUInt32 uiPlaneIndex = 0) const;

  /// \brief Returns a pointer to a given pixel or block contained in a sub-image.
  template <typename T>
  const T* GetPixelPointer(
    nsUInt32 uiMipLevel = 0, nsUInt32 uiFace = 0, nsUInt32 uiArrayIndex = 0, nsUInt32 x = 0, nsUInt32 y = 0, nsUInt32 z = 0, nsUInt32 uiPlaneIndex = 0) const;

  /// \brief Reinterprets the image with a given format; the format must have the same size in bits per pixel as the current one.
  void ReinterpretAs(nsImageFormat::Enum format);

public:
  using nsImageHeader::GetDepth;
  using nsImageHeader::GetHeight;
  using nsImageHeader::GetWidth;

  using nsImageHeader::GetNumArrayIndices;
  using nsImageHeader::GetNumFaces;
  using nsImageHeader::GetNumMipLevels;
  using nsImageHeader::GetPlaneCount;

  using nsImageHeader::GetImageFormat;

  using nsImageHeader::GetNumBlocksX;
  using nsImageHeader::GetNumBlocksY;
  using nsImageHeader::GetNumBlocksZ;

  using nsImageHeader::GetDepthPitch;
  using nsImageHeader::GetRowPitch;

protected:
  nsUInt64 ComputeLayout();

  void ValidateSubImageIndices(nsUInt32 uiMipLevel, nsUInt32 uiFace, nsUInt32 uiArrayIndex, nsUInt32 uiPlaneIndex) const;
  template <typename T>
  void ValidateDataTypeAccessor(nsUInt32 uiPlaneIndex) const;

  const nsUInt64& GetSubImageOffset(nsUInt32 uiMipLevel, nsUInt32 uiFace, nsUInt32 uiArrayIndex, nsUInt32 uiPlaneIndex) const;

  nsHybridArray<nsUInt64, 16> m_SubImageOffsets;
  nsBlobPtr<nsUInt8> m_DataPtr;
};

/// \brief A class containing image data and associated meta data.
///
/// This class is a lightweight container for image data and the description required for interpreting the data,
/// such as the image format, its dimensions, number of sub-images (i.e. cubemap faces, mip levels and array sub-images).
/// However, it does not provide any methods for interpreting or  modifying of the image data.
///
/// The sub-images are stored in a predefined order compatible with the layout of DDS files, that is, it first stores
/// the mip chain for each image, then all faces in a case of a cubemap, then the individual images of an image array.
class NS_TEXTURE_DLL nsImage : public nsImageView
{
  /// Use Reset() instead
  void operator=(const nsImage& rhs) = delete;

  /// Use Reset() instead
  void operator=(const nsImageView& rhs) = delete;

  /// \brief Constructs an image with the given header; allocating internal storage for it.
  explicit nsImage(const nsImageHeader& header);

  /// \brief Constructs an image with the given header backed by user-supplied external storage.
  explicit nsImage(const nsImageHeader& header, nsByteBlobPtr externalData);

  /// \brief Constructor from image view (copies the image data to internal storage)
  explicit nsImage(const nsImageView& other);

public:
  NS_DECLARE_MEM_RELOCATABLE_TYPE();

  /// \brief Constructs an empty image.
  nsImage();

  /// \brief Move constructor
  nsImage(nsImage&& other);

  void operator=(nsImage&& rhs);

  /// \brief Constructs an empty image. If the image is attached to an external storage, the attachment is discarded.
  void Clear();

  /// \brief Constructs an image with the given header and ensures sufficient storage is allocated.
  ///
  /// \note If this nsImage was previously attached to external storage, this will reuse that storage.
  /// However, if the external storage is not sufficiently large, ResetAndAlloc() will detach from it and allocate internal storage.
  void ResetAndAlloc(const nsImageHeader& header);

  /// \brief Constructs an image with the given header and attaches to the user-supplied external storage.
  ///
  /// The user is responsible to keep the external storage alive as long as this nsImage is alive.
  void ResetAndUseExternalStorage(const nsImageHeader& header, nsByteBlobPtr externalData);

  /// \brief Moves the given data into this object.
  ///
  /// If \a other is attached to an external storage, this object will also be attached to it,
  /// so life-time requirements for the external storage are now bound to this instance.
  void ResetAndMove(nsImage&& other);

  /// \brief Constructs from an image view. Copies the image data to internal storage.
  ///
  /// If the image is currently attached to external storage, the attachment is discarded.
  void ResetAndCopy(const nsImageView& other);

  /// \brief Convenience function to load the image from the given file.
  nsResult LoadFrom(nsStringView sFileName);

  /// \brief Convenience function to convert the image to the given format.
  nsResult Convert(nsImageFormat::Enum targetFormat);

  /// \brief Returns a view to the entire data contained in this image.
  template <typename T>
  nsBlobPtr<T> GetBlobPtr();

  nsByteBlobPtr GetByteBlobPtr();

  using nsImageView::GetBlobPtr;
  using nsImageView::GetByteBlobPtr;

  /// \brief Returns a view to the given sub-image.
  nsImage GetSubImageView(nsUInt32 uiMipLevel = 0, nsUInt32 uiFace = 0, nsUInt32 uiArrayIndex = 0);

  using nsImageView::GetSubImageView;

  /// \brief Returns a view to a sub-plane.
  nsImage GetPlaneView(nsUInt32 uiMipLevel = 0, nsUInt32 uiFace = 0, nsUInt32 uiArrayIndex = 0, nsUInt32 uiPlaneIndex = 0);

  using nsImageView::GetPlaneView;

  /// \brief Returns a view to z slice of the image.
  nsImage GetSliceView(nsUInt32 uiMipLevel = 0, nsUInt32 uiFace = 0, nsUInt32 uiArrayIndex = 0, nsUInt32 z = 0, nsUInt32 uiPlaneIndex = 0);

  using nsImageView::GetSliceView;

  /// \brief Returns a view to a row of pixels resp. blocks.
  nsImage GetRowView(nsUInt32 uiMipLevel = 0, nsUInt32 uiFace = 0, nsUInt32 uiArrayIndex = 0, nsUInt32 y = 0, nsUInt32 z = 0, nsUInt32 uiPlaneIndex = 0);

  using nsImageView::GetRowView;

  /// \brief Returns a pointer to a given pixel or block contained in a sub-image.
  template <typename T>
  T* GetPixelPointer(nsUInt32 uiMipLevel = 0, nsUInt32 uiFace = 0, nsUInt32 uiArrayIndex = 0, nsUInt32 x = 0, nsUInt32 y = 0, nsUInt32 z = 0, nsUInt32 uiPlaneIndex = 0);

  using nsImageView::GetPixelPointer;

private:
  bool UsesExternalStorage() const;

  nsBlob m_InternalStorage;
};

#include <Texture/Image/Implementation/Image_inl.h>

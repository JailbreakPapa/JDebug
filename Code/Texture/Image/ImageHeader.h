#pragma once

#include <Foundation/Basics/Assert.h>
#include <Foundation/Math/Color8UNorm.h>
#include <Foundation/Math/Math.h>

#include <Texture/Image/ImageFormat.h>
#include <Texture/TextureDLL.h>

/// \brief A class containing image meta data, such as format and dimensions.
///
/// This class has no associated behavior or functionality, and its getters and setters have no effect other than changing
/// the contained value. It is intended as a container to be modified by image utils and loaders.
class NS_TEXTURE_DLL nsImageHeader
{
public:
  /// \brief Constructs an image using an unknown format and zero size.
  nsImageHeader() { Clear(); }

  /// \brief Constructs an image using an unknown format and zero size.
  void Clear()
  {
    m_uiNumMipLevels = 1;
    m_uiNumFaces = 1;
    m_uiNumArrayIndices = 1;
    m_uiWidth = 0;
    m_uiHeight = 0;
    m_uiDepth = 1;
    m_Format = nsImageFormat::UNKNOWN;
  }

  /// \brief Sets the image format.
  void SetImageFormat(const nsImageFormat::Enum& format) { m_Format = format; }

  /// \brief Returns the image format.
  nsImageFormat::Enum GetImageFormat() const { return m_Format; }

  /// \brief Sets the image width.
  void SetWidth(nsUInt32 uiWidth) { m_uiWidth = uiWidth; }

  /// \brief Returns the image width for a given mip level, clamped to 1.
  nsUInt32 GetWidth(nsUInt32 uiMipLevel = 0) const
  {
    NS_ASSERT_DEV(uiMipLevel < m_uiNumMipLevels, "Invalid mip level");
    return nsMath::Max(m_uiWidth >> uiMipLevel, 1U);
  }

  /// \brief Sets the image height.
  void SetHeight(nsUInt32 uiHeight) { m_uiHeight = uiHeight; }

  /// \brief Returns the image height for a given mip level, clamped to 1.
  nsUInt32 GetHeight(nsUInt32 uiMipLevel = 0) const
  {
    NS_ASSERT_DEV(uiMipLevel < m_uiNumMipLevels, "Invalid mip level");
    return nsMath::Max(m_uiHeight >> uiMipLevel, 1U);
  }

  /// \brief Sets the image depth. The default is 1.
  void SetDepth(nsUInt32 uiDepth) { m_uiDepth = uiDepth; }

  /// \brief Returns the image depth for a given mip level, clamped to 1.
  nsUInt32 GetDepth(nsUInt32 uiMipLevel = 0) const
  {
    NS_ASSERT_DEV(uiMipLevel < m_uiNumMipLevels, "Invalid mip level");
    return nsMath::Max(m_uiDepth >> uiMipLevel, 1U);
  }

  /// \brief Sets the number of mip levels, including the full-size image.
  ///
  /// Setting this to 0 will result in an empty image.
  void SetNumMipLevels(nsUInt32 uiNumMipLevels) { m_uiNumMipLevels = uiNumMipLevels; }

  /// \brief Returns the number of mip levels, including the full-size image.
  nsUInt32 GetNumMipLevels() const { return m_uiNumMipLevels; }

  /// \brief Sets the number of cubemap faces. Use 1 for a non-cubemap.
  ///
  /// Setting this to 0 will result in an empty image.
  void SetNumFaces(nsUInt32 uiNumFaces) { m_uiNumFaces = uiNumFaces; }

  /// \brief Returns the number of cubemap faces, or 1 for a non-cubemap.
  nsUInt32 GetNumFaces() const { return m_uiNumFaces; }

  /// \brief Sets the number of array indices.
  ///
  /// Setting this to 0 will result in an empty image.
  void SetNumArrayIndices(nsUInt32 uiNumArrayIndices) { m_uiNumArrayIndices = uiNumArrayIndices; }

  /// \brief Returns the number of array indices.
  nsUInt32 GetNumArrayIndices() const { return m_uiNumArrayIndices; }

  /// \brief Returns the number of image planes.
  nsUInt32 GetPlaneCount() const
  {
    return nsImageFormat::GetPlaneCount(m_Format);
  }

  /// \brief Returns the number of blocks contained in a given mip level in the horizontal direction.
  nsUInt32 GetNumBlocksX(nsUInt32 uiMipLevel = 0, nsUInt32 uiPlaneIndex = 0) const
  {
    return nsImageFormat::GetNumBlocksX(m_Format, GetWidth(uiMipLevel), uiPlaneIndex);
  }

  /// \brief Returns the number of blocks contained in a given mip level in the horizontal direction.
  nsUInt32 GetNumBlocksY(nsUInt32 uiMipLevel = 0, nsUInt32 uiPlaneIndex = 0) const
  {
    return nsImageFormat::GetNumBlocksY(m_Format, GetHeight(uiMipLevel), uiPlaneIndex);
  }

  /// \brief Returns the number of blocks contained in a given mip level in the depth direction.
  nsUInt32 GetNumBlocksZ(nsUInt32 uiMipLevel = 0, nsUInt32 uiPlaneIndex = 0) const
  {
    return nsImageFormat::GetNumBlocksZ(m_Format, GetDepth(uiMipLevel), uiPlaneIndex);
  }

  /// \brief Returns the offset in bytes between two subsequent rows of the given mip level.
  nsUInt64 GetRowPitch(nsUInt32 uiMipLevel = 0, nsUInt32 uiPlaneIndex = 0) const
  {
    return nsImageFormat::GetRowPitch(m_Format, GetWidth(uiMipLevel), uiPlaneIndex);
  }

  /// \brief Returns the offset in bytes between two subsequent depth slices of the given mip level.
  nsUInt64 GetDepthPitch(nsUInt32 uiMipLevel = 0, nsUInt32 uiPlaneIndex = 0) const
  {
    return nsImageFormat::GetDepthPitch(m_Format, GetWidth(uiMipLevel), GetHeight(uiMipLevel), uiPlaneIndex);
  }

  /// \brief Computes the data size required for an image with the header's format and dimensions.
  nsUInt64 ComputeDataSize() const
  {
    nsUInt64 uiDataSize = 0;

    for (nsUInt32 uiMipLevel = 0; uiMipLevel < GetNumMipLevels(); uiMipLevel++)
    {
      for (nsUInt32 uiPlaneIndex = 0; uiPlaneIndex < GetPlaneCount(); ++uiPlaneIndex)
      {
        uiDataSize += GetDepthPitch(uiMipLevel, uiPlaneIndex) * static_cast<nsUInt64>(GetDepth(uiMipLevel));
      }
    }

    return nsMath::SafeMultiply64(uiDataSize, nsMath::SafeMultiply32(GetNumArrayIndices(), GetNumFaces()));
  }

  /// \brief Computes the number of mip maps in the full mip chain.
  nsUInt32 ComputeNumberOfMipMaps() const
  {
    nsUInt32 numMipMaps = 1;
    nsUInt32 width = GetWidth();
    nsUInt32 height = GetHeight();
    nsUInt32 depth = GetDepth();

    while (width > 1 || height > 1 || depth > 1)
    {
      width = nsMath::Max(1u, width / 2);
      height = nsMath::Max(1u, height / 2);
      depth = nsMath::Max(1u, depth / 2);

      numMipMaps++;
    }

    return numMipMaps;
  }

  bool operator==(const nsImageHeader& other) const
  {
    return m_uiNumMipLevels == other.m_uiNumMipLevels &&
           m_uiNumFaces == other.m_uiNumFaces &&
           m_uiNumArrayIndices == other.m_uiNumArrayIndices &&
           m_uiWidth == other.m_uiWidth &&
           m_uiHeight == other.m_uiHeight &&
           m_uiDepth == other.m_uiDepth &&
           m_Format == other.m_Format;
  }

  bool operator!=(const nsImageHeader& other) const
  {
    return !operator==(other);
  }

protected:
  nsUInt32 m_uiNumMipLevels;
  nsUInt32 m_uiNumFaces;
  nsUInt32 m_uiNumArrayIndices;

  nsUInt32 m_uiWidth;
  nsUInt32 m_uiHeight;
  nsUInt32 m_uiDepth;

  nsImageFormat::Enum m_Format;
};

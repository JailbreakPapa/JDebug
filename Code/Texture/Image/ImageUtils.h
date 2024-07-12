#pragma once

#include <Foundation/Math/Rect.h>
#include <Foundation/Math/Size.h>
#include <Texture/Image/Image.h>
#include <Texture/Image/ImageEnums.h>
#include <Texture/Image/ImageFilter.h>

class NS_TEXTURE_DLL nsImageUtils
{
public:
  /// \brief Returns the image with the difference (absolute values) between ImageA and ImageB.
  static void ComputeImageDifferenceABS(const nsImageView& imageA, const nsImageView& imageB, nsImage& out_difference);

  /// \brief Same as ComputeImageDifferenceABS, but for every pixel in imageA, the minimum diff in imageB is searched in a 1-pixel radius, allowing pixels in B to shift slightly without incurring a difference.
  static void ComputeImageDifferenceABSRelaxed(const nsImageView& imageA, const nsImageView& imageB, nsImage& out_difference);

  /// \brief Computes the mean square error for the block at (offsetx, offsety) to (offsetx + uiBlockSize, offsety + uiBlockSize).
  /// DifferenceImage is expected to be an image that represents the difference between two images.
  static nsUInt32 ComputeMeanSquareError(const nsImageView& differenceImage, nsUInt8 uiBlockSize, nsUInt32 uiOffsetx, nsUInt32 uiOffsety);

  /// \brief Computes the mean square error of DifferenceImage, by computing the MSE for blocks of uiBlockSize and returning the maximum MSE
  /// that was found.
  static nsUInt32 ComputeMeanSquareError(const nsImageView& differenceImage, nsUInt8 uiBlockSize);

  /// \brief Rescales pixel values to use the full value range by scaling from [min, max] to [0, 255].
  /// Computes combined min/max for RGB and separate min/max for alpha.
  static void Normalize(nsImage& ref_image);
  static void Normalize(nsImage& ref_image, nsUInt8& ref_uiMinRgb, nsUInt8& ref_uiMaxRgb, nsUInt8& ref_uiMinAlpha, nsUInt8& ref_uiMaxAlpha);

  /// \brief Extracts the alpha channel from 8bpp 4 channel images into a 8bpp single channel image.
  static void ExtractAlphaChannel(const nsImageView& inputImage, nsImage& ref_outputImage);

  /// \brief Returns the sub-image of \a input that starts at \a offset and has the size \a newsize
  static void CropImage(const nsImageView& input, const nsVec2I32& vOffset, const nsSizeU32& newsize, nsImage& ref_output);

  /// \brief rotates a sub image by 180 degrees in place. Only works with uncompressed images.
  static void RotateSubImage180(nsImage& ref_image, nsUInt32 uiMipLevel = 0, nsUInt32 uiFace = 0, nsUInt32 uiArrayIndex = 0);

  /// \brief Copies the source image into the destination image at the specified location.
  ///
  /// The image must fit, no scaling or cropping is done. Image formats must be identical. Compressed formats are not supported.
  /// If the target location leaves not enough room for the source image to be copied, bad stuff will happen.
  static nsResult Copy(const nsImageView& srcImg, const nsRectU32& srcRect, nsImage& ref_dstImg, const nsVec3U32& vDstOffset, nsUInt32 uiDstMipLevel = 0,
    nsUInt32 uiDstFace = 0, nsUInt32 uiDstArrayIndex = 0);

  /// \brief Copies the lower uiNumMips data of a 2D image into another one.
  static nsResult ExtractLowerMipChain(const nsImageView& src, nsImage& ref_dst, nsUInt32 uiNumMips);

  /// Mip map generation options
  struct MipMapOptions
  {
    /// The filter to use for mipmap generation. Defaults to bilinear filtering (Triangle filter) if none is given.
    const nsImageFilter* m_filter = nullptr;

    /// Rescale RGB components to unit length.
    bool m_renormalizeNormals = false;

    /// If true, the alpha values are scaled to preserve the average coverage when alpha testing is enabled,
    bool m_preserveCoverage = false;

    /// The alpha test threshold to use when m_preserveCoverage == true.
    float m_alphaThreshold = 0.5f;

    /// The address mode for samples when filtering outside of the image dimensions in the horizontal direction.
    nsImageAddressMode::Enum m_addressModeU = nsImageAddressMode::Clamp;

    /// The address mode for samples when filtering outside of the image dimensions in the vertical direction.
    nsImageAddressMode::Enum m_addressModeV = nsImageAddressMode::Clamp;

    /// The address mode for samples when filtering outside of the image dimensions in the depth direction.
    nsImageAddressMode::Enum m_addressModeW = nsImageAddressMode::Clamp;

    /// The border color if texture address mode equals BORDER.
    nsColor m_borderColor = nsColor::Black;

    /// How many mip maps should be generated. Pass 0 to generate all mip map levels.
    nsUInt32 m_numMipMaps = 0;
  };

  /// Scales the image.
  static nsResult Scale(const nsImageView& source, nsImage& ref_target, nsUInt32 uiWidth, nsUInt32 uiHeight, const nsImageFilter* pFilter = nullptr,
    nsImageAddressMode::Enum addressModeU = nsImageAddressMode::Clamp, nsImageAddressMode::Enum addressModeV = nsImageAddressMode::Clamp,
    const nsColor& borderColor = nsColor::Black);

  /// Scales the image.
  static nsResult Scale3D(const nsImageView& source, nsImage& ref_target, nsUInt32 uiWidth, nsUInt32 uiHeight, nsUInt32 uiDepth,
    const nsImageFilter* pFilter = nullptr, nsImageAddressMode::Enum addressModeU = nsImageAddressMode::Clamp,
    nsImageAddressMode::Enum addressModeV = nsImageAddressMode::Clamp, nsImageAddressMode::Enum addressModeW = nsImageAddressMode::Clamp,
    const nsColor& borderColor = nsColor::Black);

  /// Genererates the mip maps for the image. The input texture must be in nsImageFormat::R32_G32_B32_A32_FLOAT
  static void GenerateMipMaps(const nsImageView& source, nsImage& ref_target, const MipMapOptions& options);

  /// Assumes that the Red and Green components of an image contain XY of an unit length normal and reconstructs the Z component into B
  static void ReconstructNormalZ(nsImage& ref_source);

  /// Renormalizes a normal map to unit length.
  static void RenormalizeNormalMap(nsImage& ref_image);

  /// Adjust the roughness in lower mip levels so it maintains the same look from all distances.
  static void AdjustRoughness(nsImage& ref_roughnessMap, const nsImageView& normalMap);

  /// \brief Changes the exposure of an HDR image by 2^bias
  static void ChangeExposure(nsImage& ref_image, float fBias);

  /// \brief Creates a cubemap from srcImg and stores it in dstImg.
  ///
  /// If srcImg is already a cubemap, the data will be copied 1:1 to dstImg.
  /// If it is a 2D texture, it is analyzed and sub-images are copied to the proper faces of the output cubemap.
  ///
  /// Supported input layouts are:
  ///  * Vertical Cross
  ///  * Horizontal Cross
  ///  * Spherical mapping
  static nsResult CreateCubemapFromSingleFile(nsImage& ref_dstImg, const nsImageView& srcImg);

  /// \brief Copies the 6 given source images to the faces of dstImg.
  ///
  /// All input images must have the same square, power-of-two dimensions and mustn't be compressed.
  static nsResult CreateCubemapFrom6Files(nsImage& ref_dstImg, const nsImageView* pSourceImages);

  static nsResult CreateVolumeTextureFromSingleFile(nsImage& ref_dstImg, const nsImageView& srcImg);

  static nsUInt32 GetSampleIndex(nsUInt32 uiNumTexels, nsInt32 iIndex, nsImageAddressMode::Enum addressMode, bool& out_bUseBorderColor);

  /// \brief Samples the image at the given UV coordinates with nearest filtering.
  ///
  /// This function has to validate that the image is of the right format, and has to query the pixel pointer, which is slow.
  /// If you need to sample the image very often, use the overload that takes a pixel pointer instead of an image.
  static nsColor NearestSample(const nsImageView& image, nsImageAddressMode::Enum addressMode, nsVec2 vUv);

  /// \brief Samples the image at the given UV coordinates with nearest filtering.
  ///
  /// Prefer this function over the one that takes an nsImageView when you need to sample the image very often,
  /// as it does away with internal validation that would be redundant. Also, the pixel pointer given to this function
  /// should be retrieved only once from the source image, as nsImage::GetPixelPointer() is rather slow due to validation overhead.
  static nsColor NearestSample(const nsColor* pPixelPointer, nsUInt32 uiWidth, nsUInt32 uiHeight, nsImageAddressMode::Enum addressMode, nsVec2 vUv);

  /// \brief Samples the image at the given UV coordinates with bilinear filtering.
  ///
  /// This function has to validate that the image is of the right format, and has to query the pixel pointer, which is slow.
  /// If you need to sample the image very often, use the overload that takes a pixel pointer instead of an image.
  static nsColor BilinearSample(const nsImageView& image, nsImageAddressMode::Enum addressMode, nsVec2 vUv);

  /// \brief Samples the image at the given UV coordinates with bilinear filtering.
  ///
  /// Prefer this function over the one that takes an nsImageView when you need to sample the image very often,
  /// as it does away with internal validation that would be redundant. Also, the pixel pointer given to this function
  /// should be retrieved only once from the source image, as nsImage::GetPixelPointer() is rather slow due to validation overhead.
  static nsColor BilinearSample(const nsColor* pPixelPointer, nsUInt32 uiWidth, nsUInt32 uiHeight, nsImageAddressMode::Enum addressMode, nsVec2 vUv);

  /// \brief Copies channel 0, 1, 2 or 3 from srcImg into dstImg.
  ///
  /// Currently only supports images of format R32G32B32A32_FLOAT and with identical resolution.
  /// Returns failure if any of those requirements are not met.
  static nsResult CopyChannel(nsImage& ref_dstImg, nsUInt8 uiDstChannelIdx, const nsImage& srcImg, nsUInt8 uiSrcChannelIdx);

  /// \brief Embeds the image as Base64 encoded text into an HTML file.
  static void EmbedImageData(nsStringBuilder& out_sHtml, const nsImage& image);

  /// \brief Generates an HTML file containing the given images with mouse-over functionality to compare them.
  static void CreateImageDiffHtml(nsStringBuilder& out_sHtml, nsStringView sTitle, const nsImage& referenceImgRgb, const nsImage& referenceImgAlpha, const nsImage& capturedImgRgb, const nsImage& capturedImgAlpha, const nsImage& diffImgRgb, const nsImage& diffImgAlpha, nsUInt32 uiError, nsUInt32 uiThreshold, nsUInt8 uiMinDiffRgb, nsUInt8 uiMaxDiffRgb, nsUInt8 uiMinDiffAlpha, nsUInt8 uiMaxDiffAlpha);
};

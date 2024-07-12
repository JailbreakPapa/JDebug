#include <Texture/TexturePCH.h>

#include <Texture/Image/ImageUtils.h>

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/SimdMath/SimdVec4f.h>
#include <Foundation/Time/Timestamp.h>
#include <Texture/Image/ImageConversion.h>
#include <Texture/Image/ImageEnums.h>
#include <Texture/Image/ImageFilter.h>

template <typename TYPE>
static void SetDiff(const nsImageView& imageA, const nsImageView& imageB, nsImage& out_difference, nsUInt32 w, nsUInt32 h, nsUInt32 d, nsUInt32 uiComp)
{
  const TYPE* pA = imageA.GetPixelPointer<TYPE>(0, 0, 0, w, h, d);
  const TYPE* pB = imageB.GetPixelPointer<TYPE>(0, 0, 0, w, h, d);
  TYPE* pR = out_difference.GetPixelPointer<TYPE>(0, 0, 0, w, h, d);

  for (nsUInt32 i = 0; i < uiComp; ++i)
    pR[i] = pB[i] > pA[i] ? (pB[i] - pA[i]) : (pA[i] - pB[i]);
}

template <typename TYPE, typename ACCU, int COMP>
static void SetCompMinDiff(const nsImageView& newDifference, nsImage& out_minDifference, nsUInt32 w, nsUInt32 h, nsUInt32 d, nsUInt32 uiComp)
{
  const TYPE* pNew = newDifference.GetPixelPointer<TYPE>(0, 0, 0, w, h, d);
  TYPE* pR = out_minDifference.GetPixelPointer<TYPE>(0, 0, 0, w, h, d);

  for (nsUInt32 i = 0; i < uiComp; i += COMP)
  {
    ACCU minDiff = 0;
    ACCU newDiff = 0;
    for (nsUInt32 c = 0; c < COMP; c++)
    {
      minDiff += pR[i + c];
      newDiff += pNew[i + c];
    }
    if (minDiff > newDiff)
    {
      for (nsUInt32 c = 0; c < COMP; c++)
        pR[i + c] = pNew[i + c];
    }
  }
}

template <typename TYPE>
static nsUInt32 GetError(const nsImageView& difference, nsUInt32 w, nsUInt32 h, nsUInt32 d, nsUInt32 uiComp, nsUInt32 uiPixel)
{
  const TYPE* pR = difference.GetPixelPointer<TYPE>(0, 0, 0, w, h, d);

  nsUInt32 uiErrorSum = 0;

  for (nsUInt32 p = 0; p < uiPixel; ++p)
  {
    nsUInt32 error = 0;

    for (nsUInt32 c = 0; c < uiComp; ++c)
    {
      error += *pR;
      ++pR;
    }

    error /= uiComp;
    uiErrorSum += error * error;
  }

  return uiErrorSum;
}

void nsImageUtils::ComputeImageDifferenceABS(const nsImageView& imageA, const nsImageView& imageB, nsImage& out_difference)
{
  NS_PROFILE_SCOPE("nsImageUtils::ComputeImageDifferenceABS");

  NS_ASSERT_DEV(imageA.GetWidth() == imageB.GetWidth(), "Dimensions do not match");
  NS_ASSERT_DEV(imageA.GetHeight() == imageB.GetHeight(), "Dimensions do not match");
  NS_ASSERT_DEV(imageA.GetDepth() == imageB.GetDepth(), "Dimensions do not match");
  NS_ASSERT_DEV(imageA.GetImageFormat() == imageB.GetImageFormat(), "Format does not match");

  nsImageHeader differenceHeader;

  differenceHeader.SetWidth(imageA.GetWidth());
  differenceHeader.SetHeight(imageA.GetHeight());
  differenceHeader.SetDepth(imageA.GetDepth());
  differenceHeader.SetImageFormat(imageA.GetImageFormat());
  out_difference.ResetAndAlloc(differenceHeader);

  const nsUInt32 uiSize2D = imageA.GetHeight() * imageA.GetWidth();

  for (nsUInt32 d = 0; d < imageA.GetDepth(); ++d)
  {
    // for (nsUInt32 h = 0; h < ImageA.GetHeight(); ++h)
    {
      // for (nsUInt32 w = 0; w < ImageA.GetWidth(); ++w)
      {
        switch (imageA.GetImageFormat())
        {
          case nsImageFormat::R8G8B8A8_UNORM:
          case nsImageFormat::R8G8B8A8_UNORM_SRGB:
          case nsImageFormat::R8G8B8A8_UINT:
          case nsImageFormat::R8G8B8A8_SNORM:
          case nsImageFormat::R8G8B8A8_SINT:
          case nsImageFormat::B8G8R8A8_UNORM:
          case nsImageFormat::B8G8R8X8_UNORM:
          case nsImageFormat::B8G8R8A8_UNORM_SRGB:
          case nsImageFormat::B8G8R8X8_UNORM_SRGB:
          {
            SetDiff<nsUInt8>(imageA, imageB, out_difference, 0, 0, d, 4 * uiSize2D);
          }
          break;

          case nsImageFormat::B8G8R8_UNORM:
          {
            SetDiff<nsUInt8>(imageA, imageB, out_difference, 0, 0, d, 3 * uiSize2D);
          }
          break;

          default:
            NS_REPORT_FAILURE("The nsImageFormat {0} is not implemented", (nsUInt32)imageA.GetImageFormat());
            return;
        }
      }
    }
  }
}


void nsImageUtils::ComputeImageDifferenceABSRelaxed(const nsImageView& imageA, const nsImageView& imageB, nsImage& out_difference)
{
  NS_ASSERT_ALWAYS(imageA.GetDepth() == 1 && imageA.GetNumMipLevels() == 1, "Depth slices and mipmaps are not supported");

  NS_PROFILE_SCOPE("nsImageUtils::ComputeImageDifferenceABSRelaxed");

  ComputeImageDifferenceABS(imageA, imageB, out_difference);

  nsImage tempB;
  tempB.ResetAndCopy(imageB);
  nsImage tempDiff;
  tempDiff.ResetAndCopy(out_difference);

  for (nsInt32 yOffset = -1; yOffset <= 1; ++yOffset)
  {
    for (nsInt32 xOffset = -1; xOffset <= 1; ++xOffset)
    {
      if (yOffset == 0 && xOffset == 0)
        continue;

      nsImageUtils::Copy(imageB, nsRectU32(nsMath::Max(xOffset, 0), nsMath::Max(yOffset, 0), imageB.GetWidth() - nsMath::Abs(xOffset), imageB.GetHeight() - nsMath::Abs(yOffset)), tempB, nsVec3U32(-nsMath::Min(xOffset, 0), -nsMath::Min(yOffset, 0), 0)).AssertSuccess("");

      ComputeImageDifferenceABS(imageA, tempB, tempDiff);

      const nsUInt32 uiSize2D = imageA.GetHeight() * imageA.GetWidth();
      switch (imageA.GetImageFormat())
      {
        case nsImageFormat::R8G8B8A8_UNORM:
        case nsImageFormat::R8G8B8A8_UNORM_SRGB:
        case nsImageFormat::R8G8B8A8_UINT:
        case nsImageFormat::R8G8B8A8_SNORM:
        case nsImageFormat::R8G8B8A8_SINT:
        case nsImageFormat::B8G8R8A8_UNORM:
        case nsImageFormat::B8G8R8X8_UNORM:
        case nsImageFormat::B8G8R8A8_UNORM_SRGB:
        case nsImageFormat::B8G8R8X8_UNORM_SRGB:
        {
          SetCompMinDiff<nsUInt8, nsUInt32, 4>(tempDiff, out_difference, 0, 0, 0, 4 * uiSize2D);
        }
        break;

        case nsImageFormat::B8G8R8_UNORM:
        {
          SetCompMinDiff<nsUInt8, nsUInt32, 3>(tempDiff, out_difference, 0, 0, 0, 3 * uiSize2D);
        }
        break;

        default:
          NS_REPORT_FAILURE("The nsImageFormat {0} is not implemented", (nsUInt32)imageA.GetImageFormat());
          return;
      }
    }
  }
}

nsUInt32 nsImageUtils::ComputeMeanSquareError(const nsImageView& differenceImage, nsUInt8 uiBlockSize, nsUInt32 uiOffsetx, nsUInt32 uiOffsety)
{
  NS_PROFILE_SCOPE("nsImageUtils::ComputeMeanSquareError(detail)");

  NS_ASSERT_DEV(uiBlockSize > 1, "Blocksize must be at least 2");

  nsUInt32 uiNumComponents = nsImageFormat::GetNumChannels(differenceImage.GetImageFormat());

  nsUInt32 uiWidth = nsMath::Min(differenceImage.GetWidth(), uiOffsetx + uiBlockSize) - uiOffsetx;
  nsUInt32 uiHeight = nsMath::Min(differenceImage.GetHeight(), uiOffsety + uiBlockSize) - uiOffsety;

  // Treat image as single-component format and scale the width instead
  uiWidth *= uiNumComponents;

  if (uiWidth == 0 || uiHeight == 0)
    return 0;

  switch (differenceImage.GetImageFormat())
  {
      // Supported formats
    case nsImageFormat::R8G8B8A8_UNORM:
    case nsImageFormat::R8G8B8A8_UNORM_SRGB:
    case nsImageFormat::R8G8B8A8_UINT:
    case nsImageFormat::R8G8B8A8_SNORM:
    case nsImageFormat::R8G8B8A8_SINT:
    case nsImageFormat::B8G8R8A8_UNORM:
    case nsImageFormat::B8G8R8A8_UNORM_SRGB:
    case nsImageFormat::B8G8R8_UNORM:
      break;

    default:
      NS_REPORT_FAILURE("The nsImageFormat {0} is not implemented", (nsUInt32)differenceImage.GetImageFormat());
      return 0;
  }


  nsUInt32 error = 0;

  nsUInt64 uiRowPitch = differenceImage.GetRowPitch();
  nsUInt64 uiDepthPitch = differenceImage.GetDepthPitch();

  const nsUInt32 uiSize2D = uiWidth * uiHeight;
  const nsUInt8* pSlicePointer = differenceImage.GetPixelPointer<nsUInt8>(0, 0, 0, uiOffsetx, uiOffsety);

  for (nsUInt32 d = 0; d < differenceImage.GetDepth(); ++d)
  {
    const nsUInt8* pRowPointer = pSlicePointer;

    for (nsUInt32 y = 0; y < uiHeight; ++y)
    {
      const nsUInt8* pPixelPointer = pRowPointer;
      for (nsUInt32 x = 0; x < uiWidth; ++x)
      {
        nsUInt32 uiDiff = *pPixelPointer;
        error += uiDiff * uiDiff;

        pPixelPointer++;
      }

      pRowPointer += uiRowPitch;
    }

    pSlicePointer += uiDepthPitch;
  }

  error /= uiSize2D;
  return error;
}

nsUInt32 nsImageUtils::ComputeMeanSquareError(const nsImageView& differenceImage, nsUInt8 uiBlockSize)
{
  NS_PROFILE_SCOPE("nsImageUtils::ComputeMeanSquareError");

  NS_ASSERT_DEV(uiBlockSize > 1, "Blocksize must be at least 2");

  const nsUInt32 uiHalfBlockSize = uiBlockSize / 2;

  const nsUInt32 uiBlocksX = (differenceImage.GetWidth() / uiHalfBlockSize) + 1;
  const nsUInt32 uiBlocksY = (differenceImage.GetHeight() / uiHalfBlockSize) + 1;

  nsUInt32 uiMaxError = 0;

  for (nsUInt32 by = 0; by < uiBlocksY; ++by)
  {
    for (nsUInt32 bx = 0; bx < uiBlocksX; ++bx)
    {
      const nsUInt32 uiBlockError = ComputeMeanSquareError(differenceImage, uiBlockSize, bx * uiHalfBlockSize, by * uiHalfBlockSize);

      uiMaxError = nsMath::Max(uiMaxError, uiBlockError);
    }
  }

  return uiMaxError;
}

template <typename Func, typename ImageType>
static void ApplyFunc(ImageType& inout_image, Func func)
{
  nsUInt32 uiWidth = inout_image.GetWidth();
  nsUInt32 uiHeight = inout_image.GetHeight();
  nsUInt32 uiDepth = inout_image.GetDepth();

  NS_IGNORE_UNUSED(uiDepth);
  NS_ASSERT_DEV(uiWidth > 0 && uiHeight > 0 && uiDepth > 0, "The image passed to FindMinMax has illegal dimension {}x{}x{}.", uiWidth, uiHeight, uiDepth);

  nsUInt64 uiRowPitch = inout_image.GetRowPitch();
  nsUInt64 uiDepthPitch = inout_image.GetDepthPitch();
  nsUInt32 uiNumChannels = nsImageFormat::GetNumChannels(inout_image.GetImageFormat());

  auto pSlicePointer = inout_image.template GetPixelPointer<nsUInt8>();

  for (nsUInt32 z = 0; z < inout_image.GetDepth(); ++z)
  {
    auto pRowPointer = pSlicePointer;

    for (nsUInt32 y = 0; y < uiHeight; ++y)
    {
      auto pPixelPointer = pRowPointer;
      for (nsUInt32 x = 0; x < uiWidth; ++x)
      {
        for (nsUInt32 c = 0; c < uiNumChannels; ++c)
        {
          func(pPixelPointer++, x, y, z, c);
        }
      }

      pRowPointer += uiRowPitch;
    }

    pSlicePointer += uiDepthPitch;
  }
}

static void FindMinMax(const nsImageView& image, nsUInt8& out_uiMinRgb, nsUInt8& out_uiMaxRgb, nsUInt8& out_uiMinAlpha, nsUInt8& out_uiMaxAlpha)
{
  nsImageFormat::Enum imageFormat = image.GetImageFormat();
  NS_IGNORE_UNUSED(imageFormat);
  NS_ASSERT_DEV(nsImageFormat::GetBitsPerChannel(imageFormat, nsImageFormatChannel::R) == 8 && nsImageFormat::GetDataType(imageFormat) == nsImageFormatDataType::UNORM, "Only 8bpp unorm formats are supported in FindMinMax");

  out_uiMinRgb = 255u;
  out_uiMinAlpha = 255u;
  out_uiMaxRgb = 0u;
  out_uiMaxAlpha = 0u;

  auto minMax = [&](const nsUInt8* pPixel, nsUInt32 /*x*/, nsUInt32 /*y*/, nsUInt32 /*z*/, nsUInt32 c)
  {
    nsUInt8 val = *pPixel;

    if (c < 3)
    {
      out_uiMinRgb = nsMath::Min(out_uiMinRgb, val);
      out_uiMaxRgb = nsMath::Max(out_uiMaxRgb, val);
    }
    else
    {
      out_uiMinAlpha = nsMath::Min(out_uiMinAlpha, val);
      out_uiMaxAlpha = nsMath::Max(out_uiMaxAlpha, val);
    }
  };
  ApplyFunc(image, minMax);
}

void nsImageUtils::Normalize(nsImage& inout_image)
{
  nsUInt8 uiMinRgb, uiMaxRgb, uiMinAlpha, uiMaxAlpha;
  Normalize(inout_image, uiMinRgb, uiMaxRgb, uiMinAlpha, uiMaxAlpha);
}

void nsImageUtils::Normalize(nsImage& inout_image, nsUInt8& out_uiMinRgb, nsUInt8& out_uiMaxRgb, nsUInt8& out_uiMinAlpha, nsUInt8& out_uiMaxAlpha)
{
  NS_PROFILE_SCOPE("nsImageUtils::Normalize");

  nsImageFormat::Enum imageFormat = inout_image.GetImageFormat();

  NS_ASSERT_DEV(nsImageFormat::GetBitsPerChannel(imageFormat, nsImageFormatChannel::R) == 8 && nsImageFormat::GetDataType(imageFormat) == nsImageFormatDataType::UNORM, "Only 8bpp unorm formats are supported in NormalizeImage");

  bool ignoreAlpha = false;
  if (imageFormat == nsImageFormat::B8G8R8X8_UNORM || imageFormat == nsImageFormat::B8G8R8X8_UNORM_SRGB)
  {
    ignoreAlpha = true;
  }

  FindMinMax(inout_image, out_uiMinRgb, out_uiMaxRgb, out_uiMinAlpha, out_uiMaxAlpha);
  nsUInt8 uiRangeRgb = out_uiMaxRgb - out_uiMinRgb;
  nsUInt8 uiRangeAlpha = out_uiMaxAlpha - out_uiMinAlpha;

  auto normalize = [&](nsUInt8* pPixel, nsUInt32 /*x*/, nsUInt32 /*y*/, nsUInt32 /*z*/, nsUInt32 c)
  {
    nsUInt8 val = *pPixel;
    if (c < 3)
    {
      // color channels are uniform when min == max, in that case keep original value as scaling is not meaningful
      if (uiRangeRgb != 0)
      {
        *pPixel = static_cast<nsUInt8>(255u * (static_cast<float>(val - out_uiMinRgb) / (uiRangeRgb)));
      }
    }
    else
    {
      // alpha is uniform when minAlpha == maxAlpha, in that case keep original alpha as scaling is not meaningful
      if (!ignoreAlpha && uiRangeAlpha != 0)
      {
        *pPixel = static_cast<nsUInt8>(255u * (static_cast<float>(val - out_uiMinAlpha) / (uiRangeAlpha)));
      }
    }
  };
  ApplyFunc(inout_image, normalize);
}

void nsImageUtils::ExtractAlphaChannel(const nsImageView& inputImage, nsImage& inout_outputImage)
{
  NS_PROFILE_SCOPE("nsImageUtils::ExtractAlphaChannel");

  switch (nsImageFormat::Enum imageFormat = inputImage.GetImageFormat())
  {
    case nsImageFormat::R8G8B8A8_UNORM:
    case nsImageFormat::R8G8B8A8_UNORM_SRGB:
    case nsImageFormat::R8G8B8A8_UINT:
    case nsImageFormat::R8G8B8A8_SNORM:
    case nsImageFormat::R8G8B8A8_SINT:
    case nsImageFormat::B8G8R8A8_UNORM:
    case nsImageFormat::B8G8R8A8_UNORM_SRGB:
      break;
    default:
      NS_REPORT_FAILURE("ExtractAlpha needs an image with 8bpp and 4 channel. The nsImageFormat {} is not supported.", (nsUInt32)imageFormat);
      return;
  }

  nsImageHeader outputHeader = inputImage.GetHeader();
  outputHeader.SetImageFormat(nsImageFormat::R8_UNORM);
  inout_outputImage.ResetAndAlloc(outputHeader);

  const nsUInt8* pInputSlice = inputImage.GetPixelPointer<nsUInt8>();
  nsUInt8* pOutputSlice = inout_outputImage.GetPixelPointer<nsUInt8>();

  nsUInt64 uiInputRowPitch = inputImage.GetRowPitch();
  nsUInt64 uiInputDepthPitch = inputImage.GetDepthPitch();

  nsUInt64 uiOutputRowPitch = inout_outputImage.GetRowPitch();
  nsUInt64 uiOutputDepthPitch = inout_outputImage.GetDepthPitch();

  for (nsUInt32 d = 0; d < inputImage.GetDepth(); ++d)
  {
    const nsUInt8* pInputRow = pInputSlice;
    nsUInt8* pOutputRow = pOutputSlice;

    for (nsUInt32 y = 0; y < inputImage.GetHeight(); ++y)
    {
      const nsUInt8* pInputPixel = pInputRow;
      nsUInt8* pOutputPixel = pOutputRow;
      for (nsUInt32 x = 0; x < inputImage.GetWidth(); ++x)
      {
        *pOutputPixel = pInputPixel[3];

        pInputPixel += 4;
        ++pOutputPixel;
      }

      pInputRow += uiInputRowPitch;
      pOutputRow += uiOutputRowPitch;
    }

    pInputSlice += uiInputDepthPitch;
    pOutputSlice += uiOutputDepthPitch;
  }
}

void nsImageUtils::CropImage(const nsImageView& input, const nsVec2I32& vOffset, const nsSizeU32& newsize, nsImage& out_output)
{
  NS_PROFILE_SCOPE("nsImageUtils::CropImage");

  NS_ASSERT_DEV(vOffset.x >= 0, "Offset is invalid");
  NS_ASSERT_DEV(vOffset.y >= 0, "Offset is invalid");
  NS_ASSERT_DEV(vOffset.x < (nsInt32)input.GetWidth(), "Offset is invalid");
  NS_ASSERT_DEV(vOffset.y < (nsInt32)input.GetHeight(), "Offset is invalid");

  const nsUInt32 uiNewWidth = nsMath::Min(vOffset.x + newsize.width, input.GetWidth()) - vOffset.x;
  const nsUInt32 uiNewHeight = nsMath::Min(vOffset.y + newsize.height, input.GetHeight()) - vOffset.y;

  nsImageHeader outputHeader;
  outputHeader.SetWidth(uiNewWidth);
  outputHeader.SetHeight(uiNewHeight);
  outputHeader.SetImageFormat(input.GetImageFormat());
  out_output.ResetAndAlloc(outputHeader);

  for (nsUInt32 y = 0; y < uiNewHeight; ++y)
  {
    for (nsUInt32 x = 0; x < uiNewWidth; ++x)
    {
      switch (input.GetImageFormat())
      {
        case nsImageFormat::R8G8B8A8_UNORM:
        case nsImageFormat::R8G8B8A8_UNORM_SRGB:
        case nsImageFormat::R8G8B8A8_UINT:
        case nsImageFormat::R8G8B8A8_SNORM:
        case nsImageFormat::R8G8B8A8_SINT:
        case nsImageFormat::B8G8R8A8_UNORM:
        case nsImageFormat::B8G8R8X8_UNORM:
        case nsImageFormat::B8G8R8A8_UNORM_SRGB:
        case nsImageFormat::B8G8R8X8_UNORM_SRGB:
          out_output.GetPixelPointer<nsUInt32>(0, 0, 0, x, y)[0] = input.GetPixelPointer<nsUInt32>(0, 0, 0, vOffset.x + x, vOffset.y + y)[0];
          break;

        case nsImageFormat::B8G8R8_UNORM:
          out_output.GetPixelPointer<nsUInt8>(0, 0, 0, x, y)[0] = input.GetPixelPointer<nsUInt8>(0, 0, 0, vOffset.x + x, vOffset.y + y)[0];
          out_output.GetPixelPointer<nsUInt8>(0, 0, 0, x, y)[1] = input.GetPixelPointer<nsUInt8>(0, 0, 0, vOffset.x + x, vOffset.y + y)[1];
          out_output.GetPixelPointer<nsUInt8>(0, 0, 0, x, y)[2] = input.GetPixelPointer<nsUInt8>(0, 0, 0, vOffset.x + x, vOffset.y + y)[2];
          break;

        default:
          NS_REPORT_FAILURE("The nsImageFormat {0} is not implemented", (nsUInt32)input.GetImageFormat());
          return;
      }
    }
  }
}

namespace
{
  template <typename T>
  void rotate180(T* pStart, T* pEnd)
  {
    pEnd = pEnd - 1;
    while (pStart < pEnd)
    {
      nsMath::Swap(*pStart, *pEnd);
      pStart++;
      pEnd--;
    }
  }
} // namespace

void nsImageUtils::RotateSubImage180(nsImage& inout_image, nsUInt32 uiMipLevel /*= 0*/, nsUInt32 uiFace /*= 0*/, nsUInt32 uiArrayIndex /*= 0*/)
{
  NS_PROFILE_SCOPE("nsImageUtils::RotateSubImage180");

  nsUInt8* start = inout_image.GetPixelPointer<nsUInt8>(uiMipLevel, uiFace, uiArrayIndex);
  nsUInt8* end = start + inout_image.GetDepthPitch(uiMipLevel);

  nsUInt32 bytesPerPixel = nsImageFormat::GetBitsPerPixel(inout_image.GetImageFormat()) / 8;

  switch (bytesPerPixel)
  {
    case 4:
      rotate180<nsUInt32>(reinterpret_cast<nsUInt32*>(start), reinterpret_cast<nsUInt32*>(end));
      break;
    case 12:
      rotate180<nsVec3>(reinterpret_cast<nsVec3*>(start), reinterpret_cast<nsVec3*>(end));
      break;
    case 16:
      rotate180<nsVec4>(reinterpret_cast<nsVec4*>(start), reinterpret_cast<nsVec4*>(end));
      break;
    default:
      // fallback version
      {
        end -= bytesPerPixel;
        while (start < end)
        {
          for (nsUInt32 i = 0; i < bytesPerPixel; i++)
          {
            nsMath::Swap(start[i], end[i]);
          }
          start += bytesPerPixel;
          end -= bytesPerPixel;
        }
      }
  }
}

nsResult nsImageUtils::Copy(const nsImageView& srcImg, const nsRectU32& srcRect, nsImage& inout_dstImg, const nsVec3U32& vDstOffset, nsUInt32 uiDstMipLevel /*= 0*/, nsUInt32 uiDstFace /*= 0*/, nsUInt32 uiDstArrayIndex /*= 0*/)
{
  if (inout_dstImg.GetImageFormat() != srcImg.GetImageFormat())   // Can only copy when the image formats are identical
    return NS_FAILURE;

  if (nsImageFormat::IsCompressed(inout_dstImg.GetImageFormat())) // Compressed formats are not supported
    return NS_FAILURE;

  NS_PROFILE_SCOPE("nsImageUtils::Copy");

  const nsUInt64 uiDstRowPitch = inout_dstImg.GetRowPitch(uiDstMipLevel);
  const nsUInt64 uiSrcRowPitch = srcImg.GetRowPitch(uiDstMipLevel);
  const nsUInt32 uiCopyBytesPerRow = nsImageFormat::GetBitsPerPixel(srcImg.GetImageFormat()) * srcRect.width / 8;

  nsUInt8* dstPtr = inout_dstImg.GetPixelPointer<nsUInt8>(uiDstMipLevel, uiDstFace, uiDstArrayIndex, vDstOffset.x, vDstOffset.y, vDstOffset.z);
  const nsUInt8* srcPtr = srcImg.GetPixelPointer<nsUInt8>(0, 0, 0, srcRect.x, srcRect.y);

  for (nsUInt32 y = 0; y < srcRect.height; y++)
  {
    nsMemoryUtils::Copy(dstPtr, srcPtr, uiCopyBytesPerRow);

    dstPtr += uiDstRowPitch;
    srcPtr += uiSrcRowPitch;
  }

  return NS_SUCCESS;
}

nsResult nsImageUtils::ExtractLowerMipChain(const nsImageView& srcImg, nsImage& ref_dstImg, nsUInt32 uiNumMips)
{
  const nsImageHeader& srcImgHeader = srcImg.GetHeader();

  if (srcImgHeader.GetNumFaces() != 1 || srcImgHeader.GetNumArrayIndices() != 1)
  {
    // Lower mips aren't stored contiguously for array/cube textures and would require copying. This isn't implemented yet.
    return NS_FAILURE;
  }

  NS_PROFILE_SCOPE("nsImageUtils::ExtractLowerMipChain");

  uiNumMips = nsMath::Min(uiNumMips, srcImgHeader.GetNumMipLevels());

  nsUInt32 startMipLevel = srcImgHeader.GetNumMipLevels() - uiNumMips;

  nsImageFormat::Enum format = srcImgHeader.GetImageFormat();

  if (nsImageFormat::RequiresFirstLevelBlockAlignment(format))
  {
    // Some block compressed image formats require resolutions that are divisible by block size,
    // therefore adjust startMipLevel accordingly
    while (srcImgHeader.GetWidth(startMipLevel) % nsImageFormat::GetBlockWidth(format) != 0 || srcImgHeader.GetHeight(startMipLevel) % nsImageFormat::GetBlockHeight(format) != 0)
    {
      if (uiNumMips >= srcImgHeader.GetNumMipLevels())
        return NS_FAILURE;

      if (startMipLevel == 0)
        return NS_FAILURE;

      ++uiNumMips;
      --startMipLevel;
    }
  }

  nsImageHeader dstImgHeader = srcImgHeader;
  dstImgHeader.SetWidth(srcImgHeader.GetWidth(startMipLevel));
  dstImgHeader.SetHeight(srcImgHeader.GetHeight(startMipLevel));
  dstImgHeader.SetDepth(srcImgHeader.GetDepth(startMipLevel));
  dstImgHeader.SetNumFaces(srcImgHeader.GetNumFaces());
  dstImgHeader.SetNumArrayIndices(srcImgHeader.GetNumArrayIndices());
  dstImgHeader.SetNumMipLevels(uiNumMips);

  const nsUInt8* pDataBegin = srcImg.GetPixelPointer<nsUInt8>(startMipLevel);
  const nsUInt8* pDataEnd = srcImg.GetByteBlobPtr().GetEndPtr();
  const ptrdiff_t dataSize = reinterpret_cast<ptrdiff_t>(pDataEnd) - reinterpret_cast<ptrdiff_t>(pDataBegin);

  const nsConstByteBlobPtr lowResData(pDataBegin, static_cast<nsUInt64>(dataSize));

  nsImageView dataview;
  dataview.ResetAndViewExternalStorage(dstImgHeader, lowResData);

  ref_dstImg.ResetAndCopy(dataview);

  return NS_SUCCESS;
}

nsUInt32 nsImageUtils::GetSampleIndex(nsUInt32 uiNumTexels, nsInt32 iIndex, nsImageAddressMode::Enum addressMode, bool& out_bUseBorderColor)
{
  out_bUseBorderColor = false;
  if (nsUInt32(iIndex) >= uiNumTexels)
  {
    switch (addressMode)
    {
      case nsImageAddressMode::Repeat:
        iIndex %= uiNumTexels;

        if (iIndex < 0)
        {
          iIndex += uiNumTexels;
        }
        return iIndex;

      case nsImageAddressMode::Mirror:
      {
        if (iIndex < 0)
        {
          iIndex = -iIndex - 1;
        }
        bool flip = (iIndex / uiNumTexels) & 1;
        iIndex %= uiNumTexels;
        if (flip)
        {
          iIndex = uiNumTexels - iIndex - 1;
        }
        return iIndex;
      }

      case nsImageAddressMode::Clamp:
        return nsMath::Clamp<nsInt32>(iIndex, 0, uiNumTexels - 1);

      case nsImageAddressMode::ClampBorder:
        out_bUseBorderColor = true;
        return 0;

      default:
        NS_ASSERT_NOT_IMPLEMENTED
        return 0;
    }
  }
  return iIndex;
}

static nsSimdVec4f LoadSample(const nsSimdVec4f* pSource, nsUInt32 uiNumSourceElements, nsUInt32 uiStride, nsInt32 iIndex, nsImageAddressMode::Enum addressMode, const nsSimdVec4f& vBorderColor)
{
  bool useBorderColor = false;
  // result is in the range [-(w-1), (w-1)], bring it to [0, w - 1]
  iIndex = nsImageUtils::GetSampleIndex(uiNumSourceElements, iIndex, addressMode, useBorderColor);
  if (useBorderColor)
  {
    return vBorderColor;
  }
  return pSource[iIndex * uiStride];
}

inline static void FilterLine(
  nsUInt32 uiNumSourceElements, const nsSimdVec4f* __restrict pSourceBegin, nsSimdVec4f* __restrict pTargetBegin, nsUInt32 uiStride, const nsImageFilterWeights& weights, nsArrayPtr<const nsInt32> firstSampleIndices, nsImageAddressMode::Enum addressMode, const nsSimdVec4f& vBorderColor)
{
  // Convolve the image using the precomputed weights
  const nsUInt32 numWeights = weights.GetNumWeights();

  // When the first source index for the output is between 0 and this value,
  // we can fetch all numWeights inputs without taking addressMode into consideration,
  // which makes the inner loop a lot faster.
  const nsInt32 trivialSourceIndicesEnd = static_cast<nsInt32>(uiNumSourceElements) - static_cast<nsInt32>(numWeights);
  const auto weightsView = weights.ViewWeights();
  const float* __restrict nextWeightPtr = weightsView.GetPtr();
  NS_ASSERT_DEBUG((static_cast<nsUInt32>(weightsView.GetCount()) % numWeights) == 0, "");
  for (nsInt32 firstSourceIdx : firstSampleIndices)
  {
    nsSimdVec4f total(0.0f, 0.0f, 0.0f, 0.0f);

    if (firstSourceIdx >= 0 && firstSourceIdx < trivialSourceIndicesEnd)
    {
      const auto* __restrict sourcePtr = pSourceBegin + firstSourceIdx * uiStride;
      for (nsUInt32 weightIdx = 0; weightIdx < numWeights; ++weightIdx)
      {
        total = nsSimdVec4f::MulAdd(*sourcePtr, nsSimdVec4f(*nextWeightPtr++), total);
        sourcePtr += uiStride;
      }
    }
    else
    {
      // Very slow fallback case that respects the addressMode
      // (not a lot of pixels are taking this path, so it's probably fine)
      nsInt32 sourceIdx = firstSourceIdx;
      for (nsUInt32 weightIdx = 0; weightIdx < numWeights; ++weightIdx)
      {
        total = nsSimdVec4f::MulAdd(LoadSample(pSourceBegin, uiNumSourceElements, uiStride, sourceIdx, addressMode, vBorderColor), nsSimdVec4f(*nextWeightPtr++), total);
        sourceIdx++;
      }
    }
    // It's ok to check this once per source index, see the assert above
    // (number of weights in weightsView is divisible by numWeights)
    if (nextWeightPtr == weightsView.GetEndPtr())
    {
      nextWeightPtr = weightsView.GetPtr();
    }
    *pTargetBegin = total;
    pTargetBegin += uiStride;
  }
}

static void DownScaleFastLine(nsUInt32 uiPixelStride, const nsUInt8* pSrc, nsUInt8* pDest, nsUInt32 uiLengthIn, nsUInt32 uiStrideIn, nsUInt32 uiLengthOut, nsUInt32 uiStrideOut)
{
  const nsUInt32 downScaleFactor = uiLengthIn / uiLengthOut;
  NS_ASSERT_DEBUG(downScaleFactor >= 1, "Can't upscale");

  const nsUInt32 downScaleFactorLog2 = nsMath::Log2i(static_cast<nsUInt32>(downScaleFactor));
  const nsUInt32 roundOffset = downScaleFactor / 2;

  for (nsUInt32 offset = 0; offset < uiLengthOut; ++offset)
  {
    for (nsUInt32 channel = 0; channel < uiPixelStride; ++channel)
    {
      const nsUInt32 destOffset = offset * uiStrideOut + channel;

      nsUInt32 curChannel = roundOffset;
      for (nsUInt32 index = 0; index < downScaleFactor; ++index)
      {
        curChannel += static_cast<nsUInt32>(pSrc[channel + index * uiStrideIn]);
      }

      curChannel = curChannel >> downScaleFactorLog2;
      pDest[destOffset] = static_cast<nsUInt8>(curChannel);
    }

    pSrc += downScaleFactor * uiStrideIn;
  }
}

static void DownScaleFast(const nsImageView& image, nsImage& out_result, nsUInt32 uiWidth, nsUInt32 uiHeight)
{
  nsImageFormat::Enum format = image.GetImageFormat();

  nsUInt32 originalWidth = image.GetWidth();
  nsUInt32 originalHeight = image.GetHeight();
  nsUInt32 numArrayElements = image.GetNumArrayIndices();
  nsUInt32 numFaces = image.GetNumFaces();

  nsUInt32 pixelStride = nsImageFormat::GetBitsPerPixel(format) / 8;

  nsImageHeader intermediateHeader;
  intermediateHeader.SetWidth(uiWidth);
  intermediateHeader.SetHeight(originalHeight);
  intermediateHeader.SetNumArrayIndices(numArrayElements);
  intermediateHeader.SetNumFaces(numFaces);
  intermediateHeader.SetImageFormat(format);

  nsImage intermediate;
  intermediate.ResetAndAlloc(intermediateHeader);

  for (nsUInt32 arrayIndex = 0; arrayIndex < numArrayElements; arrayIndex++)
  {
    for (nsUInt32 face = 0; face < numFaces; face++)
    {
      for (nsUInt32 row = 0; row < originalHeight; row++)
      {
        DownScaleFastLine(pixelStride, image.GetPixelPointer<nsUInt8>(0, face, arrayIndex, 0, row), intermediate.GetPixelPointer<nsUInt8>(0, face, arrayIndex, 0, row), originalWidth, pixelStride, uiWidth, pixelStride);
      }
    }
  }

  // input and output images may be the same, so we can't access the original image below this point

  nsImageHeader outHeader;
  outHeader.SetWidth(uiWidth);
  outHeader.SetHeight(uiHeight);
  outHeader.SetNumArrayIndices(numArrayElements);
  outHeader.SetNumArrayIndices(numFaces);
  outHeader.SetImageFormat(format);

  out_result.ResetAndAlloc(outHeader);

  NS_ASSERT_DEBUG(intermediate.GetRowPitch() < nsMath::MaxValue<nsUInt32>(), "Row pitch exceeds nsUInt32 max value.");
  NS_ASSERT_DEBUG(out_result.GetRowPitch() < nsMath::MaxValue<nsUInt32>(), "Row pitch exceeds nsUInt32 max value.");

  for (nsUInt32 arrayIndex = 0; arrayIndex < numArrayElements; arrayIndex++)
  {
    for (nsUInt32 face = 0; face < numFaces; face++)
    {
      for (nsUInt32 col = 0; col < uiWidth; col++)
      {
        DownScaleFastLine(pixelStride, intermediate.GetPixelPointer<nsUInt8>(0, face, arrayIndex, col), out_result.GetPixelPointer<nsUInt8>(0, face, arrayIndex, col), originalHeight, static_cast<nsUInt32>(intermediate.GetRowPitch()), uiHeight, static_cast<nsUInt32>(out_result.GetRowPitch()));
      }
    }
  }
}

static float EvaluateAverageCoverage(nsBlobPtr<const nsColor> colors, float fAlphaThreshold)
{
  NS_PROFILE_SCOPE("EvaluateAverageCoverage");

  nsUInt64 totalPixels = colors.GetCount();
  nsUInt64 count = 0;
  for (nsUInt32 idx = 0; idx < totalPixels; ++idx)
  {
    count += colors[idx].a >= fAlphaThreshold;
  }

  return float(count) / float(totalPixels);
}

static void NormalizeCoverage(nsBlobPtr<nsColor> colors, float fAlphaThreshold, float fTargetCoverage)
{
  NS_PROFILE_SCOPE("NormalizeCoverage");

  // Based on the idea in http://the-witness.net/news/2010/09/computing-alpha-mipmaps/. Note we're using a histogram
  // to find the new alpha threshold here rather than bisecting.

  // Generate histogram of alpha values
  nsUInt64 totalPixels = colors.GetCount();
  nsUInt32 alphaHistogram[256] = {};
  for (nsUInt64 idx = 0; idx < totalPixels; ++idx)
  {
    alphaHistogram[nsMath::ColorFloatToByte(colors[idx].a)]++;
  }

  // Find range of alpha thresholds so the number of covered pixels matches by summing up the histogram
  nsInt32 targetCount = nsInt32(fTargetCoverage * totalPixels);
  nsInt32 coverageCount = 0;
  nsInt32 maxThreshold = 255;
  for (; maxThreshold >= 0; maxThreshold--)
  {
    coverageCount += alphaHistogram[maxThreshold];

    if (coverageCount >= targetCount)
    {
      break;
    }
  }

  coverageCount = targetCount;
  nsInt32 minThreshold = 0;
  for (; minThreshold < 256; minThreshold++)
  {
    coverageCount -= alphaHistogram[maxThreshold];

    if (coverageCount <= targetCount)
    {
      break;
    }
  }

  nsInt32 currentThreshold = nsMath::ColorFloatToByte(fAlphaThreshold);

  // Each of the alpha test thresholds in the range [minThreshold; maxThreshold] will result in the same coverage. Pick a new threshold
  // close to the old one so we scale by the smallest necessary amount.
  nsInt32 newThreshold;
  if (currentThreshold < minThreshold)
  {
    newThreshold = minThreshold;
  }
  else if (currentThreshold > maxThreshold)
  {
    newThreshold = maxThreshold;
  }
  else
  {
    // Avoid rescaling altogether if the current threshold already preserves coverage
    return;
  }

  // Rescale alpha values
  float alphaScale = fAlphaThreshold / (newThreshold / 255.0f);
  for (nsUInt64 idx = 0; idx < totalPixels; ++idx)
  {
    colors[idx].a *= alphaScale;
  }
}


nsResult nsImageUtils::Scale(const nsImageView& source, nsImage& ref_target, nsUInt32 uiWidth, nsUInt32 uiHeight, const nsImageFilter* pFilter, nsImageAddressMode::Enum addressModeU, nsImageAddressMode::Enum addressModeV, const nsColor& borderColor)
{
  return Scale3D(source, ref_target, uiWidth, uiHeight, 1, pFilter, addressModeU, addressModeV, nsImageAddressMode::Clamp, borderColor);
}

nsResult nsImageUtils::Scale3D(const nsImageView& source, nsImage& ref_target, nsUInt32 uiWidth, nsUInt32 uiHeight, nsUInt32 uiDepth, const nsImageFilter* pFilter /*= ns_NULL*/, nsImageAddressMode::Enum addressModeU /*= nsImageAddressMode::Clamp*/,
  nsImageAddressMode::Enum addressModeV /*= nsImageAddressMode::Clamp*/, nsImageAddressMode::Enum addressModeW /*= nsImageAddressMode::Clamp*/, const nsColor& borderColor /*= nsColors::Black*/)
{
  NS_PROFILE_SCOPE("nsImageUtils::Scale3D");

  if (uiWidth == 0 || uiHeight == 0 || uiDepth == 0)
  {
    nsImageHeader header;
    header.SetImageFormat(source.GetImageFormat());
    ref_target.ResetAndAlloc(header);
    return NS_SUCCESS;
  }

  const nsImageFormat::Enum format = source.GetImageFormat();

  const nsUInt32 originalWidth = source.GetWidth();
  const nsUInt32 originalHeight = source.GetHeight();
  const nsUInt32 originalDepth = source.GetDepth();
  const nsUInt32 numFaces = source.GetNumFaces();
  const nsUInt32 numArrayElements = source.GetNumArrayIndices();

  if (originalWidth == uiWidth && originalHeight == uiHeight && originalDepth == uiDepth)
  {
    ref_target.ResetAndCopy(source);
    return NS_SUCCESS;
  }

  // Scaling down by an even factor?
  const nsUInt32 downScaleFactorX = originalWidth / uiWidth;
  const nsUInt32 downScaleFactorY = originalHeight / uiHeight;

  if (pFilter == nullptr && (format == nsImageFormat::R8G8B8A8_UNORM || format == nsImageFormat::B8G8R8A8_UNORM || format == nsImageFormat::B8G8R8_UNORM) && downScaleFactorX * uiWidth == originalWidth && downScaleFactorY * uiHeight == originalHeight && uiDepth == 1 && originalDepth == 1 &&
      nsMath::IsPowerOf2(downScaleFactorX) && nsMath::IsPowerOf2(downScaleFactorY))
  {
    DownScaleFast(source, ref_target, uiWidth, uiHeight);
    return NS_SUCCESS;
  }

  // Fallback to default filter
  nsImageFilterTriangle defaultFilter;
  if (!pFilter)
  {
    pFilter = &defaultFilter;
  }

  const nsImageView* stepSource;

  // Manage scratch images for intermediate conversion or filtering
  const nsUInt32 maxNumScratchImages = 2;
  nsImage scratch[maxNumScratchImages];
  bool scratchUsed[maxNumScratchImages] = {};
  auto allocateScratch = [&]() -> nsImage&
  {
    for (nsUInt32 i = 0;; ++i)
    {
      NS_ASSERT_DEV(i < maxNumScratchImages, "Failed to allocate scratch image");
      if (!scratchUsed[i])
      {
        scratchUsed[i] = true;
        return scratch[i];
      }
    }
  };
  auto releaseScratch = [&](const nsImageView& image)
  {
    for (nsUInt32 i = 0; i < maxNumScratchImages; ++i)
    {
      if (&scratch[i] == &image)
      {
        scratchUsed[i] = false;
        return;
      }
    }
  };

  if (format == nsImageFormat::R32G32B32A32_FLOAT)
  {
    stepSource = &source;
  }
  else
  {
    nsImage& conversionScratch = allocateScratch();
    if (nsImageConversion::Convert(source, conversionScratch, nsImageFormat::R32G32B32A32_FLOAT).Failed())
    {
      return NS_FAILURE;
    }

    stepSource = &conversionScratch;
  };

  nsHybridArray<nsInt32, 256> firstSampleIndices;
  firstSampleIndices.Reserve(nsMath::Max(uiWidth, uiHeight, uiDepth));

  if (uiWidth != originalWidth)
  {
    nsImageFilterWeights weights(*pFilter, originalWidth, uiWidth);
    firstSampleIndices.SetCountUninitialized(uiWidth);
    for (nsUInt32 x = 0; x < uiWidth; ++x)
    {
      firstSampleIndices[x] = weights.GetFirstSourceSampleIndex(x);
    }

    nsImage* stepTarget;
    if (uiHeight == originalHeight && uiDepth == originalDepth && format == nsImageFormat::R32G32B32A32_FLOAT)
    {
      stepTarget = &ref_target;
    }
    else
    {
      stepTarget = &allocateScratch();
    }

    nsImageHeader stepHeader = stepSource->GetHeader();
    stepHeader.SetWidth(uiWidth);
    stepTarget->ResetAndAlloc(stepHeader);

    for (nsUInt32 arrayIndex = 0; arrayIndex < numArrayElements; ++arrayIndex)
    {
      for (nsUInt32 face = 0; face < numFaces; ++face)
      {
        for (nsUInt32 z = 0; z < originalDepth; ++z)
        {
          for (nsUInt32 y = 0; y < originalHeight; ++y)
          {
            const nsSimdVec4f* filterSource = stepSource->GetPixelPointer<nsSimdVec4f>(0, face, arrayIndex, 0, y, z);
            nsSimdVec4f* filterTarget = stepTarget->GetPixelPointer<nsSimdVec4f>(0, face, arrayIndex, 0, y, z);
            FilterLine(originalWidth, filterSource, filterTarget, 1, weights, firstSampleIndices, addressModeU, nsSimdVec4f(borderColor.r, borderColor.g, borderColor.b, borderColor.a));
          }
        }
      }
    }

    releaseScratch(*stepSource);
    stepSource = stepTarget;
  }

  if (uiHeight != originalHeight)
  {
    nsImageFilterWeights weights(*pFilter, originalHeight, uiHeight);
    firstSampleIndices.SetCount(uiHeight);
    for (nsUInt32 y = 0; y < uiHeight; ++y)
    {
      firstSampleIndices[y] = weights.GetFirstSourceSampleIndex(y);
    }

    nsImage* stepTarget;
    if (uiDepth == originalDepth && format == nsImageFormat::R32G32B32A32_FLOAT)
    {
      stepTarget = &ref_target;
    }
    else
    {
      stepTarget = &allocateScratch();
    }

    nsImageHeader stepHeader = stepSource->GetHeader();
    stepHeader.SetHeight(uiHeight);
    stepTarget->ResetAndAlloc(stepHeader);

    for (nsUInt32 arrayIndex = 0; arrayIndex < numArrayElements; ++arrayIndex)
    {
      for (nsUInt32 face = 0; face < numFaces; ++face)
      {
        for (nsUInt32 z = 0; z < originalDepth; ++z)
        {
          for (nsUInt32 x = 0; x < uiWidth; ++x)
          {
            const nsSimdVec4f* filterSource = stepSource->GetPixelPointer<nsSimdVec4f>(0, face, arrayIndex, x, 0, z);
            nsSimdVec4f* filterTarget = stepTarget->GetPixelPointer<nsSimdVec4f>(0, face, arrayIndex, x, 0, z);
            FilterLine(originalHeight, filterSource, filterTarget, uiWidth, weights, firstSampleIndices, addressModeV, nsSimdVec4f(borderColor.r, borderColor.g, borderColor.b, borderColor.a));
          }
        }
      }
    }

    releaseScratch(*stepSource);
    stepSource = stepTarget;
  }

  if (uiDepth != originalDepth)
  {
    nsImageFilterWeights weights(*pFilter, originalDepth, uiDepth);
    firstSampleIndices.SetCount(uiDepth);
    for (nsUInt32 z = 0; z < uiDepth; ++z)
    {
      firstSampleIndices[z] = weights.GetFirstSourceSampleIndex(z);
    }

    nsImage* stepTarget;
    if (format == nsImageFormat::R32G32B32A32_FLOAT)
    {
      stepTarget = &ref_target;
    }
    else
    {
      stepTarget = &allocateScratch();
    }

    nsImageHeader stepHeader = stepSource->GetHeader();
    stepHeader.SetDepth(uiDepth);
    stepTarget->ResetAndAlloc(stepHeader);

    for (nsUInt32 arrayIndex = 0; arrayIndex < numArrayElements; ++arrayIndex)
    {
      for (nsUInt32 face = 0; face < numFaces; ++face)
      {
        for (nsUInt32 y = 0; y < uiHeight; ++y)
        {
          for (nsUInt32 x = 0; x < uiWidth; ++x)
          {
            const nsSimdVec4f* filterSource = stepSource->GetPixelPointer<nsSimdVec4f>(0, face, arrayIndex, x, y, 0);
            nsSimdVec4f* filterTarget = stepTarget->GetPixelPointer<nsSimdVec4f>(0, face, arrayIndex, x, y, 0);
            FilterLine(originalHeight, filterSource, filterTarget, uiWidth * uiHeight, weights, firstSampleIndices, addressModeW, nsSimdVec4f(borderColor.r, borderColor.g, borderColor.b, borderColor.a));
          }
        }
      }
    }

    releaseScratch(*stepSource);
    stepSource = stepTarget;
  }

  // Convert back to original format - no-op if stepSource and target are the same
  return nsImageConversion::Convert(*stepSource, ref_target, format);
}

void nsImageUtils::GenerateMipMaps(const nsImageView& source, nsImage& ref_target, const MipMapOptions& options)
{
  NS_PROFILE_SCOPE("nsImageUtils::GenerateMipMaps");

  nsImageHeader header = source.GetHeader();
  NS_ASSERT_DEV(header.GetImageFormat() == nsImageFormat::R32G32B32A32_FLOAT, "The source image must be a RGBA 32-bit float format.");
  NS_ASSERT_DEV(&source != &ref_target, "Source and target must not be the same image.");

  // Make a local copy to be able to tweak some of the options
  nsImageUtils::MipMapOptions mipMapOptions = options;

  // alpha thresholds with extreme values are not supported at the moment
  mipMapOptions.m_alphaThreshold = nsMath::Clamp(mipMapOptions.m_alphaThreshold, 0.05f, 0.95f);

  // Enforce CLAMP addressing mode for cubemaps
  if (source.GetNumFaces() == 6)
  {
    mipMapOptions.m_addressModeU = nsImageAddressMode::Clamp;
    mipMapOptions.m_addressModeV = nsImageAddressMode::Clamp;
  }

  nsUInt32 numMipMaps = header.ComputeNumberOfMipMaps();
  if (mipMapOptions.m_numMipMaps > 0 && mipMapOptions.m_numMipMaps < numMipMaps)
  {
    numMipMaps = mipMapOptions.m_numMipMaps;
  }
  header.SetNumMipLevels(numMipMaps);

  ref_target.ResetAndAlloc(header);

  for (nsUInt32 arrayIndex = 0; arrayIndex < source.GetNumArrayIndices(); arrayIndex++)
  {
    for (nsUInt32 face = 0; face < source.GetNumFaces(); face++)
    {
      nsImageHeader currentMipMapHeader = header;
      currentMipMapHeader.SetNumMipLevels(1);
      currentMipMapHeader.SetNumFaces(1);
      currentMipMapHeader.SetNumArrayIndices(1);

      auto sourceView = source.GetSubImageView(0, face, arrayIndex).GetByteBlobPtr();
      auto targetView = ref_target.GetSubImageView(0, face, arrayIndex).GetByteBlobPtr();

      memcpy(targetView.GetPtr(), sourceView.GetPtr(), static_cast<size_t>(targetView.GetCount()));

      float targetCoverage = 0.0f;
      if (mipMapOptions.m_preserveCoverage)
      {
        targetCoverage = EvaluateAverageCoverage(source.GetSubImageView(0, face, arrayIndex).GetBlobPtr<nsColor>(), mipMapOptions.m_alphaThreshold);
      }

      for (nsUInt32 mipMapLevel = 0; mipMapLevel < numMipMaps - 1; mipMapLevel++)
      {
        nsImageHeader nextMipMapHeader = currentMipMapHeader;
        nextMipMapHeader.SetWidth(nsMath::Max(1u, nextMipMapHeader.GetWidth() / 2));
        nextMipMapHeader.SetHeight(nsMath::Max(1u, nextMipMapHeader.GetHeight() / 2));
        nextMipMapHeader.SetDepth(nsMath::Max(1u, nextMipMapHeader.GetDepth() / 2));

        auto sourceData = ref_target.GetSubImageView(mipMapLevel, face, arrayIndex).GetByteBlobPtr();
        nsImage currentMipMap;
        currentMipMap.ResetAndUseExternalStorage(currentMipMapHeader, sourceData);

        auto dstData = ref_target.GetSubImageView(mipMapLevel + 1, face, arrayIndex).GetByteBlobPtr();
        nsImage nextMipMap;
        nextMipMap.ResetAndUseExternalStorage(nextMipMapHeader, dstData);

        nsImageUtils::Scale3D(currentMipMap, nextMipMap, nextMipMapHeader.GetWidth(), nextMipMapHeader.GetHeight(), nextMipMapHeader.GetDepth(), mipMapOptions.m_filter, mipMapOptions.m_addressModeU, mipMapOptions.m_addressModeV, mipMapOptions.m_addressModeW, mipMapOptions.m_borderColor)
          .IgnoreResult();

        if (mipMapOptions.m_preserveCoverage)
        {
          NormalizeCoverage(nextMipMap.GetBlobPtr<nsColor>(), mipMapOptions.m_alphaThreshold, targetCoverage);
        }

        if (mipMapOptions.m_renormalizeNormals)
        {
          RenormalizeNormalMap(nextMipMap);
        }

        currentMipMapHeader = nextMipMapHeader;
      }
    }
  }
}

void nsImageUtils::ReconstructNormalZ(nsImage& ref_image)
{
  NS_PROFILE_SCOPE("nsImageUtils::ReconstructNormalZ");

  NS_ASSERT_DEV(ref_image.GetImageFormat() == nsImageFormat::R32G32B32A32_FLOAT, "This algorithm currently expects a RGBA 32 Float as input");

  nsSimdVec4f* cur = ref_image.GetBlobPtr<nsSimdVec4f>().GetPtr();
  nsSimdVec4f* const end = ref_image.GetBlobPtr<nsSimdVec4f>().GetEndPtr();

  nsSimdFloat oneScalar = 1.0f;

  nsSimdVec4f two(2.0f);

  nsSimdVec4f minusOne(-1.0f);

  nsSimdVec4f half(0.5f);

  for (; cur < end; cur++)
  {
    nsSimdVec4f normal;
    // unpack from [0,1] to [-1, 1]
    normal = nsSimdVec4f::MulAdd(*cur, two, minusOne);

    // compute Z component
    normal.SetZ((oneScalar - normal.Dot<2>(normal)).GetSqrt());

    // pack back to [0,1]
    *cur = nsSimdVec4f::MulAdd(half, normal, half);
  }
}

void nsImageUtils::RenormalizeNormalMap(nsImage& ref_image)
{
  NS_PROFILE_SCOPE("nsImageUtils::RenormalizeNormalMap");

  NS_ASSERT_DEV(ref_image.GetImageFormat() == nsImageFormat::R32G32B32A32_FLOAT, "This algorithm currently expects a RGBA 32 Float as input");

  nsSimdVec4f* start = ref_image.GetBlobPtr<nsSimdVec4f>().GetPtr();
  nsSimdVec4f* const end = ref_image.GetBlobPtr<nsSimdVec4f>().GetEndPtr();

  nsSimdVec4f two(2.0f);

  nsSimdVec4f minusOne(-1.0f);

  nsSimdVec4f half(0.5f);

  for (; start < end; start++)
  {
    nsSimdVec4f normal;
    normal = nsSimdVec4f::MulAdd(*start, two, minusOne);
    normal.Normalize<3>();
    *start = nsSimdVec4f::MulAdd(half, normal, half);
  }
}

void nsImageUtils::AdjustRoughness(nsImage& ref_roughnessMap, const nsImageView& normalMap)
{
  NS_PROFILE_SCOPE("nsImageUtils::AdjustRoughness");

  NS_ASSERT_DEV(ref_roughnessMap.GetImageFormat() == nsImageFormat::R32G32B32A32_FLOAT, "This algorithm currently expects a RGBA 32 Float as input");
  NS_ASSERT_DEV(normalMap.GetImageFormat() == nsImageFormat::R32G32B32A32_FLOAT, "This algorithm currently expects a RGBA 32 Float as input");

  NS_ASSERT_DEV(ref_roughnessMap.GetWidth() >= normalMap.GetWidth() && ref_roughnessMap.GetHeight() >= normalMap.GetHeight(), "The roughness map needs to be bigger or same size than the normal map.");

  nsImage filteredNormalMap;
  nsImageUtils::MipMapOptions options;

  // Box filter normal map without re-normalization so we have the average normal length in each mip map.
  if (ref_roughnessMap.GetWidth() != normalMap.GetWidth() || ref_roughnessMap.GetHeight() != normalMap.GetHeight())
  {
    nsImage temp;
    nsImageUtils::Scale(normalMap, temp, ref_roughnessMap.GetWidth(), ref_roughnessMap.GetHeight()).IgnoreResult();
    nsImageUtils::RenormalizeNormalMap(temp);
    nsImageUtils::GenerateMipMaps(temp, filteredNormalMap, options);
  }
  else
  {
    nsImageUtils::GenerateMipMaps(normalMap, filteredNormalMap, options);
  }

  NS_ASSERT_DEV(ref_roughnessMap.GetNumMipLevels() == filteredNormalMap.GetNumMipLevels(), "Roughness and normal map must have the same number of mip maps");

  nsSimdVec4f two(2.0f);
  nsSimdVec4f minusOne(-1.0f);

  nsUInt32 numMipLevels = ref_roughnessMap.GetNumMipLevels();
  for (nsUInt32 mipLevel = 1; mipLevel < numMipLevels; ++mipLevel)
  {
    nsBlobPtr<nsSimdVec4f> roughnessData = ref_roughnessMap.GetSubImageView(mipLevel, 0, 0).GetBlobPtr<nsSimdVec4f>();
    nsBlobPtr<nsSimdVec4f> normalData = filteredNormalMap.GetSubImageView(mipLevel, 0, 0).GetBlobPtr<nsSimdVec4f>();

    for (nsUInt64 i = 0; i < roughnessData.GetCount(); ++i)
    {
      nsSimdVec4f normal = nsSimdVec4f::MulAdd(normalData[i], two, minusOne);

      float avgNormalLength = normal.GetLength<3>();
      if (avgNormalLength < 1.0f)
      {
        float avgNormalLengthSquare = avgNormalLength * avgNormalLength;
        float kappa = (3.0f * avgNormalLength - avgNormalLength * avgNormalLengthSquare) / (1.0f - avgNormalLengthSquare);
        float variance = 1.0f / (2.0f * kappa);

        float oldRoughness = roughnessData[i].GetComponent<0>();
        float newRoughness = nsMath::Sqrt(oldRoughness * oldRoughness + variance);

        roughnessData[i].Set(newRoughness);
      }
    }
  }
}

void nsImageUtils::ChangeExposure(nsImage& ref_image, float fBias)
{
  NS_ASSERT_DEV(ref_image.GetImageFormat() == nsImageFormat::R32G32B32A32_FLOAT, "This function expects an RGBA 32 float image as input");

  if (fBias == 0.0f)
    return;

  NS_PROFILE_SCOPE("nsImageUtils::ChangeExposure");

  const float multiplier = nsMath::Pow2(fBias);

  for (nsColor& col : ref_image.GetBlobPtr<nsColor>())
  {
    col = multiplier * col;
  }
}

static nsResult CopyImageRectToFace(nsImage& ref_dstImg, const nsImageView& srcImg, nsUInt32 uiOffsetX, nsUInt32 uiOffsetY, nsUInt32 uiFaceIndex)
{
  nsRectU32 r;
  r.x = uiOffsetX;
  r.y = uiOffsetY;
  r.width = ref_dstImg.GetWidth();
  r.height = r.width;

  return nsImageUtils::Copy(srcImg, r, ref_dstImg, nsVec3U32(0), 0, uiFaceIndex);
}

nsResult nsImageUtils::CreateCubemapFromSingleFile(nsImage& ref_dstImg, const nsImageView& srcImg)
{
  NS_PROFILE_SCOPE("nsImageUtils::CreateCubemapFromSingleFile");

  if (srcImg.GetNumFaces() == 6)
  {
    ref_dstImg.ResetAndCopy(srcImg);
    return NS_SUCCESS;
  }
  else if (srcImg.GetNumFaces() == 1)
  {
    if (srcImg.GetWidth() % 3 == 0 && srcImg.GetHeight() % 4 == 0 && srcImg.GetWidth() / 3 == srcImg.GetHeight() / 4)
    {
      // Vertical cube map layout
      //     +---+
      //     | Y+|
      // +---+---+---+
      // | X-| Z+| X+|
      // +---+---+---+
      //     | Y-|
      //     +---+
      //     | Z-|
      //     +---+
      const nsUInt32 faceSize = srcImg.GetWidth() / 3;

      nsImageHeader imgHeader;
      imgHeader.SetWidth(faceSize);
      imgHeader.SetHeight(faceSize);
      imgHeader.SetImageFormat(srcImg.GetImageFormat());
      imgHeader.SetDepth(1);
      imgHeader.SetNumFaces(6);
      imgHeader.SetNumMipLevels(1);
      imgHeader.SetNumArrayIndices(1);

      ref_dstImg.ResetAndAlloc(imgHeader);

      // face order in dds files is: positive x, negative x, positive y, negative y, positive z, negative z

      // Positive X face
      NS_SUCCEED_OR_RETURN(CopyImageRectToFace(ref_dstImg, srcImg, faceSize * 2, faceSize, 0));

      // Negative X face
      NS_SUCCEED_OR_RETURN(CopyImageRectToFace(ref_dstImg, srcImg, 0, faceSize, 1));

      // Positive Y face
      NS_SUCCEED_OR_RETURN(CopyImageRectToFace(ref_dstImg, srcImg, faceSize, 0, 2));

      // Negative Y face
      NS_SUCCEED_OR_RETURN(CopyImageRectToFace(ref_dstImg, srcImg, faceSize, faceSize * 2, 3));

      // Positive Z face
      NS_SUCCEED_OR_RETURN(CopyImageRectToFace(ref_dstImg, srcImg, faceSize, faceSize, 4));

      // Negative Z face
      NS_SUCCEED_OR_RETURN(CopyImageRectToFace(ref_dstImg, srcImg, faceSize, faceSize * 3, 5));
      nsImageUtils::RotateSubImage180(ref_dstImg, 0, 5);
    }
    else if (srcImg.GetWidth() % 4 == 0 && srcImg.GetHeight() % 3 == 0 && srcImg.GetWidth() / 4 == srcImg.GetHeight() / 3)
    {
      // Horizontal cube map layout
      //     +---+
      //     | Y+|
      // +---+---+---+---+
      // | X-| Z+| X+| Z-|
      // +---+---+---+---+
      //     | Y-|
      //     +---+
      const nsUInt32 faceSize = srcImg.GetWidth() / 4;

      nsImageHeader imgHeader;
      imgHeader.SetWidth(faceSize);
      imgHeader.SetHeight(faceSize);
      imgHeader.SetImageFormat(srcImg.GetImageFormat());
      imgHeader.SetDepth(1);
      imgHeader.SetNumFaces(6);
      imgHeader.SetNumMipLevels(1);
      imgHeader.SetNumArrayIndices(1);

      ref_dstImg.ResetAndAlloc(imgHeader);

      // face order in dds files is: positive x, negative x, positive y, negative y, positive z, negative z

      // Positive X face
      NS_SUCCEED_OR_RETURN(CopyImageRectToFace(ref_dstImg, srcImg, faceSize * 2, faceSize, 0));

      // Negative X face
      NS_SUCCEED_OR_RETURN(CopyImageRectToFace(ref_dstImg, srcImg, 0, faceSize, 1));

      // Positive Y face
      NS_SUCCEED_OR_RETURN(CopyImageRectToFace(ref_dstImg, srcImg, faceSize, 0, 2));

      // Negative Y face
      NS_SUCCEED_OR_RETURN(CopyImageRectToFace(ref_dstImg, srcImg, faceSize, faceSize * 2, 3));

      // Positive Z face
      NS_SUCCEED_OR_RETURN(CopyImageRectToFace(ref_dstImg, srcImg, faceSize, faceSize, 4));

      // Negative Z face
      NS_SUCCEED_OR_RETURN(CopyImageRectToFace(ref_dstImg, srcImg, faceSize * 3, faceSize, 5));
    }
    else
    {
      // Spherical mapping
      if (srcImg.GetWidth() % 4 != 0)
      {
        nsLog::Error("Width of the input image should be a multiple of 4");
        return NS_FAILURE;
      }

      const nsUInt32 faceSize = srcImg.GetWidth() / 4;

      nsImageHeader imgHeader;
      imgHeader.SetWidth(faceSize);
      imgHeader.SetHeight(faceSize);
      imgHeader.SetImageFormat(srcImg.GetImageFormat());
      imgHeader.SetDepth(1);
      imgHeader.SetNumFaces(6);
      imgHeader.SetNumMipLevels(1);
      imgHeader.SetNumArrayIndices(1);

      ref_dstImg.ResetAndAlloc(imgHeader);

      // Corners of the UV space for the respective faces in model space
      const nsVec3 faceCorners[] = {
        nsVec3(0.5, 0.5, 0.5),   // X+
        nsVec3(-0.5, 0.5, -0.5), // X-
        nsVec3(-0.5, 0.5, -0.5), // Y+
        nsVec3(-0.5, -0.5, 0.5), // Y-
        nsVec3(-0.5, 0.5, 0.5),  // Z+
        nsVec3(0.5, 0.5, -0.5)   // Z-
      };

      // UV Axis of the respective faces in model space
      const nsVec3 faceAxis[] = {
        nsVec3(0, 0, -1), nsVec3(0, -1, 0), // X+
        nsVec3(0, 0, 1), nsVec3(0, -1, 0),  // X-
        nsVec3(1, 0, 0), nsVec3(0, 0, 1),   // Y+
        nsVec3(1, 0, 0), nsVec3(0, 0, -1),  // Y-
        nsVec3(1, 0, 0), nsVec3(0, -1, 0),  // Z+
        nsVec3(-1, 0, 0), nsVec3(0, -1, 0)  // Z-
      };

      const float fFaceSize = (float)faceSize;
      const float fHalfPixel = 0.5f / fFaceSize;
      const float fPixel = 1.0f / fFaceSize;

      const float fHalfSrcWidth = srcImg.GetWidth() / 2.0f;
      const float fSrcHeight = (float)srcImg.GetHeight();

      const nsUInt32 srcWidthMinus1 = srcImg.GetWidth() - 1;
      const nsUInt32 srcHeightMinus1 = srcImg.GetHeight() - 1;

      NS_ASSERT_DEBUG(srcImg.GetRowPitch() % sizeof(nsColor) == 0, "Row pitch should be a multiple of sizeof(nsColor)");
      const nsUInt64 srcRowPitch = srcImg.GetRowPitch() / sizeof(nsColor);

      NS_ASSERT_DEBUG(ref_dstImg.GetRowPitch() % sizeof(nsColor) == 0, "Row pitch should be a multiple of sizeof(nsColor)");
      const nsUInt64 faceRowPitch = ref_dstImg.GetRowPitch() / sizeof(nsColor);

      const nsColor* srcData = srcImg.GetPixelPointer<nsColor>();
      const float InvPi = 1.0f / nsMath::Pi<float>();

      for (nsUInt32 faceIndex = 0; faceIndex < 6; faceIndex++)
      {
        nsColor* faceData = ref_dstImg.GetPixelPointer<nsColor>(0, faceIndex);
        for (nsUInt32 y = 0; y < faceSize; y++)
        {
          const float dstV = (float)y * fPixel + fHalfPixel;

          for (nsUInt32 x = 0; x < faceSize; x++)
          {
            const float dstU = (float)x * fPixel + fHalfPixel;
            const nsVec3 modelSpacePos = faceCorners[faceIndex] + dstU * faceAxis[faceIndex * 2] + dstV * faceAxis[faceIndex * 2 + 1];
            const nsVec3 modelSpaceDir = modelSpacePos.GetNormalized();

            const float phi = nsMath::ATan2(modelSpaceDir.x, modelSpaceDir.z).GetRadian() + nsMath::Pi<float>();
            const float r = nsMath::Sqrt(modelSpaceDir.x * modelSpaceDir.x + modelSpaceDir.z * modelSpaceDir.z);
            const float theta = nsMath::ATan2(modelSpaceDir.y, r).GetRadian() + nsMath::Pi<float>() * 0.5f;

            NS_ASSERT_DEBUG(phi >= 0.0f && phi <= 2.0f * nsMath::Pi<float>(), "");
            NS_ASSERT_DEBUG(theta >= 0.0f && theta <= nsMath::Pi<float>(), "");

            const float srcU = phi * InvPi * fHalfSrcWidth;
            const float srcV = (1.0f - theta * InvPi) * fSrcHeight;

            nsUInt32 x1 = (nsUInt32)nsMath::Floor(srcU);
            nsUInt32 x2 = x1 + 1;
            nsUInt32 y1 = (nsUInt32)nsMath::Floor(srcV);
            nsUInt32 y2 = y1 + 1;

            const float fracX = srcU - x1;
            const float fracY = srcV - y1;

            x1 = nsMath::Clamp(x1, 0u, srcWidthMinus1);
            x2 = nsMath::Clamp(x2, 0u, srcWidthMinus1);
            y1 = nsMath::Clamp(y1, 0u, srcHeightMinus1);
            y2 = nsMath::Clamp(y2, 0u, srcHeightMinus1);

            nsColor A = srcData[x1 + y1 * srcRowPitch];
            nsColor B = srcData[x2 + y1 * srcRowPitch];
            nsColor C = srcData[x1 + y2 * srcRowPitch];
            nsColor D = srcData[x2 + y2 * srcRowPitch];

            nsColor interpolated = A * (1 - fracX) * (1 - fracY) + B * (fracX) * (1 - fracY) + C * (1 - fracX) * fracY + D * fracX * fracY;
            faceData[x + y * faceRowPitch] = interpolated;
          }
        }
      }
    }

    return NS_SUCCESS;
  }

  nsLog::Error("Unexpected number of faces in cubemap input image.");
  return NS_FAILURE;
}

nsResult nsImageUtils::CreateCubemapFrom6Files(nsImage& ref_dstImg, const nsImageView* pSourceImages)
{
  NS_PROFILE_SCOPE("nsImageUtils::CreateCubemapFrom6Files");

  nsImageHeader header = pSourceImages[0].GetHeader();
  header.SetNumFaces(6);

  if (header.GetWidth() != header.GetHeight())
    return NS_FAILURE;

  if (!nsMath::IsPowerOf2(header.GetWidth()))
    return NS_FAILURE;

  ref_dstImg.ResetAndAlloc(header);

  for (nsUInt32 i = 0; i < 6; ++i)
  {
    if (pSourceImages[i].GetImageFormat() != ref_dstImg.GetImageFormat())
      return NS_FAILURE;

    if (pSourceImages[i].GetWidth() != ref_dstImg.GetWidth())
      return NS_FAILURE;

    if (pSourceImages[i].GetHeight() != ref_dstImg.GetHeight())
      return NS_FAILURE;

    NS_SUCCEED_OR_RETURN(CopyImageRectToFace(ref_dstImg, pSourceImages[i], 0, 0, i));
  }

  return NS_SUCCESS;
}

nsResult nsImageUtils::CreateVolumeTextureFromSingleFile(nsImage& ref_dstImg, const nsImageView& srcImg)
{
  NS_PROFILE_SCOPE("nsImageUtils::CreateVolumeTextureFromSingleFile");

  const nsUInt32 uiWidthHeight = srcImg.GetHeight();
  const nsUInt32 uiDepth = srcImg.GetWidth() / uiWidthHeight;

  if (!nsMath::IsPowerOf2(uiWidthHeight))
    return NS_FAILURE;
  if (!nsMath::IsPowerOf2(uiDepth))
    return NS_FAILURE;

  nsImageHeader header;
  header.SetWidth(uiWidthHeight);
  header.SetHeight(uiWidthHeight);
  header.SetDepth(uiDepth);
  header.SetImageFormat(srcImg.GetImageFormat());

  ref_dstImg.ResetAndAlloc(header);

  const nsImageView view = srcImg.GetSubImageView();

  for (nsUInt32 d = 0; d < uiDepth; ++d)
  {
    nsRectU32 r;
    r.x = uiWidthHeight * d;
    r.y = 0;
    r.width = uiWidthHeight;
    r.height = uiWidthHeight;

    NS_SUCCEED_OR_RETURN(Copy(view, r, ref_dstImg, nsVec3U32(0, 0, d)));
  }

  return NS_SUCCESS;
}

nsColor nsImageUtils::NearestSample(const nsImageView& image, nsImageAddressMode::Enum addressMode, nsVec2 vUv)
{
  NS_ASSERT_DEBUG(image.GetDepth() == 1 && image.GetNumFaces() == 1 && image.GetNumArrayIndices() == 1, "Only 2d images are supported");
  NS_ASSERT_DEBUG(image.GetImageFormat() == nsImageFormat::R32G32B32A32_FLOAT, "Unsupported format");

  return NearestSample(image.GetPixelPointer<nsColor>(), image.GetWidth(), image.GetHeight(), addressMode, vUv);
}

nsColor nsImageUtils::NearestSample(const nsColor* pPixelPointer, nsUInt32 uiWidth, nsUInt32 uiHeight, nsImageAddressMode::Enum addressMode, nsVec2 vUv)
{
  const nsInt32 w = uiWidth;
  const nsInt32 h = uiHeight;

  vUv = vUv.CompMul(nsVec2(static_cast<float>(w), static_cast<float>(h)));
  const nsInt32 intX = (nsInt32)nsMath::Floor(vUv.x);
  const nsInt32 intY = (nsInt32)nsMath::Floor(vUv.y);

  nsInt32 x = intX;
  nsInt32 y = intY;

  if (addressMode == nsImageAddressMode::Clamp)
  {
    x = nsMath::Clamp(x, 0, w - 1);
    y = nsMath::Clamp(y, 0, h - 1);
  }
  else if (addressMode == nsImageAddressMode::Repeat)
  {
    x = x % w;
    x = x < 0 ? x + w : x;
    y = y % h;
    y = y < 0 ? y + h : y;
  }
  else
  {
    NS_ASSERT_NOT_IMPLEMENTED;
  }

  return *(pPixelPointer + (y * w) + x);
}

nsColor nsImageUtils::BilinearSample(const nsImageView& image, nsImageAddressMode::Enum addressMode, nsVec2 vUv)
{
  NS_ASSERT_DEBUG(image.GetDepth() == 1 && image.GetNumFaces() == 1 && image.GetNumArrayIndices() == 1, "Only 2d images are supported");
  NS_ASSERT_DEBUG(image.GetImageFormat() == nsImageFormat::R32G32B32A32_FLOAT, "Unsupported format");

  return BilinearSample(image.GetPixelPointer<nsColor>(), image.GetWidth(), image.GetHeight(), addressMode, vUv);
}

nsColor nsImageUtils::BilinearSample(const nsColor* pData, nsUInt32 uiWidth, nsUInt32 uiHeight, nsImageAddressMode::Enum addressMode, nsVec2 vUv)
{
  nsInt32 w = uiWidth;
  nsInt32 h = uiHeight;

  vUv = vUv.CompMul(nsVec2(static_cast<float>(w), static_cast<float>(h))) - nsVec2(0.5f);
  const float floorX = nsMath::Floor(vUv.x);
  const float floorY = nsMath::Floor(vUv.y);
  const float fractionX = vUv.x - floorX;
  const float fractionY = vUv.y - floorY;
  const nsInt32 intX = (nsInt32)floorX;
  const nsInt32 intY = (nsInt32)floorY;

  nsColor c[4];
  for (nsUInt32 i = 0; i < 4; ++i)
  {
    nsInt32 x = intX + (i % 2);
    nsInt32 y = intY + (i / 2);

    if (addressMode == nsImageAddressMode::Clamp)
    {
      x = nsMath::Clamp(x, 0, w - 1);
      y = nsMath::Clamp(y, 0, h - 1);
    }
    else if (addressMode == nsImageAddressMode::Repeat)
    {
      x = x % w;
      x = x < 0 ? x + w : x;
      y = y % h;
      y = y < 0 ? y + h : y;
    }
    else
    {
      NS_ASSERT_NOT_IMPLEMENTED;
    }

    c[i] = *(pData + (y * w) + x);
  }

  const nsColor cr0 = nsMath::Lerp(c[0], c[1], fractionX);
  const nsColor cr1 = nsMath::Lerp(c[2], c[3], fractionX);

  return nsMath::Lerp(cr0, cr1, fractionY);
}

nsResult nsImageUtils::CopyChannel(nsImage& ref_dstImg, nsUInt8 uiDstChannelIdx, const nsImage& srcImg, nsUInt8 uiSrcChannelIdx)
{
  NS_PROFILE_SCOPE("nsImageUtils::CopyChannel");

  if (uiSrcChannelIdx >= 4 || uiDstChannelIdx >= 4)
    return NS_FAILURE;

  if (ref_dstImg.GetImageFormat() != nsImageFormat::R32G32B32A32_FLOAT)
    return NS_FAILURE;

  if (srcImg.GetImageFormat() != ref_dstImg.GetImageFormat())
    return NS_FAILURE;

  if (srcImg.GetWidth() != ref_dstImg.GetWidth())
    return NS_FAILURE;

  if (srcImg.GetHeight() != ref_dstImg.GetHeight())
    return NS_FAILURE;

  const nsUInt32 uiNumPixels = srcImg.GetWidth() * srcImg.GetHeight();
  const float* pSrcPixel = srcImg.GetPixelPointer<float>();
  float* pDstPixel = ref_dstImg.GetPixelPointer<float>();

  pSrcPixel += uiSrcChannelIdx;
  pDstPixel += uiDstChannelIdx;

  for (nsUInt32 i = 0; i < uiNumPixels; ++i)
  {
    *pDstPixel = *pSrcPixel;

    pSrcPixel += 4;
    pDstPixel += 4;
  }

  return NS_SUCCESS;
}

static const nsUInt8 s_Base64EncodingTable[64] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
  'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

static const nsUInt8 BASE64_CHARS_PER_LINE = 76;

static nsUInt32 GetBase64EncodedLength(nsUInt32 uiInputLength, bool bInsertLineBreaks)
{
  nsUInt32 outputLength = (uiInputLength + 2) / 3 * 4;

  if (bInsertLineBreaks)
  {
    outputLength += outputLength / BASE64_CHARS_PER_LINE;
  }

  return outputLength;
}

static nsDynamicArray<char> ArrayToBase64(nsArrayPtr<const nsUInt8> in, bool bInsertLineBreaks = true)
{
  nsDynamicArray<char> out;
  out.SetCountUninitialized(GetBase64EncodedLength(in.GetCount(), bInsertLineBreaks));

  nsUInt32 offsetIn = 0;
  nsUInt32 offsetOut = 0;

  nsUInt32 blocksTillNewline = BASE64_CHARS_PER_LINE / 4;
  while (offsetIn < in.GetCount())
  {
    nsUInt8 ibuf[3] = {0};

    nsUInt32 ibuflen = nsMath::Min(in.GetCount() - offsetIn, 3u);

    for (nsUInt32 i = 0; i < ibuflen; ++i)
    {
      ibuf[i] = in[offsetIn++];
    }

    char obuf[4];
    obuf[0] = s_Base64EncodingTable[(ibuf[0] >> 2)];
    obuf[1] = s_Base64EncodingTable[((ibuf[0] << 4) & 0x30) | (ibuf[1] >> 4)];
    obuf[2] = s_Base64EncodingTable[((ibuf[1] << 2) & 0x3c) | (ibuf[2] >> 6)];
    obuf[3] = s_Base64EncodingTable[(ibuf[2] & 0x3f)];

    if (ibuflen >= 3)
    {
      out[offsetOut++] = obuf[0];
      out[offsetOut++] = obuf[1];
      out[offsetOut++] = obuf[2];
      out[offsetOut++] = obuf[3];
    }
    else // need to pad up to 4
    {
      switch (ibuflen)
      {
        case 1:
          out[offsetOut++] = obuf[0];
          out[offsetOut++] = obuf[1];
          out[offsetOut++] = '=';
          out[offsetOut++] = '=';
          break;
        case 2:
          out[offsetOut++] = obuf[0];
          out[offsetOut++] = obuf[1];
          out[offsetOut++] = obuf[2];
          out[offsetOut++] = '=';
          break;
      }
    }

    if (--blocksTillNewline == 0)
    {
      if (bInsertLineBreaks)
      {
        out[offsetOut++] = '\n';
      }
      blocksTillNewline = 19;
    }
  }

  NS_ASSERT_DEV(offsetOut == out.GetCount(), "All output data should have been written");
  return out;
}

void nsImageUtils::EmbedImageData(nsStringBuilder& out_sHtml, const nsImage& image)
{
  nsImageFileFormat* format = nsImageFileFormat::GetWriterFormat("png");
  NS_ASSERT_DEV(format != nullptr, "No PNG writer found");

  nsDynamicArray<nsUInt8> imgData;
  nsMemoryStreamContainerWrapperStorage<nsDynamicArray<nsUInt8>> storage(&imgData);
  nsMemoryStreamWriter writer(&storage);
  format->WriteImage(writer, image, "png").IgnoreResult();

  nsDynamicArray<char> imgDataBase64 = ArrayToBase64(imgData.GetArrayPtr());
  nsStringView imgDataBase64StringView(imgDataBase64.GetArrayPtr().GetPtr(), imgDataBase64.GetArrayPtr().GetEndPtr());
  out_sHtml.AppendFormat("data:image/png;base64,{0}", imgDataBase64StringView);
}

void nsImageUtils::CreateImageDiffHtml(nsStringBuilder& out_sHtml, nsStringView sTitle, const nsImage& referenceImgRgb, const nsImage& referenceImgAlpha, const nsImage& capturedImgRgb, const nsImage& capturedImgAlpha, const nsImage& diffImgRgb, const nsImage& diffImgAlpha, nsUInt32 uiError, nsUInt32 uiThreshold, nsUInt8 uiMinDiffRgb, nsUInt8 uiMaxDiffRgb, nsUInt8 uiMinDiffAlpha, nsUInt8 uiMaxDiffAlpha)
{
  nsStringBuilder& output = out_sHtml;
  output.Append("<!DOCTYPE html PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n"
                "<!DOCTYPE html PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n"
                "<HTML> <HEAD>\n");

  output.AppendFormat("<TITLE>{}</TITLE>\n", sTitle);
  output.Append("<script type = \"text/javascript\">\n"
                "function showReferenceImage()\n"
                "{\n"
                "    document.getElementById('image_current_rgb').style.display = 'none'\n"
                "    document.getElementById('image_current_a').style.display = 'none'\n"
                "    document.getElementById('image_reference_rgb').style.display = 'inline-block'\n"
                "    document.getElementById('image_reference_a').style.display = 'inline-block'\n"
                "    document.getElementById('image_caption_rgb').innerHTML = 'Displaying: Reference Image RGB'\n"
                "    document.getElementById('image_caption_a').innerHTML = 'Displaying: Reference Image Alpha'\n"
                "}\n"
                "function showCurrentImage()\n"
                "{\n"
                "    document.getElementById('image_current_rgb').style.display = 'inline-block'\n"
                "    document.getElementById('image_current_a').style.display = 'inline-block'\n"
                "    document.getElementById('image_reference_rgb').style.display = 'none'\n"
                "    document.getElementById('image_reference_a').style.display = 'none'\n"
                "    document.getElementById('image_caption_rgb').innerHTML = 'Displaying: Current Image RGB'\n"
                "    document.getElementById('image_caption_a').innerHTML = 'Displaying: Current Image Alpha'\n"
                "}\n"
                "function imageover()\n"
                "{\n"
                "    var mode = document.querySelector('input[name=\"image_interaction_mode\"]:checked').value\n"
                "    if (mode == 'interactive')\n"
                "    {\n"
                "        showReferenceImage()\n"
                "    }\n"
                "}\n"
                "function imageout()\n"
                "{\n"
                "    var mode = document.querySelector('input[name=\"image_interaction_mode\"]:checked').value\n"
                "    if (mode == 'interactive')\n"
                "    {\n"
                "        showCurrentImage()\n"
                "    }\n"
                "}\n"
                "function handleModeClick(clickedItem)\n"
                "{\n"
                "    if (clickedItem.value == 'current_image' || clickedItem.value == 'interactive')\n"
                "    {\n"
                "        showCurrentImage()\n"
                "    }\n"
                "    else if (clickedItem.value == 'reference_image')\n"
                "    {\n"
                "        showReferenceImage()\n"
                "    }\n"
                "}\n"
                "</script>\n"
                "</HEAD>\n"
                "<BODY bgcolor=\"#ccdddd\">\n"
                "<div style=\"line-height: 1.5; margin-top: 0px; margin-left: 10px; font-family: sans-serif;\">\n");

  output.AppendFormat("<b>Test result for \"{}\" from ", sTitle);
  nsDateTime dateTime = nsDateTime::MakeFromTimestamp(nsTimestamp::CurrentTimestamp());
  output.AppendFormat("{}-{}-{} {}:{}:{}</b><br>\n", dateTime.GetYear(), nsArgI(dateTime.GetMonth(), 2, true), nsArgI(dateTime.GetDay(), 2, true), nsArgI(dateTime.GetHour(), 2, true), nsArgI(dateTime.GetMinute(), 2, true), nsArgI(dateTime.GetSecond(), 2, true));

  output.Append("<table cellpadding=\"0\" cellspacing=\"0\" border=\"0\">\n");

  output.Append("<!-- STATS-TABLE-START -->\n");

  output.AppendFormat("<tr>\n"
                      "<td>Error metric:</td>\n"
                      "<td align=\"right\" style=\"padding-left: 2em;\">{}</td>\n"
                      "</tr>\n",
    uiError);
  output.AppendFormat("<tr>\n"
                      "<td>Error threshold:</td>\n"
                      "<td align=\"right\" style=\"padding-left: 2em;\">{}</td>\n"
                      "</tr>\n",
    uiThreshold);

  output.Append("<!-- STATS-TABLE-END -->\n");

  output.Append("</table>\n"
                "<div style=\"margin-top: 0.5em; margin-bottom: -0.75em\">\n"
                "    <input type=\"radio\" name=\"image_interaction_mode\" onclick=\"handleModeClick(this)\" value=\"interactive\" "
                "checked=\"checked\"> Mouse-Over Image Switching\n"
                "    <input type=\"radio\" name=\"image_interaction_mode\" onclick=\"handleModeClick(this)\" value=\"current_image\"> "
                "Current Image\n"
                "    <input type=\"radio\" name=\"image_interaction_mode\" onclick=\"handleModeClick(this)\" value=\"reference_image\"> "
                "Reference Image\n"
                "</div>\n");

  output.AppendFormat("<div style=\"width:{}px;display: inline-block;\">\n", capturedImgRgb.GetWidth());

  output.Append("<p id=\"image_caption_rgb\">Displaying: Current Image RGB</p>\n"

                "<div style=\"block;\" onmouseover=\"imageover()\" onmouseout=\"imageout()\">\n"
                "<img id=\"image_current_rgb\" alt=\"Captured Image RGB\" src=\"");
  EmbedImageData(output, capturedImgRgb);
  output.Append("\" />\n"
                "<img id=\"image_reference_rgb\" style=\"display: none\" alt=\"Reference Image RGB\" src=\"");
  EmbedImageData(output, referenceImgRgb);
  output.Append("\" />\n"
                "</div>\n"
                "<div style=\"display: block;\">\n");
  output.AppendFormat("<p>RGB Difference (min: {}, max: {}):</p>\n", uiMinDiffRgb, uiMaxDiffRgb);
  output.Append("<img alt=\"Diff Image RGB\" src=\"");
  EmbedImageData(output, diffImgRgb);
  output.Append("\" />\n"
                "</div>\n"
                "</div>\n");

  output.AppendFormat("<div style=\"width:{}px;display: inline-block;\">\n", capturedImgAlpha.GetWidth());

  output.Append("<p id=\"image_caption_a\">Displaying: Current Image Alpha</p>\n"
                "<div style=\"display: block;\" onmouseover=\"imageover()\" onmouseout=\"imageout()\">\n"
                "<img id=\"image_current_a\" alt=\"Captured Image Alpha\" src=\"");
  EmbedImageData(output, capturedImgAlpha);
  output.Append("\" />\n"
                "<img id=\"image_reference_a\" style=\"display: none\" alt=\"Reference Image Alpha\" src=\"");
  EmbedImageData(output, referenceImgAlpha);
  output.Append("\" />\n"
                "</div>\n"
                "<div style=\"px;display: block;\">\n");
  output.AppendFormat("<p>Alpha Difference (min: {}, max: {}):</p>\n", uiMinDiffAlpha, uiMaxDiffAlpha);
  output.Append("<img alt=\"Diff Image Alpha\" src=\"");
  EmbedImageData(output, diffImgAlpha);
  output.Append("\" />\n"
                "</div>\n"
                "</div>\n"
                "</div>\n"
                "</BODY> </HTML>");
}

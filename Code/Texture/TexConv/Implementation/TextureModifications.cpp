#include <Texture/TexturePCH.h>

#include <Foundation/Profiling/Profiling.h>
#include <Texture/Image/ImageUtils.h>
#include <Texture/TexConv/TexConvProcessor.h>

nsResult nsTexConvProcessor::ForceSRGBFormats()
{
  // if the output is going to be sRGB, assume the incoming RGB data is also already in sRGB
  if (m_Descriptor.m_Usage == nsTexConvUsage::Color)
  {
    for (const auto& mapping : m_Descriptor.m_ChannelMappings)
    {
      // do not enforce sRGB conversion for textures that are mapped to the alpha channel
      for (nsUInt32 i = 0; i < 3; ++i)
      {
        const nsInt32 iTex = mapping.m_Channel[i].m_iInputImageIndex;
        if (iTex != -1)
        {
          auto& img = m_Descriptor.m_InputImages[iTex];
          img.ReinterpretAs(nsImageFormat::AsSrgb(img.GetImageFormat()));
        }
      }
    }
  }

  return NS_SUCCESS;
}

nsResult nsTexConvProcessor::GenerateMipmaps(nsImage& img, nsUInt32 uiNumMips, MipmapChannelMode channelMode /*= MipmapChannelMode::AllChannels*/) const
{
  NS_PROFILE_SCOPE("GenerateMipmaps");

  nsImageUtils::MipMapOptions opt;
  opt.m_numMipMaps = uiNumMips;

  nsImageFilterBox filterLinear;
  nsImageFilterSincWithKaiserWindow filterKaiser;

  switch (m_Descriptor.m_MipmapMode)
  {
    case nsTexConvMipmapMode::None:
      return NS_SUCCESS;

    case nsTexConvMipmapMode::Linear:
      opt.m_filter = &filterLinear;
      break;

    case nsTexConvMipmapMode::Kaiser:
      opt.m_filter = &filterKaiser;
      break;
  }

  opt.m_addressModeU = m_Descriptor.m_AddressModeU;
  opt.m_addressModeV = m_Descriptor.m_AddressModeV;
  opt.m_addressModeW = m_Descriptor.m_AddressModeW;

  opt.m_preserveCoverage = m_Descriptor.m_bPreserveMipmapCoverage;
  opt.m_alphaThreshold = m_Descriptor.m_fMipmapAlphaThreshold;

  opt.m_renormalizeNormals = m_Descriptor.m_Usage == nsTexConvUsage::NormalMap || m_Descriptor.m_Usage == nsTexConvUsage::NormalMap_Inverted || m_Descriptor.m_Usage == nsTexConvUsage::BumpMap;

  // Copy red to alpha channel if we only have a single channel input texture
  if (opt.m_preserveCoverage && channelMode == MipmapChannelMode::SingleChannel)
  {
    auto imgData = img.GetBlobPtr<nsColor>();
    auto pData = imgData.GetPtr();
    while (pData < imgData.GetEndPtr())
    {
      pData->a = pData->r;
      ++pData;
    }
  }

  nsImage scratch;
  nsImageUtils::GenerateMipMaps(img, scratch, opt);
  img.ResetAndMove(std::move(scratch));

  if (img.GetNumMipLevels() <= 1)
  {
    nsLog::Error("Mipmap generation failed.");
    return NS_FAILURE;
  }

  // Copy alpha channel back to red
  if (opt.m_preserveCoverage && channelMode == MipmapChannelMode::SingleChannel)
  {
    auto imgData = img.GetBlobPtr<nsColor>();
    auto pData = imgData.GetPtr();
    while (pData < imgData.GetEndPtr())
    {
      pData->r = pData->a;
      ++pData;
    }
  }

  return NS_SUCCESS;
}

nsResult nsTexConvProcessor::PremultiplyAlpha(nsImage& image) const
{
  NS_PROFILE_SCOPE("PremultiplyAlpha");

  if (!m_Descriptor.m_bPremultiplyAlpha)
    return NS_SUCCESS;

  for (nsColor& col : image.GetBlobPtr<nsColor>())
  {
    col.r *= col.a;
    col.g *= col.a;
    col.b *= col.a;
  }

  return NS_SUCCESS;
}

nsResult nsTexConvProcessor::AdjustHdrExposure(nsImage& img) const
{
  NS_PROFILE_SCOPE("AdjustHdrExposure");

  nsImageUtils::ChangeExposure(img, m_Descriptor.m_fHdrExposureBias);
  return NS_SUCCESS;
}

nsResult nsTexConvProcessor::ConvertToNormalMap(nsArrayPtr<nsImage> imgs) const
{
  NS_PROFILE_SCOPE("ConvertToNormalMap");

  for (nsImage& img : imgs)
  {
    NS_SUCCEED_OR_RETURN(ConvertToNormalMap(img));
  }

  return NS_SUCCESS;
}

nsResult nsTexConvProcessor::ConvertToNormalMap(nsImage& bumpMap) const
{
  nsImageHeader newImageHeader = bumpMap.GetHeader();
  newImageHeader.SetNumMipLevels(1);
  nsImage newImage;
  newImage.ResetAndAlloc(newImageHeader);

  struct Accum
  {
    float x = 0.f;
    float y = 0.f;
  };
  nsDelegate<Accum(nsUInt32, nsUInt32)> filterKernel;

  // we'll assume that both the input bump map and the new image are using
  // RGBA 32 bit floating point as an internal format which should be tightly packed
  NS_ASSERT_DEV(bumpMap.GetImageFormat() == nsImageFormat::R32G32B32A32_FLOAT && bumpMap.GetRowPitch() % sizeof(nsColor) == 0, "");

  const nsColor* bumpPixels = bumpMap.GetPixelPointer<nsColor>(0, 0, 0, 0, 0, 0);
  const auto getBumpPixel = [&](nsUInt32 x, nsUInt32 y) -> float
  {
    const nsColor* ptr = bumpPixels + y * bumpMap.GetWidth() + x;
    return ptr->r;
  };

  nsColor* newPixels = newImage.GetPixelPointer<nsColor>(0, 0, 0, 0, 0, 0);
  auto getNewPixel = [&](nsUInt32 x, nsUInt32 y) -> nsColor&
  {
    nsColor* ptr = newPixels + y * newImage.GetWidth() + x;
    return *ptr;
  };

  switch (m_Descriptor.m_BumpMapFilter)
  {
    case nsTexConvBumpMapFilter::Finite:
      filterKernel = [&](nsUInt32 x, nsUInt32 y)
      {
        constexpr float linearKernel[3] = {-1, 0, 1};

        Accum accum;
        for (int i = -1; i <= 1; ++i)
        {
          const nsInt32 rx = nsMath::Clamp(i + static_cast<nsInt32>(x), 0, static_cast<nsInt32>(newImage.GetWidth()) - 1);
          const nsInt32 ry = nsMath::Clamp(i + static_cast<nsInt32>(y), 0, static_cast<nsInt32>(newImage.GetHeight()) - 1);

          const float depthX = getBumpPixel(rx, y);
          const float depthY = getBumpPixel(x, ry);

          accum.x += depthX * linearKernel[i + 1];
          accum.y += depthY * linearKernel[i + 1];
        }

        return accum;
      };
      break;
    case nsTexConvBumpMapFilter::Sobel:
      filterKernel = [&](nsUInt32 x, nsUInt32 y)
      {
        constexpr float kernel[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
        constexpr float weight = 1.f / 4.f;

        Accum accum;
        for (nsInt32 i = -1; i <= 1; ++i)
        {
          for (nsInt32 j = -1; j <= 1; ++j)
          {
            const nsInt32 rx = nsMath::Clamp(j + static_cast<nsInt32>(x), 0, static_cast<nsInt32>(newImage.GetWidth()) - 1);
            const nsInt32 ry = nsMath::Clamp(i + static_cast<nsInt32>(y), 0, static_cast<nsInt32>(newImage.GetHeight()) - 1);

            const float depth = getBumpPixel(rx, ry);

            accum.x += depth * kernel[i + 1][j + 1];
            accum.y += depth * kernel[j + 1][i + 1];
          }
        }

        accum.x *= weight;
        accum.y *= weight;

        return accum;
      };
      break;
    case nsTexConvBumpMapFilter::Scharr:
      filterKernel = [&](nsUInt32 x, nsUInt32 y)
      {
        constexpr float kernel[3][3] = {{-3, 0, 3}, {-10, 0, 10}, {-3, 0, 3}};
        constexpr float weight = 1.f / 16.f;

        Accum accum;
        for (nsInt32 i = -1; i <= 1; ++i)
        {
          for (nsInt32 j = -1; j <= 1; ++j)
          {
            const nsInt32 rx = nsMath::Clamp(j + static_cast<nsInt32>(x), 0, static_cast<nsInt32>(newImage.GetWidth()) - 1);
            const nsInt32 ry = nsMath::Clamp(i + static_cast<nsInt32>(y), 0, static_cast<nsInt32>(newImage.GetHeight()) - 1);

            const float depth = getBumpPixel(rx, ry);

            accum.x += depth * kernel[i + 1][j + 1];
            accum.y += depth * kernel[j + 1][i + 1];
          }
        }

        accum.x *= weight;
        accum.y *= weight;

        return accum;
      };
      break;
  };

  for (nsUInt32 y = 0; y < bumpMap.GetHeight(); ++y)
  {
    for (nsUInt32 x = 0; x < bumpMap.GetWidth(); ++x)
    {
      Accum accum = filterKernel(x, y);

      nsVec3 normal = nsVec3(1.f, 0.f, accum.x).CrossRH(nsVec3(0.f, 1.f, accum.y));
      normal.NormalizeIfNotZero(nsVec3(0, 0, 1), 0.001f).IgnoreResult();
      normal.y = -normal.y;

      normal = normal * 0.5f + nsVec3(0.5f);

      nsColor& newPixel = getNewPixel(x, y);
      newPixel.SetRGBA(normal.x, normal.y, normal.z, 0.f);
    }
  }

  bumpMap.ResetAndMove(std::move(newImage));

  return NS_SUCCESS;
}

nsResult nsTexConvProcessor::ClampInputValues(nsArrayPtr<nsImage> images, float maxValue) const
{
  for (nsImage& image : images)
  {
    NS_SUCCEED_OR_RETURN(ClampInputValues(image, maxValue));
  }

  return NS_SUCCESS;
}

nsResult nsTexConvProcessor::ClampInputValues(nsImage& image, float maxValue) const
{
  // we'll assume that at this point in the processing pipeline, the format is
  // RGBA32F which should result in tightly packed mipmaps.
  NS_ASSERT_DEV(image.GetImageFormat() == nsImageFormat::R32G32B32A32_FLOAT && image.GetRowPitch() % sizeof(float[4]) == 0, "");

  for (auto& value : image.GetBlobPtr<float>())
  {
    if (nsMath::IsNaN(value))
    {
      value = 0.f;
    }
    else
    {
      value = nsMath::Clamp(value, -maxValue, maxValue);
    }
  }

  return NS_SUCCESS;
}

static bool FillAvgImageColor(nsImage& ref_img)
{
  nsColor avg = nsColor::MakeZero();
  nsUInt32 uiValidCount = 0;

  for (const nsColor& col : ref_img.GetBlobPtr<nsColor>())
  {
    if (col.a > 0.0f)
    {
      avg += col;
      ++uiValidCount;
    }
  }

  if (uiValidCount == 0 || uiValidCount == ref_img.GetBlobPtr<nsColor>().GetCount())
  {
    // nothing to do
    return false;
  }

  avg /= static_cast<float>(uiValidCount);
  avg.NormalizeToLdrRange();
  avg.a = 0.0f;

  for (nsColor& col : ref_img.GetBlobPtr<nsColor>())
  {
    if (col.a == 0.0f)
    {
      col = avg;
    }
  }

  return true;
}

static void ClearAlpha(nsImage& ref_img, float fAlphaThreshold)
{
  for (nsColor& col : ref_img.GetBlobPtr<nsColor>())
  {
    if (col.a <= fAlphaThreshold)
    {
      col.a = 0.0f;
    }
  }
}

inline static nsColor GetPixelValue(const nsColor* pPixels, nsInt32 iWidth, nsInt32 x, nsInt32 y)
{
  return pPixels[y * iWidth + x];
}

inline static void SetPixelValue(nsColor* pPixels, nsInt32 iWidth, nsInt32 x, nsInt32 y, const nsColor& col)
{
  pPixels[y * iWidth + x] = col;
}

static nsColor GetAvgColor(nsColor* pPixels, nsInt32 iWidth, nsInt32 iHeight, nsInt32 x, nsInt32 y, float fMarkAlpha)
{
  nsColor colAt = GetPixelValue(pPixels, iWidth, x, y);

  if (colAt.a > 0)
    return colAt;

  nsColor avg = nsColor::MakeZero();
  nsUInt32 uiValidCount = 0;

  const nsInt32 iRadius = 1;

  for (nsInt32 cy = nsMath::Max<nsInt32>(0, y - iRadius); cy <= nsMath::Min<nsInt32>(y + iRadius, iHeight - 1); ++cy)
  {
    for (nsInt32 cx = nsMath::Max<nsInt32>(0, x - iRadius); cx <= nsMath::Min<nsInt32>(x + iRadius, iWidth - 1); ++cx)
    {
      const nsColor col = GetPixelValue(pPixels, iWidth, cx, cy);

      if (col.a > fMarkAlpha)
      {
        avg += col;
        ++uiValidCount;
      }
    }
  }

  if (uiValidCount == 0)
    return colAt;

  avg /= static_cast<float>(uiValidCount);
  avg.a = fMarkAlpha;

  return avg;
}

static void DilateColors(nsColor* pPixels, nsInt32 iWidth, nsInt32 iHeight, float fMarkAlpha)
{
  for (nsInt32 y = 0; y < iHeight; ++y)
  {
    for (nsInt32 x = 0; x < iWidth; ++x)
    {
      const nsColor avg = GetAvgColor(pPixels, iWidth, iHeight, x, y, fMarkAlpha);

      SetPixelValue(pPixels, iWidth, x, y, avg);
    }
  }
}

nsResult nsTexConvProcessor::DilateColor2D(nsImage& img) const
{
  if (m_Descriptor.m_uiDilateColor == 0)
    return NS_SUCCESS;

  NS_PROFILE_SCOPE("DilateColor2D");

  if (!FillAvgImageColor(img))
    return NS_SUCCESS;

  const nsUInt32 uiNumPasses = m_Descriptor.m_uiDilateColor;

  nsColor* pPixels = img.GetPixelPointer<nsColor>();
  const nsInt32 iWidth = static_cast<nsInt32>(img.GetWidth());
  const nsInt32 iHeight = static_cast<nsInt32>(img.GetHeight());

  for (nsUInt32 pass = uiNumPasses; pass > 0; --pass)
  {
    const float fAlphaThreshold = (static_cast<float>(pass) / uiNumPasses) / 256.0f; // between 0 and 1/256
    DilateColors(pPixels, iWidth, iHeight, fAlphaThreshold);
  }

  ClearAlpha(img, 1.0f / 256.0f);

  return NS_SUCCESS;
}

nsResult nsTexConvProcessor::InvertNormalMap(nsImage& image)
{
  if (m_Descriptor.m_Usage != nsTexConvUsage::NormalMap_Inverted)
    return NS_SUCCESS;

  // we'll assume that at this point in the processing pipeline, the format is
  // RGBA32F which should result in tightly packed mipmaps.
  NS_ASSERT_DEV(image.GetImageFormat() == nsImageFormat::R32G32B32A32_FLOAT && image.GetRowPitch() % sizeof(float[4]) == 0, "");

  for (auto& value : image.GetBlobPtr<nsColor>())
  {
    value.g = 1.0f - value.g;
  }

  return NS_SUCCESS;
}

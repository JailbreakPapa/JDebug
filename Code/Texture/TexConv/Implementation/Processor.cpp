#include <Texture/TexturePCH.h>

#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Texture/Image/ImageUtils.h>
#include <Texture/TexConv/TexConvProcessor.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_ENUM(nsTexConvCompressionMode, 1)
  NS_ENUM_CONSTANTS(nsTexConvCompressionMode::None, nsTexConvCompressionMode::Medium, nsTexConvCompressionMode::High)
NS_END_STATIC_REFLECTED_ENUM;

NS_BEGIN_STATIC_REFLECTED_ENUM(nsTexConvMipmapMode, 1)
  NS_ENUM_CONSTANTS(nsTexConvMipmapMode::None, nsTexConvMipmapMode::Linear, nsTexConvMipmapMode::Kaiser)
NS_END_STATIC_REFLECTED_ENUM;

NS_BEGIN_STATIC_REFLECTED_ENUM(nsTexConvUsage, 1)
  NS_ENUM_CONSTANT(nsTexConvUsage::Auto), NS_ENUM_CONSTANT(nsTexConvUsage::Color), NS_ENUM_CONSTANT(nsTexConvUsage::Linear),
  NS_ENUM_CONSTANT(nsTexConvUsage::Hdr), NS_ENUM_CONSTANT(nsTexConvUsage::NormalMap), NS_ENUM_CONSTANT(nsTexConvUsage::NormalMap_Inverted),
  NS_ENUM_CONSTANT(nsTexConvUsage::BumpMap),
NS_END_STATIC_REFLECTED_ENUM;
// clang-format on

nsTexConvProcessor::nsTexConvProcessor() = default;

nsResult nsTexConvProcessor::Process()
{
  NS_PROFILE_SCOPE("nsTexConvProcessor::Process");

  if (m_Descriptor.m_OutputType == nsTexConvOutputType::Atlas)
  {
    nsMemoryStreamWriter stream(&m_TextureAtlas);
    NS_SUCCEED_OR_RETURN(GenerateTextureAtlas(stream));
  }
  else
  {
    NS_SUCCEED_OR_RETURN(LoadInputImages());

    NS_SUCCEED_OR_RETURN(AdjustUsage(m_Descriptor.m_InputFiles[0], m_Descriptor.m_InputImages[0], m_Descriptor.m_Usage));

    nsStringBuilder sUsage;
    nsReflectionUtils::EnumerationToString(
      nsGetStaticRTTI<nsTexConvUsage>(), m_Descriptor.m_Usage.GetValue(), sUsage, nsReflectionUtils::EnumConversionMode::ValueNameOnly);
    nsLog::Info("-usage is '{}'", sUsage);

    NS_SUCCEED_OR_RETURN(ForceSRGBFormats());

    nsUInt32 uiNumChannelsUsed = 0;
    NS_SUCCEED_OR_RETURN(DetectNumChannels(m_Descriptor.m_ChannelMappings, uiNumChannelsUsed));

    nsEnum<nsImageFormat> OutputImageFormat;

    NS_SUCCEED_OR_RETURN(ChooseOutputFormat(OutputImageFormat, m_Descriptor.m_Usage, uiNumChannelsUsed));

    nsLog::Info("Output image format is '{}'", nsImageFormat::GetName(OutputImageFormat));

    nsUInt32 uiTargetResolutionX = 0;
    nsUInt32 uiTargetResolutionY = 0;

    NS_SUCCEED_OR_RETURN(DetermineTargetResolution(m_Descriptor.m_InputImages[0], OutputImageFormat, uiTargetResolutionX, uiTargetResolutionY));

    nsLog::Info("Target resolution is '{} x {}'", uiTargetResolutionX, uiTargetResolutionY);

    NS_SUCCEED_OR_RETURN(ConvertAndScaleInputImages(uiTargetResolutionX, uiTargetResolutionY, m_Descriptor.m_Usage));

    NS_SUCCEED_OR_RETURN(ClampInputValues(m_Descriptor.m_InputImages, m_Descriptor.m_fMaxValue));

    if (m_Descriptor.m_Usage == nsTexConvUsage::BumpMap)
    {
      NS_SUCCEED_OR_RETURN(ConvertToNormalMap(m_Descriptor.m_InputImages));
      m_Descriptor.m_Usage = nsTexConvUsage::NormalMap;
    }

    nsImage assembledImg;
    if (m_Descriptor.m_OutputType == nsTexConvOutputType::Texture2D || m_Descriptor.m_OutputType == nsTexConvOutputType::None)
    {
      NS_SUCCEED_OR_RETURN(Assemble2DTexture(m_Descriptor.m_InputImages[0].GetHeader(), assembledImg));

      NS_SUCCEED_OR_RETURN(InvertNormalMap(assembledImg));

      NS_SUCCEED_OR_RETURN(DilateColor2D(assembledImg));
    }
    else if (m_Descriptor.m_OutputType == nsTexConvOutputType::Cubemap)
    {
      NS_SUCCEED_OR_RETURN(AssembleCubemap(assembledImg));
    }
    else if (m_Descriptor.m_OutputType == nsTexConvOutputType::Volume)
    {
      NS_SUCCEED_OR_RETURN(Assemble3DTexture(assembledImg));
    }

    NS_SUCCEED_OR_RETURN(AdjustHdrExposure(assembledImg));

    NS_SUCCEED_OR_RETURN(GenerateMipmaps(assembledImg, 0, uiNumChannelsUsed == 1 ? MipmapChannelMode::SingleChannel : MipmapChannelMode::AllChannels));

    NS_SUCCEED_OR_RETURN(PremultiplyAlpha(assembledImg));

    NS_SUCCEED_OR_RETURN(GenerateOutput(std::move(assembledImg), m_OutputImage, OutputImageFormat));

    NS_SUCCEED_OR_RETURN(GenerateThumbnailOutput(m_OutputImage, m_ThumbnailOutputImage, m_Descriptor.m_uiThumbnailOutputResolution));

    NS_SUCCEED_OR_RETURN(GenerateLowResOutput(m_OutputImage, m_LowResOutputImage, m_Descriptor.m_uiLowResMipmaps));
  }

  return NS_SUCCESS;
}

nsResult nsTexConvProcessor::DetectNumChannels(nsArrayPtr<const nsTexConvSliceChannelMapping> channelMapping, nsUInt32& uiNumChannels)
{
  NS_PROFILE_SCOPE("DetectNumChannels");

  uiNumChannels = 0;

  for (const auto& mapping : channelMapping)
  {
    for (nsUInt32 i = 0; i < 4; ++i)
    {
      if (mapping.m_Channel[i].m_iInputImageIndex != -1 || mapping.m_Channel[i].m_ChannelValue == nsTexConvChannelValue::Black)
      {
        uiNumChannels = nsMath::Max(uiNumChannels, i + 1);
      }
    }
  }

  if (uiNumChannels == 0)
  {
    nsLog::Error("No proper channel mapping provided.");
    return NS_FAILURE;
  }

  // special case handling to detect when the alpha channel will end up white anyway and thus uiNumChannels could be 3 instead of 4
  // which enables us to use more optimized output formats
  if (uiNumChannels == 4)
  {
    uiNumChannels = 3;

    for (const auto& mapping : channelMapping)
    {
      if (mapping.m_Channel[3].m_ChannelValue == nsTexConvChannelValue::Black)
      {
        // sampling a texture without an alpha channel always returns 1, so to use all 0, we do need the channel
        uiNumChannels = 4;
        return NS_SUCCESS;
      }

      if (mapping.m_Channel[3].m_iInputImageIndex == -1)
      {
        // no fourth channel is needed for this
        continue;
      }

      nsImage& img = m_Descriptor.m_InputImages[mapping.m_Channel[3].m_iInputImageIndex];

      const nsUInt32 uiNumRequiredChannels = (nsUInt32)mapping.m_Channel[3].m_ChannelValue + 1;
      const nsUInt32 uiNumActualChannels = nsImageFormat::GetNumChannels(img.GetImageFormat());

      if (uiNumActualChannels < uiNumRequiredChannels)
      {
        // channel not available -> not needed
        continue;
      }

      if (img.Convert(nsImageFormat::R32G32B32A32_FLOAT).Failed())
      {
        // can't convert -> will fail later anyway
        continue;
      }

      const float* pColors = img.GetPixelPointer<float>();
      pColors += (uiNumRequiredChannels - 1); // offset by 0 to 3 to read red, green, blue or alpha

      NS_ASSERT_DEV(img.GetRowPitch() == img.GetWidth() * sizeof(float) * 4, "Unexpected row pitch");

      for (nsUInt32 i = 0; i < img.GetWidth() * img.GetHeight(); ++i)
      {
        if (!nsMath::IsEqual(*pColors, 1.0f, 1.0f / 255.0f))
        {
          // value is not 1.0f -> the channel is needed
          uiNumChannels = 4;
          return NS_SUCCESS;
        }

        pColors += 4;
      }
    }
  }

  return NS_SUCCESS;
}

nsResult nsTexConvProcessor::GenerateOutput(nsImage&& src, nsImage& dst, nsEnum<nsImageFormat> format)
{
  NS_PROFILE_SCOPE("GenerateOutput");

  dst.ResetAndMove(std::move(src));

  if (dst.Convert(format).Failed())
  {
    nsLog::Error("Failed to convert result image to output format '{}'", nsImageFormat::GetName(format));
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

nsResult nsTexConvProcessor::GenerateThumbnailOutput(const nsImage& srcImg, nsImage& dstImg, nsUInt32 uiTargetRes)
{
  if (uiTargetRes == 0)
    return NS_SUCCESS;

  NS_PROFILE_SCOPE("GenerateThumbnailOutput");

  nsUInt32 uiBestMip = 0;

  for (nsUInt32 m = 0; m < srcImg.GetNumMipLevels(); ++m)
  {
    if (srcImg.GetWidth(m) <= uiTargetRes && srcImg.GetHeight(m) <= uiTargetRes)
    {
      uiBestMip = m;
      break;
    }

    uiBestMip = m;
  }

  nsImage scratch1, scratch2;
  nsImage* pCurrentScratch = &scratch1;
  nsImage* pOtherScratch = &scratch2;

  pCurrentScratch->ResetAndCopy(srcImg.GetSubImageView(uiBestMip, 0));

  if (pCurrentScratch->GetWidth() > uiTargetRes || pCurrentScratch->GetHeight() > uiTargetRes)
  {
    if (pCurrentScratch->GetWidth() > pCurrentScratch->GetHeight())
    {
      const float fAspectRatio = (float)pCurrentScratch->GetWidth() / (float)uiTargetRes;
      nsUInt32 uiTargetHeight = (nsUInt32)(pCurrentScratch->GetHeight() / fAspectRatio);

      uiTargetHeight = nsMath::Max(uiTargetHeight, 4U);

      if (nsImageUtils::Scale(*pCurrentScratch, *pOtherScratch, uiTargetRes, uiTargetHeight).Failed())
      {
        nsLog::Error("Failed to resize thumbnail image from {}x{} to {}x{}", pCurrentScratch->GetWidth(), pCurrentScratch->GetHeight(), uiTargetRes,
          uiTargetHeight);
        return NS_FAILURE;
      }
    }
    else
    {
      const float fAspectRatio = (float)pCurrentScratch->GetHeight() / (float)uiTargetRes;
      nsUInt32 uiTargetWidth = (nsUInt32)(pCurrentScratch->GetWidth() / fAspectRatio);

      uiTargetWidth = nsMath::Max(uiTargetWidth, 4U);

      if (nsImageUtils::Scale(*pCurrentScratch, *pOtherScratch, uiTargetWidth, uiTargetRes).Failed())
      {
        nsLog::Error("Failed to resize thumbnail image from {}x{} to {}x{}", pCurrentScratch->GetWidth(), pCurrentScratch->GetHeight(), uiTargetWidth,
          uiTargetRes);
        return NS_FAILURE;
      }
    }

    nsMath::Swap(pCurrentScratch, pOtherScratch);
  }

  dstImg.ResetAndMove(std::move(*pCurrentScratch));

  // we want to write out the thumbnail unchanged, so make sure it has a non-sRGB format
  dstImg.ReinterpretAs(nsImageFormat::AsLinear(dstImg.GetImageFormat()));

  if (dstImg.Convert(nsImageFormat::R8G8B8A8_UNORM).Failed())
  {
    nsLog::Error("Failed to convert thumbnail image to RGBA8.");
    return NS_FAILURE;
  }

  // generate alpha checkerboard pattern
  {
    const float fTileSize = 16.0f;

    nsColorLinearUB* pPixels = dstImg.GetPixelPointer<nsColorLinearUB>();
    const nsUInt64 rowPitch = dstImg.GetRowPitch();

    nsInt32 checkCounter = 0;
    nsColor tiles[2]{nsColor::LightGray, nsColor::DarkGray};


    for (nsUInt32 y = 0; y < dstImg.GetHeight(); ++y)
    {
      checkCounter = (nsInt32)nsMath::Floor(y / fTileSize);

      for (nsUInt32 x = 0; x < dstImg.GetWidth(); ++x)
      {
        nsColorLinearUB& col = pPixels[x];

        if (col.a < 255)
        {
          const nsColor colF = col;
          const nsInt32 tileIdx = (checkCounter + (nsInt32)nsMath::Floor(x / fTileSize)) % 2;

          col = nsMath::Lerp(tiles[tileIdx], colF, nsMath::Sqrt(colF.a)).WithAlpha(colF.a);
        }
      }

      pPixels = nsMemoryUtils::AddByteOffset(pPixels, static_cast<ptrdiff_t>(rowPitch));
    }
  }

  return NS_SUCCESS;
}

nsResult nsTexConvProcessor::GenerateLowResOutput(const nsImage& srcImg, nsImage& dstImg, nsUInt32 uiLowResMip)
{
  if (uiLowResMip == 0)
    return NS_SUCCESS;

  NS_PROFILE_SCOPE("GenerateLowResOutput");

  // don't early out here in this case, otherwise external processes may consider the output to be incomplete
  // if (srcImg.GetNumMipLevels() <= uiLowResMip)
  //{
  //  // probably just a low-resolution input image, do not generate output, but also do not fail
  //  nsLog::Warning("LowRes image not generated, original resolution is already below threshold.");
  //  return NS_SUCCESS;
  //}

  if (nsImageUtils::ExtractLowerMipChain(srcImg, dstImg, uiLowResMip).Failed())
  {
    nsLog::Error("Failed to extract low-res mipmap chain from output image.");
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}



NS_STATICLINK_FILE(Texture, Texture_TexConv_Implementation_Processor);

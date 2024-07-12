#include <Texture/TexturePCH.h>

#include <Foundation/Profiling/Profiling.h>
#include <Texture/TexConv/TexConvProcessor.h>

nsResult nsTexConvProcessor::Assemble2DTexture(const nsImageHeader& refImg, nsImage& dst) const
{
  NS_PROFILE_SCOPE("Assemble2DTexture");

  dst.ResetAndAlloc(refImg);

  nsColor* pPixelOut = dst.GetPixelPointer<nsColor>();

  return Assemble2DSlice(m_Descriptor.m_ChannelMappings[0], refImg.GetWidth(), refImg.GetHeight(), pPixelOut);
}

nsResult nsTexConvProcessor::Assemble2DSlice(const nsTexConvSliceChannelMapping& mapping, nsUInt32 uiResolutionX, nsUInt32 uiResolutionY, nsColor* pPixelOut) const
{
  nsHybridArray<const nsColor*, 16> pSource;
  for (nsUInt32 i = 0; i < m_Descriptor.m_InputImages.GetCount(); ++i)
  {
    pSource.ExpandAndGetRef() = m_Descriptor.m_InputImages[i].GetPixelPointer<nsColor>();
  }

  const float fZero = 0.0f;
  const float fOne = 1.0f;
  const float* pSourceValues[4] = {nullptr, nullptr, nullptr, nullptr};
  nsUInt32 uiSourceStrides[4] = {0, 0, 0, 0};

  for (nsUInt32 channel = 0; channel < 4; ++channel)
  {
    const auto& cm = mapping.m_Channel[channel];
    const nsInt32 inputIndex = cm.m_iInputImageIndex;

    if (inputIndex != -1)
    {
      const nsColor* pSourcePixel = pSource[inputIndex];
      uiSourceStrides[channel] = 4;

      switch (cm.m_ChannelValue)
      {
        case nsTexConvChannelValue::Red:
          pSourceValues[channel] = &pSourcePixel->r;
          break;
        case nsTexConvChannelValue::Green:
          pSourceValues[channel] = &pSourcePixel->g;
          break;
        case nsTexConvChannelValue::Blue:
          pSourceValues[channel] = &pSourcePixel->b;
          break;
        case nsTexConvChannelValue::Alpha:
          pSourceValues[channel] = &pSourcePixel->a;
          break;

        default:
          NS_ASSERT_NOT_IMPLEMENTED;
          break;
      }
    }
    else
    {
      uiSourceStrides[channel] = 0; // because of the constant value

      switch (cm.m_ChannelValue)
      {
        case nsTexConvChannelValue::Black:
          pSourceValues[channel] = &fZero;
          break;

        case nsTexConvChannelValue::White:
          pSourceValues[channel] = &fOne;
          break;

        default:
          if (channel == 3)
            pSourceValues[channel] = &fOne;
          else
            pSourceValues[channel] = &fZero;
          break;
      }
    }
  }

  const bool bFlip = m_Descriptor.m_bFlipHorizontal;

  if (!bFlip && (pSourceValues[0] + 1 == pSourceValues[1]) && (pSourceValues[1] + 1 == pSourceValues[2]) &&
      (pSourceValues[2] + 1 == pSourceValues[3]))
  {
    NS_PROFILE_SCOPE("Assemble2DSlice(memcpy)");

    nsMemoryUtils::Copy<nsColor>(pPixelOut, reinterpret_cast<const nsColor*>(pSourceValues[0]), uiResolutionX * uiResolutionY);
  }
  else
  {
    NS_PROFILE_SCOPE("Assemble2DSlice(gather)");

    for (nsUInt32 y = 0; y < uiResolutionY; ++y)
    {
      const nsUInt32 pixelWriteRowOffset = uiResolutionX * (bFlip ? (uiResolutionY - y - 1) : y);

      for (nsUInt32 x = 0; x < uiResolutionX; ++x)
      {
        float* dst = &pPixelOut[pixelWriteRowOffset + x].r;

        for (nsUInt32 c = 0; c < 4; ++c)
        {
          dst[c] = *pSourceValues[c];
          pSourceValues[c] += uiSourceStrides[c];
        }
      }
    }
  }

  return NS_SUCCESS;
}

nsResult nsTexConvProcessor::DetermineTargetResolution(const nsImage& image, nsEnum<nsImageFormat> OutputImageFormat, nsUInt32& out_uiTargetResolutionX, nsUInt32& out_uiTargetResolutionY) const
{
  NS_PROFILE_SCOPE("DetermineResolution");

  NS_ASSERT_DEV(out_uiTargetResolutionX == 0 && out_uiTargetResolutionY == 0, "Target resolution already determined");

  const nsUInt32 uiOrgResX = image.GetWidth();
  const nsUInt32 uiOrgResY = image.GetHeight();

  out_uiTargetResolutionX = uiOrgResX;
  out_uiTargetResolutionY = uiOrgResY;

  out_uiTargetResolutionX /= (1 << m_Descriptor.m_uiDownscaleSteps);
  out_uiTargetResolutionY /= (1 << m_Descriptor.m_uiDownscaleSteps);

  out_uiTargetResolutionX = nsMath::Clamp(out_uiTargetResolutionX, m_Descriptor.m_uiMinResolution, m_Descriptor.m_uiMaxResolution);
  out_uiTargetResolutionY = nsMath::Clamp(out_uiTargetResolutionY, m_Descriptor.m_uiMinResolution, m_Descriptor.m_uiMaxResolution);

  // keep original aspect ratio
  if (uiOrgResX > uiOrgResY)
  {
    out_uiTargetResolutionY = (out_uiTargetResolutionX * uiOrgResY) / uiOrgResX;
  }
  else if (uiOrgResX < uiOrgResY)
  {
    out_uiTargetResolutionX = (out_uiTargetResolutionY * uiOrgResX) / uiOrgResY;
  }

  if (m_Descriptor.m_OutputType == nsTexConvOutputType::Volume)
  {
    nsUInt32 uiScaleFactor = uiOrgResY / out_uiTargetResolutionY;
    out_uiTargetResolutionX = uiOrgResX / uiScaleFactor;
  }

  if (OutputImageFormat != nsImageFormat::UNKNOWN && nsImageFormat::RequiresFirstLevelBlockAlignment(OutputImageFormat))
  {
    const nsUInt32 blockWidth = nsImageFormat::GetBlockWidth(OutputImageFormat);

    nsUInt32 currentWidth = out_uiTargetResolutionX;
    nsUInt32 currentHeight = out_uiTargetResolutionY;
    bool issueWarning = false;

    if (out_uiTargetResolutionX % blockWidth != 0)
    {
      out_uiTargetResolutionX = nsMath::RoundUp(out_uiTargetResolutionX, static_cast<nsUInt16>(blockWidth));
      issueWarning = true;
    }

    nsUInt32 blockHeight = nsImageFormat::GetBlockHeight(OutputImageFormat);
    if (out_uiTargetResolutionY % blockHeight != 0)
    {
      out_uiTargetResolutionY = nsMath::RoundUp(out_uiTargetResolutionY, static_cast<nsUInt16>(blockHeight));
      issueWarning = true;
    }

    if (issueWarning)
    {
      nsLog::Warning(
        "Chosen output image format is compressed, but target resolution does not fulfill block size requirements. {}x{} -> downscale {} / "
        "clamp({}, {}) -> {}x{}, adjusted to {}x{}",
        uiOrgResX, uiOrgResY, m_Descriptor.m_uiDownscaleSteps, m_Descriptor.m_uiMinResolution, m_Descriptor.m_uiMaxResolution, currentWidth,
        currentHeight, out_uiTargetResolutionX, out_uiTargetResolutionY);
    }
  }

  return NS_SUCCESS;
}

#include <Texture/TexturePCH.h>

#include <Foundation/Profiling/Profiling.h>
#include <Texture/TexConv/TexConvProcessor.h>

static nsImageFormat::Enum DetermineOutputFormatPC(
  nsTexConvUsage::Enum targetFormat, nsTexConvCompressionMode::Enum compressionMode, nsUInt32 uiNumChannels)
{
  if (targetFormat == nsTexConvUsage::NormalMap || targetFormat == nsTexConvUsage::NormalMap_Inverted || targetFormat == nsTexConvUsage::BumpMap)
  {
    if (compressionMode >= nsTexConvCompressionMode::High)
      return nsImageFormat::BC5_UNORM;

    if (compressionMode >= nsTexConvCompressionMode::Medium)
      return nsImageFormat::R8G8_UNORM;

    return nsImageFormat::R16G16_UNORM;
  }

  if (targetFormat == nsTexConvUsage::Color)
  {
    if (compressionMode >= nsTexConvCompressionMode::High && uiNumChannels < 4)
      return nsImageFormat::BC1_UNORM_SRGB;

    if (compressionMode >= nsTexConvCompressionMode::Medium)
      return nsImageFormat::BC7_UNORM_SRGB;

    return nsImageFormat::R8G8B8A8_UNORM_SRGB;
  }

  if (targetFormat == nsTexConvUsage::Linear)
  {
    switch (uiNumChannels)
    {
      case 1:
        if (compressionMode >= nsTexConvCompressionMode::Medium)
          return nsImageFormat::BC4_UNORM;

        return nsImageFormat::R8_UNORM;

      case 2:
        if (compressionMode >= nsTexConvCompressionMode::Medium)
          return nsImageFormat::BC5_UNORM;

        return nsImageFormat::R8G8_UNORM;

      case 3:
        if (compressionMode >= nsTexConvCompressionMode::High)
          return nsImageFormat::BC1_UNORM;

        if (compressionMode >= nsTexConvCompressionMode::Medium)
          return nsImageFormat::BC7_UNORM;

        return nsImageFormat::R8G8B8A8_UNORM;

      case 4:
        if (compressionMode >= nsTexConvCompressionMode::Medium)
          return nsImageFormat::BC7_UNORM;

        return nsImageFormat::R8G8B8A8_UNORM;

      default:
        NS_ASSERT_NOT_IMPLEMENTED;
    }
  }

  if (targetFormat == nsTexConvUsage::Hdr)
  {
    switch (uiNumChannels)
    {
      case 1:
        if (compressionMode >= nsTexConvCompressionMode::High)
          return nsImageFormat::BC6H_UF16;

        return nsImageFormat::R16_FLOAT;

      case 2:
        return nsImageFormat::R16G16_FLOAT;

      case 3:
        if (compressionMode >= nsTexConvCompressionMode::High)
          return nsImageFormat::BC6H_UF16;

        if (compressionMode >= nsTexConvCompressionMode::Medium)
          return nsImageFormat::R11G11B10_FLOAT;

        return nsImageFormat::R16G16B16A16_FLOAT;

      case 4:
        return nsImageFormat::R16G16B16A16_FLOAT;
    }
  }

  return nsImageFormat::UNKNOWN;
}

nsResult nsTexConvProcessor::ChooseOutputFormat(nsEnum<nsImageFormat>& out_Format, nsEnum<nsTexConvUsage> usage, nsUInt32 uiNumChannels) const
{
  NS_PROFILE_SCOPE("ChooseOutputFormat");

  NS_ASSERT_DEV(out_Format == nsImageFormat::UNKNOWN, "Output format already set");

  switch (m_Descriptor.m_TargetPlatform)
  {
      // case  nsTexConvTargetPlatform::Android:
      //  out_Format = DetermineOutputFormatAndroid(m_Descriptor.m_TargetFormat, m_Descriptor.m_CompressionMode);
      //  break;

    case nsTexConvTargetPlatform::PC:
      out_Format = DetermineOutputFormatPC(usage, m_Descriptor.m_CompressionMode, uiNumChannels);
      break;

    default:
      NS_ASSERT_NOT_IMPLEMENTED;
  }

  if (out_Format == nsImageFormat::UNKNOWN)
  {
    nsLog::Error("Failed to decide for an output image format.");
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

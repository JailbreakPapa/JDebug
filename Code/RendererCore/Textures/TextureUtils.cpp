#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Reflection/ReflectionUtils.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererCore/Textures/TextureUtils.h>

bool nsTextureUtils::s_bForceFullQualityAlways = false;

nsGALResourceFormat::Enum nsTextureUtils::ImageFormatToGalFormat(nsImageFormat::Enum format, bool bSRGB)
{
  switch (format)
  {
    case nsImageFormat::R8G8B8A8_UNORM:
      if (bSRGB)
        return nsGALResourceFormat::RGBAUByteNormalizedsRGB;
      else
        return nsGALResourceFormat::RGBAUByteNormalized;

      // case nsImageFormat::R8G8B8A8_TYPELESS:
    case nsImageFormat::R8G8B8A8_UNORM_SRGB:
      return nsGALResourceFormat::RGBAUByteNormalizedsRGB;

    case nsImageFormat::R8G8B8A8_UINT:
      return nsGALResourceFormat::RGBAUInt;

    case nsImageFormat::R8G8B8A8_SNORM:
      return nsGALResourceFormat::RGBAByteNormalized;

    case nsImageFormat::R8G8B8A8_SINT:
      return nsGALResourceFormat::RGBAInt;

    case nsImageFormat::B8G8R8A8_UNORM:
      if (bSRGB)
        return nsGALResourceFormat::BGRAUByteNormalizedsRGB;
      else
        return nsGALResourceFormat::BGRAUByteNormalized;

    case nsImageFormat::B8G8R8X8_UNORM:
      if (bSRGB)
        return nsGALResourceFormat::BGRAUByteNormalizedsRGB;
      else
        return nsGALResourceFormat::BGRAUByteNormalized;

      // case nsImageFormat::B8G8R8A8_TYPELESS:
    case nsImageFormat::B8G8R8A8_UNORM_SRGB:
      return nsGALResourceFormat::BGRAUByteNormalizedsRGB;

      // case nsImageFormat::B8G8R8X8_TYPELESS:
    case nsImageFormat::B8G8R8X8_UNORM_SRGB:
      return nsGALResourceFormat::BGRAUByteNormalizedsRGB;

      // case nsImageFormat::B8G8R8_UNORM:

      // case nsImageFormat::BC1_TYPELESS:
    case nsImageFormat::BC1_UNORM:
      if (bSRGB)
        return nsGALResourceFormat::BC1sRGB;
      else
        return nsGALResourceFormat::BC1;

    case nsImageFormat::BC1_UNORM_SRGB:
      return nsGALResourceFormat::BC1sRGB;

      // case nsImageFormat::BC2_TYPELESS:
    case nsImageFormat::BC2_UNORM:
      if (bSRGB)
        return nsGALResourceFormat::BC2sRGB;
      else
        return nsGALResourceFormat::BC2;

    case nsImageFormat::BC2_UNORM_SRGB:
      return nsGALResourceFormat::BC2sRGB;

      // case nsImageFormat::BC3_TYPELESS:
    case nsImageFormat::BC3_UNORM:
      if (bSRGB)
        return nsGALResourceFormat::BC3sRGB;
      else
        return nsGALResourceFormat::BC3;

    case nsImageFormat::BC3_UNORM_SRGB:
      return nsGALResourceFormat::BC3sRGB;

      // case nsImageFormat::BC4_TYPELESS:
    case nsImageFormat::BC4_UNORM:
      return nsGALResourceFormat::BC4UNormalized;

    case nsImageFormat::BC4_SNORM:
      return nsGALResourceFormat::BC4Normalized;

      // case nsImageFormat::BC5_TYPELESS:
    case nsImageFormat::BC5_UNORM:
      return nsGALResourceFormat::BC5UNormalized;

    case nsImageFormat::BC5_SNORM:
      return nsGALResourceFormat::BC5Normalized;

      // case nsImageFormat::BC6H_TYPELESS:
    case nsImageFormat::BC6H_UF16:
      return nsGALResourceFormat::BC6UFloat;

    case nsImageFormat::BC6H_SF16:
      return nsGALResourceFormat::BC6Float;

      // case nsImageFormat::BC7_TYPELESS:
    case nsImageFormat::BC7_UNORM:
      if (bSRGB)
        return nsGALResourceFormat::BC7UNormalizedsRGB;
      else
        return nsGALResourceFormat::BC7UNormalized;

    case nsImageFormat::BC7_UNORM_SRGB:
      return nsGALResourceFormat::BC7UNormalizedsRGB;

    case nsImageFormat::B5G6R5_UNORM:
      return nsGALResourceFormat::B5G6R5UNormalized; /// \todo Not supported by some GPUs ?

    case nsImageFormat::R16_FLOAT:
      return nsGALResourceFormat::RHalf;

    case nsImageFormat::R32_FLOAT:
      return nsGALResourceFormat::RFloat;

    case nsImageFormat::R16G16_FLOAT:
      return nsGALResourceFormat::RGHalf;

    case nsImageFormat::R32G32_FLOAT:
      return nsGALResourceFormat::RGFloat;

    case nsImageFormat::R32G32B32_FLOAT:
      return nsGALResourceFormat::RGBFloat;

    case nsImageFormat::R16G16B16A16_FLOAT:
      return nsGALResourceFormat::RGBAHalf;

    case nsImageFormat::R32G32B32A32_FLOAT:
      return nsGALResourceFormat::RGBAFloat;

    case nsImageFormat::R16G16B16A16_UNORM:
      return nsGALResourceFormat::RGBAUShortNormalized;

    case nsImageFormat::R8_UNORM:
      return nsGALResourceFormat::RUByteNormalized;

    case nsImageFormat::R8G8_UNORM:
      return nsGALResourceFormat::RGUByteNormalized;

    case nsImageFormat::R16G16_UNORM:
      return nsGALResourceFormat::RGUShortNormalized;

    case nsImageFormat::R11G11B10_FLOAT:
      return nsGALResourceFormat::RG11B10Float;

    default:
      NS_ASSERT_NOT_IMPLEMENTED;
      break;
  }

  return nsGALResourceFormat::Invalid;
}

nsImageFormat::Enum nsTextureUtils::GalFormatToImageFormat(nsGALResourceFormat::Enum format)
{
  switch (format)
  {
    case nsGALResourceFormat::RGBAFloat:
      return nsImageFormat::R32G32B32A32_FLOAT;
    case nsGALResourceFormat::RGBAUInt:
      return nsImageFormat::R32G32B32A32_UINT;
    case nsGALResourceFormat::RGBAInt:
      return nsImageFormat::R32G32B32A32_SINT;
    case nsGALResourceFormat::RGBFloat:
      return nsImageFormat::R32G32B32_FLOAT;
    case nsGALResourceFormat::RGBUInt:
      return nsImageFormat::R32G32B32_UINT;
    case nsGALResourceFormat::RGBInt:
      return nsImageFormat::R32G32B32_SINT;
    case nsGALResourceFormat::B5G6R5UNormalized:
      return nsImageFormat::B5G6R5_UNORM;
    case nsGALResourceFormat::BGRAUByteNormalized:
      return nsImageFormat::B8G8R8A8_UNORM;
    case nsGALResourceFormat::BGRAUByteNormalizedsRGB:
      return nsImageFormat::B8G8R8A8_UNORM_SRGB;
    case nsGALResourceFormat::RGBAHalf:
      return nsImageFormat::R16G16B16A16_FLOAT;
    case nsGALResourceFormat::RGBAUShort:
      return nsImageFormat::R16G16B16A16_UINT;
    case nsGALResourceFormat::RGBAUShortNormalized:
      return nsImageFormat::R16G16B16A16_UNORM;
    case nsGALResourceFormat::RGBAShort:
      return nsImageFormat::R16G16B16A16_SINT;
    case nsGALResourceFormat::RGBAShortNormalized:
      return nsImageFormat::R16G16B16A16_SNORM;
    case nsGALResourceFormat::RGFloat:
      return nsImageFormat::R32G32_FLOAT;
    case nsGALResourceFormat::RGUInt:
      return nsImageFormat::R32G32_UINT;
    case nsGALResourceFormat::RGInt:
      return nsImageFormat::R32G32_SINT;
    case nsGALResourceFormat::RG11B10Float:
      return nsImageFormat::R11G11B10_FLOAT;
    case nsGALResourceFormat::RGBAUByteNormalized:
      return nsImageFormat::R8G8B8A8_UNORM;
    case nsGALResourceFormat::RGBAUByteNormalizedsRGB:
      return nsImageFormat::R8G8B8A8_UNORM_SRGB;
    case nsGALResourceFormat::RGBAUByte:
      return nsImageFormat::R8G8B8A8_UINT;
    case nsGALResourceFormat::RGBAByteNormalized:
      return nsImageFormat::R8G8B8A8_SNORM;
    case nsGALResourceFormat::RGBAByte:
      return nsImageFormat::R8G8B8A8_SINT;
    case nsGALResourceFormat::RGHalf:
      return nsImageFormat::R16G16_FLOAT;
    case nsGALResourceFormat::RGUShort:
      return nsImageFormat::R16G16_UINT;
    case nsGALResourceFormat::RGUShortNormalized:
      return nsImageFormat::R16G16_UNORM;
    case nsGALResourceFormat::RGShort:
      return nsImageFormat::R16G16_SINT;
    case nsGALResourceFormat::RGShortNormalized:
      return nsImageFormat::R16G16_SNORM;
    case nsGALResourceFormat::RGUByte:
      return nsImageFormat::R8G8_UINT;
    case nsGALResourceFormat::RGUByteNormalized:
      return nsImageFormat::R8G8_UNORM;
    case nsGALResourceFormat::RGByte:
      return nsImageFormat::R8G8_SINT;
    case nsGALResourceFormat::RGByteNormalized:
      return nsImageFormat::R8G8_SNORM;
    case nsGALResourceFormat::DFloat:
      return nsImageFormat::R32_FLOAT;
    case nsGALResourceFormat::RFloat:
      return nsImageFormat::R32_FLOAT;
    case nsGALResourceFormat::RUInt:
      return nsImageFormat::R32_UINT;
    case nsGALResourceFormat::RInt:
      return nsImageFormat::R32_SINT;
    case nsGALResourceFormat::RHalf:
      return nsImageFormat::R16_FLOAT;
    case nsGALResourceFormat::RUShort:
      return nsImageFormat::R16_UINT;
    case nsGALResourceFormat::RUShortNormalized:
      return nsImageFormat::R16_UNORM;
    case nsGALResourceFormat::RShort:
      return nsImageFormat::R16_SINT;
    case nsGALResourceFormat::RShortNormalized:
      return nsImageFormat::R16_SNORM;
    case nsGALResourceFormat::RUByte:
      return nsImageFormat::R8_UINT;
    case nsGALResourceFormat::RUByteNormalized:
      return nsImageFormat::R8_UNORM;
    case nsGALResourceFormat::RByte:
      return nsImageFormat::R8_SINT;
    case nsGALResourceFormat::RByteNormalized:
      return nsImageFormat::R8_SNORM;
    case nsGALResourceFormat::AUByteNormalized:
      return nsImageFormat::R8_UNORM;
    case nsGALResourceFormat::D16:
      return nsImageFormat::R16_UINT;
    case nsGALResourceFormat::BC1:
      return nsImageFormat::BC1_UNORM;
    case nsGALResourceFormat::BC1sRGB:
      return nsImageFormat::BC1_UNORM_SRGB;
    case nsGALResourceFormat::BC2:
      return nsImageFormat::BC2_UNORM;
    case nsGALResourceFormat::BC2sRGB:
      return nsImageFormat::BC2_UNORM_SRGB;
    case nsGALResourceFormat::BC3:
      return nsImageFormat::BC3_UNORM;
    case nsGALResourceFormat::BC3sRGB:
      return nsImageFormat::BC3_UNORM_SRGB;
    case nsGALResourceFormat::BC4UNormalized:
      return nsImageFormat::BC4_UNORM;
    case nsGALResourceFormat::BC4Normalized:
      return nsImageFormat::BC4_SNORM;
    case nsGALResourceFormat::BC5UNormalized:
      return nsImageFormat::BC5_UNORM;
    case nsGALResourceFormat::BC5Normalized:
      return nsImageFormat::BC5_SNORM;
    case nsGALResourceFormat::BC6UFloat:
      return nsImageFormat::BC6H_UF16;
    case nsGALResourceFormat::BC6Float:
      return nsImageFormat::BC6H_SF16;
    case nsGALResourceFormat::BC7UNormalized:
      return nsImageFormat::BC7_UNORM;
    case nsGALResourceFormat::BC7UNormalizedsRGB:
      return nsImageFormat::BC7_UNORM_SRGB;
    case nsGALResourceFormat::RGB10A2UInt:
    case nsGALResourceFormat::RGB10A2UIntNormalized:
    case nsGALResourceFormat::D24S8:
    default:
    {
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
      nsStringBuilder sFormat;
      NS_ASSERT_DEBUG(nsReflectionUtils::EnumerationToString(nsGetStaticRTTI<nsGALResourceFormat>(), format, sFormat, nsReflectionUtils::EnumConversionMode::ValueNameOnly), "Cannot convert GAL format '{}' to string", format);
      NS_ASSERT_DEBUG(false, "The GL format: '{}' does not have a matching image format.", sFormat);
#endif
    }
  }
  return nsImageFormat::UNKNOWN;
}

nsImageFormat::Enum nsTextureUtils::GalFormatToImageFormat(nsGALResourceFormat::Enum format, bool bRemoveSRGB)
{
  nsImageFormat::Enum imageFormat = GalFormatToImageFormat(format);
  if (bRemoveSRGB)
  {
    imageFormat = nsImageFormat::AsLinear(imageFormat);
  }
  return imageFormat;
}

void nsTextureUtils::ConfigureSampler(nsTextureFilterSetting::Enum filter, nsGALSamplerStateCreationDescription& out_sampler)
{
  const nsTextureFilterSetting::Enum thisFilter = nsRenderContext::GetDefaultInstance()->GetSpecificTextureFilter(filter);

  out_sampler.m_MinFilter = nsGALTextureFilterMode::Linear;
  out_sampler.m_MagFilter = nsGALTextureFilterMode::Linear;
  out_sampler.m_MipFilter = nsGALTextureFilterMode::Linear;
  out_sampler.m_uiMaxAnisotropy = 1;

  switch (thisFilter)
  {
    case nsTextureFilterSetting::FixedNearest:
      out_sampler.m_MinFilter = nsGALTextureFilterMode::Point;
      out_sampler.m_MagFilter = nsGALTextureFilterMode::Point;
      out_sampler.m_MipFilter = nsGALTextureFilterMode::Point;
      break;
    case nsTextureFilterSetting::FixedBilinear:
      out_sampler.m_MipFilter = nsGALTextureFilterMode::Point;
      break;
    case nsTextureFilterSetting::FixedTrilinear:
      break;
    case nsTextureFilterSetting::FixedAnisotropic2x:
      out_sampler.m_MinFilter = nsGALTextureFilterMode::Anisotropic;
      out_sampler.m_MagFilter = nsGALTextureFilterMode::Anisotropic;
      out_sampler.m_uiMaxAnisotropy = 2;
      break;
    case nsTextureFilterSetting::FixedAnisotropic4x:
      out_sampler.m_MinFilter = nsGALTextureFilterMode::Anisotropic;
      out_sampler.m_MagFilter = nsGALTextureFilterMode::Anisotropic;
      out_sampler.m_uiMaxAnisotropy = 4;
      break;
    case nsTextureFilterSetting::FixedAnisotropic8x:
      out_sampler.m_MinFilter = nsGALTextureFilterMode::Anisotropic;
      out_sampler.m_MagFilter = nsGALTextureFilterMode::Anisotropic;
      out_sampler.m_uiMaxAnisotropy = 8;
      break;
    case nsTextureFilterSetting::FixedAnisotropic16x:
      out_sampler.m_MinFilter = nsGALTextureFilterMode::Anisotropic;
      out_sampler.m_MagFilter = nsGALTextureFilterMode::Anisotropic;
      out_sampler.m_uiMaxAnisotropy = 16;
      break;
    default:
      break;
  }
}

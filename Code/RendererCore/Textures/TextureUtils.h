#pragma once

#include <RendererCore/RenderContext/Implementation/RenderContextStructs.h>
#include <RendererCore/RendererCoreDLL.h>
#include <RendererFoundation/Resources/ResourceFormats.h>
#include <Texture/Image/Image.h>

struct NS_RENDERERCORE_DLL nsTextureUtils
{
  static nsGALResourceFormat::Enum ImageFormatToGalFormat(nsImageFormat::Enum format, bool bSRGB);
  static nsImageFormat::Enum GalFormatToImageFormat(nsGALResourceFormat::Enum format, bool bRemoveSRGB);
  static nsImageFormat::Enum GalFormatToImageFormat(nsGALResourceFormat::Enum format);


  static void ConfigureSampler(nsTextureFilterSetting::Enum filter, nsGALSamplerStateCreationDescription& out_sampler);

  /// \brief If enabled, textures are always loaded to full quality immediately. Mostly necessary for image comparison unit tests.
  static bool s_bForceFullQualityAlways;
};

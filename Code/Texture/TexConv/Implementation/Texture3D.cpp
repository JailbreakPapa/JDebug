#include <Texture/TexturePCH.h>

#include <Foundation/Profiling/Profiling.h>
#include <Texture/Image/ImageUtils.h>
#include <Texture/TexConv/TexConvProcessor.h>

nsResult nsTexConvProcessor::Assemble3DTexture(nsImage& dst) const
{
  NS_PROFILE_SCOPE("Assemble3DTexture");

  const auto& images = m_Descriptor.m_InputImages;

  return nsImageUtils::CreateVolumeTextureFromSingleFile(dst, images[0]);
}

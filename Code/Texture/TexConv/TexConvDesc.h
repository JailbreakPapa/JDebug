#pragma once

#include <Texture/TexConv/TexConvEnums.h>

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Types/UniquePtr.h>
#include <Texture/Image/Image.h>
#include <Texture/Image/ImageEnums.h>

struct nsTexConvChannelMapping
{
  nsInt8 m_iInputImageIndex = -1;
  nsTexConvChannelValue::Enum m_ChannelValue;
};

/// Describes from which input file to read which channel and then write it to the R, G, B, or A channel of the
/// output file. The four elements of the array represent the four channels of the output image.
struct nsTexConvSliceChannelMapping
{
  nsTexConvChannelMapping m_Channel[4] = {
    nsTexConvChannelMapping{-1, nsTexConvChannelValue::Red},
    nsTexConvChannelMapping{-1, nsTexConvChannelValue::Green},
    nsTexConvChannelMapping{-1, nsTexConvChannelValue::Blue},
    nsTexConvChannelMapping{-1, nsTexConvChannelValue::Alpha},
  };
};

class NS_TEXTURE_DLL nsTexConvDesc
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsTexConvDesc);

public:
  nsTexConvDesc() = default;

  nsHybridArray<nsString, 4> m_InputFiles;
  nsDynamicArray<nsImage> m_InputImages;

  nsHybridArray<nsTexConvSliceChannelMapping, 6> m_ChannelMappings;

  // output type / platform
  nsEnum<nsTexConvOutputType> m_OutputType;
  nsEnum<nsTexConvTargetPlatform> m_TargetPlatform; // TODO: implement android

  // low resolution output
  nsUInt32 m_uiLowResMipmaps = 0;

  // thumbnail output
  nsUInt32 m_uiThumbnailOutputResolution = 0;

  // Format / Compression
  nsEnum<nsTexConvUsage> m_Usage;
  nsEnum<nsTexConvCompressionMode> m_CompressionMode;

  // resolution clamp and downscale
  nsUInt32 m_uiMinResolution = 16;
  nsUInt32 m_uiMaxResolution = 1024 * 8;
  nsUInt32 m_uiDownscaleSteps = 0;

  // Mipmaps / filtering
  nsEnum<nsTexConvMipmapMode> m_MipmapMode;
  nsEnum<nsTextureFilterSetting> m_FilterMode; // only used when writing to ns specific formats
  nsEnum<nsImageAddressMode> m_AddressModeU;
  nsEnum<nsImageAddressMode> m_AddressModeV;
  nsEnum<nsImageAddressMode> m_AddressModeW;
  bool m_bPreserveMipmapCoverage = false;
  float m_fMipmapAlphaThreshold = 0.5f;

  // Misc options
  nsUInt8 m_uiDilateColor = 0;
  bool m_bFlipHorizontal = false;
  bool m_bPremultiplyAlpha = false;
  float m_fHdrExposureBias = 0.0f;
  float m_fMaxValue = 64000.f;

  // ns specific
  nsUInt64 m_uiAssetHash = 0;
  nsUInt16 m_uiAssetVersion = 0;

  // Texture Atlas
  nsString m_sTextureAtlasDescFile;

  // Bump map filter
  nsEnum<nsTexConvBumpMapFilter> m_BumpMapFilter;
};

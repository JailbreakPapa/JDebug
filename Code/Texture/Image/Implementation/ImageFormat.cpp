#include <Texture/TexturePCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/Containers/StaticArray.h>
#include <Texture/Image/ImageFormat.h>

namespace
{
  struct nsImageFormatMetaData
  {
    nsImageFormatMetaData()
    {
      nsMemoryUtils::ZeroFillArray(m_uiBitsPerChannel);
      nsMemoryUtils::ZeroFillArray(m_uiChannelMasks);

      m_planeData.SetCount(1);
    }

    const char* m_szName = nullptr;

    struct PlaneData
    {
      nsUInt8 m_uiBitsPerBlock{0}; ///< Bits per block for compressed formats; for uncompressed formats (which always have a block size of 1x1x1), this is equal to bits per pixel.
      nsUInt8 m_uiBlockWidth{1};
      nsUInt8 m_uiBlockHeight{1};
      nsUInt8 m_uiBlockDepth{1};
      nsImageFormat::Enum m_subFormat{nsImageFormat::UNKNOWN}; ///< Subformats when viewing only a subslice of the data.
    };

    nsStaticArray<PlaneData, 2> m_planeData;

    nsUInt8 m_uiNumChannels{0};

    nsUInt8 m_uiBitsPerChannel[nsImageFormatChannel::COUNT];
    nsUInt32 m_uiChannelMasks[nsImageFormatChannel::COUNT];


    bool m_requireFirstLevelBlockAligned{false}; ///< Only for compressed formats: If true, the first level's dimensions must be a multiple of the
                                                 ///< block size; if false, padding can be applied for compressing the first mip level, too.
    bool m_isDepth{false};
    bool m_isStencil{false};

    nsImageFormatDataType::Enum m_dataType{nsImageFormatDataType::NONE};
    nsImageFormatType::Enum m_formatType{nsImageFormatType::UNKNOWN};

    nsImageFormat::Enum m_asLinear{nsImageFormat::UNKNOWN};
    nsImageFormat::Enum m_asSrgb{nsImageFormat::UNKNOWN};

    nsUInt32 getNumBlocksX(nsUInt32 uiWidth, nsUInt32 uiPlaneIndex) const
    {
      return (uiWidth - 1) / m_planeData[uiPlaneIndex].m_uiBlockWidth + 1;
    }

    nsUInt32 getNumBlocksY(nsUInt32 uiHeight, nsUInt32 uiPlaneIndex) const
    {
      return (uiHeight - 1) / m_planeData[uiPlaneIndex].m_uiBlockHeight + 1;
    }

    nsUInt32 getNumBlocksZ(nsUInt32 uiDepth, nsUInt32 uiPlaneIndex) const
    {
      return (uiDepth - 1) / m_planeData[uiPlaneIndex].m_uiBlockDepth + 1;
    }

    nsUInt32 getRowPitch(nsUInt32 uiWidth, nsUInt32 uiPlaneIndex) const
    {
      return getNumBlocksX(uiWidth, uiPlaneIndex) * m_planeData[uiPlaneIndex].m_uiBitsPerBlock / 8;
    }
  };

  nsStaticArray<nsImageFormatMetaData, nsImageFormat::NUM_FORMATS> s_formatMetaData;

  void InitFormatLinear(nsImageFormat::Enum format, const char* szName, nsImageFormatDataType::Enum dataType, nsUInt8 uiBitsPerPixel, nsUInt8 uiBitsR,
    nsUInt8 uiBitsG, nsUInt8 uiBitsB, nsUInt8 uiBitsA, nsUInt8 uiNumChannels)
  {
    s_formatMetaData[format].m_szName = szName;

    s_formatMetaData[format].m_planeData[0].m_uiBitsPerBlock = uiBitsPerPixel;
    s_formatMetaData[format].m_dataType = dataType;
    s_formatMetaData[format].m_formatType = nsImageFormatType::LINEAR;

    s_formatMetaData[format].m_uiNumChannels = uiNumChannels;

    s_formatMetaData[format].m_uiBitsPerChannel[nsImageFormatChannel::R] = uiBitsR;
    s_formatMetaData[format].m_uiBitsPerChannel[nsImageFormatChannel::G] = uiBitsG;
    s_formatMetaData[format].m_uiBitsPerChannel[nsImageFormatChannel::B] = uiBitsB;
    s_formatMetaData[format].m_uiBitsPerChannel[nsImageFormatChannel::A] = uiBitsA;

    s_formatMetaData[format].m_asLinear = format;
    s_formatMetaData[format].m_asSrgb = format;
  }

#define INIT_FORMAT_LINEAR(format, dataType, uiBitsPerPixel, uiBitsR, uiBitsG, uiBitsB, uiBitsA, uiNumChannels) \
  InitFormatLinear(nsImageFormat::format, #format, nsImageFormatDataType::dataType, uiBitsPerPixel, uiBitsR, uiBitsG, uiBitsB, uiBitsA, uiNumChannels)

  void InitFormatCompressed(nsImageFormat::Enum format, const char* szName, nsImageFormatDataType::Enum dataType, nsUInt8 uiBitsPerBlock,
    nsUInt8 uiBlockWidth, nsUInt8 uiBlockHeight, nsUInt8 uiBlockDepth, bool bRequireFirstLevelBlockAligned, nsUInt8 uiNumChannels)
  {
    s_formatMetaData[format].m_szName = szName;

    s_formatMetaData[format].m_planeData[0].m_uiBitsPerBlock = uiBitsPerBlock;
    s_formatMetaData[format].m_planeData[0].m_uiBlockWidth = uiBlockWidth;
    s_formatMetaData[format].m_planeData[0].m_uiBlockHeight = uiBlockHeight;
    s_formatMetaData[format].m_planeData[0].m_uiBlockDepth = uiBlockDepth;
    s_formatMetaData[format].m_dataType = dataType;
    s_formatMetaData[format].m_formatType = nsImageFormatType::BLOCK_COMPRESSED;

    s_formatMetaData[format].m_uiNumChannels = uiNumChannels;

    s_formatMetaData[format].m_requireFirstLevelBlockAligned = bRequireFirstLevelBlockAligned;

    s_formatMetaData[format].m_asLinear = format;
    s_formatMetaData[format].m_asSrgb = format;
  }

#define INIT_FORMAT_COMPRESSED(                                                                                                                    \
  format, dataType, uiBitsPerBlock, uiBlockWidth, uiBlockHeight, uiBlockDepth, requireFirstLevelBlockAligned, uiNumChannels)                       \
  InitFormatCompressed(nsImageFormat::format, #format, nsImageFormatDataType::dataType, uiBitsPerBlock, uiBlockWidth, uiBlockHeight, uiBlockDepth, \
    requireFirstLevelBlockAligned, uiNumChannels)

  void InitFormatDepth(nsImageFormat::Enum format, const char* szName, nsImageFormatDataType::Enum dataType, nsUInt8 uiBitsPerPixel, bool bIsStencil,
    nsUInt8 uiBitsD, nsUInt8 uiBitsS)
  {
    s_formatMetaData[format].m_szName = szName;

    s_formatMetaData[format].m_planeData[0].m_uiBitsPerBlock = uiBitsPerPixel;
    s_formatMetaData[format].m_dataType = dataType;
    s_formatMetaData[format].m_formatType = nsImageFormatType::LINEAR;

    s_formatMetaData[format].m_isDepth = true;
    s_formatMetaData[format].m_isStencil = bIsStencil;

    s_formatMetaData[format].m_uiNumChannels = bIsStencil ? 2 : 1;

    s_formatMetaData[format].m_uiBitsPerChannel[nsImageFormatChannel::D] = uiBitsD;
    s_formatMetaData[format].m_uiBitsPerChannel[nsImageFormatChannel::S] = uiBitsS;

    s_formatMetaData[format].m_asLinear = format;
    s_formatMetaData[format].m_asSrgb = format;
  }

#define INIT_FORMAT_DEPTH(format, dataType, uiBitsPerPixel, isStencil, uiBitsD, uiBitsS) \
  InitFormatDepth(nsImageFormat::format, #format, nsImageFormatDataType::dataType, uiBitsPerPixel, isStencil, uiBitsD, uiBitsS);

  void SetupSrgbPair(nsImageFormat::Enum linearFormat, nsImageFormat::Enum srgbFormat)
  {
    s_formatMetaData[linearFormat].m_asLinear = linearFormat;
    s_formatMetaData[linearFormat].m_asSrgb = srgbFormat;

    s_formatMetaData[srgbFormat].m_asLinear = linearFormat;
    s_formatMetaData[srgbFormat].m_asSrgb = srgbFormat;
  }

} // namespace

static void SetupImageFormatTable()
{
  if (!s_formatMetaData.IsEmpty())
    return;

  s_formatMetaData.SetCount(nsImageFormat::NUM_FORMATS);

  s_formatMetaData[nsImageFormat::UNKNOWN].m_szName = "UNKNOWN";

  INIT_FORMAT_LINEAR(R32G32B32A32_FLOAT, FLOAT, 128, 32, 32, 32, 32, 4);
  INIT_FORMAT_LINEAR(R32G32B32A32_UINT, UINT, 128, 32, 32, 32, 32, 4);
  INIT_FORMAT_LINEAR(R32G32B32A32_SINT, SINT, 128, 32, 32, 32, 32, 4);

  INIT_FORMAT_LINEAR(R32G32B32_FLOAT, FLOAT, 96, 32, 32, 32, 0, 3);
  INIT_FORMAT_LINEAR(R32G32B32_UINT, UINT, 96, 32, 32, 32, 0, 3);
  INIT_FORMAT_LINEAR(R32G32B32_SINT, SINT, 96, 32, 32, 32, 0, 3);

  INIT_FORMAT_LINEAR(R32G32_FLOAT, FLOAT, 64, 32, 32, 0, 0, 2);
  INIT_FORMAT_LINEAR(R32G32_UINT, UINT, 64, 32, 32, 0, 0, 2);
  INIT_FORMAT_LINEAR(R32G32_SINT, SINT, 64, 32, 32, 0, 0, 2);

  INIT_FORMAT_LINEAR(R32_FLOAT, FLOAT, 32, 32, 0, 0, 0, 1);
  INIT_FORMAT_LINEAR(R32_UINT, UINT, 32, 32, 0, 0, 0, 1);
  INIT_FORMAT_LINEAR(R32_SINT, SINT, 32, 32, 0, 0, 0, 1);

  INIT_FORMAT_LINEAR(R16G16B16A16_FLOAT, FLOAT, 64, 16, 16, 16, 16, 4);
  INIT_FORMAT_LINEAR(R16G16B16A16_UINT, UINT, 64, 16, 16, 16, 16, 4);
  INIT_FORMAT_LINEAR(R16G16B16A16_SINT, SINT, 64, 16, 16, 16, 16, 4);
  INIT_FORMAT_LINEAR(R16G16B16A16_UNORM, UNORM, 64, 16, 16, 16, 16, 4);
  INIT_FORMAT_LINEAR(R16G16B16A16_SNORM, SNORM, 64, 16, 16, 16, 16, 4);

  INIT_FORMAT_LINEAR(R16G16B16_UNORM, UNORM, 48, 16, 16, 16, 0, 3);

  INIT_FORMAT_LINEAR(R16G16_FLOAT, FLOAT, 32, 16, 16, 0, 0, 2);
  INIT_FORMAT_LINEAR(R16G16_UINT, UINT, 32, 16, 16, 0, 0, 2);
  INIT_FORMAT_LINEAR(R16G16_SINT, SINT, 32, 16, 16, 0, 0, 2);
  INIT_FORMAT_LINEAR(R16G16_UNORM, UNORM, 32, 16, 16, 0, 0, 2);
  INIT_FORMAT_LINEAR(R16G16_SNORM, SNORM, 32, 16, 16, 0, 0, 2);

  INIT_FORMAT_LINEAR(R16_FLOAT, FLOAT, 16, 16, 0, 0, 0, 1);
  INIT_FORMAT_LINEAR(R16_UINT, UINT, 16, 16, 0, 0, 0, 1);
  INIT_FORMAT_LINEAR(R16_SINT, SINT, 16, 16, 0, 0, 0, 1);
  INIT_FORMAT_LINEAR(R16_UNORM, UNORM, 16, 16, 0, 0, 0, 1);
  INIT_FORMAT_LINEAR(R16_SNORM, SNORM, 16, 16, 0, 0, 0, 1);

  INIT_FORMAT_LINEAR(R8G8B8A8_UINT, UINT, 32, 8, 8, 8, 8, 4);
  INIT_FORMAT_LINEAR(R8G8B8A8_SINT, SINT, 32, 8, 8, 8, 8, 4);
  INIT_FORMAT_LINEAR(R8G8B8A8_SNORM, SNORM, 32, 8, 8, 8, 8, 4);

  INIT_FORMAT_LINEAR(R8G8B8A8_UNORM, UNORM, 32, 8, 8, 8, 8, 4);
  INIT_FORMAT_LINEAR(R8G8B8A8_UNORM_SRGB, UNORM, 32, 8, 8, 8, 8, 4);
  SetupSrgbPair(nsImageFormat::R8G8B8A8_UNORM, nsImageFormat::R8G8B8A8_UNORM_SRGB);

  s_formatMetaData[nsImageFormat::R8G8B8A8_UNORM].m_uiChannelMasks[nsImageFormatChannel::R] = 0x000000FF;
  s_formatMetaData[nsImageFormat::R8G8B8A8_UNORM].m_uiChannelMasks[nsImageFormatChannel::G] = 0x0000FF00;
  s_formatMetaData[nsImageFormat::R8G8B8A8_UNORM].m_uiChannelMasks[nsImageFormatChannel::B] = 0x00FF0000;
  s_formatMetaData[nsImageFormat::R8G8B8A8_UNORM].m_uiChannelMasks[nsImageFormatChannel::A] = 0xFF000000;

  INIT_FORMAT_LINEAR(R8G8B8_UNORM, UNORM, 24, 8, 8, 8, 0, 3);
  INIT_FORMAT_LINEAR(R8G8B8_UNORM_SRGB, UNORM, 24, 8, 8, 8, 0, 3);
  SetupSrgbPair(nsImageFormat::R8G8B8_UNORM, nsImageFormat::R8G8B8_UNORM_SRGB);

  INIT_FORMAT_LINEAR(B8G8R8A8_UNORM, UNORM, 32, 8, 8, 8, 8, 4);
  INIT_FORMAT_LINEAR(B8G8R8A8_UNORM_SRGB, UNORM, 32, 8, 8, 8, 8, 4);
  SetupSrgbPair(nsImageFormat::B8G8R8A8_UNORM, nsImageFormat::B8G8R8A8_UNORM_SRGB);

  s_formatMetaData[nsImageFormat::B8G8R8A8_UNORM].m_uiChannelMasks[nsImageFormatChannel::R] = 0x00FF0000;
  s_formatMetaData[nsImageFormat::B8G8R8A8_UNORM].m_uiChannelMasks[nsImageFormatChannel::G] = 0x0000FF00;
  s_formatMetaData[nsImageFormat::B8G8R8A8_UNORM].m_uiChannelMasks[nsImageFormatChannel::B] = 0x000000FF;
  s_formatMetaData[nsImageFormat::B8G8R8A8_UNORM].m_uiChannelMasks[nsImageFormatChannel::A] = 0xFF000000;

  INIT_FORMAT_LINEAR(B8G8R8X8_UNORM, UNORM, 32, 8, 8, 8, 0, 3);
  INIT_FORMAT_LINEAR(B8G8R8X8_UNORM_SRGB, UNORM, 32, 8, 8, 8, 0, 3);
  SetupSrgbPair(nsImageFormat::B8G8R8X8_UNORM, nsImageFormat::B8G8R8X8_UNORM_SRGB);

  s_formatMetaData[nsImageFormat::B8G8R8X8_UNORM].m_uiChannelMasks[nsImageFormatChannel::R] = 0x00FF0000;
  s_formatMetaData[nsImageFormat::B8G8R8X8_UNORM].m_uiChannelMasks[nsImageFormatChannel::G] = 0x0000FF00;
  s_formatMetaData[nsImageFormat::B8G8R8X8_UNORM].m_uiChannelMasks[nsImageFormatChannel::B] = 0x000000FF;
  s_formatMetaData[nsImageFormat::B8G8R8X8_UNORM].m_uiChannelMasks[nsImageFormatChannel::A] = 0x00000000;

  INIT_FORMAT_LINEAR(B8G8R8_UNORM, UNORM, 24, 8, 8, 8, 0, 3);
  INIT_FORMAT_LINEAR(B8G8R8_UNORM_SRGB, UNORM, 24, 8, 8, 8, 0, 3);
  SetupSrgbPair(nsImageFormat::B8G8R8_UNORM, nsImageFormat::B8G8R8_UNORM_SRGB);

  s_formatMetaData[nsImageFormat::B8G8R8_UNORM].m_uiChannelMasks[nsImageFormatChannel::R] = 0x00FF0000;
  s_formatMetaData[nsImageFormat::B8G8R8_UNORM].m_uiChannelMasks[nsImageFormatChannel::G] = 0x0000FF00;
  s_formatMetaData[nsImageFormat::B8G8R8_UNORM].m_uiChannelMasks[nsImageFormatChannel::B] = 0x000000FF;
  s_formatMetaData[nsImageFormat::B8G8R8_UNORM].m_uiChannelMasks[nsImageFormatChannel::A] = 0x00000000;

  INIT_FORMAT_LINEAR(R8G8_UINT, UINT, 16, 8, 8, 0, 0, 2);
  INIT_FORMAT_LINEAR(R8G8_SINT, SINT, 16, 8, 8, 0, 0, 2);
  INIT_FORMAT_LINEAR(R8G8_UNORM, UNORM, 16, 8, 8, 0, 0, 2);
  INIT_FORMAT_LINEAR(R8G8_SNORM, SNORM, 16, 8, 8, 0, 0, 2);

  INIT_FORMAT_LINEAR(R8_UINT, UINT, 8, 8, 0, 0, 0, 1);
  INIT_FORMAT_LINEAR(R8_SINT, SINT, 8, 8, 0, 0, 0, 1);
  INIT_FORMAT_LINEAR(R8_SNORM, SNORM, 8, 8, 0, 0, 0, 1);

  INIT_FORMAT_LINEAR(R8_UNORM, UNORM, 8, 8, 0, 0, 0, 1);
  s_formatMetaData[nsImageFormat::R8_UNORM].m_uiChannelMasks[nsImageFormatChannel::R] = 0xFF;
  s_formatMetaData[nsImageFormat::R8_UNORM].m_uiChannelMasks[nsImageFormatChannel::G] = 0x00;
  s_formatMetaData[nsImageFormat::R8_UNORM].m_uiChannelMasks[nsImageFormatChannel::B] = 0x00;
  s_formatMetaData[nsImageFormat::R8_UNORM].m_uiChannelMasks[nsImageFormatChannel::A] = 0x00;

  INIT_FORMAT_COMPRESSED(BC1_UNORM, UNORM, 64, 4, 4, 1, true, 4);
  INIT_FORMAT_COMPRESSED(BC1_UNORM_SRGB, UNORM, 64, 4, 4, 1, true, 4);
  SetupSrgbPair(nsImageFormat::BC1_UNORM, nsImageFormat::BC1_UNORM_SRGB);

  INIT_FORMAT_COMPRESSED(BC2_UNORM, UNORM, 128, 4, 4, 1, true, 4);
  INIT_FORMAT_COMPRESSED(BC2_UNORM_SRGB, UNORM, 128, 4, 4, 1, true, 4);
  SetupSrgbPair(nsImageFormat::BC2_UNORM, nsImageFormat::BC2_UNORM_SRGB);

  INIT_FORMAT_COMPRESSED(BC3_UNORM, UNORM, 128, 4, 4, 1, true, 4);
  INIT_FORMAT_COMPRESSED(BC3_UNORM_SRGB, UNORM, 128, 4, 4, 1, true, 4);
  SetupSrgbPair(nsImageFormat::BC3_UNORM, nsImageFormat::BC3_UNORM_SRGB);

  INIT_FORMAT_COMPRESSED(BC4_UNORM, UNORM, 64, 4, 4, 1, true, 1);
  INIT_FORMAT_COMPRESSED(BC4_SNORM, SNORM, 64, 4, 4, 1, true, 1);

  INIT_FORMAT_COMPRESSED(BC5_UNORM, UNORM, 128, 4, 4, 1, true, 2);
  INIT_FORMAT_COMPRESSED(BC5_SNORM, SNORM, 128, 4, 4, 1, true, 2);

  INIT_FORMAT_COMPRESSED(BC6H_UF16, FLOAT, 128, 4, 4, 1, true, 3);
  INIT_FORMAT_COMPRESSED(BC6H_SF16, FLOAT, 128, 4, 4, 1, true, 3);

  INIT_FORMAT_COMPRESSED(BC7_UNORM, UNORM, 128, 4, 4, 1, true, 4);
  INIT_FORMAT_COMPRESSED(BC7_UNORM_SRGB, UNORM, 128, 4, 4, 1, true, 4);
  SetupSrgbPair(nsImageFormat::BC7_UNORM, nsImageFormat::BC7_UNORM_SRGB);



  INIT_FORMAT_LINEAR(B5G5R5A1_UNORM, UNORM, 16, 5, 5, 5, 1, 4);
  INIT_FORMAT_LINEAR(B5G5R5A1_UNORM_SRGB, UNORM, 16, 5, 5, 5, 1, 4);
  SetupSrgbPair(nsImageFormat::B5G5R5A1_UNORM, nsImageFormat::B5G5R5A1_UNORM_SRGB);

  INIT_FORMAT_LINEAR(B4G4R4A4_UNORM, UNORM, 16, 4, 4, 4, 4, 4);
  s_formatMetaData[nsImageFormat::B4G4R4A4_UNORM].m_uiChannelMasks[nsImageFormatChannel::R] = 0x0F00;
  s_formatMetaData[nsImageFormat::B4G4R4A4_UNORM].m_uiChannelMasks[nsImageFormatChannel::G] = 0x00F0;
  s_formatMetaData[nsImageFormat::B4G4R4A4_UNORM].m_uiChannelMasks[nsImageFormatChannel::B] = 0x000F;
  s_formatMetaData[nsImageFormat::B4G4R4A4_UNORM].m_uiChannelMasks[nsImageFormatChannel::A] = 0xF000;
  INIT_FORMAT_LINEAR(B4G4R4A4_UNORM_SRGB, UNORM, 16, 4, 4, 4, 4, 4);
  SetupSrgbPair(nsImageFormat::B4G4R4A4_UNORM, nsImageFormat::B4G4R4A4_UNORM_SRGB);

  INIT_FORMAT_LINEAR(A4B4G4R4_UNORM, UNORM, 16, 4, 4, 4, 4, 4);
  s_formatMetaData[nsImageFormat::A4B4G4R4_UNORM].m_uiChannelMasks[nsImageFormatChannel::R] = 0xF000;
  s_formatMetaData[nsImageFormat::A4B4G4R4_UNORM].m_uiChannelMasks[nsImageFormatChannel::G] = 0x0F00;
  s_formatMetaData[nsImageFormat::A4B4G4R4_UNORM].m_uiChannelMasks[nsImageFormatChannel::B] = 0x00F0;
  s_formatMetaData[nsImageFormat::A4B4G4R4_UNORM].m_uiChannelMasks[nsImageFormatChannel::A] = 0x000F;
  INIT_FORMAT_LINEAR(A4B4G4R4_UNORM_SRGB, UNORM, 16, 4, 4, 4, 4, 4);
  SetupSrgbPair(nsImageFormat::A4B4G4R4_UNORM, nsImageFormat::A4B4G4R4_UNORM_SRGB);

  INIT_FORMAT_LINEAR(B5G6R5_UNORM, UNORM, 16, 5, 6, 5, 0, 3);
  s_formatMetaData[nsImageFormat::B5G6R5_UNORM].m_uiChannelMasks[nsImageFormatChannel::R] = 0xF800;
  s_formatMetaData[nsImageFormat::B5G6R5_UNORM].m_uiChannelMasks[nsImageFormatChannel::G] = 0x07E0;
  s_formatMetaData[nsImageFormat::B5G6R5_UNORM].m_uiChannelMasks[nsImageFormatChannel::B] = 0x001F;
  s_formatMetaData[nsImageFormat::B5G6R5_UNORM].m_uiChannelMasks[nsImageFormatChannel::A] = 0x0000;
  INIT_FORMAT_LINEAR(B5G6R5_UNORM_SRGB, UNORM, 16, 5, 6, 5, 0, 3);
  SetupSrgbPair(nsImageFormat::B5G6R5_UNORM, nsImageFormat::B5G6R5_UNORM_SRGB);

  INIT_FORMAT_LINEAR(B5G5R5A1_UNORM, UNORM, 16, 5, 5, 5, 1, 3);
  s_formatMetaData[nsImageFormat::B5G5R5A1_UNORM].m_uiChannelMasks[nsImageFormatChannel::R] = 0x7C00;
  s_formatMetaData[nsImageFormat::B5G5R5A1_UNORM].m_uiChannelMasks[nsImageFormatChannel::G] = 0x03E0;
  s_formatMetaData[nsImageFormat::B5G5R5A1_UNORM].m_uiChannelMasks[nsImageFormatChannel::B] = 0x001F;
  s_formatMetaData[nsImageFormat::B5G5R5A1_UNORM].m_uiChannelMasks[nsImageFormatChannel::A] = 0x8000;
  INIT_FORMAT_LINEAR(B5G5R5A1_UNORM_SRGB, UNORM, 16, 5, 5, 5, 1, 3);
  SetupSrgbPair(nsImageFormat::B5G5R5A1_UNORM, nsImageFormat::B5G5R5A1_UNORM_SRGB);

  INIT_FORMAT_LINEAR(B5G5R5X1_UNORM, UNORM, 16, 5, 5, 5, 0, 3);
  s_formatMetaData[nsImageFormat::B5G5R5X1_UNORM].m_uiChannelMasks[nsImageFormatChannel::R] = 0x7C00;
  s_formatMetaData[nsImageFormat::B5G5R5X1_UNORM].m_uiChannelMasks[nsImageFormatChannel::G] = 0x03E0;
  s_formatMetaData[nsImageFormat::B5G5R5X1_UNORM].m_uiChannelMasks[nsImageFormatChannel::B] = 0x001F;
  s_formatMetaData[nsImageFormat::B5G5R5X1_UNORM].m_uiChannelMasks[nsImageFormatChannel::A] = 0x0000;
  INIT_FORMAT_LINEAR(B5G5R5X1_UNORM_SRGB, UNORM, 16, 5, 5, 5, 0, 3);
  SetupSrgbPair(nsImageFormat::B5G5R5X1_UNORM, nsImageFormat::B5G5R5X1_UNORM_SRGB);

  INIT_FORMAT_LINEAR(A1B5G5R5_UNORM, UNORM, 16, 5, 5, 5, 1, 3);
  s_formatMetaData[nsImageFormat::A1B5G5R5_UNORM].m_uiChannelMasks[nsImageFormatChannel::R] = 0x7C00;
  s_formatMetaData[nsImageFormat::A1B5G5R5_UNORM].m_uiChannelMasks[nsImageFormatChannel::G] = 0x03E0;
  s_formatMetaData[nsImageFormat::A1B5G5R5_UNORM].m_uiChannelMasks[nsImageFormatChannel::B] = 0x001F;
  s_formatMetaData[nsImageFormat::A1B5G5R5_UNORM].m_uiChannelMasks[nsImageFormatChannel::A] = 0x8000;
  INIT_FORMAT_LINEAR(A1B5G5R5_UNORM_SRGB, UNORM, 16, 5, 5, 5, 1, 3);
  SetupSrgbPair(nsImageFormat::A1B5G5R5_UNORM, nsImageFormat::A1B5G5R5_UNORM_SRGB);

  INIT_FORMAT_LINEAR(X1B5G5R5_UNORM, UNORM, 16, 5, 5, 5, 1, 3);
  s_formatMetaData[nsImageFormat::X1B5G5R5_UNORM].m_uiChannelMasks[nsImageFormatChannel::R] = 0x7C00;
  s_formatMetaData[nsImageFormat::X1B5G5R5_UNORM].m_uiChannelMasks[nsImageFormatChannel::G] = 0x03E0;
  s_formatMetaData[nsImageFormat::X1B5G5R5_UNORM].m_uiChannelMasks[nsImageFormatChannel::B] = 0x001F;
  s_formatMetaData[nsImageFormat::X1B5G5R5_UNORM].m_uiChannelMasks[nsImageFormatChannel::A] = 0x0000;
  INIT_FORMAT_LINEAR(X1B5G5R5_UNORM_SRGB, UNORM, 16, 5, 5, 5, 1, 3);
  SetupSrgbPair(nsImageFormat::X1B5G5R5_UNORM, nsImageFormat::X1B5G5R5_UNORM_SRGB);

  INIT_FORMAT_LINEAR(R11G11B10_FLOAT, FLOAT, 32, 11, 11, 10, 0, 3);
  INIT_FORMAT_LINEAR(R10G10B10A2_UINT, UINT, 32, 10, 10, 10, 2, 4);
  INIT_FORMAT_LINEAR(R10G10B10A2_UNORM, UNORM, 32, 10, 10, 10, 2, 4);

  // msdn.microsoft.com/library/windows/desktop/bb943991(v=vs.85).aspx documents R10G10B10A2 as having an alpha mask of 0
  s_formatMetaData[nsImageFormat::R10G10B10A2_UNORM].m_uiChannelMasks[nsImageFormatChannel::R] = 0x000003FF;
  s_formatMetaData[nsImageFormat::R10G10B10A2_UNORM].m_uiChannelMasks[nsImageFormatChannel::G] = 0x000FFC00;
  s_formatMetaData[nsImageFormat::R10G10B10A2_UNORM].m_uiChannelMasks[nsImageFormatChannel::B] = 0x3FF00000;
  s_formatMetaData[nsImageFormat::R10G10B10A2_UNORM].m_uiChannelMasks[nsImageFormatChannel::A] = 0;

  INIT_FORMAT_DEPTH(D32_FLOAT, DEPTH_STENCIL, 32, false, 32, 0);
  INIT_FORMAT_DEPTH(D32_FLOAT_S8X24_UINT, DEPTH_STENCIL, 64, true, 32, 8);
  INIT_FORMAT_DEPTH(D24_UNORM_S8_UINT, DEPTH_STENCIL, 32, true, 24, 8);
  INIT_FORMAT_DEPTH(D16_UNORM, DEPTH_STENCIL, 16, false, 16, 0);

  INIT_FORMAT_COMPRESSED(ASTC_4x4_UNORM, UNORM, 128, 4, 4, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_5x4_UNORM, UNORM, 128, 5, 4, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_5x5_UNORM, UNORM, 128, 5, 5, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_6x5_UNORM, UNORM, 128, 6, 5, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_6x6_UNORM, UNORM, 128, 6, 6, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_8x5_UNORM, UNORM, 128, 8, 5, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_8x6_UNORM, UNORM, 128, 8, 6, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_10x5_UNORM, UNORM, 128, 10, 5, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_10x6_UNORM, UNORM, 128, 10, 6, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_8x8_UNORM, UNORM, 128, 8, 8, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_10x8_UNORM, UNORM, 128, 10, 8, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_10x10_UNORM, UNORM, 128, 10, 10, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_12x10_UNORM, UNORM, 128, 12, 10, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_12x12_UNORM, UNORM, 128, 12, 12, 1, false, 4);

  INIT_FORMAT_COMPRESSED(ASTC_4x4_UNORM_SRGB, UNORM, 128, 4, 4, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_5x4_UNORM_SRGB, UNORM, 128, 5, 4, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_5x5_UNORM_SRGB, UNORM, 128, 5, 5, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_6x5_UNORM_SRGB, UNORM, 128, 6, 5, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_6x6_UNORM_SRGB, UNORM, 128, 6, 6, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_8x5_UNORM_SRGB, UNORM, 128, 8, 5, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_8x6_UNORM_SRGB, UNORM, 128, 8, 6, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_10x5_UNORM_SRGB, UNORM, 128, 10, 5, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_10x6_UNORM_SRGB, UNORM, 128, 10, 6, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_8x8_UNORM_SRGB, UNORM, 128, 8, 8, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_10x8_UNORM_SRGB, UNORM, 128, 10, 8, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_10x10_UNORM_SRGB, UNORM, 128, 10, 10, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_12x10_UNORM_SRGB, UNORM, 128, 12, 10, 1, false, 4);
  INIT_FORMAT_COMPRESSED(ASTC_12x12_UNORM_SRGB, UNORM, 128, 12, 12, 1, false, 4);

  SetupSrgbPair(nsImageFormat::ASTC_4x4_UNORM, nsImageFormat::ASTC_4x4_UNORM_SRGB);
  SetupSrgbPair(nsImageFormat::ASTC_5x4_UNORM, nsImageFormat::ASTC_5x4_UNORM_SRGB);
  SetupSrgbPair(nsImageFormat::ASTC_5x5_UNORM, nsImageFormat::ASTC_5x5_UNORM_SRGB);
  SetupSrgbPair(nsImageFormat::ASTC_6x5_UNORM, nsImageFormat::ASTC_6x5_UNORM_SRGB);
  SetupSrgbPair(nsImageFormat::ASTC_6x6_UNORM, nsImageFormat::ASTC_6x6_UNORM_SRGB);
  SetupSrgbPair(nsImageFormat::ASTC_8x5_UNORM, nsImageFormat::ASTC_8x5_UNORM_SRGB);
  SetupSrgbPair(nsImageFormat::ASTC_8x6_UNORM, nsImageFormat::ASTC_8x6_UNORM_SRGB);
  SetupSrgbPair(nsImageFormat::ASTC_10x5_UNORM, nsImageFormat::ASTC_10x5_UNORM_SRGB);
  SetupSrgbPair(nsImageFormat::ASTC_10x6_UNORM, nsImageFormat::ASTC_10x6_UNORM_SRGB);
  SetupSrgbPair(nsImageFormat::ASTC_8x8_UNORM, nsImageFormat::ASTC_8x8_UNORM_SRGB);
  SetupSrgbPair(nsImageFormat::ASTC_10x8_UNORM, nsImageFormat::ASTC_10x8_UNORM_SRGB);
  SetupSrgbPair(nsImageFormat::ASTC_10x10_UNORM, nsImageFormat::ASTC_10x10_UNORM_SRGB);
  SetupSrgbPair(nsImageFormat::ASTC_12x10_UNORM, nsImageFormat::ASTC_12x10_UNORM_SRGB);
  SetupSrgbPair(nsImageFormat::ASTC_12x12_UNORM, nsImageFormat::ASTC_12x12_UNORM_SRGB);

  s_formatMetaData[nsImageFormat::NV12].m_szName = "NV12";
  s_formatMetaData[nsImageFormat::NV12].m_formatType = nsImageFormatType::PLANAR;
  s_formatMetaData[nsImageFormat::NV12].m_uiNumChannels = 3;

  s_formatMetaData[nsImageFormat::NV12].m_planeData.SetCount(2);

  s_formatMetaData[nsImageFormat::NV12].m_planeData[0].m_uiBitsPerBlock = 8;
  s_formatMetaData[nsImageFormat::NV12].m_planeData[0].m_uiBlockWidth = 1;
  s_formatMetaData[nsImageFormat::NV12].m_planeData[0].m_uiBlockHeight = 1;
  s_formatMetaData[nsImageFormat::NV12].m_planeData[0].m_uiBlockDepth = 1;
  s_formatMetaData[nsImageFormat::NV12].m_planeData[0].m_subFormat = nsImageFormat::R8_UNORM;

  s_formatMetaData[nsImageFormat::NV12].m_planeData[1].m_uiBitsPerBlock = 16;
  s_formatMetaData[nsImageFormat::NV12].m_planeData[1].m_uiBlockWidth = 2;
  s_formatMetaData[nsImageFormat::NV12].m_planeData[1].m_uiBlockHeight = 2;
  s_formatMetaData[nsImageFormat::NV12].m_planeData[1].m_uiBlockDepth = 1;
  s_formatMetaData[nsImageFormat::NV12].m_planeData[1].m_subFormat = nsImageFormat::R8G8_UNORM;
}

static const NS_ALWAYS_INLINE nsImageFormatMetaData& GetImageFormatMetaData(nsImageFormat::Enum format)
{
  if (s_formatMetaData.IsEmpty())
  {
    SetupImageFormatTable();
  }

  return s_formatMetaData[format];
}

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(Image, ImageFormats)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    SetupImageFormatTable();
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

nsUInt32 nsImageFormat::GetBitsPerPixel(Enum format, nsUInt32 uiPlaneIndex)
{
  const nsImageFormatMetaData& metaData = GetImageFormatMetaData(format);
  auto pixelsPerBlock = metaData.m_planeData[uiPlaneIndex].m_uiBlockWidth * metaData.m_planeData[uiPlaneIndex].m_uiBlockHeight * metaData.m_planeData[uiPlaneIndex].m_uiBlockDepth;
  return (metaData.m_planeData[uiPlaneIndex].m_uiBitsPerBlock + pixelsPerBlock - 1) / pixelsPerBlock; // Return rounded-up value
}


float nsImageFormat::GetExactBitsPerPixel(Enum format, nsUInt32 uiPlaneIndex)
{
  const nsImageFormatMetaData& metaData = GetImageFormatMetaData(format);
  auto pixelsPerBlock = metaData.m_planeData[uiPlaneIndex].m_uiBlockWidth * metaData.m_planeData[uiPlaneIndex].m_uiBlockHeight * metaData.m_planeData[uiPlaneIndex].m_uiBlockDepth;
  return static_cast<float>(metaData.m_planeData[uiPlaneIndex].m_uiBitsPerBlock) / pixelsPerBlock;
}


nsUInt32 nsImageFormat::GetBitsPerBlock(Enum format, nsUInt32 uiPlaneIndex)
{
  return GetImageFormatMetaData(format).m_planeData[uiPlaneIndex].m_uiBitsPerBlock;
}


nsUInt32 nsImageFormat::GetNumChannels(Enum format)
{
  return GetImageFormatMetaData(format).m_uiNumChannels;
}

nsImageFormat::Enum nsImageFormat::FromPixelMask(
  nsUInt32 uiRedMask, nsUInt32 uiGreenMask, nsUInt32 uiBlueMask, nsUInt32 uiAlphaMask, nsUInt32 uiBitsPerPixel)
{
  // Some DDS files in the wild are encoded as this
  if (uiBitsPerPixel == 8 && uiRedMask == 0xff && uiGreenMask == 0xff && uiBlueMask == 0xff)
  {
    return R8_UNORM;
  }

  for (nsUInt32 index = 0; index < NUM_FORMATS; index++)
  {
    Enum format = static_cast<Enum>(index);
    if (GetChannelMask(format, nsImageFormatChannel::R) == uiRedMask && GetChannelMask(format, nsImageFormatChannel::G) == uiGreenMask &&
        GetChannelMask(format, nsImageFormatChannel::B) == uiBlueMask && GetChannelMask(format, nsImageFormatChannel::A) == uiAlphaMask &&
        GetBitsPerPixel(format) == uiBitsPerPixel && GetDataType(format) == nsImageFormatDataType::UNORM && !IsCompressed(format))
    {
      return format;
    }
  }

  return UNKNOWN;
}


nsImageFormat::Enum nsImageFormat::GetPlaneSubFormat(Enum format, nsUInt32 uiPlaneIndex)
{
  const auto& metadata = GetImageFormatMetaData(format);

  if (metadata.m_formatType == nsImageFormatType::PLANAR)
  {
    return metadata.m_planeData[uiPlaneIndex].m_subFormat;
  }
  else
  {
    NS_ASSERT_DEV(uiPlaneIndex == 0, "Invalid plane index {0} for format {0}", uiPlaneIndex, nsImageFormat::GetName(format));
    return format;
  }
}

bool nsImageFormat::IsCompatible(Enum left, Enum right)
{
  if (left == right)
  {
    return true;
  }
  switch (left)
  {
    case nsImageFormat::R32G32B32A32_FLOAT:
    case nsImageFormat::R32G32B32A32_UINT:
    case nsImageFormat::R32G32B32A32_SINT:
      return (right == nsImageFormat::R32G32B32A32_FLOAT || right == nsImageFormat::R32G32B32A32_UINT || right == nsImageFormat::R32G32B32A32_SINT);
    case nsImageFormat::R32G32B32_FLOAT:
    case nsImageFormat::R32G32B32_UINT:
    case nsImageFormat::R32G32B32_SINT:
      return (right == nsImageFormat::R32G32B32_FLOAT || right == nsImageFormat::R32G32B32_UINT || right == nsImageFormat::R32G32B32_SINT);
    case nsImageFormat::R32G32_FLOAT:
    case nsImageFormat::R32G32_UINT:
    case nsImageFormat::R32G32_SINT:
      return (right == nsImageFormat::R32G32_FLOAT || right == nsImageFormat::R32G32_UINT || right == nsImageFormat::R32G32_SINT);
    case nsImageFormat::R32_FLOAT:
    case nsImageFormat::R32_UINT:
    case nsImageFormat::R32_SINT:
      return (right == nsImageFormat::R32_FLOAT || right == nsImageFormat::R32_UINT || right == nsImageFormat::R32_SINT);
    case nsImageFormat::R16G16B16A16_FLOAT:
    case nsImageFormat::R16G16B16A16_UINT:
    case nsImageFormat::R16G16B16A16_SINT:
    case nsImageFormat::R16G16B16A16_UNORM:
    case nsImageFormat::R16G16B16A16_SNORM:
      return (right == nsImageFormat::R16G16B16A16_FLOAT || right == nsImageFormat::R16G16B16A16_UINT || right == nsImageFormat::R16G16B16A16_SINT ||
              right == nsImageFormat::R16G16B16A16_UNORM || right == nsImageFormat::R16G16B16A16_SNORM);
    case nsImageFormat::R16G16_FLOAT:
    case nsImageFormat::R16G16_UINT:
    case nsImageFormat::R16G16_SINT:
    case nsImageFormat::R16G16_UNORM:
    case nsImageFormat::R16G16_SNORM:
      return (right == nsImageFormat::R16G16_FLOAT || right == nsImageFormat::R16G16_UINT || right == nsImageFormat::R16G16_SINT ||
              right == nsImageFormat::R16G16_UNORM || right == nsImageFormat::R16G16_SNORM);
    case nsImageFormat::R8G8B8A8_UINT:
    case nsImageFormat::R8G8B8A8_SINT:
    case nsImageFormat::R8G8B8A8_UNORM:
    case nsImageFormat::R8G8B8A8_SNORM:
    case nsImageFormat::R8G8B8A8_UNORM_SRGB:
      return (right == nsImageFormat::R8G8B8A8_UINT || right == nsImageFormat::R8G8B8A8_SINT || right == nsImageFormat::R8G8B8A8_UNORM ||
              right == nsImageFormat::R8G8B8A8_SNORM || right == nsImageFormat::R8G8B8A8_UNORM_SRGB);
    case nsImageFormat::B8G8R8A8_UNORM:
    case nsImageFormat::B8G8R8A8_UNORM_SRGB:
      return (right == nsImageFormat::B8G8R8A8_UNORM || right == nsImageFormat::B8G8R8A8_UNORM_SRGB);
    case nsImageFormat::B8G8R8X8_UNORM:
    case nsImageFormat::B8G8R8X8_UNORM_SRGB:
      return (right == nsImageFormat::B8G8R8X8_UNORM || right == nsImageFormat::B8G8R8X8_UNORM_SRGB);
    case nsImageFormat::B8G8R8_UNORM:
    case nsImageFormat::B8G8R8_UNORM_SRGB:
      return (right == nsImageFormat::B8G8R8_UNORM || right == nsImageFormat::B8G8R8_UNORM_SRGB);
    case nsImageFormat::R8G8_UINT:
    case nsImageFormat::R8G8_SINT:
    case nsImageFormat::R8G8_UNORM:
    case nsImageFormat::R8G8_SNORM:
      return (right == nsImageFormat::R8G8_UINT || right == nsImageFormat::R8G8_SINT || right == nsImageFormat::R8G8_UNORM ||
              right == nsImageFormat::R8G8_SNORM);
    case nsImageFormat::R8_UINT:
    case nsImageFormat::R8_SINT:
    case nsImageFormat::R8_UNORM:
    case nsImageFormat::R8_SNORM:
      return (
        right == nsImageFormat::R8_UINT || right == nsImageFormat::R8_SINT || right == nsImageFormat::R8_UNORM || right == nsImageFormat::R8_SNORM);
    case nsImageFormat::BC1_UNORM:
    case nsImageFormat::BC1_UNORM_SRGB:
      return (right == nsImageFormat::BC1_UNORM || right == nsImageFormat::BC1_UNORM_SRGB);
    case nsImageFormat::BC2_UNORM:
    case nsImageFormat::BC2_UNORM_SRGB:
      return (right == nsImageFormat::BC2_UNORM || right == nsImageFormat::BC2_UNORM_SRGB);
    case nsImageFormat::BC3_UNORM:
    case nsImageFormat::BC3_UNORM_SRGB:
      return (right == nsImageFormat::BC3_UNORM || right == nsImageFormat::BC3_UNORM_SRGB);
    case nsImageFormat::BC4_UNORM:
    case nsImageFormat::BC4_SNORM:
      return (right == nsImageFormat::BC4_UNORM || right == nsImageFormat::BC4_SNORM);
    case nsImageFormat::BC5_UNORM:
    case nsImageFormat::BC5_SNORM:
      return (right == nsImageFormat::BC5_UNORM || right == nsImageFormat::BC5_SNORM);
    case nsImageFormat::BC6H_UF16:
    case nsImageFormat::BC6H_SF16:
      return (right == nsImageFormat::BC6H_UF16 || right == nsImageFormat::BC6H_SF16);
    case nsImageFormat::BC7_UNORM:
    case nsImageFormat::BC7_UNORM_SRGB:
      return (right == nsImageFormat::BC7_UNORM || right == nsImageFormat::BC7_UNORM_SRGB);
    case nsImageFormat::R10G10B10A2_UINT:
    case nsImageFormat::R10G10B10A2_UNORM:
      return (right == nsImageFormat::R10G10B10A2_UINT || right == nsImageFormat::R10G10B10A2_UNORM);
    case nsImageFormat::ASTC_4x4_UNORM:
    case nsImageFormat::ASTC_4x4_UNORM_SRGB:
      return (right == nsImageFormat::ASTC_4x4_UNORM || right == nsImageFormat::ASTC_4x4_UNORM_SRGB);
    case nsImageFormat::ASTC_5x4_UNORM:
    case nsImageFormat::ASTC_5x4_UNORM_SRGB:
      return (right == nsImageFormat::ASTC_5x4_UNORM || right == nsImageFormat::ASTC_5x4_UNORM_SRGB);
    case nsImageFormat::ASTC_5x5_UNORM:
    case nsImageFormat::ASTC_5x5_UNORM_SRGB:
      return (right == nsImageFormat::ASTC_5x5_UNORM || right == nsImageFormat::ASTC_5x5_UNORM_SRGB);
    case nsImageFormat::ASTC_6x5_UNORM:
    case nsImageFormat::ASTC_6x5_UNORM_SRGB:
      return (right == nsImageFormat::ASTC_6x5_UNORM || right == nsImageFormat::ASTC_6x5_UNORM_SRGB);
    case nsImageFormat::ASTC_6x6_UNORM:
    case nsImageFormat::ASTC_6x6_UNORM_SRGB:
      return (right == nsImageFormat::ASTC_6x6_UNORM || right == nsImageFormat::ASTC_6x6_UNORM_SRGB);
    case nsImageFormat::ASTC_8x5_UNORM:
    case nsImageFormat::ASTC_8x5_UNORM_SRGB:
      return (right == nsImageFormat::ASTC_8x5_UNORM || right == nsImageFormat::ASTC_8x5_UNORM_SRGB);
    case nsImageFormat::ASTC_8x6_UNORM:
    case nsImageFormat::ASTC_8x6_UNORM_SRGB:
      return (right == nsImageFormat::ASTC_8x6_UNORM || right == nsImageFormat::ASTC_8x6_UNORM_SRGB);
    case nsImageFormat::ASTC_10x5_UNORM:
    case nsImageFormat::ASTC_10x5_UNORM_SRGB:
      return (right == nsImageFormat::ASTC_10x5_UNORM || right == nsImageFormat::ASTC_10x5_UNORM_SRGB);
    case nsImageFormat::ASTC_10x6_UNORM:
    case nsImageFormat::ASTC_10x6_UNORM_SRGB:
      return (right == nsImageFormat::ASTC_10x6_UNORM || right == nsImageFormat::ASTC_10x6_UNORM_SRGB);
    case nsImageFormat::ASTC_8x8_UNORM:
    case nsImageFormat::ASTC_8x8_UNORM_SRGB:
      return (right == nsImageFormat::ASTC_8x8_UNORM || right == nsImageFormat::ASTC_8x8_UNORM_SRGB);
    case nsImageFormat::ASTC_10x8_UNORM:
    case nsImageFormat::ASTC_10x8_UNORM_SRGB:
      return (right == nsImageFormat::ASTC_10x8_UNORM || right == nsImageFormat::ASTC_10x8_UNORM_SRGB);
    case nsImageFormat::ASTC_10x10_UNORM:
    case nsImageFormat::ASTC_10x10_UNORM_SRGB:
      return (right == nsImageFormat::ASTC_10x10_UNORM || right == nsImageFormat::ASTC_10x10_UNORM_SRGB);
    case nsImageFormat::ASTC_12x10_UNORM:
    case nsImageFormat::ASTC_12x10_UNORM_SRGB:
      return (right == nsImageFormat::ASTC_12x10_UNORM || right == nsImageFormat::ASTC_12x10_UNORM_SRGB);
    case nsImageFormat::ASTC_12x12_UNORM:
    case nsImageFormat::ASTC_12x12_UNORM_SRGB:
      return (right == nsImageFormat::ASTC_12x12_UNORM || right == nsImageFormat::ASTC_12x12_UNORM_SRGB);
    default:
      NS_ASSERT_DEV(false, "Encountered unhandled format: {0}", nsImageFormat::GetName(left));
      return false;
  }
}


bool nsImageFormat::RequiresFirstLevelBlockAlignment(Enum format)
{
  return GetImageFormatMetaData(format).m_requireFirstLevelBlockAligned;
}

const char* nsImageFormat::GetName(Enum format)
{
  return GetImageFormatMetaData(format).m_szName;
}

nsUInt32 nsImageFormat::GetPlaneCount(Enum format)
{
  return GetImageFormatMetaData(format).m_planeData.GetCount();
}

nsUInt32 nsImageFormat::GetChannelMask(Enum format, nsImageFormatChannel::Enum c)
{
  return GetImageFormatMetaData(format).m_uiChannelMasks[c];
}

nsUInt32 nsImageFormat::GetBitsPerChannel(Enum format, nsImageFormatChannel::Enum c)
{
  return GetImageFormatMetaData(format).m_uiBitsPerChannel[c];
}

nsUInt32 nsImageFormat::GetRedMask(Enum format)
{
  return GetImageFormatMetaData(format).m_uiChannelMasks[nsImageFormatChannel::R];
}

nsUInt32 nsImageFormat::GetGreenMask(Enum format)
{
  return GetImageFormatMetaData(format).m_uiChannelMasks[nsImageFormatChannel::G];
}

nsUInt32 nsImageFormat::GetBlueMask(Enum format)
{
  return GetImageFormatMetaData(format).m_uiChannelMasks[nsImageFormatChannel::B];
}

nsUInt32 nsImageFormat::GetAlphaMask(Enum format)
{
  return GetImageFormatMetaData(format).m_uiChannelMasks[nsImageFormatChannel::A];
}

nsUInt32 nsImageFormat::GetBlockWidth(Enum format, nsUInt32 uiPlaneIndex)
{
  return GetImageFormatMetaData(format).m_planeData[uiPlaneIndex].m_uiBlockWidth;
}

nsUInt32 nsImageFormat::GetBlockHeight(Enum format, nsUInt32 uiPlaneIndex)
{
  return GetImageFormatMetaData(format).m_planeData[uiPlaneIndex].m_uiBlockHeight;
}

nsUInt32 nsImageFormat::GetBlockDepth(Enum format, nsUInt32 uiPlaneIndex)
{
  return GetImageFormatMetaData(format).m_planeData[uiPlaneIndex].m_uiBlockDepth;
}

nsImageFormatDataType::Enum nsImageFormat::GetDataType(Enum format)
{
  return GetImageFormatMetaData(format).m_dataType;
}

bool nsImageFormat::IsCompressed(Enum format)
{
  return GetImageFormatMetaData(format).m_formatType == nsImageFormatType::BLOCK_COMPRESSED;
}

bool nsImageFormat::IsDepth(Enum format)
{
  return GetImageFormatMetaData(format).m_isDepth;
}

bool nsImageFormat::IsSrgb(Enum format)
{
  return GetImageFormatMetaData(format).m_asLinear != format;
}

bool nsImageFormat::IsStencil(Enum format)
{
  return GetImageFormatMetaData(format).m_isStencil;
}

nsImageFormat::Enum nsImageFormat::AsSrgb(Enum format)
{
  return GetImageFormatMetaData(format).m_asSrgb;
}

nsImageFormat::Enum nsImageFormat::AsLinear(Enum format)
{
  return GetImageFormatMetaData(format).m_asLinear;
}

nsUInt32 nsImageFormat::GetNumBlocksX(Enum format, nsUInt32 uiWidth, nsUInt32 uiPlaneIndex)
{
  return (uiWidth - 1) / GetBlockWidth(format, uiPlaneIndex) + 1;
}

nsUInt32 nsImageFormat::GetNumBlocksY(Enum format, nsUInt32 uiHeight, nsUInt32 uiPlaneIndex)
{
  return (uiHeight - 1) / GetBlockHeight(format, uiPlaneIndex) + 1;
}

nsUInt32 nsImageFormat::GetNumBlocksZ(Enum format, nsUInt32 uiDepth, nsUInt32 uiPlaneIndex)
{
  return (uiDepth - 1) / GetBlockDepth(format, uiPlaneIndex) + 1;
}

nsUInt64 nsImageFormat::GetRowPitch(Enum format, nsUInt32 uiWidth, nsUInt32 uiPlaneIndex)
{
  return static_cast<nsUInt64>(GetNumBlocksX(format, uiWidth, uiPlaneIndex)) * GetBitsPerBlock(format, uiPlaneIndex) / 8;
}

nsUInt64 nsImageFormat::GetDepthPitch(Enum format, nsUInt32 uiWidth, nsUInt32 uiHeight, nsUInt32 uiPlaneIndex)
{
  return static_cast<nsUInt64>(GetNumBlocksY(format, uiHeight, uiPlaneIndex)) * static_cast<nsUInt64>(GetRowPitch(format, uiWidth, uiPlaneIndex));
}

nsImageFormatType::Enum nsImageFormat::GetType(Enum format)
{
  return GetImageFormatMetaData(format).m_formatType;
}

NS_STATICLINK_FILE(Texture, Texture_Image_Implementation_ImageFormat);

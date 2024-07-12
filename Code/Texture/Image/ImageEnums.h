#pragma once

#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Types/Types.h>
#include <Texture/TextureDLL.h>

struct NS_TEXTURE_DLL nsImageAddressMode
{
  using StorageType = nsUInt8;

  enum Enum
  {
    Repeat,
    Clamp,
    ClampBorder,
    Mirror,

    ENUM_COUNT,

    Default = Repeat
  };
};

NS_DECLARE_REFLECTABLE_TYPE(NS_TEXTURE_DLL, nsImageAddressMode);

//////////////////////////////////////////////////////////////////////////
// nsTextureFilterSetting
//////////////////////////////////////////////////////////////////////////

struct NS_TEXTURE_DLL nsTextureFilterSetting
{
  using StorageType = nsUInt8;

  enum Enum
  {
    FixedNearest,
    FixedBilinear,
    FixedTrilinear,
    FixedAnisotropic2x,
    FixedAnisotropic4x,
    FixedAnisotropic8x,
    FixedAnisotropic16x,

    LowestQuality,
    LowQuality,
    DefaultQuality,
    HighQuality,
    HighestQuality,

    Default = DefaultQuality
  };
};

NS_DECLARE_REFLECTABLE_TYPE(NS_TEXTURE_DLL, nsTextureFilterSetting);

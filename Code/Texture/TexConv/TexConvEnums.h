#pragma once

#include <Texture/TextureDLL.h>

#include <Foundation/Reflection/Reflection.h>

struct nsTexConvOutputType
{
  enum Enum
  {
    None,
    Texture2D,
    Volume,
    Cubemap,
    Atlas,

    Default = Texture2D
  };

  using StorageType = nsUInt8;
};

struct nsTexConvCompressionMode
{
  enum Enum
  {
    // note: order of enum values matters
    None = 0,   // uncompressed
    Medium = 1, // compressed with high quality, if possible
    High = 2,   // strongest compression, if possible

    Default = Medium,
  };

  using StorageType = nsUInt8;
};

NS_DECLARE_REFLECTABLE_TYPE(NS_TEXTURE_DLL, nsTexConvCompressionMode);

struct nsTexConvUsage
{
  enum Enum
  {
    Auto, ///< Target format will be detected from heuristics (filename, content)

    // Exact format will be decided together with nsTexConvCompressionMode

    Color,
    Linear,
    Hdr,

    NormalMap,
    NormalMap_Inverted,

    BumpMap,

    Default = Auto
  };

  using StorageType = nsUInt8;
};

NS_DECLARE_REFLECTABLE_TYPE(NS_TEXTURE_DLL, nsTexConvUsage);

struct nsTexConvMipmapMode
{
  enum Enum
  {
    None, ///< Mipmap generation is disabled, output will have no mipmaps
    Linear,
    Kaiser,

    Default = Kaiser
  };

  using StorageType = nsUInt8;
};

NS_DECLARE_REFLECTABLE_TYPE(NS_TEXTURE_DLL, nsTexConvMipmapMode);

struct nsTexConvTargetPlatform
{
  enum Enum
  {
    PC,
    Android,

    Default = PC
  };

  using StorageType = nsUInt8;
};

/// \brief Defines which channel of another texture to read to get a value
struct nsTexConvChannelValue
{
  enum Enum
  {
    Red,   ///< read the RED channel
    Green, ///< read the GREEN channel
    Blue,  ///< read the BLUE channel
    Alpha, ///< read the ALPHA channel

    Black, ///< don't read any channel, just take the constant value 0
    White, ///< don't read any channel, just take the constant value 0xFF / 1.0f
  };
};

/// \brief Defines which filter kernel is used to approximate the x/y bump map gradients
struct nsTexConvBumpMapFilter
{
  enum Enum
  {
    Finite, ///< Simple finite differences in a 4-Neighborhood
    Sobel,  ///< Sobel kernel (8-Neighborhood)
    Scharr, ///< Scharr kernel (8-Neighborhood)

    Default = Finite
  };

  using StorageType = nsUInt8;
};

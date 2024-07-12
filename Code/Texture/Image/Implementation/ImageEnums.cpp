#include <Texture/TexturePCH.h>

#include <Texture/Image/ImageEnums.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_ENUM(nsImageAddressMode, 1)
  NS_ENUM_CONSTANT(nsImageAddressMode::Repeat),
  NS_ENUM_CONSTANT(nsImageAddressMode::Clamp),
  NS_ENUM_CONSTANT(nsImageAddressMode::ClampBorder),
  NS_ENUM_CONSTANT(nsImageAddressMode::Mirror),
NS_END_STATIC_REFLECTED_ENUM;
// clang-format on

// clang-format off
NS_BEGIN_STATIC_REFLECTED_ENUM(nsTextureFilterSetting, 1)
  NS_ENUM_CONSTANT(nsTextureFilterSetting::FixedNearest),
  NS_ENUM_CONSTANT(nsTextureFilterSetting::FixedBilinear),
  NS_ENUM_CONSTANT(nsTextureFilterSetting::FixedTrilinear),
  NS_ENUM_CONSTANT(nsTextureFilterSetting::FixedAnisotropic2x),
  NS_ENUM_CONSTANT(nsTextureFilterSetting::FixedAnisotropic4x),
  NS_ENUM_CONSTANT(nsTextureFilterSetting::FixedAnisotropic8x),
  NS_ENUM_CONSTANT(nsTextureFilterSetting::FixedAnisotropic16x),
  NS_ENUM_CONSTANT(nsTextureFilterSetting::LowestQuality),
  NS_ENUM_CONSTANT(nsTextureFilterSetting::LowQuality),
  NS_ENUM_CONSTANT(nsTextureFilterSetting::DefaultQuality),
  NS_ENUM_CONSTANT(nsTextureFilterSetting::HighQuality),
  NS_ENUM_CONSTANT(nsTextureFilterSetting::HighestQuality),
NS_END_STATIC_REFLECTED_ENUM;
// clang-format on


NS_STATICLINK_FILE(Texture, Texture_Image_Implementation_ImageEnums);

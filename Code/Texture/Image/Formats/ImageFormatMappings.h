#pragma once

#include <Texture/Image/ImageFormat.h>

/// \brief Helper class containing methods to convert between nsImageFormat::Enum and platform-specific image formats.
class NS_TEXTURE_DLL nsImageFormatMappings
{
public:
  /// \brief Maps an nsImageFormat::Enum to an equivalent Direct3D DXGI_FORMAT.
  static nsUInt32 ToDxgiFormat(nsImageFormat::Enum format);

  /// \brief Maps a Direct3D DXGI_FORMAT to an equivalent nsImageFormat::Enum.
  static nsImageFormat::Enum FromDxgiFormat(nsUInt32 uiDxgiFormat);

  /// \brief Maps an nsImageFormat::Enum to an equivalent FourCC code.
  static nsUInt32 ToFourCc(nsImageFormat::Enum format);

  /// \brief Maps a FourCC code to an equivalent nsImageFormat::Enum.
  static nsImageFormat::Enum FromFourCc(nsUInt32 uiFourCc);
};

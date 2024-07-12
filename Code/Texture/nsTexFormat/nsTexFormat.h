#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Types/Enum.h>
#include <Texture/Image/ImageEnums.h>
#include <Texture/TexConv/TexConvEnums.h>

class nsStreamWriter;
class nsStreamReader;

struct NS_TEXTURE_DLL nsTexFormat
{
  bool m_bSRGB = false;
  nsEnum<nsImageAddressMode> m_AddressModeU;
  nsEnum<nsImageAddressMode> m_AddressModeV;
  nsEnum<nsImageAddressMode> m_AddressModeW;

  // version 2
  nsEnum<nsTextureFilterSetting> m_TextureFilter;

  // version 3
  nsInt16 m_iRenderTargetResolutionX = 0;
  nsInt16 m_iRenderTargetResolutionY = 0;

  // version 4
  float m_fResolutionScale = 1.0f;

  // version 5
  int m_GalRenderTargetFormat = 0;

  void WriteTextureHeader(nsStreamWriter& inout_stream) const;
  void WriteRenderTargetHeader(nsStreamWriter& inout_stream) const;
  void ReadHeader(nsStreamReader& inout_stream);
};

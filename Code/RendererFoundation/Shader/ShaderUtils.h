#pragma once

#include <RendererFoundation/RendererFoundationDLL.h>

#include <Foundation/Math/Float16.h>
#include <Foundation/Math/Vec2.h>
#include <Foundation/Math/Vec3.h>

class nsShaderUtils
{
public:
  NS_ALWAYS_INLINE static nsUInt32 Float3ToRGB10(nsVec3 value)
  {
    const nsVec3 unsignedValue = value * 0.5f + nsVec3(0.5f);

    const nsUInt32 r = nsMath::Clamp(static_cast<nsUInt32>(unsignedValue.x * 1023.0f + 0.5f), 0u, 1023u);
    const nsUInt32 g = nsMath::Clamp(static_cast<nsUInt32>(unsignedValue.y * 1023.0f + 0.5f), 0u, 1023u);
    const nsUInt32 b = nsMath::Clamp(static_cast<nsUInt32>(unsignedValue.z * 1023.0f + 0.5f), 0u, 1023u);

    return r | (g << 10) | (b << 20);
  }

  NS_ALWAYS_INLINE static nsUInt32 PackFloat16intoUint(nsFloat16 x, nsFloat16 y)
  {
    const nsUInt32 r = x.GetRawData();
    const nsUInt32 g = y.GetRawData();

    return r | (g << 16);
  }

  NS_ALWAYS_INLINE static nsUInt32 Float2ToRG16F(nsVec2 value)
  {
    const nsUInt32 r = nsFloat16(value.x).GetRawData();
    const nsUInt32 g = nsFloat16(value.y).GetRawData();

    return r | (g << 16);
  }

  NS_ALWAYS_INLINE static void Float4ToRGBA16F(nsVec4 value, nsUInt32& out_uiRG, nsUInt32& out_uiBA)
  {
    out_uiRG = Float2ToRG16F(nsVec2(value.x, value.y));
    out_uiBA = Float2ToRG16F(nsVec2(value.z, value.w));
  }

  enum class nsBuiltinShaderType
  {
    CopyImage,
    CopyImageArray,
    DownscaleImage,
    DownscaleImageArray,
  };

  struct nsBuiltinShader
  {
    nsGALShaderHandle m_hActiveGALShader;
    nsGALBlendStateHandle m_hBlendState;
    nsGALDepthStencilStateHandle m_hDepthStencilState;
    nsGALRasterizerStateHandle m_hRasterizerState;
  };

  NS_RENDERERFOUNDATION_DLL static nsDelegate<void(nsBuiltinShaderType type, nsBuiltinShader& out_shader)> g_RequestBuiltinShaderCallback;

  NS_ALWAYS_INLINE static void RequestBuiltinShader(nsBuiltinShaderType type, nsBuiltinShader& out_shader)
  {
    g_RequestBuiltinShaderCallback(type, out_shader);
  }
};
NS_DEFINE_AS_POD_TYPE(nsShaderUtils::nsBuiltinShaderType);

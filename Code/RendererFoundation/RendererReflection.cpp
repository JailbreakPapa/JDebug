#include <RendererFoundation/RendererFoundationPCH.h>

#include <RendererFoundation/RendererFoundationDLL.h>
// clang-format off
NS_BEGIN_STATIC_REFLECTED_ENUM(nsGALResourceFormat, 1)
  NS_ENUM_CONSTANT(nsGALResourceFormat::RGBAFloat),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGBAUInt),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGBAInt),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGBFloat),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGBUInt),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGBInt),
    NS_ENUM_CONSTANT(nsGALResourceFormat::B5G6R5UNormalized),
    NS_ENUM_CONSTANT(nsGALResourceFormat::BGRAUByteNormalized),
    NS_ENUM_CONSTANT(nsGALResourceFormat::BGRAUByteNormalizedsRGB),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGBAHalf),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGBAUShort),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGBAUShortNormalized),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGBAShort),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGBAShortNormalized),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGFloat),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGUInt),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGInt),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGB10A2UInt),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGB10A2UIntNormalized),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RG11B10Float),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGBAUByteNormalized),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGBAUByteNormalizedsRGB),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGBAUByte),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGBAByteNormalized),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGBAByte),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGHalf),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGUShort),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGUShortNormalized),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGShort),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGShortNormalized),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGUByte),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGUByteNormalized),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGByte),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RGByteNormalized),
    NS_ENUM_CONSTANT(nsGALResourceFormat::DFloat),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RFloat),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RUInt),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RInt),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RHalf),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RUShort),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RUShortNormalized),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RShort),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RShortNormalized),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RUByte),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RUByteNormalized),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RByte),
    NS_ENUM_CONSTANT(nsGALResourceFormat::RByteNormalized),
    NS_ENUM_CONSTANT(nsGALResourceFormat::AUByteNormalized),
    NS_ENUM_CONSTANT(nsGALResourceFormat::D16),
    NS_ENUM_CONSTANT(nsGALResourceFormat::D24S8),
    NS_ENUM_CONSTANT(nsGALResourceFormat::BC1),
    NS_ENUM_CONSTANT(nsGALResourceFormat::BC1sRGB),
    NS_ENUM_CONSTANT(nsGALResourceFormat::BC2),
    NS_ENUM_CONSTANT(nsGALResourceFormat::BC2sRGB),
    NS_ENUM_CONSTANT(nsGALResourceFormat::BC3),
    NS_ENUM_CONSTANT(nsGALResourceFormat::BC3sRGB),
    NS_ENUM_CONSTANT(nsGALResourceFormat::BC4UNormalized),
    NS_ENUM_CONSTANT(nsGALResourceFormat::BC4Normalized),
    NS_ENUM_CONSTANT(nsGALResourceFormat::BC5UNormalized),
    NS_ENUM_CONSTANT(nsGALResourceFormat::BC5Normalized),
    NS_ENUM_CONSTANT(nsGALResourceFormat::BC6UFloat),
    NS_ENUM_CONSTANT(nsGALResourceFormat::BC6Float),
    NS_ENUM_CONSTANT(nsGALResourceFormat::BC7UNormalized),
    NS_ENUM_CONSTANT(nsGALResourceFormat::BC7UNormalizedsRGB)
NS_END_STATIC_REFLECTED_ENUM;

NS_BEGIN_STATIC_REFLECTED_ENUM(nsGALMSAASampleCount, 1)
  NS_ENUM_CONSTANTS(nsGALMSAASampleCount::None, nsGALMSAASampleCount::TwoSamples, nsGALMSAASampleCount::FourSamples, nsGALMSAASampleCount::EightSamples)
NS_END_STATIC_REFLECTED_ENUM;

NS_BEGIN_STATIC_REFLECTED_ENUM(nsGALTextureType, 1)
  NS_ENUM_CONSTANTS(nsGALTextureType::Invalid, nsGALTextureType::Texture2D, nsGALTextureType::TextureCube, nsGALTextureType::Texture3D, nsGALTextureType::Texture2DProxy, nsGALTextureType::Texture2DShared)
NS_END_STATIC_REFLECTED_ENUM;

NS_BEGIN_STATIC_REFLECTED_TYPE(nsGALResourceAccess, nsNoBase, 1, nsRTTIDefaultAllocator<nsGALResourceAccess>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("ReadBack", m_bReadBack),
    NS_MEMBER_PROPERTY("Immutable", m_bImmutable),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;

NS_BEGIN_STATIC_REFLECTED_TYPE(nsGALTextureCreationDescription, nsNoBase, 1, nsRTTIDefaultAllocator<nsGALTextureCreationDescription>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Width", m_uiWidth),
    NS_MEMBER_PROPERTY("Height", m_uiHeight),
    NS_MEMBER_PROPERTY("Depth", m_uiDepth),
    NS_MEMBER_PROPERTY("MipLevelCount", m_uiMipLevelCount),
    NS_MEMBER_PROPERTY("ArraySize", m_uiArraySize),
    NS_ENUM_MEMBER_PROPERTY("Format", nsGALResourceFormat, m_Format),
    NS_ENUM_MEMBER_PROPERTY("SampleCount", nsGALMSAASampleCount, m_SampleCount),
    NS_ENUM_MEMBER_PROPERTY("Type", nsGALTextureType, m_Type),
    NS_MEMBER_PROPERTY("AllowShaderResourceView", m_bAllowShaderResourceView),
    NS_MEMBER_PROPERTY("AllowUAV", m_bAllowUAV),
    NS_MEMBER_PROPERTY("CreateRenderTarget", m_bCreateRenderTarget),
    NS_MEMBER_PROPERTY("AllowDynamicMipGeneration", m_bAllowDynamicMipGeneration),
    NS_MEMBER_PROPERTY("ResourceAccess", m_ResourceAccess),
    // m_pExisitingNativeObject deliberately not reflected as it can't be serialized in any meaningful way.
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;

NS_BEGIN_STATIC_REFLECTED_TYPE(nsGALPlatformSharedHandle, nsNoBase, 1, nsRTTIDefaultAllocator<nsGALPlatformSharedHandle>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("SharedTexture", m_hSharedTexture),
    NS_MEMBER_PROPERTY("Semaphore", m_hSemaphore),
    NS_MEMBER_PROPERTY("ProcessId", m_uiProcessId),
    NS_MEMBER_PROPERTY("MemoryTypeIndex", m_uiMemoryTypeIndex),
    NS_MEMBER_PROPERTY("Size", m_uiSize),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

NS_STATICLINK_FILE(RendererFoundation, RendererFoundation_RendererReflection);

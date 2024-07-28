#pragma once

#include <RendererFoundation/RendererFoundationDLL.h>

#include <Foundation/Types/Bitflags.h>

/// \brief Defines which operations can be performed on an nsGALResourceFormat
/// \sa nsGALDeviceCapabilities::m_FormatSupport
struct nsGALResourceFormatSupport
{
  using StorageType = nsUInt8;

  enum Enum
  {
    None = 0,
    Texture = NS_BIT(0),         ///< Can be used as a texture and bound to a nsGALShaderResourceType::Texture slot.
    RenderTarget = NS_BIT(1),    ///< Can be used as a texture and bound as a render target.
    TextureRW = NS_BIT(2),       ///< Can be used as a texture and bound to a nsGALShaderResourceType::TextureRW slot.
    MSAA2x = NS_BIT(3),          ///< The format supports 2x MSAA
    MSAA4x = NS_BIT(4),          ///< The format supports 4x MSAA
    MSAA8x = NS_BIT(5),          ///< The format supports 8x MSAA
    VertexAttribute = NS_BIT(6), ///< The format can be used as a vertex attribute.
    Default = 0
  };

  struct Bits
  {
    StorageType Texture : 1;
    StorageType RenderTarget : 1;
    StorageType TextureRW : 1;
    StorageType MSAA2x : 1;
    StorageType MSAA4x : 1;
    StorageType MSAA8x : 1;
    StorageType VertexAttribute : 1;
  };
};
NS_DECLARE_FLAGS_OPERATORS(nsGALResourceFormatSupport);

/// \brief This struct holds information about the rendering device capabilities (e.g. what shader stages are supported and more)
/// To get the device capabilities you need to call the GetCapabilities() function on an nsGALDevice object.
struct NS_RENDERERFOUNDATION_DLL nsGALDeviceCapabilities
{
  // Device description
  nsString m_sAdapterName = "Unknown";
  nsUInt64 m_uiDedicatedVRAM = 0;
  nsUInt64 m_uiDedicatedSystemRAM = 0;
  nsUInt64 m_uiSharedSystemRAM = 0;
  bool m_bHardwareAccelerated = false;

  // General capabilities
  bool m_bMultithreadedResourceCreation = false; ///< whether creating resources is allowed on other threads than the main thread
  bool m_bNoOverwriteBufferUpdate = false;

  // Draw related capabilities
  bool m_bShaderStageSupported[nsGALShaderStage::ENUM_COUNT] = {};
  bool m_bInstancing = false;
  bool m_b32BitIndices = false;
  bool m_bIndirectDraw = false;
  bool m_bConservativeRasterization = false;
  bool m_bVertexShaderRenderTargetArrayIndex = false;
  nsUInt16 m_uiMaxConstantBuffers = 0;
  nsUInt16 m_uiMaxPushConstantsSize = 0;


  // Texture related capabilities
  bool m_bTextureArrays = false;
  bool m_bCubemapArrays = false;
  bool m_bSharedTextures = false;
  nsUInt16 m_uiMaxTextureDimension = 0;
  nsUInt16 m_uiMaxCubemapDimension = 0;
  nsUInt16 m_uiMax3DTextureDimension = 0;
  nsUInt16 m_uiMaxAnisotropy = 0;
  nsDynamicArray<nsBitflags<nsGALResourceFormatSupport>> m_FormatSupport;

  // Output related capabilities
  nsUInt16 m_uiMaxRendertargets = 0;
  nsUInt16 m_uiUAVCount = 0;
  bool m_bAlphaToCoverage = false;
};

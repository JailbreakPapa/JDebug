
#pragma once

#include <Foundation/Algorithm/HashableStruct.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Containers/StaticArray.h>
#include <Foundation/Math/Color.h>
#include <Foundation/Types/SharedPtr.h>
#include <RendererFoundation/Descriptors/Enumerations.h>
#include <RendererFoundation/RendererFoundationDLL.h>
#include <RendererFoundation/Resources/ResourceFormats.h>
#include <RendererFoundation/Shader/ShaderByteCode.h>
#include <Texture/Image/ImageEnums.h>

class nsWindowBase;



struct nsGALWindowSwapChainCreationDescription : public nsHashableStruct<nsGALWindowSwapChainCreationDescription>
{
  nsWindowBase* m_pWindow = nullptr;

  // Describes the format that should be used for the backbuffer.
  // Note however, that different platforms may enforce restrictions on this.
  nsGALMSAASampleCount::Enum m_SampleCount = nsGALMSAASampleCount::None;
  nsGALResourceFormat::Enum m_BackBufferFormat = nsGALResourceFormat::RGBAUByteNormalizedsRGB;
  nsEnum<nsGALPresentMode> m_InitialPresentMode = nsGALPresentMode::VSync;

  bool m_bDoubleBuffered = true;
  bool m_bAllowScreenshots = false;
};

struct nsGALSwapChainCreationDescription : public nsHashableStruct<nsGALSwapChainCreationDescription>
{
  const nsRTTI* m_pSwapChainType = nullptr;
};

struct nsGALDeviceCreationDescription
{
  bool m_bDebugDevice = false;
};

struct nsGALShaderCreationDescription : public nsHashableStruct<nsGALShaderCreationDescription>
{
  nsGALShaderCreationDescription();
  ~nsGALShaderCreationDescription();

  bool HasByteCodeForStage(nsGALShaderStage::Enum stage) const;

  nsSharedPtr<nsGALShaderByteCode> m_ByteCodes[nsGALShaderStage::ENUM_COUNT];
};

struct nsGALRenderTargetBlendDescription : public nsHashableStruct<nsGALRenderTargetBlendDescription>
{
  nsEnum<nsGALBlend> m_SourceBlend = nsGALBlend::One;
  nsEnum<nsGALBlend> m_DestBlend = nsGALBlend::One;
  nsEnum<nsGALBlendOp> m_BlendOp = nsGALBlendOp::Add;

  nsEnum<nsGALBlend> m_SourceBlendAlpha = nsGALBlend::One;
  nsEnum<nsGALBlend> m_DestBlendAlpha = nsGALBlend::One;
  nsEnum<nsGALBlendOp> m_BlendOpAlpha = nsGALBlendOp::Add;

  nsUInt8 m_uiWriteMask = 0xFF;    ///< Enables writes to color channels. Bit1 = Red Channel, Bit2 = Green Channel, Bit3 = Blue Channel, Bit4 = Alpha
                                   ///< Channel, Bit 5-8 are unused
  bool m_bBlendingEnabled = false; ///< If enabled, the color will be blended into the render target. Otherwise it will overwrite the render target.
                                   ///< Set m_uiWriteMask to 0 to disable all writes to the render target.
};

struct nsGALBlendStateCreationDescription : public nsHashableStruct<nsGALBlendStateCreationDescription>
{
  nsGALRenderTargetBlendDescription m_RenderTargetBlendDescriptions[NS_GAL_MAX_RENDERTARGET_COUNT];

  bool m_bAlphaToCoverage = false;  ///< Alpha-to-coverage can only be used with MSAA render targets. Default is false.
  bool m_bIndependentBlend = false; ///< If disabled, the blend state of the first render target is used for all render targets. Otherwise each
                                    ///< render target uses a different blend state.
};

struct nsGALStencilOpDescription : public nsHashableStruct<nsGALStencilOpDescription>
{
  nsEnum<nsGALStencilOp> m_FailOp = nsGALStencilOp::Keep;
  nsEnum<nsGALStencilOp> m_DepthFailOp = nsGALStencilOp::Keep;
  nsEnum<nsGALStencilOp> m_PassOp = nsGALStencilOp::Keep;

  nsEnum<nsGALCompareFunc> m_StencilFunc = nsGALCompareFunc::Always;
};

struct nsGALDepthStencilStateCreationDescription : public nsHashableStruct<nsGALDepthStencilStateCreationDescription>
{
  nsGALStencilOpDescription m_FrontFaceStencilOp;
  nsGALStencilOpDescription m_BackFaceStencilOp;

  nsEnum<nsGALCompareFunc> m_DepthTestFunc = nsGALCompareFunc::Less;

  bool m_bSeparateFrontAndBack = false; ///< If false, DX11 will use front face values for both front & back face values, GL will not call
                                        ///< gl*Separate() funcs
  bool m_bDepthTest = true;
  bool m_bDepthWrite = true;
  bool m_bStencilTest = false;
  nsUInt8 m_uiStencilReadMask = 0xFF;
  nsUInt8 m_uiStencilWriteMask = 0xFF;
};

/// \brief Describes the settings for a new rasterizer state. See nsGALDevice::CreateRasterizerState
struct nsGALRasterizerStateCreationDescription : public nsHashableStruct<nsGALRasterizerStateCreationDescription>
{
  nsEnum<nsGALCullMode> m_CullMode = nsGALCullMode::Back; ///< Which sides of a triangle to cull. Default is nsGALCullMode::Back
  nsInt32 m_iDepthBias = 0;                               ///< The pixel depth bias. Default is 0
  float m_fDepthBiasClamp = 0.0f;                         ///< The pixel depth bias clamp. Default is 0
  float m_fSlopeScaledDepthBias = 0.0f;                   ///< The pixel slope scaled depth bias clamp. Default is 0
  bool m_bWireFrame = false;                              ///< Whether triangles are rendered filled or as wireframe. Default is false
  bool m_bFrontCounterClockwise = false;                  ///< Sets which triangle winding order defines the 'front' of a triangle. If true, the front of a triangle
                                                          ///< is the one where the vertices appear in counter clockwise order. Default is false
  bool m_bScissorTest = false;
  bool m_bConservativeRasterization = false;              ///< Whether conservative rasterization is enabled
};

struct nsGALSamplerStateCreationDescription : public nsHashableStruct<nsGALSamplerStateCreationDescription>
{
  nsEnum<nsGALTextureFilterMode> m_MinFilter;
  nsEnum<nsGALTextureFilterMode> m_MagFilter;
  nsEnum<nsGALTextureFilterMode> m_MipFilter;

  nsEnum<nsImageAddressMode> m_AddressU;
  nsEnum<nsImageAddressMode> m_AddressV;
  nsEnum<nsImageAddressMode> m_AddressW;

  nsEnum<nsGALCompareFunc> m_SampleCompareFunc;

  nsColor m_BorderColor = nsColor::Black;

  float m_fMipLodBias = 0.0f;
  float m_fMinMip = -1.0f;
  float m_fMaxMip = 42000.0f;

  nsUInt32 m_uiMaxAnisotropy = 4;
};

struct NS_RENDERERFOUNDATION_DLL nsGALVertexAttribute
{
  nsGALVertexAttribute() = default;

  nsGALVertexAttribute(nsGALVertexAttributeSemantic::Enum semantic, nsGALResourceFormat::Enum format, nsUInt16 uiOffset, nsUInt8 uiVertexBufferSlot,
    bool bInstanceData);

  nsGALVertexAttributeSemantic::Enum m_eSemantic = nsGALVertexAttributeSemantic::Position;
  nsGALResourceFormat::Enum m_eFormat = nsGALResourceFormat::XYZFloat;
  nsUInt16 m_uiOffset = 0;
  nsUInt8 m_uiVertexBufferSlot = 0;
  bool m_bInstanceData = false;
};

struct NS_RENDERERFOUNDATION_DLL nsGALVertexDeclarationCreationDescription : public nsHashableStruct<nsGALVertexDeclarationCreationDescription>
{
  nsGALShaderHandle m_hShader;
  nsStaticArray<nsGALVertexAttribute, 16> m_VertexAttributes;
};

// Need to add: immutable (GPU only), default(GPU only, but allows CopyToTempStorage updates), transient (allows nsGALUpdateMode::Discard), staging: read(back), staging: write (constantly mapped), unified memory (mobile, onboard GPU, allows all ops)
// Or use VmaMemoryUsage  + read write flags?
struct nsGALResourceAccess
{
  NS_ALWAYS_INLINE bool IsImmutable() const { return m_bImmutable; }

  bool m_bReadBack = false;
  bool m_bImmutable = true;
};

struct nsGALBufferCreationDescription : public nsHashableStruct<nsGALBufferCreationDescription>
{
  nsUInt32 m_uiTotalSize = 0;
  nsUInt32 m_uiStructSize = 0; // Struct or texel size
  nsBitflags<nsGALBufferUsageFlags> m_BufferFlags;
  nsGALResourceAccess m_ResourceAccess;
};

struct nsGALTextureCreationDescription : public nsHashableStruct<nsGALTextureCreationDescription>
{
  void SetAsRenderTarget(nsUInt32 uiWidth, nsUInt32 uiHeight, nsGALResourceFormat::Enum format, nsGALMSAASampleCount::Enum sampleCount = nsGALMSAASampleCount::None);

  nsUInt32 m_uiWidth = 0;
  nsUInt32 m_uiHeight = 0;
  nsUInt32 m_uiDepth = 1;
  nsUInt32 m_uiMipLevelCount = 1;
  nsUInt32 m_uiArraySize = 1;

  nsEnum<nsGALResourceFormat> m_Format = nsGALResourceFormat::Invalid;
  nsEnum<nsGALMSAASampleCount> m_SampleCount = nsGALMSAASampleCount::None;
  nsEnum<nsGALTextureType> m_Type = nsGALTextureType::Texture2D;

  bool m_bAllowShaderResourceView = true;
  bool m_bAllowUAV = false;
  bool m_bCreateRenderTarget = false;
  bool m_bAllowDynamicMipGeneration = false;

  nsGALResourceAccess m_ResourceAccess;

  void* m_pExisitingNativeObject = nullptr; ///< Can be used to encapsulate existing native textures in objects usable by the GAL
};

struct nsGALTextureResourceViewCreationDescription : public nsHashableStruct<nsGALTextureResourceViewCreationDescription>
{
  nsGALTextureHandle m_hTexture;
  nsEnum<nsGALResourceFormat> m_OverrideViewFormat = nsGALResourceFormat::Invalid;
  nsUInt32 m_uiMostDetailedMipLevel = 0;
  nsUInt32 m_uiMipLevelsToUse = 0xFFFFFFFFu;
  nsUInt32 m_uiFirstArraySlice = 0; // For cubemap array: index of first 2d slice to start with
  nsUInt32 m_uiArraySize = 1;       // For cubemap array: number of cubemaps
};

struct nsGALBufferResourceViewCreationDescription : public nsHashableStruct<nsGALBufferResourceViewCreationDescription>
{
  nsGALBufferHandle m_hBuffer;
  nsEnum<nsGALResourceFormat> m_OverrideViewFormat = nsGALResourceFormat::Invalid;
  nsUInt32 m_uiFirstElement = 0;
  nsUInt32 m_uiNumElements = 0;
  bool m_bRawView = false;
};

struct nsGALRenderTargetViewCreationDescription : public nsHashableStruct<nsGALRenderTargetViewCreationDescription>
{
  nsGALTextureHandle m_hTexture;

  nsEnum<nsGALResourceFormat> m_OverrideViewFormat = nsGALResourceFormat::Invalid;

  nsUInt32 m_uiMipLevel = 0;

  nsUInt32 m_uiFirstSlice = 0;
  nsUInt32 m_uiSliceCount = 1;

  bool m_bReadOnly = false; ///< Can be used for depth stencil views to create read only views (e.g. for soft particles using the native depth buffer)
};

struct nsGALTextureUnorderedAccessViewCreationDescription : public nsHashableStruct<nsGALTextureUnorderedAccessViewCreationDescription>
{
  nsGALTextureHandle m_hTexture;
  nsUInt32 m_uiFirstArraySlice = 0; ///< First depth slice for 3D Textures.
  nsUInt32 m_uiArraySize = 1;       ///< Number of depth slices for 3D textures.
  nsUInt16 m_uiMipLevelToUse = 0;   ///< Which MipLevel is accessed with this UAV
  nsEnum<nsGALResourceFormat> m_OverrideViewFormat = nsGALResourceFormat::Invalid;
};

struct nsGALBufferUnorderedAccessViewCreationDescription : public nsHashableStruct<nsGALBufferUnorderedAccessViewCreationDescription>
{
  nsGALBufferHandle m_hBuffer;
  nsUInt32 m_uiFirstElement = 0;
  nsUInt32 m_uiNumElements = 0;
  nsEnum<nsGALResourceFormat> m_OverrideViewFormat = nsGALResourceFormat::Invalid;
  bool m_bRawView = false;
};

struct nsGALQueryCreationDescription : public nsHashableStruct<nsGALQueryCreationDescription>
{
  nsEnum<nsGALQueryType> m_type = nsGALQueryType::NumSamplesPassed;

  /// In case this query is used for occlusion culling (type AnySamplesPassed), this determines whether drawing should be done if the query
  /// status is still unknown.
  bool m_bDrawIfUnknown = true;
};

/// \brief Type for important GAL events.
struct nsGALDeviceEvent
{
  enum Type
  {
    AfterInit,
    BeforeShutdown,
    BeforeBeginFrame,
    AfterBeginFrame,
    BeforeEndFrame,
    AfterEndFrame,
    BeforeBeginPipeline,
    AfterBeginPipeline,
    BeforeEndPipeline,
    AfterEndPipeline,
    // could add resource creation/destruction events, if this would be useful
  };

  Type m_Type;
  class nsGALDevice* m_pDevice = nullptr;
  nsGALSwapChainHandle m_hSwapChain;
};

// Opaque platform specific handle
// Typically holds a platform specific handle for the texture and it's synchronization primitive
struct nsGALPlatformSharedHandle : public nsHashableStruct<nsGALPlatformSharedHandle>
{
  nsUInt64 m_hSharedTexture = 0;
  nsUInt64 m_hSemaphore = 0;
  nsUInt32 m_uiProcessId = 0;
  nsUInt32 m_uiMemoryTypeIndex = 0;
  nsUInt64 m_uiSize = 0;
};

#include <RendererFoundation/Descriptors/Implementation/Descriptors_inl.h>

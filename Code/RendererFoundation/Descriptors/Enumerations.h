#pragma once

#include <RendererFoundation/RendererFoundationDLL.h>

/// \brief The type of a shader resource (nsShaderResourceBinding).
/// Shader resources need to be bound to a shader for it to function. This includes samplers, constant buffers, textures etc. which are all handled by NS via GAL resource types / views. However, vertex buffers, index buffers and vertex layouts are not considered shader resources and are handled separately.
/// \sa nsGALShaderTextureType, nsShaderResourceBinding
struct nsGALShaderResourceType
{
  using StorageType = nsUInt8;
  enum Enum : nsUInt8
  {
    Unknown = 0,
    /// Texture sampler (nsGALSamplerStateHandle). HLSL: SamplerState, SamplerComparisonState
    Sampler,

    /// Read-only struct (nsGALBufferHandle). HLSL: cbuffer, ConstantBuffer
    ConstantBuffer,
    // Read-only struct. Set directly via nsGALCommandEncoder::SetPushConstants. HLSL: Use macro BEGIN_PUSH_CONSTANTS, END_PUSH_CONSTANTS, GET_PUSH_CONSTANT
    PushConstants,

    /// \name Shader Resource Views (SRVs). These are set via nsGALTextureResourceViewHandle / nsGALBufferResourceViewHandle.
    ///@{

    /// Read-only texture view. When set, nsGALShaderTextureType is also set. HLSL: Texture*
    Texture,
    /// Read-only texture view with attached sampler. When set, nsGALShaderTextureType is also set. HLSL: Name sampler the same as texture with _AutoSampler appended.
    TextureAndSampler,
    /// Read-only texel buffer. It's like a 1D texture. HLSL: Buffer
    TexelBuffer,
    /// Read-only array of structs. HLSL: StructuredBuffer<T>, ByteAddressBuffer.
    StructuredBuffer,

    ///@}
    /// \name Unordered Access Views (UAVs). These are set via nsGALTextureUnorderedAccessViewHandle / nsGALBufferUnorderedAccessViewHandle.
    ///@{

    /// Read-write texture view. When set, nsGALShaderTextureType is also set. HLSL: RWTexture*
    TextureRW,
    /// Read-write texel buffer. It's like a 1D texture. HLSL: RWBuffer
    TexelBufferRW,
    /// Read-write array of structs. HLSL: RWStructuredBuffer<T>, RWByteAddressBuffer, AppendStructuredBuffer, ConsumeStructuredBuffer
    StructuredBufferRW,

    ///@}

    // #TODO_SHADER: Future work:
    // Not supported: NS does not support AppendStructuredBuffer, ConsumeStructuredBuffer yet so while the shader can be compiled, nothing can be bound to these resources. On Vulkan, will probably need yet another type to distinguish the data from the count resource (uav_counter_binding).
    // Not supported: tbuffer, TextureBuffer, these map to CBV on DX11 and to eStorageBuffer on Vulkan, requiring to use a constantBufferView or a UAV. Thus, it bleeds platform implementation details.
    // Not supported: (Vulkan) VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, frame-buffer local read-only image view. Required for render passes on mobile.
    // Not supported: (Vulkan) VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK,Vulkan 1.3 addition, surpasses push-constants but not widely supported yet. May be able to abstract this via PushConstants and custom shader compiler / GAL implementations.
    // Not supported: (Vulkan) VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, Vulkan extension for raytracing.

    Default = Unknown
  };
};

/// \brief General category of the shader resource (nsShaderResourceBinding).
/// Note that these are flags because some resources can be multiple resource types, e.g. nsGALShaderResourceType::TextureAndSampler.
struct nsGALShaderResourceCategory
{
  using StorageType = nsUInt8;
  static constexpr int ENUM_COUNT = 4;
  enum Enum : nsUInt8
  {
    Sampler = NS_BIT(0),        //< Sampler (nsGALSamplerStateHandle).
    ConstantBuffer = NS_BIT(1), //< Constant Buffer (nsGALBufferHandle)
    TextureSRV = NS_BIT(2),     //< Shader Resource Views (nsGALTextureResourceViewHandle).
    BufferSRV = NS_BIT(3),      //< Shader Resource Views (nsGALBufferResourceViewHandle).
    TextureUAV = NS_BIT(4),     //< Unordered Access Views (nsGALTextureUnorderedAccessViewHandle).
    BufferUAV = NS_BIT(5),      //< Unordered Access Views (nsGALBufferUnorderedAccessViewHandle).
    Default = 0
  };

  struct Bits
  {
    StorageType Sampler : 1;
    StorageType ConstantBuffer : 1;
    StorageType TextureSRV : 1;
    StorageType BufferSRV : 1;
    StorageType TextureUAV : 1;
    StorageType BufferUAV : 1;
  };

  static nsBitflags<nsGALShaderResourceCategory> MakeFromShaderDescriptorType(nsGALShaderResourceType::Enum type);
};

NS_DECLARE_FLAGS_OPERATORS(nsGALShaderResourceCategory);

/// \brief The texture type of the shader resource (nsShaderResourceBinding).
struct nsGALShaderTextureType
{
  using StorageType = nsUInt8;
  enum Enum : nsUInt8
  {
    Unknown = 0,
    Texture1D = 1,
    Texture1DArray = 2,
    Texture2D = 3,
    Texture2DArray = 4,
    Texture2DMS = 5,
    Texture2DMSArray = 6,
    Texture3D = 7,
    TextureCube = 8,
    TextureCubeArray = 9,

    Default = Unknown
  };

  static bool IsArray(nsGALShaderTextureType::Enum format);
};

/// \brief Defines a swap chain's present mode.
/// \sa nsGALWindowSwapChainCreationDescription
struct nsGALPresentMode
{
  using StorageType = nsUInt8;

  enum Enum
  {
    Immediate,
    VSync,
    ENUM_COUNT,
    Default = VSync
  };
};

/// \brief Defines the usage semantic of a vertex attribute.
/// \sa nsGALVertexAttribute
struct nsGALVertexAttributeSemantic
{
  using StorageType = nsUInt8;

  enum Enum : nsUInt8
  {
    Position,
    Normal,
    Tangent,
    Color0,
    Color1,
    Color2,
    Color3,
    Color4,
    Color5,
    Color6,
    Color7,
    TexCoord0,
    TexCoord1,
    TexCoord2,
    TexCoord3,
    TexCoord4,
    TexCoord5,
    TexCoord6,
    TexCoord7,
    TexCoord8,
    TexCoord9,

    BiTangent,
    BoneIndices0,
    BoneIndices1,
    BoneWeights0,
    BoneWeights1,

    ENUM_COUNT,
    Default = Position
  };
};

/// \brief Defines for what purpose a buffer can be used for.
/// \sa nsGALBufferCreationDescription
struct nsGALBufferUsageFlags
{
  using StorageType = nsUInt16;

  enum Enum
  {
    VertexBuffer = NS_BIT(0),      ///< Can be used as a vertex buffer.
    IndexBuffer = NS_BIT(1),       ///< Can be used as an index buffer.
    ConstantBuffer = NS_BIT(2),    ///< Can be used as a constant buffer. Can't be combined with any of the other *Buffer flags.
    TexelBuffer = NS_BIT(3),       ///< Can be used as a texel buffer.
    StructuredBuffer = NS_BIT(4),  ///< nsGALShaderResourceType::StructuredBuffer
    ByteAddressBuffer = NS_BIT(5), ///< nsGALShaderResourceType::ByteAddressBuffer (RAW)

    ShaderResource = NS_BIT(6),    ///< Can be used for nsGALShaderResourceType in the SRV section.
    UnorderedAccess = NS_BIT(7),   ///< Can be used for nsGALShaderResourceType in the UAV section.
    DrawIndirect = NS_BIT(8),      ///< Can be used in an indirect draw call.

    Default = 0
  };

  struct Bits
  {
    StorageType VertexBuffer : 1;
    StorageType IndexBuffer : 1;
    StorageType ConstantBuffer : 1;
    StorageType TexelBuffer : 1;
    StorageType StructuredBuffer : 1;
    StorageType ByteAddressBuffer : 1;
    StorageType ShaderResource : 1;
    StorageType UnorderedAccess : 1;
    StorageType DrawIndirect : 1;
  };
};
NS_DECLARE_FLAGS_OPERATORS(nsGALBufferUsageFlags);

/// \brief Type of GPU->CPU query.
/// \sa nsGALQueryCreationDescription
struct nsGALQueryType
{
  using StorageType = nsUInt8;

  enum Enum
  {
    /// Number of samples that passed the depth and stencil test between begin and end (on a context).
    NumSamplesPassed,
    /// Boolean version of NumSamplesPassed.
    AnySamplesPassed,

    Default = NumSamplesPassed

    // Note:
    // GALFence provides an implementation of "event queries".
  };
};

/// \brief Type of the shared texture (INTERNAL)
struct nsGALSharedTextureType
{
  using StorageType = nsUInt8;

  enum Enum : nsUInt8
  {
    None,     ///< Not shared
    Exported, ///< Allocation owned by this process
    Imported, ///< Allocation owned by a different process
    Default = None
  };
};

#include <RendererFoundation/Descriptors/Implementation/Enumerations_inl.h>

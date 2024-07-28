#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Types/Id.h>
#include <Foundation/Types/RefCounted.h>

// Configure the DLL Import/Export Define
#if NS_ENABLED(NS_COMPILE_ENGINE_AS_DLL)
#  ifdef BUILDSYSTEM_BUILDING_RENDERERFOUNDATION_LIB
#    define NS_RENDERERFOUNDATION_DLL NS_DECL_EXPORT
#  else
#    define NS_RENDERERFOUNDATION_DLL NS_DECL_IMPORT
#  endif
#else
#  define NS_RENDERERFOUNDATION_DLL
#endif

// #TODO_SHADER obsolete, DX11 only
#define NS_GAL_MAX_CONSTANT_BUFFER_COUNT 16
#define NS_GAL_MAX_SAMPLER_COUNT 16

// Necessary array sizes
#define NS_GAL_MAX_VERTEX_BUFFER_COUNT 16
#define NS_GAL_MAX_RENDERTARGET_COUNT 8

// Forward declarations

struct nsGALDeviceCreationDescription;
struct nsGALSwapChainCreationDescription;
struct nsGALWindowSwapChainCreationDescription;
struct nsGALShaderCreationDescription;
struct nsGALTextureCreationDescription;
struct nsGALBufferCreationDescription;
struct nsGALDepthStencilStateCreationDescription;
struct nsGALBlendStateCreationDescription;
struct nsGALRasterizerStateCreationDescription;
struct nsGALVertexDeclarationCreationDescription;
struct nsGALQueryCreationDescription;
struct nsGALSamplerStateCreationDescription;
struct nsGALTextureResourceViewCreationDescription;
struct nsGALBufferResourceViewCreationDescription;
struct nsGALRenderTargetViewCreationDescription;
struct nsGALTextureUnorderedAccessViewCreationDescription;
struct nsGALBufferUnorderedAccessViewCreationDescription;

class nsGALSwapChain;
class nsGALShader;
class nsGALResourceBase;
class nsGALTexture;
class nsGALSharedTexture;
class nsGALBuffer;
class nsGALDepthStencilState;
class nsGALBlendState;
class nsGALRasterizerState;
class nsGALRenderTargetSetup;
class nsGALVertexDeclaration;
class nsGALQuery;
class nsGALSamplerState;
class nsGALTextureResourceView;
class nsGALBufferResourceView;
class nsGALRenderTargetView;
class nsGALTextureUnorderedAccessView;
class nsGALBufferUnorderedAccessView;
class nsGALDevice;
class nsGALPass;
class nsGALCommandEncoder;
class nsGALRenderCommandEncoder;
class nsGALComputeCommandEncoder;

// Basic enums
struct nsGALPrimitiveTopology
{
  using StorageType = nsUInt8;
  enum Enum
  {
    // keep this order, it is used to allocate the desired number of indices in nsMeshBufferResourceDescriptor::AllocateStreams
    Points,    // 1 index per primitive
    Lines,     // 2 indices per primitive
    Triangles, // 3 indices per primitive
    ENUM_COUNT,
    Default = Triangles
  };

  static nsUInt32 VerticesPerPrimitive(nsGALPrimitiveTopology::Enum e) { return (nsUInt32)e + 1; }
};

struct NS_RENDERERFOUNDATION_DLL nsGALIndexType
{
  enum Enum
  {
    None,   // indices are not used, vertices are just used in order to form primitives
    UShort, // 16 bit indices are used to select which vertices shall form a primitive, thus meshes can only use up to 65535 vertices
    UInt,   // 32 bit indices are used to select which vertices shall form a primitive

    ENUM_COUNT
  };


  /// \brief The size in bytes of a single element of the given index format.
  static nsUInt8 GetSize(nsGALIndexType::Enum format) { return s_Size[format]; }

private:
  static const nsUInt8 s_Size[nsGALIndexType::ENUM_COUNT];
};

/// \brief The stage of a shader. A complete shader can consist of multiple stages.
/// \sa nsGALShaderStageFlags, nsGALShaderCreationDescription
struct NS_RENDERERFOUNDATION_DLL nsGALShaderStage
{
  using StorageType = nsUInt8;

  enum Enum : nsUInt8
  {
    VertexShader,
    HullShader,
    DomainShader,
    GeometryShader,
    PixelShader,
    ComputeShader,
    /*
    // #TODO_SHADER: Future work:
    TaskShader,
    MeshShader,
    RayGenShader,
    RayAnyHitShader,
    RayClosestHitShader,
    RayMissShader,
    RayIntersectionShader,
    */
    ENUM_COUNT,
    Default = VertexShader
  };

  static const char* Names[ENUM_COUNT];
};

/// \brief A set of shader stages.
/// \sa nsGALShaderStage, nsShaderResourceBinding
struct NS_RENDERERFOUNDATION_DLL nsGALShaderStageFlags
{
  using StorageType = nsUInt16;

  enum Enum : nsUInt16
  {
    VertexShader = NS_BIT(0),
    HullShader = NS_BIT(1),
    DomainShader = NS_BIT(2),
    GeometryShader = NS_BIT(3),
    PixelShader = NS_BIT(4),
    ComputeShader = NS_BIT(5),
    /*
    // TODO: #TODO_SHADER: Future work:
    TaskShader = NS_BIT(6),
    MeshShader = NS_BIT(7),
    RayGenShader = NS_BIT(8),
    RayAnyHitShader = NS_BIT(9),
    RayClosestHitShader = NS_BIT(10),
    RayMissShader = NS_BIT(11),
    RayIntersectionShader = NS_BIT(12),
    */
    Default = 0
  };

  struct Bits
  {
    StorageType VertexShader : 1;
    StorageType HullShader : 1;
    StorageType DomainShader : 1;
    StorageType GeometryShader : 1;
    StorageType PixelShader : 1;
    StorageType ComputeShader : 1;
  };

  inline static nsGALShaderStageFlags::Enum MakeFromShaderStage(nsGALShaderStage::Enum stage)
  {
    return static_cast<nsGALShaderStageFlags::Enum>(NS_BIT(stage));
  }
};
NS_DECLARE_FLAGS_OPERATORS(nsGALShaderStageFlags);


struct NS_RENDERERFOUNDATION_DLL nsGALMSAASampleCount
{
  using StorageType = nsUInt8;

  enum Enum
  {
    None = 1,
    TwoSamples = 2,
    FourSamples = 4,
    EightSamples = 8,

    ENUM_COUNT = 4,

    Default = None
  };
};

struct nsGALTextureType
{
  using StorageType = nsUInt8;

  enum Enum
  {
    Invalid = -1,
    Texture2D = 0,
    TextureCube,
    Texture3D,
    Texture2DProxy,
    Texture2DShared,

    ENUM_COUNT,

    Default = Texture2D
  };
};

struct nsGALBlend
{
  using StorageType = nsUInt8;

  enum Enum
  {
    Zero = 0,
    One,
    SrcColor,
    InvSrcColor,
    SrcAlpha,
    InvSrcAlpha,
    DestAlpha,
    InvDestAlpha,
    DestColor,
    InvDestColor,
    SrcAlphaSaturated,
    BlendFactor,
    InvBlendFactor,

    ENUM_COUNT,

    Default = One
  };
};

struct nsGALBlendOp
{
  using StorageType = nsUInt8;

  enum Enum
  {
    Add = 0,
    Subtract,
    RevSubtract,
    Min,
    Max,

    ENUM_COUNT,
    Default = Add
  };
};

struct nsGALStencilOp
{
  using StorageType = nsUInt8;

  enum Enum
  {
    Keep = 0,
    Zero,
    Replace,
    IncrementSaturated,
    DecrementSaturated,
    Invert,
    Increment,
    Decrement,

    ENUM_COUNT,

    Default = Keep
  };
};

struct nsGALCompareFunc
{
  using StorageType = nsUInt8;

  enum Enum
  {
    Never = 0,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    Always,

    ENUM_COUNT,

    Default = Never
  };
};

/// \brief Defines which sides of a polygon gets culled by the graphics card
struct nsGALCullMode
{
  using StorageType = nsUInt8;

  /// \brief Defines which sides of a polygon gets culled by the graphics card
  enum Enum
  {
    None = 0,  ///< Triangles do not get culled
    Front = 1, ///< When the 'front' of a triangle is visible, it gets culled. The rasterizer state defines which side is the 'front'. See
               ///< nsGALRasterizerStateCreationDescription for details.
    Back = 2,  ///< When the 'back'  of a triangle is visible, it gets culled. The rasterizer state defines which side is the 'front'. See
               ///< nsGALRasterizerStateCreationDescription for details.

    ENUM_COUNT,

    Default = Back
  };
};

struct nsGALTextureFilterMode
{
  using StorageType = nsUInt8;

  enum Enum
  {
    Point = 0,
    Linear,
    Anisotropic,

    Default = Linear
  };
};

struct nsGALUpdateMode
{
  enum Enum
  {
    Discard,          ///< Buffer must be completely overwritten. No old data will be read. Data will not persist across frames.
    NoOverwrite,      ///< User is responsible for synchronizing access between GPU and CPU.
    CopyToTempStorage ///< Upload to temp buffer, then buffer to buffer transfer at the current time in the command buffer.
  };
};

// Basic structs
struct nsGALTextureSubresource
{
  nsUInt32 m_uiMipLevel = 0;
  nsUInt32 m_uiArraySlice = 0;
};

struct nsGALSystemMemoryDescription
{
  void* m_pData = nullptr;
  nsUInt32 m_uiRowPitch = 0;
  nsUInt32 m_uiSlicePitch = 0;
};

/// \brief Base class for GAL objects, stores a creation description of the object and also allows for reference counting.
template <typename CreationDescription>
class nsGALObject : public nsRefCounted
{
public:
  nsGALObject(const CreationDescription& description)
    : m_Description(description)
  {
  }

  NS_ALWAYS_INLINE const CreationDescription& GetDescription() const { return m_Description; }

protected:
  const CreationDescription m_Description;
};

// Handles
namespace nsGAL
{
  using ns16_16Id = nsGenericId<16, 16>;
  using ns18_14Id = nsGenericId<18, 14>;
  using ns20_12Id = nsGenericId<20, 12>;
} // namespace nsGAL

class nsGALSwapChainHandle
{
  NS_DECLARE_HANDLE_TYPE(nsGALSwapChainHandle, nsGAL::ns16_16Id);

  friend class nsGALDevice;
};

class nsGALShaderHandle
{
  NS_DECLARE_HANDLE_TYPE(nsGALShaderHandle, nsGAL::ns18_14Id);

  friend class nsGALDevice;
};

class nsGALTextureHandle
{
  NS_DECLARE_HANDLE_TYPE(nsGALTextureHandle, nsGAL::ns18_14Id);

  friend class nsGALDevice;
};

class nsGALBufferHandle
{
  NS_DECLARE_HANDLE_TYPE(nsGALBufferHandle, nsGAL::ns18_14Id);

  friend class nsGALDevice;
};

class nsGALTextureResourceViewHandle
{
  NS_DECLARE_HANDLE_TYPE(nsGALTextureResourceViewHandle, nsGAL::ns18_14Id);

  friend class nsGALDevice;
};

class nsGALBufferResourceViewHandle
{
  NS_DECLARE_HANDLE_TYPE(nsGALBufferResourceViewHandle, nsGAL::ns18_14Id);

  friend class nsGALDevice;
};

class nsGALTextureUnorderedAccessViewHandle
{
  NS_DECLARE_HANDLE_TYPE(nsGALTextureUnorderedAccessViewHandle, nsGAL::ns18_14Id);

  friend class nsGALDevice;
};

class nsGALBufferUnorderedAccessViewHandle
{
  NS_DECLARE_HANDLE_TYPE(nsGALBufferUnorderedAccessViewHandle, nsGAL::ns18_14Id);

  friend class nsGALDevice;
};

class nsGALRenderTargetViewHandle
{
  NS_DECLARE_HANDLE_TYPE(nsGALRenderTargetViewHandle, nsGAL::ns18_14Id);

  friend class nsGALDevice;
};

class nsGALDepthStencilStateHandle
{
  NS_DECLARE_HANDLE_TYPE(nsGALDepthStencilStateHandle, nsGAL::ns16_16Id);

  friend class nsGALDevice;
};

class nsGALBlendStateHandle
{
  NS_DECLARE_HANDLE_TYPE(nsGALBlendStateHandle, nsGAL::ns16_16Id);

  friend class nsGALDevice;
};

class nsGALRasterizerStateHandle
{
  NS_DECLARE_HANDLE_TYPE(nsGALRasterizerStateHandle, nsGAL::ns16_16Id);

  friend class nsGALDevice;
};

class nsGALSamplerStateHandle
{
  NS_DECLARE_HANDLE_TYPE(nsGALSamplerStateHandle, nsGAL::ns16_16Id);

  friend class nsGALDevice;
};

class nsGALVertexDeclarationHandle
{
  NS_DECLARE_HANDLE_TYPE(nsGALVertexDeclarationHandle, nsGAL::ns18_14Id);

  friend class nsGALDevice;
};

class nsGALQueryHandle
{
  NS_DECLARE_HANDLE_TYPE(nsGALQueryHandle, nsGAL::ns20_12Id);

  friend class nsGALDevice;
};

struct nsGALTimestampHandle
{
  NS_DECLARE_POD_TYPE();

  nsUInt64 m_uiIndex;
  nsUInt64 m_uiFrameCounter;
};

namespace nsGAL
{
  struct ModifiedRange
  {
    NS_ALWAYS_INLINE void Reset()
    {
      m_uiMin = nsInvalidIndex;
      m_uiMax = 0;
    }

    NS_FORCE_INLINE void SetToIncludeValue(nsUInt32 value)
    {
      m_uiMin = nsMath::Min(m_uiMin, value);
      m_uiMax = nsMath::Max(m_uiMax, value);
    }

    NS_FORCE_INLINE void SetToIncludeRange(nsUInt32 uiMin, nsUInt32 uiMax)
    {
      m_uiMin = nsMath::Min(m_uiMin, uiMin);
      m_uiMax = nsMath::Max(m_uiMax, uiMax);
    }

    NS_ALWAYS_INLINE bool IsValid() const { return m_uiMin <= m_uiMax; }

    NS_ALWAYS_INLINE nsUInt32 GetCount() const { return m_uiMax - m_uiMin + 1; }

    nsUInt32 m_uiMin = nsInvalidIndex;
    nsUInt32 m_uiMax = 0;
  };
} // namespace nsGAL

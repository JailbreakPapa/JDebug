#pragma once

#include <Foundation/Reflection/Reflection.h>
#include <RendererCore/Declarations.h>

//////////////////////////////////////////////////////////////////////////
// nsShaderBindFlags
//////////////////////////////////////////////////////////////////////////

struct NS_RENDERERCORE_DLL nsShaderBindFlags
{
  using StorageType = nsUInt32;

  enum Enum
  {
    None = 0,                ///< No flags causes the default shader binding behavior (all render states are applied)
    ForceRebind = NS_BIT(0), ///< Executes shader binding (and state setting), even if the shader hasn't changed. Use this, when the same shader was
                             ///< previously used with custom bound states
    NoRasterizerState =
      NS_BIT(1),             ///< The rasterizer state that is associated with the shader will not be bound. Use this when you intend to bind a custom rasterizer
    NoDepthStencilState = NS_BIT(
      2),                    ///< The depth-stencil state that is associated with the shader will not be bound. Use this when you intend to bind a custom depth-stencil
    NoBlendState =
      NS_BIT(3),             ///< The blend state that is associated with the shader will not be bound. Use this when you intend to bind a custom blend
    NoStateBinding = NoRasterizerState | NoDepthStencilState | NoBlendState,

    Default = None
  };

  struct Bits
  {
    StorageType ForceRebind : 1;
    StorageType NoRasterizerState : 1;
    StorageType NoDepthStencilState : 1;
    StorageType NoBlendState : 1;
  };
};

NS_DECLARE_FLAGS_OPERATORS(nsShaderBindFlags);

//////////////////////////////////////////////////////////////////////////
// nsRenderContextFlags
//////////////////////////////////////////////////////////////////////////

struct NS_RENDERERCORE_DLL nsRenderContextFlags
{
  using StorageType = nsUInt32;

  enum Enum
  {
    None = 0,
    ShaderStateChanged = NS_BIT(0),
    TextureBindingChanged = NS_BIT(1),
    UAVBindingChanged = NS_BIT(2),
    SamplerBindingChanged = NS_BIT(3),
    BufferBindingChanged = NS_BIT(4),
    ConstantBufferBindingChanged = NS_BIT(5),
    MeshBufferBindingChanged = NS_BIT(6),
    MaterialBindingChanged = NS_BIT(7),

    AllStatesInvalid = ShaderStateChanged | TextureBindingChanged | UAVBindingChanged | SamplerBindingChanged | BufferBindingChanged |
                       ConstantBufferBindingChanged | MeshBufferBindingChanged,
    Default = None
  };

  struct Bits
  {
    StorageType ShaderStateChanged : 1;
    StorageType TextureBindingChanged : 1;
    StorageType UAVBindingChanged : 1;
    StorageType SamplerBindingChanged : 1;
    StorageType BufferBindingChanged : 1;
    StorageType ConstantBufferBindingChanged : 1;
    StorageType MeshBufferBindingChanged : 1;
    StorageType MaterialBindingChanged : 1;
  };
};

NS_DECLARE_FLAGS_OPERATORS(nsRenderContextFlags);

//////////////////////////////////////////////////////////////////////////
// nsDefaultSamplerFlags
//////////////////////////////////////////////////////////////////////////

struct NS_RENDERERCORE_DLL nsDefaultSamplerFlags
{
  using StorageType = nsUInt32;

  enum Enum
  {
    PointFiltering = 0,
    LinearFiltering = NS_BIT(0),

    Wrap = 0,
    Clamp = NS_BIT(1)
  };

  struct Bits
  {
    StorageType LinearFiltering : 1;
    StorageType Clamp : 1;
  };
};

NS_DECLARE_FLAGS_OPERATORS(nsDefaultSamplerFlags);

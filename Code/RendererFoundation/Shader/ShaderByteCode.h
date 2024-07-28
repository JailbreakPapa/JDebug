
#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Types/RefCounted.h>
#include <RendererFoundation/Descriptors/Enumerations.h>
#include <RendererFoundation/RendererFoundationDLL.h>

/// \brief The reflection data of a constant in a shader constant buffer.
/// \sa nsShaderConstantBufferLayout
struct NS_RENDERERFOUNDATION_DLL nsShaderConstant
{
  NS_DECLARE_MEM_RELOCATABLE_TYPE();

  struct Type
  {
    using StorageType = nsUInt8;

    enum Enum
    {
      Default,
      Float1,
      Float2,
      Float3,
      Float4,
      Int1,
      Int2,
      Int3,
      Int4,
      UInt1,
      UInt2,
      UInt3,
      UInt4,
      Mat3x3,
      Mat4x4,
      Transform,
      Bool,
      Struct,
      ENUM_COUNT
    };
  };

  static nsUInt32 s_TypeSize[Type::ENUM_COUNT];

  void CopyDataFormVariant(nsUInt8* pDest, nsVariant* pValue) const;

  nsHashedString m_sName;
  nsEnum<Type> m_Type;
  nsUInt8 m_uiArrayElements = 0;
  nsUInt16 m_uiOffset = 0;
};

/// \brief Reflection data of a shader constant buffer.
/// \sa nsShaderResourceBinding
class NS_RENDERERFOUNDATION_DLL nsShaderConstantBufferLayout : public nsRefCounted
{
public:
  nsUInt32 m_uiTotalSize = 0;
  nsHybridArray<nsShaderConstant, 16> m_Constants;
};

/// \brief Shader reflection of the vertex shader input.
/// This is needed to figure out how to map a nsGALVertexDeclaration to a vertex shader stage.
/// \sa nsGALShaderByteCode
struct NS_RENDERERFOUNDATION_DLL nsShaderVertexInputAttribute
{
  NS_DECLARE_MEM_RELOCATABLE_TYPE();

  nsEnum<nsGALVertexAttributeSemantic> m_eSemantic = nsGALVertexAttributeSemantic::Position;
  nsEnum<nsGALResourceFormat> m_eFormat = nsGALResourceFormat::XYZFloat;
  nsUInt8 m_uiLocation = 0; // The bind slot of a vertex input
};

/// \brief Shader reflection of a single shader resource (texture, constant buffer, etc.).
/// \sa nsGALShaderByteCode
struct NS_RENDERERFOUNDATION_DLL nsShaderResourceBinding
{
  NS_DECLARE_MEM_RELOCATABLE_TYPE();
  nsEnum<nsGALShaderResourceType> m_ResourceType;             //< The type of shader resource. Note, not all are supported by NS right now.
  nsEnum<nsGALShaderTextureType> m_TextureType;               //< Only valid if m_ResourceType is Texture, TextureRW or TextureAndSampler.
  nsBitflags<nsGALShaderStageFlags> m_Stages;                 //< The shader stages under which this resource is bound.
  nsInt16 m_iSet = -1;                                        //< The set to which this resource belongs. Aka. Vulkan descriptor set.
  nsInt16 m_iSlot = -1;                                       //< The slot under which the resource needs to be bound in the set.
  nsUInt32 m_uiArraySize = 1;                                 //< Number of array elements. Only 1 is currently supported. 0 if bindless.
  nsHashedString m_sName;                                     //< Name under which a resource must be bound to fulfill this resource binding.
  nsScopedRefPointer<nsShaderConstantBufferLayout> m_pLayout; //< Only valid if nsGALShaderResourceType is ConstantBuffer or PushConstants. #TODO_SHADER We could also support this for StructuredBuffer / StructuredBufferRW, but currently there is no use case for that.

  static nsResult CreateMergedShaderResourceBinding(const nsArrayPtr<nsArrayPtr<const nsShaderResourceBinding>>& resourcesPerStage, nsDynamicArray<nsShaderResourceBinding>& out_bindings, bool bAllowMultipleBindingPerName);
};

/// \brief This class wraps shader byte code storage.
/// Since byte code can have different requirements for alignment, padding etc. this class manages it.
/// Also since byte code is shared between multiple shaders (e.g. same vertex shaders for different pixel shaders)
/// the instances of the byte codes are reference counted.
class NS_RENDERERFOUNDATION_DLL nsGALShaderByteCode : public nsRefCounted
{
public:
  nsGALShaderByteCode();
  ~nsGALShaderByteCode();

  inline const void* GetByteCode() const;
  inline nsUInt32 GetSize() const;
  inline bool IsValid() const;

public:
  // Filled out by Shader Compiler platform implementation
  nsDynamicArray<nsUInt8> m_ByteCode;
  nsHybridArray<nsShaderResourceBinding, 8> m_ShaderResourceBindings;
  nsHybridArray<nsShaderVertexInputAttribute, 8> m_ShaderVertexInput;
  // Only set in the hull shader.
  nsUInt8 m_uiTessellationPatchControlPoints = 0;

  // Filled out by compiler base library
  nsEnum<nsGALShaderStage> m_Stage = nsGALShaderStage::ENUM_COUNT;
  bool m_bWasCompiledWithDebug = false;
};

#include <RendererFoundation/Shader/Implementation/ShaderByteCode_inl.h>

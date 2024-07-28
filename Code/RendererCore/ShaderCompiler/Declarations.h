#pragma once

#include <Foundation/Strings/String.h>
#include <Foundation/Types/Bitflags.h>
#include <Foundation/Types/SharedPtr.h>
#include <RendererCore/Declarations.h>

/// \brief Output of ParseShaderResources. A shader resource definition found inside the shader source code.
struct nsShaderResourceDefinition
{
  /// \brief Just the declaration inside the shader source, e.g. "Texture1D Texture".
  nsStringView m_sDeclaration;
  /// \brief The declaration with any optional register mappings, e.g. "Texture1D Texture : register(12t, space3)"
  nsStringView m_sDeclarationAndRegister;
  /// \brief The extracted reflection of the resource containing type, slot, set etc.
  nsShaderResourceBinding m_Binding;
};

/// \brief Flags that affect the compilation process of a shader
struct nsShaderCompilerFlags
{
  using StorageType = nsUInt8;
  enum Enum
  {
    Debug = NS_BIT(0),
    Default = 0,
  };

  struct Bits
  {
    StorageType Debug : 1;
  };
};
NS_DECLARE_FLAGS_OPERATORS(nsShaderCompilerFlags);

/// \brief Storage used during the shader compilation process.
struct NS_RENDERERCORE_DLL nsShaderProgramData
{
  nsShaderProgramData()
  {
    m_sPlatform = {};
    m_sSourceFile = {};

    for (nsUInt32 stage = 0; stage < nsGALShaderStage::ENUM_COUNT; ++stage)
    {
      m_bWriteToDisk[stage] = true;
      m_sShaderSource[stage].Clear();
      m_Resources[stage].Clear();
      m_uiSourceHash[stage] = 0;
      m_ByteCode[stage].Clear();
    }
  }

  nsBitflags<nsShaderCompilerFlags> m_Flags;
  nsStringView m_sPlatform;
  nsStringView m_sSourceFile;
  nsString m_sShaderSource[nsGALShaderStage::ENUM_COUNT];
  nsHybridArray<nsShaderResourceDefinition, 8> m_Resources[nsGALShaderStage::ENUM_COUNT];
  nsUInt32 m_uiSourceHash[nsGALShaderStage::ENUM_COUNT];
  nsSharedPtr<nsGALShaderByteCode> m_ByteCode[nsGALShaderStage::ENUM_COUNT];
  bool m_bWriteToDisk[nsGALShaderStage::ENUM_COUNT];
};
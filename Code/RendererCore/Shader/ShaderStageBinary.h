#pragma once

#include <RendererCore/RendererCoreDLL.h>

#include <Foundation/Containers/HashTable.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/IO/Stream.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/Enum.h>
#include <Foundation/Types/SharedPtr.h>
#include <RendererFoundation/Descriptors/Descriptors.h>

/// Serializes nsGALShaderByteCode and provides access to the shader cache via LoadStageBinary.
class NS_RENDERERCORE_DLL nsShaderStageBinary
{
public:
  enum Version
  {
    Version0,
    Version1,
    Version2,
    Version3, // Added Material Parameters
    Version4, // Constant buffer layouts
    Version5, // Debug flag
    Version6, // Rewrite, no backwards compatibility. Moves all data into nsGALShaderByteCode.
    Version7, // Added tessellation support (m_uiTessellationPatchControlPoints)

    ENUM_COUNT,
    VersionCurrent = ENUM_COUNT - 1
  };

  nsShaderStageBinary();
  ~nsShaderStageBinary();

  nsSharedPtr<const nsGALShaderByteCode> GetByteCode() const;

private:
  friend class nsRenderContext;
  friend class nsShaderCompiler;
  friend class nsShaderPermutationResource;
  friend class nsShaderPermutationResourceLoader;

  nsResult WriteStageBinary(nsLogInterface* pLog, nsStringView sPlatform) const;
  nsResult Write(nsStreamWriter& inout_stream) const;
  nsResult Read(nsStreamReader& inout_stream);
  nsResult Write(nsStreamWriter& inout_stream, const nsShaderConstantBufferLayout& layout) const;
  nsResult Read(nsStreamReader& inout_stream, nsShaderConstantBufferLayout& out_layout);

private:
  nsUInt32 m_uiSourceHash = 0;
  nsSharedPtr<nsGALShaderByteCode> m_pGALByteCode;

private: // statics
  static nsShaderStageBinary* LoadStageBinary(nsGALShaderStage::Enum Stage, nsUInt32 uiHash, nsStringView sPlatform);

  static void OnEngineShutdown();

  static nsMap<nsUInt32, nsShaderStageBinary> s_ShaderStageBinaries[nsGALShaderStage::ENUM_COUNT];
};

#pragma once

#include <Foundation/IO/DependencyFile.h>
#include <RendererCore/Declarations.h>
#include <RendererCore/Shader/ShaderStageBinary.h>
#include <RendererFoundation/Descriptors/Descriptors.h>

struct NS_RENDERERCORE_DLL nsShaderStateResourceDescriptor
{
  nsGALBlendStateCreationDescription m_BlendDesc;
  nsGALDepthStencilStateCreationDescription m_DepthStencilDesc;
  nsGALRasterizerStateCreationDescription m_RasterizerDesc;

  nsResult Parse(const char* szSource);
  void Load(nsStreamReader& inout_stream);
  void Save(nsStreamWriter& inout_stream) const;

  nsUInt32 CalculateHash() const;
};

/// \brief Serialized state of a shader permutation used by nsShaderPermutationResourceLoader to convert into a nsShaderPermutationResource.
class NS_RENDERERCORE_DLL nsShaderPermutationBinary
{
public:
  nsShaderPermutationBinary();

  nsResult Write(nsStreamWriter& inout_stream);
  nsResult Read(nsStreamReader& inout_stream, bool& out_bOldVersion);

  // Actual binary will be loaded from the hash via nsShaderStageBinary::LoadStageBinary to produce nsShaderStageBinary
  nsUInt32 m_uiShaderStageHashes[nsGALShaderStage::ENUM_COUNT];

  nsDependencyFile m_DependencyFile;

  nsShaderStateResourceDescriptor m_StateDescriptor;

  nsHybridArray<nsPermutationVar, 16> m_PermutationVars;
};

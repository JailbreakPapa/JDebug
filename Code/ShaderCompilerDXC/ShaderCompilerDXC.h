#pragma once

#include <RendererCore/ShaderCompiler/ShaderCompiler.h>
#include <ShaderCompilerDXC/ShaderCompilerDXCDLL.h>

struct SpvReflectDescriptorBinding;
struct SpvReflectBlockVariable;

class NS_SHADERCOMPILERDXC_DLL nsShaderCompilerDXC : public nsShaderProgramCompiler
{
  NS_ADD_DYNAMIC_REFLECTION(nsShaderCompilerDXC, nsShaderProgramCompiler);

public:
  virtual void GetSupportedPlatforms(nsHybridArray<nsString, 4>& out_platforms) override { out_platforms.PushBack("VULKAN"); }

  virtual nsResult ModifyShaderSource(nsShaderProgramData& inout_data, nsLogInterface* pLog) override;
  virtual nsResult Compile(nsShaderProgramData& inout_Data, nsLogInterface* pLog) override;

private:
  /// \brief Sets fixed set / slot bindings to each resource.
  /// The end result will have these properties:
  /// 1. Every binding name has a unique set / slot.
  /// 2. Bindings that already had a fixed set or slot (e.g. != -1) should not have these changed.
  /// 2. Set / slots can only be the same for two bindings if they have been changed to nsGALShaderResourceType::TextureAndSampler.
  nsResult DefineShaderResourceBindings(const nsShaderProgramData& data, nsHashTable<nsHashedString, nsShaderResourceBinding>& inout_resourceBinding, nsLogInterface* pLog);

  void CreateNewShaderResourceDeclaration(nsStringView sPlatform, nsStringView sDeclaration, const nsShaderResourceBinding& binding, nsStringBuilder& out_sDeclaration);

  nsResult ReflectShaderStage(nsShaderProgramData& inout_Data, nsGALShaderStage::Enum Stage);
  nsShaderConstantBufferLayout* ReflectConstantBufferLayout(nsGALShaderByteCode& pStageBinary, const char* szName, const SpvReflectBlockVariable& block);
  nsResult FillResourceBinding(nsGALShaderByteCode& shaderBinary, nsShaderResourceBinding& binding, const SpvReflectDescriptorBinding& info);
  nsResult FillSRVResourceBinding(nsGALShaderByteCode& shaderBinary, nsShaderResourceBinding& binding, const SpvReflectDescriptorBinding& info);
  nsResult FillUAVResourceBinding(nsGALShaderByteCode& shaderBinary, nsShaderResourceBinding& binding, const SpvReflectDescriptorBinding& info);
  static nsGALShaderTextureType::Enum GetTextureType(const SpvReflectDescriptorBinding& info);
  nsResult Initialize();

private:
  nsMap<const char*, nsGALVertexAttributeSemantic::Enum, CompareConstChar> m_VertexInputMapping;
};

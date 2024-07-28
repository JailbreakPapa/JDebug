#include <RendererFoundation/RendererFoundationPCH.h>

#include <RendererFoundation/Shader/Shader.h>
#include <RendererFoundation/Shader/ShaderUtils.h>

nsGALShader::nsGALShader(const nsGALShaderCreationDescription& Description)
  : nsGALObject(Description)
{
}
nsArrayPtr<const nsShaderResourceBinding> nsGALShader::GetBindingMapping() const
{
  return m_BindingMapping;
}

const nsShaderResourceBinding* nsGALShader::GetShaderResourceBinding(const nsTempHashedString& sName) const
{
  for (auto& binding : m_BindingMapping)
  {
    if (binding.m_sName == sName)
    {
      return &binding;
    }
  }
  return nullptr;
}

nsArrayPtr<const nsShaderVertexInputAttribute> nsGALShader::GetVertexInputAttributes() const
{
  if (m_Description.HasByteCodeForStage(nsGALShaderStage::VertexShader))
  {
    return m_Description.m_ByteCodes[nsGALShaderStage::VertexShader]->m_ShaderVertexInput;
  }
  return {};
}

nsResult nsGALShader::CreateBindingMapping(bool bAllowMultipleBindingPerName)
{
  nsHybridArray<nsArrayPtr<const nsShaderResourceBinding>, nsGALShaderStage::ENUM_COUNT> resourceBinding;
  resourceBinding.SetCount(nsGALShaderStage::ENUM_COUNT);
  for (nsUInt32 stage = nsGALShaderStage::VertexShader; stage < nsGALShaderStage::ENUM_COUNT; ++stage)
  {
    if (m_Description.HasByteCodeForStage((nsGALShaderStage::Enum)stage))
    {
      resourceBinding[stage] = m_Description.m_ByteCodes[stage]->m_ShaderResourceBindings;
    }
  }
  return nsShaderResourceBinding::CreateMergedShaderResourceBinding(resourceBinding, m_BindingMapping, bAllowMultipleBindingPerName);
}

void nsGALShader::DestroyBindingMapping()
{
  m_BindingMapping.Clear();
}

nsGALShader::~nsGALShader() = default;

nsDelegate<void(nsShaderUtils::nsBuiltinShaderType type, nsShaderUtils::nsBuiltinShader& out_shader)> nsShaderUtils::g_RequestBuiltinShaderCallback;

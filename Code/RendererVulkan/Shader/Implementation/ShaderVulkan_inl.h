
vk::ShaderModule nsGALShaderVulkan::GetShader(nsGALShaderStage::Enum stage) const
{
  return m_Shaders[stage];
}

nsUInt32 nsGALShaderVulkan::GetSetCount() const
{
  return m_SetBindings.GetCount();
}

vk::DescriptorSetLayout nsGALShaderVulkan::GetDescriptorSetLayout(nsUInt32 uiSet) const
{
  NS_ASSERT_DEBUG(uiSet < m_descriptorSetLayout.GetCount(), "Set index out of range.");
  return m_descriptorSetLayout[uiSet];
}

nsArrayPtr<const nsShaderResourceBinding> nsGALShaderVulkan::GetBindings(nsUInt32 uiSet) const
{
  NS_ASSERT_DEBUG(uiSet < m_SetBindings.GetCount(), "Set index out of range.");
  return m_SetBindings[uiSet].GetArrayPtr();
}

vk::PushConstantRange nsGALShaderVulkan::GetPushConstantRange() const
{
  return m_pushConstants;
}

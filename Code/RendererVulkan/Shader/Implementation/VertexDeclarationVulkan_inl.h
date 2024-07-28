

nsArrayPtr<const vk::VertexInputAttributeDescription> nsGALVertexDeclarationVulkan::GetAttributes() const
{
  return m_attributes.GetArrayPtr();
}

nsArrayPtr<const vk::VertexInputBindingDescription> nsGALVertexDeclarationVulkan::GetBindings() const
{
  return m_bindings.GetArrayPtr();
}


NS_ALWAYS_INLINE const vk::PipelineColorBlendStateCreateInfo* nsGALBlendStateVulkan::GetBlendState() const
{
  return &m_blendState;
}

NS_ALWAYS_INLINE const vk::PipelineDepthStencilStateCreateInfo* nsGALDepthStencilStateVulkan::GetDepthStencilState() const
{
  return &m_depthStencilState;
}

NS_ALWAYS_INLINE const vk::PipelineRasterizationStateCreateInfo* nsGALRasterizerStateVulkan::GetRasterizerState() const
{
  return &m_rasterizerState;
}

NS_ALWAYS_INLINE const vk::DescriptorImageInfo& nsGALSamplerStateVulkan::GetImageInfo() const
{
  return m_resourceImageInfo;
}

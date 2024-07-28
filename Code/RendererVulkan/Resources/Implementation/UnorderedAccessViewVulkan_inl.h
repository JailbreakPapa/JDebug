

NS_ALWAYS_INLINE const vk::DescriptorImageInfo& nsGALTextureUnorderedAccessViewVulkan::GetImageInfo() const
{
  return m_resourceImageInfo;
}

NS_ALWAYS_INLINE vk::ImageSubresourceRange nsGALTextureUnorderedAccessViewVulkan::GetRange() const
{
  return m_range;
}

NS_ALWAYS_INLINE const vk::BufferView& nsGALBufferUnorderedAccessViewVulkan::GetBufferView() const
{
  return m_bufferView;
}

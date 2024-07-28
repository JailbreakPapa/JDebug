NS_ALWAYS_INLINE const vk::DescriptorImageInfo& nsGALTextureResourceViewVulkan::GetImageInfo(bool bIsArray) const
{
  NS_ASSERT_DEBUG((bIsArray ? m_resourceImageInfoArray : m_resourceImageInfo).imageView, "View does not support bIsArray: {}", bIsArray);
  return bIsArray ? m_resourceImageInfoArray : m_resourceImageInfo;
}

NS_ALWAYS_INLINE vk::ImageSubresourceRange nsGALTextureResourceViewVulkan::GetRange() const
{
  return m_range;
}

NS_ALWAYS_INLINE const vk::BufferView& nsGALBufferResourceViewVulkan::GetBufferView() const
{
  return m_bufferView;
}

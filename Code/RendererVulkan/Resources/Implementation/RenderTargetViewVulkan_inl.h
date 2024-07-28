

NS_ALWAYS_INLINE vk::ImageView nsGALRenderTargetViewVulkan::GetImageView() const
{
  return m_imageView;
}

NS_ALWAYS_INLINE bool nsGALRenderTargetViewVulkan::IsFullRange() const
{
  return m_bfullRange;
}

NS_ALWAYS_INLINE vk::ImageSubresourceRange nsGALRenderTargetViewVulkan::GetRange() const
{
  return m_range;
}

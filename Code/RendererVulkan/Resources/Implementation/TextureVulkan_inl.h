vk::Image nsGALTextureVulkan::GetImage() const
{
  return m_image;
}

vk::ImageLayout nsGALTextureVulkan::GetPreferredLayout() const
{
  return m_preferredLayout;
}

vk::ImageLayout nsGALTextureVulkan::GetPreferredLayout(vk::ImageLayout targetLayout) const
{
  return targetLayout;
  // #TODO_VULKAN Maintaining UAVs in general layout causes verification failures. For now, switch back and forth between layouts.
  // return m_preferredLayout == vk::ImageLayout::eGeneral ? vk::ImageLayout::eGeneral : targetLayout;
}

vk::PipelineStageFlags nsGALTextureVulkan::GetUsedByPipelineStage() const
{
  return m_stages;
}

vk::AccessFlags nsGALTextureVulkan::GetAccessMask() const
{
  return m_access;
}

nsVulkanAllocation nsGALTextureVulkan::GetAllocation() const
{
  return m_alloc;
}

const nsVulkanAllocationInfo& nsGALTextureVulkan::GetAllocationInfo() const
{
  return m_allocInfo;
}

bool nsGALTextureVulkan::IsLinearLayout() const
{
  return m_bLinearCPU;
}

nsGALTextureVulkan::StagingMode nsGALTextureVulkan::GetStagingMode() const
{
  return m_stagingMode;
}

nsGALTextureHandle nsGALTextureVulkan::GetStagingTexture() const
{
  return m_hStagingTexture;
}

nsGALBufferHandle nsGALTextureVulkan::GetStagingBuffer() const
{
  return m_hStagingBuffer;
}

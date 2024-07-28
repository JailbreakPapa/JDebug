
vk::Buffer nsGALBufferVulkan::GetVkBuffer() const
{
  m_currentBuffer.m_currentFrame = m_pDeviceVulkan->GetCurrentFrame();
  return m_currentBuffer.m_buffer;
}

vk::IndexType nsGALBufferVulkan::GetIndexType() const
{
  return m_indexType;
}

nsVulkanAllocation nsGALBufferVulkan::GetAllocation() const
{
  return m_currentBuffer.m_alloc;
}

const nsVulkanAllocationInfo& nsGALBufferVulkan::GetAllocationInfo() const
{
  return m_allocInfo;
}

vk::PipelineStageFlags nsGALBufferVulkan::GetUsedByPipelineStage() const
{
  return m_stages;
}

vk::AccessFlags nsGALBufferVulkan::GetAccessMask() const
{
  return m_access;
}

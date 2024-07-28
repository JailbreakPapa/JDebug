#include <RendererVulkan/RendererVulkanPCH.h>

#include <RendererVulkan/Pools/StagingBufferPoolVulkan.h>

#include <RendererVulkan/Device/DeviceVulkan.h>

void nsStagingBufferPoolVulkan::Initialize(nsGALDeviceVulkan* pDevice)
{
  m_pDevice = pDevice;
  m_device = pDevice->GetVulkanDevice();
}

void nsStagingBufferPoolVulkan::DeInitialize()
{
  m_device = nullptr;
}

nsStagingBufferVulkan nsStagingBufferPoolVulkan::AllocateBuffer(vk::DeviceSize alignment, vk::DeviceSize size)
{
  // #TODO_VULKAN alignment
  nsStagingBufferVulkan buffer;

  NS_ASSERT_DEBUG(m_device, "nsStagingBufferPoolVulkan::Initialize not called");
  vk::BufferCreateInfo bufferCreateInfo = {};
  bufferCreateInfo.size = size;
  bufferCreateInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;

  bufferCreateInfo.pQueueFamilyIndices = nullptr;
  bufferCreateInfo.queueFamilyIndexCount = 0;
  bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;


  nsVulkanAllocationCreateInfo allocInfo;
  allocInfo.m_usage = nsVulkanMemoryUsage::Auto;
  allocInfo.m_flags = nsVulkanAllocationCreateFlags::HostAccessSequentialWrite;

  VK_ASSERT_DEV(nsMemoryAllocatorVulkan::CreateBuffer(bufferCreateInfo, allocInfo, buffer.m_buffer, buffer.m_alloc, &buffer.m_allocInfo));

  return buffer;
}

void nsStagingBufferPoolVulkan::ReclaimBuffer(nsStagingBufferVulkan& buffer)
{
  m_pDevice->DeleteLater(buffer.m_buffer, buffer.m_alloc);

  // NS_ASSERT_DEBUG(m_device, "nsStagingBufferPoolVulkan::Initialize not called");
  // nsMemoryAllocatorVulkan::DestroyBuffer(buffer.m_buffer, buffer.m_alloc);
}

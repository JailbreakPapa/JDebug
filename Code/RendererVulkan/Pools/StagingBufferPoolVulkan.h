#pragma once

#include <RendererVulkan/MemoryAllocator/MemoryAllocatorVulkan.h>
#include <RendererVulkan/RendererVulkanDLL.h>

#include <vulkan/vulkan.hpp>

class nsGALDeviceVulkan;

struct nsStagingBufferVulkan
{
  vk::Buffer m_buffer;
  nsVulkanAllocation m_alloc;
  nsVulkanAllocationInfo m_allocInfo;
};

class NS_RENDERERVULKAN_DLL nsStagingBufferPoolVulkan
{
public:
  void Initialize(nsGALDeviceVulkan* pDevice);
  void DeInitialize();

  nsStagingBufferVulkan AllocateBuffer(vk::DeviceSize alignment, vk::DeviceSize size);
  void ReclaimBuffer(nsStagingBufferVulkan& buffer);

private:
  nsGALDeviceVulkan* m_pDevice = nullptr;
  vk::Device m_device;
};

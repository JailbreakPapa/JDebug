
#pragma once

#include <RendererFoundation/Resources/Buffer.h>

#include <RendererVulkan/Device/DeviceVulkan.h>

#include <vulkan/vulkan.hpp>

class NS_RENDERERVULKAN_DLL nsGALBufferVulkan : public nsGALBuffer
{
public:
  void DiscardBuffer() const;
  NS_ALWAYS_INLINE vk::Buffer GetVkBuffer() const;
  const vk::DescriptorBufferInfo& GetBufferInfo() const;

  NS_ALWAYS_INLINE vk::IndexType GetIndexType() const;
  NS_ALWAYS_INLINE nsVulkanAllocation GetAllocation() const;
  NS_ALWAYS_INLINE const nsVulkanAllocationInfo& GetAllocationInfo() const;
  NS_ALWAYS_INLINE vk::PipelineStageFlags GetUsedByPipelineStage() const;
  NS_ALWAYS_INLINE vk::AccessFlags GetAccessMask() const;
  static vk::DeviceSize GetAlignment(const nsGALDeviceVulkan* pDevice, vk::BufferUsageFlags usage);

protected:
  struct BufferVulkan
  {
    vk::Buffer m_buffer;
    nsVulkanAllocation m_alloc;
    mutable nsUInt64 m_currentFrame = 0;
  };

  friend class nsGALDeviceVulkan;
  friend class nsMemoryUtils;

  nsGALBufferVulkan(const nsGALBufferCreationDescription& Description, bool bCPU = false);

  virtual ~nsGALBufferVulkan();

  virtual nsResult InitPlatform(nsGALDevice* pDevice, nsArrayPtr<const nsUInt8> pInitialData) override;
  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) override;
  virtual void SetDebugNamePlatform(const char* szName) const override;
  void CreateBuffer() const;

  mutable BufferVulkan m_currentBuffer;
  mutable vk::DescriptorBufferInfo m_resourceBufferInfo;
  mutable nsDeque<BufferVulkan> m_usedBuffers;
  mutable nsVulkanAllocationInfo m_allocInfo;

  // Data for memory barriers and access
  vk::PipelineStageFlags m_stages = {};
  vk::AccessFlags m_access = {};
  vk::IndexType m_indexType = vk::IndexType::eUint16; // Only applicable for index buffers
  vk::BufferUsageFlags m_usage = {};
  vk::DeviceSize m_size = 0;

  nsGALDeviceVulkan* m_pDeviceVulkan = nullptr;
  vk::Device m_device;

  bool m_bCPU = false;
  mutable nsString m_sDebugName;
};

#include <RendererVulkan/Resources/Implementation/BufferVulkan_inl.h>

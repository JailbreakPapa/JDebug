#include <RendererVulkan/RendererVulkanPCH.h>

#include <Foundation/Memory/MemoryUtils.h>
#include <RendererVulkan/Device/DeviceVulkan.h>
#include <RendererVulkan/RendererVulkanDLL.h>
#include <RendererVulkan/Resources/BufferVulkan.h>

nsGALBufferVulkan::nsGALBufferVulkan(const nsGALBufferCreationDescription& Description, bool bCPU)
  : nsGALBuffer(Description)
  , m_bCPU(bCPU)
{
}

nsGALBufferVulkan::~nsGALBufferVulkan() {}

nsResult nsGALBufferVulkan::InitPlatform(nsGALDevice* pDevice, nsArrayPtr<const nsUInt8> pInitialData)
{
  m_pDeviceVulkan = static_cast<nsGALDeviceVulkan*>(pDevice);
  m_device = m_pDeviceVulkan->GetVulkanDevice();
  m_stages = vk::PipelineStageFlagBits::eTransfer;

  const bool bSRV = m_Description.m_BufferFlags.IsSet(nsGALBufferUsageFlags::ShaderResource);
  const bool bUAV = m_Description.m_BufferFlags.IsSet(nsGALBufferUsageFlags::UnorderedAccess);
  for (nsGALBufferUsageFlags::Enum flag : m_Description.m_BufferFlags)
  {
    switch (flag)
    {
      case nsGALBufferUsageFlags::VertexBuffer:
        m_usage |= vk::BufferUsageFlagBits::eVertexBuffer;
        m_stages |= vk::PipelineStageFlagBits::eVertexInput;
        m_access |= vk::AccessFlagBits::eVertexAttributeRead;
        // NS_ASSERT_DEBUG(!bSRV && !bUAV, "Not implemented");
        break;
      case nsGALBufferUsageFlags::IndexBuffer:
        m_usage |= vk::BufferUsageFlagBits::eIndexBuffer;
        m_stages |= vk::PipelineStageFlagBits::eVertexInput;
        m_access |= vk::AccessFlagBits::eIndexRead;
        m_indexType = m_Description.m_uiStructSize == 2 ? vk::IndexType::eUint16 : vk::IndexType::eUint32;
        // NS_ASSERT_DEBUG(!bSRV && !bUAV, "Not implemented");
        break;
      case nsGALBufferUsageFlags::ConstantBuffer:
        m_usage |= vk::BufferUsageFlagBits::eUniformBuffer;
        m_stages |= m_pDeviceVulkan->GetSupportedStages();
        m_access |= vk::AccessFlagBits::eUniformRead;
        break;
      case nsGALBufferUsageFlags::TexelBuffer:
        if (bSRV)
          m_usage |= vk::BufferUsageFlagBits::eUniformTexelBuffer;
        if (bUAV)
          m_usage |= vk::BufferUsageFlagBits::eStorageTexelBuffer;
        break;
      case nsGALBufferUsageFlags::StructuredBuffer:
      case nsGALBufferUsageFlags::ByteAddressBuffer:
        m_usage |= vk::BufferUsageFlagBits::eStorageBuffer;
        break;
      case nsGALBufferUsageFlags::ShaderResource:
        m_stages |= m_pDeviceVulkan->GetSupportedStages();
        m_access |= vk::AccessFlagBits::eShaderRead;
        break;
      case nsGALBufferUsageFlags::UnorderedAccess:
        m_stages |= m_pDeviceVulkan->GetSupportedStages();
        m_access |= vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
        break;
      case nsGALBufferUsageFlags::DrawIndirect:
        m_usage |= vk::BufferUsageFlagBits::eIndirectBuffer;
        m_stages |= vk::PipelineStageFlagBits::eDrawIndirect;
        m_access |= vk::AccessFlagBits::eIndirectCommandRead;
        break;
      default:
        nsLog::Error("Unknown buffer type supplied to CreateBuffer()!");
        return NS_FAILURE;
    }
  }

  // if (m_Description.m_ResourceAccess.m_bReadBack)
  {
    m_usage |= vk::BufferUsageFlagBits::eTransferSrc;
    m_access |= vk::AccessFlagBits::eTransferRead;
  }

  m_usage |= vk::BufferUsageFlagBits::eTransferDst;
  m_access |= vk::AccessFlagBits::eTransferWrite;

  NS_ASSERT_DEBUG(pInitialData.GetCount() <= m_Description.m_uiTotalSize, "Initial data is bigger than target buffer.");
  vk::DeviceSize alignment = GetAlignment(m_pDeviceVulkan, m_usage);
  m_size = nsMemoryUtils::AlignSize((vk::DeviceSize)m_Description.m_uiTotalSize, alignment);

  CreateBuffer();

  m_resourceBufferInfo.offset = 0;
  m_resourceBufferInfo.range = m_size;

  if (!pInitialData.IsEmpty())
  {
    void* pData = nullptr;
    VK_ASSERT_DEV(nsMemoryAllocatorVulkan::MapMemory(m_currentBuffer.m_alloc, &pData));
    NS_ASSERT_DEV(pData, "Implementation error");
    nsMemoryUtils::Copy((nsUInt8*)pData, pInitialData.GetPtr(), pInitialData.GetCount());
    nsMemoryAllocatorVulkan::UnmapMemory(m_currentBuffer.m_alloc);
  }
  return NS_SUCCESS;
}

nsResult nsGALBufferVulkan::DeInitPlatform(nsGALDevice* pDevice)
{
  if (m_currentBuffer.m_buffer)
  {
    m_pDeviceVulkan->DeleteLater(m_currentBuffer.m_buffer, m_currentBuffer.m_alloc);
    m_allocInfo = {};
  }
  for (auto& bufferVulkan : m_usedBuffers)
  {
    m_pDeviceVulkan->DeleteLater(bufferVulkan.m_buffer, bufferVulkan.m_alloc);
  }
  m_usedBuffers.Clear();
  m_resourceBufferInfo = vk::DescriptorBufferInfo();

  m_stages = {};
  m_access = {};
  m_indexType = vk::IndexType::eUint16;
  m_usage = {};
  m_size = 0;

  m_pDeviceVulkan = nullptr;
  m_device = nullptr;

  return NS_SUCCESS;
}

void nsGALBufferVulkan::DiscardBuffer() const
{
  m_usedBuffers.PushBack(m_currentBuffer);
  m_currentBuffer = {};

  nsUInt64 uiSafeFrame = m_pDeviceVulkan->GetSafeFrame();
  if (m_usedBuffers.PeekFront().m_currentFrame <= uiSafeFrame)
  {
    m_currentBuffer = m_usedBuffers.PeekFront();
    m_usedBuffers.PopFront();
    m_allocInfo = nsMemoryAllocatorVulkan::GetAllocationInfo(m_currentBuffer.m_alloc);
  }
  else
  {
    CreateBuffer();
    SetDebugNamePlatform(m_sDebugName);
  }
}

const vk::DescriptorBufferInfo& nsGALBufferVulkan::GetBufferInfo() const
{
  m_currentBuffer.m_currentFrame = m_pDeviceVulkan->GetCurrentFrame();
  // Vulkan buffers get constantly swapped out for new ones so the vk::Buffer pointer is not persistent.
  // We need to acquire the latest one on every request for rendering.
  m_resourceBufferInfo.buffer = m_currentBuffer.m_buffer;
  return m_resourceBufferInfo;
}

void nsGALBufferVulkan::CreateBuffer() const
{
  vk::BufferCreateInfo bufferCreateInfo;
  bufferCreateInfo.usage = m_usage;
  bufferCreateInfo.pQueueFamilyIndices = nullptr;
  bufferCreateInfo.queueFamilyIndexCount = 0;
  bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
  bufferCreateInfo.size = m_size;

  nsVulkanAllocationCreateInfo allocCreateInfo;
  allocCreateInfo.m_usage = nsVulkanMemoryUsage::Auto;
  if (m_bCPU)
  {
    allocCreateInfo.m_flags = nsVulkanAllocationCreateFlags::HostAccessRandom;
  }
  else
  {
    allocCreateInfo.m_flags = nsVulkanAllocationCreateFlags::HostAccessSequentialWrite;
  }
  VK_ASSERT_DEV(nsMemoryAllocatorVulkan::CreateBuffer(bufferCreateInfo, allocCreateInfo, m_currentBuffer.m_buffer, m_currentBuffer.m_alloc, &m_allocInfo));
}

void nsGALBufferVulkan::SetDebugNamePlatform(const char* szName) const
{
  m_sDebugName = szName;
  m_pDeviceVulkan->SetDebugName(szName, m_currentBuffer.m_buffer, m_currentBuffer.m_alloc);
}

vk::DeviceSize nsGALBufferVulkan::GetAlignment(const nsGALDeviceVulkan* pDevice, vk::BufferUsageFlags usage)
{
  const vk::PhysicalDeviceProperties& properties = pDevice->GetPhysicalDeviceProperties();

  vk::DeviceSize alignment = nsMath::Max<vk::DeviceSize>(4, properties.limits.nonCoherentAtomSize);

  if (usage & vk::BufferUsageFlagBits::eUniformBuffer)
    alignment = nsMath::Max(alignment, properties.limits.minUniformBufferOffsetAlignment);

  if (usage & vk::BufferUsageFlagBits::eStorageBuffer)
    alignment = nsMath::Max(alignment, properties.limits.minStorageBufferOffsetAlignment);

  if (usage & (vk::BufferUsageFlagBits::eUniformTexelBuffer | vk::BufferUsageFlagBits::eStorageTexelBuffer))
    alignment = nsMath::Max(alignment, properties.limits.minTexelBufferOffsetAlignment);

  if (usage & (vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eIndirectBuffer))
    alignment = nsMath::Max(alignment, VkDeviceSize(16)); // If no cache line aligned perf will suffer.

  if (usage & (vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst))
    alignment = nsMath::Max(alignment, properties.limits.optimalBufferCopyOffsetAlignment);

  return alignment;
}

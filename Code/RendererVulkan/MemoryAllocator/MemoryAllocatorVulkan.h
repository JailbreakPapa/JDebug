#pragma once

#include <Foundation/Types/Bitflags.h>
#include <RendererVulkan/RendererVulkanDLL.h>

#include <vulkan/vulkan.hpp>

/// \brief Subset of VmaAllocationCreateFlagBits. Duplicated for abstraction purposes.
struct nsVulkanAllocationCreateFlags
{
  typedef nsUInt32 StorageType;
  enum Enum
  {
    DedicatedMemory = 0x00000001,
    NeverAllocate = 0x00000002,
    Mapped = 0x00000004,
    CanAlias = 0x00000200,
    HostAccessSequentialWrite = 0x00000400,
    HostAccessRandom = 0x00000800,
    StrategyMinMemory = 0x00010000,
    StrategyMinTime = 0x00020000,
    Default = 0,
  };

  struct Bits
  {
    StorageType DedicatedMemory : 1;
    StorageType NeverAllocate : 1;
    StorageType Mapped : 1;
    StorageType UnusedBit3 : 1;

    StorageType UnusedBit4 : 1;
    StorageType UnusedBit5 : 1;
    StorageType UnusedBit6 : 1;
    StorageType UnusedBit7 : 1;

    StorageType UnusedBit8 : 1;
    StorageType CanAlias : 1;
    StorageType HostAccessSequentialWrite : 1;
    StorageType HostAccessRandom : 1;

    StorageType UnusedBit12 : 1;
    StorageType UnusedBit13 : 1;
    StorageType UnusedBit14 : 1;
    StorageType StrategyMinMemory : 1;

    StorageType StrategyMinTime : 1;
  };
};
NS_DECLARE_FLAGS_OPERATORS(nsVulkanAllocationCreateFlags);

/// \brief Subset of VmaMemoryUsage. Duplicated for abstraction purposes.
struct nsVulkanMemoryUsage
{
  typedef nsUInt8 StorageType;
  enum Enum
  {
    Unknown = 0,
    GpuLazilyAllocated = 6,
    Auto = 7,
    AutoPreferDevice = 8,
    AutoPreferHost = 9,
    Default = Unknown,
  };
};

/// \brief Subset of VmaAllocationCreateInfo. Duplicated for abstraction purposes.
struct nsVulkanAllocationCreateInfo
{
  nsBitflags<nsVulkanAllocationCreateFlags> m_flags;
  nsEnum<nsVulkanMemoryUsage> m_usage;
  const char* m_pUserData = nullptr;
  bool m_bExportSharedAllocation = false; // If this allocation should be exported so other processes can access it.
};

/// \brief Subset of VmaAllocationInfo. Duplicated for abstraction purposes.
struct nsVulkanAllocationInfo
{
  uint32_t m_memoryType;
  vk::DeviceMemory m_deviceMemory;
  vk::DeviceSize m_offset;
  vk::DeviceSize m_size;
  void* m_pMappedData;
  void* m_pUserData;
  const char* m_pName;
};


VK_DEFINE_HANDLE(nsVulkanAllocation)

/// \brief Thin abstraction layer over VulkanMemoryAllocator to allow for abstraction and prevent pulling in its massive header into other files.
/// Functions are a subset of VMA's. To be extended once a use-case comes up.
class NS_RENDERERVULKAN_DLL nsMemoryAllocatorVulkan
{
public:
  static vk::Result Initialize(vk::PhysicalDevice physicalDevice, vk::Device device, vk::Instance instance);
  static void DeInitialize();

  static vk::Result CreateImage(const vk::ImageCreateInfo& imageCreateInfo, const nsVulkanAllocationCreateInfo& allocationCreateInfo, vk::Image& out_image, nsVulkanAllocation& out_alloc, nsVulkanAllocationInfo* pAllocInfo = nullptr);
  static void DestroyImage(vk::Image& image, nsVulkanAllocation& alloc);

  static vk::Result CreateBuffer(const vk::BufferCreateInfo& bufferCreateInfo, const nsVulkanAllocationCreateInfo& allocationCreateInfo, vk::Buffer& out_buffer, nsVulkanAllocation& out_alloc, nsVulkanAllocationInfo* pAllocInfo = nullptr);
  static void DestroyBuffer(vk::Buffer& buffer, nsVulkanAllocation& alloc);

  static nsVulkanAllocationInfo GetAllocationInfo(nsVulkanAllocation alloc);
  static void SetAllocationUserData(nsVulkanAllocation alloc, const char* pUserData);

  static vk::Result MapMemory(nsVulkanAllocation alloc, void** pData);
  static void UnmapMemory(nsVulkanAllocation alloc);
  static vk::Result FlushAllocation(nsVulkanAllocation alloc, vk::DeviceSize offset = 0, vk::DeviceSize size = VK_WHOLE_SIZE);
  static vk::Result InvalidateAllocation(nsVulkanAllocation alloc, vk::DeviceSize offset = 0, vk::DeviceSize size = VK_WHOLE_SIZE);


private:
  struct Impl;
  static Impl* m_pImpl;
};

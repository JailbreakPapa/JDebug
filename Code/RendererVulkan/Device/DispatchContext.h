#pragma once

#include <vulkan/vulkan.h>

class nsGALDeviceVulkan;

#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
#  define NS_DISPATCH_CONTEXT_MEMBER_NAME(Name) m_p##Name
#else
#  define NS_DISPATCH_CONTEXT_MEMBER_NAME(Name) Name
#endif

// A vulkan hpp compatible dispatch context.
class nsVulkanDispatchContext
{
public:
  void Init(nsGALDeviceVulkan& device);

  nsUInt32 getVkHeaderVersion() const { return VK_HEADER_VERSION; }

#if NS_ENABLED(NS_PLATFORM_LINUX)
  // VK_KHR_external_memory_fd
  PFN_vkGetMemoryFdKHR NS_DISPATCH_CONTEXT_MEMBER_NAME(vkGetMemoryFdKHR) = nullptr;
  PFN_vkGetMemoryFdPropertiesKHR NS_DISPATCH_CONTEXT_MEMBER_NAME(vkGetMemoryFdPropertiesKHR) = nullptr;

  // VK_KHR_external_semaphore_fd
  PFN_vkGetSemaphoreFdKHR NS_DISPATCH_CONTEXT_MEMBER_NAME(vkGetSemaphoreFdKHR) = nullptr;
  PFN_vkImportSemaphoreFdKHR NS_DISPATCH_CONTEXT_MEMBER_NAME(vkImportSemaphoreFdKHR) = nullptr;
#elif NS_ENABLED(NS_PLATFORM_WINDOWS)
  // VK_KHR_external_memory_win32
  PFN_vkGetMemoryWin32HandleKHR NS_DISPATCH_CONTEXT_MEMBER_NAME(vkGetMemoryWin32HandleKHR) = nullptr;
  PFN_vkGetMemoryWin32HandlePropertiesKHR NS_DISPATCH_CONTEXT_MEMBER_NAME(vkGetMemoryWin32HandlePropertiesKHR) = nullptr;

  // VK_KHR_external_semaphore_win32
  PFN_vkGetSemaphoreWin32HandleKHR NS_DISPATCH_CONTEXT_MEMBER_NAME(vkGetSemaphoreWin32HandleKHR) = nullptr;
  PFN_vkImportSemaphoreWin32HandleKHR NS_DISPATCH_CONTEXT_MEMBER_NAME(vkImportSemaphoreWin32HandleKHR) = nullptr;
#endif

#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
#  if NS_ENABLED(NS_PLATFORM_LINUX)
  // VK_KHR_external_memory_fd
  VkResult vkGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd) const;
  VkResult vkGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd, VkMemoryFdPropertiesKHR* pMemoryFdProperties) const;

  // VK_KHR_external_semaphore_fd
  VkResult vkGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd) const;
  VkResult vkImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo) const;
#  elif NS_ENABLED(NS_PLATFORM_WINDOWS)
  // VK_KHR_external_memory_win32
  VkResult vkGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pWin32Handle) const;
  VkResult vkGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, HANDLE Win32Handle, VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties) const;

  // VK_KHR_external_semaphore_win32
  VkResult vkGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pWin32Handle) const;
  VkResult vkImportSemaphoreWin32HandleKHR(VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo) const;
#  endif
#endif

private:
  nsGALDeviceVulkan* m_pDevice = nullptr;
};

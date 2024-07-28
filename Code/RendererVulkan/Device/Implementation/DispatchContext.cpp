#include <RendererVulkan/RendererVulkanPCH.h>

#include <RendererVulkan/Device/DeviceVulkan.h>
#include <RendererVulkan/Device/DispatchContext.h>

void nsVulkanDispatchContext::Init(nsGALDeviceVulkan& device)
{
  m_pDevice = &device;

  VkDevice nativeDevice = (VkDevice)device.GetVulkanDevice();
  const nsGALDeviceVulkan::Extensions& extensions = device.GetExtensions();
#if NS_ENABLED(NS_PLATFORM_LINUX)
  if (extensions.m_bExternalMemoryFd)
  {
    NS_DISPATCH_CONTEXT_MEMBER_NAME(vkGetMemoryFdKHR) = (PFN_vkGetMemoryFdKHR)vkGetDeviceProcAddr(nativeDevice, "vkGetMemoryFdKHR");
    NS_DISPATCH_CONTEXT_MEMBER_NAME(vkGetMemoryFdPropertiesKHR) = (PFN_vkGetMemoryFdPropertiesKHR)vkGetDeviceProcAddr(nativeDevice, "vkGetMemoryFdPropertiesKHR");
  }

  if (extensions.m_bExternalSemaphoreFd)
  {
    NS_DISPATCH_CONTEXT_MEMBER_NAME(vkGetSemaphoreFdKHR) = (PFN_vkGetSemaphoreFdKHR)vkGetDeviceProcAddr(nativeDevice, "vkGetSemaphoreFdKHR");
    NS_DISPATCH_CONTEXT_MEMBER_NAME(vkImportSemaphoreFdKHR) = (PFN_vkImportSemaphoreFdKHR)vkGetDeviceProcAddr(nativeDevice, "vkImportSemaphoreFdKHR");
  }
#elif NS_ENABLED(NS_PLATFORM_WINDOWS)
  if (extensions.m_bExternalMemoryWin32)
  {
    NS_DISPATCH_CONTEXT_MEMBER_NAME(vkGetMemoryWin32HandleKHR) = (PFN_vkGetMemoryWin32HandleKHR)vkGetDeviceProcAddr(nativeDevice, "vkGetMemoryWin32HandleKHR");
    NS_DISPATCH_CONTEXT_MEMBER_NAME(vkGetMemoryWin32HandlePropertiesKHR) = (PFN_vkGetMemoryWin32HandlePropertiesKHR)vkGetDeviceProcAddr(nativeDevice, "vkGetMemoryWin32HandlePropertiesKHR");
  }

  if (extensions.m_bExternalSemaphoreWin32)
  {
    NS_DISPATCH_CONTEXT_MEMBER_NAME(vkGetSemaphoreWin32HandleKHR) = (PFN_vkGetSemaphoreWin32HandleKHR)vkGetDeviceProcAddr(nativeDevice, "vkGetSemaphoreWin32HandleKHR");
    NS_DISPATCH_CONTEXT_MEMBER_NAME(vkImportSemaphoreWin32HandleKHR) = (PFN_vkImportSemaphoreWin32HandleKHR)vkGetDeviceProcAddr(nativeDevice, "vkImportSemaphoreWin32HandleKHR");
  }
#endif
}

#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
#  if NS_ENABLED(NS_PLATFORM_LINUX)

VkResult nsVulkanDispatchContext::vkGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd) const
{
  NS_ASSERT_DEBUG(m_pvkGetMemoryFdKHR != nullptr, "vkGetMemoryFdKHR not supported");
  return m_pvkGetMemoryFdKHR(device, pGetFdInfo, pFd);
}

VkResult nsVulkanDispatchContext::vkGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd, VkMemoryFdPropertiesKHR* pMemoryFdProperties) const
{
  NS_ASSERT_DEBUG(m_pvkGetMemoryFdPropertiesKHR != nullptr, "vkGetMemoryFdPropertiesKHR not supported");
  return m_pvkGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);
}

VkResult nsVulkanDispatchContext::vkGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd) const
{
  NS_ASSERT_DEBUG(m_pvkGetSemaphoreFdKHR != nullptr, "vkGetSemaphoreFdKHR not supported");
  return m_pvkGetSemaphoreFdKHR(device, pGetFdInfo, pFd);
}

VkResult nsVulkanDispatchContext::vkImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo) const
{
  NS_ASSERT_DEBUG(m_pvkImportSemaphoreFdKHR != nullptr, "vkImportSemaphoreFdKHR not supported");
  return m_pvkImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
}

#  elif NS_ENABLED(NS_PLATFORM_WINDOWS)

VkResult nsVulkanDispatchContext::vkGetMemoryWin32HandleKHR(VkDevice device, const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pWin32Handle) const
{
  NS_ASSERT_DEBUG(m_pvkGetMemoryWin32HandleKHR != nullptr, "vkGetMemoryWin32HandleKHR not supported");
  return m_pvkGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pWin32Handle);
}

VkResult nsVulkanDispatchContext::vkGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, HANDLE Win32Handle, VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties) const
{
  NS_ASSERT_DEBUG(m_pvkGetMemoryWin32HandlePropertiesKHR != nullptr, "vkGetMemoryWin32HandlePropertiesKHR not supported");
  return m_pvkGetMemoryWin32HandlePropertiesKHR(device, handleType, Win32Handle, pMemoryWin32HandleProperties);
}

VkResult nsVulkanDispatchContext::vkGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pWin32Handle) const
{
  NS_ASSERT_DEBUG(m_pvkGetSemaphoreWin32HandleKHR != nullptr, "vkGetSemaphoreWin32HandleKHR not supported");
  return m_pvkGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pWin32Handle);
}

VkResult nsVulkanDispatchContext::vkImportSemaphoreWin32HandleKHR(VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo) const
{
  NS_ASSERT_DEBUG(m_pvkImportSemaphoreWin32HandleKHR != nullptr, "vkImportSemaphoreWin32HandleKHR not supported");
  return m_pvkImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);
}

#  endif
#endif

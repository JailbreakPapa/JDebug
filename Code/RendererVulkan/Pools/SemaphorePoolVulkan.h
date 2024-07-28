#pragma once

#include <RendererVulkan/RendererVulkanDLL.h>

#include <vulkan/vulkan.hpp>

/// \brief Simple pool for semaphores
///
/// Do not call ReclaimSemaphore manually, instead call nsGALDeviceVulkan::ReclaimLater which will make sure to reclaim the semaphore once it is no longer in use.
/// Usage:
/// \code{.cpp}
///   vk::Semaphore s = nsSemaphorePoolVulkan::RequestSemaphore();
///   ...
///   nsGALDeviceVulkan* pDevice = ...;
///   pDevice->ReclaimLater(s);
/// \endcode
class NS_RENDERERVULKAN_DLL nsSemaphorePoolVulkan
{
public:
  static void Initialize(vk::Device device);
  static void DeInitialize();

  static vk::Semaphore RequestSemaphore();
  static void ReclaimSemaphore(vk::Semaphore& semaphore);

private:
  static nsHybridArray<vk::Semaphore, 4> s_semaphores;
  static vk::Device s_device;
};

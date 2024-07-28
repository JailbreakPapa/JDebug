#include <RendererVulkan/RendererVulkanPCH.h>

#include <RendererVulkan/Pools/SemaphorePoolVulkan.h>

vk::Device nsSemaphorePoolVulkan::s_device;
nsHybridArray<vk::Semaphore, 4> nsSemaphorePoolVulkan::s_semaphores;

void nsSemaphorePoolVulkan::Initialize(vk::Device device)
{
  s_device = device;
}

void nsSemaphorePoolVulkan::DeInitialize()
{
  for (vk::Semaphore& semaphore : s_semaphores)
  {
    s_device.destroySemaphore(semaphore, nullptr);
  }
  s_semaphores.Clear();
  s_semaphores.Compact();

  s_device = nullptr;
}

vk::Semaphore nsSemaphorePoolVulkan::RequestSemaphore()
{
  NS_ASSERT_DEBUG(s_device, "nsSemaphorePoolVulkan::Initialize not called");
  if (!s_semaphores.IsEmpty())
  {
    vk::Semaphore semaphore = s_semaphores.PeekBack();
    s_semaphores.PopBack();
    return semaphore;
  }
  else
  {
    vk::Semaphore semaphore;
    vk::SemaphoreCreateInfo semaphoreCreateInfo;
    VK_ASSERT_DEV(s_device.createSemaphore(&semaphoreCreateInfo, nullptr, &semaphore));
    return semaphore;
  }
}

void nsSemaphorePoolVulkan::ReclaimSemaphore(vk::Semaphore& semaphore)
{
  NS_ASSERT_DEBUG(s_device, "nsSemaphorePoolVulkan::Initialize not called");
  s_semaphores.PushBack(semaphore);
}

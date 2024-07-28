
#pragma once

#include <RendererFoundation/Descriptors/Descriptors.h>
#include <RendererFoundation/Device/SwapChain.h>
#include <RendererVulkan/RendererVulkanDLL.h>

#include <vulkan/vulkan.hpp>

class nsGALDeviceVulkan;

class nsGALSwapChainVulkan : public nsGALWindowSwapChain
{
public:
  virtual void AcquireNextRenderTarget(nsGALDevice* pDevice) override;
  virtual void PresentRenderTarget(nsGALDevice* pDevice) override;
  virtual nsResult UpdateSwapChain(nsGALDevice* pDevice, nsEnum<nsGALPresentMode> newPresentMode) override;

  NS_ALWAYS_INLINE vk::SwapchainKHR GetVulkanSwapChain() const;

protected:
  friend class nsGALDeviceVulkan;
  friend class nsMemoryUtils;

  nsGALSwapChainVulkan(const nsGALWindowSwapChainCreationDescription& Description);

  virtual ~nsGALSwapChainVulkan();

  virtual nsResult InitPlatform(nsGALDevice* pDevice) override;
  nsResult CreateSwapChainInternal();
  void DestroySwapChainInternal(nsGALDeviceVulkan* pVulkanDevice);
  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) override;

protected:
  nsGALDeviceVulkan* m_pVulkanDevice = nullptr;
  nsEnum<nsGALPresentMode> m_currentPresentMode;

  vk::SurfaceKHR m_vulkanSurface;
  vk::SwapchainKHR m_vulkanSwapChain;
  nsHybridArray<vk::Image, 3> m_swapChainImages;
  nsHybridArray<nsGALTextureHandle, 3> m_swapChainTextures;
  nsUInt32 m_uiCurrentSwapChainImage = 0;

  vk::Semaphore m_currentPipelineImageAvailableSemaphore;
};

#include <RendererVulkan/Device/Implementation/SwapChainVulkan_inl.h>

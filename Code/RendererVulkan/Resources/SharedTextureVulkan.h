#pragma once

#include <RendererVulkan/Resources/TextureVulkan.h>

class nsGALSharedTextureVulkan : public nsGALTextureVulkan, public nsGALSharedTexture
{
  using SUPER = nsGALTextureVulkan;

protected:
  friend class nsGALDeviceVulkan;
  friend class nsMemoryUtils;

  nsGALSharedTextureVulkan(const nsGALTextureCreationDescription& Description, nsEnum<nsGALSharedTextureType> sharedType, nsGALPlatformSharedHandle hSharedHandle);
  ~nsGALSharedTextureVulkan();

  virtual nsResult InitPlatform(nsGALDevice* pDevice, nsArrayPtr<nsGALSystemMemoryDescription> pInitialData) override;
  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) override;

  virtual nsGALPlatformSharedHandle GetSharedHandle() const override;
  virtual void WaitSemaphoreGPU(nsUInt64 uiValue) const override;
  virtual void SignalSemaphoreGPU(nsUInt64 uiValue) const override;

protected:
  nsEnum<nsGALSharedTextureType> m_SharedType = nsGALSharedTextureType::None;
  nsGALPlatformSharedHandle m_hSharedHandle;
  vk::Semaphore m_SharedSemaphore;
};

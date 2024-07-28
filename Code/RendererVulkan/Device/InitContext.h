
#pragma once

#include <Foundation/Types/UniquePtr.h>

class nsGALDeviceVulkan;
class nsPipelineBarrierVulkan;
class nsCommandBufferPoolVulkan;
class nsStagingBufferPoolVulkan;

/// \brief Thread-safe context for initializing resources. Records a command buffer that transitions all newly created resources into their initial state.
class nsInitContextVulkan
{
public:
  nsInitContextVulkan(nsGALDeviceVulkan* pDevice);
  ~nsInitContextVulkan();

  /// \brief Returns a finished command buffer of all background loading up to this point.
  ///    The command buffer is already ended and marked to be reclaimed so the only thing done on it should be to submit it.
  vk::CommandBuffer GetFinishedCommandBuffer();

  /// \brief Initializes a texture and moves it into its default state.
  /// \param pTexture The texture to initialize.
  /// \param createInfo The image creation info for the texture. Needed for initial state information.
  /// \param pInitialData The initial data of the texture. If not set, the initial content will be undefined.
  void InitTexture(const nsGALTextureVulkan* pTexture, vk::ImageCreateInfo& createInfo, nsArrayPtr<nsGALSystemMemoryDescription> pInitialData);

  /// \brief Needs to be called by the nsGALDeviceVulkan just before a texture is destroyed to clean up stale barriers.
  void TextureDestroyed(const nsGALTextureVulkan* pTexture);

private:
  void EnsureCommandBufferExists();

  nsGALDeviceVulkan* m_pDevice = nullptr;

  nsMutex m_Lock;
  vk::CommandBuffer m_currentCommandBuffer;
  nsUniquePtr<nsPipelineBarrierVulkan> m_pPipelineBarrier;
  nsUniquePtr<nsCommandBufferPoolVulkan> m_pCommandBufferPool;
  nsUniquePtr<nsStagingBufferPoolVulkan> m_pStagingBufferPool;
};

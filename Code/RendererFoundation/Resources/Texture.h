
#pragma once

#include <RendererFoundation/Descriptors/Descriptors.h>
#include <RendererFoundation/Resources/Resource.h>

class NS_RENDERERFOUNDATION_DLL nsGALTexture : public nsGALResource<nsGALTextureCreationDescription>
{
public:
protected:
  friend class nsGALDevice;

  nsGALTexture(const nsGALTextureCreationDescription& Description);

  virtual ~nsGALTexture();

  virtual nsResult InitPlatform(nsGALDevice* pDevice, nsArrayPtr<nsGALSystemMemoryDescription> pInitialData) = 0;

  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) = 0;

protected:
  nsGALTextureResourceViewHandle m_hDefaultResourceView;
  nsGALRenderTargetViewHandle m_hDefaultRenderTargetView;

  nsHashTable<nsUInt32, nsGALTextureResourceViewHandle> m_ResourceViews;
  nsHashTable<nsUInt32, nsGALRenderTargetViewHandle> m_RenderTargetViews;
  nsHashTable<nsUInt32, nsGALTextureUnorderedAccessViewHandle> m_UnorderedAccessViews;
};

/// \brief Optional interface for nsGALTexture if it was created via nsGALDevice::CreateSharedTexture.
/// A nsGALTexture can be a shared texture, but doesn't have to be. Access through nsGALDevice::GetSharedTexture.
class NS_RENDERERFOUNDATION_DLL nsGALSharedTexture
{
public:
  /// \brief Returns the handle that can be used to open this texture on another device / process. Call  nsGALDevice::OpenSharedTexture to do so.
  virtual nsGALPlatformSharedHandle GetSharedHandle() const = 0;
  /// \brief Before the current render pipeline is executed, the GPU will wait for the semaphore to have the given value.
  /// \param iValue Value the semaphore needs to have before the texture can be used.
  virtual void WaitSemaphoreGPU(nsUInt64 uiValue) const = 0;
  /// \brief Once the current render pipeline is done on the GPU, the semaphore will be signaled with the given value.
  /// \param iValue Value the semaphore is set to once we are done using the texture (after the current render pipeline).
  virtual void SignalSemaphoreGPU(nsUInt64 uiValue) const = 0;
};

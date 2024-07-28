#pragma once

#include <RendererFoundation/RendererFoundationDLL.h>

#include <RendererFoundation/Device/SwapChain.h>
#include <RendererFoundation/Resources/Texture.h>

/// \brief Description of a shared texture swap chain. Use nsGALSharedTextureSwapChain::Create to create instance.
struct nsGALSharedTextureSwapChainCreationDescription : public nsHashableStruct<nsGALSharedTextureSwapChainCreationDescription>
{
  nsGALTextureCreationDescription m_TextureDesc;
  nsHybridArray<nsGALPlatformSharedHandle, 3> m_Textures;
  /// \brief Called when rendering to a swap chain texture has been submitted to the GPU queue. Use this to get the new semaphore value of the previously armed texture.
  nsDelegate<void(nsUInt32 uiTextureIndex, nsUInt64 uiSemaphoreValue)> m_OnPresent;
};

/// \brief Use to render to a set of shared textures.
/// To use it, it needs to be armed with the next shared texture index and its current semaphore value.
class NS_RENDERERFOUNDATION_DLL nsGALSharedTextureSwapChain : public nsGALSwapChain
{
  friend class nsGALDevice;

public:
  using Functor = nsDelegate<nsGALSwapChainHandle(const nsGALSharedTextureSwapChainCreationDescription&)>;
  static void SetFactoryMethod(Functor factory);

  /// \brief Creates an instance of a nsGALSharedTextureSwapChain.
  static nsGALSwapChainHandle Create(const nsGALSharedTextureSwapChainCreationDescription& desc);

public:
  /// \brief Call this before rendering.
  /// \param uiTextureIndex Texture to render into.
  /// \param uiCurrentSemaphoreValue Current semaphore value of the texture.
  void Arm(nsUInt32 uiTextureIndex, nsUInt64 uiCurrentSemaphoreValue);

protected:
  nsGALSharedTextureSwapChain(const nsGALSharedTextureSwapChainCreationDescription& desc);
  virtual void AcquireNextRenderTarget(nsGALDevice* pDevice) override;
  virtual void PresentRenderTarget(nsGALDevice* pDevice) override;
  virtual nsResult UpdateSwapChain(nsGALDevice* pDevice, nsEnum<nsGALPresentMode> newPresentMode) override;
  virtual nsResult InitPlatform(nsGALDevice* pDevice) override;
  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) override;

protected:
  static Functor s_Factory;

protected:
  nsUInt32 m_uiCurrentTexture = nsMath::MaxValue<nsUInt32>();
  nsUInt64 m_uiCurrentSemaphoreValue = 0;
  nsHybridArray<nsGALTextureHandle, 3> m_SharedTextureHandles;
  nsHybridArray<const nsGALSharedTexture*, 3> m_SharedTextureInterfaces;
  nsHybridArray<nsUInt64, 3> m_CurrentSemaphoreValue;
  nsGALSharedTextureSwapChainCreationDescription m_Desc = {};
};
NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERFOUNDATION_DLL, nsGALSharedTextureSwapChain);

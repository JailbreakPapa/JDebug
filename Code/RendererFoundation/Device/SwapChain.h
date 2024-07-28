
#pragma once

#include <RendererFoundation/RendererFoundationDLL.h>

#include <Foundation/Math/Size.h>
#include <RendererFoundation/Descriptors/Descriptors.h>
#include <RendererFoundation/Resources/RenderTargetSetup.h>

class NS_RENDERERFOUNDATION_DLL nsGALSwapChain : public nsGALObject<nsGALSwapChainCreationDescription>
{
public:
  const nsGALRenderTargets& GetRenderTargets() const { return m_RenderTargets; }
  nsGALTextureHandle GetBackBufferTexture() const { return m_RenderTargets.m_hRTs[0]; }
  nsSizeU32 GetCurrentSize() const { return m_CurrentSize; }

  virtual void AcquireNextRenderTarget(nsGALDevice* pDevice) = 0;
  virtual void PresentRenderTarget(nsGALDevice* pDevice) = 0;
  virtual nsResult UpdateSwapChain(nsGALDevice* pDevice, nsEnum<nsGALPresentMode> newPresentMode) = 0;

  virtual ~nsGALSwapChain();

protected:
  friend class nsGALDevice;

  nsGALSwapChain(const nsRTTI* pSwapChainType);

  virtual nsResult InitPlatform(nsGALDevice* pDevice) = 0;
  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) = 0;

  nsGALRenderTargets m_RenderTargets;
  nsSizeU32 m_CurrentSize = {};
};
NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERFOUNDATION_DLL, nsGALSwapChain);


class NS_RENDERERFOUNDATION_DLL nsGALWindowSwapChain : public nsGALSwapChain
{
public:
  using Functor = nsDelegate<nsGALSwapChainHandle(const nsGALWindowSwapChainCreationDescription&)>;
  static void SetFactoryMethod(Functor factory);

  static nsGALSwapChainHandle Create(const nsGALWindowSwapChainCreationDescription& desc);

public:
  const nsGALWindowSwapChainCreationDescription& GetWindowDescription() const { return m_WindowDesc; }

protected:
  nsGALWindowSwapChain(const nsGALWindowSwapChainCreationDescription& Description);

protected:
  static Functor s_Factory;

protected:
  nsGALWindowSwapChainCreationDescription m_WindowDesc;
};
NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERFOUNDATION_DLL, nsGALWindowSwapChain);

#include <RendererFoundation/Device/Implementation/SwapChain_inl.h>

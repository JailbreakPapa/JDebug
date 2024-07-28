#include <RendererFoundation/RendererFoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Device/SharedTextureSwapChain.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsGALSharedTextureSwapChain, nsGALSwapChain, 1, nsRTTINoAllocator)
{
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

nsGALSharedTextureSwapChain::Functor nsGALSharedTextureSwapChain::s_Factory;

void nsGALSharedTextureSwapChain::SetFactoryMethod(Functor factory)
{
  s_Factory = factory;
}

nsGALSwapChainHandle nsGALSharedTextureSwapChain::Create(const nsGALSharedTextureSwapChainCreationDescription& desc)
{
  NS_ASSERT_DEV(s_Factory.IsValid(), "No factory method assigned for nsGALWindowSwapChain.");
  return s_Factory(desc);
}

nsGALSharedTextureSwapChain::nsGALSharedTextureSwapChain(const nsGALSharedTextureSwapChainCreationDescription& desc)
  : nsGALSwapChain(nsGetStaticRTTI<nsGALSharedTextureSwapChain>())
  , m_Desc(desc)
{
}

void nsGALSharedTextureSwapChain::Arm(nsUInt32 uiTextureIndex, nsUInt64 uiCurrentSemaphoreValue)
{
  if (m_uiCurrentTexture != nsMath::MaxValue<nsUInt32>())
  {
    // We did not use the previous texture index.
    m_Desc.m_OnPresent(m_uiCurrentTexture, m_uiCurrentSemaphoreValue);
  }
  m_uiCurrentTexture = uiTextureIndex;
  m_uiCurrentSemaphoreValue = uiCurrentSemaphoreValue;

  m_RenderTargets.m_hRTs[0] = m_SharedTextureHandles[m_uiCurrentTexture];
}

void nsGALSharedTextureSwapChain::AcquireNextRenderTarget(nsGALDevice* pDevice)
{
  NS_ASSERT_DEV(m_uiCurrentTexture != nsMath::MaxValue<nsUInt32>(), "Acquire called without calling Arm first.");

  m_RenderTargets.m_hRTs[0] = m_SharedTextureHandles[m_uiCurrentTexture];
  m_SharedTextureInterfaces[m_uiCurrentTexture]->WaitSemaphoreGPU(m_uiCurrentSemaphoreValue);
}

void nsGALSharedTextureSwapChain::PresentRenderTarget(nsGALDevice* pDevice)
{
  m_RenderTargets.m_hRTs[0].Invalidate();

  NS_ASSERT_DEV(m_uiCurrentTexture != nsMath::MaxValue<nsUInt32>(), "Present called without calling Arm first.");

  m_SharedTextureInterfaces[m_uiCurrentTexture]->SignalSemaphoreGPU(m_uiCurrentSemaphoreValue + 1);
  m_Desc.m_OnPresent(m_uiCurrentTexture, m_uiCurrentSemaphoreValue + 1);

  pDevice->Flush();

  m_uiCurrentTexture = nsMath::MaxValue<nsUInt32>();
}

nsResult nsGALSharedTextureSwapChain::UpdateSwapChain(nsGALDevice* pDevice, nsEnum<nsGALPresentMode> newPresentMode)
{
  return NS_SUCCESS;
}

nsResult nsGALSharedTextureSwapChain::InitPlatform(nsGALDevice* pDevice)
{
  // Create textures
  for (nsUInt32 i = 0; i < m_Desc.m_Textures.GetCount(); ++i)
  {
    nsGALPlatformSharedHandle handle = m_Desc.m_Textures[i];
    nsGALTextureHandle hTexture = pDevice->OpenSharedTexture(m_Desc.m_TextureDesc, handle);
    if (hTexture.IsInvalidated())
    {
      nsLog::Error("Failed to open shared texture");
      return NS_FAILURE;
    }
    m_SharedTextureHandles.PushBack(hTexture);
    const nsGALSharedTexture* pSharedTexture = pDevice->GetSharedTexture(hTexture);
    if (pSharedTexture == nullptr)
    {
      nsLog::Error("Created texture is not a shared texture");
      return NS_FAILURE;
    }
    m_SharedTextureInterfaces.PushBack(pSharedTexture);
    m_CurrentSemaphoreValue.PushBack(0);
  }
  m_RenderTargets.m_hRTs[0] = m_SharedTextureHandles[0];
  m_CurrentSize = {m_Desc.m_TextureDesc.m_uiWidth, m_Desc.m_TextureDesc.m_uiHeight};
  return NS_SUCCESS;
}

nsResult nsGALSharedTextureSwapChain::DeInitPlatform(nsGALDevice* pDevice)
{
  for (nsUInt32 i = 0; i < m_SharedTextureHandles.GetCount(); ++i)
  {
    pDevice->DestroySharedTexture(m_SharedTextureHandles[i]);
  }
  m_uiCurrentTexture = nsMath::MaxValue<nsUInt32>();
  m_uiCurrentSemaphoreValue = 0;
  m_SharedTextureHandles.Clear();
  m_SharedTextureInterfaces.Clear();
  m_CurrentSemaphoreValue.Clear();

  return NS_SUCCESS;
}

NS_STATICLINK_FILE(RendererFoundation, RendererFoundation_Device_Implementation_SharedTextureSwapChain);

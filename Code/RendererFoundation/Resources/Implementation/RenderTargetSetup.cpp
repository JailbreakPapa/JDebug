#include <RendererFoundation/RendererFoundationPCH.h>

#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Resources/RenderTargetSetup.h>

bool nsGALRenderTargets::operator==(const nsGALRenderTargets& other) const
{
  if (m_hDSTarget != other.m_hDSTarget)
    return false;

  for (nsUInt8 uiRTIndex = 0; uiRTIndex < NS_GAL_MAX_RENDERTARGET_COUNT; ++uiRTIndex)
  {
    if (m_hRTs[uiRTIndex] != other.m_hRTs[uiRTIndex])
      return false;
  }
  return true;
}

bool nsGALRenderTargets::operator!=(const nsGALRenderTargets& other) const
{
  return !(*this == other);
}

nsGALRenderTargetSetup::nsGALRenderTargetSetup() = default;

nsGALRenderTargetSetup& nsGALRenderTargetSetup::SetRenderTarget(nsUInt8 uiIndex, nsGALRenderTargetViewHandle hRenderTarget)
{
  NS_ASSERT_DEV(uiIndex < NS_GAL_MAX_RENDERTARGET_COUNT, "Render target index out of bounds - should be less than NS_GAL_MAX_RENDERTARGET_COUNT");

  m_hRTs[uiIndex] = hRenderTarget;

  m_uiRTCount = nsMath::Max(m_uiRTCount, static_cast<nsUInt8>(uiIndex + 1u));

  return *this;
}

nsGALRenderTargetSetup& nsGALRenderTargetSetup::SetDepthStencilTarget(nsGALRenderTargetViewHandle hDSTarget)
{
  m_hDSTarget = hDSTarget;

  return *this;
}

bool nsGALRenderTargetSetup::operator==(const nsGALRenderTargetSetup& other) const
{
  if (m_hDSTarget != other.m_hDSTarget)
    return false;

  if (m_uiRTCount != other.m_uiRTCount)
    return false;

  for (nsUInt8 uiRTIndex = 0; uiRTIndex < m_uiRTCount; ++uiRTIndex)
  {
    if (m_hRTs[uiRTIndex] != other.m_hRTs[uiRTIndex])
      return false;
  }

  return true;
}

bool nsGALRenderTargetSetup::operator!=(const nsGALRenderTargetSetup& other) const
{
  return !(*this == other);
}

void nsGALRenderTargetSetup::DestroyAllAttachedViews()
{
  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  nsArrayPtr<nsGALRenderTargetViewHandle> colorViews(m_hRTs);
  for (nsGALRenderTargetViewHandle& hView : colorViews)
  {
    if (!hView.IsInvalidated())
    {
      pDevice->DestroyRenderTargetView(hView);
      hView.Invalidate();
    }
  }

  if (!m_hDSTarget.IsInvalidated())
  {
    pDevice->DestroyRenderTargetView(m_hDSTarget);
    m_hDSTarget.Invalidate();
  }
  m_uiRTCount = 0;
}

bool nsGALRenderingSetup::operator==(const nsGALRenderingSetup& other) const
{
  return m_RenderTargetSetup == other.m_RenderTargetSetup && m_ClearColor == other.m_ClearColor && m_uiRenderTargetClearMask == other.m_uiRenderTargetClearMask && m_fDepthClear == other.m_fDepthClear && m_uiStencilClear == other.m_uiStencilClear && m_bClearDepth == other.m_bClearDepth && m_bClearStencil == other.m_bClearStencil && m_bDiscardColor == other.m_bDiscardColor && m_bDiscardDepth == other.m_bDiscardDepth;
}

bool nsGALRenderingSetup::operator!=(const nsGALRenderingSetup& other) const
{
  return !(*this == other);
}

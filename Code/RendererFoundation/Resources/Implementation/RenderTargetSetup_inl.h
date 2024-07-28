
#pragma once

nsUInt8 nsGALRenderTargetSetup::GetRenderTargetCount() const
{
  return m_uiRTCount;
}

nsGALRenderTargetViewHandle nsGALRenderTargetSetup::GetRenderTarget(nsUInt8 uiIndex) const
{
  NS_ASSERT_DEBUG(uiIndex < m_uiRTCount, "Render target index out of range");

  return m_hRTs[uiIndex];
}

nsGALRenderTargetViewHandle nsGALRenderTargetSetup::GetDepthStencilTarget() const
{
  return m_hDSTarget;
}

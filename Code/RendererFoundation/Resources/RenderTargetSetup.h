
#pragma once

#include <Foundation/Basics.h>
#include <RendererFoundation/RendererFoundationDLL.h>

// \brief This class can be used to define the render targets to be used by an nsView.
struct NS_RENDERERFOUNDATION_DLL nsGALRenderTargets
{
  bool operator==(const nsGALRenderTargets& other) const;
  bool operator!=(const nsGALRenderTargets& other) const;

  nsGALTextureHandle m_hRTs[NS_GAL_MAX_RENDERTARGET_COUNT];
  nsGALTextureHandle m_hDSTarget;
};

// \brief This class can be used to construct render target setups on the stack.
class NS_RENDERERFOUNDATION_DLL nsGALRenderTargetSetup
{
public:
  nsGALRenderTargetSetup();

  nsGALRenderTargetSetup& SetRenderTarget(nsUInt8 uiIndex, nsGALRenderTargetViewHandle hRenderTarget);
  nsGALRenderTargetSetup& SetDepthStencilTarget(nsGALRenderTargetViewHandle hDSTarget);

  bool operator==(const nsGALRenderTargetSetup& other) const;
  bool operator!=(const nsGALRenderTargetSetup& other) const;

  inline nsUInt8 GetRenderTargetCount() const;

  inline nsGALRenderTargetViewHandle GetRenderTarget(nsUInt8 uiIndex) const;
  inline nsGALRenderTargetViewHandle GetDepthStencilTarget() const;

  void DestroyAllAttachedViews();

protected:
  nsGALRenderTargetViewHandle m_hRTs[NS_GAL_MAX_RENDERTARGET_COUNT];
  nsGALRenderTargetViewHandle m_hDSTarget;

  nsUInt8 m_uiRTCount = 0;
};

struct NS_RENDERERFOUNDATION_DLL nsGALRenderingSetup
{
  bool operator==(const nsGALRenderingSetup& other) const;
  bool operator!=(const nsGALRenderingSetup& other) const;

  nsGALRenderTargetSetup m_RenderTargetSetup;
  nsColor m_ClearColor = nsColor(0, 0, 0, 0);
  nsUInt32 m_uiRenderTargetClearMask = 0x0;
  float m_fDepthClear = 1.0f;
  nsUInt8 m_uiStencilClear = 0;
  bool m_bClearDepth = false;
  bool m_bClearStencil = false;
  bool m_bDiscardColor = false;
  bool m_bDiscardDepth = false;
};

#include <RendererFoundation/Resources/Implementation/RenderTargetSetup_inl.h>

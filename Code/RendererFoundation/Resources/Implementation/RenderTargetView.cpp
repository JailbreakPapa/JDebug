#include <RendererFoundation/RendererFoundationPCH.h>

#include <RendererFoundation/Resources/RenderTargetView.h>


nsGALRenderTargetView::nsGALRenderTargetView(nsGALTexture* pTexture, const nsGALRenderTargetViewCreationDescription& description)
  : nsGALObject(description)
  , m_pTexture(pTexture)
{
  NS_ASSERT_DEV(m_pTexture != nullptr, "Texture must not be null");
}

nsGALRenderTargetView::~nsGALRenderTargetView() = default;

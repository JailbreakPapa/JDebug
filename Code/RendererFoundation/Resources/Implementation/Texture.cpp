#include <RendererFoundation/RendererFoundationPCH.h>

#include <RendererFoundation/Resources/Texture.h>

nsGALTexture::nsGALTexture(const nsGALTextureCreationDescription& Description)
  : nsGALResource(Description)
{
}

nsGALTexture::~nsGALTexture()
{
  NS_ASSERT_DEV(m_hDefaultResourceView.IsInvalidated(), "");
  NS_ASSERT_DEV(m_hDefaultRenderTargetView.IsInvalidated(), "");

  NS_ASSERT_DEV(m_ResourceViews.IsEmpty(), "Dangling resource views");
  NS_ASSERT_DEV(m_RenderTargetViews.IsEmpty(), "Dangling render target views");
  NS_ASSERT_DEV(m_UnorderedAccessViews.IsEmpty(), "Dangling unordered access views");
}

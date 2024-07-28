#include <RendererFoundation/RendererFoundationPCH.h>

#include <RendererFoundation/Resources/UnorderedAccesView.h>

nsGALTextureUnorderedAccessView::nsGALTextureUnorderedAccessView(nsGALTexture* pResource, const nsGALTextureUnorderedAccessViewCreationDescription& description)
  : nsGALObject(description)
  , m_pResource(pResource)
{
  NS_ASSERT_DEV(m_pResource != nullptr, "Resource must not be null");
}

nsGALTextureUnorderedAccessView::~nsGALTextureUnorderedAccessView() = default;

nsGALBufferUnorderedAccessView::nsGALBufferUnorderedAccessView(nsGALBuffer* pResource, const nsGALBufferUnorderedAccessViewCreationDescription& description)
  : nsGALObject(description)
  , m_pResource(pResource)
{
  NS_ASSERT_DEV(m_pResource != nullptr, "Resource must not be null");
}

nsGALBufferUnorderedAccessView::~nsGALBufferUnorderedAccessView() = default;

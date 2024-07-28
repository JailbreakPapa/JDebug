#include <RendererFoundation/RendererFoundationPCH.h>

#include <RendererFoundation/Resources/ResourceView.h>


nsGALTextureResourceView::nsGALTextureResourceView(nsGALTexture* pResource, const nsGALTextureResourceViewCreationDescription& description)
  : nsGALObject(description)
  , m_pResource(pResource)
{
  NS_ASSERT_DEV(m_pResource != nullptr, "Resource must not be null");
}

nsGALTextureResourceView::~nsGALTextureResourceView() = default;

nsGALBufferResourceView::nsGALBufferResourceView(nsGALBuffer* pResource, const nsGALBufferResourceViewCreationDescription& description)
  : nsGALObject(description)
  , m_pResource(pResource)
{
  NS_ASSERT_DEV(m_pResource != nullptr, "Resource must not be null");
}

nsGALBufferResourceView::~nsGALBufferResourceView() = default;

#include <RendererFoundation/RendererFoundationPCH.h>

#include <RendererFoundation/Resources/Buffer.h>

nsGALBuffer::nsGALBuffer(const nsGALBufferCreationDescription& Description)
  : nsGALResource(Description)
{
}

nsGALBuffer::~nsGALBuffer()
{
  NS_ASSERT_DEV(m_hDefaultResourceView.IsInvalidated(), "");
  NS_ASSERT_DEV(m_ResourceViews.IsEmpty(), "Dangling resource views");
  NS_ASSERT_DEV(m_UnorderedAccessViews.IsEmpty(), "Dangling unordered access views");
}

#include <RendererFoundation/RendererFoundationPCH.h>

#include <RendererFoundation/Resources/ProxyTexture.h>

namespace
{
  nsGALTextureCreationDescription MakeProxyDesc(const nsGALTextureCreationDescription& parentDesc)
  {
    nsGALTextureCreationDescription desc = parentDesc;
    desc.m_Type = nsGALTextureType::Texture2DProxy;
    return desc;
  }
} // namespace

nsGALProxyTexture::nsGALProxyTexture(const nsGALTexture& parentTexture)
  : nsGALTexture(MakeProxyDesc(parentTexture.GetDescription()))
  , m_pParentTexture(&parentTexture)
{
}

nsGALProxyTexture::~nsGALProxyTexture() = default;


const nsGALResourceBase* nsGALProxyTexture::GetParentResource() const
{
  return m_pParentTexture;
}

nsResult nsGALProxyTexture::InitPlatform(nsGALDevice* pDevice, nsArrayPtr<nsGALSystemMemoryDescription> pInitialData)
{
  return NS_SUCCESS;
}

nsResult nsGALProxyTexture::DeInitPlatform(nsGALDevice* pDevice)
{
  return NS_SUCCESS;
}

void nsGALProxyTexture::SetDebugNamePlatform(const char* szName) const {}

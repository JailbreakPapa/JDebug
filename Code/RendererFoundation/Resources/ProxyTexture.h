
#pragma once

#include <RendererFoundation/Resources/Texture.h>

class NS_RENDERERFOUNDATION_DLL nsGALProxyTexture : public nsGALTexture
{
public:
  virtual ~nsGALProxyTexture();

  virtual const nsGALResourceBase* GetParentResource() const override;

protected:
  friend class nsGALDevice;

  nsGALProxyTexture(const nsGALTexture& parentTexture);

  virtual nsResult InitPlatform(nsGALDevice* pDevice, nsArrayPtr<nsGALSystemMemoryDescription> pInitialData) override;
  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) override;

  virtual void SetDebugNamePlatform(const char* szName) const override;

  const nsGALTexture* m_pParentTexture;
};

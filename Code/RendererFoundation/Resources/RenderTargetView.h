
#pragma once

#include <RendererFoundation/Descriptors/Descriptors.h>
#include <RendererFoundation/Resources/Resource.h>

class NS_RENDERERFOUNDATION_DLL nsGALRenderTargetView : public nsGALObject<nsGALRenderTargetViewCreationDescription>
{
public:
  NS_ALWAYS_INLINE nsGALTexture* GetTexture() const { return m_pTexture; }

protected:
  friend class nsGALDevice;

  nsGALRenderTargetView(nsGALTexture* pTexture, const nsGALRenderTargetViewCreationDescription& description);

  virtual ~nsGALRenderTargetView();

  virtual nsResult InitPlatform(nsGALDevice* pDevice) = 0;

  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) = 0;

  nsGALTexture* m_pTexture;
};


#pragma once

#include <RendererFoundation/Descriptors/Descriptors.h>
#include <RendererFoundation/Resources/Resource.h>

class nsGALTexture;
class nsGALBuffer;

class NS_RENDERERFOUNDATION_DLL nsGALTextureResourceView : public nsGALObject<nsGALTextureResourceViewCreationDescription>
{
public:
  NS_ALWAYS_INLINE nsGALTexture* GetResource() const { return m_pResource; }

protected:
  friend class nsGALDevice;

  nsGALTextureResourceView(nsGALTexture* pResource, const nsGALTextureResourceViewCreationDescription& description);

  virtual ~nsGALTextureResourceView();

  virtual nsResult InitPlatform(nsGALDevice* pDevice) = 0;

  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) = 0;

  nsGALTexture* m_pResource;
};

class NS_RENDERERFOUNDATION_DLL nsGALBufferResourceView : public nsGALObject<nsGALBufferResourceViewCreationDescription>
{
public:
  NS_ALWAYS_INLINE nsGALBuffer* GetResource() const { return m_pResource; }

protected:
  friend class nsGALDevice;

  nsGALBufferResourceView(nsGALBuffer* pResource, const nsGALBufferResourceViewCreationDescription& description);

  virtual ~nsGALBufferResourceView();

  virtual nsResult InitPlatform(nsGALDevice* pDevice) = 0;

  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) = 0;

  nsGALBuffer* m_pResource;
};

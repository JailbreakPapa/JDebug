
#pragma once

#include <RendererFoundation/Descriptors/Descriptors.h>
#include <RendererFoundation/Resources/Resource.h>

class nsGALTexture;
class nsGALBuffer;

class NS_RENDERERFOUNDATION_DLL nsGALTextureUnorderedAccessView : public nsGALObject<nsGALTextureUnorderedAccessViewCreationDescription>
{
public:
  NS_ALWAYS_INLINE nsGALResourceBase* GetResource() const { return m_pResource; }

protected:
  friend class nsGALDevice;

  nsGALTextureUnorderedAccessView(nsGALTexture* pResource, const nsGALTextureUnorderedAccessViewCreationDescription& description);

  virtual ~nsGALTextureUnorderedAccessView();
  virtual nsResult InitPlatform(nsGALDevice* pDevice) = 0;
  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) = 0;

  nsGALTexture* m_pResource;
};

class NS_RENDERERFOUNDATION_DLL nsGALBufferUnorderedAccessView : public nsGALObject<nsGALBufferUnorderedAccessViewCreationDescription>
{
public:
  NS_ALWAYS_INLINE nsGALBuffer* GetResource() const { return m_pResource; }

protected:
  friend class nsGALDevice;

  nsGALBufferUnorderedAccessView(nsGALBuffer* pResource, const nsGALBufferUnorderedAccessViewCreationDescription& description);

  virtual ~nsGALBufferUnorderedAccessView();
  virtual nsResult InitPlatform(nsGALDevice* pDevice) = 0;
  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) = 0;

  nsGALBuffer* m_pResource;
};
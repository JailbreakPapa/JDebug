
#pragma once

#include <RendererFoundation/Descriptors/Descriptors.h>
#include <RendererFoundation/Resources/Resource.h>

class NS_RENDERERFOUNDATION_DLL nsGALBuffer : public nsGALResource<nsGALBufferCreationDescription>
{
public:
  NS_ALWAYS_INLINE nsUInt32 GetSize() const;

protected:
  friend class nsGALDevice;

  nsGALBuffer(const nsGALBufferCreationDescription& Description);

  virtual ~nsGALBuffer();

  virtual nsResult InitPlatform(nsGALDevice* pDevice, nsArrayPtr<const nsUInt8> pInitialData) = 0;

  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) = 0;

protected:
  nsGALBufferResourceViewHandle m_hDefaultResourceView;
  nsHashTable<nsUInt32, nsGALBufferResourceViewHandle> m_ResourceViews;
  nsHashTable<nsUInt32, nsGALBufferUnorderedAccessViewHandle> m_UnorderedAccessViews;
};

#include <RendererFoundation/Resources/Implementation/Buffer_inl.h>

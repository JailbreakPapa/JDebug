#pragma once

#include <RendererFoundation/Descriptors/Descriptors.h>
#include <RendererFoundation/Resources/Resource.h>

class NS_RENDERERFOUNDATION_DLL nsGALQuery : public nsGALResource<nsGALQueryCreationDescription>
{
public:
protected:
  friend class nsGALDevice;
  friend class nsGALCommandEncoder;

  nsGALQuery(const nsGALQueryCreationDescription& Description);

  virtual ~nsGALQuery();

  virtual nsResult InitPlatform(nsGALDevice* pDevice) = 0;

  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) = 0;

  bool m_bStarted = false;
};

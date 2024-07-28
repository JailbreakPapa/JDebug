
#pragma once

#include <RendererFoundation/Descriptors/Descriptors.h>
#include <RendererFoundation/RendererFoundationDLL.h>

class NS_RENDERERFOUNDATION_DLL nsGALVertexDeclaration : public nsGALObject<nsGALVertexDeclarationCreationDescription>
{
public:
protected:
  friend class nsGALDevice;

  virtual nsResult InitPlatform(nsGALDevice* pDevice) = 0;

  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) = 0;

  nsGALVertexDeclaration(const nsGALVertexDeclarationCreationDescription& Description);

  virtual ~nsGALVertexDeclaration();
};

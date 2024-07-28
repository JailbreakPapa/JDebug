
#pragma once

#include <RendererFoundation/Descriptors/Descriptors.h>

class NS_RENDERERFOUNDATION_DLL nsGALBlendState : public nsGALObject<nsGALBlendStateCreationDescription>
{
public:
protected:
  nsGALBlendState(const nsGALBlendStateCreationDescription& Description);

  virtual ~nsGALBlendState();

  virtual nsResult InitPlatform(nsGALDevice* pDevice) = 0;

  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) = 0;
};

class NS_RENDERERFOUNDATION_DLL nsGALDepthStencilState : public nsGALObject<nsGALDepthStencilStateCreationDescription>
{
public:
protected:
  nsGALDepthStencilState(const nsGALDepthStencilStateCreationDescription& Description);

  virtual ~nsGALDepthStencilState();

  virtual nsResult InitPlatform(nsGALDevice* pDevice) = 0;

  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) = 0;
};

class NS_RENDERERFOUNDATION_DLL nsGALRasterizerState : public nsGALObject<nsGALRasterizerStateCreationDescription>
{
public:
protected:
  nsGALRasterizerState(const nsGALRasterizerStateCreationDescription& Description);

  virtual ~nsGALRasterizerState();

  virtual nsResult InitPlatform(nsGALDevice* pDevice) = 0;

  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) = 0;
};

class NS_RENDERERFOUNDATION_DLL nsGALSamplerState : public nsGALObject<nsGALSamplerStateCreationDescription>
{
public:
protected:
  nsGALSamplerState(const nsGALSamplerStateCreationDescription& Description);

  virtual ~nsGALSamplerState();

  virtual nsResult InitPlatform(nsGALDevice* pDevice) = 0;

  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) = 0;
};

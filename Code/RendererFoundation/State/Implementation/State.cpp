#include <RendererFoundation/RendererFoundationPCH.h>

#include <RendererFoundation/RendererFoundationDLL.h>
#include <RendererFoundation/State/State.h>

nsGALBlendState::nsGALBlendState(const nsGALBlendStateCreationDescription& Description)
  : nsGALObject(Description)
{
}

nsGALBlendState::~nsGALBlendState() = default;



nsGALDepthStencilState::nsGALDepthStencilState(const nsGALDepthStencilStateCreationDescription& Description)
  : nsGALObject(Description)
{
}

nsGALDepthStencilState::~nsGALDepthStencilState() = default;



nsGALRasterizerState::nsGALRasterizerState(const nsGALRasterizerStateCreationDescription& Description)
  : nsGALObject(Description)
{
}

nsGALRasterizerState::~nsGALRasterizerState() = default;


nsGALSamplerState::nsGALSamplerState(const nsGALSamplerStateCreationDescription& Description)
  : nsGALObject(Description)
{
}

nsGALSamplerState::~nsGALSamplerState() = default;

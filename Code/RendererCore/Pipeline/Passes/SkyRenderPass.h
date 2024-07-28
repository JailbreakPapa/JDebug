#pragma once

#include <RendererCore/Pipeline/Passes/ForwardRenderPass.h>

/// \brief A forward render pass that renders all sky objects into the color target.
class NS_RENDERERCORE_DLL nsSkyRenderPass : public nsForwardRenderPass
{
  NS_ADD_DYNAMIC_REFLECTION(nsSkyRenderPass, nsForwardRenderPass);

public:
  nsSkyRenderPass(const char* szName = "SkyRenderPass");
  ~nsSkyRenderPass();

protected:
  virtual void RenderObjects(const nsRenderViewContext& renderViewContext) override;
};

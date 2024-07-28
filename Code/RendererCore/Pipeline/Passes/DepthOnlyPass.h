#pragma once

#include <RendererCore/Pipeline/RenderPipelinePass.h>

/// \brief A render pass that renders into a depth target only.
class NS_RENDERERCORE_DLL nsDepthOnlyPass : public nsRenderPipelinePass
{
  NS_ADD_DYNAMIC_REFLECTION(nsDepthOnlyPass, nsRenderPipelinePass);

public:
  nsDepthOnlyPass(const char* szName = "DepthOnlyPass");
  ~nsDepthOnlyPass();

  virtual bool GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs) override;
  virtual void Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;

protected:
  nsRenderPipelineNodePassThrougPin m_PinDepthStencil;
};

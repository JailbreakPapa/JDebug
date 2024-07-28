#pragma once

#include <RendererCore/Pipeline/Passes/ForwardRenderPass.h>

/// \brief A forward render pass that renders all transparent objects into the color target.
class NS_RENDERERCORE_DLL nsTransparentForwardRenderPass : public nsForwardRenderPass
{
  NS_ADD_DYNAMIC_REFLECTION(nsTransparentForwardRenderPass, nsForwardRenderPass);

public:
  nsTransparentForwardRenderPass(const char* szName = "TransparentForwardRenderPass");
  ~nsTransparentForwardRenderPass();

  virtual void Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;

protected:
  virtual void SetupResources(nsGALPass* pGALPass, const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;
  virtual void RenderObjects(const nsRenderViewContext& renderViewContext) override;

  void UpdateSceneColorTexture(const nsRenderViewContext& renderViewContext, nsGALTextureHandle hSceneColorTexture, nsGALTextureHandle hCurrentColorTexture);
  void CreateSamplerState();

  nsRenderPipelineNodeInputPin m_PinResolvedDepth;

  nsGALSamplerStateHandle m_hSceneColorSamplerState;
};

#pragma once

#include <RendererCore/Pipeline/Passes/ForwardRenderPass.h>

/// \brief A forward render pass that renders all opaque objects into the color target.
class NS_RENDERERCORE_DLL nsOpaqueForwardRenderPass : public nsForwardRenderPass
{
  NS_ADD_DYNAMIC_REFLECTION(nsOpaqueForwardRenderPass, nsForwardRenderPass);

public:
  nsOpaqueForwardRenderPass(const char* szName = "OpaqueForwardRenderPass");
  ~nsOpaqueForwardRenderPass();

  virtual bool GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs) override;

protected:
  virtual void SetupResources(nsGALPass* pGALPass, const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;
  virtual void SetupPermutationVars(const nsRenderViewContext& renderViewContext) override;

  virtual void RenderObjects(const nsRenderViewContext& renderViewContext) override;

  nsRenderPipelineNodeInputPin m_PinSSAO;
  // nsRenderPipelineNodeOutputPin m_PinNormal;
  // nsRenderPipelineNodeOutputPin m_PinSpecularColorRoughness;

  bool m_bWriteDepth = true;

  nsTexture2DResourceHandle m_hWhiteTexture;
};

#pragma once

#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ShaderResource.h>

class NS_RENDERERCORE_DLL nsMsaaResolvePass : public nsRenderPipelinePass
{
  NS_ADD_DYNAMIC_REFLECTION(nsMsaaResolvePass, nsRenderPipelinePass);

public:
  nsMsaaResolvePass();
  ~nsMsaaResolvePass();

  virtual bool GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs) override;

  virtual void Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;

protected:
  nsRenderPipelineNodeInputPin m_PinInput;
  nsRenderPipelineNodeOutputPin m_PinOutput;

  bool m_bIsDepth = false;
  nsGALMSAASampleCount::Enum m_MsaaSampleCount = nsGALMSAASampleCount::None;
  nsShaderResourceHandle m_hDepthResolveShader;
};

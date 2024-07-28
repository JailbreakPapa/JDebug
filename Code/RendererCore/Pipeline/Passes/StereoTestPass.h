#pragma once

#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ShaderResource.h>

class NS_RENDERERCORE_DLL nsStereoTestPass : public nsRenderPipelinePass
{
  NS_ADD_DYNAMIC_REFLECTION(nsStereoTestPass, nsRenderPipelinePass);

public:
  nsStereoTestPass();
  ~nsStereoTestPass();

  virtual bool GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs) override;

  virtual void Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;

protected:
  nsRenderPipelineNodeInputPin m_PinInput;
  nsRenderPipelineNodeOutputPin m_PinOutput;

  nsShaderResourceHandle m_hShader;
};

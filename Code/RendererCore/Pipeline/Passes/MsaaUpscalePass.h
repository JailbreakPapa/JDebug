#pragma once

#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ShaderResource.h>

class NS_RENDERERCORE_DLL nsMsaaUpscalePass : public nsRenderPipelinePass
{
  NS_ADD_DYNAMIC_REFLECTION(nsMsaaUpscalePass, nsRenderPipelinePass);

public:
  nsMsaaUpscalePass();
  ~nsMsaaUpscalePass();

  virtual bool GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs) override;

  virtual void Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;
  virtual nsResult Serialize(nsStreamWriter& inout_stream) const override;
  virtual nsResult Deserialize(nsStreamReader& inout_stream) override;

protected:
  nsRenderPipelineNodeInputPin m_PinInput;
  nsRenderPipelineNodeOutputPin m_PinOutput;

  nsEnum<nsGALMSAASampleCount> m_MsaaMode = nsGALMSAASampleCount::None;
  nsShaderResourceHandle m_hShader;
};
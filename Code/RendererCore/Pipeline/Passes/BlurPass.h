#pragma once

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>

/// \brief Blurs input and writes it to an output buffer of the same format.
class NS_RENDERERCORE_DLL nsBlurPass : public nsRenderPipelinePass
{
  NS_ADD_DYNAMIC_REFLECTION(nsBlurPass, nsRenderPipelinePass);

public:
  nsBlurPass();
  ~nsBlurPass();

  virtual bool GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs) override;

  virtual void Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;
  virtual nsResult Serialize(nsStreamWriter& inout_stream) const override;
  virtual nsResult Deserialize(nsStreamReader& inout_stream) override;

  void SetRadius(nsInt32 iRadius);
  nsInt32 GetRadius() const;

protected:
  nsRenderPipelineNodeInputPin m_PinInput;
  nsRenderPipelineNodeOutputPin m_PinOutput;

  nsInt32 m_iRadius = 15;
  nsConstantBufferStorageHandle m_hBlurCB;
  nsShaderResourceHandle m_hShader;
};

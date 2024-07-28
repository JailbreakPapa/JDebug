#pragma once

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>

class NS_RENDERERCORE_DLL nsBloomPass : public nsRenderPipelinePass
{
  NS_ADD_DYNAMIC_REFLECTION(nsBloomPass, nsRenderPipelinePass);

public:
  nsBloomPass();
  ~nsBloomPass();

  virtual bool GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs) override;

  virtual void Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;
  virtual void ExecuteInactive(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;
  virtual nsResult Serialize(nsStreamWriter& inout_stream) const override;
  virtual nsResult Deserialize(nsStreamReader& inout_stream) override;

protected:
  void UpdateConstantBuffer(nsVec2 pixelSize, const nsColor& tintColor);

  nsRenderPipelineNodeInputPin m_PinInput;
  nsRenderPipelineNodeOutputPin m_PinOutput;

  float m_fRadius = 0.2f;
  float m_fThreshold = 1.0f;
  float m_fIntensity = 0.3f;
  nsColorGammaUB m_InnerTintColor = nsColor::White;
  nsColorGammaUB m_MidTintColor = nsColor::White;
  nsColorGammaUB m_OuterTintColor = nsColor::White;
  nsConstantBufferStorageHandle m_hConstantBuffer;
  nsShaderResourceHandle m_hShader;
};

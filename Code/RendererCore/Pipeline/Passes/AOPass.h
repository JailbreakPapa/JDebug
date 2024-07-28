#pragma once

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>

class NS_RENDERERCORE_DLL nsAOPass : public nsRenderPipelinePass
{
  NS_ADD_DYNAMIC_REFLECTION(nsAOPass, nsRenderPipelinePass);

public:
  nsAOPass();
  ~nsAOPass();

  virtual bool GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs) override;

  virtual void Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;
  virtual void ExecuteInactive(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;
  virtual nsResult Serialize(nsStreamWriter& inout_stream) const override;
  virtual nsResult Deserialize(nsStreamReader& inout_stream) override;

  void SetFadeOutStart(float fStart);
  float GetFadeOutStart() const;

  void SetFadeOutEnd(float fEnd);
  float GetFadeOutEnd() const;

protected:
  void CreateSamplerState();

  nsRenderPipelineNodeInputPin m_PinDepthInput;
  nsRenderPipelineNodeOutputPin m_PinOutput;

  float m_fRadius = 1.0f;
  float m_fMaxScreenSpaceRadius = 1.0f;
  float m_fContrast = 2.0f;
  float m_fIntensity = 0.7f;

  float m_fFadeOutStart = 80.0f;
  float m_fFadeOutEnd = 100.0f;

  float m_fPositionBias = 5.0f;
  float m_fMipLevelScale = 10.0f;
  float m_fDepthBlurThreshold = 2.0f;

  nsConstantBufferStorageHandle m_hDownscaleConstantBuffer;
  nsConstantBufferStorageHandle m_hSSAOConstantBuffer;

  nsTexture2DResourceHandle m_hNoiseTexture;

  nsGALSamplerStateHandle m_hSSAOSamplerState;

  nsShaderResourceHandle m_hDownscaleShader;
  nsShaderResourceHandle m_hSSAOShader;
  nsShaderResourceHandle m_hBlurShader;
};

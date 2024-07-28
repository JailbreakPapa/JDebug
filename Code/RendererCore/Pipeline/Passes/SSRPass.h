#pragma once

#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>

class NS_RENDERERCORE_DLL nsSSRPass : public nsRenderPipelinePass
{
  NS_ADD_DYNAMIC_REFLECTION(nsSSRPass, nsRenderPipelinePass);

public:
  nsSSRPass(const char* szName = "SSRPass");
  ~nsSSRPass();

  virtual bool GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs) override;
  virtual void Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;
  virtual void ExecuteInactive(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs);
  virtual nsResult Serialize(nsStreamWriter& inout_stream) const override;
  virtual nsResult Deserialize(nsStreamReader& inout_stream) override;

protected:
  void CreateSamplerState();

  nsRenderPipelineNodeInputPin m_PinColor;
  /// Depth Input
  nsRenderPipelineNodeInputPin m_PinDepthStencil;
  nsRenderPipelineNodeOutputPin m_PinOutput;
  /// Depth Sampler
  nsGALSamplerStateHandle m_hSSRColorSamplerState;
  /// Color Sampler
  nsGALSamplerStateHandle m_hSSRSamplerState;
  nsShaderResourceHandle m_hSSRShader;
  nsConstantBufferStorageHandle m_hSSRConstantBuffer;

  // SSR settings
  float maxDistance = 8.0f;
  float resolution = 0.3f;
  float maxScreenSpaceRadius = 15.0f;
  float m_fpositionBias = 0.1f;
  float worldRadius = 10.0f;
  int steps = 5;
  float thickness = 0.5f;
};

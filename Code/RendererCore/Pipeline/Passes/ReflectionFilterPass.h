#pragma once

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>

class NS_RENDERERCORE_DLL nsReflectionFilterPass : public nsRenderPipelinePass
{
  NS_ADD_DYNAMIC_REFLECTION(nsReflectionFilterPass, nsRenderPipelinePass);

public:
  nsReflectionFilterPass();
  ~nsReflectionFilterPass();

  virtual bool GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs) override;

  virtual void Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;
  virtual nsResult Serialize(nsStreamWriter& inout_stream) const override;
  virtual nsResult Deserialize(nsStreamReader& inout_stream) override;

  nsUInt32 GetInputCubemap() const;
  void SetInputCubemap(nsUInt32 uiCubemapHandle);

protected:
  void UpdateFilteredSpecularConstantBuffer(nsUInt32 uiMipMapIndex, nsUInt32 uiNumMipMaps);
  void UpdateIrradianceConstantBuffer();

  nsRenderPipelineNodeOutputPin m_PinFilteredSpecular;
  nsRenderPipelineNodeOutputPin m_PinAvgLuminance;
  nsRenderPipelineNodeOutputPin m_PinIrradianceData;

  float m_fIntensity = 1.0f;
  float m_fSaturation = 1.0f;
  nsUInt32 m_uiSpecularOutputIndex = 0;
  nsUInt32 m_uiIrradianceOutputIndex = 0;

  nsGALTextureHandle m_hInputCubemap;

  nsConstantBufferStorageHandle m_hFilteredSpecularConstantBuffer;
  nsShaderResourceHandle m_hFilteredSpecularShader;

  nsConstantBufferStorageHandle m_hIrradianceConstantBuffer;
  nsShaderResourceHandle m_hIrradianceShader;
};

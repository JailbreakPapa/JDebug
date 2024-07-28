#pragma once

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>
#include <RendererCore/Textures/Texture3DResource.h>

class NS_RENDERERCORE_DLL nsTonemapPass : public nsRenderPipelinePass
{
  NS_ADD_DYNAMIC_REFLECTION(nsTonemapPass, nsRenderPipelinePass);

public:
  nsTonemapPass();
  ~nsTonemapPass();

  virtual bool GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs) override;

  virtual void Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;
  virtual nsResult Serialize(nsStreamWriter& inout_stream) const override;
  virtual nsResult Deserialize(nsStreamReader& inout_stream) override;

protected:
  nsRenderPipelineNodeInputPin m_PinColorInput;
  nsRenderPipelineNodeInputPin m_PinBloomInput;
  nsRenderPipelineNodeOutputPin m_PinOutput;

  void SetVignettingTextureFile(const char* szFile);
  const char* GetVignettingTextureFile() const;

  void SetLUT1TextureFile(const char* szFile);
  const char* GetLUT1TextureFile() const;

  void SetLUT2TextureFile(const char* szFile);
  const char* GetLUT2TextureFile() const;

  nsTexture2DResourceHandle m_hVignettingTexture;
  nsTexture2DResourceHandle m_hNoiseTexture;
  nsTexture3DResourceHandle m_hLUT1;
  nsTexture3DResourceHandle m_hLUT2;

  nsColor m_MoodColor;
  float m_fMoodStrength;
  float m_fSaturation;
  float m_fContrast;
  float m_fLut1Strength;
  float m_fLut2Strength;

  nsConstantBufferStorageHandle m_hConstantBuffer;
  nsShaderResourceHandle m_hShader;
};

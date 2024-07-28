#pragma once

#include <Foundation/Reflection/Reflection.h>

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>

class NS_RENDERERCORE_DLL nsFilmGrainPass : public nsRenderPipelinePass
{
  NS_ADD_DYNAMIC_REFLECTION(nsFilmGrainPass, nsRenderPipelinePass);

public:
  nsFilmGrainPass();
  ~nsFilmGrainPass() override;

  bool GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs) override;

  void Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;
  void ExecuteInactive(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;

protected:
  void UpdateConstantBuffer() const;

  nsRenderPipelineNodeInputPin m_PinInput;
  nsRenderPipelineNodeOutputPin m_PinOutput;

  float m_fIntensity;
  float m_fSpeed;
  float m_fMean;
  float m_fVariance;

  nsShaderResourceHandle m_hShader;
  nsConstantBufferStorageHandle m_hConstantBuffer;
};

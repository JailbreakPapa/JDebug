#pragma once

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>

class NS_RENDERERCORE_DLL nsDepthOfFieldPass : public nsRenderPipelinePass
{
  NS_ADD_DYNAMIC_REFLECTION(nsDepthOfFieldPass, nsRenderPipelinePass);

public:
  nsDepthOfFieldPass();
  ~nsDepthOfFieldPass() override;

  bool GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs) override;
  void Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;
  void ExecuteInactive(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;

protected:
  void UpdateConstantBuffer();

  nsRenderPipelineNodeInputPin m_PinInput;
  nsRenderPipelineNodeInputPin m_PinDepth;
  nsRenderPipelineNodeOutputPin m_PinOutput;

  nsShaderResourceHandle m_hShader;
  nsConstantBufferStorageHandle m_hConstantBuffer;

  float m_fRadius;
};

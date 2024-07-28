#pragma once

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>

struct nsSharpeningConstants;

class NS_RENDERERCORE_DLL nsSharpeningPass : public nsRenderPipelinePass
{
  NS_ADD_DYNAMIC_REFLECTION(nsSharpeningPass, nsRenderPipelinePass);

public:
  nsSharpeningPass();
  ~nsSharpeningPass() override;

  bool GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs) override;
  void Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;
  void ExecuteInactive(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;

protected:
  void UpdateConstantBuffer() const;

  nsRenderPipelineNodeInputPin m_PinInput;
  nsRenderPipelineNodeOutputPin m_PinOutput;

  nsShaderResourceHandle m_hShader;
  nsConstantBufferStorageHandle m_hConstantBuffer;

  float m_fStrength;
};

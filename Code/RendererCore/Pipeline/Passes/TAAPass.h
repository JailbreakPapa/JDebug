#pragma once

#include <Foundation/Reflection/Reflection.h>

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>

class NS_RENDERERCORE_DLL nsTAAPass : public nsRenderPipelinePass
{
  NS_ADD_DYNAMIC_REFLECTION(nsTAAPass, nsRenderPipelinePass);

public:
  nsTAAPass();
  ~nsTAAPass() override;

  bool GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs) override;

  void Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;
  void ExecuteInactive(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;

protected:
  void UpdateTAAConstantBuffer() const;
  void UpdateCopyConstantBuffer(nsVec2I32 offset, nsVec2U32 size) const;

  nsRenderPipelineNodeInputPin m_PinInputColor;
  nsRenderPipelineNodeInputPin m_PinInputVelocity;
  nsRenderPipelineNodeInputPin m_PinInputHistory;
  nsRenderPipelineNodeInputPin m_PinInputDepth;
  nsRenderPipelineNodeOutputPin m_PinOutput;

  bool m_bUpsample;

  nsShaderResourceHandle m_hTAAShader;
  nsConstantBufferStorageHandle m_hTAAConstantBuffer;

  nsShaderResourceHandle m_hCopyShader;
  nsConstantBufferStorageHandle m_hCopyConstantBuffer;

  nsGALTextureHandle m_hPreviousVelocity;
  nsGALTextureHandle m_hHistory;
};

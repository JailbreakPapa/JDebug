#pragma once

#include <RendererCore/Pipeline/RenderPipelinePass.h>

struct nsGALRenderTargets;

class NS_RENDERERCORE_DLL nsTargetPass : public nsRenderPipelinePass
{
  NS_ADD_DYNAMIC_REFLECTION(nsTargetPass, nsRenderPipelinePass);

public:
  nsTargetPass(const char* szName = "TargetPass");
  ~nsTargetPass();

  const nsGALTextureHandle* GetTextureHandle(const nsGALRenderTargets& renderTargets, const nsRenderPipelineNodePin* pPin);

  virtual bool GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs) override;
  virtual void Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;

private:
  bool VerifyInput(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, const char* szPinName);

protected:
  nsRenderPipelineNodeInputPin m_PinColor0;
  nsRenderPipelineNodeInputPin m_PinColor1;
  nsRenderPipelineNodeInputPin m_PinColor2;
  nsRenderPipelineNodeInputPin m_PinColor3;
  nsRenderPipelineNodeInputPin m_PinColor4;
  nsRenderPipelineNodeInputPin m_PinColor5;
  nsRenderPipelineNodeInputPin m_PinColor6;
  nsRenderPipelineNodeInputPin m_PinColor7;
  nsRenderPipelineNodeInputPin m_PinDepthStencil;
};

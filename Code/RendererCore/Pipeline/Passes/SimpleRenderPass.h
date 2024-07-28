#pragma once

#include <RendererCore/Pipeline/RenderPipelinePass.h>

/// \brief A very basic render pass that renders into the color target.
///
/// Can either works as passthrough or if no input is present creates
/// output targets matching the view's render target.
/// Needs to be connected to a nsTargetPass to function.
class NS_RENDERERCORE_DLL nsSimpleRenderPass : public nsRenderPipelinePass
{
  NS_ADD_DYNAMIC_REFLECTION(nsSimpleRenderPass, nsRenderPipelinePass);

public:
  nsSimpleRenderPass(const char* szName = "SimpleRenderPass");
  ~nsSimpleRenderPass();

  virtual bool GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs) override;
  virtual void Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;
  virtual nsResult Serialize(nsStreamWriter& inout_stream) const override;
  virtual nsResult Deserialize(nsStreamReader& inout_stream) override;

  void SetMessage(const char* szMessage);

protected:
  nsRenderPipelineNodePassThrougPin m_PinColor;
  nsRenderPipelineNodePassThrougPin m_PinDepthStencil;

  nsString m_sMessage;
};

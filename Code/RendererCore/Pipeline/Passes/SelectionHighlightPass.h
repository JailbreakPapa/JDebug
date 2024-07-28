#pragma once

#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>

class NS_RENDERERCORE_DLL nsSelectionHighlightPass : public nsRenderPipelinePass
{
  NS_ADD_DYNAMIC_REFLECTION(nsSelectionHighlightPass, nsRenderPipelinePass);

public:
  nsSelectionHighlightPass(const char* szName = "SelectionHighlightPass");
  ~nsSelectionHighlightPass();

  virtual bool GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs) override;
  virtual void Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;
  virtual nsResult Serialize(nsStreamWriter& inout_stream) const override;
  virtual nsResult Deserialize(nsStreamReader& inout_stream) override;

protected:
  nsRenderPipelineNodePassThrougPin m_PinColor;
  nsRenderPipelineNodeInputPin m_PinDepthStencil;

  nsShaderResourceHandle m_hShader;
  nsConstantBufferStorageHandle m_hConstantBuffer;

  nsColor m_HighlightColor = nsColorScheme::LightUI(nsColorScheme::Yellow);
  float m_fOverlayOpacity = 0.1f;
};

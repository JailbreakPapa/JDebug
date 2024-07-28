#pragma once

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>

struct nsForwardRenderShadingQuality
{
  using StorageType = nsInt8;

  enum Enum
  {
    Normal,
    Simplified,

    Default = Normal,
  };
};

NS_DECLARE_REFLECTABLE_TYPE(NS_NO_LINKAGE, nsForwardRenderShadingQuality);

/// \brief A standard forward render pass that renders into the color target.
class NS_RENDERERCORE_DLL nsForwardRenderPass : public nsRenderPipelinePass
{
  NS_ADD_DYNAMIC_REFLECTION(nsForwardRenderPass, nsRenderPipelinePass);

public:
  nsForwardRenderPass(const char* szName = "ForwardRenderPass");
  ~nsForwardRenderPass();

  virtual bool GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs) override;
  virtual void Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;
  virtual nsResult Serialize(nsStreamWriter& inout_stream) const override;
  virtual nsResult Deserialize(nsStreamReader& inout_stream) override;

protected:
  virtual void SetupResources(nsGALPass* pGALPass, const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs);
  virtual void SetupPermutationVars(const nsRenderViewContext& renderViewContext);
  virtual void SetupLighting(const nsRenderViewContext& renderViewContext);

  virtual void RenderObjects(const nsRenderViewContext& renderViewContext) = 0;

  nsRenderPipelineNodePassThrougPin m_PinColor;
  nsRenderPipelineNodePassThrougPin m_PinDepthStencil;

  nsEnum<nsForwardRenderShadingQuality> m_ShadingQuality;
};

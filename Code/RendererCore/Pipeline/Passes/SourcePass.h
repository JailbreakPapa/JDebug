#pragma once

#include <RendererCore/Pipeline/RenderPipelinePass.h>

struct nsSourceFormat
{
  using StorageType = nsUInt8;

  enum Enum
  {
    Color4Channel8BitNormalized_sRGB,
    Color4Channel8BitNormalized,
    Color4Channel16BitFloat,
    Color4Channel32BitFloat,
    Color3Channel11_11_10BitFloat,
    Depth16Bit,
    Depth24BitStencil8Bit,
    Depth32BitFloat,

    Default = Color4Channel8BitNormalized_sRGB
  };
};
NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsSourceFormat);


class NS_RENDERERCORE_DLL nsSourcePass : public nsRenderPipelinePass
{
  NS_ADD_DYNAMIC_REFLECTION(nsSourcePass, nsRenderPipelinePass);

public:
  nsSourcePass(const char* szName = "SourcePass");
  ~nsSourcePass();

  virtual bool GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs) override;
  virtual void Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;
  virtual nsResult Serialize(nsStreamWriter& inout_stream) const override;
  virtual nsResult Deserialize(nsStreamReader& inout_stream) override;

protected:
  nsRenderPipelineNodeOutputPin m_PinOutput;

  nsEnum<nsSourceFormat> m_Format;
  nsEnum<nsGALMSAASampleCount> m_MsaaMode;
  nsColor m_ClearColor;
  bool m_bClear;
};

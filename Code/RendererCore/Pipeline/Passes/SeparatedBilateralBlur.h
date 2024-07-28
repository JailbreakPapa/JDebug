#pragma once

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>

/// \brief Depth aware blur on input and writes it to an output buffer of the same format.
///
/// In theory it is mathematical nonsense to separate a bilateral blur, but it is common praxis and works good enough.
/// (Thus the name "separated" in contrast to "separable")
class NS_RENDERERCORE_DLL nsSeparatedBilateralBlurPass : public nsRenderPipelinePass
{
  NS_ADD_DYNAMIC_REFLECTION(nsSeparatedBilateralBlurPass, nsRenderPipelinePass);

public:
  nsSeparatedBilateralBlurPass();
  ~nsSeparatedBilateralBlurPass();

  virtual bool GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs) override;

  virtual void Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;
  virtual nsResult Serialize(nsStreamWriter& inout_stream) const override;
  virtual nsResult Deserialize(nsStreamReader& inout_stream) override;

  void SetRadius(nsUInt32 uiRadius);
  nsUInt32 GetRadius() const;

  void SetGaussianSigma(float fSigma);
  float GetGaussianSigma() const;

  void SetSharpness(float fSharpness);
  float GetSharpness() const;

protected:
  nsRenderPipelineNodeInputPin m_PinBlurSourceInput;
  nsRenderPipelineNodeInputPin m_PinDepthInput;
  nsRenderPipelineNodeOutputPin m_PinOutput;

  nsUInt32 m_uiRadius = 7;
  float m_fGaussianSigma = 3.5f;
  float m_fSharpness = 120.0f;
  nsConstantBufferStorageHandle m_hBilateralBlurCB;
  nsShaderResourceHandle m_hShader;
};

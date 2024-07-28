#pragma once

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderResource.h>

struct nsMotionBlurMode
{
  using StorageType = nsUInt8;

  enum Enum
  {
    /// @brief Applies motion blur on objects when they are moving and on camera movements.
    /// This won't affect the skybox since it's always static in the scene.
    /// @note This mode requires the Velocity texture as input.
    ObjectBased,

    /// @brief Applies motion blur on the whole screen only on camera movements.
    /// This will affect the skybox too.
    /// @note This mode requires the Depth texture as input.
    ScreenBased,

    Default = ObjectBased,
  };
};

NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsMotionBlurMode);

class NS_RENDERERCORE_DLL nsMotionBlurPass : public nsRenderPipelinePass
{
  NS_ADD_DYNAMIC_REFLECTION(nsMotionBlurPass, nsRenderPipelinePass);

public:
  nsMotionBlurPass();
  ~nsMotionBlurPass() override;

  bool GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs) override;
  void Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;
  void ExecuteInactive(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) override;

protected:
  nsRenderPipelineNodeInputPin m_PinInputColor;
  nsRenderPipelineNodeInputPin m_PinInputVelocity;
  nsRenderPipelineNodeInputPin m_PinInputDepth;
  nsRenderPipelineNodeOutputPin m_PinOutput;

  nsShaderResourceHandle m_hShader;
  nsConstantBufferStorageHandle m_hConstantBuffer;

  float m_fStrength;
  nsEnum<nsMotionBlurMode> m_eMode;
};

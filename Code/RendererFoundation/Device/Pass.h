
#pragma once

#include <RendererFoundation/Resources/RenderTargetSetup.h>

class NS_RENDERERFOUNDATION_DLL nsGALPass
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsGALPass);

public:
  nsGALRenderCommandEncoder* BeginRendering(const nsGALRenderingSetup& renderingSetup, const char* szName = "");
  void EndRendering(nsGALRenderCommandEncoder* pCommandEncoder);

  nsGALComputeCommandEncoder* BeginCompute(const char* szName = "");
  void EndCompute(nsGALComputeCommandEncoder* pCommandEncoder);

  // BeginRaytracing() could be here as well (would match Vulkan)

protected:
  virtual nsGALRenderCommandEncoder* BeginRenderingPlatform(const nsGALRenderingSetup& renderingSetup, const char* szName) = 0;
  virtual void EndRenderingPlatform(nsGALRenderCommandEncoder* pCommandEncoder) = 0;

  virtual nsGALComputeCommandEncoder* BeginComputePlatform(const char* szName) = 0;
  virtual void EndComputePlatform(nsGALComputeCommandEncoder* pCommandEncoder) = 0;

  nsGALPass(nsGALDevice& device);
  virtual ~nsGALPass();

  nsGALDevice& m_Device;

  enum class CommandEncoderType
  {
    Invalid,
    Render,
    Compute
  };

  CommandEncoderType m_CurrentCommandEncoderType = CommandEncoderType::Invalid;
  bool m_bMarker = false;
};

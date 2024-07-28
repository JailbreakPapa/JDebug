
#pragma once

#include <Foundation/Types/UniquePtr.h>
#include <RendererFoundation/Device/Pass.h>

struct nsGALCommandEncoderRenderState;
class nsGALRenderCommandEncoder;
class nsGALComputeCommandEncoder;
class nsGALCommandEncoderImplVulkan;

class nsGALPassVulkan : public nsGALPass
{
protected:
  friend class nsGALDeviceVulkan;
  friend class nsMemoryUtils;
  void Reset();
  void MarkDirty();
  void SetCurrentCommandBuffer(vk::CommandBuffer* commandBuffer, nsPipelineBarrierVulkan* pipelineBarrier);

  virtual nsGALRenderCommandEncoder* BeginRenderingPlatform(const nsGALRenderingSetup& renderingSetup, const char* szName) override;
  virtual void EndRenderingPlatform(nsGALRenderCommandEncoder* pCommandEncoder) override;

  virtual nsGALComputeCommandEncoder* BeginComputePlatform(const char* szName) override;
  virtual void EndComputePlatform(nsGALComputeCommandEncoder* pCommandEncoder) override;

  nsGALPassVulkan(nsGALDevice& device);
  virtual ~nsGALPassVulkan();

  void BeginPass(const char* szName);
  void EndPass();

private:
  nsUniquePtr<nsGALCommandEncoderRenderState> m_pCommandEncoderState;
  nsUniquePtr<nsGALCommandEncoderImplVulkan> m_pCommandEncoderImpl;

  nsUniquePtr<nsGALRenderCommandEncoder> m_pRenderCommandEncoder;
  nsUniquePtr<nsGALComputeCommandEncoder> m_pComputeCommandEncoder;
};


#pragma once

#include <RendererFoundation/CommandEncoder/CommandEncoder.h>

class NS_RENDERERFOUNDATION_DLL nsGALComputeCommandEncoder : public nsGALCommandEncoder
{
public:
  nsGALComputeCommandEncoder(nsGALDevice& ref_device, nsGALCommandEncoderState& ref_state, nsGALCommandEncoderCommonPlatformInterface& ref_commonImpl, nsGALCommandEncoderComputePlatformInterface& ref_computeImpl);
  virtual ~nsGALComputeCommandEncoder();

  // Dispatch

  nsResult Dispatch(nsUInt32 uiThreadGroupCountX, nsUInt32 uiThreadGroupCountY, nsUInt32 uiThreadGroupCountZ);
  nsResult DispatchIndirect(nsGALBufferHandle hIndirectArgumentBuffer, nsUInt32 uiArgumentOffsetInBytes);

  virtual void ClearStatisticsCounters() override;

private:
  void CountDispatchCall() { m_uiDispatchCalls++; }

  // Statistic variables
  nsUInt32 m_uiDispatchCalls = 0;

  nsGALCommandEncoderComputePlatformInterface& m_ComputeImpl;
};

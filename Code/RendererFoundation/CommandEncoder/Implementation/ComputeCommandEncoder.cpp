#include <RendererFoundation/RendererFoundationPCH.h>

#include <RendererFoundation/CommandEncoder/ComputeCommandEncoder.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Resources/Buffer.h>

nsGALComputeCommandEncoder::nsGALComputeCommandEncoder(nsGALDevice& ref_device, nsGALCommandEncoderState& ref_state, nsGALCommandEncoderCommonPlatformInterface& ref_commonImpl, nsGALCommandEncoderComputePlatformInterface& ref_computeImpl)
  : nsGALCommandEncoder(ref_device, ref_state, ref_commonImpl)
  , m_ComputeImpl(ref_computeImpl)
{
}

nsGALComputeCommandEncoder::~nsGALComputeCommandEncoder() = default;

nsResult nsGALComputeCommandEncoder::Dispatch(nsUInt32 uiThreadGroupCountX, nsUInt32 uiThreadGroupCountY, nsUInt32 uiThreadGroupCountZ)
{
  AssertRenderingThread();

  NS_ASSERT_DEBUG(uiThreadGroupCountX > 0 && uiThreadGroupCountY > 0 && uiThreadGroupCountZ > 0, "Thread group counts of zero are not meaningful. Did you mean 1?");

  CountDispatchCall();
  return m_ComputeImpl.DispatchPlatform(uiThreadGroupCountX, uiThreadGroupCountY, uiThreadGroupCountZ);
}

nsResult nsGALComputeCommandEncoder::DispatchIndirect(nsGALBufferHandle hIndirectArgumentBuffer, nsUInt32 uiArgumentOffsetInBytes)
{
  AssertRenderingThread();

  const nsGALBuffer* pBuffer = GetDevice().GetBuffer(hIndirectArgumentBuffer);
  NS_ASSERT_DEV(pBuffer != nullptr, "Invalid buffer handle for indirect arguments!");

  CountDispatchCall();
  return m_ComputeImpl.DispatchIndirectPlatform(pBuffer, uiArgumentOffsetInBytes);
}

void nsGALComputeCommandEncoder::ClearStatisticsCounters()
{
  nsGALCommandEncoder::ClearStatisticsCounters();

  m_uiDispatchCalls = 0;
}

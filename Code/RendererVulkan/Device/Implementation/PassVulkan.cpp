#include <RendererVulkan/RendererVulkanPCH.h>

#include <RendererFoundation/CommandEncoder/CommandEncoderState.h>
#include <RendererFoundation/CommandEncoder/ComputeCommandEncoder.h>
#include <RendererFoundation/CommandEncoder/RenderCommandEncoder.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererVulkan/CommandEncoder/CommandEncoderImplVulkan.h>
#include <RendererVulkan/Device/DeviceVulkan.h>
#include <RendererVulkan/Device/PassVulkan.h>

nsGALPassVulkan::nsGALPassVulkan(nsGALDevice& device)
  : nsGALPass(device)
{
  m_pCommandEncoderState = NS_DEFAULT_NEW(nsGALCommandEncoderRenderState);
  m_pCommandEncoderImpl = NS_DEFAULT_NEW(nsGALCommandEncoderImplVulkan, static_cast<nsGALDeviceVulkan&>(device));

  m_pRenderCommandEncoder = NS_DEFAULT_NEW(nsGALRenderCommandEncoder, device, *m_pCommandEncoderState, *m_pCommandEncoderImpl, *m_pCommandEncoderImpl);
  m_pComputeCommandEncoder = NS_DEFAULT_NEW(nsGALComputeCommandEncoder, device, *m_pCommandEncoderState, *m_pCommandEncoderImpl, *m_pCommandEncoderImpl);
}

nsGALPassVulkan::~nsGALPassVulkan() = default;

void nsGALPassVulkan::Reset()
{
  m_pCommandEncoderImpl->Reset();
  m_pRenderCommandEncoder->InvalidateState();
  m_pComputeCommandEncoder->InvalidateState();
}

void nsGALPassVulkan::MarkDirty()
{
  m_pCommandEncoderImpl->MarkDirty();
}

void nsGALPassVulkan::SetCurrentCommandBuffer(vk::CommandBuffer* commandBuffer, nsPipelineBarrierVulkan* pipelineBarrier)
{
  m_pCommandEncoderImpl->SetCurrentCommandBuffer(commandBuffer, pipelineBarrier);
}

nsGALRenderCommandEncoder* nsGALPassVulkan::BeginRenderingPlatform(const nsGALRenderingSetup& renderingSetup, const char* szName)
{
  auto& deviceVulkan = static_cast<nsGALDeviceVulkan&>(m_Device);
  deviceVulkan.GetCurrentCommandBuffer();

  m_pCommandEncoderImpl->BeginRendering(renderingSetup);

  return m_pRenderCommandEncoder.Borrow();
}

void nsGALPassVulkan::EndRenderingPlatform(nsGALRenderCommandEncoder* pCommandEncoder)
{
  NS_ASSERT_DEV(m_pRenderCommandEncoder.Borrow() == pCommandEncoder, "Invalid command encoder");

  m_pCommandEncoderImpl->EndRendering();
}

nsGALComputeCommandEncoder* nsGALPassVulkan::BeginComputePlatform(const char* szName)
{
  auto& deviceVulkan = static_cast<nsGALDeviceVulkan&>(m_Device);
  deviceVulkan.GetCurrentCommandBuffer();

  m_pCommandEncoderImpl->BeginCompute();

  return m_pComputeCommandEncoder.Borrow();
}

void nsGALPassVulkan::EndComputePlatform(nsGALComputeCommandEncoder* pCommandEncoder)
{
  NS_ASSERT_DEV(m_pComputeCommandEncoder.Borrow() == pCommandEncoder, "Invalid command encoder");

  m_pCommandEncoderImpl->EndCompute();
}

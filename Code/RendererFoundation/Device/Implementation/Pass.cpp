#include <RendererFoundation/RendererFoundationPCH.h>

#include <RendererFoundation/CommandEncoder/ComputeCommandEncoder.h>
#include <RendererFoundation/CommandEncoder/RenderCommandEncoder.h>
#include <RendererFoundation/Device/Pass.h>

nsGALRenderCommandEncoder* nsGALPass::BeginRendering(const nsGALRenderingSetup& renderingSetup, const char* szName /*= ""*/)
{
  NS_ASSERT_DEV(m_CurrentCommandEncoderType == CommandEncoderType::Invalid, "Nested Command Encoder are not allowed");
  m_CurrentCommandEncoderType = CommandEncoderType::Render;

  nsGALRenderCommandEncoder* pCommandEncoder = BeginRenderingPlatform(renderingSetup, szName);

  m_bMarker = !nsStringUtils::IsNullOrEmpty(szName);
  if (m_bMarker)
  {
    pCommandEncoder->PushMarker(szName);
  }

  return pCommandEncoder;
}

void nsGALPass::EndRendering(nsGALRenderCommandEncoder* pCommandEncoder)
{
  NS_ASSERT_DEV(m_CurrentCommandEncoderType == CommandEncoderType::Render, "BeginRendering has not been called");
  m_CurrentCommandEncoderType = CommandEncoderType::Invalid;

  if (m_bMarker)
  {
    pCommandEncoder->PopMarker();
    m_bMarker = false;
  }

  EndRenderingPlatform(pCommandEncoder);
}

nsGALComputeCommandEncoder* nsGALPass::BeginCompute(const char* szName /*= ""*/)
{
  NS_ASSERT_DEV(m_CurrentCommandEncoderType == CommandEncoderType::Invalid, "Nested Command Encoder are not allowed");
  m_CurrentCommandEncoderType = CommandEncoderType::Compute;

  nsGALComputeCommandEncoder* pCommandEncoder = BeginComputePlatform(szName);

  m_bMarker = !nsStringUtils::IsNullOrEmpty(szName);
  if (m_bMarker)
  {
    pCommandEncoder->PushMarker(szName);
  }

  return pCommandEncoder;
}

void nsGALPass::EndCompute(nsGALComputeCommandEncoder* pCommandEncoder)
{
  NS_ASSERT_DEV(m_CurrentCommandEncoderType == CommandEncoderType::Compute, "BeginCompute has not been called");
  m_CurrentCommandEncoderType = CommandEncoderType::Invalid;

  if (m_bMarker)
  {
    pCommandEncoder->PopMarker();
    m_bMarker = false;
  }

  EndComputePlatform(pCommandEncoder);
}

nsGALPass::nsGALPass(nsGALDevice& device)
  : m_Device(device)
{
}

nsGALPass::~nsGALPass() = default;

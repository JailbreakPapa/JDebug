#include <RendererFoundation/RendererFoundationPCH.h>

#include <RendererFoundation/CommandEncoder/RenderCommandEncoder.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Resources/Buffer.h>
#include <RendererFoundation/Resources/Query.h>
#include <RendererFoundation/Resources/RenderTargetView.h>
#include <RendererFoundation/Resources/ResourceView.h>
#include <RendererFoundation/Resources/Texture.h>
#include <RendererFoundation/Resources/UnorderedAccesView.h>

nsGALRenderCommandEncoder::nsGALRenderCommandEncoder(nsGALDevice& ref_device, nsGALCommandEncoderRenderState& ref_renderState, nsGALCommandEncoderCommonPlatformInterface& ref_commonImpl, nsGALCommandEncoderRenderPlatformInterface& ref_renderImpl)
  : nsGALCommandEncoder(ref_device, ref_renderState, ref_commonImpl)
  , m_RenderState(ref_renderState)
  , m_RenderImpl(ref_renderImpl)
{
}

nsGALRenderCommandEncoder::~nsGALRenderCommandEncoder() = default;

void nsGALRenderCommandEncoder::Clear(const nsColor& clearColor, nsUInt32 uiRenderTargetClearMask /*= 0xFFFFFFFFu*/, bool bClearDepth /*= true*/, bool bClearStencil /*= true*/, float fDepthClear /*= 1.0f*/, nsUInt8 uiStencilClear /*= 0x0u*/)
{
  AssertRenderingThread();
  m_RenderImpl.ClearPlatform(clearColor, uiRenderTargetClearMask, bClearDepth, bClearStencil, fDepthClear, uiStencilClear);
}

nsResult nsGALRenderCommandEncoder::Draw(nsUInt32 uiVertexCount, nsUInt32 uiStartVertex)
{
  AssertRenderingThread();
  CountDrawCall();
  return m_RenderImpl.DrawPlatform(uiVertexCount, uiStartVertex);
}

nsResult nsGALRenderCommandEncoder::DrawIndexed(nsUInt32 uiIndexCount, nsUInt32 uiStartIndex)
{
  AssertRenderingThread();
  CountDrawCall();
  return m_RenderImpl.DrawIndexedPlatform(uiIndexCount, uiStartIndex);
}

nsResult nsGALRenderCommandEncoder::DrawIndexedInstanced(nsUInt32 uiIndexCountPerInstance, nsUInt32 uiInstanceCount, nsUInt32 uiStartIndex)
{
  AssertRenderingThread();
  CountDrawCall();
  return m_RenderImpl.DrawIndexedInstancedPlatform(uiIndexCountPerInstance, uiInstanceCount, uiStartIndex);
}

nsResult nsGALRenderCommandEncoder::DrawIndexedInstancedIndirect(nsGALBufferHandle hIndirectArgumentBuffer, nsUInt32 uiArgumentOffsetInBytes)
{
  AssertRenderingThread();
  const nsGALBuffer* pBuffer = GetDevice().GetBuffer(hIndirectArgumentBuffer);
  NS_ASSERT_DEV(pBuffer != nullptr, "Invalid buffer handle for indirect arguments!");
  CountDrawCall();
  return m_RenderImpl.DrawIndexedInstancedIndirectPlatform(pBuffer, uiArgumentOffsetInBytes);
}

nsResult nsGALRenderCommandEncoder::DrawInstanced(nsUInt32 uiVertexCountPerInstance, nsUInt32 uiInstanceCount, nsUInt32 uiStartVertex)
{
  AssertRenderingThread();
  CountDrawCall();
  return m_RenderImpl.DrawInstancedPlatform(uiVertexCountPerInstance, uiInstanceCount, uiStartVertex);
}

nsResult nsGALRenderCommandEncoder::DrawInstancedIndirect(nsGALBufferHandle hIndirectArgumentBuffer, nsUInt32 uiArgumentOffsetInBytes)
{
  AssertRenderingThread();
  const nsGALBuffer* pBuffer = GetDevice().GetBuffer(hIndirectArgumentBuffer);
  NS_ASSERT_DEV(pBuffer != nullptr, "Invalid buffer handle for indirect arguments!");
  CountDrawCall();
  return m_RenderImpl.DrawInstancedIndirectPlatform(pBuffer, uiArgumentOffsetInBytes);
}

void nsGALRenderCommandEncoder::SetIndexBuffer(nsGALBufferHandle hIndexBuffer)
{
  if (m_RenderState.m_hIndexBuffer == hIndexBuffer)
  {
    return;
  }

  const nsGALBuffer* pBuffer = GetDevice().GetBuffer(hIndexBuffer);
  /// \todo Assert on index buffer type (if non nullptr)
  // Note that GL4 can bind arbitrary buffer to arbitrary binding points (index/vertex/transform-feedback/indirect-draw/...)

  m_RenderImpl.SetIndexBufferPlatform(pBuffer);

  m_RenderState.m_hIndexBuffer = hIndexBuffer;
}

void nsGALRenderCommandEncoder::SetVertexBuffer(nsUInt32 uiSlot, nsGALBufferHandle hVertexBuffer)
{
  if (m_RenderState.m_hVertexBuffers[uiSlot] == hVertexBuffer)
  {
    return;
  }

  const nsGALBuffer* pBuffer = GetDevice().GetBuffer(hVertexBuffer);
  // Assert on vertex buffer type (if non-zero)
  // Note that GL4 can bind arbitrary buffer to arbitrary binding points (index/vertex/transform-feedback/indirect-draw/...)

  m_RenderImpl.SetVertexBufferPlatform(uiSlot, pBuffer);

  m_RenderState.m_hVertexBuffers[uiSlot] = hVertexBuffer;
}

void nsGALRenderCommandEncoder::SetPrimitiveTopology(nsGALPrimitiveTopology::Enum topology)
{
  AssertRenderingThread();

  if (m_RenderState.m_Topology == topology)
  {
    return;
  }

  m_RenderImpl.SetPrimitiveTopologyPlatform(topology);

  m_RenderState.m_Topology = topology;
}

void nsGALRenderCommandEncoder::SetVertexDeclaration(nsGALVertexDeclarationHandle hVertexDeclaration)
{
  AssertRenderingThread();

  if (m_RenderState.m_hVertexDeclaration == hVertexDeclaration)
  {
    return;
  }

  const nsGALVertexDeclaration* pVertexDeclaration = GetDevice().GetVertexDeclaration(hVertexDeclaration);
  // Assert on vertex buffer type (if non-zero)

  m_RenderImpl.SetVertexDeclarationPlatform(pVertexDeclaration);

  m_RenderState.m_hVertexDeclaration = hVertexDeclaration;
}

void nsGALRenderCommandEncoder::SetBlendState(nsGALBlendStateHandle hBlendState, const nsColor& blendFactor, nsUInt32 uiSampleMask)
{
  AssertRenderingThread();

  if (m_RenderState.m_hBlendState == hBlendState && m_RenderState.m_BlendFactor.IsEqualRGBA(blendFactor, 0.001f) && m_RenderState.m_uiSampleMask == uiSampleMask)
  {
    return;
  }

  const nsGALBlendState* pBlendState = GetDevice().GetBlendState(hBlendState);

  m_RenderImpl.SetBlendStatePlatform(pBlendState, blendFactor, uiSampleMask);

  m_RenderState.m_hBlendState = hBlendState;
  m_RenderState.m_BlendFactor = blendFactor;
  m_RenderState.m_uiSampleMask = uiSampleMask;
}

void nsGALRenderCommandEncoder::SetDepthStencilState(nsGALDepthStencilStateHandle hDepthStencilState, nsUInt8 uiStencilRefValue /*= 0xFFu*/)
{
  AssertRenderingThread();

  if (m_RenderState.m_hDepthStencilState == hDepthStencilState && m_RenderState.m_uiStencilRefValue == uiStencilRefValue)
  {
    return;
  }

  const nsGALDepthStencilState* pDepthStencilState = GetDevice().GetDepthStencilState(hDepthStencilState);

  m_RenderImpl.SetDepthStencilStatePlatform(pDepthStencilState, uiStencilRefValue);

  m_RenderState.m_hDepthStencilState = hDepthStencilState;
  m_RenderState.m_uiStencilRefValue = uiStencilRefValue;
}

void nsGALRenderCommandEncoder::SetRasterizerState(nsGALRasterizerStateHandle hRasterizerState)
{
  AssertRenderingThread();

  if (m_RenderState.m_hRasterizerState == hRasterizerState)
  {
    return;
  }

  const nsGALRasterizerState* pRasterizerState = GetDevice().GetRasterizerState(hRasterizerState);

  m_RenderImpl.SetRasterizerStatePlatform(pRasterizerState);

  m_RenderState.m_hRasterizerState = hRasterizerState;
}

void nsGALRenderCommandEncoder::SetViewport(const nsRectFloat& rect, float fMinDepth, float fMaxDepth)
{
  AssertRenderingThread();

  if (m_RenderState.m_ViewPortRect == rect && m_RenderState.m_fViewPortMinDepth == fMinDepth && m_RenderState.m_fViewPortMaxDepth == fMaxDepth)
  {
    return;
  }

  m_RenderImpl.SetViewportPlatform(rect, fMinDepth, fMaxDepth);

  m_RenderState.m_ViewPortRect = rect;
  m_RenderState.m_fViewPortMinDepth = fMinDepth;
  m_RenderState.m_fViewPortMaxDepth = fMaxDepth;
}

void nsGALRenderCommandEncoder::SetScissorRect(const nsRectU32& rect)
{
  AssertRenderingThread();

  if (m_RenderState.m_ScissorRect == rect)
  {
    return;
  }

  m_RenderImpl.SetScissorRectPlatform(rect);

  m_RenderState.m_ScissorRect = rect;
}

void nsGALRenderCommandEncoder::ClearStatisticsCounters()
{
  nsGALCommandEncoder::ClearStatisticsCounters();

  m_uiDrawCalls = 0;
}

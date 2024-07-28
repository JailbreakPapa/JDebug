
#pragma once

#include <Foundation/Math/Rect.h>
#include <RendererFoundation/CommandEncoder/CommandEncoder.h>

class NS_RENDERERFOUNDATION_DLL nsGALRenderCommandEncoder : public nsGALCommandEncoder
{
public:
  nsGALRenderCommandEncoder(nsGALDevice& ref_device, nsGALCommandEncoderRenderState& ref_renderState, nsGALCommandEncoderCommonPlatformInterface& ref_commonImpl, nsGALCommandEncoderRenderPlatformInterface& ref_renderImpl);
  virtual ~nsGALRenderCommandEncoder();

  // Draw functions

  /// \brief Clears active rendertargets.
  ///
  /// \param uiRenderTargetClearMask
  ///   Each bit represents a bound color target. If all bits are set, all bound color targets will be cleared.
  void Clear(const nsColor& clearColor, nsUInt32 uiRenderTargetClearMask = 0xFFFFFFFFu, bool bClearDepth = true, bool bClearStencil = true, float fDepthClear = 1.0f, nsUInt8 uiStencilClear = 0x0u);

  nsResult Draw(nsUInt32 uiVertexCount, nsUInt32 uiStartVertex);
  nsResult DrawIndexed(nsUInt32 uiIndexCount, nsUInt32 uiStartIndex);
  nsResult DrawIndexedInstanced(nsUInt32 uiIndexCountPerInstance, nsUInt32 uiInstanceCount, nsUInt32 uiStartIndex);
  nsResult DrawIndexedInstancedIndirect(nsGALBufferHandle hIndirectArgumentBuffer, nsUInt32 uiArgumentOffsetInBytes);
  nsResult DrawInstanced(nsUInt32 uiVertexCountPerInstance, nsUInt32 uiInstanceCount, nsUInt32 uiStartVertex);
  nsResult DrawInstancedIndirect(nsGALBufferHandle hIndirectArgumentBuffer, nsUInt32 uiArgumentOffsetInBytes);

  // State functions

  void SetIndexBuffer(nsGALBufferHandle hIndexBuffer);
  void SetVertexBuffer(nsUInt32 uiSlot, nsGALBufferHandle hVertexBuffer);
  void SetVertexDeclaration(nsGALVertexDeclarationHandle hVertexDeclaration);

  nsGALPrimitiveTopology::Enum GetPrimitiveTopology() const { return m_RenderState.m_Topology; }
  void SetPrimitiveTopology(nsGALPrimitiveTopology::Enum topology);

  void SetBlendState(nsGALBlendStateHandle hBlendState, const nsColor& blendFactor = nsColor::White, nsUInt32 uiSampleMask = 0xFFFFFFFFu);
  void SetDepthStencilState(nsGALDepthStencilStateHandle hDepthStencilState, nsUInt8 uiStencilRefValue = 0xFFu);
  void SetRasterizerState(nsGALRasterizerStateHandle hRasterizerState);

  void SetViewport(const nsRectFloat& rect, float fMinDepth = 0.0f, float fMaxDepth = 1.0f);
  void SetScissorRect(const nsRectU32& rect);

  virtual void ClearStatisticsCounters() override;

private:
  void CountDrawCall() { m_uiDrawCalls++; }

  // Statistic variables
  nsUInt32 m_uiDrawCalls = 0;

  nsGALCommandEncoderRenderState& m_RenderState;

  nsGALCommandEncoderRenderPlatformInterface& m_RenderImpl;
};

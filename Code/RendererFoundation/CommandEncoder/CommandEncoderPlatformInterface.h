
#pragma once

#include <Foundation/Math/Color.h>
#include <Foundation/Math/Rect.h>
#include <RendererFoundation/RendererFoundationDLL.h>

struct nsShaderResourceBinding;

class NS_RENDERERFOUNDATION_DLL nsGALCommandEncoderCommonPlatformInterface
{
public:
  // State setting functions

  virtual void SetShaderPlatform(const nsGALShader* pShader) = 0;

  virtual void SetConstantBufferPlatform(const nsShaderResourceBinding& binding, const nsGALBuffer* pBuffer) = 0;
  virtual void SetSamplerStatePlatform(const nsShaderResourceBinding& binding, const nsGALSamplerState* pSamplerState) = 0;
  virtual void SetResourceViewPlatform(const nsShaderResourceBinding& binding, const nsGALTextureResourceView* pResourceView) = 0;
  virtual void SetResourceViewPlatform(const nsShaderResourceBinding& binding, const nsGALBufferResourceView* pResourceView) = 0;
  virtual void SetUnorderedAccessViewPlatform(const nsShaderResourceBinding& binding, const nsGALTextureUnorderedAccessView* pUnorderedAccessView) = 0;
  virtual void SetUnorderedAccessViewPlatform(const nsShaderResourceBinding& binding, const nsGALBufferUnorderedAccessView* pUnorderedAccessView) = 0;
  virtual void SetPushConstantsPlatform(nsArrayPtr<const nsUInt8> data) = 0;

  // Query functions

  virtual void BeginQueryPlatform(const nsGALQuery* pQuery) = 0;
  virtual void EndQueryPlatform(const nsGALQuery* pQuery) = 0;
  virtual nsResult GetQueryResultPlatform(const nsGALQuery* pQuery, nsUInt64& ref_uiQueryResult) = 0;

  // Timestamp functions

  virtual void InsertTimestampPlatform(nsGALTimestampHandle hTimestamp) = 0;

  // Resource update functions

  virtual void ClearUnorderedAccessViewPlatform(const nsGALTextureUnorderedAccessView* pUnorderedAccessView, nsVec4 vClearValues) = 0;
  virtual void ClearUnorderedAccessViewPlatform(const nsGALTextureUnorderedAccessView* pUnorderedAccessView, nsVec4U32 vClearValues) = 0;

  virtual void ClearUnorderedAccessViewPlatform(const nsGALBufferUnorderedAccessView* pUnorderedAccessView, nsVec4 vClearValues) = 0;
  virtual void ClearUnorderedAccessViewPlatform(const nsGALBufferUnorderedAccessView* pUnorderedAccessView, nsVec4U32 vClearValues) = 0;

  virtual void CopyBufferPlatform(const nsGALBuffer* pDestination, const nsGALBuffer* pSource) = 0;
  virtual void CopyBufferRegionPlatform(const nsGALBuffer* pDestination, nsUInt32 uiDestOffset, const nsGALBuffer* pSource, nsUInt32 uiSourceOffset, nsUInt32 uiByteCount) = 0;

  virtual void UpdateBufferPlatform(const nsGALBuffer* pDestination, nsUInt32 uiDestOffset, nsArrayPtr<const nsUInt8> sourceData, nsGALUpdateMode::Enum updateMode) = 0;

  virtual void CopyTexturePlatform(const nsGALTexture* pDestination, const nsGALTexture* pSource) = 0;
  virtual void CopyTextureRegionPlatform(const nsGALTexture* pDestination, const nsGALTextureSubresource& destinationSubResource, const nsVec3U32& vDestinationPoint, const nsGALTexture* pSource, const nsGALTextureSubresource& sourceSubResource, const nsBoundingBoxu32& box) = 0;

  virtual void UpdateTexturePlatform(const nsGALTexture* pDestination, const nsGALTextureSubresource& destinationSubResource, const nsBoundingBoxu32& destinationBox, const nsGALSystemMemoryDescription& sourceData) = 0;

  virtual void ResolveTexturePlatform(const nsGALTexture* pDestination, const nsGALTextureSubresource& destinationSubResource, const nsGALTexture* pSource, const nsGALTextureSubresource& sourceSubResource) = 0;

  virtual void ReadbackTexturePlatform(const nsGALTexture* pTexture) = 0;

  virtual void CopyTextureReadbackResultPlatform(const nsGALTexture* pTexture, nsArrayPtr<nsGALTextureSubresource> sourceSubResource, nsArrayPtr<nsGALSystemMemoryDescription> targetData) = 0;

  virtual void GenerateMipMapsPlatform(const nsGALTextureResourceView* pResourceView) = 0;

  // Misc

  virtual void FlushPlatform() = 0;

  // Debug helper functions

  virtual void PushMarkerPlatform(const char* szMarker) = 0;
  virtual void PopMarkerPlatform() = 0;
  virtual void InsertEventMarkerPlatform(const char* szMarker) = 0;
};

class NS_RENDERERFOUNDATION_DLL nsGALCommandEncoderRenderPlatformInterface
{
public:
  // Draw functions

  virtual void ClearPlatform(const nsColor& clearColor, nsUInt32 uiRenderTargetClearMask, bool bClearDepth, bool bClearStencil, float fDepthClear, nsUInt8 uiStencilClear) = 0;

  virtual nsResult DrawPlatform(nsUInt32 uiVertexCount, nsUInt32 uiStartVertex) = 0;
  virtual nsResult DrawIndexedPlatform(nsUInt32 uiIndexCount, nsUInt32 uiStartIndex) = 0;
  virtual nsResult DrawIndexedInstancedPlatform(nsUInt32 uiIndexCountPerInstance, nsUInt32 uiInstanceCount, nsUInt32 uiStartIndex) = 0;
  virtual nsResult DrawIndexedInstancedIndirectPlatform(const nsGALBuffer* pIndirectArgumentBuffer, nsUInt32 uiArgumentOffsetInBytes) = 0;
  virtual nsResult DrawInstancedPlatform(nsUInt32 uiVertexCountPerInstance, nsUInt32 uiInstanceCount, nsUInt32 uiStartVertex) = 0;
  virtual nsResult DrawInstancedIndirectPlatform(const nsGALBuffer* pIndirectArgumentBuffer, nsUInt32 uiArgumentOffsetInBytes) = 0;

  // State functions

  virtual void SetIndexBufferPlatform(const nsGALBuffer* pIndexBuffer) = 0;
  virtual void SetVertexBufferPlatform(nsUInt32 uiSlot, const nsGALBuffer* pVertexBuffer) = 0;
  virtual void SetVertexDeclarationPlatform(const nsGALVertexDeclaration* pVertexDeclaration) = 0;
  virtual void SetPrimitiveTopologyPlatform(nsGALPrimitiveTopology::Enum topology) = 0;

  virtual void SetBlendStatePlatform(const nsGALBlendState* pBlendState, const nsColor& blendFactor, nsUInt32 uiSampleMask) = 0;
  virtual void SetDepthStencilStatePlatform(const nsGALDepthStencilState* pDepthStencilState, nsUInt8 uiStencilRefValue) = 0;
  virtual void SetRasterizerStatePlatform(const nsGALRasterizerState* pRasterizerState) = 0;

  virtual void SetViewportPlatform(const nsRectFloat& rect, float fMinDepth, float fMaxDepth) = 0;
  virtual void SetScissorRectPlatform(const nsRectU32& rect) = 0;
};

class NS_RENDERERFOUNDATION_DLL nsGALCommandEncoderComputePlatformInterface
{
public:
  // Dispatch

  virtual nsResult DispatchPlatform(nsUInt32 uiThreadGroupCountX, nsUInt32 uiThreadGroupCountY, nsUInt32 uiThreadGroupCountZ) = 0;
  virtual nsResult DispatchIndirectPlatform(const nsGALBuffer* pIndirectArgumentBuffer, nsUInt32 uiArgumentOffsetInBytes) = 0;
};


#pragma once

#include <Foundation/Threading/ThreadUtils.h>
#include <RendererFoundation/CommandEncoder/CommandEncoderPlatformInterface.h>
#include <RendererFoundation/CommandEncoder/CommandEncoderState.h>

class NS_RENDERERFOUNDATION_DLL nsGALCommandEncoder
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsGALCommandEncoder);

public:
  // State setting functions

  void SetShader(nsGALShaderHandle hShader);

  void SetConstantBuffer(const nsShaderResourceBinding& binding, nsGALBufferHandle hBuffer);
  void SetSamplerState(const nsShaderResourceBinding& binding, nsGALSamplerStateHandle hSamplerState);
  void SetResourceView(const nsShaderResourceBinding& binding, nsGALTextureResourceViewHandle hResourceView);
  void SetResourceView(const nsShaderResourceBinding& binding, nsGALBufferResourceViewHandle hResourceView);
  void SetUnorderedAccessView(const nsShaderResourceBinding& binding, nsGALTextureUnorderedAccessViewHandle hUnorderedAccessView);
  void SetUnorderedAccessView(const nsShaderResourceBinding& binding, nsGALBufferUnorderedAccessViewHandle hUnorderedAccessView);
  void SetPushConstants(nsArrayPtr<const nsUInt8> data);

  // Query functions

  void BeginQuery(nsGALQueryHandle hQuery);
  void EndQuery(nsGALQueryHandle hQuery);

  /// \return Success if retrieving the query succeeded.
  nsResult GetQueryResult(nsGALQueryHandle hQuery, nsUInt64& ref_uiQueryResult);

  // Timestamp functions

  nsGALTimestampHandle InsertTimestamp();

  // Resource functions

  /// Clears an unordered access view with a float value.
  void ClearUnorderedAccessView(nsGALTextureUnorderedAccessViewHandle hUnorderedAccessView, nsVec4 vClearValues);
  void ClearUnorderedAccessView(nsGALBufferUnorderedAccessViewHandle hUnorderedAccessView, nsVec4 vClearValues);

  /// Clears an unordered access view with an int value.
  void ClearUnorderedAccessView(nsGALTextureUnorderedAccessViewHandle hUnorderedAccessView, nsVec4U32 vClearValues);
  void ClearUnorderedAccessView(nsGALBufferUnorderedAccessViewHandle hUnorderedAccessView, nsVec4U32 vClearValues);

  void CopyBuffer(nsGALBufferHandle hDest, nsGALBufferHandle hSource);
  void CopyBufferRegion(nsGALBufferHandle hDest, nsUInt32 uiDestOffset, nsGALBufferHandle hSource, nsUInt32 uiSourceOffset, nsUInt32 uiByteCount);
  void UpdateBuffer(nsGALBufferHandle hDest, nsUInt32 uiDestOffset, nsArrayPtr<const nsUInt8> sourceData, nsGALUpdateMode::Enum updateMode = nsGALUpdateMode::Discard);

  void CopyTexture(nsGALTextureHandle hDest, nsGALTextureHandle hSource);
  void CopyTextureRegion(nsGALTextureHandle hDest, const nsGALTextureSubresource& destinationSubResource, const nsVec3U32& vDestinationPoint, nsGALTextureHandle hSource, const nsGALTextureSubresource& sourceSubResource, const nsBoundingBoxu32& box);

  void UpdateTexture(nsGALTextureHandle hDest, const nsGALTextureSubresource& destinationSubResource, const nsBoundingBoxu32& destinationBox, const nsGALSystemMemoryDescription& sourceData);

  void ResolveTexture(nsGALTextureHandle hDest, const nsGALTextureSubresource& destinationSubResource, nsGALTextureHandle hSource, const nsGALTextureSubresource& sourceSubResource);

  void ReadbackTexture(nsGALTextureHandle hTexture);
  void CopyTextureReadbackResult(nsGALTextureHandle hTexture, nsArrayPtr<nsGALTextureSubresource> sourceSubResource, nsArrayPtr<nsGALSystemMemoryDescription> targetData);

  void GenerateMipMaps(nsGALTextureResourceViewHandle hResourceView);

  // Misc

  void Flush();

  // Debug helper functions

  void PushMarker(const char* szMarker);
  void PopMarker();
  void InsertEventMarker(const char* szMarker);

  virtual void ClearStatisticsCounters();

  NS_ALWAYS_INLINE nsGALDevice& GetDevice() { return m_Device; }
  // Don't use light hearted ;)
  void InvalidateState();

protected:
  friend class nsGALDevice;

  nsGALCommandEncoder(nsGALDevice& device, nsGALCommandEncoderState& state, nsGALCommandEncoderCommonPlatformInterface& commonImpl);
  virtual ~nsGALCommandEncoder();


  void AssertRenderingThread()
  {
    NS_ASSERT_DEV(nsThreadUtils::IsMainThread(), "This function can only be executed on the main thread.");
  }

private:
  friend class nsMemoryUtils;

  // Parent Device
  nsGALDevice& m_Device;
  nsGALCommandEncoderState& m_State;
  nsGALCommandEncoderCommonPlatformInterface& m_CommonImpl;
};

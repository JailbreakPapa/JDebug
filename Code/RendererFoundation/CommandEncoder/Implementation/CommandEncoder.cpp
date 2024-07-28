#include <RendererFoundation/RendererFoundationPCH.h>

#include <RendererFoundation/CommandEncoder/CommandEncoder.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Resources/Buffer.h>
#include <RendererFoundation/Resources/Query.h>
#include <RendererFoundation/Resources/RenderTargetView.h>
#include <RendererFoundation/Resources/ResourceView.h>
#include <RendererFoundation/Resources/Texture.h>
#include <RendererFoundation/Resources/UnorderedAccesView.h>

void nsGALCommandEncoder::SetShader(nsGALShaderHandle hShader)
{
  AssertRenderingThread();
  /// \todo Assert for shader capabilities (supported shader stages etc.)

  if (m_State.m_hShader == hShader)
    return;

  const nsGALShader* pShader = m_Device.GetShader(hShader);
  NS_ASSERT_DEV(pShader != nullptr, "The given shader handle isn't valid, this may be a use after destroy!");

  m_CommonImpl.SetShaderPlatform(pShader);

  m_State.m_hShader = hShader;
}

void nsGALCommandEncoder::SetConstantBuffer(const nsShaderResourceBinding& binding, nsGALBufferHandle hBuffer)
{
  AssertRenderingThread();

  const nsGALBuffer* pBuffer = m_Device.GetBuffer(hBuffer);
  NS_ASSERT_DEV(pBuffer == nullptr || pBuffer->GetDescription().m_BufferFlags.IsSet(nsGALBufferUsageFlags::ConstantBuffer), "Wrong buffer type");

  m_CommonImpl.SetConstantBufferPlatform(binding, pBuffer);
}

void nsGALCommandEncoder::SetSamplerState(const nsShaderResourceBinding& binding, nsGALSamplerStateHandle hSamplerState)
{
  AssertRenderingThread();

  const nsGALSamplerState* pSamplerState = m_Device.GetSamplerState(hSamplerState);

  m_CommonImpl.SetSamplerStatePlatform(binding, pSamplerState);
}

void nsGALCommandEncoder::SetResourceView(const nsShaderResourceBinding& binding, nsGALTextureResourceViewHandle hResourceView)
{
  AssertRenderingThread();

  const nsGALTextureResourceView* pResourceView = m_Device.GetResourceView(hResourceView);

  m_CommonImpl.SetResourceViewPlatform(binding, pResourceView);
}

void nsGALCommandEncoder::SetResourceView(const nsShaderResourceBinding& binding, nsGALBufferResourceViewHandle hResourceView)
{
  AssertRenderingThread();

  const nsGALBufferResourceView* pResourceView = m_Device.GetResourceView(hResourceView);

  m_CommonImpl.SetResourceViewPlatform(binding, pResourceView);
}

void nsGALCommandEncoder::SetUnorderedAccessView(const nsShaderResourceBinding& binding, nsGALTextureUnorderedAccessViewHandle hUnorderedAccessView)
{
  AssertRenderingThread();

  const nsGALTextureUnorderedAccessView* pUnorderedAccessView = m_Device.GetUnorderedAccessView(hUnorderedAccessView);
  m_CommonImpl.SetUnorderedAccessViewPlatform(binding, pUnorderedAccessView);
}

void nsGALCommandEncoder::SetUnorderedAccessView(const nsShaderResourceBinding& binding, nsGALBufferUnorderedAccessViewHandle hUnorderedAccessView)
{
  AssertRenderingThread();

  const nsGALBufferUnorderedAccessView* pUnorderedAccessView = m_Device.GetUnorderedAccessView(hUnorderedAccessView);
  m_CommonImpl.SetUnorderedAccessViewPlatform(binding, pUnorderedAccessView);
}

void nsGALCommandEncoder::SetPushConstants(nsArrayPtr<const nsUInt8> data)
{
  AssertRenderingThread();
  m_CommonImpl.SetPushConstantsPlatform(data);
}

void nsGALCommandEncoder::BeginQuery(nsGALQueryHandle hQuery)
{
  AssertRenderingThread();

  auto query = m_Device.GetQuery(hQuery);
  NS_ASSERT_DEV(!query->m_bStarted, "Can't stat nsGALQuery because it is already running.");

  m_CommonImpl.BeginQueryPlatform(query);
}

void nsGALCommandEncoder::EndQuery(nsGALQueryHandle hQuery)
{
  AssertRenderingThread();

  auto query = m_Device.GetQuery(hQuery);
  NS_ASSERT_DEV(query->m_bStarted, "Can't end nsGALQuery, query hasn't started yet.");

  m_CommonImpl.EndQueryPlatform(query);
}

nsResult nsGALCommandEncoder::GetQueryResult(nsGALQueryHandle hQuery, nsUInt64& ref_uiQueryResult)
{
  AssertRenderingThread();

  auto query = m_Device.GetQuery(hQuery);
  NS_ASSERT_DEV(!query->m_bStarted, "Can't retrieve data from nsGALQuery while query is still running.");

  return m_CommonImpl.GetQueryResultPlatform(query, ref_uiQueryResult);
}

nsGALTimestampHandle nsGALCommandEncoder::InsertTimestamp()
{
  nsGALTimestampHandle hTimestamp = m_Device.GetTimestamp();

  m_CommonImpl.InsertTimestampPlatform(hTimestamp);

  return hTimestamp;
}

void nsGALCommandEncoder::ClearUnorderedAccessView(nsGALTextureUnorderedAccessViewHandle hUnorderedAccessView, nsVec4 vClearValues)
{
  AssertRenderingThread();

  const nsGALTextureUnorderedAccessView* pUnorderedAccessView = m_Device.GetUnorderedAccessView(hUnorderedAccessView);
  if (pUnorderedAccessView == nullptr)
  {
    NS_REPORT_FAILURE("ClearUnorderedAccessView failed, unordered access view handle invalid.");
    return;
  }

  m_CommonImpl.ClearUnorderedAccessViewPlatform(pUnorderedAccessView, vClearValues);
}

void nsGALCommandEncoder::ClearUnorderedAccessView(nsGALBufferUnorderedAccessViewHandle hUnorderedAccessView, nsVec4 vClearValues)
{
  AssertRenderingThread();

  const nsGALBufferUnorderedAccessView* pUnorderedAccessView = m_Device.GetUnorderedAccessView(hUnorderedAccessView);
  if (pUnorderedAccessView == nullptr)
  {
    NS_REPORT_FAILURE("ClearUnorderedAccessView failed, unordered access view handle invalid.");
    return;
  }

  m_CommonImpl.ClearUnorderedAccessViewPlatform(pUnorderedAccessView, vClearValues);
}

void nsGALCommandEncoder::ClearUnorderedAccessView(nsGALTextureUnorderedAccessViewHandle hUnorderedAccessView, nsVec4U32 vClearValues)
{
  AssertRenderingThread();

  const nsGALTextureUnorderedAccessView* pUnorderedAccessView = m_Device.GetUnorderedAccessView(hUnorderedAccessView);
  if (pUnorderedAccessView == nullptr)
  {
    NS_REPORT_FAILURE("ClearUnorderedAccessView failed, unordered access view handle invalid.");
    return;
  }

  m_CommonImpl.ClearUnorderedAccessViewPlatform(pUnorderedAccessView, vClearValues);
}

void nsGALCommandEncoder::ClearUnorderedAccessView(nsGALBufferUnorderedAccessViewHandle hUnorderedAccessView, nsVec4U32 vClearValues)
{
  AssertRenderingThread();

  const nsGALBufferUnorderedAccessView* pUnorderedAccessView = m_Device.GetUnorderedAccessView(hUnorderedAccessView);
  if (pUnorderedAccessView == nullptr)
  {
    NS_REPORT_FAILURE("ClearUnorderedAccessView failed, unordered access view handle invalid.");
    return;
  }

  m_CommonImpl.ClearUnorderedAccessViewPlatform(pUnorderedAccessView, vClearValues);
}

void nsGALCommandEncoder::CopyBuffer(nsGALBufferHandle hDest, nsGALBufferHandle hSource)
{
  AssertRenderingThread();

  const nsGALBuffer* pDest = m_Device.GetBuffer(hDest);
  const nsGALBuffer* pSource = m_Device.GetBuffer(hSource);

  if (pDest != nullptr && pSource != nullptr)
  {
    m_CommonImpl.CopyBufferPlatform(pDest, pSource);
  }
  else
  {
    NS_REPORT_FAILURE("CopyBuffer failed, buffer handle invalid - destination = {0}, source = {1}", nsArgP(pDest), nsArgP(pSource));
  }
}

void nsGALCommandEncoder::CopyBufferRegion(
  nsGALBufferHandle hDest, nsUInt32 uiDestOffset, nsGALBufferHandle hSource, nsUInt32 uiSourceOffset, nsUInt32 uiByteCount)
{
  AssertRenderingThread();

  const nsGALBuffer* pDest = m_Device.GetBuffer(hDest);
  const nsGALBuffer* pSource = m_Device.GetBuffer(hSource);

  if (pDest != nullptr && pSource != nullptr)
  {
    const nsUInt32 uiDestSize = pDest->GetSize();
    const nsUInt32 uiSourceSize = pSource->GetSize();

    NS_IGNORE_UNUSED(uiDestSize);
    NS_ASSERT_DEV(uiDestSize >= uiDestOffset + uiByteCount, "Destination buffer too small (or offset too big)");
    NS_IGNORE_UNUSED(uiSourceSize);
    NS_ASSERT_DEV(uiSourceSize >= uiSourceOffset + uiByteCount, "Source buffer too small (or offset too big)");

    m_CommonImpl.CopyBufferRegionPlatform(pDest, uiDestOffset, pSource, uiSourceOffset, uiByteCount);
  }
  else
  {
    NS_REPORT_FAILURE("CopyBuffer failed, buffer handle invalid - destination = {0}, source = {1}", nsArgP(pDest), nsArgP(pSource));
  }
}

void nsGALCommandEncoder::UpdateBuffer(nsGALBufferHandle hDest, nsUInt32 uiDestOffset, nsArrayPtr<const nsUInt8> sourceData, nsGALUpdateMode::Enum updateMode)
{
  AssertRenderingThread();

  NS_ASSERT_DEV(!sourceData.IsEmpty(), "Source data for buffer update is invalid!");

  const nsGALBuffer* pDest = m_Device.GetBuffer(hDest);

  if (pDest != nullptr)
  {
    if (updateMode == nsGALUpdateMode::NoOverwrite && !(GetDevice().GetCapabilities().m_bNoOverwriteBufferUpdate))
    {
      updateMode = nsGALUpdateMode::CopyToTempStorage;
    }

    NS_ASSERT_DEV(pDest->GetSize() >= (uiDestOffset + sourceData.GetCount()), "Buffer {} is too small (or offset {} too big) for {} bytes", pDest->GetSize(), uiDestOffset, sourceData.GetCount());
    m_CommonImpl.UpdateBufferPlatform(pDest, uiDestOffset, sourceData, updateMode);
  }
  else
  {
    NS_REPORT_FAILURE("UpdateBuffer failed, buffer handle invalid");
  }
}

void nsGALCommandEncoder::CopyTexture(nsGALTextureHandle hDest, nsGALTextureHandle hSource)
{
  AssertRenderingThread();

  const nsGALTexture* pDest = m_Device.GetTexture(hDest);
  const nsGALTexture* pSource = m_Device.GetTexture(hSource);

  if (pDest != nullptr && pSource != nullptr)
  {
    m_CommonImpl.CopyTexturePlatform(pDest, pSource);
  }
  else
  {
    NS_REPORT_FAILURE("CopyTexture failed, texture handle invalid - destination = {0}, source = {1}", nsArgP(pDest), nsArgP(pSource));
  }
}

void nsGALCommandEncoder::CopyTextureRegion(nsGALTextureHandle hDest, const nsGALTextureSubresource& destinationSubResource,
  const nsVec3U32& vDestinationPoint, nsGALTextureHandle hSource, const nsGALTextureSubresource& sourceSubResource, const nsBoundingBoxu32& box)
{
  AssertRenderingThread();

  const nsGALTexture* pDest = m_Device.GetTexture(hDest);
  const nsGALTexture* pSource = m_Device.GetTexture(hSource);

  if (pDest != nullptr && pSource != nullptr)
  {
    m_CommonImpl.CopyTextureRegionPlatform(pDest, destinationSubResource, vDestinationPoint, pSource, sourceSubResource, box);
  }
  else
  {
    NS_REPORT_FAILURE("CopyTextureRegion failed, texture handle invalid - destination = {0}, source = {1}", nsArgP(pDest), nsArgP(pSource));
  }
}

void nsGALCommandEncoder::UpdateTexture(nsGALTextureHandle hDest, const nsGALTextureSubresource& destinationSubResource,
  const nsBoundingBoxu32& destinationBox, const nsGALSystemMemoryDescription& sourceData)
{
  AssertRenderingThread();

  const nsGALTexture* pDest = m_Device.GetTexture(hDest);

  if (pDest != nullptr)
  {
    m_CommonImpl.UpdateTexturePlatform(pDest, destinationSubResource, destinationBox, sourceData);
  }
  else
  {
    NS_REPORT_FAILURE("UpdateTexture failed, texture handle invalid - destination = {0}", nsArgP(pDest));
  }
}

void nsGALCommandEncoder::ResolveTexture(nsGALTextureHandle hDest, const nsGALTextureSubresource& destinationSubResource, nsGALTextureHandle hSource,
  const nsGALTextureSubresource& sourceSubResource)
{
  AssertRenderingThread();

  const nsGALTexture* pDest = m_Device.GetTexture(hDest);
  const nsGALTexture* pSource = m_Device.GetTexture(hSource);

  if (pDest != nullptr && pSource != nullptr)
  {
    m_CommonImpl.ResolveTexturePlatform(pDest, destinationSubResource, pSource, sourceSubResource);
  }
  else
  {
    NS_REPORT_FAILURE("ResolveTexture failed, texture handle invalid - destination = {0}, source = {1}", nsArgP(pDest), nsArgP(pSource));
  }
}

void nsGALCommandEncoder::ReadbackTexture(nsGALTextureHandle hTexture)
{
  AssertRenderingThread();

  const nsGALTexture* pTexture = m_Device.GetTexture(hTexture);

  if (pTexture != nullptr)
  {
    NS_ASSERT_RELEASE(pTexture->GetDescription().m_ResourceAccess.m_bReadBack,
      "A texture supplied to read-back needs to be created with the correct resource usage (m_bReadBack = true)!");

    m_CommonImpl.ReadbackTexturePlatform(pTexture);
  }
}

void nsGALCommandEncoder::CopyTextureReadbackResult(nsGALTextureHandle hTexture, nsArrayPtr<nsGALTextureSubresource> sourceSubResource, nsArrayPtr<nsGALSystemMemoryDescription> targetData)
{
  AssertRenderingThread();

  const nsGALTexture* pTexture = m_Device.GetTexture(hTexture);

  if (pTexture != nullptr)
  {
    NS_ASSERT_RELEASE(pTexture->GetDescription().m_ResourceAccess.m_bReadBack,
      "A texture supplied to read-back needs to be created with the correct resource usage (m_bReadBack = true)!");

    m_CommonImpl.CopyTextureReadbackResultPlatform(pTexture, sourceSubResource, targetData);
  }
}

void nsGALCommandEncoder::GenerateMipMaps(nsGALTextureResourceViewHandle hResourceView)
{
  AssertRenderingThread();

  const nsGALTextureResourceView* pResourceView = m_Device.GetResourceView(hResourceView);
  if (pResourceView != nullptr)
  {
    NS_ASSERT_DEV(!pResourceView->GetDescription().m_hTexture.IsInvalidated(), "Resource view needs a valid texture to generate mip maps.");
    const nsGALTexture* pTexture = m_Device.GetTexture(pResourceView->GetDescription().m_hTexture);
    NS_IGNORE_UNUSED(pTexture);
    NS_ASSERT_DEV(pTexture->GetDescription().m_bAllowDynamicMipGeneration,
      "Dynamic mip map generation needs to be enabled (m_bAllowDynamicMipGeneration = true)!");

    m_CommonImpl.GenerateMipMapsPlatform(pResourceView);
  }
}

void nsGALCommandEncoder::Flush()
{
  AssertRenderingThread();

  m_CommonImpl.FlushPlatform();
}

// Debug helper functions

void nsGALCommandEncoder::PushMarker(const char* szMarker)
{
  AssertRenderingThread();

  NS_ASSERT_DEV(szMarker != nullptr, "Invalid marker!");

  m_CommonImpl.PushMarkerPlatform(szMarker);
}

void nsGALCommandEncoder::PopMarker()
{
  AssertRenderingThread();

  m_CommonImpl.PopMarkerPlatform();
}

void nsGALCommandEncoder::InsertEventMarker(const char* szMarker)
{
  AssertRenderingThread();

  NS_ASSERT_DEV(szMarker != nullptr, "Invalid marker!");

  m_CommonImpl.InsertEventMarkerPlatform(szMarker);
}

void nsGALCommandEncoder::ClearStatisticsCounters()
{
}

nsGALCommandEncoder::nsGALCommandEncoder(nsGALDevice& device, nsGALCommandEncoderState& state, nsGALCommandEncoderCommonPlatformInterface& commonImpl)
  : m_Device(device)
  , m_State(state)
  , m_CommonImpl(commonImpl)
{
}

nsGALCommandEncoder::~nsGALCommandEncoder() = default;

void nsGALCommandEncoder::InvalidateState()
{
  m_State.InvalidateState();
}

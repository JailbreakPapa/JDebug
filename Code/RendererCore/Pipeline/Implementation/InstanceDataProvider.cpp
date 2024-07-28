#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Pipeline/ExtractedRenderData.h>
#include <RendererCore/Pipeline/InstanceDataProvider.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Profiling/Profiling.h>

#include <Shaders/Common/ObjectConstants.h>

nsInstanceData::nsInstanceData(nsUInt32 uiMaxInstanceCount /*= 1024*/)

{
  CreateBuffer(uiMaxInstanceCount);

  m_hConstantBuffer = nsRenderContext::CreateConstantBufferStorage<nsObjectConstants>();
}

nsInstanceData::~nsInstanceData()
{
  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  pDevice->DestroyBuffer(m_hInstanceDataBuffer);

  nsRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
}

void nsInstanceData::BindResources(nsRenderContext* pRenderContext)
{
  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  pRenderContext->BindBuffer("perInstanceData", pDevice->GetDefaultResourceView(m_hInstanceDataBuffer));
  pRenderContext->BindConstantBuffer("nsObjectConstants", m_hConstantBuffer);
}

nsArrayPtr<nsPerInstanceData> nsInstanceData::GetInstanceData(nsUInt32 uiCount, nsUInt32& out_uiOffset)
{
  uiCount = nsMath::Min(uiCount, m_uiBufferSize);
  if (m_uiBufferOffset + uiCount > m_uiBufferSize)
  {
    m_uiBufferOffset = 0;
  }

  out_uiOffset = m_uiBufferOffset;
  return m_PerInstanceData.GetArrayPtr().GetSubArray(m_uiBufferOffset, uiCount);
}

void nsInstanceData::UpdateInstanceData(nsRenderContext* pRenderContext, nsUInt32 uiCount)
{
  NS_ASSERT_DEV(m_uiBufferOffset + uiCount <= m_uiBufferSize, "Implementation error");

  nsGALCommandEncoder* pGALCommandEncoder = pRenderContext->GetCommandEncoder();

  nsUInt32 uiDestOffset = m_uiBufferOffset * sizeof(nsPerInstanceData);
  auto pSourceData = m_PerInstanceData.GetArrayPtr().GetSubArray(m_uiBufferOffset, uiCount);
  nsGALUpdateMode::Enum updateMode = (m_uiBufferOffset == 0) ? nsGALUpdateMode::Discard : nsGALUpdateMode::NoOverwrite;

  pGALCommandEncoder->UpdateBuffer(m_hInstanceDataBuffer, uiDestOffset, pSourceData.ToByteArray(), updateMode);


  nsObjectConstants* pConstants = pRenderContext->GetConstantBufferData<nsObjectConstants>(m_hConstantBuffer);
  pConstants->InstanceDataOffset = m_uiBufferOffset;

  m_uiBufferOffset += uiCount;
}

void nsInstanceData::CreateBuffer(nsUInt32 uiSize)
{
  m_uiBufferSize = uiSize;
  m_PerInstanceData.SetCountUninitialized(m_uiBufferSize);

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  nsGALBufferCreationDescription desc;
  desc.m_uiStructSize = sizeof(nsPerInstanceData);
  desc.m_uiTotalSize = desc.m_uiStructSize * uiSize;
  desc.m_BufferFlags = nsGALBufferUsageFlags::StructuredBuffer | nsGALBufferUsageFlags::ShaderResource;
  desc.m_ResourceAccess.m_bImmutable = false;

  m_hInstanceDataBuffer = pDevice->CreateBuffer(desc);
}

void nsInstanceData::Reset()
{
  m_uiBufferOffset = 0;
}

//////////////////////////////////////////////////////////////////////////

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsInstanceDataProvider, 1, nsRTTIDefaultAllocator<nsInstanceDataProvider>)
  {
  }
NS_END_DYNAMIC_REFLECTED_TYPE;

nsInstanceDataProvider::nsInstanceDataProvider() = default;

nsInstanceDataProvider::~nsInstanceDataProvider() = default;

void* nsInstanceDataProvider::UpdateData(const nsRenderViewContext& renderViewContext, const nsExtractedRenderData& extractedData)
{
  m_Data.Reset();

  return &m_Data;
}



NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_InstanceDataProvider);

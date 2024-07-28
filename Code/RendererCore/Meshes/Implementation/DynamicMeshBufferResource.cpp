#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Meshes/DynamicMeshBufferResource.h>
#include <RendererCore/Meshes/MeshBufferUtils.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Resources/Buffer.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsDynamicMeshBufferResource, 1, nsRTTIDefaultAllocator<nsDynamicMeshBufferResource>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_RESOURCE_IMPLEMENT_COMMON_CODE(nsDynamicMeshBufferResource);
// clang-format on

nsDynamicMeshBufferResource::nsDynamicMeshBufferResource()
  : nsResource(DoUpdate::OnAnyThread, 1)
{
}

nsDynamicMeshBufferResource::~nsDynamicMeshBufferResource()
{
  NS_ASSERT_DEBUG(m_hVertexBuffer.IsInvalidated(), "Implementation error");
  NS_ASSERT_DEBUG(m_hIndexBuffer.IsInvalidated(), "Implementation error");
  NS_ASSERT_DEBUG(m_hColorBuffer.IsInvalidated(), "Implementation error");
}

nsResourceLoadDesc nsDynamicMeshBufferResource::UnloadData(Unload WhatToUnload)
{
  if (!m_hVertexBuffer.IsInvalidated())
  {
    nsGALDevice::GetDefaultDevice()->DestroyBuffer(m_hVertexBuffer);
    m_hVertexBuffer.Invalidate();
  }

  if (!m_hIndexBuffer.IsInvalidated())
  {
    nsGALDevice::GetDefaultDevice()->DestroyBuffer(m_hIndexBuffer);
    m_hIndexBuffer.Invalidate();
  }

  if (!m_hColorBuffer.IsInvalidated())
  {
    nsGALDevice::GetDefaultDevice()->DestroyBuffer(m_hColorBuffer);
    m_hColorBuffer.Invalidate();
  }

  // we cannot compute this in UpdateMemoryUsage(), so we only read the data there, therefore we need to update this information here
  ModifyMemoryUsage().m_uiMemoryGPU = 0;

  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Unloaded;

  return res;
}

nsResourceLoadDesc nsDynamicMeshBufferResource::UpdateContent(nsStreamReader* Stream)
{
  NS_REPORT_FAILURE("This resource type does not support loading data from file.");

  return nsResourceLoadDesc();
}

void nsDynamicMeshBufferResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  // we cannot compute this data here, so we update it wherever we know the memory usage

  out_NewMemoryUsage.m_uiMemoryCPU = sizeof(nsDynamicMeshBufferResource) + m_VertexData.GetHeapMemoryUsage() + m_Index16Data.GetHeapMemoryUsage() + m_Index32Data.GetHeapMemoryUsage() + m_ColorData.GetHeapMemoryUsage();
  out_NewMemoryUsage.m_uiMemoryGPU = ModifyMemoryUsage().m_uiMemoryGPU;
}

NS_RESOURCE_IMPLEMENT_CREATEABLE(nsDynamicMeshBufferResource, nsDynamicMeshBufferResourceDescriptor)
{
  NS_ASSERT_DEBUG(m_hVertexBuffer.IsInvalidated(), "Implementation error");
  NS_ASSERT_DEBUG(m_hIndexBuffer.IsInvalidated(), "Implementation error");
  NS_ASSERT_DEBUG(m_hColorBuffer.IsInvalidated(), "Implementation error");

  m_Descriptor = descriptor;

  m_VertexData.SetCountUninitialized(m_Descriptor.m_uiMaxVertices);

  {
    nsVertexStreamInfo si;
    si.m_uiOffset = 0;
    si.m_Format = nsGALResourceFormat::XYZFloat;
    si.m_Semantic = nsGALVertexAttributeSemantic::Position;
    si.m_uiElementSize = sizeof(nsVec3);
    m_VertexDeclaration.m_VertexStreams.PushBack(si);

    si.m_uiOffset += si.m_uiElementSize;
    si.m_Format = nsGALResourceFormat::XYFloat;
    si.m_Semantic = nsGALVertexAttributeSemantic::TexCoord0;
    si.m_uiElementSize = sizeof(nsVec2);
    m_VertexDeclaration.m_VertexStreams.PushBack(si);

    si.m_uiOffset += si.m_uiElementSize;
    si.m_Format = nsGALResourceFormat::XYZFloat;
    si.m_Semantic = nsGALVertexAttributeSemantic::Normal;
    si.m_uiElementSize = sizeof(nsVec3);
    m_VertexDeclaration.m_VertexStreams.PushBack(si);

    si.m_uiOffset += si.m_uiElementSize;
    si.m_Format = nsGALResourceFormat::XYZWFloat;
    si.m_Semantic = nsGALVertexAttributeSemantic::Tangent;
    si.m_uiElementSize = sizeof(nsVec4);
    m_VertexDeclaration.m_VertexStreams.PushBack(si);

    if (m_Descriptor.m_bColorStream)
    {
      si.m_uiVertexBufferSlot = 1; // separate buffer
      si.m_uiOffset = 0;
      si.m_Format = nsGALResourceFormat::RGBAUByteNormalized;
      si.m_Semantic = nsGALVertexAttributeSemantic::Color0;
      si.m_uiElementSize = sizeof(nsColorLinearUB);
      m_VertexDeclaration.m_VertexStreams.PushBack(si);
    }

    m_VertexDeclaration.ComputeHash();
  }

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  m_hVertexBuffer = pDevice->CreateVertexBuffer(sizeof(nsDynamicMeshVertex), m_Descriptor.m_uiMaxVertices /* no initial data -> mutable */);

  nsStringBuilder sName;
  sName.SetFormat("{0} - Dynamic Vertex Buffer", GetResourceDescription());
  pDevice->GetBuffer(m_hVertexBuffer)->SetDebugName(sName);

  const nsUInt32 uiMaxIndices = nsGALPrimitiveTopology::VerticesPerPrimitive(m_Descriptor.m_Topology) * m_Descriptor.m_uiMaxPrimitives;

  if (m_Descriptor.m_bColorStream)
  {
    m_ColorData.SetCountUninitialized(uiMaxIndices);
    m_hColorBuffer = pDevice->CreateVertexBuffer(sizeof(nsColorLinearUB), m_Descriptor.m_uiMaxVertices /* no initial data -> mutable */);

    sName.SetFormat("{0} - Dynamic Color Buffer", GetResourceDescription());
    pDevice->GetBuffer(m_hColorBuffer)->SetDebugName(sName);
  }

  if (m_Descriptor.m_IndexType == nsGALIndexType::UInt)
  {
    m_Index32Data.SetCountUninitialized(uiMaxIndices);

    m_hIndexBuffer = pDevice->CreateIndexBuffer(nsGALIndexType::UInt, uiMaxIndices /* no initial data -> mutable */);

    sName.SetFormat("{0} - Dynamic Index32 Buffer", GetResourceDescription());
    pDevice->GetBuffer(m_hIndexBuffer)->SetDebugName(sName);
  }
  else if (m_Descriptor.m_IndexType == nsGALIndexType::UShort)
  {
    m_Index16Data.SetCountUninitialized(uiMaxIndices);

    m_hIndexBuffer = pDevice->CreateIndexBuffer(nsGALIndexType::UShort, uiMaxIndices /* no initial data -> mutable */);

    sName.SetFormat("{0} - Dynamic Index16 Buffer", GetResourceDescription());
    pDevice->GetBuffer(m_hIndexBuffer)->SetDebugName(sName);
  }

  // we only know the memory usage here, so we write it back to the internal variable directly and then read it in UpdateMemoryUsage() again
  ModifyMemoryUsage().m_uiMemoryGPU = m_VertexData.GetHeapMemoryUsage() + m_Index32Data.GetHeapMemoryUsage() + m_Index16Data.GetHeapMemoryUsage() + m_ColorData.GetHeapMemoryUsage();

  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Loaded;

  return res;
}

void nsDynamicMeshBufferResource::UpdateGpuBuffer(nsGALCommandEncoder* pGALCommandEncoder, nsUInt32 uiFirstVertex, nsUInt32 uiNumVertices, nsUInt32 uiFirstIndex, nsUInt32 uiNumIndices, nsGALUpdateMode::Enum mode /*= nsGALUpdateMode::Discard*/)
{
  if (m_bAccessedVB && uiNumVertices > 0)
  {
    if (uiNumVertices == nsMath::MaxValue<nsUInt32>())
      uiNumVertices = m_VertexData.GetCount() - uiFirstVertex;

    NS_ASSERT_DEV(uiNumVertices <= m_VertexData.GetCount(), "Can't upload {} vertices, the buffer was allocated to hold a maximum of {} vertices.", uiNumVertices, m_VertexData.GetCount());

    m_bAccessedVB = false;

    pGALCommandEncoder->UpdateBuffer(m_hVertexBuffer, sizeof(nsDynamicMeshVertex) * uiFirstVertex, m_VertexData.GetArrayPtr().GetSubArray(uiFirstVertex, uiNumVertices).ToByteArray(), mode);
  }

  if (m_bAccessedCB && uiNumVertices > 0)
  {
    if (uiNumVertices == nsMath::MaxValue<nsUInt32>())
      uiNumVertices = m_ColorData.GetCount() - uiFirstVertex;

    NS_ASSERT_DEV(uiNumVertices <= m_ColorData.GetCount(), "Can't upload {} vertices, the buffer was allocated to hold a maximum of {} vertices.", uiNumVertices, m_ColorData.GetCount());

    m_bAccessedCB = false;

    pGALCommandEncoder->UpdateBuffer(m_hColorBuffer, sizeof(nsColorLinearUB) * uiFirstVertex, m_ColorData.GetArrayPtr().GetSubArray(uiFirstVertex, uiNumVertices).ToByteArray(), mode);
  }

  if (m_bAccessedIB && uiNumIndices > 0 && !m_hIndexBuffer.IsInvalidated())
  {
    m_bAccessedIB = false;

    if (!m_Index16Data.IsEmpty())
    {
      NS_ASSERT_DEV(uiFirstIndex < m_Index16Data.GetCount(), "Invalid first index value {}", uiFirstIndex);

      if (uiNumIndices == nsMath::MaxValue<nsUInt32>())
        uiNumIndices = m_Index16Data.GetCount() - uiFirstIndex;

      NS_ASSERT_DEV(uiNumIndices <= m_Index16Data.GetCount(), "Can't upload {} indices, the buffer was allocated to hold a maximum of {} indices.", uiNumIndices, m_Index16Data.GetCount());

      pGALCommandEncoder->UpdateBuffer(m_hIndexBuffer, sizeof(nsUInt16) * uiFirstIndex, m_Index16Data.GetArrayPtr().GetSubArray(uiFirstIndex, uiNumIndices).ToByteArray(), mode);
    }
    else if (!m_Index32Data.IsEmpty())
    {
      NS_ASSERT_DEV(uiFirstIndex < m_Index32Data.GetCount(), "Invalid first index value {}", uiFirstIndex);

      if (uiNumIndices == nsMath::MaxValue<nsUInt32>())
        uiNumIndices = m_Index32Data.GetCount() - uiFirstIndex;

      NS_ASSERT_DEV(uiNumIndices <= m_Index32Data.GetCount(), "Can't upload {} indices, the buffer was allocated to hold a maximum of {} indices.", uiNumIndices, m_Index32Data.GetCount());

      pGALCommandEncoder->UpdateBuffer(m_hIndexBuffer, sizeof(nsUInt32) * uiFirstIndex, m_Index32Data.GetArrayPtr().GetSubArray(uiFirstIndex, uiNumIndices).ToByteArray(), mode);
    }
  }
}


NS_STATICLINK_FILE(RendererCore, RendererCore_Meshes_Implementation_DynamicMeshBufferResource);

#include <RendererCore/RendererCorePCH.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Meshes/SkinnedMeshComponent.h>
#include <RendererCore/RenderWorld/RenderWorld.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Shader/Types.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSkinnedMeshRenderData, 1, nsRTTIDefaultAllocator<nsSkinnedMeshRenderData>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void nsSkinnedMeshRenderData::FillBatchIdAndSortingKey()
{
  FillBatchIdAndSortingKeyInternal(m_hSkinningTransforms.GetInternalID().m_Data);
}

nsSkinningState::nsSkinningState() = default;

nsSkinningState::~nsSkinningState()
{
  Clear();
}

void nsSkinningState::Clear()
{
  if (!m_hGpuBuffer.IsInvalidated())
  {
    nsGALDevice::GetDefaultDevice()->DestroyBuffer(m_hGpuBuffer);
    m_hGpuBuffer.Invalidate();
  }

  m_bTransformsUpdated[0] = nullptr;
  m_bTransformsUpdated[1] = nullptr;
  m_Transforms.Clear();
}

void nsSkinningState::TransformsChanged()
{
  if (m_hGpuBuffer.IsInvalidated())
  {
    if (m_Transforms.GetCount() == 0)
      return;

    nsGALBufferCreationDescription BufferDesc;
    BufferDesc.m_uiStructSize = sizeof(nsShaderTransform);
    BufferDesc.m_uiTotalSize = BufferDesc.m_uiStructSize * m_Transforms.GetCount();
    BufferDesc.m_BufferFlags = nsGALBufferUsageFlags::StructuredBuffer | nsGALBufferUsageFlags::ShaderResource;
    BufferDesc.m_ResourceAccess.m_bImmutable = false;

    m_hGpuBuffer = nsGALDevice::GetDefaultDevice()->CreateBuffer(BufferDesc, m_Transforms.GetArrayPtr().ToByteArray());

    m_bTransformsUpdated[0] = std::make_shared<bool>(true);
    m_bTransformsUpdated[1] = std::make_shared<bool>(true);
  }
  else
  {
    const nsUInt32 uiRenIdx = nsRenderWorld::GetDataIndexForExtraction();
    *m_bTransformsUpdated[uiRenIdx] = false;
  }
}

void nsSkinningState::FillSkinnedMeshRenderData(nsSkinnedMeshRenderData& ref_renderData) const
{
  ref_renderData.m_hSkinningTransforms = m_hGpuBuffer;

  const nsUInt32 uiExIdx = nsRenderWorld::GetDataIndexForExtraction();

  if (m_bTransformsUpdated[uiExIdx] && *m_bTransformsUpdated[uiExIdx] == false)
  {
    auto pSkinningMatrices = NS_NEW_ARRAY(nsFrameAllocator::GetCurrentAllocator(), nsShaderTransform, m_Transforms.GetCount());
    pSkinningMatrices.CopyFrom(m_Transforms);

    ref_renderData.m_pNewSkinningTransformData = pSkinningMatrices.ToByteArray();
    ref_renderData.m_bTransformsUpdated = m_bTransformsUpdated[uiExIdx];
  }
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Meshes_Implementation_SkinnedMeshComponent);

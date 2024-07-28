#include <RendererCore/RendererCorePCH.h>

#include <Core/Graphics/Camera.h>
#include <RendererCore/Pipeline/RenderData.h>
#include <RendererCore/Pipeline/SortingFunctions.h>

namespace
{
  NS_ALWAYS_INLINE nsUInt32 CalculateTypeHash(const nsRenderData* pRenderData)
  {
    nsUInt32 uiTypeHash = nsHashingUtils::StringHashTo32(pRenderData->GetDynamicRTTI()->GetTypeNameHash());
    return (uiTypeHash >> 16) ^ (uiTypeHash & 0xFFFF);
  }

  NS_FORCE_INLINE nsUInt32 CalculateDistance(const nsRenderData* pRenderData, const nsCamera& camera)
  {
    ///\todo far-plane is not enough to normalize distance
    const float fDistance = (camera.GetPosition() - pRenderData->m_GlobalTransform.m_vPosition).GetLength() + pRenderData->m_fSortingDepthOffset;
    const float fNormalizedDistance = nsMath::Clamp(fDistance / camera.GetFarPlane(), 0.0f, 1.0f);
    return static_cast<nsUInt32>(fNormalizedDistance * 65535.0f);
  }
} // namespace

// static
nsUInt64 nsRenderSortingFunctions::ByRenderDataThenFrontToBack(const nsRenderData* pRenderData, const nsCamera& camera)
{
  const nsUInt64 uiTypeHash = CalculateTypeHash(pRenderData);
  const nsUInt64 uiRenderDataSortingKey64 = pRenderData->m_uiSortingKey;
  const nsUInt64 uiDistance = CalculateDistance(pRenderData, camera);

  const nsUInt64 uiSortingKey = (uiTypeHash << 48) | (uiRenderDataSortingKey64 << 16) | uiDistance;
  return uiSortingKey;
}

// static
nsUInt64 nsRenderSortingFunctions::BackToFrontThenByRenderData(const nsRenderData* pRenderData, const nsCamera& camera)
{
  const nsUInt64 uiTypeHash = CalculateTypeHash(pRenderData);
  const nsUInt64 uiRenderDataSortingKey64 = pRenderData->m_uiSortingKey;
  const nsUInt64 uiInvDistance = 0xFFFF - CalculateDistance(pRenderData, camera);

  const nsUInt64 uiSortingKey = (uiInvDistance << 48) | (uiTypeHash << 32) | uiRenderDataSortingKey64;
  return uiSortingKey;
}

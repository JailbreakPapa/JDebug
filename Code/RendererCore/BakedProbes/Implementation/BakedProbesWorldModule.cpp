#include <RendererCore/RendererCorePCH.h>

#include <Foundation/SimdMath/SimdConversion.h>
#include <Foundation/SimdMath/SimdVec4i.h>
#include <RendererCore/BakedProbes/BakedProbesWorldModule.h>
#include <RendererCore/BakedProbes/ProbeTreeSectorResource.h>

// clang-format off
NS_IMPLEMENT_WORLD_MODULE(nsBakedProbesWorldModule);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsBakedProbesWorldModule, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE
// clang-format on

nsBakedProbesWorldModule::nsBakedProbesWorldModule(nsWorld* pWorld)
  : nsWorldModule(pWorld)
{
}

nsBakedProbesWorldModule::~nsBakedProbesWorldModule() = default;

void nsBakedProbesWorldModule::Initialize()
{
}

void nsBakedProbesWorldModule::Deinitialize()
{
}

bool nsBakedProbesWorldModule::HasProbeData() const
{
  return m_hProbeTree.IsValid();
}

nsResult nsBakedProbesWorldModule::GetProbeIndexData(const nsVec3& vGlobalPosition, const nsVec3& vNormal, ProbeIndexData& out_probeIndexData) const
{
  // TODO: optimize

  if (!HasProbeData())
    return NS_FAILURE;

  nsResourceLock<nsProbeTreeSectorResource> pProbeTree(m_hProbeTree, nsResourceAcquireMode::BlockTillLoaded_NeverFail);
  if (pProbeTree.GetAcquireResult() != nsResourceAcquireResult::Final)
    return NS_FAILURE;

  nsSimdVec4f gridSpacePos = nsSimdConversion::ToVec3((vGlobalPosition - pProbeTree->GetGridOrigin()).CompDiv(pProbeTree->GetProbeSpacing()));
  gridSpacePos = gridSpacePos.CompMax(nsSimdVec4f::MakeZero());

  nsSimdVec4f gridSpacePosFloor = gridSpacePos.Floor();
  nsSimdVec4f weights = gridSpacePos - gridSpacePosFloor;

  nsSimdVec4i maxIndices = nsSimdVec4i(pProbeTree->GetProbeCount().x, pProbeTree->GetProbeCount().y, pProbeTree->GetProbeCount().z) - nsSimdVec4i(1);
  nsSimdVec4i pos0 = nsSimdVec4i::Truncate(gridSpacePosFloor).CompMin(maxIndices);
  nsSimdVec4i pos1 = (pos0 + nsSimdVec4i(1)).CompMin(maxIndices);

  nsUInt32 x0 = pos0.x();
  nsUInt32 y0 = pos0.y();
  nsUInt32 z0 = pos0.z();

  nsUInt32 x1 = pos1.x();
  nsUInt32 y1 = pos1.y();
  nsUInt32 z1 = pos1.z();

  nsUInt32 xCount = pProbeTree->GetProbeCount().x;
  nsUInt32 xyCount = xCount * pProbeTree->GetProbeCount().y;

  out_probeIndexData.m_probeIndices[0] = z0 * xyCount + y0 * xCount + x0;
  out_probeIndexData.m_probeIndices[1] = z0 * xyCount + y0 * xCount + x1;
  out_probeIndexData.m_probeIndices[2] = z0 * xyCount + y1 * xCount + x0;
  out_probeIndexData.m_probeIndices[3] = z0 * xyCount + y1 * xCount + x1;
  out_probeIndexData.m_probeIndices[4] = z1 * xyCount + y0 * xCount + x0;
  out_probeIndexData.m_probeIndices[5] = z1 * xyCount + y0 * xCount + x1;
  out_probeIndexData.m_probeIndices[6] = z1 * xyCount + y1 * xCount + x0;
  out_probeIndexData.m_probeIndices[7] = z1 * xyCount + y1 * xCount + x1;

  nsVec3 w1 = nsSimdConversion::ToVec3(weights);
  nsVec3 w0 = nsVec3(1.0f) - w1;

  // TODO: add geometry factor to weight
  out_probeIndexData.m_probeWeights[0] = w0.x * w0.y * w0.z;
  out_probeIndexData.m_probeWeights[1] = w1.x * w0.y * w0.z;
  out_probeIndexData.m_probeWeights[2] = w0.x * w1.y * w0.z;
  out_probeIndexData.m_probeWeights[3] = w1.x * w1.y * w0.z;
  out_probeIndexData.m_probeWeights[4] = w0.x * w0.y * w1.z;
  out_probeIndexData.m_probeWeights[5] = w1.x * w0.y * w1.z;
  out_probeIndexData.m_probeWeights[6] = w0.x * w1.y * w1.z;
  out_probeIndexData.m_probeWeights[7] = w1.x * w1.y * w1.z;

  float weightSum = 0;
  for (nsUInt32 i = 0; i < ProbeIndexData::NumProbes; ++i)
  {
    weightSum += out_probeIndexData.m_probeWeights[i];
  }

  float normalizeFactor = 1.0f / weightSum;
  for (nsUInt32 i = 0; i < ProbeIndexData::NumProbes; ++i)
  {
    out_probeIndexData.m_probeWeights[i] *= normalizeFactor;
  }

  return NS_SUCCESS;
}

nsAmbientCube<float> nsBakedProbesWorldModule::GetSkyVisibility(const ProbeIndexData& indexData) const
{
  // TODO: optimize

  nsAmbientCube<float> result;

  nsResourceLock<nsProbeTreeSectorResource> pProbeTree(m_hProbeTree, nsResourceAcquireMode::BlockTillLoaded_NeverFail);
  if (pProbeTree.GetAcquireResult() != nsResourceAcquireResult::Final)
    return result;

  auto compressedSkyVisibility = pProbeTree->GetSkyVisibility();
  nsAmbientCube<float> skyVisibility;

  for (nsUInt32 i = 0; i < ProbeIndexData::NumProbes; ++i)
  {
    nsBakingUtils::DecompressSkyVisibility(compressedSkyVisibility[indexData.m_probeIndices[i]], skyVisibility);

    for (nsUInt32 d = 0; d < nsAmbientCubeBasis::NumDirs; ++d)
    {
      result.m_Values[d] += skyVisibility.m_Values[d] * indexData.m_probeWeights[i];
    }
  }

  return result;
}

void nsBakedProbesWorldModule::SetProbeTreeResourcePrefix(const nsHashedString& prefix)
{
  nsStringBuilder sResourcePath;
  sResourcePath.SetFormat("{}_Global.nsProbeTreeSector", prefix);

  m_hProbeTree = nsResourceManager::LoadResource<nsProbeTreeSectorResource>(sResourcePath);
}


NS_STATICLINK_FILE(RendererCore, RendererCore_BakedProbes_Implementation_BakedProbesWorldModule);

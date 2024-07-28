#include <RendererCore/RendererCorePCH.h>

#include <Core/Graphics/Camera.h>
#include <Foundation/Configuration/CVar.h>
#include <Foundation/IO/TypeVersionContext.h>
#include <Foundation/Profiling/Profiling.h>
#include <RendererCore/Components/FogComponent.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Lights/AmbientLightComponent.h>
#include <RendererCore/Lights/ClusteredDataExtractor.h>
#include <RendererCore/Lights/Implementation/ClusteredDataUtils.h>
#include <RendererCore/Pipeline/ExtractedRenderData.h>
#include <RendererCore/Pipeline/View.h>

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
nsCVarBool cvar_RenderingLightingVisClusterData("Rendering.Lighting.VisClusterData", false, nsCVarFlags::Default, "Enables debug visualization of clustered light data");
nsCVarInt cvar_RenderingLightingVisClusterDepthSlice("Rendering.Lighting.VisClusterDepthSlice", -1, nsCVarFlags::Default, "Show the debug visualization only for the given depth slice");

namespace
{
  void VisualizeClusteredData(const nsView& view, const nsClusteredDataCPU* pData, nsArrayPtr<nsSimdBSphere> boundingSpheres)
  {
    if (!cvar_RenderingLightingVisClusterData)
      return;

    const nsCamera* pCamera = view.GetCullingCamera();

    if (pCamera->IsOrthographic())
      return;

    float fAspectRatio = view.GetViewport().width / view.GetViewport().height;

    nsMat4 mProj;
    pCamera->GetProjectionMatrix(fAspectRatio, mProj);

    nsAngle fFovLeft;
    nsAngle fFovRight;
    nsAngle fFovBottom;
    nsAngle fFovTop;
    nsGraphicsUtils::ExtractPerspectiveMatrixFieldOfView(mProj, fFovLeft, fFovRight, fFovBottom, fFovTop);

    const float fTanLeft = nsMath::Tan(fFovLeft);
    const float fTanRight = nsMath::Tan(fFovRight);
    const float fTanBottom = nsMath::Tan(fFovBottom);
    const float fTanTop = nsMath::Tan(fFovTop);

    nsColor lineColor = nsColor(1.0f, 1.0f, 1.0f, 0.1f);

    nsInt32 debugSlice = cvar_RenderingLightingVisClusterDepthSlice;
    nsUInt32 maxSlice = debugSlice < 0 ? NUM_CLUSTERS_Z : debugSlice + 1;
    nsUInt32 minSlice = debugSlice < 0 ? 0 : debugSlice;

    bool bDrawBoundingSphere = false;

    for (nsUInt32 z = maxSlice; z-- > minSlice;)
    {
      float fZf = GetDepthFromSliceIndex(z);
      float fZn = (z > 0) ? GetDepthFromSliceIndex(z - 1) : 0.0f;
      for (nsInt32 y = 0; y < NUM_CLUSTERS_Y; ++y)
      {
        for (nsInt32 x = 0; x < NUM_CLUSTERS_X; ++x)
        {
          nsUInt32 clusterIndex = GetClusterIndexFromCoord(x, y, z);
          auto& clusterData = pData->m_ClusterData[clusterIndex];

          if (clusterData.counts > 0)
          {
            if (bDrawBoundingSphere)
            {
              nsBoundingSphere s = nsSimdConversion::ToBSphere(boundingSpheres[clusterIndex]);
              nsDebugRenderer::DrawLineSphere(view.GetHandle(), s, lineColor);
            }

            nsVec3 cc[8];
            GetClusterCornerPoints(*pCamera, fZf, fZn, fTanLeft, fTanRight, fTanBottom, fTanTop, x, y, z, cc);

            float lightCount = (float)GET_LIGHT_INDEX(clusterData.counts);
            float decalCount = (float)GET_DECAL_INDEX(clusterData.counts);
            float r = nsMath::Clamp(lightCount / 16.0f, 0.0f, 1.0f);
            float g = nsMath::Clamp(decalCount / 16.0f, 0.0f, 1.0f);

            nsDebugRenderer::Triangle tris[12];
            // back
            tris[0] = nsDebugRenderer::Triangle(cc[0], cc[2], cc[1]);
            tris[1] = nsDebugRenderer::Triangle(cc[2], cc[3], cc[1]);
            // front
            tris[2] = nsDebugRenderer::Triangle(cc[4], cc[5], cc[6]);
            tris[3] = nsDebugRenderer::Triangle(cc[6], cc[5], cc[7]);
            // top
            tris[4] = nsDebugRenderer::Triangle(cc[4], cc[0], cc[5]);
            tris[5] = nsDebugRenderer::Triangle(cc[0], cc[1], cc[5]);
            // bottom
            tris[6] = nsDebugRenderer::Triangle(cc[6], cc[7], cc[2]);
            tris[7] = nsDebugRenderer::Triangle(cc[2], cc[7], cc[3]);
            // left
            tris[8] = nsDebugRenderer::Triangle(cc[4], cc[6], cc[0]);
            tris[9] = nsDebugRenderer::Triangle(cc[0], cc[6], cc[2]);
            // right
            tris[10] = nsDebugRenderer::Triangle(cc[5], cc[1], cc[7]);
            tris[11] = nsDebugRenderer::Triangle(cc[1], cc[3], cc[7]);

            nsDebugRenderer::DrawSolidTriangles(view.GetHandle(), tris, nsColor(r, g, 0.0f, 0.1f));

            nsDebugRenderer::Line lines[12];
            lines[0] = nsDebugRenderer::Line(cc[4], cc[5]);
            lines[1] = nsDebugRenderer::Line(cc[5], cc[7]);
            lines[2] = nsDebugRenderer::Line(cc[7], cc[6]);
            lines[3] = nsDebugRenderer::Line(cc[6], cc[4]);

            lines[4] = nsDebugRenderer::Line(cc[0], cc[1]);
            lines[5] = nsDebugRenderer::Line(cc[1], cc[3]);
            lines[6] = nsDebugRenderer::Line(cc[3], cc[2]);
            lines[7] = nsDebugRenderer::Line(cc[2], cc[0]);

            lines[8] = nsDebugRenderer::Line(cc[4], cc[0]);
            lines[9] = nsDebugRenderer::Line(cc[5], cc[1]);
            lines[10] = nsDebugRenderer::Line(cc[7], cc[3]);
            lines[11] = nsDebugRenderer::Line(cc[6], cc[2]);

            nsDebugRenderer::DrawLines(view.GetHandle(), lines, nsColor(r, g, 0.0f));
          }
        }
      }

      {
        nsVec3 leftWidth = pCamera->GetDirRight() * fZf * fTanLeft;
        nsVec3 rightWidth = pCamera->GetDirRight() * fZf * fTanRight;
        nsVec3 bottomHeight = pCamera->GetDirUp() * fZf * fTanBottom;
        nsVec3 topHeight = pCamera->GetDirUp() * fZf * fTanTop;

        nsVec3 depthFar = pCamera->GetPosition() + pCamera->GetDirForwards() * fZf;
        nsVec3 p0 = depthFar + rightWidth + topHeight;
        nsVec3 p1 = depthFar + rightWidth + bottomHeight;
        nsVec3 p2 = depthFar + leftWidth + bottomHeight;
        nsVec3 p3 = depthFar + leftWidth + topHeight;

        nsDebugRenderer::Line lines[4];
        lines[0] = nsDebugRenderer::Line(p0, p1);
        lines[1] = nsDebugRenderer::Line(p1, p2);
        lines[2] = nsDebugRenderer::Line(p2, p3);
        lines[3] = nsDebugRenderer::Line(p3, p0);

        nsDebugRenderer::DrawLines(view.GetHandle(), lines, lineColor);
      }
    }
  }
} // namespace
#endif

//////////////////////////////////////////////////////////////////////////

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsClusteredDataCPU, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

nsClusteredDataCPU::nsClusteredDataCPU() = default;
nsClusteredDataCPU::~nsClusteredDataCPU() = default;

//////////////////////////////////////////////////////////////////////////

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsClusteredDataExtractor, 1, nsRTTIDefaultAllocator<nsClusteredDataExtractor>)
NS_END_DYNAMIC_REFLECTED_TYPE;

nsClusteredDataExtractor::nsClusteredDataExtractor(const char* szName)
  : nsExtractor(szName)
{
  m_DependsOn.PushBack(nsMakeHashedString("nsVisibleObjectsExtractor"));

  m_TempLightsClusters.SetCountUninitialized(NUM_CLUSTERS);
  m_TempDecalsClusters.SetCountUninitialized(NUM_CLUSTERS);
  m_TempReflectionProbeClusters.SetCountUninitialized(NUM_CLUSTERS);
  m_ClusterBoundingSpheres.SetCountUninitialized(NUM_CLUSTERS);
}

nsClusteredDataExtractor::~nsClusteredDataExtractor() = default;

void nsClusteredDataExtractor::PostSortAndBatch(
  const nsView& view, const nsDynamicArray<const nsGameObject*>& visibleObjects, nsExtractedRenderData& ref_extractedRenderData)
{
  NS_PROFILE_SCOPE("PostSortAndBatch");

  const nsCamera* pCamera = view.GetCullingCamera();
  const float fAspectRatio = view.GetViewport().width / view.GetViewport().height;

  FillClusterBoundingSpheres(*pCamera, fAspectRatio, m_ClusterBoundingSpheres);
  nsClusteredDataCPU* pData = NS_NEW(nsFrameAllocator::GetCurrentAllocator(), nsClusteredDataCPU);
  pData->m_ClusterData = NS_NEW_ARRAY(nsFrameAllocator::GetCurrentAllocator(), nsPerClusterData, NUM_CLUSTERS);

  nsMat4 tmp = pCamera->GetViewMatrix();
  nsSimdMat4f viewMatrix = nsSimdConversion::ToMat4(tmp);

  pCamera->GetProjectionMatrix(fAspectRatio, tmp);
  nsSimdMat4f projectionMatrix = nsSimdConversion::ToMat4(tmp);

  nsSimdMat4f viewProjectionMatrix = projectionMatrix * viewMatrix;

  // Lights
  {
    NS_PROFILE_SCOPE("Lights");
    m_TempLightData.Clear();
    nsMemoryUtils::ZeroFill(m_TempLightsClusters.GetData(), NUM_CLUSTERS);

    auto batchList = ref_extractedRenderData.GetRenderDataBatchesWithCategory(nsDefaultRenderDataCategories::Light);
    const nsUInt32 uiBatchCount = batchList.GetBatchCount();
    for (nsUInt32 i = 0; i < uiBatchCount; ++i)
    {
      const nsRenderDataBatch& batch = batchList.GetBatch(i);

      for (auto it = batch.GetIterator<nsRenderData>(); it.IsValid(); ++it)
      {
        const nsUInt32 uiLightIndex = m_TempLightData.GetCount();

        if (uiLightIndex == nsClusteredDataCPU::MAX_LIGHT_DATA)
        {
          nsLog::Warning("Maximum number of lights reached ({0}). Further lights will be discarded.", nsClusteredDataCPU::MAX_LIGHT_DATA);
          break;
        }

        if (auto pPointLightRenderData = nsDynamicCast<const nsPointLightRenderData*>(it))
        {
          FillPointLightData(m_TempLightData.ExpandAndGetRef(), pPointLightRenderData);

          nsSimdBSphere pointLightSphere =
            nsSimdBSphere(nsSimdConversion::ToVec3(pPointLightRenderData->m_GlobalTransform.m_vPosition), pPointLightRenderData->m_fRange);
          RasterizeSphere(
            pointLightSphere, uiLightIndex, viewMatrix, projectionMatrix, m_TempLightsClusters.GetData(), m_ClusterBoundingSpheres.GetData());

          if (false)
          {
            nsSimdBBox ssb = GetScreenSpaceBounds(pointLightSphere, viewMatrix, projectionMatrix);
            float minX = ((float)ssb.m_Min.x() * 0.5f + 0.5f) * view.GetViewport().width;
            float maxX = ((float)ssb.m_Max.x() * 0.5f + 0.5f) * view.GetViewport().width;
            float minY = ((float)ssb.m_Max.y() * -0.5f + 0.5f) * view.GetViewport().height;
            float maxY = ((float)ssb.m_Min.y() * -0.5f + 0.5f) * view.GetViewport().height;

            nsRectFloat rect(minX, minY, maxX - minX, maxY - minY);
            nsDebugRenderer::Draw2DRectangle(view.GetHandle(), rect, 0.0f, nsColor::Blue.WithAlpha(0.3f));
          }
        }
        else if (auto pSpotLightRenderData = nsDynamicCast<const nsSpotLightRenderData*>(it))
        {
          FillSpotLightData(m_TempLightData.ExpandAndGetRef(), pSpotLightRenderData);

          nsAngle halfAngle = pSpotLightRenderData->m_OuterSpotAngle / 2.0f;

          BoundingCone cone;
          cone.m_PositionAndRange = nsSimdConversion::ToVec3(pSpotLightRenderData->m_GlobalTransform.m_vPosition);
          cone.m_PositionAndRange.SetW(pSpotLightRenderData->m_fRange);
          cone.m_ForwardDir = nsSimdConversion::ToVec3(pSpotLightRenderData->m_GlobalTransform.m_qRotation * nsVec3(1.0f, 0.0f, 0.0f));
          cone.m_SinCosAngle = nsSimdVec4f(nsMath::Sin(halfAngle), nsMath::Cos(halfAngle), 0.0f);
          RasterizeSpotLight(cone, uiLightIndex, viewMatrix, projectionMatrix, m_TempLightsClusters.GetData(), m_ClusterBoundingSpheres.GetData());
        }
        else if (auto pDirLightRenderData = nsDynamicCast<const nsDirectionalLightRenderData*>(it))
        {
          FillDirLightData(m_TempLightData.ExpandAndGetRef(), pDirLightRenderData);

          RasterizeDirLight(pDirLightRenderData, uiLightIndex, m_TempLightsClusters.GetArrayPtr());
        }
        else if (auto pFogRenderData = nsDynamicCast<const nsFogRenderData*>(it))
        {
          float fogBaseHeight = pFogRenderData->m_GlobalTransform.m_vPosition.z;
          float fogHeightFalloff = pFogRenderData->m_fHeightFalloff > 0.0f ? nsMath::Ln(0.0001f) / pFogRenderData->m_fHeightFalloff : 0.0f;

          float fogAtCameraPos = fogHeightFalloff * (pCamera->GetPosition().z - fogBaseHeight);
          if (fogAtCameraPos >= 80.0f) // Prevent infs
          {
            fogHeightFalloff = 0.0f;
          }

          pData->m_fFogHeight = -fogHeightFalloff * fogBaseHeight;
          pData->m_fFogHeightFalloff = fogHeightFalloff;
          pData->m_fFogDensityAtCameraPos = nsMath::Exp(nsMath::Clamp(fogAtCameraPos, -80.0f, 80.0f)); // Prevent infs
          pData->m_fFogDensity = pFogRenderData->m_fDensity;
          pData->m_fFogInvSkyDistance = pFogRenderData->m_fInvSkyDistance;

          pData->m_FogColor = pFogRenderData->m_Color;
        }
        else
        {
          nsLog::Warning("Unhandled render data type '{}' in 'Light' category", it->GetDynamicRTTI()->GetTypeName());
        }
      }
    }

    pData->m_LightData = NS_NEW_ARRAY(nsFrameAllocator::GetCurrentAllocator(), nsPerLightData, m_TempLightData.GetCount());
    pData->m_LightData.CopyFrom(m_TempLightData);

    pData->m_uiSkyIrradianceIndex = view.GetWorld()->GetIndex();
    pData->m_cameraUsageHint = view.GetCameraUsageHint();
  }

  // Decals
  {
    NS_PROFILE_SCOPE("Decals");
    m_TempDecalData.Clear();
    nsMemoryUtils::ZeroFill(m_TempDecalsClusters.GetData(), NUM_CLUSTERS);

    auto batchList = ref_extractedRenderData.GetRenderDataBatchesWithCategory(nsDefaultRenderDataCategories::Decal);
    const nsUInt32 uiBatchCount = batchList.GetBatchCount();
    for (nsUInt32 i = 0; i < uiBatchCount; ++i)
    {
      const nsRenderDataBatch& batch = batchList.GetBatch(i);

      for (auto it = batch.GetIterator<nsRenderData>(); it.IsValid(); ++it)
      {
        const nsUInt32 uiDecalIndex = m_TempDecalData.GetCount();

        if (uiDecalIndex == nsClusteredDataCPU::MAX_DECAL_DATA)
        {
          nsLog::Warning("Maximum number of decals reached ({0}). Further decals will be discarded.", nsClusteredDataCPU::MAX_DECAL_DATA);
          break;
        }

        if (auto pDecalRenderData = nsDynamicCast<const nsDecalRenderData*>(it))
        {
          FillDecalData(m_TempDecalData.ExpandAndGetRef(), pDecalRenderData);

          RasterizeBox(pDecalRenderData->m_GlobalTransform, uiDecalIndex, viewProjectionMatrix, m_TempDecalsClusters.GetData(), m_ClusterBoundingSpheres.GetData());
        }
        else
        {
          nsLog::Warning("Unhandled render data type '{}' in 'Decal' category", it->GetDynamicRTTI()->GetTypeName());
        }
      }
    }

    pData->m_DecalData = NS_NEW_ARRAY(nsFrameAllocator::GetCurrentAllocator(), nsPerDecalData, m_TempDecalData.GetCount());
    pData->m_DecalData.CopyFrom(m_TempDecalData);
  }

  // Reflection Probes
  {
    NS_PROFILE_SCOPE("Probes");
    m_TempReflectionProbeData.Clear();
    nsMemoryUtils::ZeroFill(m_TempReflectionProbeClusters.GetData(), NUM_CLUSTERS);

    auto batchList = ref_extractedRenderData.GetRenderDataBatchesWithCategory(nsDefaultRenderDataCategories::ReflectionProbe);
    const nsUInt32 uiBatchCount = batchList.GetBatchCount();
    for (nsUInt32 i = 0; i < uiBatchCount; ++i)
    {
      const nsRenderDataBatch& batch = batchList.GetBatch(i);

      for (auto it = batch.GetIterator<nsRenderData>(); it.IsValid(); ++it)
      {
        const nsUInt32 uiProbeIndex = m_TempReflectionProbeData.GetCount();

        if (uiProbeIndex == nsClusteredDataCPU::MAX_REFLECTION_PROBE_DATA)
        {
          nsLog::Warning("Maximum number of reflection probes reached ({0}). Further reflection probes will be discarded.", nsClusteredDataCPU::MAX_REFLECTION_PROBE_DATA);
          break;
        }

        if (auto pReflectionProbeRenderData = nsDynamicCast<const nsReflectionProbeRenderData*>(it))
        {
          auto& probeData = m_TempReflectionProbeData.ExpandAndGetRef();
          FillReflectionProbeData(probeData, pReflectionProbeRenderData);

          const nsVec3 vFullScale = pReflectionProbeRenderData->m_vHalfExtents.CompMul(pReflectionProbeRenderData->m_GlobalTransform.m_vScale);

          bool bRasterizeSphere = false;
          float fMaxRadius = 0.0f;
          if (pReflectionProbeRenderData->m_uiIndex & REFLECTION_PROBE_IS_SPHERE)
          {
            constexpr float fSphereConstant = (4.0f / 3.0f) * nsMath::Pi<float>();
            fMaxRadius = nsMath::Max(nsMath::Max(nsMath::Abs(vFullScale.x), nsMath::Abs(vFullScale.y)), nsMath::Abs(vFullScale.z));
            const float fSphereVolume = fSphereConstant * nsMath::Pow(fMaxRadius, 3.0f);
            const float fBoxVolume = nsMath::Abs(vFullScale.x * vFullScale.y * vFullScale.z * 8);
            if (fSphereVolume < fBoxVolume)
            {
              bRasterizeSphere = true;
            }
          }

          if (bRasterizeSphere)
          {
            nsSimdBSphere pointLightSphere =
              nsSimdBSphere(nsSimdConversion::ToVec3(pReflectionProbeRenderData->m_GlobalTransform.m_vPosition), fMaxRadius);
            RasterizeSphere(
              pointLightSphere, uiProbeIndex, viewMatrix, projectionMatrix, m_TempReflectionProbeClusters.GetData(), m_ClusterBoundingSpheres.GetData());
          }
          else
          {
            nsTransform transform = pReflectionProbeRenderData->m_GlobalTransform;
            transform.m_vScale = vFullScale.CompMul(probeData.InfluenceScale.GetAsVec3());
            transform.m_vPosition += transform.m_qRotation * vFullScale.CompMul(probeData.InfluenceShift.GetAsVec3());

            // const nsBoundingBox aabb(nsVec3(-1.0f), nsVec3(1.0f));
            // nsDebugRenderer::DrawLineBox(view.GetHandle(), aabb, nsColor::DarkBlue, transform);

            RasterizeBox(transform, uiProbeIndex, viewProjectionMatrix, m_TempReflectionProbeClusters.GetData(), m_ClusterBoundingSpheres.GetData());
          }
        }
        else
        {
          nsLog::Warning("Unhandled render data type '{}' in 'ReflectionProbe' category", it->GetDynamicRTTI()->GetTypeName());
        }
      }
    }

    pData->m_ReflectionProbeData = NS_NEW_ARRAY(nsFrameAllocator::GetCurrentAllocator(), nsPerReflectionProbeData, m_TempReflectionProbeData.GetCount());
    pData->m_ReflectionProbeData.CopyFrom(m_TempReflectionProbeData);
  }

  FillItemListAndClusterData(pData);

  ref_extractedRenderData.AddFrameData(pData);

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  VisualizeClusteredData(view, pData, m_ClusterBoundingSpheres);
#endif
}

nsResult nsClusteredDataExtractor::Serialize(nsStreamWriter& inout_stream) const
{
  NS_SUCCEED_OR_RETURN(SUPER::Serialize(inout_stream));
  return NS_SUCCESS;
}


nsResult nsClusteredDataExtractor::Deserialize(nsStreamReader& inout_stream)
{
  NS_SUCCEED_OR_RETURN(SUPER::Deserialize(inout_stream));
  const nsUInt32 uiVersion = nsTypeVersionReadContext::GetContext()->GetTypeVersion(GetStaticRTTI());
  NS_IGNORE_UNUSED(uiVersion);
  return NS_SUCCESS;
}

namespace
{
  nsUInt32 PackIndex(nsUInt32 uiLightIndex, nsUInt32 uiDecalIndex)
  {
    return uiDecalIndex << 10 | uiLightIndex;
  }

  nsUInt32 PackReflectionProbeIndex(nsUInt32 uiData, nsUInt32 uiReflectionProbeIndex)
  {
    return uiReflectionProbeIndex << 20 | uiData;
  }
} // namespace

void nsClusteredDataExtractor::FillItemListAndClusterData(nsClusteredDataCPU* pData)
{
  NS_PROFILE_SCOPE("FillItemListAndClusterData");
  m_TempClusterItemList.Clear();

  const nsUInt32 uiNumLights = m_TempLightData.GetCount();
  const nsUInt32 uiMaxLightBlockIndex = (uiNumLights + 31) / 32;

  const nsUInt32 uiNumDecals = m_TempDecalData.GetCount();
  const nsUInt32 uiMaxDecalBlockIndex = (uiNumDecals + 31) / 32;

  const nsUInt32 uiNumReflectionProbes = m_TempReflectionProbeData.GetCount();
  const nsUInt32 uiMaxReflectionProbeBlockIndex = (uiNumReflectionProbes + 31) / 32;

  const nsUInt32 uiWorstCase = nsMath::Max(uiNumLights, uiNumDecals, uiNumReflectionProbes);
  for (nsUInt32 i = 0; i < NUM_CLUSTERS; ++i)
  {
    const nsUInt32 uiOffset = m_TempClusterItemList.GetCount();
    nsUInt32 uiLightCount = 0;

    // We expand m_TempClusterItemList by the worst case this loop can produce and then cut it down again to the actual size once we have filled the data. This makes sure we do not waste time on boundary checks or potential out of line calls like PushBack or PushBackUnchecked.
    m_TempClusterItemList.SetCountUninitialized(uiOffset + uiWorstCase);
    nsUInt32* pTempClusterItemListRange = m_TempClusterItemList.GetData() + uiOffset;

    // Lights
    {
      auto& tempCluster = m_TempLightsClusters[i];
      for (nsUInt32 uiBlockIndex = 0; uiBlockIndex < uiMaxLightBlockIndex; ++uiBlockIndex)
      {
        nsUInt32 mask = tempCluster.m_BitMask[uiBlockIndex];

        while (mask > 0)
        {
          nsUInt32 uiLightIndex = nsMath::FirstBitLow(mask);
          mask &= ~(1 << uiLightIndex);

          uiLightIndex += uiBlockIndex * 32;
          pTempClusterItemListRange[uiLightCount] = uiLightIndex;
          ++uiLightCount;
        }
      }
    }

    nsUInt32 uiDecalCount = 0;

    // Decals
    {
      auto& tempCluster = m_TempDecalsClusters[i];
      for (nsUInt32 uiBlockIndex = 0; uiBlockIndex < uiMaxDecalBlockIndex; ++uiBlockIndex)
      {
        nsUInt32 mask = tempCluster.m_BitMask[uiBlockIndex];

        while (mask > 0)
        {
          nsUInt32 uiDecalIndex = nsMath::FirstBitLow(mask);
          mask &= ~(1 << uiDecalIndex);

          uiDecalIndex += uiBlockIndex * 32;

          if (uiDecalCount < uiLightCount)
          {
            auto& item = pTempClusterItemListRange[uiDecalCount];
            item = PackIndex(item, uiDecalIndex);
          }
          else
          {
            pTempClusterItemListRange[uiDecalCount] = PackIndex(0, uiDecalIndex);
          }

          ++uiDecalCount;
        }
      }
    }

    nsUInt32 uiReflectionProbeCount = 0;
    const nsUInt32 uiMaxUsed = nsMath::Max(uiLightCount, uiDecalCount);
    // Reflection Probes
    {
      auto& tempCluster = m_TempReflectionProbeClusters[i];
      for (nsUInt32 uiBlockIndex = 0; uiBlockIndex < uiMaxReflectionProbeBlockIndex; ++uiBlockIndex)
      {
        nsUInt32 mask = tempCluster.m_BitMask[uiBlockIndex];

        while (mask > 0)
        {
          nsUInt32 uiReflectionProbeIndex = nsMath::FirstBitLow(mask);
          mask &= ~(1 << uiReflectionProbeIndex);

          uiReflectionProbeIndex += uiBlockIndex * 32;

          if (uiReflectionProbeCount < uiMaxUsed)
          {
            auto& item = pTempClusterItemListRange[uiReflectionProbeCount];
            item = PackReflectionProbeIndex(item, uiReflectionProbeIndex);
          }
          else
          {
            pTempClusterItemListRange[uiReflectionProbeCount] = PackReflectionProbeIndex(0, uiReflectionProbeIndex);
          }

          ++uiReflectionProbeCount;
        }
      }
    }

    // Cut down the array to the actual number of elements we have written.
    const nsUInt32 uiActualCase = nsMath::Max(uiLightCount, uiDecalCount, uiReflectionProbeCount);
    m_TempClusterItemList.SetCountUninitialized(uiOffset + uiActualCase);

    auto& clusterData = pData->m_ClusterData[i];
    clusterData.offset = uiOffset;
    clusterData.counts = PackReflectionProbeIndex(PackIndex(uiLightCount, uiDecalCount), uiReflectionProbeCount);
  }

  pData->m_ClusterItemList = NS_NEW_ARRAY(nsFrameAllocator::GetCurrentAllocator(), nsUInt32, m_TempClusterItemList.GetCount());
  pData->m_ClusterItemList.CopyFrom(m_TempClusterItemList);
}



NS_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_ClusteredDataExtractor);

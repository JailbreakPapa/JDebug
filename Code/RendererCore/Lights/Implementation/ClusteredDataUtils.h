#pragma once

#include <RendererCore/Decals/DecalComponent.h>
#include <RendererCore/Lights/DirectionalLightComponent.h>
#include <RendererCore/Lights/Implementation/ReflectionProbeData.h>
#include <RendererCore/Lights/PointLightComponent.h>
#include <RendererCore/Lights/SpotLightComponent.h>
#include <RendererFoundation/Shader/ShaderUtils.h>

#include <RendererCore/../../../Data/Base/Shaders/Common/LightData.h>
NS_DEFINE_AS_POD_TYPE(nsPerLightData);
NS_DEFINE_AS_POD_TYPE(nsPerDecalData);
NS_DEFINE_AS_POD_TYPE(nsPerReflectionProbeData);
NS_DEFINE_AS_POD_TYPE(nsPerClusterData);

#include <Core/Graphics/Camera.h>
#include <Foundation/Math/Float16.h>
#include <Foundation/SimdMath/SimdConversion.h>
#include <Foundation/SimdMath/SimdVec4i.h>
#include <Foundation/Utilities/GraphicsUtils.h>

namespace
{
  ///\todo Make this configurable.
  static float s_fMinLightDistance = 5.0f;
  static float s_fMaxLightDistance = 500.0f;

  static float s_fDepthSliceScale = (NUM_CLUSTERS_Z - 1) / (nsMath::Log2(s_fMaxLightDistance) - nsMath::Log2(s_fMinLightDistance));
  static float s_fDepthSliceBias = -s_fDepthSliceScale * nsMath::Log2(s_fMinLightDistance) + 1.0f;

  NS_ALWAYS_INLINE float GetDepthFromSliceIndex(nsUInt32 uiSliceIndex)
  {
    return nsMath::Pow(2.0f, (uiSliceIndex - s_fDepthSliceBias + 1.0f) / s_fDepthSliceScale);
  }

  NS_ALWAYS_INLINE nsUInt32 GetSliceIndexFromDepth(float fLinearDepth)
  {
    return nsMath::Clamp((nsInt32)(nsMath::Log2(fLinearDepth) * s_fDepthSliceScale + s_fDepthSliceBias), 0, NUM_CLUSTERS_Z - 1);
  }

  NS_ALWAYS_INLINE nsUInt32 GetClusterIndexFromCoord(nsUInt32 x, nsUInt32 y, nsUInt32 z)
  {
    return z * NUM_CLUSTERS_XY + y * NUM_CLUSTERS_X + x;
  }

  // in order: tlf, trf, blf, brf, tln, trn, bln, brn
  NS_FORCE_INLINE void GetClusterCornerPoints(
    const nsCamera& camera, float fZf, float fZn, float fTanLeft, float fTanRight, float fTanBottom, float fTanTop, nsInt32 x, nsInt32 y, nsInt32 z, nsVec3* out_pCorners)
  {
    const nsVec3& pos = camera.GetPosition();
    const nsVec3& dirForward = camera.GetDirForwards();
    const nsVec3& dirRight = camera.GetDirRight();
    const nsVec3& dirUp = camera.GetDirUp();

    const float fStartXf = fZf * fTanLeft;
    const float fStartYf = fZf * fTanBottom;
    const float fEndXf = fZf * fTanRight;
    const float fEndYf = fZf * fTanTop;

    float fStepXf = (fEndXf - fStartXf) / NUM_CLUSTERS_X;
    float fStepYf = (fEndYf - fStartYf) / NUM_CLUSTERS_Y;

    float fXf = fStartXf + x * fStepXf;
    float fYf = fStartYf + y * fStepYf;

    out_pCorners[0] = pos + dirForward * fZf + dirRight * fXf - dirUp * fYf;
    out_pCorners[1] = out_pCorners[0] + dirRight * fStepXf;
    out_pCorners[2] = out_pCorners[0] - dirUp * fStepYf;
    out_pCorners[3] = out_pCorners[2] + dirRight * fStepXf;

    const float fStartXn = fZn * fTanLeft;
    const float fStartYn = fZn * fTanBottom;
    const float fEndXn = fZn * fTanRight;
    const float fEndYn = fZn * fTanTop;

    float fStepXn = (fEndXn - fStartXn) / NUM_CLUSTERS_X;
    float fStepYn = (fEndYn - fStartYn) / NUM_CLUSTERS_Y;
    float fXn = fStartXn + x * fStepXn;
    float fYn = fStartYn + y * fStepYn;

    out_pCorners[4] = pos + dirForward * fZn + dirRight * fXn - dirUp * fYn;
    out_pCorners[5] = out_pCorners[4] + dirRight * fStepXn;
    out_pCorners[6] = out_pCorners[4] - dirUp * fStepYn;
    out_pCorners[7] = out_pCorners[6] + dirRight * fStepXn;
  }

  void FillClusterBoundingSpheres(const nsCamera& camera, float fAspectRatio, nsArrayPtr<nsSimdBSphere> clusterBoundingSpheres)
  {
    NS_PROFILE_SCOPE("FillClusterBoundingSpheres");

    ///\todo proper implementation for orthographic views
    if (camera.IsOrthographic())
      return;

    nsMat4 mProj;
    camera.GetProjectionMatrix(fAspectRatio, mProj);

    nsSimdVec4f stepScale;
    nsSimdVec4f tanLBLB;
    {
      nsAngle fFovLeft;
      nsAngle fFovRight;
      nsAngle fFovBottom;
      nsAngle fFovTop;
      nsGraphicsUtils::ExtractPerspectiveMatrixFieldOfView(mProj, fFovLeft, fFovRight, fFovBottom, fFovTop);

      const float fTanLeft = nsMath::Tan(fFovLeft);
      const float fTanRight = nsMath::Tan(fFovRight);
      const float fTanBottom = nsMath::Tan(fFovBottom);
      const float fTanTop = nsMath::Tan(fFovTop);

      float fStepXf = (fTanRight - fTanLeft) / NUM_CLUSTERS_X;
      float fStepYf = (fTanTop - fTanBottom) / NUM_CLUSTERS_Y;

      stepScale = nsSimdVec4f(fStepXf, fStepYf, fStepXf, fStepYf);
      tanLBLB = nsSimdVec4f(fTanLeft, fTanBottom, fTanLeft, fTanBottom);
    }

    nsSimdVec4f pos = nsSimdConversion::ToVec3(camera.GetPosition());
    nsSimdVec4f dirForward = nsSimdConversion::ToVec3(camera.GetDirForwards());
    nsSimdVec4f dirRight = nsSimdConversion::ToVec3(camera.GetDirRight());
    nsSimdVec4f dirUp = nsSimdConversion::ToVec3(camera.GetDirUp());


    nsSimdVec4f fZn = nsSimdVec4f::MakeZero();
    nsSimdVec4f cc[8];

    for (nsInt32 z = 0; z < NUM_CLUSTERS_Z; z++)
    {
      nsSimdVec4f fZf = nsSimdVec4f(GetDepthFromSliceIndex(z));
      nsSimdVec4f zff_znn = fZf.GetCombined<nsSwizzle::XXXX>(fZn);
      nsSimdVec4f steps = zff_znn.CompMul(stepScale);

      nsSimdVec4f depthF = pos + dirForward * fZf.x();
      nsSimdVec4f depthN = pos + dirForward * fZn.x();

      nsSimdVec4f startLBLB = zff_znn.CompMul(tanLBLB);

      for (nsInt32 y = 0; y < NUM_CLUSTERS_Y; y++)
      {
        for (nsInt32 x = 0; x < NUM_CLUSTERS_X; x++)
        {
          nsSimdVec4f xyxy = nsSimdVec4i(x, y, x, y).ToFloat();
          nsSimdVec4f xfyf = startLBLB + (xyxy).CompMul(steps);

          cc[0] = depthF + dirRight * xfyf.x() - dirUp * xfyf.y();
          cc[1] = cc[0] + dirRight * steps.x();
          cc[2] = cc[0] - dirUp * steps.y();
          cc[3] = cc[2] + dirRight * steps.x();

          cc[4] = depthN + dirRight * xfyf.z() - dirUp * xfyf.w();
          cc[5] = cc[4] + dirRight * steps.z();
          cc[6] = cc[4] - dirUp * steps.w();
          cc[7] = cc[6] + dirRight * steps.z();

          clusterBoundingSpheres[GetClusterIndexFromCoord(x, y, z)] = nsSimdBSphere::MakeFromPoints(cc, 8);
        }
      }

      fZn = fZf;
    }
  }

  NS_ALWAYS_INLINE void FillLightData(nsPerLightData& ref_perLightData, const nsLightRenderData* pLightRenderData, nsUInt8 uiType)
  {
    nsMemoryUtils::ZeroFill(&ref_perLightData, 1);

    nsColorLinearUB lightColor = pLightRenderData->m_LightColor;
    lightColor.a = uiType;

    ref_perLightData.colorAndType = *reinterpret_cast<nsUInt32*>(&lightColor.r);
    ref_perLightData.intensity = pLightRenderData->m_fIntensity;
    ref_perLightData.specularMultiplier = pLightRenderData->m_fSpecularMultiplier;
    ref_perLightData.shadowDataOffset = pLightRenderData->m_uiShadowDataOffset;
  }

  void FillPointLightData(nsPerLightData& ref_perLightData, const nsPointLightRenderData* pPointLightRenderData)
  {
    FillLightData(ref_perLightData, pPointLightRenderData, LIGHT_TYPE_POINT);

    ref_perLightData.position = pPointLightRenderData->m_GlobalTransform.m_vPosition;
    ref_perLightData.invSqrAttRadius = 1.0f / (pPointLightRenderData->m_fRange * pPointLightRenderData->m_fRange);
  }

  void FillSpotLightData(nsPerLightData& ref_perLightData, const nsSpotLightRenderData* pSpotLightRenderData)
  {
    FillLightData(ref_perLightData, pSpotLightRenderData, LIGHT_TYPE_SPOT);

    ref_perLightData.direction = nsShaderUtils::Float3ToRGB10(pSpotLightRenderData->m_GlobalTransform.m_qRotation * nsVec3(-1, 0, 0));
    ref_perLightData.position = pSpotLightRenderData->m_GlobalTransform.m_vPosition;
    ref_perLightData.invSqrAttRadius = 1.0f / (pSpotLightRenderData->m_fRange * pSpotLightRenderData->m_fRange);

    const float fCosInner = nsMath::Cos(pSpotLightRenderData->m_InnerSpotAngle * 0.5f);
    const float fCosOuter = nsMath::Cos(pSpotLightRenderData->m_OuterSpotAngle * 0.5f);
    const float fSpotParamScale = 1.0f / nsMath::Max(0.001f, (fCosInner - fCosOuter));
    const float fSpotParamOffset = -fCosOuter * fSpotParamScale;
    ref_perLightData.spotParams = nsShaderUtils::Float2ToRG16F(nsVec2(fSpotParamScale, fSpotParamOffset));
  }

  void FillDirLightData(nsPerLightData& ref_perLightData, const nsDirectionalLightRenderData* pDirLightRenderData)
  {
    FillLightData(ref_perLightData, pDirLightRenderData, LIGHT_TYPE_DIR);

    ref_perLightData.direction = nsShaderUtils::Float3ToRGB10(pDirLightRenderData->m_GlobalTransform.m_qRotation * nsVec3(-1, 0, 0));
  }

  void FillDecalData(nsPerDecalData& ref_perDecalData, const nsDecalRenderData* pDecalRenderData)
  {
    nsVec3 position = pDecalRenderData->m_GlobalTransform.m_vPosition;
    nsVec3 dirForwards = pDecalRenderData->m_GlobalTransform.m_qRotation * nsVec3(1.0f, 0.0, 0.0f);
    nsVec3 dirUp = pDecalRenderData->m_GlobalTransform.m_qRotation * nsVec3(0.0f, 0.0, 1.0f);
    nsVec3 scale = pDecalRenderData->m_GlobalTransform.m_vScale;

    // the CompMax prevents division by zero (thus inf, thus NaN later, then crash)
    // if negative scaling should be allowed, this would need to be changed
    scale = nsVec3(1.0f).CompDiv(scale.CompMax(nsVec3(0.00001f)));

    const nsMat4 lookAt = nsGraphicsUtils::CreateLookAtViewMatrix(position, position + dirForwards, dirUp);
    nsMat4 scaleMat = nsMat4::MakeScaling(nsVec3(scale.y, -scale.z, scale.x));

    ref_perDecalData.worldToDecalMatrix = scaleMat * lookAt;
    ref_perDecalData.applyOnlyToId = pDecalRenderData->m_uiApplyOnlyToId;
    ref_perDecalData.decalFlags = pDecalRenderData->m_uiFlags;
    ref_perDecalData.angleFadeParams = pDecalRenderData->m_uiAngleFadeParams;
    ref_perDecalData.baseColor = *reinterpret_cast<const nsUInt32*>(&pDecalRenderData->m_BaseColor.r);
    ref_perDecalData.emissiveColorRG = nsShaderUtils::PackFloat16intoUint(pDecalRenderData->m_EmissiveColor.r, pDecalRenderData->m_EmissiveColor.g);
    ref_perDecalData.emissiveColorBA = nsShaderUtils::PackFloat16intoUint(pDecalRenderData->m_EmissiveColor.b, pDecalRenderData->m_EmissiveColor.a);
    ref_perDecalData.baseColorAtlasScale = pDecalRenderData->m_uiBaseColorAtlasScale;
    ref_perDecalData.baseColorAtlasOffset = pDecalRenderData->m_uiBaseColorAtlasOffset;
    ref_perDecalData.normalAtlasScale = pDecalRenderData->m_uiNormalAtlasScale;
    ref_perDecalData.normalAtlasOffset = pDecalRenderData->m_uiNormalAtlasOffset;
    ref_perDecalData.ormAtlasScale = pDecalRenderData->m_uiORMAtlasScale;
    ref_perDecalData.ormAtlasOffset = pDecalRenderData->m_uiORMAtlasOffset;
  }

  void FillReflectionProbeData(nsPerReflectionProbeData& ref_perReflectionProbeData, const nsReflectionProbeRenderData* pReflectionProbeRenderData)
  {
    nsVec3 position = pReflectionProbeRenderData->m_GlobalTransform.m_vPosition;
    nsVec3 scale = pReflectionProbeRenderData->m_GlobalTransform.m_vScale.CompMul(pReflectionProbeRenderData->m_vHalfExtents);

    // We store scale separately so we easily transform into probe projection space (with scale), influence space (scale + offset) and cube map space (no scale).
    auto trans = pReflectionProbeRenderData->m_GlobalTransform;
    trans.m_vScale = nsVec3(1.0f, 1.0f, 1.0f);
    auto inverse = trans.GetAsMat4().GetInverse();

    // the CompMax prevents division by zero (thus inf, thus NaN later, then crash)
    // if negative scaling should be allowed, this would need to be changed
    scale = nsVec3(1.0f).CompDiv(scale.CompMax(nsVec3(0.00001f)));
    ref_perReflectionProbeData.WorldToProbeProjectionMatrix = inverse;

    ref_perReflectionProbeData.ProbePosition = pReflectionProbeRenderData->m_vProbePosition.GetAsVec4(1.0f); // W isn't used.
    ref_perReflectionProbeData.Scale = scale.GetAsVec4(0.0f);                                                // W isn't used.

    ref_perReflectionProbeData.InfluenceScale = pReflectionProbeRenderData->m_vInfluenceScale.GetAsVec4(0.0f);
    ref_perReflectionProbeData.InfluenceShift = pReflectionProbeRenderData->m_vInfluenceShift.CompMul(nsVec3(1.0f) - pReflectionProbeRenderData->m_vInfluenceScale).GetAsVec4(0.0f);

    ref_perReflectionProbeData.PositiveFalloff = pReflectionProbeRenderData->m_vPositiveFalloff.GetAsVec4(0.0f);
    ref_perReflectionProbeData.NegativeFalloff = pReflectionProbeRenderData->m_vNegativeFalloff.GetAsVec4(0.0f);
    ref_perReflectionProbeData.Index = pReflectionProbeRenderData->m_uiIndex;
  }


  NS_FORCE_INLINE nsSimdBBox GetScreenSpaceBounds(const nsSimdBSphere& sphere, const nsSimdMat4f& mViewMatrix, const nsSimdMat4f& mProjectionMatrix)
  {
    nsSimdVec4f viewSpaceCenter = mViewMatrix.TransformPosition(sphere.GetCenter());
    nsSimdFloat depth = viewSpaceCenter.z();
    nsSimdFloat radius = sphere.GetRadius();

    nsSimdVec4f mi;
    nsSimdVec4f ma;

    if (viewSpaceCenter.GetLength<3>() > radius && depth > radius)
    {
      nsSimdVec4f one = nsSimdVec4f(1.0f);
      nsSimdVec4f oneNegOne = nsSimdVec4f(1.0f, -1.0f, 1.0f, -1.0f);

      nsSimdVec4f pRadius = nsSimdVec4f(radius / depth);
      nsSimdVec4f pRadius2 = pRadius.CompMul(pRadius);

      nsSimdVec4f xy = viewSpaceCenter / depth;
      nsSimdVec4f xxyy = xy.Get<nsSwizzle::XXYY>();
      nsSimdVec4f nom = (pRadius2.CompMul(xxyy.CompMul(xxyy) - pRadius2 + one)).GetSqrt() - xxyy.CompMul(oneNegOne);
      nsSimdVec4f denom = pRadius2 - one;

      nsSimdVec4f projection = mProjectionMatrix.m_col0.GetCombined<nsSwizzle::XXYY>(mProjectionMatrix.m_col1);
      nsSimdVec4f minXmaxX_minYmaxY = nom.CompDiv(denom).CompMul(oneNegOne).CompMul(projection);

      mi = minXmaxX_minYmaxY.Get<nsSwizzle::XZXX>();
      ma = minXmaxX_minYmaxY.Get<nsSwizzle::YWYY>();
    }
    else
    {
      mi = nsSimdVec4f(-1.0f);
      ma = nsSimdVec4f(1.0f);
    }

    mi.SetZ(depth - radius);
    ma.SetZ(depth + radius);

    return nsSimdBBox(mi, ma);
  }

  template <typename Cluster, typename IntersectionFunc>
  NS_FORCE_INLINE void FillCluster(const nsSimdBBox& screenSpaceBounds, nsUInt32 uiBlockIndex, nsUInt32 uiMask, Cluster* pClusters, IntersectionFunc func)
  {
    nsSimdVec4f scale = nsSimdVec4f(0.5f * NUM_CLUSTERS_X, -0.5f * NUM_CLUSTERS_Y, 1.0f, 1.0f);
    nsSimdVec4f bias = nsSimdVec4f(0.5f * NUM_CLUSTERS_X, 0.5f * NUM_CLUSTERS_Y, 0.0f, 0.0f);

    nsSimdVec4f mi = nsSimdVec4f::MulAdd(screenSpaceBounds.m_Min, scale, bias);
    nsSimdVec4f ma = nsSimdVec4f::MulAdd(screenSpaceBounds.m_Max, scale, bias);

    nsSimdVec4i minXY_maxXY = nsSimdVec4i::Truncate(mi.GetCombined<nsSwizzle::XYXY>(ma));

    nsSimdVec4i maxClusterIndex = nsSimdVec4i(NUM_CLUSTERS_X, NUM_CLUSTERS_Y, NUM_CLUSTERS_X, NUM_CLUSTERS_Y);
    minXY_maxXY = minXY_maxXY.CompMin(maxClusterIndex - nsSimdVec4i(1));
    minXY_maxXY = minXY_maxXY.CompMax(nsSimdVec4i::MakeZero());

    nsUInt32 xMin = minXY_maxXY.x();
    nsUInt32 yMin = minXY_maxXY.w();

    nsUInt32 xMax = minXY_maxXY.z();
    nsUInt32 yMax = minXY_maxXY.y();

    nsUInt32 zMin = GetSliceIndexFromDepth(screenSpaceBounds.m_Min.z());
    nsUInt32 zMax = GetSliceIndexFromDepth(screenSpaceBounds.m_Max.z());

    for (nsUInt32 z = zMin; z <= zMax; ++z)
    {
      for (nsUInt32 y = yMin; y <= yMax; ++y)
      {
        for (nsUInt32 x = xMin; x <= xMax; ++x)
        {
          nsUInt32 uiClusterIndex = GetClusterIndexFromCoord(x, y, z);
          if (func(uiClusterIndex))
          {
            pClusters[uiClusterIndex].m_BitMask[uiBlockIndex] |= uiMask;
          }
        }
      }
    }
  }

  template <typename Cluster>
  void RasterizeSphere(const nsSimdBSphere& pointLightSphere, nsUInt32 uiLightIndex, const nsSimdMat4f& mViewMatrix,
    const nsSimdMat4f& mProjectionMatrix, Cluster* pClusters, nsSimdBSphere* pClusterBoundingSpheres)
  {
    nsSimdBBox screenSpaceBounds = GetScreenSpaceBounds(pointLightSphere, mViewMatrix, mProjectionMatrix);

    const nsUInt32 uiBlockIndex = uiLightIndex / 32;
    const nsUInt32 uiMask = 1 << (uiLightIndex - uiBlockIndex * 32);

    FillCluster(screenSpaceBounds, uiBlockIndex, uiMask, pClusters,
      [&](nsUInt32 uiClusterIndex)
      { return pointLightSphere.Overlaps(pClusterBoundingSpheres[uiClusterIndex]); });
  }

  struct BoundingCone
  {
    nsSimdBSphere m_BoundingSphere;
    nsSimdVec4f m_PositionAndRange;
    nsSimdVec4f m_ForwardDir;
    nsSimdVec4f m_SinCosAngle;
  };

  template <typename Cluster>
  void RasterizeSpotLight(const BoundingCone& spotLightCone, nsUInt32 uiLightIndex, const nsSimdMat4f& mViewMatrix,
    const nsSimdMat4f& mProjectionMatrix, Cluster* pClusters, nsSimdBSphere* pClusterBoundingSpheres)
  {
    nsSimdVec4f position = spotLightCone.m_PositionAndRange;
    nsSimdFloat range = spotLightCone.m_PositionAndRange.w();
    nsSimdVec4f forwardDir = spotLightCone.m_ForwardDir;
    nsSimdFloat sinAngle = spotLightCone.m_SinCosAngle.x();
    nsSimdFloat cosAngle = spotLightCone.m_SinCosAngle.y();

    // First calculate a bounding sphere around the cone to get min and max bounds
    nsSimdVec4f bSphereCenter;
    nsSimdFloat bSphereRadius;
    if (sinAngle > 0.707107f) // sin(45)
    {
      bSphereCenter = position + forwardDir * cosAngle * range;
      bSphereRadius = sinAngle * range;
    }
    else
    {
      bSphereRadius = range / (cosAngle + cosAngle);
      bSphereCenter = position + forwardDir * bSphereRadius;
    }

    nsSimdBSphere spotLightSphere(bSphereCenter, bSphereRadius);
    nsSimdBBox screenSpaceBounds = GetScreenSpaceBounds(spotLightSphere, mViewMatrix, mProjectionMatrix);

    const nsUInt32 uiBlockIndex = uiLightIndex / 32;
    const nsUInt32 uiMask = 1 << (uiLightIndex - uiBlockIndex * 32);

    FillCluster(screenSpaceBounds, uiBlockIndex, uiMask, pClusters, [&](nsUInt32 uiClusterIndex)
      {
      nsSimdBSphere clusterSphere = pClusterBoundingSpheres[uiClusterIndex];
      nsSimdFloat clusterRadius = clusterSphere.GetRadius();

      nsSimdVec4f toConePos = clusterSphere.m_CenterAndRadius - position;
      nsSimdFloat projected = forwardDir.Dot<3>(toConePos);
      nsSimdFloat distToConeSq = toConePos.Dot<3>(toConePos);
      nsSimdFloat distClosestP = cosAngle * (distToConeSq - projected * projected).GetSqrt() - projected * sinAngle;

      bool angleCull = distClosestP > clusterRadius;
      bool frontCull = projected > clusterRadius + range;
      bool backCull = projected < -clusterRadius;

      return !(angleCull || frontCull || backCull); });
  }

  template <typename Cluster>
  void RasterizeDirLight(const nsDirectionalLightRenderData* pDirLightRenderData, nsUInt32 uiLightIndex, nsArrayPtr<Cluster> clusters)
  {
    const nsUInt32 uiBlockIndex = uiLightIndex / 32;
    const nsUInt32 uiMask = 1 << (uiLightIndex - uiBlockIndex * 32);

    for (nsUInt32 i = 0; i < clusters.GetCount(); ++i)
    {
      clusters[i].m_BitMask[uiBlockIndex] |= uiMask;
    }
  }

  template <typename Cluster>
  void RasterizeBox(const nsTransform& transform, nsUInt32 uiDecalIndex, const nsSimdMat4f& mViewProjectionMatrix, Cluster* pClusters,
    nsSimdBSphere* pClusterBoundingSpheres)
  {
    nsSimdMat4f decalToWorld = nsSimdConversion::ToTransform(transform).GetAsMat4();
    nsSimdMat4f worldToDecal = decalToWorld.GetInverse();

    nsVec3 corners[8];
    nsBoundingBox::MakeFromMinMax(nsVec3(-1), nsVec3(1)).GetCorners(corners);

    nsSimdMat4f decalToScreen = mViewProjectionMatrix * decalToWorld;
    nsSimdBBox screenSpaceBounds = nsSimdBBox::MakeInvalid();
    bool bInsideBox = false;
    for (nsUInt32 i = 0; i < 8; ++i)
    {
      nsSimdVec4f corner = nsSimdConversion::ToVec3(corners[i]);
      nsSimdVec4f screenSpaceCorner = decalToScreen.TransformPosition(corner);
      nsSimdFloat depth = screenSpaceCorner.w();
      bInsideBox |= depth < nsSimdFloat::MakeZero();

      screenSpaceCorner /= depth;
      screenSpaceCorner = screenSpaceCorner.GetCombined<nsSwizzle::XYZW>(nsSimdVec4f(depth));

      screenSpaceBounds.m_Min = screenSpaceBounds.m_Min.CompMin(screenSpaceCorner);
      screenSpaceBounds.m_Max = screenSpaceBounds.m_Max.CompMax(screenSpaceCorner);
    }

    if (bInsideBox)
    {
      screenSpaceBounds.m_Min = nsSimdVec4f(-1.0f).GetCombined<nsSwizzle::XYZW>(screenSpaceBounds.m_Min);
      screenSpaceBounds.m_Max = nsSimdVec4f(1.0f).GetCombined<nsSwizzle::XYZW>(screenSpaceBounds.m_Max);
    }

    nsSimdVec4f decalHalfExtents = nsSimdVec4f(1.0f);
    nsSimdBBox localDecalBounds = nsSimdBBox(-decalHalfExtents, decalHalfExtents);

    const nsUInt32 uiBlockIndex = uiDecalIndex / 32;
    const nsUInt32 uiMask = 1 << (uiDecalIndex - uiBlockIndex * 32);

    FillCluster(screenSpaceBounds, uiBlockIndex, uiMask, pClusters, [&](nsUInt32 uiClusterIndex)
      {
      nsSimdBSphere clusterSphere = pClusterBoundingSpheres[uiClusterIndex];
      clusterSphere.Transform(worldToDecal);

      return localDecalBounds.Overlaps(clusterSphere); });
  }
} // namespace

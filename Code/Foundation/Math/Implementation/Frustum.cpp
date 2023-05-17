#include <Foundation/FoundationPCH.h>

#include <Foundation/Math/Frustum.h>
#include <Foundation/SimdMath/SimdBBox.h>
#include <Foundation/SimdMath/SimdConversion.h>
#include <Foundation/SimdMath/SimdVec4f.h>
#include <Foundation/Utilities/GraphicsUtils.h>

wdFrustum::wdFrustum() = default;
wdFrustum::~wdFrustum() = default;

const wdPlane& wdFrustum::GetPlane(wdUInt8 uiPlane) const
{
  WD_ASSERT_DEBUG(uiPlane < PLANE_COUNT, "Invalid plane index.");

  return m_Planes[uiPlane];
}

wdPlane& wdFrustum::AccessPlane(wdUInt8 uiPlane)
{
  WD_ASSERT_DEBUG(uiPlane < PLANE_COUNT, "Invalid plane index.");

  return m_Planes[uiPlane];
}

bool wdFrustum::IsValid() const
{
  for (wdUInt32 i = 0; i < PLANE_COUNT; ++i)
  {
    if (!m_Planes[i].IsValid())
      return false;
  }

  return true;
}

void wdFrustum::SetFrustum(const wdPlane* pPlanes)
{
  for (wdUInt32 i = 0; i < PLANE_COUNT; ++i)
    m_Planes[i] = pPlanes[i];
}

void wdFrustum::TransformFrustum(const wdMat4& mTransform)
{
  for (wdUInt32 i = 0; i < PLANE_COUNT; ++i)
    m_Planes[i].Transform(mTransform);
}

wdVolumePosition::Enum wdFrustum::GetObjectPosition(const wdVec3* pVertices, wdUInt32 uiNumVertices) const
{
  /// \test Not yet tested

  bool bOnSomePlane = false;

  for (wdUInt32 i = 0; i < PLANE_COUNT; ++i)
  {
    const wdPositionOnPlane::Enum pos = m_Planes[i].GetObjectPosition(pVertices, uiNumVertices);

    if (pos == wdPositionOnPlane::Back)
      continue;

    if (pos == wdPositionOnPlane::Front)
      return wdVolumePosition::Outside;

    bOnSomePlane = true;
  }

  if (bOnSomePlane)
    return wdVolumePosition::Intersecting;

  return wdVolumePosition::Inside;
}

static wdPositionOnPlane::Enum GetPlaneObjectPosition(const wdPlane& p, const wdVec3* const pPoints, wdUInt32 uiVertices, const wdMat4& mTransform)
{
  bool bFront = false;
  bool bBack = false;

  for (wdUInt32 i = 0; i < uiVertices; ++i)
  {
    switch (p.GetPointPosition(mTransform * pPoints[i]))
    {
      case wdPositionOnPlane::Front:
      {
        if (bBack)
          return wdPositionOnPlane::Spanning;

        bFront = true;
      }
      break;

      case wdPositionOnPlane::Back:
      {
        if (bFront)
          return (wdPositionOnPlane::Spanning);

        bBack = true;
      }
      break;

      default:
        break;
    }
  }

  return (bFront ? wdPositionOnPlane::Front : wdPositionOnPlane::Back);
}


wdVolumePosition::Enum wdFrustum::GetObjectPosition(const wdVec3* pVertices, wdUInt32 uiNumVertices, const wdMat4& mObjectTransform) const
{
  /// \test Not yet tested

  bool bOnSomePlane = false;

  for (wdUInt32 i = 0; i < PLANE_COUNT; ++i)
  {
    const wdPositionOnPlane::Enum pos = GetPlaneObjectPosition(m_Planes[i], pVertices, uiNumVertices, mObjectTransform);

    if (pos == wdPositionOnPlane::Back)
      continue;

    if (pos == wdPositionOnPlane::Front)
      return wdVolumePosition::Outside;

    bOnSomePlane = true;
  }

  if (bOnSomePlane)
    return wdVolumePosition::Intersecting;

  return wdVolumePosition::Inside;
}

wdVolumePosition::Enum wdFrustum::GetObjectPosition(const wdBoundingSphere& sphere) const
{
  /// \test Not yet tested

  bool bOnSomePlane = false;

  for (wdUInt32 i = 0; i < PLANE_COUNT; ++i)
  {
    const wdPositionOnPlane::Enum pos = m_Planes[i].GetObjectPosition(sphere);

    if (pos == wdPositionOnPlane::Back)
      continue;

    if (pos == wdPositionOnPlane::Front)
      return wdVolumePosition::Outside;

    bOnSomePlane = true;
  }

  if (bOnSomePlane)
    return wdVolumePosition::Intersecting;

  return wdVolumePosition::Inside;
}

wdVolumePosition::Enum wdFrustum::GetObjectPosition(const wdBoundingBox& box) const
{
  /// \test Not yet tested

  bool bOnSomePlane = false;

  for (wdUInt32 i = 0; i < PLANE_COUNT; ++i)
  {
    const wdPositionOnPlane::Enum pos = m_Planes[i].GetObjectPosition(box);

    if (pos == wdPositionOnPlane::Back)
      continue;

    if (pos == wdPositionOnPlane::Front)
      return wdVolumePosition::Outside;

    bOnSomePlane = true;
  }

  if (bOnSomePlane)
    return wdVolumePosition::Intersecting;

  return wdVolumePosition::Inside;
}

void wdFrustum::InvertFrustum()
{
  for (wdUInt32 i = 0; i < PLANE_COUNT; ++i)
    m_Planes[i].Flip();
}

void wdFrustum::ComputeCornerPoints(wdVec3 out_pPoints[FrustumCorner::CORNER_COUNT]) const
{
  // clang-format off
  wdPlane::GetPlanesIntersectionPoint(m_Planes[NearPlane], m_Planes[TopPlane], m_Planes[LeftPlane], out_pPoints[FrustumCorner::NearTopLeft]).IgnoreResult();
  wdPlane::GetPlanesIntersectionPoint(m_Planes[NearPlane], m_Planes[TopPlane], m_Planes[RightPlane], out_pPoints[FrustumCorner::NearTopRight]).IgnoreResult();
  wdPlane::GetPlanesIntersectionPoint(m_Planes[NearPlane], m_Planes[BottomPlane], m_Planes[LeftPlane], out_pPoints[FrustumCorner::NearBottomLeft]).IgnoreResult();
  wdPlane::GetPlanesIntersectionPoint(m_Planes[NearPlane], m_Planes[BottomPlane], m_Planes[RightPlane], out_pPoints[FrustumCorner::NearBottomRight]).IgnoreResult();

  wdPlane::GetPlanesIntersectionPoint(m_Planes[FarPlane], m_Planes[TopPlane], m_Planes[LeftPlane], out_pPoints[FrustumCorner::FarTopLeft]).IgnoreResult();
  wdPlane::GetPlanesIntersectionPoint(m_Planes[FarPlane], m_Planes[TopPlane], m_Planes[RightPlane], out_pPoints[FrustumCorner::FarTopRight]).IgnoreResult();
  wdPlane::GetPlanesIntersectionPoint(m_Planes[FarPlane], m_Planes[BottomPlane], m_Planes[LeftPlane], out_pPoints[FrustumCorner::FarBottomLeft]).IgnoreResult();
  wdPlane::GetPlanesIntersectionPoint(m_Planes[FarPlane], m_Planes[BottomPlane], m_Planes[RightPlane], out_pPoints[FrustumCorner::FarBottomRight]).IgnoreResult();
  // clang-format on
}

void wdFrustum::SetFrustum(const wdMat4& mModelViewProjection0, wdClipSpaceDepthRange::Enum depthRange, wdHandedness::Enum handedness)
{
  wdMat4 ModelViewProjection = mModelViewProjection0;
  wdGraphicsUtils::ConvertProjectionMatrixDepthRange(ModelViewProjection, depthRange, wdClipSpaceDepthRange::MinusOneToOne);

  wdVec4 planes[6];

  if (handedness == wdHandedness::LeftHanded)
  {
    ModelViewProjection.SetRow(0, -ModelViewProjection.GetRow(0));
  }

  planes[LeftPlane] = -ModelViewProjection.GetRow(3) - ModelViewProjection.GetRow(0);
  planes[RightPlane] = -ModelViewProjection.GetRow(3) + ModelViewProjection.GetRow(0);
  planes[BottomPlane] = -ModelViewProjection.GetRow(3) - ModelViewProjection.GetRow(1);
  planes[TopPlane] = -ModelViewProjection.GetRow(3) + ModelViewProjection.GetRow(1);
  planes[NearPlane] = -ModelViewProjection.GetRow(3) - ModelViewProjection.GetRow(2);
  planes[FarPlane] = -ModelViewProjection.GetRow(3) + ModelViewProjection.GetRow(2);

  // Normalize planes
  for (int p = 0; p < 6; ++p)
  {
    const float len = planes[p].GetAsVec3().GetLength();
    // doing the division here manually since we want to accept the case where length is 0 (infinite plane)
    const float invLen = 1.f / len;
    planes[p].x *= wdMath::IsFinite(invLen) ? invLen : 0.f;
    planes[p].y *= wdMath::IsFinite(invLen) ? invLen : 0.f;
    planes[p].z *= wdMath::IsFinite(invLen) ? invLen : 0.f;
    planes[p].w *= invLen;
  }

  // The last matrix row is giving the camera's plane, which means its normal is
  // also the camera's viewing direction.
  const wdVec3 cameraViewDirection = ModelViewProjection.GetRow(3).GetAsVec3();

  // Making sure the near/far plane is always closest/farthest. The way we derive the
  // planes always yields the closer plane pointing towards the camera and the farther
  // plane pointing away from the camera, so flip when that relationship inverts.
  if (planes[FarPlane].GetAsVec3().Dot(cameraViewDirection) < 0)
  {
    WD_ASSERT_DEBUG(planes[NearPlane].GetAsVec3().Dot(cameraViewDirection) >= 0, "");
    wdMath::Swap(planes[NearPlane], planes[FarPlane]);
  }

  // In case we have an infinity far plane projection, the normal is invalid.
  // We'll just take the mirrored normal from the near plane.
  WD_ASSERT_DEBUG(planes[NearPlane].IsValid(), "Near plane is expected to be non-nan and finite at this point!");
  if (wdMath::Abs(planes[FarPlane].w) == wdMath::Infinity<float>())
  {
    planes[FarPlane] = (-planes[NearPlane].GetAsVec3()).GetAsVec4(planes[FarPlane].w);
  }

  static_assert(offsetof(wdPlane, m_vNormal) == offsetof(wdVec4, x) && offsetof(wdPlane, m_fNegDistance) == offsetof(wdVec4, w));
  wdMemoryUtils::Copy(m_Planes, (wdPlane*)planes, 6);
}

void wdFrustum::SetFrustum(const wdVec3& vPosition, const wdVec3& vForwards, const wdVec3& vUp, wdAngle fovX, wdAngle fovY, float fNearPlane, float fFarPlane)
{
  WD_ASSERT_DEBUG(wdMath::Abs(vForwards.GetNormalized().Dot(vUp.GetNormalized())) < 0.999f, "Up dir must be different from forward direction");

  const wdVec3 vForwardsNorm = vForwards.GetNormalized();
  const wdVec3 vRightNorm = vForwards.CrossRH(vUp).GetNormalized();
  const wdVec3 vUpNorm = vRightNorm.CrossRH(vForwards).GetNormalized();

  // Near Plane
  m_Planes[NearPlane].SetFromNormalAndPoint(-vForwardsNorm, vPosition + fNearPlane * vForwardsNorm);

  // Far Plane
  m_Planes[FarPlane].SetFromNormalAndPoint(vForwardsNorm, vPosition + fFarPlane * vForwardsNorm);

  // Making sure the near/far plane is always closest/farthest.
  if (fNearPlane > fFarPlane)
  {
    wdMath::Swap(m_Planes[NearPlane], m_Planes[FarPlane]);
  }

  wdMat3 mLocalFrame;
  mLocalFrame.SetColumn(0, vRightNorm);
  mLocalFrame.SetColumn(1, vUpNorm);
  mLocalFrame.SetColumn(2, -vForwardsNorm);

  const float fCosFovX = wdMath::Cos(fovX * 0.5f);
  const float fSinFovX = wdMath::Sin(fovX * 0.5f);

  const float fCosFovY = wdMath::Cos(fovY * 0.5f);
  const float fSinFovY = wdMath::Sin(fovY * 0.5f);

  // Left Plane
  {
    wdVec3 vPlaneNormal = mLocalFrame * wdVec3(-fCosFovX, 0, fSinFovX);
    vPlaneNormal.Normalize();

    m_Planes[LeftPlane].SetFromNormalAndPoint(vPlaneNormal, vPosition);
  }

  // Right Plane
  {
    wdVec3 vPlaneNormal = mLocalFrame * wdVec3(fCosFovX, 0, fSinFovX);
    vPlaneNormal.Normalize();

    m_Planes[RightPlane].SetFromNormalAndPoint(vPlaneNormal, vPosition);
  }

  // Bottom Plane
  {
    wdVec3 vPlaneNormal = mLocalFrame * wdVec3(0, -fCosFovY, fSinFovY);
    vPlaneNormal.Normalize();

    m_Planes[BottomPlane].SetFromNormalAndPoint(vPlaneNormal, vPosition);
  }

  // Top Plane
  {
    wdVec3 vPlaneNormal = mLocalFrame * wdVec3(0, fCosFovY, fSinFovY);
    vPlaneNormal.Normalize();

    m_Planes[TopPlane].SetFromNormalAndPoint(vPlaneNormal, vPosition);
  }
}

WD_STATICLINK_FILE(Foundation, Foundation_Math_Implementation_Frustum);

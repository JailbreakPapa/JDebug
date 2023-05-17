#pragma once

#include <Foundation/Math/Mat4.h>

template <typename Type>
WD_FORCE_INLINE wdPlaneTemplate<Type>::wdPlaneTemplate()
{
#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
  // Initialize all data to NaN in debug mode to find problems with uninitialized data easier.
  const Type TypeNaN = wdMath::NaN<Type>();
  m_vNormal.Set(TypeNaN);
  m_fNegDistance = TypeNaN;
#endif
}

template <typename Type>
wdPlaneTemplate<Type>::wdPlaneTemplate(const wdVec3Template<Type>& vNormal, const wdVec3Template<Type>& vPointOnPlane)
{
  SetFromNormalAndPoint(vNormal, vPointOnPlane);
}

template <typename Type>
wdPlaneTemplate<Type>::wdPlaneTemplate(const wdVec3Template<Type>& v1, const wdVec3Template<Type>& v2, const wdVec3Template<Type>& v3)
{
  SetFromPoints(v1, v2, v3).IgnoreResult();
}

template <typename Type>
wdPlaneTemplate<Type>::wdPlaneTemplate(const wdVec3Template<Type>* const pVertices)
{
  SetFromPoints(pVertices).IgnoreResult();
}

template <typename Type>
wdPlaneTemplate<Type>::wdPlaneTemplate(const wdVec3Template<Type>* const pVertices, wdUInt32 uiMaxVertices)
{
  SetFromPoints(pVertices, uiMaxVertices).IgnoreResult();
}

template <typename Type>
wdVec4Template<Type> wdPlaneTemplate<Type>::GetAsVec4() const
{
  return wdVec4(m_vNormal.x, m_vNormal.y, m_vNormal.z, m_fNegDistance);
}

template <typename Type>
void wdPlaneTemplate<Type>::SetFromNormalAndPoint(const wdVec3Template<Type>& vNormal, const wdVec3Template<Type>& vPointOnPlane)
{
  WD_ASSERT_DEBUG(vNormal.IsNormalized(), "Normal must be normalized.");

  m_vNormal = vNormal;
  m_fNegDistance = -m_vNormal.Dot(vPointOnPlane);
}

template <typename Type>
wdResult wdPlaneTemplate<Type>::SetFromPoints(const wdVec3Template<Type>& v1, const wdVec3Template<Type>& v2, const wdVec3Template<Type>& v3)
{
  if (m_vNormal.CalculateNormal(v1, v2, v3) == WD_FAILURE)
    return WD_FAILURE;

  m_fNegDistance = -m_vNormal.Dot(v1);
  return WD_SUCCESS;
}

template <typename Type>
wdResult wdPlaneTemplate<Type>::SetFromPoints(const wdVec3Template<Type>* const pVertices)
{
  if (m_vNormal.CalculateNormal(pVertices[0], pVertices[1], pVertices[2]) == WD_FAILURE)
    return WD_FAILURE;

  m_fNegDistance = -m_vNormal.Dot(pVertices[0]);
  return WD_SUCCESS;
}

template <typename Type>
wdResult wdPlaneTemplate<Type>::SetFromDirections(const wdVec3Template<Type>& vTangent1, const wdVec3Template<Type>& vTangent2, const wdVec3Template<Type>& vPointOnPlane)
{
  wdVec3Template<Type> vNormal = vTangent1.CrossRH(vTangent2);
  wdResult res = vNormal.NormalizeIfNotZero();

  m_vNormal = vNormal;
  m_fNegDistance = -vNormal.Dot(vPointOnPlane);
  return res;
}

template <typename Type>
void wdPlaneTemplate<Type>::Transform(const wdMat3Template<Type>& m)
{
  wdVec3Template<Type> vPointOnPlane = m_vNormal * -m_fNegDistance;

  // rotate the normal, translate the point
  wdVec3Template<Type> vTransformedNormal = m.TransformDirection(m_vNormal);

  // If the plane's distance is already infinite, there won't be any meaningful change
  // to it as a result of the transformation.
  if (!wdMath::IsFinite(m_fNegDistance))
  {
    m_vNormal = vTransformedNormal;
  }
  else
  {
    SetFromNormalAndPoint(vTransformedNormal, m * vPointOnPlane);
  } 
}

template <typename Type>
void wdPlaneTemplate<Type>::Transform(const wdMat4Template<Type>& m)
{
  wdVec3Template<Type> vPointOnPlane = m_vNormal * -m_fNegDistance;

  // rotate the normal, translate the point
  wdVec3Template<Type> vTransformedNormal = m.TransformDirection(m_vNormal);

  // If the plane's distance is already infinite, there won't be any meaningful change
  // to it as a result of the transformation.
  if (!wdMath::IsFinite(m_fNegDistance))
  {
    m_vNormal = vTransformedNormal;
  }
  else
  {
    SetFromNormalAndPoint(vTransformedNormal, m * vPointOnPlane);
  }
}

template <typename Type>
WD_FORCE_INLINE void wdPlaneTemplate<Type>::Flip()
{
  m_fNegDistance = -m_fNegDistance;
  m_vNormal = -m_vNormal;
}

template <typename Type>
WD_FORCE_INLINE Type wdPlaneTemplate<Type>::GetDistanceTo(const wdVec3Template<Type>& vPoint) const
{
  return (m_vNormal.Dot(vPoint) + m_fNegDistance);
}

template <typename Type>
WD_FORCE_INLINE wdPositionOnPlane::Enum wdPlaneTemplate<Type>::GetPointPosition(const wdVec3Template<Type>& vPoint) const
{
  return (m_vNormal.Dot(vPoint) < -m_fNegDistance ? wdPositionOnPlane::Back : wdPositionOnPlane::Front);
}

template <typename Type>
wdPositionOnPlane::Enum wdPlaneTemplate<Type>::GetPointPosition(const wdVec3Template<Type>& vPoint, Type fPlaneHalfWidth) const
{
  const Type f = m_vNormal.Dot(vPoint);

  if (f + fPlaneHalfWidth < -m_fNegDistance)
    return wdPositionOnPlane::Back;

  if (f - fPlaneHalfWidth > -m_fNegDistance)
    return wdPositionOnPlane::Front;

  return wdPositionOnPlane::OnPlane;
}

template <typename Type>
WD_FORCE_INLINE const wdVec3Template<Type> wdPlaneTemplate<Type>::ProjectOntoPlane(const wdVec3Template<Type>& vPoint) const
{
  return vPoint - m_vNormal * (m_vNormal.Dot(vPoint) + m_fNegDistance);
}

template <typename Type>
WD_FORCE_INLINE const wdVec3Template<Type> wdPlaneTemplate<Type>::Mirror(const wdVec3Template<Type>& vPoint) const
{
  return vPoint - (Type)2 * GetDistanceTo(vPoint) * m_vNormal;
}

template <typename Type>
const wdVec3Template<Type> wdPlaneTemplate<Type>::GetCoplanarDirection(const wdVec3Template<Type>& vDirection) const
{
  wdVec3Template<Type> res = vDirection;
  res.MakeOrthogonalTo(m_vNormal);
  return res;
}

template <typename Type>
bool wdPlaneTemplate<Type>::IsIdentical(const wdPlaneTemplate& rhs) const
{
  return m_vNormal.IsIdentical(rhs.m_vNormal) && m_fNegDistance == rhs.m_fNegDistance;
}

template <typename Type>
bool wdPlaneTemplate<Type>::IsEqual(const wdPlaneTemplate& rhs, Type fEpsilon) const
{
  return m_vNormal.IsEqual(rhs.m_vNormal, fEpsilon) && wdMath::IsEqual(m_fNegDistance, rhs.m_fNegDistance, fEpsilon);
}

template <typename Type>
WD_ALWAYS_INLINE bool operator==(const wdPlaneTemplate<Type>& lhs, const wdPlaneTemplate<Type>& rhs)
{
  return lhs.IsIdentical(rhs);
}

template <typename Type>
WD_ALWAYS_INLINE bool operator!=(const wdPlaneTemplate<Type>& lhs, const wdPlaneTemplate<Type>& rhs)
{
  return !lhs.IsIdentical(rhs);
}

template <typename Type>
bool wdPlaneTemplate<Type>::FlipIfNecessary(const wdVec3Template<Type>& vPoint, bool bPlaneShouldFacePoint)
{
  if ((GetPointPosition(vPoint) == wdPositionOnPlane::Front) != bPlaneShouldFacePoint)
  {
    Flip();
    return true;
  }

  return false;
}

template <typename Type>
void wdPlaneTemplate<Type>::SetInvalid()
{
  m_vNormal.Set(0);
  m_fNegDistance = 0;
}

template <typename Type>
bool wdPlaneTemplate<Type>::IsValid() const
{
  return !IsNaN() && m_vNormal.IsNormalized(wdMath::DefaultEpsilon<Type>());
}

template <typename Type>
bool wdPlaneTemplate<Type>::IsNaN() const
{
  return wdMath::IsNaN(m_fNegDistance) || m_vNormal.IsNaN();
}

template <typename Type>
bool wdPlaneTemplate<Type>::IsFinite() const
{
  return m_vNormal.IsValid() && wdMath::IsFinite(m_fNegDistance);
}

/*! The given vertices can be partially equal or lie on the same line. The algorithm will try to find 3 vertices, that
  form a plane, and deduce the normal from them. This algorithm is much slower, than all the other methods, so only
  use it, when you know, that your data can contain such configurations. */
template <typename Type>
wdResult wdPlaneTemplate<Type>::SetFromPoints(const wdVec3Template<Type>* const pVertices, wdUInt32 uiMaxVertices)
{
  wdInt32 iPoints[3];

  if (FindSupportPoints(pVertices, uiMaxVertices, iPoints[0], iPoints[1], iPoints[2]) == WD_FAILURE)
  {
    SetFromPoints(pVertices).IgnoreResult();
    return WD_FAILURE;
  }

  SetFromPoints(pVertices[iPoints[0]], pVertices[iPoints[1]], pVertices[iPoints[2]]).IgnoreResult();
  return WD_SUCCESS;
}

template <typename Type>
wdResult wdPlaneTemplate<Type>::FindSupportPoints(const wdVec3Template<Type>* const pVertices, int iMaxVertices, int& out_i1, int& out_i2, int& out_i3)
{
  const wdVec3Template<Type> v1 = pVertices[0];

  bool bFoundSecond = false;

  int i = 1;
  while (i < iMaxVertices)
  {
    if (pVertices[i].IsEqual(v1, 0.001f) == false)
    {
      bFoundSecond = true;
      break;
    }

    ++i;
  }

  if (!bFoundSecond)
    return WD_FAILURE;

  const wdVec3Template<Type> v2 = pVertices[i];

  const wdVec3Template<Type> vDir1 = (v1 - v2).GetNormalized();

  out_i1 = 0;
  out_i2 = i;

  ++i;

  while (i < iMaxVertices)
  {
    // check for inequality, then for non-collinearity
    if ((pVertices[i].IsEqual(v2, 0.001f) == false) && (wdMath::Abs((pVertices[i] - v2).GetNormalized().Dot(vDir1)) < (Type)0.999))
    {
      out_i3 = i;
      return WD_SUCCESS;
    }

    ++i;
  }

  return WD_FAILURE;
}

template <typename Type>
wdPositionOnPlane::Enum wdPlaneTemplate<Type>::GetObjectPosition(const wdVec3Template<Type>* const pPoints, wdUInt32 uiVertices) const
{
  bool bFront = false;
  bool bBack = false;

  for (wdUInt32 i = 0; i < uiVertices; ++i)
  {
    switch (GetPointPosition(pPoints[i]))
    {
      case wdPositionOnPlane::Front:
        if (bBack)
          return (wdPositionOnPlane::Spanning);
        bFront = true;
        break;
      case wdPositionOnPlane::Back:
        if (bFront)
          return (wdPositionOnPlane::Spanning);
        bBack = true;
        break;

      default:
        break;
    }
  }

  return (bFront ? wdPositionOnPlane::Front : wdPositionOnPlane::Back);
}

template <typename Type>
wdPositionOnPlane::Enum wdPlaneTemplate<Type>::GetObjectPosition(const wdVec3Template<Type>* const pPoints, wdUInt32 uiVertices, Type fPlaneHalfWidth) const
{
  bool bFront = false;
  bool bBack = false;

  for (wdUInt32 i = 0; i < uiVertices; ++i)
  {
    switch (GetPointPosition(pPoints[i], fPlaneHalfWidth))
    {
      case wdPositionOnPlane::Front:
        if (bBack)
          return (wdPositionOnPlane::Spanning);
        bFront = true;
        break;
      case wdPositionOnPlane::Back:
        if (bFront)
          return (wdPositionOnPlane::Spanning);
        bBack = true;
        break;

      default:
        break;
    }
  }

  if (bFront)
    return (wdPositionOnPlane::Front);
  if (bBack)
    return (wdPositionOnPlane::Back);

  return (wdPositionOnPlane::OnPlane);
}

template <typename Type>
bool wdPlaneTemplate<Type>::GetRayIntersection(const wdVec3Template<Type>& vRayStartPos, const wdVec3Template<Type>& vRayDir, Type* out_pIntersectionDistance, wdVec3Template<Type>* out_pIntersection) const
{
  WD_ASSERT_DEBUG(vRayStartPos.IsValid(), "Ray start position must be valid.");
  WD_ASSERT_DEBUG(vRayDir.IsValid(), "Ray direction must be valid.");

  const Type fPlaneSide = GetDistanceTo(vRayStartPos);
  const Type fCosAlpha = m_vNormal.Dot(vRayDir);

  if (fCosAlpha == 0) // ray is orthogonal to plane
    return false;

  if (wdMath::Sign(fPlaneSide) == wdMath::Sign(fCosAlpha)) // ray points away from the plane
    return false;

  const Type fTime = -fPlaneSide / fCosAlpha;

  if (out_pIntersectionDistance)
    *out_pIntersectionDistance = fTime;

  if (out_pIntersection)
    *out_pIntersection = vRayStartPos + fTime * vRayDir;

  return true;
}

template <typename Type>
bool wdPlaneTemplate<Type>::GetRayIntersectionBiDirectional(const wdVec3Template<Type>& vRayStartPos, const wdVec3Template<Type>& vRayDir, Type* out_pIntersectionDistance, wdVec3Template<Type>* out_pIntersection) const
{
  WD_ASSERT_DEBUG(vRayStartPos.IsValid(), "Ray start position must be valid.");
  WD_ASSERT_DEBUG(vRayDir.IsValid(), "Ray direction must be valid.");

  const Type fPlaneSide = GetDistanceTo(vRayStartPos);
  const Type fCosAlpha = m_vNormal.Dot(vRayDir);

  if (fCosAlpha == 0) // ray is orthogonal to plane
    return false;

  const Type fTime = -fPlaneSide / fCosAlpha;

  if (out_pIntersectionDistance)
    *out_pIntersectionDistance = fTime;

  if (out_pIntersection)
    *out_pIntersection = vRayStartPos + fTime * vRayDir;

  return true;
}

template <typename Type>
bool wdPlaneTemplate<Type>::GetLineSegmentIntersection(const wdVec3Template<Type>& vLineStartPos, const wdVec3Template<Type>& vLineEndPos, Type* out_pHitFraction, wdVec3Template<Type>* out_pIntersection) const
{
  Type fTime = 0;

  if (!GetRayIntersection(vLineStartPos, vLineEndPos - vLineStartPos, &fTime, out_pIntersection))
    return false;

  if (out_pHitFraction)
    *out_pHitFraction = fTime;

  return (fTime <= 1);
}

template <typename Type>
Type wdPlaneTemplate<Type>::GetMinimumDistanceTo(const wdVec3Template<Type>* pPoints, wdUInt32 uiNumPoints, wdUInt32 uiStride /* = sizeof (wdVec3Template<Type>) */) const
{
  WD_ASSERT_DEBUG(pPoints != nullptr, "Array may not be nullptr.");
  WD_ASSERT_DEBUG(uiStride >= sizeof(wdVec3Template<Type>), "Stride must be at least sizeof(wdVec3Template) to not have overlapping data.");
  WD_ASSERT_DEBUG(uiNumPoints >= 1, "Array must contain at least one point.");

  Type fMinDist = wdMath::MaxValue<Type>();

  const wdVec3Template<Type>* pCurPoint = pPoints;

  for (wdUInt32 i = 0; i < uiNumPoints; ++i)
  {
    fMinDist = wdMath::Min(m_vNormal.Dot(*pCurPoint), fMinDist);

    pCurPoint = wdMemoryUtils::AddByteOffset(pCurPoint, uiStride);
  }

  return fMinDist + m_fNegDistance;
}

template <typename Type>
void wdPlaneTemplate<Type>::GetMinMaxDistanceTo(Type& out_fMin, Type& out_fMax, const wdVec3Template<Type>* pPoints, wdUInt32 uiNumPoints, wdUInt32 uiStride /* = sizeof (wdVec3Template<Type>) */) const
{
  WD_ASSERT_DEBUG(pPoints != nullptr, "Array may not be nullptr.");
  WD_ASSERT_DEBUG(uiStride >= sizeof(wdVec3Template<Type>), "Stride must be at least sizeof(wdVec3Template) to not have overlapping data.");
  WD_ASSERT_DEBUG(uiNumPoints >= 1, "Array must contain at least one point.");

  out_fMin = wdMath::MaxValue<Type>();
  out_fMax = -wdMath::MaxValue<Type>();

  const wdVec3Template<Type>* pCurPoint = pPoints;

  for (wdUInt32 i = 0; i < uiNumPoints; ++i)
  {
    const Type f = m_vNormal.Dot(*pCurPoint);

    out_fMin = wdMath::Min(f, out_fMin);
    out_fMax = wdMath::Max(f, out_fMax);

    pCurPoint = wdMemoryUtils::AddByteOffset(pCurPoint, uiStride);
  }

  out_fMin += m_fNegDistance;
  out_fMax += m_fNegDistance;
}

template <typename Type>
wdResult wdPlaneTemplate<Type>::GetPlanesIntersectionPoint(const wdPlaneTemplate& p0, const wdPlaneTemplate& p1, const wdPlaneTemplate& p2, wdVec3Template<Type>& out_vResult)
{
  const wdVec3Template<Type> n1(p0.m_vNormal);
  const wdVec3Template<Type> n2(p1.m_vNormal);
  const wdVec3Template<Type> n3(p2.m_vNormal);

  const Type det = n1.Dot(n2.CrossRH(n3));

  if (wdMath::IsZero<Type>(det, wdMath::LargeEpsilon<Type>()))
    return WD_FAILURE;

  out_vResult = (-p0.m_fNegDistance * n2.CrossRH(n3) + -p1.m_fNegDistance * n3.CrossRH(n1) + -p2.m_fNegDistance * n1.CrossRH(n2)) / det;

  return WD_SUCCESS;
}

#include <Foundation/Math/Implementation/AllClasses_inl.h>

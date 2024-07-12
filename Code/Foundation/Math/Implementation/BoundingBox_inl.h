#pragma once

#include <Foundation/Math/Mat4.h>

template <typename Type>
NS_ALWAYS_INLINE nsBoundingBoxTemplate<Type>::nsBoundingBoxTemplate() = default;

template <typename Type>
NS_FORCE_INLINE nsBoundingBoxTemplate<Type>::nsBoundingBoxTemplate(const nsVec3Template<Type>& vMin, const nsVec3Template<Type>& vMax)
{
  *this = MakeFromMinMax(vMin, vMax);
}

template <typename Type>
NS_FORCE_INLINE nsBoundingBoxTemplate<Type> nsBoundingBoxTemplate<Type>::MakeZero()
{
  nsBoundingBoxTemplate<Type> res;
  res.m_vMin = nsVec3Template<Type>::MakeZero();
  res.m_vMax = nsVec3Template<Type>::MakeZero();
  return res;
}

template <typename Type>
NS_FORCE_INLINE nsBoundingBoxTemplate<Type> nsBoundingBoxTemplate<Type>::MakeInvalid()
{
  nsBoundingBoxTemplate<Type> res;
  res.m_vMin.Set(nsMath::MaxValue<Type>());
  res.m_vMax.Set(-nsMath::MaxValue<Type>());
  return res;
}

template <typename Type>
NS_FORCE_INLINE nsBoundingBoxTemplate<Type> nsBoundingBoxTemplate<Type>::MakeFromCenterAndHalfExtents(const nsVec3Template<Type>& vCenter, const nsVec3Template<Type>& vHalfExtents)
{
  nsBoundingBoxTemplate<Type> res;
  res.m_vMin = vCenter - vHalfExtents;
  res.m_vMax = vCenter + vHalfExtents;
  return res;
}

template <typename Type>
NS_FORCE_INLINE nsBoundingBoxTemplate<Type> nsBoundingBoxTemplate<Type>::MakeFromMinMax(const nsVec3Template<Type>& vMin, const nsVec3Template<Type>& vMax)
{
  nsBoundingBoxTemplate<Type> res;
  res.m_vMin = vMin;
  res.m_vMax = vMax;

  NS_ASSERT_DEBUG(res.IsValid(), "The given values don't create a valid bounding box ({0} | {1} | {2} - {3} | {4} | {5})", nsArgF(vMin.x, 2), nsArgF(vMin.y, 2), nsArgF(vMin.z, 2), nsArgF(vMax.x, 2), nsArgF(vMax.y, 2), nsArgF(vMax.z, 2));

  return res;
}

template <typename Type>
NS_FORCE_INLINE nsBoundingBoxTemplate<Type> nsBoundingBoxTemplate<Type>::MakeFromPoints(const nsVec3Template<Type>* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride /*= sizeof(nsVec3Template<Type>)*/)
{
  nsBoundingBoxTemplate<Type> res = MakeInvalid();
  res.ExpandToInclude(pPoints, uiNumPoints, uiStride);
  return res;
}

template <typename Type>
void nsBoundingBoxTemplate<Type>::GetCorners(nsVec3Template<Type>* out_pCorners) const
{
  NS_NAN_ASSERT(this);
  NS_ASSERT_DEBUG(out_pCorners != nullptr, "Out Parameter must not be nullptr.");

  out_pCorners[0].Set(m_vMin.x, m_vMin.y, m_vMin.z);
  out_pCorners[1].Set(m_vMin.x, m_vMin.y, m_vMax.z);
  out_pCorners[2].Set(m_vMin.x, m_vMax.y, m_vMin.z);
  out_pCorners[3].Set(m_vMin.x, m_vMax.y, m_vMax.z);
  out_pCorners[4].Set(m_vMax.x, m_vMin.y, m_vMin.z);
  out_pCorners[5].Set(m_vMax.x, m_vMin.y, m_vMax.z);
  out_pCorners[6].Set(m_vMax.x, m_vMax.y, m_vMin.z);
  out_pCorners[7].Set(m_vMax.x, m_vMax.y, m_vMax.z);
}

template <typename Type>
NS_FORCE_INLINE const nsVec3Template<Type> nsBoundingBoxTemplate<Type>::GetCenter() const
{
  return m_vMin + GetHalfExtents();
}

template <typename Type>
NS_ALWAYS_INLINE const nsVec3Template<Type> nsBoundingBoxTemplate<Type>::GetExtents() const
{
  return m_vMax - m_vMin;
}

template <typename Type>
NS_FORCE_INLINE const nsVec3Template<Type> nsBoundingBoxTemplate<Type>::GetHalfExtents() const
{
  return (m_vMax - m_vMin) / (Type)2;
}

template <typename Type>
bool nsBoundingBoxTemplate<Type>::IsValid() const
{
  return (m_vMin.IsValid() && m_vMax.IsValid() && m_vMin.x <= m_vMax.x && m_vMin.y <= m_vMax.y && m_vMin.z <= m_vMax.z);
}

template <typename Type>
bool nsBoundingBoxTemplate<Type>::IsNaN() const
{
  return m_vMin.IsNaN() || m_vMax.IsNaN();
}

template <typename Type>
NS_FORCE_INLINE void nsBoundingBoxTemplate<Type>::ExpandToInclude(const nsVec3Template<Type>& vPoint)
{
  m_vMin = m_vMin.CompMin(vPoint);
  m_vMax = m_vMax.CompMax(vPoint);
}

template <typename Type>
NS_FORCE_INLINE void nsBoundingBoxTemplate<Type>::ExpandToInclude(const nsBoundingBoxTemplate<Type>& rhs)
{
  NS_ASSERT_DEBUG(rhs.IsValid(), "rhs must be a valid AABB.");
  m_vMin = m_vMin.CompMin(rhs.m_vMin);
  m_vMax = m_vMax.CompMax(rhs.m_vMax);
}

template <typename Type>
void nsBoundingBoxTemplate<Type>::ExpandToInclude(const nsVec3Template<Type>* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride)
{
  NS_ASSERT_DEBUG(pPoints != nullptr, "Array may not be nullptr.");
  NS_ASSERT_DEBUG(uiStride >= sizeof(nsVec3Template<Type>), "Data may not overlap.");

  const nsVec3Template<Type>* pCur = &pPoints[0];

  for (nsUInt32 i = 0; i < uiNumPoints; ++i)
  {
    ExpandToInclude(*pCur);

    pCur = nsMemoryUtils::AddByteOffset(pCur, uiStride);
  }
}

template <typename Type>
void nsBoundingBoxTemplate<Type>::ExpandToCube()
{
  nsVec3Template<Type> vHalfExtents = GetHalfExtents();
  const nsVec3Template<Type> vCenter = m_vMin + vHalfExtents;

  const Type f = nsMath::Max(vHalfExtents.x, vHalfExtents.y, vHalfExtents.z);

  m_vMin = vCenter - nsVec3Template<Type>(f);
  m_vMax = vCenter + nsVec3Template<Type>(f);
}

template <typename Type>
NS_FORCE_INLINE void nsBoundingBoxTemplate<Type>::Grow(const nsVec3Template<Type>& vDiff)
{
  NS_ASSERT_DEBUG(IsValid(), "Cannot grow a box that is invalid.");

  m_vMax += vDiff;
  m_vMin -= vDiff;

  NS_ASSERT_DEBUG(IsValid(), "The grown box has become invalid.");
}

template <typename Type>
NS_FORCE_INLINE bool nsBoundingBoxTemplate<Type>::Contains(const nsVec3Template<Type>& vPoint) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&vPoint);

  return (nsMath::IsInRange(vPoint.x, m_vMin.x, m_vMax.x) && nsMath::IsInRange(vPoint.y, m_vMin.y, m_vMax.y) &&
          nsMath::IsInRange(vPoint.z, m_vMin.z, m_vMax.z));
}

template <typename Type>
NS_FORCE_INLINE bool nsBoundingBoxTemplate<Type>::Contains(const nsBoundingBoxTemplate<Type>& rhs) const
{
  return Contains(rhs.m_vMin) && Contains(rhs.m_vMax);
}

template <typename Type>
bool nsBoundingBoxTemplate<Type>::Contains(const nsVec3Template<Type>* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride /* = sizeof(nsVec3Template<Type>) */) const
{
  NS_ASSERT_DEBUG(pPoints != nullptr, "Array must not be NuLL.");
  NS_ASSERT_DEBUG(uiStride >= sizeof(nsVec3Template<Type>), "Data must not overlap.");

  const nsVec3Template<Type>* pCur = &pPoints[0];

  for (nsUInt32 i = 0; i < uiNumPoints; ++i)
  {
    if (!Contains(*pCur))
      return false;

    pCur = nsMemoryUtils::AddByteOffset(pCur, uiStride);
  }

  return true;
}

template <typename Type>
bool nsBoundingBoxTemplate<Type>::Overlaps(const nsBoundingBoxTemplate<Type>& rhs) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  if (rhs.m_vMin.x >= m_vMax.x)
    return false;
  if (rhs.m_vMin.y >= m_vMax.y)
    return false;
  if (rhs.m_vMin.z >= m_vMax.z)
    return false;

  if (m_vMin.x >= rhs.m_vMax.x)
    return false;
  if (m_vMin.y >= rhs.m_vMax.y)
    return false;
  if (m_vMin.z >= rhs.m_vMax.z)
    return false;

  return true;
}

template <typename Type>
bool nsBoundingBoxTemplate<Type>::Overlaps(const nsVec3Template<Type>* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride /* = sizeof(nsVec3Template<Type>) */) const
{
  NS_ASSERT_DEBUG(pPoints != nullptr, "Array must not be NuLL.");
  NS_ASSERT_DEBUG(uiStride >= sizeof(nsVec3Template<Type>), "Data must not overlap.");

  const nsVec3Template<Type>* pCur = &pPoints[0];

  for (nsUInt32 i = 0; i < uiNumPoints; ++i)
  {
    if (Contains(*pCur))
      return true;

    pCur = nsMemoryUtils::AddByteOffset(pCur, uiStride);
  }

  return false;
}

template <typename Type>
NS_ALWAYS_INLINE bool nsBoundingBoxTemplate<Type>::IsIdentical(const nsBoundingBoxTemplate<Type>& rhs) const
{
  return (m_vMin == rhs.m_vMin && m_vMax == rhs.m_vMax);
}

template <typename Type>
bool nsBoundingBoxTemplate<Type>::IsEqual(const nsBoundingBoxTemplate<Type>& rhs, Type fEpsilon) const
{
  return (m_vMin.IsEqual(rhs.m_vMin, fEpsilon) && m_vMax.IsEqual(rhs.m_vMax, fEpsilon));
}

template <typename Type>
NS_ALWAYS_INLINE bool operator==(const nsBoundingBoxTemplate<Type>& lhs, const nsBoundingBoxTemplate<Type>& rhs)
{
  return lhs.IsIdentical(rhs);
}

template <typename Type>
NS_ALWAYS_INLINE bool operator!=(const nsBoundingBoxTemplate<Type>& lhs, const nsBoundingBoxTemplate<Type>& rhs)
{
  return !lhs.IsIdentical(rhs);
}

template <typename Type>
NS_FORCE_INLINE void nsBoundingBoxTemplate<Type>::Translate(const nsVec3Template<Type>& vDiff)
{
  m_vMin += vDiff;
  m_vMax += vDiff;
}

template <typename Type>
void nsBoundingBoxTemplate<Type>::ScaleFromCenter(const nsVec3Template<Type>& vScale)
{
  const nsVec3Template<Type> vCenter = GetCenter();
  const nsVec3 vNewMin = vCenter + (m_vMin - vCenter).CompMul(vScale);
  const nsVec3 vNewMax = vCenter + (m_vMax - vCenter).CompMul(vScale);

  // this is necessary for negative scalings to work as expected
  m_vMin = vNewMin.CompMin(vNewMax);
  m_vMax = vNewMin.CompMax(vNewMax);
}

template <typename Type>
NS_FORCE_INLINE void nsBoundingBoxTemplate<Type>::ScaleFromOrigin(const nsVec3Template<Type>& vScale)
{
  const nsVec3 vNewMin = m_vMin.CompMul(vScale);
  const nsVec3 vNewMax = m_vMax.CompMul(vScale);

  // this is necessary for negative scalings to work as expected
  m_vMin = vNewMin.CompMin(vNewMax);
  m_vMax = vNewMin.CompMax(vNewMax);
}

template <typename Type>
void nsBoundingBoxTemplate<Type>::TransformFromCenter(const nsMat4Template<Type>& mTransform)
{
  nsVec3Template<Type> vCorners[8];
  GetCorners(vCorners);

  const nsVec3Template<Type> vCenter = GetCenter();
  *this = MakeInvalid();

  for (nsUInt32 i = 0; i < 8; ++i)
    ExpandToInclude(vCenter + mTransform.TransformPosition(vCorners[i] - vCenter));
}

template <typename Type>
void nsBoundingBoxTemplate<Type>::TransformFromOrigin(const nsMat4Template<Type>& mTransform)
{
  nsVec3Template<Type> vCorners[8];
  GetCorners(vCorners);

  mTransform.TransformPosition(vCorners, 8);

  *this = MakeInvalid();
  ExpandToInclude(vCorners, 8);
}

template <typename Type>
NS_FORCE_INLINE const nsVec3Template<Type> nsBoundingBoxTemplate<Type>::GetClampedPoint(const nsVec3Template<Type>& vPoint) const
{
  return vPoint.CompMin(m_vMax).CompMax(m_vMin);
}

template <typename Type>
Type nsBoundingBoxTemplate<Type>::GetDistanceTo(const nsVec3Template<Type>& vPoint) const
{
  const nsVec3Template<Type> vClamped = GetClampedPoint(vPoint);

  return (vPoint - vClamped).GetLength();
}

template <typename Type>
Type nsBoundingBoxTemplate<Type>::GetDistanceSquaredTo(const nsVec3Template<Type>& vPoint) const
{
  const nsVec3Template<Type> vClamped = GetClampedPoint(vPoint);

  return (vPoint - vClamped).GetLengthSquared();
}

template <typename Type>
Type nsBoundingBoxTemplate<Type>::GetDistanceSquaredTo(const nsBoundingBoxTemplate<Type>& rhs) const
{
  // This will return zero for overlapping boxes

  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  Type fDistSQR = 0.0f;

  {
    if (rhs.m_vMin.x > m_vMax.x)
    {
      fDistSQR += nsMath::Square(rhs.m_vMin.x - m_vMax.x);
    }
    else if (rhs.m_vMax.x < m_vMin.x)
    {
      fDistSQR += nsMath::Square(m_vMin.x - rhs.m_vMax.x);
    }
  }

  {
    if (rhs.m_vMin.y > m_vMax.y)
    {
      fDistSQR += nsMath::Square(rhs.m_vMin.y - m_vMax.y);
    }
    else if (rhs.m_vMax.y < m_vMin.y)
    {
      fDistSQR += nsMath::Square(m_vMin.y - rhs.m_vMax.y);
    }
  }

  {
    if (rhs.m_vMin.z > m_vMax.z)
    {
      fDistSQR += nsMath::Square(rhs.m_vMin.z - m_vMax.z);
    }
    else if (rhs.m_vMax.z < m_vMin.z)
    {
      fDistSQR += nsMath::Square(m_vMin.z - rhs.m_vMax.z);
    }
  }

  return fDistSQR;
}

template <typename Type>
Type nsBoundingBoxTemplate<Type>::GetDistanceTo(const nsBoundingBoxTemplate<Type>& rhs) const
{
  return nsMath::Sqrt(GetDistanceSquaredTo(rhs));
}

template <typename Type>
bool nsBoundingBoxTemplate<Type>::GetRayIntersection(const nsVec3Template<Type>& vStartPos, const nsVec3Template<Type>& vRayDir, Type* out_pIntersectionDistance, nsVec3Template<Type>* out_pIntersection) const
{
  // This code was taken from: http://people.csail.mit.edu/amy/papers/box-jgt.pdf
  // "An Efficient and Robust Ray-Box Intersection Algorithm"
  // Contrary to previous implementation, this one actually works with ray/box configurations
  // that produce division by zero and multiplication with infinity (which can produce NaNs).

  NS_ASSERT_DEBUG(nsMath::SupportsInfinity<Type>(), "This type does not support infinite values, which is required for this algorithm.");
  NS_ASSERT_DEBUG(vStartPos.IsValid(), "Ray start position must be valid.");
  NS_ASSERT_DEBUG(vRayDir.IsValid(), "Ray direction must be valid.");

  NS_NAN_ASSERT(this);

  float tMin, tMax;

  // Compare along X and Z axis, find intersection point
  {
    float tMinY, tMaxY;

    const float fDivX = 1.0f / vRayDir.x;
    const float fDivY = 1.0f / vRayDir.y;

    if (vRayDir.x >= 0.0f)
    {
      tMin = (m_vMin.x - vStartPos.x) * fDivX;
      tMax = (m_vMax.x - vStartPos.x) * fDivX;
    }
    else
    {
      tMin = (m_vMax.x - vStartPos.x) * fDivX;
      tMax = (m_vMin.x - vStartPos.x) * fDivX;
    }

    if (vRayDir.y >= 0.0f)
    {
      tMinY = (m_vMin.y - vStartPos.y) * fDivY;
      tMaxY = (m_vMax.y - vStartPos.y) * fDivY;
    }
    else
    {
      tMinY = (m_vMax.y - vStartPos.y) * fDivY;
      tMaxY = (m_vMin.y - vStartPos.y) * fDivY;
    }

    if (tMin > tMaxY || tMinY > tMax)
      return false;

    if (tMinY > tMin)
      tMin = tMinY;
    if (tMaxY < tMax)
      tMax = tMaxY;
  }

  // Compare along Z axis and previous result, find intersection point
  {
    float tMinZ, tMaxZ;

    const float fDivZ = 1.0f / vRayDir.z;

    if (vRayDir.z >= 0.0f)
    {
      tMinZ = (m_vMin.z - vStartPos.z) * fDivZ;
      tMaxZ = (m_vMax.z - vStartPos.z) * fDivZ;
    }
    else
    {
      tMinZ = (m_vMax.z - vStartPos.z) * fDivZ;
      tMaxZ = (m_vMin.z - vStartPos.z) * fDivZ;
    }

    if (tMin > tMaxZ || tMinZ > tMax)
      return false;

    if (tMinZ > tMin)
      tMin = tMinZ;
    if (tMaxZ < tMax)
      tMax = tMaxZ;
  }

  // rays that start inside the box are considered as not hitting the box
  if (tMax <= 0.0f)
    return false;

  if (out_pIntersectionDistance)
    *out_pIntersectionDistance = tMin;

  if (out_pIntersection)
    *out_pIntersection = vStartPos + tMin * vRayDir;

  return true;
}

template <typename Type>
bool nsBoundingBoxTemplate<Type>::GetLineSegmentIntersection(const nsVec3Template<Type>& vStartPos, const nsVec3Template<Type>& vEndPos, Type* out_pLineFraction, nsVec3Template<Type>* out_pIntersection) const
{
  const nsVec3Template<Type> vRayDir = vEndPos - vStartPos;

  Type fIntersection = 0.0f;
  if (!GetRayIntersection(vStartPos, vRayDir, &fIntersection, out_pIntersection))
    return false;

  if (out_pLineFraction)
    *out_pLineFraction = fIntersection;

  return fIntersection <= 1.0f;
}



#include <Foundation/Math/Implementation/AllClasses_inl.h>

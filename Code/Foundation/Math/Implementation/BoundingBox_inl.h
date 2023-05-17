#pragma once

#include <Foundation/Math/Mat4.h>

template <typename Type>
WD_ALWAYS_INLINE wdBoundingBoxTemplate<Type>::wdBoundingBoxTemplate()
{
}

template <typename Type>
WD_FORCE_INLINE wdBoundingBoxTemplate<Type>::wdBoundingBoxTemplate(const wdVec3Template<Type>& vMin, const wdVec3Template<Type>& vMax)
{
  SetElements(vMin, vMax);
}

template <typename Type>
WD_FORCE_INLINE void wdBoundingBoxTemplate<Type>::SetElements(const wdVec3Template<Type>& vMin, const wdVec3Template<Type>& vMax)
{
  m_vMin = vMin;
  m_vMax = vMax;

  WD_ASSERT_DEBUG(IsValid(), "The given values did not create a valid bounding box ({0} | {1} | {2} - {3} | {4} | {5})", wdArgF(vMin.x, 2),
    wdArgF(vMin.y, 2), wdArgF(vMin.z, 2), wdArgF(vMax.x, 2), wdArgF(vMax.y, 2), wdArgF(vMax.z, 2));
}

template <typename Type>
void wdBoundingBoxTemplate<Type>::SetFromPoints(
  const wdVec3Template<Type>* pPoints, wdUInt32 uiNumPoints, wdUInt32 uiStride /* = sizeof(wdVec3Template<Type>) */)
{
  SetInvalid();
  ExpandToInclude(pPoints, uiNumPoints, uiStride);
}

template <typename Type>
void wdBoundingBoxTemplate<Type>::GetCorners(wdVec3Template<Type>* out_pCorners) const
{
  WD_NAN_ASSERT(this);
  WD_ASSERT_DEBUG(out_pCorners != nullptr, "Out Parameter must not be nullptr.");

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
WD_FORCE_INLINE const wdVec3Template<Type> wdBoundingBoxTemplate<Type>::GetCenter() const
{
  return m_vMin + GetHalfExtents();
}

template <typename Type>
WD_ALWAYS_INLINE const wdVec3Template<Type> wdBoundingBoxTemplate<Type>::GetExtents() const
{
  return m_vMax - m_vMin;
}

template <typename Type>
const wdVec3Template<Type> wdBoundingBoxTemplate<Type>::GetHalfExtents() const
{
  return (m_vMax - m_vMin) / (Type)2;
}

template <typename Type>
void wdBoundingBoxTemplate<Type>::SetCenterAndHalfExtents(const wdVec3Template<Type>& vCenter, const wdVec3Template<Type>& vHalfExtents)
{
  m_vMin = vCenter - vHalfExtents;
  m_vMax = vCenter + vHalfExtents;
}

template <typename Type>
void wdBoundingBoxTemplate<Type>::SetInvalid()
{
  m_vMin.Set(wdMath::MaxValue<Type>());
  m_vMax.Set(-wdMath::MaxValue<Type>());
}

template <typename Type>
bool wdBoundingBoxTemplate<Type>::IsValid() const
{
  return (m_vMin.IsValid() && m_vMax.IsValid() && m_vMin.x <= m_vMax.x && m_vMin.y <= m_vMax.y && m_vMin.z <= m_vMax.z);
}

template <typename Type>
bool wdBoundingBoxTemplate<Type>::IsNaN() const
{
  return m_vMin.IsNaN() || m_vMax.IsNaN();
}

template <typename Type>
WD_FORCE_INLINE void wdBoundingBoxTemplate<Type>::ExpandToInclude(const wdVec3Template<Type>& vPoint)
{
  m_vMin = m_vMin.CompMin(vPoint);
  m_vMax = m_vMax.CompMax(vPoint);
}

template <typename Type>
WD_FORCE_INLINE void wdBoundingBoxTemplate<Type>::ExpandToInclude(const wdBoundingBoxTemplate<Type>& rhs)
{
  WD_ASSERT_DEBUG(rhs.IsValid(), "rhs must be a valid AABB.");
  ExpandToInclude(rhs.m_vMin);
  ExpandToInclude(rhs.m_vMax);
}

template <typename Type>
void wdBoundingBoxTemplate<Type>::ExpandToInclude(const wdVec3Template<Type>* pPoints, wdUInt32 uiNumPoints, wdUInt32 uiStride)
{
  WD_ASSERT_DEBUG(pPoints != nullptr, "Array may not be nullptr.");
  WD_ASSERT_DEBUG(uiStride >= sizeof(wdVec3Template<Type>), "Data may not overlap.");

  const wdVec3Template<Type>* pCur = &pPoints[0];

  for (wdUInt32 i = 0; i < uiNumPoints; ++i)
  {
    ExpandToInclude(*pCur);

    pCur = wdMemoryUtils::AddByteOffset(pCur, uiStride);
  }
}

template <typename Type>
void wdBoundingBoxTemplate<Type>::ExpandToCube()
{
  wdVec3Template<Type> vHalfExtents = GetHalfExtents();
  const wdVec3Template<Type> vCenter = m_vMin + vHalfExtents;

  const Type f = wdMath::Max(vHalfExtents.x, vHalfExtents.y, vHalfExtents.z);

  m_vMin = vCenter - wdVec3Template<Type>(f);
  m_vMax = vCenter + wdVec3Template<Type>(f);
}

template <typename Type>
WD_FORCE_INLINE void wdBoundingBoxTemplate<Type>::Grow(const wdVec3Template<Type>& vDiff)
{
  WD_ASSERT_DEBUG(IsValid(), "Cannot grow a box that is invalid.");

  m_vMax += vDiff;
  m_vMin -= vDiff;

  WD_ASSERT_DEBUG(IsValid(), "The grown box has become invalid.");
}

template <typename Type>
WD_FORCE_INLINE bool wdBoundingBoxTemplate<Type>::Contains(const wdVec3Template<Type>& vPoint) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&vPoint);

  return (wdMath::IsInRange(vPoint.x, m_vMin.x, m_vMax.x) && wdMath::IsInRange(vPoint.y, m_vMin.y, m_vMax.y) &&
          wdMath::IsInRange(vPoint.z, m_vMin.z, m_vMax.z));
}

template <typename Type>
WD_FORCE_INLINE bool wdBoundingBoxTemplate<Type>::Contains(const wdBoundingBoxTemplate<Type>& rhs) const
{
  return Contains(rhs.m_vMin) && Contains(rhs.m_vMax);
}

template <typename Type>
bool wdBoundingBoxTemplate<Type>::Contains(
  const wdVec3Template<Type>* pPoints, wdUInt32 uiNumPoints, wdUInt32 uiStride /* = sizeof(wdVec3Template<Type>) */) const
{
  WD_ASSERT_DEBUG(pPoints != nullptr, "Array must not be NuLL.");
  WD_ASSERT_DEBUG(uiStride >= sizeof(wdVec3Template<Type>), "Data must not overlap.");

  const wdVec3Template<Type>* pCur = &pPoints[0];

  for (wdUInt32 i = 0; i < uiNumPoints; ++i)
  {
    if (!Contains(*pCur))
      return false;

    pCur = wdMemoryUtils::AddByteOffset(pCur, uiStride);
  }

  return true;
}

template <typename Type>
bool wdBoundingBoxTemplate<Type>::Overlaps(const wdBoundingBoxTemplate<Type>& rhs) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

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
bool wdBoundingBoxTemplate<Type>::Overlaps(
  const wdVec3Template<Type>* pPoints, wdUInt32 uiNumPoints, wdUInt32 uiStride /* = sizeof(wdVec3Template<Type>) */) const
{
  WD_ASSERT_DEBUG(pPoints != nullptr, "Array must not be NuLL.");
  WD_ASSERT_DEBUG(uiStride >= sizeof(wdVec3Template<Type>), "Data must not overlap.");

  const wdVec3Template<Type>* pCur = &pPoints[0];

  for (wdUInt32 i = 0; i < uiNumPoints; ++i)
  {
    if (Contains(*pCur))
      return true;

    pCur = wdMemoryUtils::AddByteOffset(pCur, uiStride);
  }

  return false;
}

template <typename Type>
WD_ALWAYS_INLINE bool wdBoundingBoxTemplate<Type>::IsIdentical(const wdBoundingBoxTemplate<Type>& rhs) const
{
  return (m_vMin == rhs.m_vMin && m_vMax == rhs.m_vMax);
}

template <typename Type>
bool wdBoundingBoxTemplate<Type>::IsEqual(const wdBoundingBoxTemplate<Type>& rhs, Type fEpsilon) const
{
  return (m_vMin.IsEqual(rhs.m_vMin, fEpsilon) && m_vMax.IsEqual(rhs.m_vMax, fEpsilon));
}

template <typename Type>
WD_ALWAYS_INLINE bool operator==(const wdBoundingBoxTemplate<Type>& lhs, const wdBoundingBoxTemplate<Type>& rhs)
{
  return lhs.IsIdentical(rhs);
}

template <typename Type>
WD_ALWAYS_INLINE bool operator!=(const wdBoundingBoxTemplate<Type>& lhs, const wdBoundingBoxTemplate<Type>& rhs)
{
  return !lhs.IsIdentical(rhs);
}

template <typename Type>
WD_FORCE_INLINE void wdBoundingBoxTemplate<Type>::Translate(const wdVec3Template<Type>& vDiff)
{
  m_vMin += vDiff;
  m_vMax += vDiff;
}

template <typename Type>
void wdBoundingBoxTemplate<Type>::ScaleFromCenter(const wdVec3Template<Type>& vScale)
{
  const wdVec3Template<Type> vCenter = GetCenter();
  const wdVec3 vNewMin = vCenter + (m_vMin - vCenter).CompMul(vScale);
  const wdVec3 vNewMax = vCenter + (m_vMax - vCenter).CompMul(vScale);

  // this is necessary for negative scalings to work as expected
  m_vMin = vNewMin.CompMin(vNewMax);
  m_vMax = vNewMin.CompMax(vNewMax);
}

template <typename Type>
WD_FORCE_INLINE void wdBoundingBoxTemplate<Type>::ScaleFromOrigin(const wdVec3Template<Type>& vScale)
{
  const wdVec3 vNewMin = m_vMin.CompMul(vScale);
  const wdVec3 vNewMax = m_vMax.CompMul(vScale);

  // this is necessary for negative scalings to work as expected
  m_vMin = vNewMin.CompMin(vNewMax);
  m_vMax = vNewMin.CompMax(vNewMax);
}

template <typename Type>
void wdBoundingBoxTemplate<Type>::TransformFromCenter(const wdMat4Template<Type>& mTransform)
{
  wdVec3Template<Type> vCorners[8];
  GetCorners(vCorners);

  const wdVec3Template<Type> vCenter = GetCenter();
  SetInvalid();

  for (wdUInt32 i = 0; i < 8; ++i)
    ExpandToInclude(vCenter + mTransform.TransformPosition(vCorners[i] - vCenter));
}

template <typename Type>
void wdBoundingBoxTemplate<Type>::TransformFromOrigin(const wdMat4Template<Type>& mTransform)
{
  wdVec3Template<Type> vCorners[8];
  GetCorners(vCorners);

  mTransform.TransformPosition(vCorners, 8);

  SetInvalid();
  ExpandToInclude(vCorners, 8);
}

template <typename Type>
WD_FORCE_INLINE const wdVec3Template<Type> wdBoundingBoxTemplate<Type>::GetClampedPoint(const wdVec3Template<Type>& vPoint) const
{
  return vPoint.CompMin(m_vMax).CompMax(m_vMin);
}

template <typename Type>
Type wdBoundingBoxTemplate<Type>::GetDistanceTo(const wdVec3Template<Type>& vPoint) const
{
  const wdVec3Template<Type> vClamped = GetClampedPoint(vPoint);

  return (vPoint - vClamped).GetLength();
}

template <typename Type>
Type wdBoundingBoxTemplate<Type>::GetDistanceSquaredTo(const wdVec3Template<Type>& vPoint) const
{
  const wdVec3Template<Type> vClamped = GetClampedPoint(vPoint);

  return (vPoint - vClamped).GetLengthSquared();
}

template <typename Type>
Type wdBoundingBoxTemplate<Type>::GetDistanceSquaredTo(const wdBoundingBoxTemplate<Type>& rhs) const
{
  // This will return zero for overlapping boxes

  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  Type fDistSQR = 0.0f;

  {
    if (rhs.m_vMin.x > m_vMax.x)
    {
      fDistSQR += wdMath::Square(rhs.m_vMin.x - m_vMax.x);
    }
    else if (rhs.m_vMax.x < m_vMin.x)
    {
      fDistSQR += wdMath::Square(m_vMin.x - rhs.m_vMax.x);
    }
  }

  {
    if (rhs.m_vMin.y > m_vMax.y)
    {
      fDistSQR += wdMath::Square(rhs.m_vMin.y - m_vMax.y);
    }
    else if (rhs.m_vMax.y < m_vMin.y)
    {
      fDistSQR += wdMath::Square(m_vMin.y - rhs.m_vMax.y);
    }
  }

  {
    if (rhs.m_vMin.z > m_vMax.z)
    {
      fDistSQR += wdMath::Square(rhs.m_vMin.z - m_vMax.z);
    }
    else if (rhs.m_vMax.z < m_vMin.z)
    {
      fDistSQR += wdMath::Square(m_vMin.z - rhs.m_vMax.z);
    }
  }

  return fDistSQR;
}

template <typename Type>
Type wdBoundingBoxTemplate<Type>::GetDistanceTo(const wdBoundingBoxTemplate<Type>& rhs) const
{
  return wdMath::Sqrt(GetDistanceSquaredTo(rhs));
}

template <typename Type>
bool wdBoundingBoxTemplate<Type>::GetRayIntersection(
  const wdVec3Template<Type>& vStartPos, const wdVec3Template<Type>& vRayDir, Type* out_pIntersectionDistance, wdVec3Template<Type>* out_pIntersection) const
{
  // This code was taken from: http://people.csail.mit.edu/amy/papers/box-jgt.pdf
  // "An Efficient and Robust Ray-Box Intersection Algorithm"
  // Contrary to previous implementation, this one actually works with ray/box configurations
  // that produce division by zero and multiplication with infinity (which can produce NaNs).

  WD_ASSERT_DEBUG(wdMath::SupportsInfinity<Type>(), "This type does not support infinite values, which is required for this algorithm.");
  WD_ASSERT_DEBUG(vStartPos.IsValid(), "Ray start position must be valid.");
  WD_ASSERT_DEBUG(vRayDir.IsValid(), "Ray direction must be valid.");

  WD_NAN_ASSERT(this);

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
bool wdBoundingBoxTemplate<Type>::GetLineSegmentIntersection(
  const wdVec3Template<Type>& vStartPos, const wdVec3Template<Type>& vEndPos, Type* out_pLineFraction, wdVec3Template<Type>* out_pIntersection) const
{
  const wdVec3Template<Type> vRayDir = vEndPos - vStartPos;

  Type fIntersection = 0.0f;
  if (!GetRayIntersection(vStartPos, vRayDir, &fIntersection, out_pIntersection))
    return false;

  if (out_pLineFraction)
    *out_pLineFraction = fIntersection;

  return fIntersection <= 1.0f;
}



#include <Foundation/Math/Implementation/AllClasses_inl.h>

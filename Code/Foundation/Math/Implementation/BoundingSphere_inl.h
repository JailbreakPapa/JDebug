#pragma once

#include <Foundation/Math/Mat4.h>

template <typename Type>
WD_FORCE_INLINE wdBoundingSphereTemplate<Type>::wdBoundingSphereTemplate()
{
#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
  // Initialize all data to NaN in debug mode to find problems with uninitialized data easier.
  // m_vCenter is already initialized to NaN by its own constructor.
  const Type TypeNaN = wdMath::NaN<Type>();
  m_fRadius = TypeNaN;
#endif
}

template <typename Type>
WD_FORCE_INLINE wdBoundingSphereTemplate<Type>::wdBoundingSphereTemplate(const wdVec3Template<Type>& vCenter, Type fRadius)
{
  m_vCenter = vCenter;
  m_fRadius = fRadius;
}

template <typename Type>
void wdBoundingSphereTemplate<Type>::SetZero()
{
  m_vCenter.SetZero();
  m_fRadius = 0.0f;
}

template <typename Type>
bool wdBoundingSphereTemplate<Type>::IsZero(Type fEpsilon /* = wdMath::DefaultEpsilon<Type>() */) const
{
  return m_vCenter.IsZero(fEpsilon) && wdMath::IsZero(m_fRadius, fEpsilon);
}

template <typename Type>
void wdBoundingSphereTemplate<Type>::SetInvalid()
{
  m_vCenter.SetZero();
  m_fRadius = -wdMath::SmallEpsilon<Type>();
}

template <typename Type>
bool wdBoundingSphereTemplate<Type>::IsValid() const
{
  return (m_vCenter.IsValid() && m_fRadius >= 0.0f);
}

template <typename Type>
bool wdBoundingSphereTemplate<Type>::IsNaN() const
{
  return (m_vCenter.IsNaN() || wdMath::IsNaN(m_fRadius));
}

template <typename Type>
WD_FORCE_INLINE void wdBoundingSphereTemplate<Type>::SetElements(const wdVec3Template<Type>& vCenter, Type fRadius)
{
  m_vCenter = vCenter;
  m_fRadius = fRadius;

  WD_ASSERT_DEBUG(IsValid(), "The sphere was created with invalid values.");
}

template <typename Type>
void wdBoundingSphereTemplate<Type>::ExpandToInclude(const wdVec3Template<Type>& vPoint)
{
  const Type fDistSQR = (vPoint - m_vCenter).GetLengthSquared();

  if (wdMath::Square(m_fRadius) < fDistSQR)
    m_fRadius = wdMath::Sqrt(fDistSQR);
}

template <typename Type>
void wdBoundingSphereTemplate<Type>::ExpandToInclude(const wdBoundingSphereTemplate<Type>& rhs)
{
  const Type fReqRadius = (rhs.m_vCenter - m_vCenter).GetLength() + rhs.m_fRadius;

  m_fRadius = wdMath::Max(m_fRadius, fReqRadius);
}

template <typename Type>
WD_FORCE_INLINE void wdBoundingSphereTemplate<Type>::Grow(Type fDiff)
{
  WD_ASSERT_DEBUG(IsValid(), "Cannot grow a sphere that is invalid.");

  m_fRadius += fDiff;

  WD_ASSERT_DEBUG(IsValid(), "The grown sphere has become invalid.");
}

template <typename Type>
bool wdBoundingSphereTemplate<Type>::IsIdentical(const wdBoundingSphereTemplate<Type>& rhs) const
{
  return (m_vCenter.IsIdentical(rhs.m_vCenter) && m_fRadius == rhs.m_fRadius);
}

template <typename Type>
bool wdBoundingSphereTemplate<Type>::IsEqual(const wdBoundingSphereTemplate<Type>& rhs, Type fEpsilon) const
{
  return (m_vCenter.IsEqual(rhs.m_vCenter, fEpsilon) && wdMath::IsEqual(m_fRadius, rhs.m_fRadius, fEpsilon));
}

template <typename Type>
WD_ALWAYS_INLINE bool operator==(const wdBoundingSphereTemplate<Type>& lhs, const wdBoundingSphereTemplate<Type>& rhs)
{
  return lhs.IsIdentical(rhs);
}

template <typename Type>
WD_ALWAYS_INLINE bool operator!=(const wdBoundingSphereTemplate<Type>& lhs, const wdBoundingSphereTemplate<Type>& rhs)
{
  return !lhs.IsIdentical(rhs);
}

template <typename Type>
WD_ALWAYS_INLINE void wdBoundingSphereTemplate<Type>::Translate(const wdVec3Template<Type>& vTranslation)
{
  m_vCenter += vTranslation;
}

template <typename Type>
WD_FORCE_INLINE void wdBoundingSphereTemplate<Type>::ScaleFromCenter(Type fScale)
{
  WD_ASSERT_DEBUG(fScale >= 0.0f, "Cannot invert the sphere.");

  m_fRadius *= fScale;

  WD_NAN_ASSERT(this);
}

template <typename Type>
void wdBoundingSphereTemplate<Type>::ScaleFromOrigin(const wdVec3Template<Type>& vScale)
{
  WD_ASSERT_DEBUG(vScale.x >= 0.0f, "Cannot invert the sphere.");
  WD_ASSERT_DEBUG(vScale.y >= 0.0f, "Cannot invert the sphere.");
  WD_ASSERT_DEBUG(vScale.z >= 0.0f, "Cannot invert the sphere.");

  m_vCenter = m_vCenter.CompMul(vScale);

  // scale the radius by the maximum scaling factor (the sphere cannot become an ellipsoid,
  // so to be a 'bounding' sphere, it should be as large as possible
  m_fRadius *= wdMath::Max(vScale.x, vScale.y, vScale.z);
}

template <typename Type>
void wdBoundingSphereTemplate<Type>::TransformFromOrigin(const wdMat4Template<Type>& mTransform)
{
  m_vCenter = mTransform.TransformPosition(m_vCenter);

  const wdVec3Template<Type> Scale = mTransform.GetScalingFactors();
  m_fRadius *= wdMath::Max(Scale.x, Scale.y, Scale.z);
}

template <typename Type>
void wdBoundingSphereTemplate<Type>::TransformFromCenter(const wdMat4Template<Type>& mTransform)
{
  m_vCenter += mTransform.GetTranslationVector();

  const wdVec3Template<Type> Scale = mTransform.GetScalingFactors();
  m_fRadius *= wdMath::Max(Scale.x, Scale.y, Scale.z);
}

template <typename Type>
Type wdBoundingSphereTemplate<Type>::GetDistanceTo(const wdVec3Template<Type>& vPoint) const
{
  return (vPoint - m_vCenter).GetLength() - m_fRadius;
}

template <typename Type>
Type wdBoundingSphereTemplate<Type>::GetDistanceTo(const wdBoundingSphereTemplate<Type>& rhs) const
{
  return (rhs.m_vCenter - m_vCenter).GetLength() - m_fRadius - rhs.m_fRadius;
}

template <typename Type>
bool wdBoundingSphereTemplate<Type>::Contains(const wdVec3Template<Type>& vPoint) const
{
  return (vPoint - m_vCenter).GetLengthSquared() <= wdMath::Square(m_fRadius);
}

template <typename Type>
bool wdBoundingSphereTemplate<Type>::Contains(const wdBoundingSphereTemplate<Type>& rhs) const
{
  return (rhs.m_vCenter - m_vCenter).GetLength() + rhs.m_fRadius <= m_fRadius;
}

template <typename Type>
bool wdBoundingSphereTemplate<Type>::Overlaps(const wdBoundingSphereTemplate<Type>& rhs) const
{
  return (rhs.m_vCenter - m_vCenter).GetLengthSquared() < wdMath::Square(rhs.m_fRadius + m_fRadius);
}

template <typename Type>
const wdVec3Template<Type> wdBoundingSphereTemplate<Type>::GetClampedPoint(const wdVec3Template<Type>& vPoint)
{
  const wdVec3Template<Type> vDir = vPoint - m_vCenter;
  const Type fDistSQR = vDir.GetLengthSquared();

  // return the point, if it is already inside the sphere
  if (fDistSQR <= wdMath::Square(m_fRadius))
    return vPoint;

  // otherwise return a point on the surface of the sphere

  const Type fLength = wdMath::Sqrt(fDistSQR);

  return m_vCenter + m_fRadius * (vDir / fLength);
}

template <typename Type>
bool wdBoundingSphereTemplate<Type>::Contains(
  const wdVec3Template<Type>* pPoints, wdUInt32 uiNumPoints, wdUInt32 uiStride /* = sizeof(wdVec3Template) */) const
{
  WD_ASSERT_DEBUG(pPoints != nullptr, "The array must not be empty.");
  WD_ASSERT_DEBUG(uiNumPoints > 0, "The array must contain at least one point.");
  WD_ASSERT_DEBUG(uiStride >= sizeof(wdVec3Template<Type>), "The data must not overlap.");

  const Type fRadiusSQR = wdMath::Square(m_fRadius);

  const wdVec3Template<Type>* pCur = &pPoints[0];

  for (wdUInt32 i = 0; i < uiNumPoints; ++i)
  {
    if ((*pCur - m_vCenter).GetLengthSquared() > fRadiusSQR)
      return false;

    pCur = wdMemoryUtils::AddByteOffset(pCur, uiStride);
  }

  return true;
}

template <typename Type>
bool wdBoundingSphereTemplate<Type>::Overlaps(
  const wdVec3Template<Type>* pPoints, wdUInt32 uiNumPoints, wdUInt32 uiStride /* = sizeof(wdVec3Template) */) const
{
  WD_ASSERT_DEBUG(pPoints != nullptr, "The array must not be empty.");
  WD_ASSERT_DEBUG(uiNumPoints > 0, "The array must contain at least one point.");
  WD_ASSERT_DEBUG(uiStride >= sizeof(wdVec3Template<Type>), "The data must not overlap.");

  const Type fRadiusSQR = wdMath::Square(m_fRadius);

  const wdVec3Template<Type>* pCur = &pPoints[0];

  for (wdUInt32 i = 0; i < uiNumPoints; ++i)
  {
    if ((*pCur - m_vCenter).GetLengthSquared() <= fRadiusSQR)
      return true;

    pCur = wdMemoryUtils::AddByteOffset(pCur, uiStride);
  }

  return false;
}

template <typename Type>
void wdBoundingSphereTemplate<Type>::SetFromPoints(
  const wdVec3Template<Type>* pPoints, wdUInt32 uiNumPoints, wdUInt32 uiStride /* = sizeof(wdVec3Template) */)
{
  WD_ASSERT_DEBUG(pPoints != nullptr, "The array must not be empty.");
  WD_ASSERT_DEBUG(uiStride >= sizeof(wdVec3Template<Type>), "The data must not overlap.");
  WD_ASSERT_DEBUG(uiNumPoints > 0, "The array must contain at least one point.");

  const wdVec3Template<Type>* pCur = &pPoints[0];

  wdVec3Template<Type> vCenter(0.0f);

  for (wdUInt32 i = 0; i < uiNumPoints; ++i)
  {
    vCenter += *pCur;
    pCur = wdMemoryUtils::AddByteOffset(pCur, uiStride);
  }

  vCenter /= (Type)uiNumPoints;

  Type fMaxDistSQR = 0.0f;

  pCur = &pPoints[0];
  for (wdUInt32 i = 0; i < uiNumPoints; ++i)
  {
    const Type fDistSQR = (*pCur - vCenter).GetLengthSquared();
    fMaxDistSQR = wdMath::Max(fMaxDistSQR, fDistSQR);

    pCur = wdMemoryUtils::AddByteOffset(pCur, uiStride);
  }

  m_vCenter = vCenter;
  m_fRadius = wdMath::Sqrt(fMaxDistSQR);

  WD_ASSERT_DEBUG(IsValid(), "The point cloud contained corrupted data.");
}

template <typename Type>
void wdBoundingSphereTemplate<Type>::ExpandToInclude(
  const wdVec3Template<Type>* pPoints, wdUInt32 uiNumPoints, wdUInt32 uiStride /* = sizeof(wdVec3Template) */)
{
  WD_ASSERT_DEBUG(pPoints != nullptr, "The array must not be empty.");
  WD_ASSERT_DEBUG(uiStride >= sizeof(wdVec3Template<Type>), "The data must not overlap.");

  const wdVec3Template<Type>* pCur = &pPoints[0];

  Type fMaxDistSQR = 0.0f;

  for (wdUInt32 i = 0; i < uiNumPoints; ++i)
  {
    const Type fDistSQR = (*pCur - m_vCenter).GetLengthSquared();
    fMaxDistSQR = wdMath::Max(fMaxDistSQR, fDistSQR);

    pCur = wdMemoryUtils::AddByteOffset(pCur, uiStride);
  }

  if (wdMath::Square(m_fRadius) < fMaxDistSQR)
    m_fRadius = wdMath::Sqrt(fMaxDistSQR);
}

template <typename Type>
Type wdBoundingSphereTemplate<Type>::GetDistanceTo(
  const wdVec3Template<Type>* pPoints, wdUInt32 uiNumPoints, wdUInt32 uiStride /* = sizeof(wdVec3Template) */) const
{
  WD_ASSERT_DEBUG(pPoints != nullptr, "The array must not be empty.");
  WD_ASSERT_DEBUG(uiNumPoints > 0, "The array must contain at least one point.");
  WD_ASSERT_DEBUG(uiStride >= sizeof(wdVec3Template<Type>), "The data must not overlap.");

  const wdVec3Template<Type>* pCur = &pPoints[0];

  Type fMinDistSQR = wdMath::MaxValue<Type>();

  for (wdUInt32 i = 0; i < uiNumPoints; ++i)
  {
    const Type fDistSQR = (*pCur - m_vCenter).GetLengthSquared();

    fMinDistSQR = wdMath::Min(fMinDistSQR, fDistSQR);

    pCur = wdMemoryUtils::AddByteOffset(pCur, uiStride);
  }

  return wdMath::Sqrt(fMinDistSQR);
}

template <typename Type>
bool wdBoundingSphereTemplate<Type>::GetRayIntersection(const wdVec3Template<Type>& vRayStartPos, const wdVec3Template<Type>& vRayDirNormalized,
  Type* out_pIntersectionDistance /* = nullptr */, wdVec3Template<Type>* out_pIntersection /* = nullptr */) const
{
  WD_ASSERT_DEBUG(vRayDirNormalized.IsNormalized(), "The ray direction must be normalized.");

  // Ugly Code taken from 'Real Time Rendering First Edition' Page 299

  const Type fRadiusSQR = wdMath::Square(m_fRadius);
  const wdVec3Template<Type> vRelPos = m_vCenter - vRayStartPos;

  const Type d = vRelPos.Dot(vRayDirNormalized);
  const Type fRelPosLenSQR = vRelPos.GetLengthSquared();

  if (d < 0.0f && fRelPosLenSQR > fRadiusSQR)
    return false;

  const Type m2 = fRelPosLenSQR - wdMath::Square(d);

  if (m2 > fRadiusSQR)
    return false;

  const Type q = wdMath::Sqrt(fRadiusSQR - m2);

  Type fIntersectionTime;

  if (fRelPosLenSQR > fRadiusSQR)
    fIntersectionTime = d - q;
  else
    fIntersectionTime = d + q;

  if (out_pIntersectionDistance)
    *out_pIntersectionDistance = fIntersectionTime;
  if (out_pIntersection)
    *out_pIntersection = vRayStartPos + vRayDirNormalized * fIntersectionTime;

  return true;
}

template <typename Type>
bool wdBoundingSphereTemplate<Type>::GetLineSegmentIntersection(const wdVec3Template<Type>& vLineStartPos, const wdVec3Template<Type>& vLineEndPos,
  Type* out_pHitFraction /* = nullptr */, wdVec3Template<Type>* out_pIntersection /* = nullptr */) const
{
  Type fIntersection = 0.0f;

  const wdVec3Template<Type> vDir = vLineEndPos - vLineStartPos;
  wdVec3Template<Type> vDirNorm = vDir;
  const Type fLen = vDirNorm.GetLengthAndNormalize();

  if (!GetRayIntersection(vLineStartPos, vDirNorm, &fIntersection))
    return false;

  if (fIntersection > fLen)
    return false;

  if (out_pHitFraction)
    *out_pHitFraction = fIntersection / fLen;

  if (out_pIntersection)
    *out_pIntersection = vLineStartPos + vDirNorm * fIntersection;

  return true;
}

#include <Foundation/Math/Implementation/AllClasses_inl.h>

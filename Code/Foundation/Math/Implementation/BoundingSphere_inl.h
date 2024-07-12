#pragma once

#include <Foundation/Math/Mat4.h>

template <typename Type>
NS_FORCE_INLINE nsBoundingSphereTemplate<Type>::nsBoundingSphereTemplate()
{
#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
  // Initialize all data to NaN in debug mode to find problems with uninitialized data easier.
  // m_vCenter is already initialized to NaN by its own constructor.
  const Type TypeNaN = nsMath::NaN<Type>();
  m_fRadius = TypeNaN;
#endif
}

template <typename Type>
NS_FORCE_INLINE nsBoundingSphereTemplate<Type> nsBoundingSphereTemplate<Type>::MakeZero()
{
  nsBoundingSphereTemplate<Type> res;
  res.m_vCenter.SetZero();
  res.m_fRadius = 0.0f;
  return res;
}

template <typename Type>
NS_FORCE_INLINE nsBoundingSphereTemplate<Type> nsBoundingSphereTemplate<Type>::MakeInvalid(const nsVec3Template<Type>& vCenter)
{
  nsBoundingSphereTemplate<Type> res;
  res.m_vCenter = vCenter;
  res.m_fRadius = -nsMath::SmallEpsilon<Type>(); // has to be very small for ExpandToInclude to work
  return res;
}

template <typename Type>
NS_FORCE_INLINE nsBoundingSphereTemplate<Type> nsBoundingSphereTemplate<Type>::MakeFromCenterAndRadius(const nsVec3Template<Type>& vCenter, Type fRadius)
{
  nsBoundingSphereTemplate<Type> res;
  res.m_vCenter = vCenter;
  res.m_fRadius = fRadius;
  NS_ASSERT_DEBUG(res.IsValid(), "The sphere was created with invalid values.");
  return res;
}

template <typename Type>
NS_FORCE_INLINE nsBoundingSphereTemplate<Type> nsBoundingSphereTemplate<Type>::MakeFromPoints(const nsVec3Template<Type>* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride /*= sizeof(nsVec3Template<Type>)*/)
{
  NS_ASSERT_DEBUG(pPoints != nullptr, "The array must not be empty.");
  NS_ASSERT_DEBUG(uiStride >= sizeof(nsVec3Template<Type>), "The data must not overlap.");
  NS_ASSERT_DEBUG(uiNumPoints > 0, "The array must contain at least one point.");

  const nsVec3Template<Type>* pCur = &pPoints[0];

  nsVec3Template<Type> vCenter(0.0f);

  for (nsUInt32 i = 0; i < uiNumPoints; ++i)
  {
    vCenter += *pCur;
    pCur = nsMemoryUtils::AddByteOffset(pCur, uiStride);
  }

  vCenter /= (Type)uiNumPoints;

  Type fMaxDistSQR = 0.0f;

  pCur = &pPoints[0];
  for (nsUInt32 i = 0; i < uiNumPoints; ++i)
  {
    const Type fDistSQR = (*pCur - vCenter).GetLengthSquared();
    fMaxDistSQR = nsMath::Max(fMaxDistSQR, fDistSQR);

    pCur = nsMemoryUtils::AddByteOffset(pCur, uiStride);
  }

  nsBoundingSphereTemplate<Type> res;
  res.m_vCenter = vCenter;
  res.m_fRadius = nsMath::Sqrt(fMaxDistSQR);

  NS_ASSERT_DEBUG(res.IsValid(), "The point cloud contained corrupted data.");

  return res;
}

template <typename Type>
bool nsBoundingSphereTemplate<Type>::IsZero(Type fEpsilon /* = nsMath::DefaultEpsilon<Type>() */) const
{
  return m_vCenter.IsZero(fEpsilon) && nsMath::IsZero(m_fRadius, fEpsilon);
}

template <typename Type>
bool nsBoundingSphereTemplate<Type>::IsValid() const
{
  return (m_vCenter.IsValid() && m_fRadius >= 0.0f);
}

template <typename Type>
bool nsBoundingSphereTemplate<Type>::IsNaN() const
{
  return (m_vCenter.IsNaN() || nsMath::IsNaN(m_fRadius));
}

template <typename Type>
void nsBoundingSphereTemplate<Type>::ExpandToInclude(const nsVec3Template<Type>& vPoint)
{
  const Type fDistSQR = (vPoint - m_vCenter).GetLengthSquared();

  if (nsMath::Square(m_fRadius) < fDistSQR)
    m_fRadius = nsMath::Sqrt(fDistSQR);
}

template <typename Type>
void nsBoundingSphereTemplate<Type>::ExpandToInclude(const nsBoundingSphereTemplate<Type>& rhs)
{
  const Type fReqRadius = (rhs.m_vCenter - m_vCenter).GetLength() + rhs.m_fRadius;

  m_fRadius = nsMath::Max(m_fRadius, fReqRadius);
}

template <typename Type>
NS_FORCE_INLINE void nsBoundingSphereTemplate<Type>::Grow(Type fDiff)
{
  NS_ASSERT_DEBUG(IsValid(), "Cannot grow a sphere that is invalid.");

  m_fRadius += fDiff;

  NS_ASSERT_DEBUG(IsValid(), "The grown sphere has become invalid.");
}

template <typename Type>
bool nsBoundingSphereTemplate<Type>::IsIdentical(const nsBoundingSphereTemplate<Type>& rhs) const
{
  return (m_vCenter.IsIdentical(rhs.m_vCenter) && m_fRadius == rhs.m_fRadius);
}

template <typename Type>
bool nsBoundingSphereTemplate<Type>::IsEqual(const nsBoundingSphereTemplate<Type>& rhs, Type fEpsilon) const
{
  return (m_vCenter.IsEqual(rhs.m_vCenter, fEpsilon) && nsMath::IsEqual(m_fRadius, rhs.m_fRadius, fEpsilon));
}

template <typename Type>
NS_ALWAYS_INLINE bool operator==(const nsBoundingSphereTemplate<Type>& lhs, const nsBoundingSphereTemplate<Type>& rhs)
{
  return lhs.IsIdentical(rhs);
}

template <typename Type>
NS_ALWAYS_INLINE bool operator!=(const nsBoundingSphereTemplate<Type>& lhs, const nsBoundingSphereTemplate<Type>& rhs)
{
  return !lhs.IsIdentical(rhs);
}

template <typename Type>
NS_ALWAYS_INLINE void nsBoundingSphereTemplate<Type>::Translate(const nsVec3Template<Type>& vTranslation)
{
  m_vCenter += vTranslation;
}

template <typename Type>
NS_FORCE_INLINE void nsBoundingSphereTemplate<Type>::ScaleFromCenter(Type fScale)
{
  NS_ASSERT_DEBUG(fScale >= 0.0f, "Cannot invert the sphere.");

  m_fRadius *= fScale;

  NS_NAN_ASSERT(this);
}

template <typename Type>
void nsBoundingSphereTemplate<Type>::ScaleFromOrigin(const nsVec3Template<Type>& vScale)
{
  NS_ASSERT_DEBUG(vScale.x >= 0.0f, "Cannot invert the sphere.");
  NS_ASSERT_DEBUG(vScale.y >= 0.0f, "Cannot invert the sphere.");
  NS_ASSERT_DEBUG(vScale.z >= 0.0f, "Cannot invert the sphere.");

  m_vCenter = m_vCenter.CompMul(vScale);

  // scale the radius by the maximum scaling factor (the sphere cannot become an ellipsoid,
  // so to be a 'bounding' sphere, it should be as large as possible
  m_fRadius *= nsMath::Max(vScale.x, vScale.y, vScale.z);
}

template <typename Type>
void nsBoundingSphereTemplate<Type>::TransformFromOrigin(const nsMat4Template<Type>& mTransform)
{
  m_vCenter = mTransform.TransformPosition(m_vCenter);

  const nsVec3Template<Type> Scale = mTransform.GetScalingFactors();
  m_fRadius *= nsMath::Max(Scale.x, Scale.y, Scale.z);
}

template <typename Type>
void nsBoundingSphereTemplate<Type>::TransformFromCenter(const nsMat4Template<Type>& mTransform)
{
  m_vCenter += mTransform.GetTranslationVector();

  const nsVec3Template<Type> Scale = mTransform.GetScalingFactors();
  m_fRadius *= nsMath::Max(Scale.x, Scale.y, Scale.z);
}

template <typename Type>
Type nsBoundingSphereTemplate<Type>::GetDistanceTo(const nsVec3Template<Type>& vPoint) const
{
  return (vPoint - m_vCenter).GetLength() - m_fRadius;
}

template <typename Type>
Type nsBoundingSphereTemplate<Type>::GetDistanceTo(const nsBoundingSphereTemplate<Type>& rhs) const
{
  return (rhs.m_vCenter - m_vCenter).GetLength() - m_fRadius - rhs.m_fRadius;
}

template <typename Type>
bool nsBoundingSphereTemplate<Type>::Contains(const nsVec3Template<Type>& vPoint) const
{
  return (vPoint - m_vCenter).GetLengthSquared() <= nsMath::Square(m_fRadius);
}

template <typename Type>
bool nsBoundingSphereTemplate<Type>::Contains(const nsBoundingSphereTemplate<Type>& rhs) const
{
  return (rhs.m_vCenter - m_vCenter).GetLength() + rhs.m_fRadius <= m_fRadius;
}

template <typename Type>
bool nsBoundingSphereTemplate<Type>::Overlaps(const nsBoundingSphereTemplate<Type>& rhs) const
{
  return (rhs.m_vCenter - m_vCenter).GetLengthSquared() < nsMath::Square(rhs.m_fRadius + m_fRadius);
}

template <typename Type>
const nsVec3Template<Type> nsBoundingSphereTemplate<Type>::GetClampedPoint(const nsVec3Template<Type>& vPoint)
{
  const nsVec3Template<Type> vDir = vPoint - m_vCenter;
  const Type fDistSQR = vDir.GetLengthSquared();

  // return the point, if it is already inside the sphere
  if (fDistSQR <= nsMath::Square(m_fRadius))
    return vPoint;

  // otherwise return a point on the surface of the sphere

  const Type fLength = nsMath::Sqrt(fDistSQR);

  return m_vCenter + m_fRadius * (vDir / fLength);
}

template <typename Type>
bool nsBoundingSphereTemplate<Type>::Contains(const nsVec3Template<Type>* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride /* = sizeof(nsVec3Template) */) const
{
  NS_ASSERT_DEBUG(pPoints != nullptr, "The array must not be empty.");
  NS_ASSERT_DEBUG(uiNumPoints > 0, "The array must contain at least one point.");
  NS_ASSERT_DEBUG(uiStride >= sizeof(nsVec3Template<Type>), "The data must not overlap.");

  const Type fRadiusSQR = nsMath::Square(m_fRadius);

  const nsVec3Template<Type>* pCur = &pPoints[0];

  for (nsUInt32 i = 0; i < uiNumPoints; ++i)
  {
    if ((*pCur - m_vCenter).GetLengthSquared() > fRadiusSQR)
      return false;

    pCur = nsMemoryUtils::AddByteOffset(pCur, uiStride);
  }

  return true;
}

template <typename Type>
bool nsBoundingSphereTemplate<Type>::Overlaps(const nsVec3Template<Type>* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride /* = sizeof(nsVec3Template) */) const
{
  NS_ASSERT_DEBUG(pPoints != nullptr, "The array must not be empty.");
  NS_ASSERT_DEBUG(uiNumPoints > 0, "The array must contain at least one point.");
  NS_ASSERT_DEBUG(uiStride >= sizeof(nsVec3Template<Type>), "The data must not overlap.");

  const Type fRadiusSQR = nsMath::Square(m_fRadius);

  const nsVec3Template<Type>* pCur = &pPoints[0];

  for (nsUInt32 i = 0; i < uiNumPoints; ++i)
  {
    if ((*pCur - m_vCenter).GetLengthSquared() <= fRadiusSQR)
      return true;

    pCur = nsMemoryUtils::AddByteOffset(pCur, uiStride);
  }

  return false;
}

template <typename Type>
void nsBoundingSphereTemplate<Type>::ExpandToInclude(const nsVec3Template<Type>* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride /* = sizeof(nsVec3Template) */)
{
  NS_ASSERT_DEBUG(pPoints != nullptr, "The array must not be empty.");
  NS_ASSERT_DEBUG(uiStride >= sizeof(nsVec3Template<Type>), "The data must not overlap.");

  const nsVec3Template<Type>* pCur = &pPoints[0];

  Type fMaxDistSQR = 0.0f;

  for (nsUInt32 i = 0; i < uiNumPoints; ++i)
  {
    const Type fDistSQR = (*pCur - m_vCenter).GetLengthSquared();
    fMaxDistSQR = nsMath::Max(fMaxDistSQR, fDistSQR);

    pCur = nsMemoryUtils::AddByteOffset(pCur, uiStride);
  }

  if (nsMath::Square(m_fRadius) < fMaxDistSQR)
    m_fRadius = nsMath::Sqrt(fMaxDistSQR);
}

template <typename Type>
Type nsBoundingSphereTemplate<Type>::GetDistanceTo(const nsVec3Template<Type>* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride /* = sizeof(nsVec3Template) */) const
{
  NS_ASSERT_DEBUG(pPoints != nullptr, "The array must not be empty.");
  NS_ASSERT_DEBUG(uiNumPoints > 0, "The array must contain at least one point.");
  NS_ASSERT_DEBUG(uiStride >= sizeof(nsVec3Template<Type>), "The data must not overlap.");

  const nsVec3Template<Type>* pCur = &pPoints[0];

  Type fMinDistSQR = nsMath::MaxValue<Type>();

  for (nsUInt32 i = 0; i < uiNumPoints; ++i)
  {
    const Type fDistSQR = (*pCur - m_vCenter).GetLengthSquared();

    fMinDistSQR = nsMath::Min(fMinDistSQR, fDistSQR);

    pCur = nsMemoryUtils::AddByteOffset(pCur, uiStride);
  }

  return nsMath::Sqrt(fMinDistSQR);
}

template <typename Type>
bool nsBoundingSphereTemplate<Type>::GetRayIntersection(const nsVec3Template<Type>& vRayStartPos, const nsVec3Template<Type>& vRayDirNormalized,
  Type* out_pIntersectionDistance /* = nullptr */, nsVec3Template<Type>* out_pIntersection /* = nullptr */) const
{
  NS_ASSERT_DEBUG(vRayDirNormalized.IsNormalized(), "The ray direction must be normalized.");

  // Ugly Code taken from 'Real Time Rendering First Edition' Page 299

  const Type fRadiusSQR = nsMath::Square(m_fRadius);
  const nsVec3Template<Type> vRelPos = m_vCenter - vRayStartPos;

  const Type d = vRelPos.Dot(vRayDirNormalized);
  const Type fRelPosLenSQR = vRelPos.GetLengthSquared();

  if (d < 0.0f && fRelPosLenSQR > fRadiusSQR)
    return false;

  const Type m2 = fRelPosLenSQR - nsMath::Square(d);

  if (m2 > fRadiusSQR)
    return false;

  const Type q = nsMath::Sqrt(fRadiusSQR - m2);

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
bool nsBoundingSphereTemplate<Type>::GetLineSegmentIntersection(const nsVec3Template<Type>& vLineStartPos, const nsVec3Template<Type>& vLineEndPos,
  Type* out_pHitFraction /* = nullptr */, nsVec3Template<Type>* out_pIntersection /* = nullptr */) const
{
  Type fIntersection = 0.0f;

  const nsVec3Template<Type> vDir = vLineEndPos - vLineStartPos;
  nsVec3Template<Type> vDirNorm = vDir;
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

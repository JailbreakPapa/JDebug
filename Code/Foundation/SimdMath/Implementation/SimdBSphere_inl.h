#pragma once

WD_ALWAYS_INLINE wdSimdBSphere::wdSimdBSphere() {}

WD_ALWAYS_INLINE wdSimdBSphere::wdSimdBSphere(const wdSimdVec4f& vCenter, const wdSimdFloat& fRadius)
{
  m_CenterAndRadius = vCenter;
  m_CenterAndRadius.SetW(fRadius);
}

WD_ALWAYS_INLINE void wdSimdBSphere::SetInvalid()
{
  m_CenterAndRadius.Set(0.0f, 0.0f, 0.0f, -wdMath::SmallEpsilon<float>());
}

WD_ALWAYS_INLINE bool wdSimdBSphere::IsValid() const
{
  return m_CenterAndRadius.IsValid<4>() && GetRadius() >= wdSimdFloat::Zero();
}

WD_ALWAYS_INLINE bool wdSimdBSphere::IsNaN() const
{
  return m_CenterAndRadius.IsNaN<4>();
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdBSphere::GetCenter() const
{
  return m_CenterAndRadius;
}

WD_ALWAYS_INLINE wdSimdFloat wdSimdBSphere::GetRadius() const
{
  return m_CenterAndRadius.w();
}

inline void wdSimdBSphere::SetFromPoints(const wdSimdVec4f* pPoints, wdUInt32 uiNumPoints, wdUInt32 uiStride)
{
  WD_ASSERT_DEBUG(pPoints != nullptr, "The array must not be empty.");
  WD_ASSERT_DEBUG(uiStride >= sizeof(wdSimdVec4f), "The data must not overlap.");
  WD_ASSERT_DEBUG(uiNumPoints > 0, "The array must contain at least one point.");

  const wdSimdVec4f* pCur = pPoints;

  wdSimdVec4f vCenter = wdSimdVec4f::ZeroVector();
  for (wdUInt32 i = 0; i < uiNumPoints; ++i)
  {
    vCenter += *pCur;
    pCur = wdMemoryUtils::AddByteOffset(pCur, uiStride);
  }

  m_CenterAndRadius = vCenter / wdSimdFloat(uiNumPoints);

  pCur = pPoints;

  wdSimdFloat fMaxDistSquare = wdSimdFloat::Zero();
  for (wdUInt32 i = 0; i < uiNumPoints; ++i)
  {
    const wdSimdFloat fDistSQR = (*pCur - m_CenterAndRadius).GetLengthSquared<3>();
    fMaxDistSquare = fMaxDistSquare.Max(fDistSQR);

    pCur = wdMemoryUtils::AddByteOffset(pCur, uiStride);
  }

  m_CenterAndRadius.SetW(fMaxDistSquare.GetSqrt());
}

inline void wdSimdBSphere::ExpandToInclude(const wdSimdVec4f& vPoint)
{
  const wdSimdFloat fDist = (vPoint - m_CenterAndRadius).GetLength<3>();

  m_CenterAndRadius.SetW(fDist.Max(GetRadius()));
}

inline void wdSimdBSphere::ExpandToInclude(const wdSimdVec4f* pPoints, wdUInt32 uiNumPoints, wdUInt32 uiStride)
{
  WD_ASSERT_DEBUG(pPoints != nullptr, "The array must not be empty.");
  WD_ASSERT_DEBUG(uiStride >= sizeof(wdSimdVec4f), "The data must not overlap.");

  const wdSimdVec4f* pCur = pPoints;

  wdSimdFloat fMaxDistSquare = wdSimdFloat::Zero();

  for (wdUInt32 i = 0; i < uiNumPoints; ++i)
  {
    const wdSimdFloat fDistSQR = (*pCur - m_CenterAndRadius).GetLengthSquared<3>();
    fMaxDistSquare = fMaxDistSquare.Max(fDistSQR);

    pCur = wdMemoryUtils::AddByteOffset(pCur, uiStride);
  }

  m_CenterAndRadius.SetW(fMaxDistSquare.GetSqrt().Max(GetRadius()));
}

inline void wdSimdBSphere::ExpandToInclude(const wdSimdBSphere& rhs)
{
  const wdSimdFloat fReqRadius = (rhs.m_CenterAndRadius - m_CenterAndRadius).GetLength<3>() + rhs.GetRadius();

  m_CenterAndRadius.SetW(fReqRadius.Max(GetRadius()));
}

inline void wdSimdBSphere::Transform(const wdSimdTransform& t)
{
  wdSimdVec4f newCenterAndRadius = t.TransformPosition(m_CenterAndRadius);
  newCenterAndRadius.SetW(t.GetMaxScale() * GetRadius());

  m_CenterAndRadius = newCenterAndRadius;
}

inline void wdSimdBSphere::Transform(const wdSimdMat4f& mMat)
{
  wdSimdFloat radius = m_CenterAndRadius.w();
  m_CenterAndRadius = mMat.TransformPosition(m_CenterAndRadius);

  wdSimdFloat maxRadius = mMat.m_col0.Dot<3>(mMat.m_col0);
  maxRadius = maxRadius.Max(mMat.m_col1.Dot<3>(mMat.m_col1));
  maxRadius = maxRadius.Max(mMat.m_col2.Dot<3>(mMat.m_col2));
  radius *= maxRadius.GetSqrt();

  m_CenterAndRadius.SetW(radius);
}

WD_ALWAYS_INLINE wdSimdFloat wdSimdBSphere::GetDistanceTo(const wdSimdVec4f& vPoint) const
{
  return (vPoint - m_CenterAndRadius).GetLength<3>() - GetRadius();
}

WD_ALWAYS_INLINE wdSimdFloat wdSimdBSphere::GetDistanceTo(const wdSimdBSphere& rhs) const
{
  return (rhs.m_CenterAndRadius - m_CenterAndRadius).GetLength<3>() - GetRadius() - rhs.GetRadius();
}

WD_ALWAYS_INLINE bool wdSimdBSphere::Contains(const wdSimdVec4f& vPoint) const
{
  wdSimdFloat radius = GetRadius();
  return (vPoint - m_CenterAndRadius).GetLengthSquared<3>() <= (radius * radius);
}

WD_ALWAYS_INLINE bool wdSimdBSphere::Contains(const wdSimdBSphere& rhs) const
{
  return (rhs.m_CenterAndRadius - m_CenterAndRadius).GetLength<3>() + rhs.GetRadius() <= GetRadius();
}

WD_ALWAYS_INLINE bool wdSimdBSphere::Overlaps(const wdSimdBSphere& rhs) const
{
  wdSimdFloat radius = (rhs.m_CenterAndRadius + m_CenterAndRadius).w();
  return (rhs.m_CenterAndRadius - m_CenterAndRadius).GetLengthSquared<3>() < (radius * radius);
}

inline wdSimdVec4f wdSimdBSphere::GetClampedPoint(const wdSimdVec4f& vPoint)
{
  wdSimdVec4f vDir = vPoint - m_CenterAndRadius;
  wdSimdFloat fDist = vDir.GetLengthAndNormalize<3>().Min(GetRadius());

  return m_CenterAndRadius + (vDir * fDist);
}

WD_ALWAYS_INLINE bool wdSimdBSphere::operator==(const wdSimdBSphere& rhs) const
{
  return (m_CenterAndRadius == rhs.m_CenterAndRadius).AllSet();
}

WD_ALWAYS_INLINE bool wdSimdBSphere::operator!=(const wdSimdBSphere& rhs) const
{
  return (m_CenterAndRadius != rhs.m_CenterAndRadius).AnySet();
}

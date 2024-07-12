#pragma once

NS_ALWAYS_INLINE nsSimdBSphere::nsSimdBSphere() = default;

NS_ALWAYS_INLINE nsSimdBSphere::nsSimdBSphere(const nsSimdVec4f& vCenter, const nsSimdFloat& fRadius)
  : m_CenterAndRadius(vCenter)
{
  m_CenterAndRadius.SetW(fRadius);
}

NS_ALWAYS_INLINE nsSimdBSphere nsSimdBSphere::MakeZero()
{
  nsSimdBSphere res;
  res.m_CenterAndRadius = nsSimdVec4f::MakeZero();
  return res;
}

NS_ALWAYS_INLINE nsSimdBSphere nsSimdBSphere::MakeInvalid(const nsSimdVec4f& vCenter /*= nsSimdVec4f::MakeZero()*/)
{
  nsSimdBSphere res;
  res.m_CenterAndRadius.Set(0.0f, 0.0f, 0.0f, -nsMath::SmallEpsilon<float>());
  return res;
}

NS_ALWAYS_INLINE nsSimdBSphere nsSimdBSphere::MakeFromCenterAndRadius(const nsSimdVec4f& vCenter, const nsSimdFloat& fRadius)
{
  return nsSimdBSphere(vCenter, fRadius);
}

inline nsSimdBSphere nsSimdBSphere::MakeFromPoints(const nsSimdVec4f* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride /*= sizeof(nsSimdVec4f)*/)
{
  NS_ASSERT_DEBUG(pPoints != nullptr, "The array must not be empty.");
  NS_ASSERT_DEBUG(uiStride >= sizeof(nsSimdVec4f), "The data must not overlap.");
  NS_ASSERT_DEBUG(uiNumPoints > 0, "The array must contain at least one point.");

  nsSimdBSphere res;

  const nsSimdVec4f* pCur = pPoints;

  nsSimdVec4f vCenter = nsSimdVec4f::MakeZero();
  for (nsUInt32 i = 0; i < uiNumPoints; ++i)
  {
    vCenter += *pCur;
    pCur = nsMemoryUtils::AddByteOffset(pCur, uiStride);
  }

  res.m_CenterAndRadius = vCenter / nsSimdFloat(uiNumPoints);

  pCur = pPoints;

  nsSimdFloat fMaxDistSquare = nsSimdFloat::MakeZero();
  for (nsUInt32 i = 0; i < uiNumPoints; ++i)
  {
    const nsSimdFloat fDistSQR = (*pCur - res.m_CenterAndRadius).GetLengthSquared<3>();
    fMaxDistSquare = fMaxDistSquare.Max(fDistSQR);

    pCur = nsMemoryUtils::AddByteOffset(pCur, uiStride);
  }

  res.m_CenterAndRadius.SetW(fMaxDistSquare.GetSqrt());

  return res;
}

NS_ALWAYS_INLINE void nsSimdBSphere::SetInvalid()
{
  m_CenterAndRadius.Set(0.0f, 0.0f, 0.0f, -nsMath::SmallEpsilon<float>());
}

NS_ALWAYS_INLINE bool nsSimdBSphere::IsValid() const
{
  return m_CenterAndRadius.IsValid<4>() && GetRadius() >= nsSimdFloat::MakeZero();
}

NS_ALWAYS_INLINE bool nsSimdBSphere::IsNaN() const
{
  return m_CenterAndRadius.IsNaN<4>();
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdBSphere::GetCenter() const
{
  return m_CenterAndRadius;
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdBSphere::GetRadius() const
{
  return m_CenterAndRadius.w();
}

NS_ALWAYS_INLINE void nsSimdBSphere::SetFromPoints(const nsSimdVec4f* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride)
{
  *this = MakeFromPoints(pPoints, uiNumPoints, uiStride);
}

NS_ALWAYS_INLINE void nsSimdBSphere::ExpandToInclude(const nsSimdVec4f& vPoint)
{
  const nsSimdFloat fDist = (vPoint - m_CenterAndRadius).GetLength<3>();

  m_CenterAndRadius.SetW(fDist.Max(GetRadius()));
}

inline void nsSimdBSphere::ExpandToInclude(const nsSimdVec4f* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride)
{
  NS_ASSERT_DEBUG(pPoints != nullptr, "The array must not be empty.");
  NS_ASSERT_DEBUG(uiStride >= sizeof(nsSimdVec4f), "The data must not overlap.");

  const nsSimdVec4f* pCur = pPoints;

  nsSimdFloat fMaxDistSquare = nsSimdFloat::MakeZero();

  for (nsUInt32 i = 0; i < uiNumPoints; ++i)
  {
    const nsSimdFloat fDistSQR = (*pCur - m_CenterAndRadius).GetLengthSquared<3>();
    fMaxDistSquare = fMaxDistSquare.Max(fDistSQR);

    pCur = nsMemoryUtils::AddByteOffset(pCur, uiStride);
  }

  m_CenterAndRadius.SetW(fMaxDistSquare.GetSqrt().Max(GetRadius()));
}

NS_ALWAYS_INLINE void nsSimdBSphere::ExpandToInclude(const nsSimdBSphere& rhs)
{
  const nsSimdFloat fReqRadius = (rhs.m_CenterAndRadius - m_CenterAndRadius).GetLength<3>() + rhs.GetRadius();

  m_CenterAndRadius.SetW(fReqRadius.Max(GetRadius()));
}

inline void nsSimdBSphere::Transform(const nsSimdTransform& t)
{
  nsSimdVec4f newCenterAndRadius = t.TransformPosition(m_CenterAndRadius);
  newCenterAndRadius.SetW(t.GetMaxScale() * GetRadius());

  m_CenterAndRadius = newCenterAndRadius;
}

inline void nsSimdBSphere::Transform(const nsSimdMat4f& mMat)
{
  nsSimdFloat radius = m_CenterAndRadius.w();
  m_CenterAndRadius = mMat.TransformPosition(m_CenterAndRadius);

  nsSimdFloat maxRadius = mMat.m_col0.Dot<3>(mMat.m_col0);
  maxRadius = maxRadius.Max(mMat.m_col1.Dot<3>(mMat.m_col1));
  maxRadius = maxRadius.Max(mMat.m_col2.Dot<3>(mMat.m_col2));
  radius *= maxRadius.GetSqrt();

  m_CenterAndRadius.SetW(radius);
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdBSphere::GetDistanceTo(const nsSimdVec4f& vPoint) const
{
  return (vPoint - m_CenterAndRadius).GetLength<3>() - GetRadius();
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdBSphere::GetDistanceTo(const nsSimdBSphere& rhs) const
{
  return (rhs.m_CenterAndRadius - m_CenterAndRadius).GetLength<3>() - GetRadius() - rhs.GetRadius();
}

NS_ALWAYS_INLINE bool nsSimdBSphere::Contains(const nsSimdVec4f& vPoint) const
{
  nsSimdFloat radius = GetRadius();
  return (vPoint - m_CenterAndRadius).GetLengthSquared<3>() <= (radius * radius);
}

NS_ALWAYS_INLINE bool nsSimdBSphere::Contains(const nsSimdBSphere& rhs) const
{
  return (rhs.m_CenterAndRadius - m_CenterAndRadius).GetLength<3>() + rhs.GetRadius() <= GetRadius();
}

NS_ALWAYS_INLINE bool nsSimdBSphere::Overlaps(const nsSimdBSphere& rhs) const
{
  nsSimdFloat radius = (rhs.m_CenterAndRadius + m_CenterAndRadius).w();
  return (rhs.m_CenterAndRadius - m_CenterAndRadius).GetLengthSquared<3>() < (radius * radius);
}

inline nsSimdVec4f nsSimdBSphere::GetClampedPoint(const nsSimdVec4f& vPoint)
{
  nsSimdVec4f vDir = vPoint - m_CenterAndRadius;
  nsSimdFloat fDist = vDir.GetLengthAndNormalize<3>().Min(GetRadius());

  return m_CenterAndRadius + (vDir * fDist);
}

NS_ALWAYS_INLINE bool nsSimdBSphere::operator==(const nsSimdBSphere& rhs) const
{
  return (m_CenterAndRadius == rhs.m_CenterAndRadius).AllSet();
}

NS_ALWAYS_INLINE bool nsSimdBSphere::operator!=(const nsSimdBSphere& rhs) const
{
  return (m_CenterAndRadius != rhs.m_CenterAndRadius).AnySet();
}

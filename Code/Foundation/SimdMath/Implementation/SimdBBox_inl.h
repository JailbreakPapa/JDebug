#pragma once

NS_ALWAYS_INLINE nsSimdBBox::nsSimdBBox() = default;

NS_ALWAYS_INLINE nsSimdBBox::nsSimdBBox(const nsSimdVec4f& vMin, const nsSimdVec4f& vMax)
  : m_Min(vMin)
  , m_Max(vMax)
{
}

NS_ALWAYS_INLINE nsSimdBBox nsSimdBBox::MakeZero()
{
  return nsSimdBBox(nsSimdVec4f::MakeZero(), nsSimdVec4f::MakeZero());
}

NS_ALWAYS_INLINE nsSimdBBox nsSimdBBox::MakeInvalid()
{
  return nsSimdBBox(nsSimdVec4f(nsMath::MaxValue<float>()), nsSimdVec4f(-nsMath::MaxValue<float>()));
}

NS_ALWAYS_INLINE nsSimdBBox nsSimdBBox::MakeFromCenterAndHalfExtents(const nsSimdVec4f& vCenter, const nsSimdVec4f& vHalfExtents)
{
  return nsSimdBBox(vCenter - vHalfExtents, vCenter + vHalfExtents);
}

NS_ALWAYS_INLINE nsSimdBBox nsSimdBBox::MakeFromMinMax(const nsSimdVec4f& vMin, const nsSimdVec4f& vMax)
{
  return nsSimdBBox(vMin, vMax);
}

NS_ALWAYS_INLINE nsSimdBBox nsSimdBBox::MakeFromPoints(const nsSimdVec4f* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride /*= sizeof(nsSimdVec4f)*/)
{
  nsSimdBBox box = nsSimdBBox::MakeInvalid();
  box.ExpandToInclude(pPoints, uiNumPoints, uiStride);
  return box;
}

NS_ALWAYS_INLINE void nsSimdBBox::SetInvalid()
{
  m_Min.Set(nsMath::MaxValue<float>());
  m_Max.Set(-nsMath::MaxValue<float>());
}

NS_ALWAYS_INLINE void nsSimdBBox::SetCenterAndHalfExtents(const nsSimdVec4f& vCenter, const nsSimdVec4f& vHalfExtents)
{
  m_Min = vCenter - vHalfExtents;
  m_Max = vCenter + vHalfExtents;
}

NS_ALWAYS_INLINE void nsSimdBBox::SetFromPoints(const nsSimdVec4f* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride)
{
  *this = MakeInvalid();
  ExpandToInclude(pPoints, uiNumPoints, uiStride);
}

NS_ALWAYS_INLINE bool nsSimdBBox::IsValid() const
{
  return m_Min.IsValid<3>() && m_Max.IsValid<3>() && (m_Min <= m_Max).AllSet<3>();
}

NS_ALWAYS_INLINE bool nsSimdBBox::IsNaN() const
{
  return m_Min.IsNaN<3>() || m_Max.IsNaN<3>();
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdBBox::GetCenter() const
{
  return (m_Min + m_Max) * nsSimdFloat(0.5f);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdBBox::GetExtents() const
{
  return m_Max - m_Min;
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdBBox::GetHalfExtents() const
{
  return (m_Max - m_Min) * nsSimdFloat(0.5f);
}

NS_ALWAYS_INLINE void nsSimdBBox::ExpandToInclude(const nsSimdVec4f& vPoint)
{
  m_Min = m_Min.CompMin(vPoint);
  m_Max = m_Max.CompMax(vPoint);
}

inline void nsSimdBBox::ExpandToInclude(const nsSimdVec4f* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride)
{
  NS_ASSERT_DEBUG(pPoints != nullptr, "Array may not be nullptr.");
  NS_ASSERT_DEBUG(uiStride >= sizeof(nsSimdVec4f), "Data may not overlap.");

  const nsSimdVec4f* pCur = pPoints;

  for (nsUInt32 i = 0; i < uiNumPoints; ++i)
  {
    ExpandToInclude(*pCur);

    pCur = nsMemoryUtils::AddByteOffset(pCur, uiStride);
  }
}

NS_ALWAYS_INLINE void nsSimdBBox::ExpandToInclude(const nsSimdBBox& rhs)
{
  m_Min = m_Min.CompMin(rhs.m_Min);
  m_Max = m_Max.CompMax(rhs.m_Max);
}

inline void nsSimdBBox::ExpandToCube()
{
  const nsSimdVec4f center = GetCenter();
  const nsSimdVec4f halfExtents = center - m_Min;

  *this = nsSimdBBox::MakeFromCenterAndHalfExtents(center, nsSimdVec4f(halfExtents.HorizontalMax<3>()));
}

NS_ALWAYS_INLINE bool nsSimdBBox::Contains(const nsSimdVec4f& vPoint) const
{
  return ((vPoint >= m_Min) && (vPoint <= m_Max)).AllSet<3>();
}

NS_ALWAYS_INLINE bool nsSimdBBox::Contains(const nsSimdBBox& rhs) const
{
  return Contains(rhs.m_Min) && Contains(rhs.m_Max);
}

inline bool nsSimdBBox::Contains(const nsSimdBSphere& rhs) const
{
  const nsSimdBBox otherBox = nsSimdBBox::MakeFromCenterAndHalfExtents(rhs.GetCenter(), nsSimdVec4f(rhs.GetRadius()));

  return Contains(otherBox);
}

NS_ALWAYS_INLINE bool nsSimdBBox::Overlaps(const nsSimdBBox& rhs) const
{
  return ((m_Max > rhs.m_Min) && (m_Min < rhs.m_Max)).AllSet<3>();
}

inline bool nsSimdBBox::Overlaps(const nsSimdBSphere& rhs) const
{
  // check whether the closest point between box and sphere is inside the sphere (it is definitely inside the box)
  return rhs.Contains(GetClampedPoint(rhs.GetCenter()));
}

NS_ALWAYS_INLINE void nsSimdBBox::Grow(const nsSimdVec4f& vDiff)
{
  m_Max += vDiff;
  m_Min -= vDiff;
}

NS_ALWAYS_INLINE void nsSimdBBox::Translate(const nsSimdVec4f& vDiff)
{
  m_Min += vDiff;
  m_Max += vDiff;
}

NS_ALWAYS_INLINE void nsSimdBBox::Transform(const nsSimdTransform& t)
{
  Transform(t.GetAsMat4());
}

NS_ALWAYS_INLINE void nsSimdBBox::Transform(const nsSimdMat4f& mMat)
{
  const nsSimdVec4f center = GetCenter();
  const nsSimdVec4f halfExtents = center - m_Min;

  const nsSimdVec4f newCenter = mMat.TransformPosition(center);

  nsSimdVec4f newHalfExtents = mMat.m_col0.Abs() * halfExtents.x();
  newHalfExtents += mMat.m_col1.Abs() * halfExtents.y();
  newHalfExtents += mMat.m_col2.Abs() * halfExtents.z();

  *this = nsSimdBBox::MakeFromCenterAndHalfExtents(newCenter, newHalfExtents);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdBBox::GetClampedPoint(const nsSimdVec4f& vPoint) const
{
  return vPoint.CompMin(m_Max).CompMax(m_Min);
}

inline nsSimdFloat nsSimdBBox::GetDistanceSquaredTo(const nsSimdVec4f& vPoint) const
{
  const nsSimdVec4f vClamped = GetClampedPoint(vPoint);

  return (vPoint - vClamped).GetLengthSquared<3>();
}

inline nsSimdFloat nsSimdBBox::GetDistanceTo(const nsSimdVec4f& vPoint) const
{
  const nsSimdVec4f vClamped = GetClampedPoint(vPoint);

  return (vPoint - vClamped).GetLength<3>();
}

NS_ALWAYS_INLINE bool nsSimdBBox::operator==(const nsSimdBBox& rhs) const
{
  return ((m_Min == rhs.m_Min) && (m_Max == rhs.m_Max)).AllSet<3>();
}

NS_ALWAYS_INLINE bool nsSimdBBox::operator!=(const nsSimdBBox& rhs) const
{
  return ((m_Min != rhs.m_Min) || (m_Max != rhs.m_Max)).AnySet<3>();
}

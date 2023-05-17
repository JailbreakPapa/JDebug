#pragma once

WD_ALWAYS_INLINE wdSimdBBox::wdSimdBBox() {}

WD_ALWAYS_INLINE wdSimdBBox::wdSimdBBox(const wdSimdVec4f& vMin, const wdSimdVec4f& vMax)
{
  m_Min = vMin;
  m_Max = vMax;
}

WD_ALWAYS_INLINE void wdSimdBBox::SetInvalid()
{
  m_Min.Set(wdMath::MaxValue<float>());
  m_Max.Set(-wdMath::MaxValue<float>());
}

WD_ALWAYS_INLINE void wdSimdBBox::SetCenterAndHalfExtents(const wdSimdVec4f& vCenter, const wdSimdVec4f& vHalfExtents)
{
  m_Min = vCenter - vHalfExtents;
  m_Max = vCenter + vHalfExtents;
}

WD_ALWAYS_INLINE void wdSimdBBox::SetFromPoints(const wdSimdVec4f* pPoints, wdUInt32 uiNumPoints, wdUInt32 uiStride)
{
  SetInvalid();
  ExpandToInclude(pPoints, uiNumPoints, uiStride);
}

WD_ALWAYS_INLINE bool wdSimdBBox::IsValid() const
{
  return m_Min.IsValid<3>() && m_Max.IsValid<3>() && (m_Min <= m_Max).AllSet<3>();
}

WD_ALWAYS_INLINE bool wdSimdBBox::IsNaN() const
{
  return m_Min.IsNaN<3>() || m_Max.IsNaN<3>();
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdBBox::GetCenter() const
{
  return (m_Min + m_Max) * wdSimdFloat(0.5f);
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdBBox::GetExtents() const
{
  return m_Max - m_Min;
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdBBox::GetHalfExtents() const
{
  return (m_Max - m_Min) * wdSimdFloat(0.5f);
}

WD_ALWAYS_INLINE void wdSimdBBox::ExpandToInclude(const wdSimdVec4f& vPoint)
{
  m_Min = m_Min.CompMin(vPoint);
  m_Max = m_Max.CompMax(vPoint);
}

inline void wdSimdBBox::ExpandToInclude(const wdSimdVec4f* pPoints, wdUInt32 uiNumPoints, wdUInt32 uiStride)
{
  WD_ASSERT_DEBUG(pPoints != nullptr, "Array may not be nullptr.");
  WD_ASSERT_DEBUG(uiStride >= sizeof(wdSimdVec4f), "Data may not overlap.");

  const wdSimdVec4f* pCur = pPoints;

  for (wdUInt32 i = 0; i < uiNumPoints; ++i)
  {
    ExpandToInclude(*pCur);

    pCur = wdMemoryUtils::AddByteOffset(pCur, uiStride);
  }
}

WD_ALWAYS_INLINE void wdSimdBBox::ExpandToInclude(const wdSimdBBox& rhs)
{
  m_Min = m_Min.CompMin(rhs.m_Min);
  m_Max = m_Max.CompMax(rhs.m_Max);
}

inline void wdSimdBBox::ExpandToCube()
{
  const wdSimdVec4f center = GetCenter();
  const wdSimdVec4f halfExtents = center - m_Min;

  SetCenterAndHalfExtents(center, wdSimdVec4f(halfExtents.HorizontalMax<3>()));
}

WD_ALWAYS_INLINE bool wdSimdBBox::Contains(const wdSimdVec4f& vPoint) const
{
  return ((vPoint >= m_Min) && (vPoint <= m_Max)).AllSet<3>();
}

WD_ALWAYS_INLINE bool wdSimdBBox::Contains(const wdSimdBBox& rhs) const
{
  return Contains(rhs.m_Min) && Contains(rhs.m_Max);
}

inline bool wdSimdBBox::Contains(const wdSimdBSphere& rhs) const
{
  wdSimdBBox otherBox;
  otherBox.SetCenterAndHalfExtents(rhs.GetCenter(), wdSimdVec4f(rhs.GetRadius()));

  return Contains(otherBox);
}

WD_ALWAYS_INLINE bool wdSimdBBox::Overlaps(const wdSimdBBox& rhs) const
{
  return ((m_Max > rhs.m_Min) && (m_Min < rhs.m_Max)).AllSet<3>();
}

inline bool wdSimdBBox::Overlaps(const wdSimdBSphere& rhs) const
{
  // check whether the closest point between box and sphere is inside the sphere (it is definitely inside the box)
  return rhs.Contains(GetClampedPoint(rhs.GetCenter()));
}

WD_ALWAYS_INLINE void wdSimdBBox::Grow(const wdSimdVec4f& vDiff)
{
  m_Max += vDiff;
  m_Min -= vDiff;
}

WD_ALWAYS_INLINE void wdSimdBBox::Translate(const wdSimdVec4f& vDiff)
{
  m_Min += vDiff;
  m_Max += vDiff;
}

WD_ALWAYS_INLINE void wdSimdBBox::Transform(const wdSimdTransform& t)
{
  Transform(t.GetAsMat4());
}

WD_ALWAYS_INLINE void wdSimdBBox::Transform(const wdSimdMat4f& mMat)
{
  const wdSimdVec4f center = GetCenter();
  const wdSimdVec4f halfExtents = center - m_Min;

  const wdSimdVec4f newCenter = mMat.TransformPosition(center);

  wdSimdVec4f newHalfExtents = mMat.m_col0.Abs() * halfExtents.x();
  newHalfExtents += mMat.m_col1.Abs() * halfExtents.y();
  newHalfExtents += mMat.m_col2.Abs() * halfExtents.z();

  SetCenterAndHalfExtents(newCenter, newHalfExtents);
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdBBox::GetClampedPoint(const wdSimdVec4f& vPoint) const
{
  return vPoint.CompMin(m_Max).CompMax(m_Min);
}

inline wdSimdFloat wdSimdBBox::GetDistanceSquaredTo(const wdSimdVec4f& vPoint) const
{
  const wdSimdVec4f vClamped = GetClampedPoint(vPoint);

  return (vPoint - vClamped).GetLengthSquared<3>();
}

inline wdSimdFloat wdSimdBBox::GetDistanceTo(const wdSimdVec4f& vPoint) const
{
  const wdSimdVec4f vClamped = GetClampedPoint(vPoint);

  return (vPoint - vClamped).GetLength<3>();
}

WD_ALWAYS_INLINE bool wdSimdBBox::operator==(const wdSimdBBox& rhs) const
{
  return ((m_Min == rhs.m_Min) && (m_Max == rhs.m_Max)).AllSet<3>();
}

WD_ALWAYS_INLINE bool wdSimdBBox::operator!=(const wdSimdBBox& rhs) const
{
  return ((m_Min != rhs.m_Min) || (m_Max != rhs.m_Max)).AnySet<3>();
}

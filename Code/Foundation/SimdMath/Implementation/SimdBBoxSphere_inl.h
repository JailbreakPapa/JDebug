#pragma once

WD_ALWAYS_INLINE wdSimdBBoxSphere::wdSimdBBoxSphere() {}

WD_ALWAYS_INLINE wdSimdBBoxSphere::wdSimdBBoxSphere(const wdSimdVec4f& vCenter, const wdSimdVec4f& vBoxHalfExtents, const wdSimdFloat& fSphereRadius)
{
  m_CenterAndRadius = vCenter;
  m_CenterAndRadius.SetW(fSphereRadius);
  m_BoxHalfExtents = vBoxHalfExtents;
}

inline wdSimdBBoxSphere::wdSimdBBoxSphere(const wdSimdBBox& box, const wdSimdBSphere& sphere)
{
  m_CenterAndRadius = box.GetCenter();
  m_BoxHalfExtents = m_CenterAndRadius - box.m_Min;
  m_CenterAndRadius.SetW(m_BoxHalfExtents.GetLength<3>().Min((sphere.GetCenter() - m_CenterAndRadius).GetLength<3>() + sphere.GetRadius()));
}

inline wdSimdBBoxSphere::wdSimdBBoxSphere(const wdSimdBBox& box)
{
  m_CenterAndRadius = box.GetCenter();
  m_BoxHalfExtents = m_CenterAndRadius - box.m_Min;
  m_CenterAndRadius.SetW(m_BoxHalfExtents.GetLength<3>());
}

WD_ALWAYS_INLINE wdSimdBBoxSphere::wdSimdBBoxSphere(const wdSimdBSphere& sphere)
{
  m_CenterAndRadius = sphere.m_CenterAndRadius;
  m_BoxHalfExtents = wdSimdVec4f(sphere.GetRadius());
}

WD_ALWAYS_INLINE void wdSimdBBoxSphere::SetInvalid()
{
  m_CenterAndRadius.Set(0.0f, 0.0f, 0.0f, -wdMath::SmallEpsilon<float>());
  m_BoxHalfExtents.Set(-wdMath::MaxValue<float>());
}

WD_ALWAYS_INLINE bool wdSimdBBoxSphere::IsValid() const
{
  return m_CenterAndRadius.IsValid<4>() && m_CenterAndRadius.w() >= wdSimdFloat::Zero() && m_BoxHalfExtents.IsValid<3>() &&
         (m_BoxHalfExtents >= wdSimdVec4f::ZeroVector()).AllSet<3>();
}

inline bool wdSimdBBoxSphere::IsNaN() const
{
  return m_CenterAndRadius.IsNaN<4>() || m_BoxHalfExtents.IsNaN<3>();
}

inline void wdSimdBBoxSphere::SetFromPoints(const wdSimdVec4f* pPoints, wdUInt32 uiNumPoints, wdUInt32 uiStride)
{
  wdSimdBBox box;
  box.SetFromPoints(pPoints, uiNumPoints, uiStride);

  m_CenterAndRadius = box.GetCenter();
  m_BoxHalfExtents = m_CenterAndRadius - box.m_Min;

  wdSimdBSphere sphere(m_CenterAndRadius, wdSimdFloat::Zero());
  sphere.ExpandToInclude(pPoints, uiNumPoints, uiStride);

  m_CenterAndRadius.SetW(sphere.GetRadius());
}

WD_ALWAYS_INLINE wdSimdBBox wdSimdBBoxSphere::GetBox() const
{
  wdSimdBBox box;
  box.SetCenterAndHalfExtents(m_CenterAndRadius, m_BoxHalfExtents);
  return box;
}

WD_ALWAYS_INLINE wdSimdBSphere wdSimdBBoxSphere::GetSphere() const
{
  wdSimdBSphere sphere;
  sphere.m_CenterAndRadius = m_CenterAndRadius;
  return sphere;
}

inline void wdSimdBBoxSphere::ExpandToInclude(const wdSimdBBoxSphere& rhs)
{
  wdSimdBBox box = GetBox();
  box.ExpandToInclude(rhs.GetBox());

  wdSimdVec4f center = box.GetCenter();
  wdSimdVec4f boxHalfExtents = center - box.m_Min;
  wdSimdFloat tmpRadius = boxHalfExtents.GetLength<3>();

  const wdSimdFloat fSphereRadiusA = (m_CenterAndRadius - center).GetLength<3>() + m_CenterAndRadius.w();
  const wdSimdFloat fSphereRadiusB = (rhs.m_CenterAndRadius - center).GetLength<3>() + rhs.m_CenterAndRadius.w();

  m_CenterAndRadius = center;
  m_CenterAndRadius.SetW(tmpRadius.Min(fSphereRadiusA.Max(fSphereRadiusB)));
  m_BoxHalfExtents = boxHalfExtents;
}

WD_ALWAYS_INLINE void wdSimdBBoxSphere::Transform(const wdSimdTransform& t)
{
  Transform(t.GetAsMat4());
}

WD_ALWAYS_INLINE void wdSimdBBoxSphere::Transform(const wdSimdMat4f& mMat)
{
  wdSimdFloat radius = m_CenterAndRadius.w();
  m_CenterAndRadius = mMat.TransformPosition(m_CenterAndRadius);

  wdSimdFloat maxRadius = mMat.m_col0.Dot<3>(mMat.m_col0);
  maxRadius = maxRadius.Max(mMat.m_col1.Dot<3>(mMat.m_col1));
  maxRadius = maxRadius.Max(mMat.m_col2.Dot<3>(mMat.m_col2));
  radius *= maxRadius.GetSqrt();

  m_CenterAndRadius.SetW(radius);

  wdSimdVec4f newHalfExtents = mMat.m_col0.Abs() * m_BoxHalfExtents.x();
  newHalfExtents += mMat.m_col1.Abs() * m_BoxHalfExtents.y();
  newHalfExtents += mMat.m_col2.Abs() * m_BoxHalfExtents.z();

  m_BoxHalfExtents = newHalfExtents.CompMin(wdSimdVec4f(radius));
}

WD_ALWAYS_INLINE bool wdSimdBBoxSphere::operator==(const wdSimdBBoxSphere& rhs) const
{
  return (m_CenterAndRadius == rhs.m_CenterAndRadius).AllSet<4>() && (m_BoxHalfExtents == rhs.m_BoxHalfExtents).AllSet<3>();
}

WD_ALWAYS_INLINE bool wdSimdBBoxSphere::operator!=(const wdSimdBBoxSphere& rhs) const
{
  return !(*this == rhs);
}

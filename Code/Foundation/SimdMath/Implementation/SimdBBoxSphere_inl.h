#pragma once

NS_ALWAYS_INLINE nsSimdBBoxSphere::nsSimdBBoxSphere() = default;

NS_ALWAYS_INLINE nsSimdBBoxSphere::nsSimdBBoxSphere(const nsSimdVec4f& vCenter, const nsSimdVec4f& vBoxHalfExtents, const nsSimdFloat& fSphereRadius)
  : m_CenterAndRadius(vCenter)
  , m_BoxHalfExtents(vBoxHalfExtents)
{
  m_CenterAndRadius.SetW(fSphereRadius);
}

inline nsSimdBBoxSphere::nsSimdBBoxSphere(const nsSimdBBox& box, const nsSimdBSphere& sphere)
{
  *this = MakeFromBoxAndSphere(box, sphere);
}

inline nsSimdBBoxSphere::nsSimdBBoxSphere(const nsSimdBBox& box)
  : m_CenterAndRadius(box.GetCenter())
  , m_BoxHalfExtents(m_CenterAndRadius - box.m_Min)
{
  m_CenterAndRadius.SetW(m_BoxHalfExtents.GetLength<3>());
}

NS_ALWAYS_INLINE nsSimdBBoxSphere::nsSimdBBoxSphere(const nsSimdBSphere& sphere)
  : m_CenterAndRadius(sphere.m_CenterAndRadius)
  , m_BoxHalfExtents(nsSimdVec4f(sphere.GetRadius()))
{
}

NS_ALWAYS_INLINE nsSimdBBoxSphere nsSimdBBoxSphere::MakeZero()
{
  nsSimdBBoxSphere res;
  res.m_CenterAndRadius = nsSimdVec4f::MakeZero();
  res.m_BoxHalfExtents = nsSimdVec4f::MakeZero();
  return res;
}

NS_ALWAYS_INLINE nsSimdBBoxSphere nsSimdBBoxSphere::MakeInvalid()
{
  nsSimdBBoxSphere res;
  res.m_CenterAndRadius.Set(0.0f, 0.0f, 0.0f, -nsMath::SmallEpsilon<float>());
  res.m_BoxHalfExtents.Set(-nsMath::MaxValue<float>());
  return res;
}

NS_ALWAYS_INLINE nsSimdBBoxSphere nsSimdBBoxSphere::MakeFromCenterExtents(const nsSimdVec4f& vCenter, const nsSimdVec4f& vBoxHalfExtents, const nsSimdFloat& fSphereRadius)
{
  nsSimdBBoxSphere res;
  res.m_CenterAndRadius = vCenter;
  res.m_BoxHalfExtents = vBoxHalfExtents;
  res.m_CenterAndRadius.SetW(fSphereRadius);
  return res;
}

inline nsSimdBBoxSphere nsSimdBBoxSphere::MakeFromPoints(const nsSimdVec4f* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride /*= sizeof(nsSimdVec4f)*/)
{
  const nsSimdBBox box = nsSimdBBox::MakeFromPoints(pPoints, uiNumPoints, uiStride);

  nsSimdBBoxSphere res;

  res.m_CenterAndRadius = box.GetCenter();
  res.m_BoxHalfExtents = res.m_CenterAndRadius - box.m_Min;

  nsSimdBSphere sphere(res.m_CenterAndRadius, nsSimdFloat::MakeZero());
  sphere.ExpandToInclude(pPoints, uiNumPoints, uiStride);

  res.m_CenterAndRadius.SetW(sphere.GetRadius());

  return res;
}

NS_ALWAYS_INLINE nsSimdBBoxSphere nsSimdBBoxSphere::MakeFromBox(const nsSimdBBox& box)
{
  return nsSimdBBoxSphere(box);
}

NS_ALWAYS_INLINE nsSimdBBoxSphere nsSimdBBoxSphere::MakeFromSphere(const nsSimdBSphere& sphere)
{
  return nsSimdBBoxSphere(sphere);
}

NS_ALWAYS_INLINE nsSimdBBoxSphere nsSimdBBoxSphere::MakeFromBoxAndSphere(const nsSimdBBox& box, const nsSimdBSphere& sphere)
{
  nsSimdBBoxSphere res;
  res.m_CenterAndRadius = box.GetCenter();
  res.m_BoxHalfExtents = res.m_CenterAndRadius - box.m_Min;
  res.m_CenterAndRadius.SetW(res.m_BoxHalfExtents.GetLength<3>().Min((sphere.GetCenter() - res.m_CenterAndRadius).GetLength<3>() + sphere.GetRadius()));
  return res;
}

NS_ALWAYS_INLINE void nsSimdBBoxSphere::SetInvalid()
{
  m_CenterAndRadius.Set(0.0f, 0.0f, 0.0f, -nsMath::SmallEpsilon<float>());
  m_BoxHalfExtents.Set(-nsMath::MaxValue<float>());
}

NS_ALWAYS_INLINE bool nsSimdBBoxSphere::IsValid() const
{
  return m_CenterAndRadius.IsValid<4>() && m_CenterAndRadius.w() >= nsSimdFloat::MakeZero() && m_BoxHalfExtents.IsValid<3>() &&
         (m_BoxHalfExtents >= nsSimdVec4f::MakeZero()).AllSet<3>();
}

inline bool nsSimdBBoxSphere::IsNaN() const
{
  return m_CenterAndRadius.IsNaN<4>() || m_BoxHalfExtents.IsNaN<3>();
}

NS_ALWAYS_INLINE void nsSimdBBoxSphere::SetFromPoints(const nsSimdVec4f* pPoints, nsUInt32 uiNumPoints, nsUInt32 uiStride)
{
  *this = MakeFromPoints(pPoints, uiNumPoints, uiStride);
}

NS_ALWAYS_INLINE nsSimdBBox nsSimdBBoxSphere::GetBox() const
{
  return nsSimdBBox::MakeFromCenterAndHalfExtents(m_CenterAndRadius, m_BoxHalfExtents);
}

NS_ALWAYS_INLINE nsSimdBSphere nsSimdBBoxSphere::GetSphere() const
{
  nsSimdBSphere sphere;
  sphere.m_CenterAndRadius = m_CenterAndRadius;
  return sphere;
}

inline void nsSimdBBoxSphere::ExpandToInclude(const nsSimdBBoxSphere& rhs)
{
  nsSimdBBox box = GetBox();
  box.ExpandToInclude(rhs.GetBox());

  nsSimdVec4f center = box.GetCenter();
  nsSimdVec4f boxHalfExtents = center - box.m_Min;
  nsSimdFloat tmpRadius = boxHalfExtents.GetLength<3>();

  const nsSimdFloat fSphereRadiusA = (m_CenterAndRadius - center).GetLength<3>() + m_CenterAndRadius.w();
  const nsSimdFloat fSphereRadiusB = (rhs.m_CenterAndRadius - center).GetLength<3>() + rhs.m_CenterAndRadius.w();

  m_CenterAndRadius = center;
  m_CenterAndRadius.SetW(tmpRadius.Min(fSphereRadiusA.Max(fSphereRadiusB)));
  m_BoxHalfExtents = boxHalfExtents;
}

NS_ALWAYS_INLINE void nsSimdBBoxSphere::Transform(const nsSimdTransform& t)
{
  Transform(t.GetAsMat4());
}

NS_ALWAYS_INLINE void nsSimdBBoxSphere::Transform(const nsSimdMat4f& mMat)
{
  nsSimdFloat radius = m_CenterAndRadius.w();
  m_CenterAndRadius = mMat.TransformPosition(m_CenterAndRadius);

  nsSimdFloat maxRadius = mMat.m_col0.Dot<3>(mMat.m_col0);
  maxRadius = maxRadius.Max(mMat.m_col1.Dot<3>(mMat.m_col1));
  maxRadius = maxRadius.Max(mMat.m_col2.Dot<3>(mMat.m_col2));
  radius *= maxRadius.GetSqrt();

  m_CenterAndRadius.SetW(radius);

  nsSimdVec4f newHalfExtents = mMat.m_col0.Abs() * m_BoxHalfExtents.x();
  newHalfExtents += mMat.m_col1.Abs() * m_BoxHalfExtents.y();
  newHalfExtents += mMat.m_col2.Abs() * m_BoxHalfExtents.z();

  m_BoxHalfExtents = newHalfExtents.CompMin(nsSimdVec4f(radius));
}

NS_ALWAYS_INLINE bool nsSimdBBoxSphere::operator==(const nsSimdBBoxSphere& rhs) const
{
  return (m_CenterAndRadius == rhs.m_CenterAndRadius).AllSet<4>() && (m_BoxHalfExtents == rhs.m_BoxHalfExtents).AllSet<3>();
}

NS_ALWAYS_INLINE bool nsSimdBBoxSphere::operator!=(const nsSimdBBoxSphere& rhs) const
{
  return !(*this == rhs);
}

#pragma once

WD_ALWAYS_INLINE wdSimdTransform::wdSimdTransform() {}

WD_ALWAYS_INLINE wdSimdTransform::wdSimdTransform(const wdSimdVec4f& vPosition, const wdSimdQuat& qRotation, const wdSimdVec4f& vScale)
{
  m_Position = vPosition;
  m_Rotation = qRotation;
  m_Scale = vScale;
}

WD_ALWAYS_INLINE wdSimdTransform::wdSimdTransform(const wdSimdQuat& qRotation)
{
  m_Position.SetZero();
  m_Rotation = qRotation;
  m_Scale.Set(1.0f);
}

WD_ALWAYS_INLINE void wdSimdTransform::SetIdentity()
{
  m_Position.SetZero();
  m_Rotation.SetIdentity();
  m_Scale.Set(1.0f);
}

// static
WD_ALWAYS_INLINE wdSimdTransform wdSimdTransform::IdentityTransform()
{
  wdSimdTransform result;
  result.SetIdentity();
  return result;
}

WD_ALWAYS_INLINE wdSimdFloat wdSimdTransform::GetMaxScale() const
{
  return m_Scale.Abs().HorizontalMax<3>();
}

WD_ALWAYS_INLINE bool wdSimdTransform::ContainsNegativeScale() const
{
  return (m_Scale.x() * m_Scale.y() * m_Scale.z()) < wdSimdFloat::Zero();
}

WD_ALWAYS_INLINE bool wdSimdTransform::ContainsUniformScale() const
{
  const wdSimdFloat fEpsilon = wdMath::DefaultEpsilon<float>();
  return m_Scale.x().IsEqual(m_Scale.y(), fEpsilon) && m_Scale.x().IsEqual(m_Scale.z(), fEpsilon);
}

WD_ALWAYS_INLINE bool wdSimdTransform::IsEqual(const wdSimdTransform& rhs, const wdSimdFloat& fEpsilon) const
{
  return m_Position.IsEqual(rhs.m_Position, fEpsilon).AllSet<3>() && m_Rotation.IsEqualRotation(rhs.m_Rotation, fEpsilon) &&
         m_Scale.IsEqual(rhs.m_Scale, fEpsilon).AllSet<3>();
}

WD_ALWAYS_INLINE void wdSimdTransform::Invert()
{
  (*this) = GetInverse();
}

WD_ALWAYS_INLINE wdSimdTransform wdSimdTransform::GetInverse() const
{
  wdSimdQuat invRot = -m_Rotation;
  wdSimdVec4f invScale = m_Scale.GetReciprocal();
  wdSimdVec4f invPos = invRot * (invScale.CompMul(-m_Position));

  return wdSimdTransform(invPos, invRot, invScale);
}

inline void wdSimdTransform::SetLocalTransform(const wdSimdTransform& globalTransformParent, const wdSimdTransform& globalTransformChild)
{
  wdSimdQuat invRot = -globalTransformParent.m_Rotation;
  wdSimdVec4f invScale = globalTransformParent.m_Scale.GetReciprocal();

  m_Position = (invRot * (globalTransformChild.m_Position - globalTransformParent.m_Position)).CompMul(invScale);
  m_Rotation = invRot * globalTransformChild.m_Rotation;
  m_Scale = invScale.CompMul(globalTransformChild.m_Scale);
}

WD_ALWAYS_INLINE void wdSimdTransform::SetGlobalTransform(const wdSimdTransform& globalTransformParent, const wdSimdTransform& localTransformChild)
{
  *this = globalTransformParent * localTransformChild;
}

WD_FORCE_INLINE wdSimdMat4f wdSimdTransform::GetAsMat4() const
{
  wdSimdMat4f result = m_Rotation.GetAsMat4();

  result.m_col0 *= m_Scale.x();
  result.m_col1 *= m_Scale.y();
  result.m_col2 *= m_Scale.z();
  result.m_col3 = m_Position;
  result.m_col3.SetW(1.0f);

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdTransform::TransformPosition(const wdSimdVec4f& v) const
{
  const wdSimdVec4f scaled = m_Scale.CompMul(v);
  const wdSimdVec4f rotated = m_Rotation * scaled;
  return m_Position + rotated;
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdTransform::TransformDirection(const wdSimdVec4f& v) const
{
  const wdSimdVec4f scaled = m_Scale.CompMul(v);
  return m_Rotation * scaled;
}

WD_ALWAYS_INLINE const wdSimdVec4f operator*(const wdSimdTransform& t, const wdSimdVec4f& v)
{
  return t.TransformPosition(v);
}

inline const wdSimdTransform operator*(const wdSimdTransform& lhs, const wdSimdTransform& rhs)
{
  wdSimdTransform t;

  t.m_Position = (lhs.m_Rotation * rhs.m_Position.CompMul(lhs.m_Scale)) + lhs.m_Position;
  t.m_Rotation = lhs.m_Rotation * rhs.m_Rotation;
  t.m_Scale = lhs.m_Scale.CompMul(rhs.m_Scale);

  return t;
}

WD_ALWAYS_INLINE void wdSimdTransform::operator*=(const wdSimdTransform& other)
{
  (*this) = (*this) * other;
}

WD_ALWAYS_INLINE const wdSimdTransform operator*(const wdSimdTransform& lhs, const wdSimdQuat& q)
{
  wdSimdTransform t;
  t.m_Position = lhs.m_Position;
  t.m_Rotation = lhs.m_Rotation * q;
  t.m_Scale = lhs.m_Scale;
  return t;
}

WD_ALWAYS_INLINE const wdSimdTransform operator*(const wdSimdQuat& q, const wdSimdTransform& rhs)
{
  wdSimdTransform t;
  t.m_Position = rhs.m_Position;
  t.m_Rotation = q * rhs.m_Rotation;
  t.m_Scale = rhs.m_Scale;
  return t;
}

WD_ALWAYS_INLINE void wdSimdTransform::operator*=(const wdSimdQuat& q)
{
  m_Rotation = m_Rotation * q;
}

WD_ALWAYS_INLINE const wdSimdTransform operator+(const wdSimdTransform& lhs, const wdSimdVec4f& v)
{
  wdSimdTransform t;

  t.m_Position = lhs.m_Position + v;
  t.m_Rotation = lhs.m_Rotation;
  t.m_Scale = lhs.m_Scale;

  return t;
}

WD_ALWAYS_INLINE const wdSimdTransform operator-(const wdSimdTransform& lhs, const wdSimdVec4f& v)
{
  wdSimdTransform t;

  t.m_Position = lhs.m_Position - v;
  t.m_Rotation = lhs.m_Rotation;
  t.m_Scale = lhs.m_Scale;

  return t;
}

WD_ALWAYS_INLINE void wdSimdTransform::operator+=(const wdSimdVec4f& v)
{
  m_Position += v;
}

WD_ALWAYS_INLINE void wdSimdTransform::operator-=(const wdSimdVec4f& v)
{
  m_Position -= v;
}

WD_ALWAYS_INLINE bool operator==(const wdSimdTransform& lhs, const wdSimdTransform& rhs)
{
  return (lhs.m_Position == rhs.m_Position).AllSet<3>() && lhs.m_Rotation == rhs.m_Rotation && (lhs.m_Scale == rhs.m_Scale).AllSet<3>();
}

WD_ALWAYS_INLINE bool operator!=(const wdSimdTransform& lhs, const wdSimdTransform& rhs)
{
  return !(lhs == rhs);
}

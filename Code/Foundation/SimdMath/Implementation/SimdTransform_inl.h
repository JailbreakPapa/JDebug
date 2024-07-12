#pragma once

NS_ALWAYS_INLINE nsSimdTransform::nsSimdTransform() = default;

NS_ALWAYS_INLINE nsSimdTransform::nsSimdTransform(const nsSimdVec4f& vPosition, const nsSimdQuat& qRotation, const nsSimdVec4f& vScale)
  : m_Position(vPosition)
  , m_Rotation(qRotation)
  , m_Scale(vScale)
{
}

NS_ALWAYS_INLINE nsSimdTransform::nsSimdTransform(const nsSimdQuat& qRotation)
  : m_Rotation(qRotation)
{
  m_Position.SetZero();
  m_Scale.Set(1.0f);
}

inline nsSimdTransform nsSimdTransform::Make(const nsSimdVec4f& vPosition, const nsSimdQuat& qRotation /*= nsSimdQuat::IdentityQuaternion()*/, const nsSimdVec4f& vScale /*= nsSimdVec4f(1.0f)*/)
{
  nsSimdTransform res;
  res.m_Position = vPosition;
  res.m_Rotation = qRotation;
  res.m_Scale = vScale;
  return res;
}

NS_ALWAYS_INLINE nsSimdTransform nsSimdTransform::MakeIdentity()
{
  nsSimdTransform res;
  res.m_Position.SetZero();
  res.m_Rotation = nsSimdQuat::MakeIdentity();
  res.m_Scale.Set(1.0f);
  return res;
}

inline nsSimdTransform nsSimdTransform::MakeLocalTransform(const nsSimdTransform& globalTransformParent, const nsSimdTransform& globalTransformChild)
{
  const nsSimdQuat invRot = -globalTransformParent.m_Rotation;
  const nsSimdVec4f invScale = globalTransformParent.m_Scale.GetReciprocal();

  nsSimdTransform res;
  res.m_Position = (invRot * (globalTransformChild.m_Position - globalTransformParent.m_Position)).CompMul(invScale);
  res.m_Rotation = invRot * globalTransformChild.m_Rotation;
  res.m_Scale = invScale.CompMul(globalTransformChild.m_Scale);
  return res;
}

NS_ALWAYS_INLINE nsSimdTransform nsSimdTransform::MakeGlobalTransform(const nsSimdTransform& globalTransformParent, const nsSimdTransform& localTransformChild)
{
  return globalTransformParent * localTransformChild;
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdTransform::GetMaxScale() const
{
  return m_Scale.Abs().HorizontalMax<3>();
}

NS_ALWAYS_INLINE bool nsSimdTransform::ContainsNegativeScale() const
{
  return (m_Scale.x() * m_Scale.y() * m_Scale.z()) < nsSimdFloat::MakeZero();
}

NS_ALWAYS_INLINE bool nsSimdTransform::ContainsUniformScale() const
{
  const nsSimdFloat fEpsilon = nsMath::DefaultEpsilon<float>();
  return m_Scale.x().IsEqual(m_Scale.y(), fEpsilon) && m_Scale.x().IsEqual(m_Scale.z(), fEpsilon);
}

NS_ALWAYS_INLINE bool nsSimdTransform::IsEqual(const nsSimdTransform& rhs, const nsSimdFloat& fEpsilon) const
{
  return m_Position.IsEqual(rhs.m_Position, fEpsilon).AllSet<3>() && m_Rotation.IsEqualRotation(rhs.m_Rotation, fEpsilon) &&
         m_Scale.IsEqual(rhs.m_Scale, fEpsilon).AllSet<3>();
}

NS_ALWAYS_INLINE void nsSimdTransform::Invert()
{
  (*this) = GetInverse();
}

NS_ALWAYS_INLINE nsSimdTransform nsSimdTransform::GetInverse() const
{
  nsSimdQuat invRot = -m_Rotation;
  nsSimdVec4f invScale = m_Scale.GetReciprocal();
  nsSimdVec4f invPos = invRot * (invScale.CompMul(-m_Position));

  return nsSimdTransform(invPos, invRot, invScale);
}

NS_FORCE_INLINE nsSimdMat4f nsSimdTransform::GetAsMat4() const
{
  nsSimdMat4f result = m_Rotation.GetAsMat4();

  result.m_col0 *= m_Scale.x();
  result.m_col1 *= m_Scale.y();
  result.m_col2 *= m_Scale.z();
  result.m_col3 = m_Position;
  result.m_col3.SetW(1.0f);

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdTransform::TransformPosition(const nsSimdVec4f& v) const
{
  const nsSimdVec4f scaled = m_Scale.CompMul(v);
  const nsSimdVec4f rotated = m_Rotation * scaled;
  return m_Position + rotated;
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdTransform::TransformDirection(const nsSimdVec4f& v) const
{
  const nsSimdVec4f scaled = m_Scale.CompMul(v);
  return m_Rotation * scaled;
}

NS_ALWAYS_INLINE const nsSimdVec4f operator*(const nsSimdTransform& t, const nsSimdVec4f& v)
{
  return t.TransformPosition(v);
}

inline const nsSimdTransform operator*(const nsSimdTransform& lhs, const nsSimdTransform& rhs)
{
  nsSimdTransform t;

  t.m_Position = (lhs.m_Rotation * rhs.m_Position.CompMul(lhs.m_Scale)) + lhs.m_Position;
  t.m_Rotation = lhs.m_Rotation * rhs.m_Rotation;
  t.m_Scale = lhs.m_Scale.CompMul(rhs.m_Scale);

  return t;
}

NS_ALWAYS_INLINE void nsSimdTransform::operator*=(const nsSimdTransform& other)
{
  (*this) = (*this) * other;
}

NS_ALWAYS_INLINE const nsSimdTransform operator*(const nsSimdTransform& lhs, const nsSimdQuat& q)
{
  nsSimdTransform t;
  t.m_Position = lhs.m_Position;
  t.m_Rotation = lhs.m_Rotation * q;
  t.m_Scale = lhs.m_Scale;
  return t;
}

NS_ALWAYS_INLINE const nsSimdTransform operator*(const nsSimdQuat& q, const nsSimdTransform& rhs)
{
  nsSimdTransform t;
  t.m_Position = rhs.m_Position;
  t.m_Rotation = q * rhs.m_Rotation;
  t.m_Scale = rhs.m_Scale;
  return t;
}

NS_ALWAYS_INLINE void nsSimdTransform::operator*=(const nsSimdQuat& q)
{
  m_Rotation = m_Rotation * q;
}

NS_ALWAYS_INLINE const nsSimdTransform operator+(const nsSimdTransform& lhs, const nsSimdVec4f& v)
{
  nsSimdTransform t;

  t.m_Position = lhs.m_Position + v;
  t.m_Rotation = lhs.m_Rotation;
  t.m_Scale = lhs.m_Scale;

  return t;
}

NS_ALWAYS_INLINE const nsSimdTransform operator-(const nsSimdTransform& lhs, const nsSimdVec4f& v)
{
  nsSimdTransform t;

  t.m_Position = lhs.m_Position - v;
  t.m_Rotation = lhs.m_Rotation;
  t.m_Scale = lhs.m_Scale;

  return t;
}

NS_ALWAYS_INLINE void nsSimdTransform::operator+=(const nsSimdVec4f& v)
{
  m_Position += v;
}

NS_ALWAYS_INLINE void nsSimdTransform::operator-=(const nsSimdVec4f& v)
{
  m_Position -= v;
}

NS_ALWAYS_INLINE bool operator==(const nsSimdTransform& lhs, const nsSimdTransform& rhs)
{
  return (lhs.m_Position == rhs.m_Position).AllSet<3>() && lhs.m_Rotation == rhs.m_Rotation && (lhs.m_Scale == rhs.m_Scale).AllSet<3>();
}

NS_ALWAYS_INLINE bool operator!=(const nsSimdTransform& lhs, const nsSimdTransform& rhs)
{
  return !(lhs == rhs);
}

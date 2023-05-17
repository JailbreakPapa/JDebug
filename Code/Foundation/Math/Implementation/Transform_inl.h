#pragma once

#include <Foundation/Math/Transform.h>

template <typename Type>
inline wdTransformTemplate<Type>::wdTransformTemplate(const wdVec3Template<Type>& vPosition, const wdQuatTemplate<Type>& qRotation, const wdVec3Template<Type>& vScale)
{
  m_vPosition = vPosition;
  m_qRotation = qRotation;
  m_vScale = vScale;
}

template <typename Type>
void wdTransformTemplate<Type>::SetFromMat4(const wdMat4Template<Type>& mMat)
{
  wdMat3Template<Type> mRot = mMat.GetRotationalPart();

  m_vPosition = mMat.GetTranslationVector();
  m_vScale = mRot.GetScalingFactors();
  mRot.SetScalingFactors(wdVec3Template<Type>(1)).IgnoreResult();
  m_qRotation.SetFromMat3(mRot);
}

template <typename Type>
inline void wdTransformTemplate<Type>::SetIdentity()
{
  m_vPosition.SetZero();
  m_qRotation.SetIdentity();
  m_vScale.Set(1);
}

// static
template <typename Type>
inline const wdTransformTemplate<Type> wdTransformTemplate<Type>::IdentityTransform()
{
  return wdTransformTemplate<Type>(wdVec3Template<Type>::ZeroVector(), wdQuatTemplate<Type>::IdentityQuaternion(), wdVec3Template<Type>(1));
}

template <typename Type>
WD_ALWAYS_INLINE Type wdTransformTemplate<Type>::GetMaxScale() const
{
  auto absScale = m_vScale.Abs();
  return wdMath::Max(absScale.x, wdMath::Max(absScale.y, absScale.z));
}

template <typename Type>
WD_ALWAYS_INLINE bool wdTransformTemplate<Type>::ContainsNegativeScale() const
{
  return (m_vScale.x * m_vScale.y * m_vScale.z) < 0.0f;
}

template <typename Type>
WD_ALWAYS_INLINE bool wdTransformTemplate<Type>::ContainsUniformScale() const
{
  const Type fEpsilon = wdMath::DefaultEpsilon<Type>();
  return wdMath::IsEqual(m_vScale.x, m_vScale.y, fEpsilon) && wdMath::IsEqual(m_vScale.x, m_vScale.z, fEpsilon);
}

template <typename Type>
inline bool wdTransformTemplate<Type>::IsIdentical(const wdTransformTemplate<Type>& rhs) const
{
  return m_vPosition.IsIdentical(rhs.m_vPosition) && (m_qRotation == rhs.m_qRotation) && m_vScale.IsIdentical(rhs.m_vScale);
}

template <typename Type>
inline bool wdTransformTemplate<Type>::IsEqual(const wdTransformTemplate<Type>& rhs, Type fEpsilon) const
{
  return m_vPosition.IsEqual(rhs.m_vPosition, fEpsilon) && m_qRotation.IsEqualRotation(rhs.m_qRotation, fEpsilon) && m_vScale.IsEqual(rhs.m_vScale, fEpsilon);
}

template <typename Type>
inline void wdTransformTemplate<Type>::SetLocalTransform(const wdTransformTemplate<Type>& globalTransformParent, const wdTransformTemplate<Type>& globalTransformChild)
{
  const auto invRot = -globalTransformParent.m_qRotation;
  const auto invScale = wdVec3Template<Type>(1).CompDiv(globalTransformParent.m_vScale);

  m_vPosition = (invRot * (globalTransformChild.m_vPosition - globalTransformParent.m_vPosition)).CompMul(invScale);
  m_qRotation = invRot * globalTransformChild.m_qRotation;
  m_vScale = invScale.CompMul(globalTransformChild.m_vScale);
}

template <typename Type>
inline void wdTransformTemplate<Type>::SetGlobalTransform(const wdTransformTemplate<Type>& globalTransformParent, const wdTransformTemplate<Type>& localTransformChild)
{
  *this = globalTransformParent * localTransformChild;
}

template <typename Type>
WD_ALWAYS_INLINE const wdMat4Template<Type> wdTransformTemplate<Type>::GetAsMat4() const
{
  auto result = m_qRotation.GetAsMat4();

  result.m_fElementsCM[0] *= m_vScale.x;
  result.m_fElementsCM[1] *= m_vScale.x;
  result.m_fElementsCM[2] *= m_vScale.x;

  result.m_fElementsCM[4] *= m_vScale.y;
  result.m_fElementsCM[5] *= m_vScale.y;
  result.m_fElementsCM[6] *= m_vScale.y;

  result.m_fElementsCM[8] *= m_vScale.z;
  result.m_fElementsCM[9] *= m_vScale.z;
  result.m_fElementsCM[10] *= m_vScale.z;

  result.m_fElementsCM[12] = m_vPosition.x;
  result.m_fElementsCM[13] = m_vPosition.y;
  result.m_fElementsCM[14] = m_vPosition.z;

  return result;
}


template <typename Type>
void wdTransformTemplate<Type>::operator+=(const wdVec3Template<Type>& v)
{
  m_vPosition += v;
}

template <typename Type>
void wdTransformTemplate<Type>::operator-=(const wdVec3Template<Type>& v)
{
  m_vPosition -= v;
}

template <typename Type>
WD_ALWAYS_INLINE wdVec3Template<Type> wdTransformTemplate<Type>::TransformPosition(const wdVec3Template<Type>& v) const
{
  const auto scaled = m_vScale.CompMul(v);
  const auto rotated = m_qRotation * scaled;
  return m_vPosition + rotated;
}

template <typename Type>
WD_ALWAYS_INLINE wdVec3Template<Type> wdTransformTemplate<Type>::TransformDirection(const wdVec3Template<Type>& v) const
{
  const auto scaled = m_vScale.CompMul(v);
  const auto rotated = m_qRotation * scaled;
  return rotated;
}

template <typename Type>
WD_ALWAYS_INLINE const wdTransformTemplate<Type> operator*(const wdQuatTemplate<Type>& q, const wdTransformTemplate<Type>& t)
{
  wdTransform r;

  r.m_vPosition = t.m_vPosition;
  r.m_qRotation = q * t.m_qRotation;
  r.m_vScale = t.m_vScale;

  return r;
}

template <typename Type>
WD_ALWAYS_INLINE const wdTransformTemplate<Type> operator*(const wdTransformTemplate<Type>& t, const wdQuatTemplate<Type>& q)
{
  wdTransform r;

  r.m_vPosition = t.m_vPosition;
  r.m_qRotation = t.m_qRotation * q;
  r.m_vScale = t.m_vScale;

  return r;
}

template <typename Type>
WD_ALWAYS_INLINE const wdTransformTemplate<Type> operator+(const wdTransformTemplate<Type>& t, const wdVec3Template<Type>& v)
{
  return wdTransformTemplate<Type>(t.m_vPosition + v, t.m_qRotation, t.m_vScale);
}

template <typename Type>
WD_ALWAYS_INLINE const wdTransformTemplate<Type> operator-(const wdTransformTemplate<Type>& t, const wdVec3Template<Type>& v)
{
  return wdTransformTemplate<Type>(t.m_vPosition - v, t.m_qRotation, t.m_vScale);
}

template <typename Type>
WD_ALWAYS_INLINE const wdVec3Template<Type> operator*(const wdTransformTemplate<Type>& t, const wdVec3Template<Type>& v)
{
  return t.TransformPosition(v);
}

template <typename Type>
inline const wdTransformTemplate<Type> operator*(const wdTransformTemplate<Type>& t1, const wdTransformTemplate<Type>& t2)
{
  wdTransformTemplate<Type> t;

  t.m_vPosition = (t1.m_qRotation * t2.m_vPosition.CompMul(t1.m_vScale)) + t1.m_vPosition;
  t.m_qRotation = t1.m_qRotation * t2.m_qRotation;
  t.m_vScale = t1.m_vScale.CompMul(t2.m_vScale);

  return t;
}

template <typename Type>
WD_ALWAYS_INLINE bool operator==(const wdTransformTemplate<Type>& t1, const wdTransformTemplate<Type>& t2)
{
  return t1.IsIdentical(t2);
}

template <typename Type>
WD_ALWAYS_INLINE bool operator!=(const wdTransformTemplate<Type>& t1, const wdTransformTemplate<Type>& t2)
{
  return !t1.IsIdentical(t2);
}

template <typename Type>
WD_ALWAYS_INLINE void wdTransformTemplate<Type>::Invert()
{
  (*this) = GetInverse();
}

template <typename Type>
inline const wdTransformTemplate<Type> wdTransformTemplate<Type>::GetInverse() const
{
  const auto invRot = -m_qRotation;
  const auto invScale = wdVec3Template<Type>(1).CompDiv(m_vScale);
  const auto invPos = invRot * (invScale.CompMul(-m_vPosition));

  return wdTransformTemplate<Type>(invPos, invRot, invScale);
}

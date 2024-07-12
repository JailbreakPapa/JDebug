#pragma once

#include <Foundation/Math/Transform.h>

template <typename Type>
inline nsTransformTemplate<Type>::nsTransformTemplate(const nsVec3Template<Type>& vPosition,
  const nsQuatTemplate<Type>& qRotation, const nsVec3Template<Type>& vScale)
  : m_vPosition(vPosition)
  , m_qRotation(qRotation)
  , m_vScale(vScale)
{
}

template <typename Type>
inline nsTransformTemplate<Type> nsTransformTemplate<Type>::Make(const nsVec3Template<Type>& vPosition, const nsQuatTemplate<Type>& qRotation /*= nsQuatTemplate<Type>::IdentityQuaternion()*/, const nsVec3Template<Type>& vScale /*= nsVec3Template<Type>(1)*/)
{
  nsTransformTemplate<Type> res;
  res.m_vPosition = vPosition;
  res.m_qRotation = qRotation;
  res.m_vScale = vScale;
  return res;
}

template <typename Type>
inline nsTransformTemplate<Type> nsTransformTemplate<Type>::MakeIdentity()
{
  nsTransformTemplate<Type> res;
  res.m_vPosition.SetZero();
  res.m_qRotation = nsQuatTemplate<Type>::MakeIdentity();
  res.m_vScale.Set(1.0f);
  return res;
}

template <typename Type>
nsTransformTemplate<Type> nsTransformTemplate<Type>::MakeFromMat4(const nsMat4Template<Type>& mMat)
{
  nsMat3Template<Type> mRot = mMat.GetRotationalPart();

  nsTransformTemplate<Type> res;
  res.m_vPosition = mMat.GetTranslationVector();
  res.m_vScale = mRot.GetScalingFactors();
  mRot.SetScalingFactors(nsVec3Template<Type>(1)).IgnoreResult();
  res.m_qRotation = nsQuat::MakeFromMat3(mRot);
  return res;
}

template <typename Type>
nsTransformTemplate<Type> nsTransformTemplate<Type>::MakeLocalTransform(const nsTransformTemplate& globalTransformParent, const nsTransformTemplate& globalTransformChild)
{
  const auto invRot = globalTransformParent.m_qRotation.GetInverse();
  const auto invScale = nsVec3Template<Type>(1).CompDiv(globalTransformParent.m_vScale);

  nsTransformTemplate<Type> res;
  res.m_vPosition = (invRot * (globalTransformChild.m_vPosition - globalTransformParent.m_vPosition)).CompMul(invScale);
  res.m_qRotation = invRot * globalTransformChild.m_qRotation;
  res.m_vScale = invScale.CompMul(globalTransformChild.m_vScale);
  return res;
}

template <typename Type>
NS_ALWAYS_INLINE nsTransformTemplate<Type> nsTransformTemplate<Type>::MakeGlobalTransform(const nsTransformTemplate& globalTransformParent, const nsTransformTemplate& localTransformChild)
{
  return globalTransformParent * localTransformChild;
}

template <typename Type>
NS_ALWAYS_INLINE void nsTransformTemplate<Type>::SetIdentity()
{
  *this = MakeIdentity();
}

template <typename Type>
NS_ALWAYS_INLINE Type nsTransformTemplate<Type>::GetMaxScale() const
{
  auto absScale = m_vScale.Abs();
  return nsMath::Max(absScale.x, nsMath::Max(absScale.y, absScale.z));
}

template <typename Type>
NS_ALWAYS_INLINE bool nsTransformTemplate<Type>::ContainsNegativeScale() const
{
  return (m_vScale.x * m_vScale.y * m_vScale.z) < 0.0f;
}

template <typename Type>
NS_ALWAYS_INLINE bool nsTransformTemplate<Type>::ContainsUniformScale() const
{
  const Type fEpsilon = nsMath::DefaultEpsilon<Type>();
  return nsMath::IsEqual(m_vScale.x, m_vScale.y, fEpsilon) && nsMath::IsEqual(m_vScale.x, m_vScale.z, fEpsilon);
}

template <typename Type>
inline bool nsTransformTemplate<Type>::IsIdentical(const nsTransformTemplate<Type>& rhs) const
{
  return m_vPosition.IsIdentical(rhs.m_vPosition) && (m_qRotation == rhs.m_qRotation) && m_vScale.IsIdentical(rhs.m_vScale);
}

template <typename Type>
inline bool nsTransformTemplate<Type>::IsEqual(const nsTransformTemplate<Type>& rhs, Type fEpsilon) const
{
  return m_vPosition.IsEqual(rhs.m_vPosition, fEpsilon) && m_qRotation.IsEqualRotation(rhs.m_qRotation, fEpsilon) && m_vScale.IsEqual(rhs.m_vScale, fEpsilon);
}

template <typename Type>
inline bool nsTransformTemplate<Type>::IsValid() const
{
  return m_vPosition.IsValid() && m_qRotation.IsValid(0.005f) && m_vScale.IsValid();
}

template <typename Type>
NS_ALWAYS_INLINE const nsMat4Template<Type> nsTransformTemplate<Type>::GetAsMat4() const
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
NS_ALWAYS_INLINE void nsTransformTemplate<Type>::operator+=(const nsVec3Template<Type>& v)
{
  m_vPosition += v;
}

template <typename Type>
NS_ALWAYS_INLINE void nsTransformTemplate<Type>::operator-=(const nsVec3Template<Type>& v)
{
  m_vPosition -= v;
}

template <typename Type>
NS_ALWAYS_INLINE nsVec3Template<Type> nsTransformTemplate<Type>::TransformPosition(const nsVec3Template<Type>& v) const
{
  const auto scaled = m_vScale.CompMul(v);
  const auto rotated = m_qRotation * scaled;
  return m_vPosition + rotated;
}

template <typename Type>
NS_ALWAYS_INLINE nsVec3Template<Type> nsTransformTemplate<Type>::TransformDirection(const nsVec3Template<Type>& v) const
{
  const auto scaled = m_vScale.CompMul(v);
  const auto rotated = m_qRotation * scaled;
  return rotated;
}

template <typename Type>
NS_ALWAYS_INLINE const nsTransformTemplate<Type> operator*(const nsQuatTemplate<Type>& q, const nsTransformTemplate<Type>& t)
{
  nsTransform r;

  r.m_vPosition = t.m_vPosition;
  r.m_qRotation = q * t.m_qRotation;
  r.m_vScale = t.m_vScale;

  return r;
}

template <typename Type>
NS_ALWAYS_INLINE const nsTransformTemplate<Type> operator*(const nsTransformTemplate<Type>& t, const nsQuatTemplate<Type>& q)
{
  nsTransform r;

  r.m_vPosition = t.m_vPosition;
  r.m_qRotation = t.m_qRotation * q;
  r.m_vScale = t.m_vScale;

  return r;
}

template <typename Type>
NS_ALWAYS_INLINE const nsTransformTemplate<Type> operator+(const nsTransformTemplate<Type>& t, const nsVec3Template<Type>& v)
{
  return nsTransformTemplate<Type>(t.m_vPosition + v, t.m_qRotation, t.m_vScale);
}

template <typename Type>
NS_ALWAYS_INLINE const nsTransformTemplate<Type> operator-(const nsTransformTemplate<Type>& t, const nsVec3Template<Type>& v)
{
  return nsTransformTemplate<Type>(t.m_vPosition - v, t.m_qRotation, t.m_vScale);
}

template <typename Type>
NS_ALWAYS_INLINE const nsVec3Template<Type> operator*(const nsTransformTemplate<Type>& t, const nsVec3Template<Type>& v)
{
  return t.TransformPosition(v);
}

template <typename Type>
inline const nsTransformTemplate<Type> operator*(const nsTransformTemplate<Type>& t1, const nsTransformTemplate<Type>& t2)
{
  nsTransformTemplate<Type> t;

  t.m_vPosition = (t1.m_qRotation * t2.m_vPosition.CompMul(t1.m_vScale)) + t1.m_vPosition;
  t.m_qRotation = t1.m_qRotation * t2.m_qRotation;
  t.m_vScale = t1.m_vScale.CompMul(t2.m_vScale);

  return t;
}

template <typename Type>
NS_ALWAYS_INLINE bool operator==(const nsTransformTemplate<Type>& t1, const nsTransformTemplate<Type>& t2)
{
  return t1.IsIdentical(t2);
}

template <typename Type>
NS_ALWAYS_INLINE bool operator!=(const nsTransformTemplate<Type>& t1, const nsTransformTemplate<Type>& t2)
{
  return !t1.IsIdentical(t2);
}

template <typename Type>
NS_ALWAYS_INLINE void nsTransformTemplate<Type>::Invert()
{
  (*this) = GetInverse();
}

template <typename Type>
inline const nsTransformTemplate<Type> nsTransformTemplate<Type>::GetInverse() const
{
  const auto invRot = m_qRotation.GetInverse();
  const auto invScale = nsVec3Template<Type>(1).CompDiv(m_vScale);
  const auto invPos = invRot * (invScale.CompMul(-m_vPosition));

  return nsTransformTemplate<Type>(invPos, invRot, invScale);
}

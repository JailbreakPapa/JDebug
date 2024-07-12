#pragma once

template <typename Type>
nsMat3Template<Type>::nsMat3Template()
{
#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
  // Initialize all data to NaN in debug mode to find problems with uninitialized data easier.
  const Type TypeNaN = nsMath::NaN<Type>();
  for (nsUInt32 i = 0; i < 9; ++i)
    m_fElementsCM[i] = TypeNaN;
#endif
}

template <typename Type>
void nsMat3Template<Type>::GetAsArray(Type* out_pData, nsMatrixLayout::Enum layout) const
{
  NS_NAN_ASSERT(this);

  if (layout == nsMatrixLayout::ColumnMajor)
  {
    nsMemoryUtils::Copy(out_pData, m_fElementsCM, 9);
  }
  else
  {
    for (int i = 0; i < 3; ++i)
    {
      out_pData[i * 3 + 0] = Element(0, i);
      out_pData[i * 3 + 1] = Element(1, i);
      out_pData[i * 3 + 2] = Element(2, i);
    }
  }
}

template <typename Type>
nsMat3Template<Type> nsMat3Template<Type>::MakeZero()
{
  nsMat3Template<Type> res;

  for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(res.m_fElementsCM); ++i)
    res.m_fElementsCM[i] = 0.0f;

  return res;
}

template <typename Type>
nsMat3Template<Type> nsMat3Template<Type>::MakeIdentity()
{
  nsMat3Template<Type> res;
  res.m_fElementsCM[0] = 1.0f;
  res.m_fElementsCM[1] = 0.0f;
  res.m_fElementsCM[2] = 0.0f;
  res.m_fElementsCM[3] = 0.0f;
  res.m_fElementsCM[4] = 1.0f;
  res.m_fElementsCM[5] = 0.0f;
  res.m_fElementsCM[6] = 0.0f;
  res.m_fElementsCM[7] = 0.0f;
  res.m_fElementsCM[8] = 1.0f;
  return res;
}

template <typename Type>
nsMat3Template<Type> nsMat3Template<Type>::MakeFromRowMajorArray(const Type* const pData)
{
  nsMat3Template<Type> res;
  for (int i = 0; i < 3; ++i)
  {
    res.Element(0, i) = pData[i * 3 + 0];
    res.Element(1, i) = pData[i * 3 + 1];
    res.Element(2, i) = pData[i * 3 + 2];
  }
  return res;
}

template <typename Type>
nsMat3Template<Type> nsMat3Template<Type>::MakeFromColumnMajorArray(const Type* const pData)
{
  nsMat3Template<Type> res;
  nsMemoryUtils::Copy(res.m_fElementsCM, pData, 9);
  return res;
}

template <typename Type>
nsMat3Template<Type> nsMat3Template<Type>::MakeFromValues(Type c1r1, Type c2r1, Type c3r1, Type c1r2, Type c2r2, Type c3r2, Type c1r3, Type c2r3, Type c3r3)
{
  nsMat3Template<Type> res;
  res.Element(0, 0) = c1r1;
  res.Element(1, 0) = c2r1;
  res.Element(2, 0) = c3r1;
  res.Element(0, 1) = c1r2;
  res.Element(1, 1) = c2r2;
  res.Element(2, 1) = c3r2;
  res.Element(0, 2) = c1r3;
  res.Element(1, 2) = c2r3;
  res.Element(2, 2) = c3r3;
  return res;
}

template <typename Type>
nsMat3Template<Type> nsMat3Template<Type>::MakeScaling(const nsVec3Template<Type>& vScale)
{
  nsMat3Template<Type> res;
  res.Element(0, 0) = vScale.x;
  res.Element(1, 0) = 0;
  res.Element(2, 0) = 0;
  res.Element(0, 1) = 0;
  res.Element(1, 1) = vScale.y;
  res.Element(2, 1) = 0;
  res.Element(0, 2) = 0;
  res.Element(1, 2) = 0;
  res.Element(2, 2) = vScale.z;
  return res;
}

template <typename Type>
nsMat3Template<Type> nsMat3Template<Type>::MakeRotationX(nsAngle angle)
{
  const Type fSin = nsMath::Sin(angle);
  const Type fCos = nsMath::Cos(angle);

  return nsMat3Template<Type>::MakeFromValues(1.0f, 0.0f, 0.0f, 0.0f, fCos, -fSin, 0.0f, fSin, fCos);
}

template <typename Type>
nsMat3Template<Type> nsMat3Template<Type>::MakeRotationY(nsAngle angle)
{
  const Type fSin = nsMath::Sin(angle);
  const Type fCos = nsMath::Cos(angle);

  return nsMat3Template<Type>::MakeFromValues(fCos, 0.0f, fSin, 0.0f, 1.0f, 0.0f, -fSin, 0.0f, fCos);
}

template <typename Type>
nsMat3Template<Type> nsMat3Template<Type>::MakeRotationZ(nsAngle angle)
{
  const Type fSin = nsMath::Sin(angle);
  const Type fCos = nsMath::Cos(angle);

  return nsMat3Template<Type>::MakeFromValues(fCos, -fSin, 0.0f, fSin, fCos, 0.0f, 0.0f, 0.0f, 1.0f);
}

template <typename Type>
void nsMat3Template<Type>::SetZero()
{
  *this = MakeZero();
}

template <typename Type>
void nsMat3Template<Type>::SetIdentity()
{
  *this = MakeIdentity();
}

template <typename Type>
void nsMat3Template<Type>::Transpose()
{
  nsMath::Swap(Element(0, 1), Element(1, 0));
  nsMath::Swap(Element(0, 2), Element(2, 0));
  nsMath::Swap(Element(1, 2), Element(2, 1));
}

template <typename Type>
const nsMat3Template<Type> nsMat3Template<Type>::GetTranspose() const
{
  return nsMat3Template::MakeFromRowMajorArray(m_fElementsCM);
}

template <typename Type>
const nsMat3Template<Type> nsMat3Template<Type>::GetInverse(Type fEpsilon) const
{
  nsMat3Template<Type> Inverse = *this;
  nsResult res = Inverse.Invert(fEpsilon);
  NS_ASSERT_DEBUG(res.Succeeded(), "Could not invert the given Mat3.");
  NS_IGNORE_UNUSED(res);
  return Inverse;
}

template <typename Type>
nsVec3Template<Type> nsMat3Template<Type>::GetRow(nsUInt32 uiRow) const
{
  NS_ASSERT_DEBUG(uiRow <= 2, "Invalid Row Index {0}", uiRow);

  nsVec3Template<Type> r;
  r.x = Element(0, uiRow);
  r.y = Element(1, uiRow);
  r.z = Element(2, uiRow);

  NS_NAN_ASSERT(&r);
  return r;
}

template <typename Type>
void nsMat3Template<Type>::SetRow(nsUInt32 uiRow, const nsVec3Template<Type>& vRow)
{
  NS_ASSERT_DEBUG(uiRow <= 2, "Invalid Row Index {0}", uiRow);

  Element(0, uiRow) = vRow.x;
  Element(1, uiRow) = vRow.y;
  Element(2, uiRow) = vRow.z;
}

template <typename Type>
nsVec3Template<Type> nsMat3Template<Type>::GetColumn(nsUInt32 uiColumn) const
{
  NS_ASSERT_DEBUG(uiColumn <= 2, "Invalid Column Index {0}", uiColumn);

  nsVec3Template<Type> r;
  r.x = Element(uiColumn, 0);
  r.y = Element(uiColumn, 1);
  r.z = Element(uiColumn, 2);

  NS_NAN_ASSERT(&r);
  return r;
}

template <typename Type>
void nsMat3Template<Type>::SetColumn(nsUInt32 uiColumn, const nsVec3Template<Type>& vColumn)
{
  NS_ASSERT_DEBUG(uiColumn <= 2, "Invalid Column Index {0}", uiColumn);

  Element(uiColumn, 0) = vColumn.x;
  Element(uiColumn, 1) = vColumn.y;
  Element(uiColumn, 2) = vColumn.z;
}

template <typename Type>
nsVec3Template<Type> nsMat3Template<Type>::GetDiagonal() const
{
  NS_NAN_ASSERT(this);

  return nsVec3Template<Type>(Element(0, 0), Element(1, 1), Element(2, 2));
}

template <typename Type>
void nsMat3Template<Type>::SetDiagonal(const nsVec3Template<Type>& vDiag)
{
  Element(0, 0) = vDiag.x;
  Element(1, 1) = vDiag.y;
  Element(2, 2) = vDiag.z;
}

template <typename Type>
NS_FORCE_INLINE const nsVec3Template<Type> nsMat3Template<Type>::TransformDirection(const nsVec3Template<Type>& v) const
{
  nsVec3Template<Type> r;
  r.x = Element(0, 0) * v.x + Element(1, 0) * v.y + Element(2, 0) * v.z;
  r.y = Element(0, 1) * v.x + Element(1, 1) * v.y + Element(2, 1) * v.z;
  r.z = Element(0, 2) * v.x + Element(1, 2) * v.y + Element(2, 2) * v.z;

  NS_NAN_ASSERT(&r);
  return r;
}

template <typename Type>
NS_FORCE_INLINE void nsMat3Template<Type>::operator*=(Type f)
{
  for (nsInt32 i = 0; i < 9; ++i)
    m_fElementsCM[i] *= f;

  NS_NAN_ASSERT(this);
}

template <typename Type>
NS_FORCE_INLINE void nsMat3Template<Type>::operator/=(Type f)
{
  const Type fInv = nsMath::Invert(f);

  operator*=(fInv);
}

template <typename Type>
const nsMat3Template<Type> operator*(const nsMat3Template<Type>& m1, const nsMat3Template<Type>& m2)
{
  nsMat3Template<Type> r;
  for (nsInt32 i = 0; i < 3; ++i)
  {
    r.Element(0, i) = m1.Element(0, i) * m2.Element(0, 0) + m1.Element(1, i) * m2.Element(0, 1) + m1.Element(2, i) * m2.Element(0, 2);
    r.Element(1, i) = m1.Element(0, i) * m2.Element(1, 0) + m1.Element(1, i) * m2.Element(1, 1) + m1.Element(2, i) * m2.Element(1, 2);
    r.Element(2, i) = m1.Element(0, i) * m2.Element(2, 0) + m1.Element(1, i) * m2.Element(2, 1) + m1.Element(2, i) * m2.Element(2, 2);
  }

  NS_NAN_ASSERT(&r);
  return r;
}

template <typename Type>
NS_ALWAYS_INLINE const nsVec3Template<Type> operator*(const nsMat3Template<Type>& m, const nsVec3Template<Type>& v)
{
  return m.TransformDirection(v);
}



// *** free functions ***

template <typename Type>
NS_ALWAYS_INLINE const nsMat3Template<Type> operator*(Type f, const nsMat3Template<Type>& m1)
{
  return operator*(m1, f);
}

template <typename Type>
const nsMat3Template<Type> operator*(const nsMat3Template<Type>& m1, Type f)
{
  nsMat3Template<Type> r;

  for (nsUInt32 i = 0; i < 9; ++i)
    r.m_fElementsCM[i] = m1.m_fElementsCM[i] * f;

  NS_NAN_ASSERT(&r);

  return r;
}

template <typename Type>
NS_ALWAYS_INLINE const nsMat3Template<Type> operator/(const nsMat3Template<Type>& m1, Type f)
{
  return operator*(m1, nsMath::Invert(f));
}

template <typename Type>
const nsMat3Template<Type> operator+(const nsMat3Template<Type>& m1, const nsMat3Template<Type>& m2)
{
  nsMat3Template<Type> r;

  for (nsUInt32 i = 0; i < 9; ++i)
    r.m_fElementsCM[i] = m1.m_fElementsCM[i] + m2.m_fElementsCM[i];

  NS_NAN_ASSERT(&r);

  return r;
}

template <typename Type>
const nsMat3Template<Type> operator-(const nsMat3Template<Type>& m1, const nsMat3Template<Type>& m2)
{
  nsMat3Template<Type> r;

  for (nsUInt32 i = 0; i < 9; ++i)
    r.m_fElementsCM[i] = m1.m_fElementsCM[i] - m2.m_fElementsCM[i];

  NS_NAN_ASSERT(&r);

  return r;
}

template <typename Type>
bool nsMat3Template<Type>::IsIdentical(const nsMat3Template<Type>& rhs) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  for (nsUInt32 i = 0; i < 9; ++i)
  {
    if (m_fElementsCM[i] != rhs.m_fElementsCM[i])
      return false;
  }

  return true;
}

template <typename Type>
bool nsMat3Template<Type>::IsEqual(const nsMat3Template<Type>& rhs, Type fEpsilon) const
{
  NS_NAN_ASSERT(this);
  NS_NAN_ASSERT(&rhs);

  NS_ASSERT_DEBUG(fEpsilon >= 0.0f, "Epsilon may not be negative.");

  for (nsUInt32 i = 0; i < 9; ++i)
  {
    if (!nsMath::IsEqual(m_fElementsCM[i], rhs.m_fElementsCM[i], fEpsilon))
      return false;
  }

  return true;
}

template <typename Type>
NS_ALWAYS_INLINE bool operator==(const nsMat3Template<Type>& lhs, const nsMat3Template<Type>& rhs)
{
  return lhs.IsIdentical(rhs);
}

template <typename Type>
NS_ALWAYS_INLINE bool operator!=(const nsMat3Template<Type>& lhs, const nsMat3Template<Type>& rhs)
{
  return !lhs.IsIdentical(rhs);
}

template <typename Type>
bool nsMat3Template<Type>::IsZero(Type fEpsilon) const
{
  NS_NAN_ASSERT(this);

  for (nsUInt32 i = 0; i < 9; ++i)
  {
    if (!nsMath::IsZero(m_fElementsCM[i], fEpsilon))
      return false;
  }

  return true;
}

template <typename Type>
bool nsMat3Template<Type>::IsIdentity(Type fEpsilon) const
{
  NS_NAN_ASSERT(this);

  if (!nsMath::IsEqual(Element(0, 0), (Type)1, fEpsilon))
    return false;
  if (!nsMath::IsEqual(Element(0, 1), (Type)0, fEpsilon))
    return false;
  if (!nsMath::IsEqual(Element(0, 2), (Type)0, fEpsilon))
    return false;

  if (!nsMath::IsEqual(Element(1, 0), (Type)0, fEpsilon))
    return false;
  if (!nsMath::IsEqual(Element(1, 1), (Type)1, fEpsilon))
    return false;
  if (!nsMath::IsEqual(Element(1, 2), (Type)0, fEpsilon))
    return false;

  if (!nsMath::IsEqual(Element(2, 0), (Type)0, fEpsilon))
    return false;
  if (!nsMath::IsEqual(Element(2, 1), (Type)0, fEpsilon))
    return false;
  if (!nsMath::IsEqual(Element(2, 2), (Type)1, fEpsilon))
    return false;

  return true;
}

template <typename Type>
bool nsMat3Template<Type>::IsValid() const
{
  for (nsUInt32 i = 0; i < 9; ++i)
  {
    if (!nsMath::IsFinite(m_fElementsCM[i]))
      return false;
  }

  return true;
}

template <typename Type>
bool nsMat3Template<Type>::IsNaN() const
{
  for (nsUInt32 i = 0; i < 9; ++i)
  {
    if (nsMath::IsNaN(m_fElementsCM[i]))
      return true;
  }

  return false;
}

template <typename Type>
const nsVec3Template<Type> nsMat3Template<Type>::GetScalingFactors() const
{
  nsVec3Template<Type> v;

  v.x = nsVec3Template<Type>(Element(0, 0), Element(0, 1), Element(0, 2)).GetLength();
  v.y = nsVec3Template<Type>(Element(1, 0), Element(1, 1), Element(1, 2)).GetLength();
  v.z = nsVec3Template<Type>(Element(2, 0), Element(2, 1), Element(2, 2)).GetLength();

  NS_NAN_ASSERT(&v);
  return v;
}

template <typename Type>
nsResult nsMat3Template<Type>::SetScalingFactors(const nsVec3Template<Type>& vXYZ, Type fEpsilon /* = nsMath::DefaultEpsilon<Type>() */)
{
  nsVec3Template<Type> tx(Element(0, 0), Element(0, 1), Element(0, 2));
  nsVec3Template<Type> ty(Element(1, 0), Element(1, 1), Element(1, 2));
  nsVec3Template<Type> tz(Element(2, 0), Element(2, 1), Element(2, 2));

  if (tx.SetLength(vXYZ.x, fEpsilon) == NS_FAILURE)
    return NS_FAILURE;
  if (ty.SetLength(vXYZ.y, fEpsilon) == NS_FAILURE)
    return NS_FAILURE;
  if (tz.SetLength(vXYZ.z, fEpsilon) == NS_FAILURE)
    return NS_FAILURE;


  Element(0, 0) = tx.x;
  Element(0, 1) = tx.y;
  Element(0, 2) = tx.z;
  Element(1, 0) = ty.x;
  Element(1, 1) = ty.y;
  Element(1, 2) = ty.z;
  Element(2, 0) = tz.x;
  Element(2, 1) = tz.y;
  Element(2, 2) = tz.z;

  return NS_SUCCESS;
}

template <typename Type>
Type nsMat3Template<Type>::GetDeterminant() const
{
  // Using rule of Sarrus
  Type fDeterminant = 0;
  for (int i = 0; i < 3; i++)
  {
    fDeterminant += Element(i, 0) * Element((i + 1) % 3, 1) * Element((i + 2) % 3, 2);
    fDeterminant -= Element(i, 2) * Element((i + 1) % 3, 1) * Element((i + 2) % 3, 0);
  }
  return fDeterminant;
}

#include <Foundation/Math/Implementation/AllClasses_inl.h>

#pragma once

template <typename Type>
wdMat3Template<Type>::wdMat3Template()
{
#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
  // Initialize all data to NaN in debug mode to find problems with uninitialized data easier.
  const Type TypeNaN = wdMath::NaN<Type>();
  SetElements(TypeNaN, TypeNaN, TypeNaN, TypeNaN, TypeNaN, TypeNaN, TypeNaN, TypeNaN, TypeNaN);
#endif
}

template <typename Type>
wdMat3Template<Type>::wdMat3Template(const Type* const pData, wdMatrixLayout::Enum layout)
{
  SetFromArray(pData, layout);
}

template <typename Type>
wdMat3Template<Type>::wdMat3Template(Type c1r1, Type c2r1, Type c3r1, Type c1r2, Type c2r2, Type c3r2, Type c1r3, Type c2r3, Type c3r3)
{
  SetElements(c1r1, c2r1, c3r1, c1r2, c2r2, c3r2, c1r3, c2r3, c3r3);
}

template <typename Type>
WD_ALWAYS_INLINE const wdMat3Template<Type> wdMat3Template<Type>::IdentityMatrix()
{
  return wdMat3Template<Type>(1, 0, 0, 0, 1, 0, 0, 0, 1);
}

template <typename Type>
WD_ALWAYS_INLINE const wdMat3Template<Type> wdMat3Template<Type>::ZeroMatrix()
{
  return wdMat3Template<Type>(0, 0, 0, 0, 0, 0, 0, 0, 0);
}

template <typename Type>
void wdMat3Template<Type>::SetFromArray(const Type* const pData, wdMatrixLayout::Enum layout)
{
  if (layout == wdMatrixLayout::ColumnMajor)
  {
    wdMemoryUtils::Copy(m_fElementsCM, pData, 9);
  }
  else
  {
    for (int i = 0; i < 3; ++i)
    {
      Element(0, i) = pData[i * 3 + 0];
      Element(1, i) = pData[i * 3 + 1];
      Element(2, i) = pData[i * 3 + 2];
    }
  }
}

template <typename Type>
void wdMat3Template<Type>::GetAsArray(Type* out_pData, wdMatrixLayout::Enum layout) const
{
  WD_NAN_ASSERT(this);

  if (layout == wdMatrixLayout::ColumnMajor)
  {
    wdMemoryUtils::Copy(out_pData, m_fElementsCM, 9);
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
void wdMat3Template<Type>::SetElements(Type c1r1, Type c2r1, Type c3r1, Type c1r2, Type c2r2, Type c3r2, Type c1r3, Type c2r3, Type c3r3)
{
  Element(0, 0) = c1r1;
  Element(1, 0) = c2r1;
  Element(2, 0) = c3r1;
  Element(0, 1) = c1r2;
  Element(1, 1) = c2r2;
  Element(2, 1) = c3r2;
  Element(0, 2) = c1r3;
  Element(1, 2) = c2r3;
  Element(2, 2) = c3r3;
}

template <typename Type>
void wdMat3Template<Type>::SetZero()
{
  SetElements(0, 0, 0, 0, 0, 0, 0, 0, 0);
}

template <typename Type>
void wdMat3Template<Type>::SetIdentity()
{
  SetElements(1, 0, 0, 0, 1, 0, 0, 0, 1);
}

template <typename Type>
void wdMat3Template<Type>::SetScalingMatrix(const wdVec3Template<Type>& s)
{
  SetElements(s.x, 0, 0, 0, s.y, 0, 0, 0, s.z);
}

template <typename Type>
void wdMat3Template<Type>::SetRotationMatrixX(wdAngle angle)
{
  const Type fSin = wdMath::Sin(angle);
  const Type fCos = wdMath::Cos(angle);

  SetElements(1.0f, 0.0f, 0.0f, 0.0f, fCos, -fSin, 0.0f, fSin, fCos);
}

template <typename Type>
void wdMat3Template<Type>::SetRotationMatrixY(wdAngle angle)
{
  const Type fSin = wdMath::Sin(angle);
  const Type fCos = wdMath::Cos(angle);


  SetElements(fCos, 0.0f, fSin, 0.0f, 1.0f, 0.0f, -fSin, 0.0f, fCos);
}

template <typename Type>
void wdMat3Template<Type>::SetRotationMatrixZ(wdAngle angle)
{
  const Type fSin = wdMath::Sin(angle);
  const Type fCos = wdMath::Cos(angle);

  SetElements(fCos, -fSin, 0.0f, fSin, fCos, 0.0f, 0.0f, 0.0f, 1.0f);
}

template <typename Type>
void wdMat3Template<Type>::Transpose()
{
  wdMath::Swap(Element(0, 1), Element(1, 0));
  wdMath::Swap(Element(0, 2), Element(2, 0));
  wdMath::Swap(Element(1, 2), Element(2, 1));
}

template <typename Type>
const wdMat3Template<Type> wdMat3Template<Type>::GetTranspose() const
{
  return wdMat3Template(m_fElementsCM, wdMatrixLayout::RowMajor);
}

template <typename Type>
const wdMat3Template<Type> wdMat3Template<Type>::GetInverse(Type fEpsilon) const
{
  wdMat3Template<Type> Inverse = *this;
  wdResult res = Inverse.Invert(fEpsilon);
  WD_ASSERT_DEBUG(res.Succeeded(), "Could not invert the given Mat3.");
  WD_IGNORE_UNUSED(res);
  return Inverse;
}

template <typename Type>
wdVec3Template<Type> wdMat3Template<Type>::GetRow(wdUInt32 uiRow) const
{
  WD_ASSERT_DEBUG(uiRow <= 2, "Invalid Row Index {0}", uiRow);

  wdVec3Template<Type> r;
  r.x = Element(0, uiRow);
  r.y = Element(1, uiRow);
  r.z = Element(2, uiRow);

  WD_NAN_ASSERT(&r);
  return r;
}

template <typename Type>
void wdMat3Template<Type>::SetRow(wdUInt32 uiRow, const wdVec3Template<Type>& vRow)
{
  WD_ASSERT_DEBUG(uiRow <= 2, "Invalid Row Index {0}", uiRow);

  Element(0, uiRow) = vRow.x;
  Element(1, uiRow) = vRow.y;
  Element(2, uiRow) = vRow.z;
}

template <typename Type>
wdVec3Template<Type> wdMat3Template<Type>::GetColumn(wdUInt32 uiColumn) const
{
  WD_ASSERT_DEBUG(uiColumn <= 2, "Invalid Column Index {0}", uiColumn);

  wdVec3Template<Type> r;
  r.x = Element(uiColumn, 0);
  r.y = Element(uiColumn, 1);
  r.z = Element(uiColumn, 2);

  WD_NAN_ASSERT(&r);
  return r;
}

template <typename Type>
void wdMat3Template<Type>::SetColumn(wdUInt32 uiColumn, const wdVec3Template<Type>& vColumn)
{
  WD_ASSERT_DEBUG(uiColumn <= 2, "Invalid Column Index {0}", uiColumn);

  Element(uiColumn, 0) = vColumn.x;
  Element(uiColumn, 1) = vColumn.y;
  Element(uiColumn, 2) = vColumn.z;
}

template <typename Type>
wdVec3Template<Type> wdMat3Template<Type>::GetDiagonal() const
{
  WD_NAN_ASSERT(this);

  return wdVec3Template<Type>(Element(0, 0), Element(1, 1), Element(2, 2));
}

template <typename Type>
void wdMat3Template<Type>::SetDiagonal(const wdVec3Template<Type>& vDiag)
{
  Element(0, 0) = vDiag.x;
  Element(1, 1) = vDiag.y;
  Element(2, 2) = vDiag.z;
}

template <typename Type>
WD_FORCE_INLINE const wdVec3Template<Type> wdMat3Template<Type>::TransformDirection(const wdVec3Template<Type>& v) const
{
  wdVec3Template<Type> r;
  r.x = Element(0, 0) * v.x + Element(1, 0) * v.y + Element(2, 0) * v.z;
  r.y = Element(0, 1) * v.x + Element(1, 1) * v.y + Element(2, 1) * v.z;
  r.z = Element(0, 2) * v.x + Element(1, 2) * v.y + Element(2, 2) * v.z;

  WD_NAN_ASSERT(&r);
  return r;
}

template <typename Type>
WD_FORCE_INLINE void wdMat3Template<Type>::operator*=(Type f)
{
  for (wdInt32 i = 0; i < 9; ++i)
    m_fElementsCM[i] *= f;

  WD_NAN_ASSERT(this);
}

template <typename Type>
WD_FORCE_INLINE void wdMat3Template<Type>::operator/=(Type f)
{
  const Type fInv = wdMath::Invert(f);

  operator*=(fInv);
}

template <typename Type>
const wdMat3Template<Type> operator*(const wdMat3Template<Type>& m1, const wdMat3Template<Type>& m2)
{
  wdMat3Template<Type> r;
  for (wdInt32 i = 0; i < 3; ++i)
  {
    r.Element(0, i) = m1.Element(0, i) * m2.Element(0, 0) + m1.Element(1, i) * m2.Element(0, 1) + m1.Element(2, i) * m2.Element(0, 2);
    r.Element(1, i) = m1.Element(0, i) * m2.Element(1, 0) + m1.Element(1, i) * m2.Element(1, 1) + m1.Element(2, i) * m2.Element(1, 2);
    r.Element(2, i) = m1.Element(0, i) * m2.Element(2, 0) + m1.Element(1, i) * m2.Element(2, 1) + m1.Element(2, i) * m2.Element(2, 2);
  }

  WD_NAN_ASSERT(&r);
  return r;
}

template <typename Type>
WD_ALWAYS_INLINE const wdVec3Template<Type> operator*(const wdMat3Template<Type>& m, const wdVec3Template<Type>& v)
{
  return m.TransformDirection(v);
}



// *** free functions ***

template <typename Type>
WD_ALWAYS_INLINE const wdMat3Template<Type> operator*(Type f, const wdMat3Template<Type>& m1)
{
  return operator*(m1, f);
}

template <typename Type>
const wdMat3Template<Type> operator*(const wdMat3Template<Type>& m1, Type f)
{
  wdMat3Template<Type> r;

  for (wdUInt32 i = 0; i < 9; ++i)
    r.m_fElementsCM[i] = m1.m_fElementsCM[i] * f;

  WD_NAN_ASSERT(&r);

  return r;
}

template <typename Type>
WD_ALWAYS_INLINE const wdMat3Template<Type> operator/(const wdMat3Template<Type>& m1, Type f)
{
  return operator*(m1, wdMath::Invert(f));
}

template <typename Type>
const wdMat3Template<Type> operator+(const wdMat3Template<Type>& m1, const wdMat3Template<Type>& m2)
{
  wdMat3Template<Type> r;

  for (wdUInt32 i = 0; i < 9; ++i)
    r.m_fElementsCM[i] = m1.m_fElementsCM[i] + m2.m_fElementsCM[i];

  WD_NAN_ASSERT(&r);

  return r;
}

template <typename Type>
const wdMat3Template<Type> operator-(const wdMat3Template<Type>& m1, const wdMat3Template<Type>& m2)
{
  wdMat3Template<Type> r;

  for (wdUInt32 i = 0; i < 9; ++i)
    r.m_fElementsCM[i] = m1.m_fElementsCM[i] - m2.m_fElementsCM[i];

  WD_NAN_ASSERT(&r);

  return r;
}

template <typename Type>
bool wdMat3Template<Type>::IsIdentical(const wdMat3Template<Type>& rhs) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  for (wdUInt32 i = 0; i < 9; ++i)
  {
    if (m_fElementsCM[i] != rhs.m_fElementsCM[i])
      return false;
  }

  return true;
}

template <typename Type>
bool wdMat3Template<Type>::IsEqual(const wdMat3Template<Type>& rhs, Type fEpsilon) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  WD_ASSERT_DEBUG(fEpsilon >= 0.0f, "Epsilon may not be negative.");

  for (wdUInt32 i = 0; i < 9; ++i)
  {
    if (!wdMath::IsEqual(m_fElementsCM[i], rhs.m_fElementsCM[i], fEpsilon))
      return false;
  }

  return true;
}

template <typename Type>
WD_ALWAYS_INLINE bool operator==(const wdMat3Template<Type>& lhs, const wdMat3Template<Type>& rhs)
{
  return lhs.IsIdentical(rhs);
}

template <typename Type>
WD_ALWAYS_INLINE bool operator!=(const wdMat3Template<Type>& lhs, const wdMat3Template<Type>& rhs)
{
  return !lhs.IsIdentical(rhs);
}

template <typename Type>
bool wdMat3Template<Type>::IsZero(Type fEpsilon) const
{
  WD_NAN_ASSERT(this);

  for (wdUInt32 i = 0; i < 9; ++i)
  {
    if (!wdMath::IsZero(m_fElementsCM[i], fEpsilon))
      return false;
  }

  return true;
}

template <typename Type>
bool wdMat3Template<Type>::IsIdentity(Type fEpsilon) const
{
  WD_NAN_ASSERT(this);

  if (!wdMath::IsEqual(Element(0, 0), (Type)1, fEpsilon))
    return false;
  if (!wdMath::IsEqual(Element(0, 1), (Type)0, fEpsilon))
    return false;
  if (!wdMath::IsEqual(Element(0, 2), (Type)0, fEpsilon))
    return false;

  if (!wdMath::IsEqual(Element(1, 0), (Type)0, fEpsilon))
    return false;
  if (!wdMath::IsEqual(Element(1, 1), (Type)1, fEpsilon))
    return false;
  if (!wdMath::IsEqual(Element(1, 2), (Type)0, fEpsilon))
    return false;

  if (!wdMath::IsEqual(Element(2, 0), (Type)0, fEpsilon))
    return false;
  if (!wdMath::IsEqual(Element(2, 1), (Type)0, fEpsilon))
    return false;
  if (!wdMath::IsEqual(Element(2, 2), (Type)1, fEpsilon))
    return false;

  return true;
}

template <typename Type>
bool wdMat3Template<Type>::IsValid() const
{
  for (wdUInt32 i = 0; i < 9; ++i)
  {
    if (!wdMath::IsFinite(m_fElementsCM[i]))
      return false;
  }

  return true;
}

template <typename Type>
bool wdMat3Template<Type>::IsNaN() const
{
  for (wdUInt32 i = 0; i < 9; ++i)
  {
    if (wdMath::IsNaN(m_fElementsCM[i]))
      return true;
  }

  return false;
}

template <typename Type>
const wdVec3Template<Type> wdMat3Template<Type>::GetScalingFactors() const
{
  wdVec3Template<Type> v;

  v.x = wdVec3Template<Type>(Element(0, 0), Element(0, 1), Element(0, 2)).GetLength();
  v.y = wdVec3Template<Type>(Element(1, 0), Element(1, 1), Element(1, 2)).GetLength();
  v.z = wdVec3Template<Type>(Element(2, 0), Element(2, 1), Element(2, 2)).GetLength();

  WD_NAN_ASSERT(&v);
  return v;
}

template <typename Type>
wdResult wdMat3Template<Type>::SetScalingFactors(const wdVec3Template<Type>& vXYZ, Type fEpsilon /* = wdMath::DefaultEpsilon<Type>() */)
{
  wdVec3Template<Type> tx(Element(0, 0), Element(0, 1), Element(0, 2));
  wdVec3Template<Type> ty(Element(1, 0), Element(1, 1), Element(1, 2));
  wdVec3Template<Type> tz(Element(2, 0), Element(2, 1), Element(2, 2));

  if (tx.SetLength(vXYZ.x, fEpsilon) == WD_FAILURE)
    return WD_FAILURE;
  if (ty.SetLength(vXYZ.y, fEpsilon) == WD_FAILURE)
    return WD_FAILURE;
  if (tz.SetLength(vXYZ.z, fEpsilon) == WD_FAILURE)
    return WD_FAILURE;


  Element(0, 0) = tx.x;
  Element(0, 1) = tx.y;
  Element(0, 2) = tx.z;
  Element(1, 0) = ty.x;
  Element(1, 1) = ty.y;
  Element(1, 2) = ty.z;
  Element(2, 0) = tz.x;
  Element(2, 1) = tz.y;
  Element(2, 2) = tz.z;

  return WD_SUCCESS;
}

template <typename Type>
Type wdMat3Template<Type>::GetDeterminant() const
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

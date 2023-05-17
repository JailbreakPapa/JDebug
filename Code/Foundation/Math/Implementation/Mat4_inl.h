#pragma once

#include <Foundation/Math/Mat3.h>

template <typename Type>
wdMat4Template<Type>::wdMat4Template()
{
#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
  // Initialize all data to NaN in debug mode to find problems with uninitialized data easier.
  const Type TypeNaN = wdMath::NaN<Type>();
  SetElements(
    TypeNaN, TypeNaN, TypeNaN, TypeNaN, TypeNaN, TypeNaN, TypeNaN, TypeNaN, TypeNaN, TypeNaN, TypeNaN, TypeNaN, TypeNaN, TypeNaN, TypeNaN, TypeNaN);
#endif
}

template <typename Type>
wdMat4Template<Type>::wdMat4Template(const Type* const pData, wdMatrixLayout::Enum layout)
{
  SetFromArray(pData, layout);
}

template <typename Type>
wdMat4Template<Type>::wdMat4Template(Type c1r1, Type c2r1, Type c3r1, Type c4r1, Type c1r2, Type c2r2, Type c3r2, Type c4r2, Type c1r3, Type c2r3,
  Type c3r3, Type c4r3, Type c1r4, Type c2r4, Type c3r4, Type c4r4)
{
  SetElements(c1r1, c2r1, c3r1, c4r1, c1r2, c2r2, c3r2, c4r2, c1r3, c2r3, c3r3, c4r3, c1r4, c2r4, c3r4, c4r4);
}

template <typename Type>
wdMat4Template<Type>::wdMat4Template(const wdMat3Template<Type>& mRotation, const wdVec3Template<Type>& vTranslation)
{
  SetTransformationMatrix(mRotation, vTranslation);
}

template <typename Type>
const wdMat4Template<Type> wdMat4Template<Type>::IdentityMatrix()
{
  return wdMat4Template(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
}

template <typename Type>
const wdMat4Template<Type> wdMat4Template<Type>::ZeroMatrix()
{
  return wdMat4Template(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

template <typename Type>
void wdMat4Template<Type>::SetFromArray(const Type* const pData, wdMatrixLayout::Enum layout)
{
  if (layout == wdMatrixLayout::ColumnMajor)
  {
    wdMemoryUtils::Copy(m_fElementsCM, pData, 16);
  }
  else
  {
    for (int i = 0; i < 4; ++i)
    {
      Element(0, i) = pData[i * 4 + 0];
      Element(1, i) = pData[i * 4 + 1];
      Element(2, i) = pData[i * 4 + 2];
      Element(3, i) = pData[i * 4 + 3];
    }
  }
}

template <typename Type>
void wdMat4Template<Type>::SetTransformationMatrix(const wdMat3Template<Type>& mRotation, const wdVec3Template<Type>& vTranslation)
{
  SetRotationalPart(mRotation);
  SetTranslationVector(vTranslation);
  SetRow(3, wdVec4Template<Type>(0, 0, 0, 1));
}

template <typename Type>
void wdMat4Template<Type>::GetAsArray(Type* out_pData, wdMatrixLayout::Enum layout) const
{
  WD_NAN_ASSERT(this);

  if (layout == wdMatrixLayout::ColumnMajor)
  {
    wdMemoryUtils::Copy(out_pData, m_fElementsCM, 16);
  }
  else
  {
    for (int i = 0; i < 4; ++i)
    {
      out_pData[i * 4 + 0] = Element(0, i);
      out_pData[i * 4 + 1] = Element(1, i);
      out_pData[i * 4 + 2] = Element(2, i);
      out_pData[i * 4 + 3] = Element(3, i);
    }
  }
}

template <typename Type>
void wdMat4Template<Type>::SetElements(Type c1r1, Type c2r1, Type c3r1, Type c4r1, Type c1r2, Type c2r2, Type c3r2, Type c4r2, Type c1r3, Type c2r3,
  Type c3r3, Type c4r3, Type c1r4, Type c2r4, Type c3r4, Type c4r4)
{
  Element(0, 0) = c1r1;
  Element(1, 0) = c2r1;
  Element(2, 0) = c3r1;
  Element(3, 0) = c4r1;
  Element(0, 1) = c1r2;
  Element(1, 1) = c2r2;
  Element(2, 1) = c3r2;
  Element(3, 1) = c4r2;
  Element(0, 2) = c1r3;
  Element(1, 2) = c2r3;
  Element(2, 2) = c3r3;
  Element(3, 2) = c4r3;
  Element(0, 3) = c1r4;
  Element(1, 3) = c2r4;
  Element(2, 3) = c3r4;
  Element(3, 3) = c4r4;
}

template <typename Type>
void wdMat4Template<Type>::SetZero()
{
  SetElements(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

template <typename Type>
void wdMat4Template<Type>::SetIdentity()
{
  SetElements(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
}

template <typename Type>
void wdMat4Template<Type>::SetTranslationMatrix(const wdVec3Template<Type>& vTranslation)
{
  SetElements(1, 0, 0, vTranslation.x, 0, 1, 0, vTranslation.y, 0, 0, 1, vTranslation.z, 0, 0, 0, 1);
}

template <typename Type>
void wdMat4Template<Type>::SetScalingMatrix(const wdVec3Template<Type>& s)
{
  SetElements(s.x, 0, 0, 0, 0, s.y, 0, 0, 0, 0, s.z, 0, 0, 0, 0, 1);
}

template <typename Type>
void wdMat4Template<Type>::SetRotationMatrixX(wdAngle angle)
{
  const Type fSin = wdMath::Sin(angle);
  const Type fCos = wdMath::Cos(angle);

  SetElements(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, fCos, -fSin, 0.0f, 0.0f, fSin, fCos, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}

template <typename Type>
void wdMat4Template<Type>::SetRotationMatrixY(wdAngle angle)
{
  const Type fSin = wdMath::Sin(angle);
  const Type fCos = wdMath::Cos(angle);


  SetElements(fCos, 0.0f, fSin, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -fSin, 0.0f, fCos, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}

template <typename Type>
void wdMat4Template<Type>::SetRotationMatrixZ(wdAngle angle)
{
  const Type fSin = wdMath::Sin(angle);
  const Type fCos = wdMath::Cos(angle);

  SetElements(fCos, -fSin, 0.0f, 0.0f, fSin, fCos, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}

template <typename Type>
void wdMat4Template<Type>::Transpose()
{
  wdMath::Swap(Element(0, 1), Element(1, 0));
  wdMath::Swap(Element(0, 2), Element(2, 0));
  wdMath::Swap(Element(0, 3), Element(3, 0));
  wdMath::Swap(Element(1, 2), Element(2, 1));
  wdMath::Swap(Element(1, 3), Element(3, 1));
  wdMath::Swap(Element(2, 3), Element(3, 2));
}

template <typename Type>
const wdMat4Template<Type> wdMat4Template<Type>::GetTranspose() const
{
  WD_NAN_ASSERT(this);

  return wdMat4Template(m_fElementsCM, wdMatrixLayout::RowMajor);
}

template <typename Type>
const wdMat4Template<Type> wdMat4Template<Type>::GetInverse(Type fEpsilon) const
{
  wdMat4Template<Type> Inverse = *this;
  wdResult res = Inverse.Invert(fEpsilon);
  WD_ASSERT_DEBUG(res.Succeeded(), "Could not invert the given Mat4.");
  WD_IGNORE_UNUSED(res);
  return Inverse;
}

template <typename Type>
wdVec4Template<Type> wdMat4Template<Type>::GetRow(wdUInt32 uiRow) const
{
  WD_NAN_ASSERT(this);
  WD_ASSERT_DEBUG(uiRow <= 3, "Invalid Row Index {0}", uiRow);

  wdVec4Template<Type> r;
  r.x = Element(0, uiRow);
  r.y = Element(1, uiRow);
  r.z = Element(2, uiRow);
  r.w = Element(3, uiRow);

  return r;
}

template <typename Type>
void wdMat4Template<Type>::SetRow(wdUInt32 uiRow, const wdVec4Template<Type>& vRow)
{
  WD_ASSERT_DEBUG(uiRow <= 3, "Invalid Row Index {0}", uiRow);

  Element(0, uiRow) = vRow.x;
  Element(1, uiRow) = vRow.y;
  Element(2, uiRow) = vRow.z;
  Element(3, uiRow) = vRow.w;
}

template <typename Type>
wdVec4Template<Type> wdMat4Template<Type>::GetColumn(wdUInt32 uiColumn) const
{
  WD_NAN_ASSERT(this);
  WD_ASSERT_DEBUG(uiColumn <= 3, "Invalid Column Index {0}", uiColumn);

  wdVec4Template<Type> r;
  r.x = Element(uiColumn, 0);
  r.y = Element(uiColumn, 1);
  r.z = Element(uiColumn, 2);
  r.w = Element(uiColumn, 3);

  return r;
}

template <typename Type>
void wdMat4Template<Type>::SetColumn(wdUInt32 uiColumn, const wdVec4Template<Type>& vColumn)
{
  WD_ASSERT_DEBUG(uiColumn <= 3, "Invalid Column Index {0}", uiColumn);

  Element(uiColumn, 0) = vColumn.x;
  Element(uiColumn, 1) = vColumn.y;
  Element(uiColumn, 2) = vColumn.z;
  Element(uiColumn, 3) = vColumn.w;
}

template <typename Type>
wdVec4Template<Type> wdMat4Template<Type>::GetDiagonal() const
{
  WD_NAN_ASSERT(this);

  return wdVec4Template<Type>(Element(0, 0), Element(1, 1), Element(2, 2), Element(3, 3));
}

template <typename Type>
void wdMat4Template<Type>::SetDiagonal(const wdVec4Template<Type>& vDiag)
{
  Element(0, 0) = vDiag.x;
  Element(1, 1) = vDiag.y;
  Element(2, 2) = vDiag.z;
  Element(3, 3) = vDiag.w;
}

template <typename Type>
const wdVec3Template<Type> wdMat4Template<Type>::TransformPosition(const wdVec3Template<Type>& v) const
{
  wdVec3Template<Type> r;
  r.x = Element(0, 0) * v.x + Element(1, 0) * v.y + Element(2, 0) * v.z + Element(3, 0);
  r.y = Element(0, 1) * v.x + Element(1, 1) * v.y + Element(2, 1) * v.z + Element(3, 1);
  r.z = Element(0, 2) * v.x + Element(1, 2) * v.y + Element(2, 2) * v.z + Element(3, 2);

  WD_NAN_ASSERT(&r);
  return r;
}

template <typename Type>
void wdMat4Template<Type>::TransformPosition(
  wdVec3Template<Type>* pV, wdUInt32 uiNumVectors, wdUInt32 uiStride /* = sizeof(wdVec3Template) */) const
{
  WD_ASSERT_DEBUG(pV != nullptr, "Array must not be nullptr.");
  WD_ASSERT_DEBUG(uiStride >= sizeof(wdVec3Template<Type>), "Data must not overlap.");

  wdVec3Template<Type>* pCur = pV;

  for (wdUInt32 i = 0; i < uiNumVectors; ++i)
  {
    *pCur = TransformPosition(*pCur);
    pCur = wdMemoryUtils::AddByteOffset(pCur, uiStride);
  }
}

template <typename Type>
const wdVec3Template<Type> wdMat4Template<Type>::TransformDirection(const wdVec3Template<Type>& v) const
{
  wdVec3Template<Type> r;
  r.x = Element(0, 0) * v.x + Element(1, 0) * v.y + Element(2, 0) * v.z;
  r.y = Element(0, 1) * v.x + Element(1, 1) * v.y + Element(2, 1) * v.z;
  r.z = Element(0, 2) * v.x + Element(1, 2) * v.y + Element(2, 2) * v.z;

  WD_NAN_ASSERT(&r);
  return r;
}

template <typename Type>
void wdMat4Template<Type>::TransformDirection(
  wdVec3Template<Type>* pV, wdUInt32 uiNumVectors, wdUInt32 uiStride /* = sizeof(wdVec3Template<Type>) */) const
{
  WD_ASSERT_DEBUG(pV != nullptr, "Array must not be nullptr.");
  WD_ASSERT_DEBUG(uiStride >= sizeof(wdVec3Template<Type>), "Data must not overlap.");

  wdVec3Template<Type>* pCur = pV;

  for (wdUInt32 i = 0; i < uiNumVectors; ++i)
  {
    *pCur = TransformDirection(*pCur);
    pCur = wdMemoryUtils::AddByteOffset(pCur, uiStride);
  }
}

template <typename Type>
const wdVec4Template<Type> wdMat4Template<Type>::Transform(const wdVec4Template<Type>& v) const
{
  wdVec4Template<Type> r;
  r.x = Element(0, 0) * v.x + Element(1, 0) * v.y + Element(2, 0) * v.z + Element(3, 0) * v.w;
  r.y = Element(0, 1) * v.x + Element(1, 1) * v.y + Element(2, 1) * v.z + Element(3, 1) * v.w;
  r.z = Element(0, 2) * v.x + Element(1, 2) * v.y + Element(2, 2) * v.z + Element(3, 2) * v.w;
  r.w = Element(0, 3) * v.x + Element(1, 3) * v.y + Element(2, 3) * v.z + Element(3, 3) * v.w;

  WD_NAN_ASSERT(&r);
  return r;
}

template <typename Type>
void wdMat4Template<Type>::Transform(wdVec4Template<Type>* pV, wdUInt32 uiNumVectors, wdUInt32 uiStride /* = sizeof(wdVec4Template) */) const
{
  WD_ASSERT_DEBUG(pV != nullptr, "Array must not be nullptr.");
  WD_ASSERT_DEBUG(uiStride >= sizeof(wdVec4Template<Type>), "Data must not overlap.");

  wdVec4Template<Type>* pCur = pV;

  for (wdUInt32 i = 0; i < uiNumVectors; ++i)
  {
    *pCur = Transform(*pCur);
    pCur = wdMemoryUtils::AddByteOffset(pCur, uiStride);
  }
}

template <typename Type>
WD_FORCE_INLINE const wdVec3Template<Type> wdMat4Template<Type>::GetTranslationVector() const
{
  WD_NAN_ASSERT(this);

  return wdVec3Template<Type>(Element(3, 0), Element(3, 1), Element(3, 2));
}

template <typename Type>
WD_ALWAYS_INLINE void wdMat4Template<Type>::SetTranslationVector(const wdVec3Template<Type>& v)
{
  Element(3, 0) = v.x;
  Element(3, 1) = v.y;
  Element(3, 2) = v.z;
}

template <typename Type>
void wdMat4Template<Type>::SetRotationalPart(const wdMat3Template<Type>& mRotation)
{
  for (wdUInt32 col = 0; col < 3; ++col)
  {
    for (wdUInt32 row = 0; row < 3; ++row)
    {
      Element(col, row) = mRotation.Element(col, row);
    }
  }
}

template <typename Type>
const wdMat3Template<Type> wdMat4Template<Type>::GetRotationalPart() const
{
  wdMat3Template<Type> r;

  for (wdUInt32 col = 0; col < 3; ++col)
  {
    for (wdUInt32 row = 0; row < 3; ++row)
    {
      r.Element(col, row) = Element(col, row);
    }
  }

  WD_NAN_ASSERT(&r);
  return r;
}

template <typename Type>
void wdMat4Template<Type>::operator*=(Type f)
{
  for (wdInt32 i = 0; i < 16; ++i)
    m_fElementsCM[i] *= f;

  WD_NAN_ASSERT(this);
}

template <typename Type>
void wdMat4Template<Type>::operator/=(Type f)
{
  const Type fInv = wdMath::Invert(f);

  operator*=(fInv);
}

template <typename Type>
const wdMat4Template<Type> operator*(const wdMat4Template<Type>& m1, const wdMat4Template<Type>& m2)
{
  wdMat4Template<Type> r;
  for (wdInt32 i = 0; i < 4; ++i)
  {
    r.Element(0, i) = m1.Element(0, i) * m2.Element(0, 0) + m1.Element(1, i) * m2.Element(0, 1) + m1.Element(2, i) * m2.Element(0, 2) +
                      m1.Element(3, i) * m2.Element(0, 3);
    r.Element(1, i) = m1.Element(0, i) * m2.Element(1, 0) + m1.Element(1, i) * m2.Element(1, 1) + m1.Element(2, i) * m2.Element(1, 2) +
                      m1.Element(3, i) * m2.Element(1, 3);
    r.Element(2, i) = m1.Element(0, i) * m2.Element(2, 0) + m1.Element(1, i) * m2.Element(2, 1) + m1.Element(2, i) * m2.Element(2, 2) +
                      m1.Element(3, i) * m2.Element(2, 3);
    r.Element(3, i) = m1.Element(0, i) * m2.Element(3, 0) + m1.Element(1, i) * m2.Element(3, 1) + m1.Element(2, i) * m2.Element(3, 2) +
                      m1.Element(3, i) * m2.Element(3, 3);
  }

  WD_NAN_ASSERT(&r);
  return r;
}

template <typename Type>
WD_ALWAYS_INLINE const wdVec3Template<Type> operator*(const wdMat4Template<Type>& m, const wdVec3Template<Type>& v)
{
  return m.TransformPosition(v);
}

template <typename Type>
WD_ALWAYS_INLINE const wdVec4Template<Type> operator*(const wdMat4Template<Type>& m, const wdVec4Template<Type>& v)
{
  return m.Transform(v);
}



// *** Stuff needed for matrix inversion ***

template <typename Type>
WD_FORCE_INLINE Type GetDeterminantOf3x3SubMatrix(const wdMat4Template<Type>& m, wdInt32 i, wdInt32 j)
{
  const wdInt32 si0 = 0 + ((i <= 0) ? 1 : 0);
  const wdInt32 si1 = 1 + ((i <= 1) ? 1 : 0);
  const wdInt32 si2 = 2 + ((i <= 2) ? 1 : 0);

  const wdInt32 sj0 = 0 + ((j <= 0) ? 1 : 0);
  const wdInt32 sj1 = 1 + ((j <= 1) ? 1 : 0);
  const wdInt32 sj2 = 2 + ((j <= 2) ? 1 : 0);

  Type fDet2 = ((m.Element(sj0, si0) * m.Element(sj1, si1) * m.Element(sj2, si2) + m.Element(sj1, si0) * m.Element(sj2, si1) * m.Element(sj0, si2) +
                  m.Element(sj2, si0) * m.Element(sj0, si1) * m.Element(sj1, si2)) -
                (m.Element(sj0, si2) * m.Element(sj1, si1) * m.Element(sj2, si0) + m.Element(sj1, si2) * m.Element(sj2, si1) * m.Element(sj0, si0) +
                  m.Element(sj2, si2) * m.Element(sj0, si1) * m.Element(sj1, si0)));

  return fDet2;
}

template <typename Type>
WD_FORCE_INLINE Type GetDeterminantOf4x4Matrix(const wdMat4Template<Type>& m)
{
  Type det = 0.0;

  det += m.Element(0, 0) * GetDeterminantOf3x3SubMatrix(m, 0, 0);
  det += -m.Element(1, 0) * GetDeterminantOf3x3SubMatrix(m, 0, 1);
  det += m.Element(2, 0) * GetDeterminantOf3x3SubMatrix(m, 0, 2);
  det += -m.Element(3, 0) * GetDeterminantOf3x3SubMatrix(m, 0, 3);

  return det;
}


// *** free functions ***

template <typename Type>
WD_ALWAYS_INLINE const wdMat4Template<Type> operator*(Type f, const wdMat4Template<Type>& m1)
{
  return operator*(m1, f);
}

template <typename Type>
const wdMat4Template<Type> operator*(const wdMat4Template<Type>& m1, Type f)
{
  wdMat4Template<Type> r;

  for (wdUInt32 i = 0; i < 16; ++i)
    r.m_fElementsCM[i] = m1.m_fElementsCM[i] * f;

  WD_NAN_ASSERT(&r);
  return r;
}

template <typename Type>
const wdMat4Template<Type> operator/(const wdMat4Template<Type>& m1, Type f)
{
  return operator*(m1, wdMath::Invert(f));
}

template <typename Type>
const wdMat4Template<Type> operator+(const wdMat4Template<Type>& m1, const wdMat4Template<Type>& m2)
{
  wdMat4Template<Type> r;

  for (wdUInt32 i = 0; i < 16; ++i)
    r.m_fElementsCM[i] = m1.m_fElementsCM[i] + m2.m_fElementsCM[i];

  WD_NAN_ASSERT(&r);
  return r;
}

template <typename Type>
const wdMat4Template<Type> operator-(const wdMat4Template<Type>& m1, const wdMat4Template<Type>& m2)
{
  wdMat4Template<Type> r;

  for (wdUInt32 i = 0; i < 16; ++i)
    r.m_fElementsCM[i] = m1.m_fElementsCM[i] - m2.m_fElementsCM[i];

  WD_NAN_ASSERT(&r);
  return r;
}

template <typename Type>
bool wdMat4Template<Type>::IsIdentical(const wdMat4Template<Type>& rhs) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  for (wdUInt32 i = 0; i < 16; ++i)
  {
    if (m_fElementsCM[i] != rhs.m_fElementsCM[i])
      return false;
  }

  return true;
}

template <typename Type>
bool wdMat4Template<Type>::IsEqual(const wdMat4Template<Type>& rhs, Type fEpsilon) const
{
  WD_NAN_ASSERT(this);
  WD_NAN_ASSERT(&rhs);

  WD_ASSERT_DEBUG(fEpsilon >= 0.0f, "Epsilon may not be negative.");

  for (wdUInt32 i = 0; i < 16; ++i)
  {
    if (!wdMath::IsEqual(m_fElementsCM[i], rhs.m_fElementsCM[i], fEpsilon))
      return false;
  }

  return true;
}

template <typename Type>
WD_ALWAYS_INLINE bool operator==(const wdMat4Template<Type>& lhs, const wdMat4Template<Type>& rhs)
{
  return lhs.IsIdentical(rhs);
}

template <typename Type>
WD_ALWAYS_INLINE bool operator!=(const wdMat4Template<Type>& lhs, const wdMat4Template<Type>& rhs)
{
  return !lhs.IsIdentical(rhs);
}

template <typename Type>
bool wdMat4Template<Type>::IsZero(Type fEpsilon) const
{
  WD_NAN_ASSERT(this);

  for (wdUInt32 i = 0; i < 16; ++i)
  {
    if (!wdMath::IsZero(m_fElementsCM[i], fEpsilon))
      return false;
  }

  return true;
}

template <typename Type>
bool wdMat4Template<Type>::IsIdentity(Type fEpsilon) const
{
  WD_NAN_ASSERT(this);

  if (!wdMath::IsEqual(Element(0, 0), (Type)1, fEpsilon))
    return false;
  if (!wdMath::IsEqual(Element(0, 1), (Type)0, fEpsilon))
    return false;
  if (!wdMath::IsEqual(Element(0, 2), (Type)0, fEpsilon))
    return false;
  if (!wdMath::IsEqual(Element(0, 3), (Type)0, fEpsilon))
    return false;

  if (!wdMath::IsEqual(Element(1, 0), (Type)0, fEpsilon))
    return false;
  if (!wdMath::IsEqual(Element(1, 1), (Type)1, fEpsilon))
    return false;
  if (!wdMath::IsEqual(Element(1, 2), (Type)0, fEpsilon))
    return false;
  if (!wdMath::IsEqual(Element(1, 3), (Type)0, fEpsilon))
    return false;

  if (!wdMath::IsEqual(Element(2, 0), (Type)0, fEpsilon))
    return false;
  if (!wdMath::IsEqual(Element(2, 1), (Type)0, fEpsilon))
    return false;
  if (!wdMath::IsEqual(Element(2, 2), (Type)1, fEpsilon))
    return false;
  if (!wdMath::IsEqual(Element(2, 3), (Type)0, fEpsilon))
    return false;

  if (!wdMath::IsEqual(Element(3, 0), (Type)0, fEpsilon))
    return false;
  if (!wdMath::IsEqual(Element(3, 1), (Type)0, fEpsilon))
    return false;
  if (!wdMath::IsEqual(Element(3, 2), (Type)0, fEpsilon))
    return false;
  if (!wdMath::IsEqual(Element(3, 3), (Type)1, fEpsilon))
    return false;

  return true;
}

template <typename Type>
bool wdMat4Template<Type>::IsValid() const
{
  for (wdUInt32 i = 0; i < 16; ++i)
  {
    if (!wdMath::IsFinite(m_fElementsCM[i]))
      return false;
  }

  return true;
}

template <typename Type>
bool wdMat4Template<Type>::IsNaN() const
{
  for (wdUInt32 i = 0; i < 16; ++i)
  {
    if (wdMath::IsNaN(m_fElementsCM[i]))
      return true;
  }

  return false;
}

template <typename Type>
const wdVec3Template<Type> wdMat4Template<Type>::GetScalingFactors() const
{
  wdVec3Template<Type> v;

  v.x = wdVec3Template<Type>(Element(0, 0), Element(0, 1), Element(0, 2)).GetLength();
  v.y = wdVec3Template<Type>(Element(1, 0), Element(1, 1), Element(1, 2)).GetLength();
  v.z = wdVec3Template<Type>(Element(2, 0), Element(2, 1), Element(2, 2)).GetLength();

  WD_NAN_ASSERT(&v);
  return v;
}

template <typename Type>
wdResult wdMat4Template<Type>::SetScalingFactors(const wdVec3Template<Type>& vXYZ, Type fEpsilon /* = wdMath::DefaultEpsilon<Type>() */)
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

#include <Foundation/Math/Implementation/AllClasses_inl.h>

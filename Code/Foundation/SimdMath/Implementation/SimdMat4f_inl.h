#pragma once

WD_ALWAYS_INLINE wdSimdMat4f::wdSimdMat4f() {}

WD_ALWAYS_INLINE wdSimdMat4f::wdSimdMat4f(const float* const pData, wdMatrixLayout::Enum layout)
{
  SetFromArray(pData, layout);
}

WD_ALWAYS_INLINE wdSimdMat4f::wdSimdMat4f(const wdSimdVec4f& vCol0, const wdSimdVec4f& vCol1, const wdSimdVec4f& vCol2, const wdSimdVec4f& vCol3)
{
  m_col0 = vCol0;
  m_col1 = vCol1;
  m_col2 = vCol2;
  m_col3 = vCol3;
}

WD_ALWAYS_INLINE wdSimdMat4f::wdSimdMat4f(float f1r1, float f2r1, float f3r1, float f4r1, float f1r2, float f2r2, float f3r2, float f4r2, float f1r3,
  float f2r3, float f3r3, float f4r3, float f1r4, float f2r4, float f3r4, float f4r4)
{
  m_col0.Set(f1r1, f1r2, f1r3, f1r4);
  m_col1.Set(f2r1, f2r2, f2r3, f2r4);
  m_col2.Set(f3r1, f3r2, f3r3, f3r4);
  m_col3.Set(f4r1, f4r2, f4r3, f4r4);
}

inline void wdSimdMat4f::SetFromArray(const float* const pData, wdMatrixLayout::Enum layout)
{
  m_col0.Load<4>(pData + 0);
  m_col1.Load<4>(pData + 4);
  m_col2.Load<4>(pData + 8);
  m_col3.Load<4>(pData + 12);

  if (layout == wdMatrixLayout::RowMajor)
  {
    Transpose();
  }
}

inline void wdSimdMat4f::GetAsArray(float* out_pData, wdMatrixLayout::Enum layout) const
{
  wdSimdMat4f tmp = *this;

  if (layout == wdMatrixLayout::RowMajor)
  {
    tmp.Transpose();
  }

  tmp.m_col0.Store<4>(out_pData + 0);
  tmp.m_col1.Store<4>(out_pData + 4);
  tmp.m_col2.Store<4>(out_pData + 8);
  tmp.m_col3.Store<4>(out_pData + 12);
}

WD_ALWAYS_INLINE void wdSimdMat4f::SetIdentity()
{
  m_col0.Set(1, 0, 0, 0);
  m_col1.Set(0, 1, 0, 0);
  m_col2.Set(0, 0, 1, 0);
  m_col3.Set(0, 0, 0, 1);
}

// static
WD_ALWAYS_INLINE wdSimdMat4f wdSimdMat4f::IdentityMatrix()
{
  wdSimdMat4f result;
  result.SetIdentity();
  return result;
}

WD_ALWAYS_INLINE wdSimdMat4f wdSimdMat4f::GetTranspose() const
{
  wdSimdMat4f result = *this;
  result.Transpose();
  return result;
}

WD_ALWAYS_INLINE wdSimdMat4f wdSimdMat4f::GetInverse(const wdSimdFloat& fEpsilon) const
{
  wdSimdMat4f result = *this;
  result.Invert(fEpsilon).IgnoreResult();
  return result;
}

inline bool wdSimdMat4f::IsEqual(const wdSimdMat4f& rhs, const wdSimdFloat& fEpsilon) const
{
  return (m_col0.IsEqual(rhs.m_col0, fEpsilon) && m_col1.IsEqual(rhs.m_col1, fEpsilon) && m_col2.IsEqual(rhs.m_col2, fEpsilon) &&
          m_col3.IsEqual(rhs.m_col3, fEpsilon))
    .AllSet<4>();
}

inline bool wdSimdMat4f::IsIdentity(const wdSimdFloat& fEpsilon) const
{
  return (m_col0.IsEqual(wdSimdVec4f(1, 0, 0, 0), fEpsilon) && m_col1.IsEqual(wdSimdVec4f(0, 1, 0, 0), fEpsilon) &&
          m_col2.IsEqual(wdSimdVec4f(0, 0, 1, 0), fEpsilon) && m_col3.IsEqual(wdSimdVec4f(0, 0, 0, 1), fEpsilon))
    .AllSet<4>();
}

inline bool wdSimdMat4f::IsValid() const
{
  return m_col0.IsValid<4>() && m_col1.IsValid<4>() && m_col2.IsValid<4>() && m_col3.IsValid<4>();
}

inline bool wdSimdMat4f::IsNaN() const
{
  return m_col0.IsNaN<4>() || m_col1.IsNaN<4>() || m_col2.IsNaN<4>() || m_col3.IsNaN<4>();
}

WD_ALWAYS_INLINE void wdSimdMat4f::SetRows(const wdSimdVec4f& vRow0, const wdSimdVec4f& vRow1, const wdSimdVec4f& vRow2, const wdSimdVec4f& vRow3)
{
  m_col0 = vRow0;
  m_col1 = vRow1;
  m_col2 = vRow2;
  m_col3 = vRow3;

  Transpose();
}

WD_ALWAYS_INLINE void wdSimdMat4f::GetRows(wdSimdVec4f& ref_vRow0, wdSimdVec4f& ref_vRow1, wdSimdVec4f& ref_vRow2, wdSimdVec4f& ref_vRow3) const
{
  wdSimdMat4f tmp = *this;
  tmp.Transpose();

  ref_vRow0 = tmp.m_col0;
  ref_vRow1 = tmp.m_col1;
  ref_vRow2 = tmp.m_col2;
  ref_vRow3 = tmp.m_col3;
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdMat4f::TransformPosition(const wdSimdVec4f& v) const
{
  wdSimdVec4f result;
  result = m_col0 * v.x();
  result += m_col1 * v.y();
  result += m_col2 * v.z();
  result += m_col3;

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdMat4f::TransformDirection(const wdSimdVec4f& v) const
{
  wdSimdVec4f result;
  result = m_col0 * v.x();
  result += m_col1 * v.y();
  result += m_col2 * v.z();

  return result;
}

WD_ALWAYS_INLINE wdSimdMat4f wdSimdMat4f::operator*(const wdSimdMat4f& rhs) const
{
  wdSimdMat4f result;

  result.m_col0 = m_col0 * rhs.m_col0.x();
  result.m_col0 += m_col1 * rhs.m_col0.y();
  result.m_col0 += m_col2 * rhs.m_col0.z();
  result.m_col0 += m_col3 * rhs.m_col0.w();

  result.m_col1 = m_col0 * rhs.m_col1.x();
  result.m_col1 += m_col1 * rhs.m_col1.y();
  result.m_col1 += m_col2 * rhs.m_col1.z();
  result.m_col1 += m_col3 * rhs.m_col1.w();

  result.m_col2 = m_col0 * rhs.m_col2.x();
  result.m_col2 += m_col1 * rhs.m_col2.y();
  result.m_col2 += m_col2 * rhs.m_col2.z();
  result.m_col2 += m_col3 * rhs.m_col2.w();

  result.m_col3 = m_col0 * rhs.m_col3.x();
  result.m_col3 += m_col1 * rhs.m_col3.y();
  result.m_col3 += m_col2 * rhs.m_col3.z();
  result.m_col3 += m_col3 * rhs.m_col3.w();

  return result;
}

WD_ALWAYS_INLINE void wdSimdMat4f::operator*=(const wdSimdMat4f& rhs)
{
  *this = *this * rhs;
}

WD_ALWAYS_INLINE bool wdSimdMat4f::operator==(const wdSimdMat4f& other) const
{
  return (m_col0 == other.m_col0 && m_col1 == other.m_col1 && m_col2 == other.m_col2 && m_col3 == other.m_col3).AllSet<4>();
}

WD_ALWAYS_INLINE bool wdSimdMat4f::operator!=(const wdSimdMat4f& other) const
{
  return !(*this == other);
}

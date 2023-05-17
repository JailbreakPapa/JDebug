#pragma once

WD_ALWAYS_INLINE wdSimdVec4f::wdSimdVec4f(wdInternal::QuadFloat v)
{
  m_v = v;
}

template <int N, wdMathAcc::Enum acc>
WD_ALWAYS_INLINE wdSimdFloat wdSimdVec4f::GetLength() const
{
  const wdSimdFloat squaredLen = GetLengthSquared<N>();
  return squaredLen.GetSqrt<acc>();
}

template <int N, wdMathAcc::Enum acc>
WD_ALWAYS_INLINE wdSimdFloat wdSimdVec4f::GetInvLength() const
{
  const wdSimdFloat squaredLen = GetLengthSquared<N>();
  return squaredLen.GetInvSqrt<acc>();
}

template <int N>
WD_ALWAYS_INLINE wdSimdFloat wdSimdVec4f::GetLengthSquared() const
{
  return Dot<N>(*this);
}

template <int N, wdMathAcc::Enum acc>
WD_ALWAYS_INLINE wdSimdFloat wdSimdVec4f::GetLengthAndNormalize()
{
  const wdSimdFloat squaredLen = GetLengthSquared<N>();
  const wdSimdFloat reciprocalLen = squaredLen.GetInvSqrt<acc>();
  *this = (*this) * reciprocalLen;
  return squaredLen * reciprocalLen;
}

template <int N, wdMathAcc::Enum acc>
WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::GetNormalized() const
{
  return (*this) * GetInvLength<N, acc>();
}

template <int N, wdMathAcc::Enum acc>
WD_ALWAYS_INLINE void wdSimdVec4f::Normalize()
{
  *this = GetNormalized<N, acc>();
}

template <int N>
WD_ALWAYS_INLINE bool wdSimdVec4f::IsNormalized(const wdSimdFloat& fEpsilon) const
{
  const wdSimdFloat sqLength = GetLengthSquared<N>();
  return sqLength.IsEqual(1.0f, fEpsilon);
}

inline wdSimdFloat wdSimdVec4f::GetComponent(int i) const
{
  switch (i)
  {
    case 0:
      return GetComponent<0>();

    case 1:
      return GetComponent<1>();

    case 2:
      return GetComponent<2>();

    default:
      return GetComponent<3>();
  }
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::Fraction() const
{
  return *this - Trunc();
}

// static
WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4f::Lerp(const wdSimdVec4f& a, const wdSimdVec4f& b, const wdSimdVec4f& t)
{
  return a + t.CompMul(b - a);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4f::IsEqual(const wdSimdVec4f& rhs, const wdSimdFloat& fEpsilon) const
{
  wdSimdVec4f minusEps = rhs - wdSimdVec4f(fEpsilon);
  wdSimdVec4f plusEps = rhs + wdSimdVec4f(fEpsilon);
  return (*this >= minusEps) && (*this <= plusEps);
}

template <>
WD_ALWAYS_INLINE wdSimdFloat wdSimdVec4f::HorizontalSum<1>() const
{
  return GetComponent<0>();
}

template <>
WD_ALWAYS_INLINE wdSimdFloat wdSimdVec4f::HorizontalMin<1>() const
{
  return GetComponent<0>();
}

template <>
WD_ALWAYS_INLINE wdSimdFloat wdSimdVec4f::HorizontalMax<1>() const
{
  return GetComponent<0>();
}

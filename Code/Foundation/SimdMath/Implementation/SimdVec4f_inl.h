#pragma once

NS_ALWAYS_INLINE nsSimdVec4f::nsSimdVec4f(nsInternal::QuadFloat v)
{
  m_v = v;
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::MakeZero()
{
  return nsSimdVec4f(nsSimdFloat::MakeZero());
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::MakeNaN()
{
  return nsSimdVec4f(nsSimdFloat::MakeNaN());
}

template <int N, nsMathAcc::Enum acc>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::GetLength() const
{
  const nsSimdFloat squaredLen = GetLengthSquared<N>();
  return squaredLen.GetSqrt<acc>();
}

template <int N, nsMathAcc::Enum acc>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::GetInvLength() const
{
  const nsSimdFloat squaredLen = GetLengthSquared<N>();
  return squaredLen.GetInvSqrt<acc>();
}

template <int N>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::GetLengthSquared() const
{
  return Dot<N>(*this);
}

template <int N, nsMathAcc::Enum acc>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::GetLengthAndNormalize()
{
  const nsSimdFloat squaredLen = GetLengthSquared<N>();
  const nsSimdFloat reciprocalLen = squaredLen.GetInvSqrt<acc>();
  *this = (*this) * reciprocalLen;
  return squaredLen * reciprocalLen;
}

template <int N, nsMathAcc::Enum acc>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetNormalized() const
{
  return (*this) * GetInvLength<N, acc>();
}

template <int N, nsMathAcc::Enum acc>
NS_ALWAYS_INLINE void nsSimdVec4f::Normalize()
{
  *this = GetNormalized<N, acc>();
}

template <int N>
NS_ALWAYS_INLINE bool nsSimdVec4f::IsNormalized(const nsSimdFloat& fEpsilon) const
{
  const nsSimdFloat sqLength = GetLengthSquared<N>();
  return sqLength.IsEqual(1.0f, fEpsilon);
}

inline nsSimdFloat nsSimdVec4f::GetComponent(int i) const
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

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::Fraction() const
{
  return *this - Trunc();
}

// static
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::Lerp(const nsSimdVec4f& a, const nsSimdVec4f& b, const nsSimdVec4f& t)
{
  return a + t.CompMul(b - a);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4f::IsEqual(const nsSimdVec4f& rhs, const nsSimdFloat& fEpsilon) const
{
  nsSimdVec4f minusEps = rhs - nsSimdVec4f(fEpsilon);
  nsSimdVec4f plusEps = rhs + nsSimdVec4f(fEpsilon);
  return (*this >= minusEps) && (*this <= plusEps);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalSum<1>() const
{
  return GetComponent<0>();
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalMin<1>() const
{
  return GetComponent<0>();
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalMax<1>() const
{
  return GetComponent<0>();
}

#pragma once

NS_ALWAYS_INLINE nsSimdVec4b::nsSimdVec4b()
{
  NS_CHECK_SIMD_ALIGNMENT(this);
}

NS_ALWAYS_INLINE nsSimdVec4b::nsSimdVec4b(bool b)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  nsUInt32 mask = b ? 0xFFFFFFFF : 0;
  __m128 tmp = _mm_load_ss((float*)&mask);
  m_v = _mm_shuffle_ps(tmp, tmp, NS_TO_SHUFFLE(nsSwizzle::XXXX));
}

NS_ALWAYS_INLINE nsSimdVec4b::nsSimdVec4b(bool x, bool y, bool z, bool w)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  alignas(16) nsUInt32 mask[4] = {x ? 0xFFFFFFFF : 0, y ? 0xFFFFFFFF : 0, z ? 0xFFFFFFFF : 0, w ? 0xFFFFFFFF : 0};
  m_v = _mm_load_ps((float*)mask);
}

NS_ALWAYS_INLINE nsSimdVec4b::nsSimdVec4b(nsInternal::QuadBool v)
{
  m_v = v;
}

template <int N>
NS_ALWAYS_INLINE bool nsSimdVec4b::GetComponent() const
{
  return _mm_movemask_ps(_mm_shuffle_ps(m_v, m_v, NS_SHUFFLE(N, N, N, N))) != 0;
}

NS_ALWAYS_INLINE bool nsSimdVec4b::x() const
{
  return GetComponent<0>();
}

NS_ALWAYS_INLINE bool nsSimdVec4b::y() const
{
  return GetComponent<1>();
}

NS_ALWAYS_INLINE bool nsSimdVec4b::z() const
{
  return GetComponent<2>();
}

NS_ALWAYS_INLINE bool nsSimdVec4b::w() const
{
  return GetComponent<3>();
}

template <nsSwizzle::Enum s>
NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4b::Get() const
{
  return _mm_shuffle_ps(m_v, m_v, NS_TO_SHUFFLE(s));
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4b::operator&&(const nsSimdVec4b& rhs) const
{
  return _mm_and_ps(m_v, rhs.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4b::operator||(const nsSimdVec4b& rhs) const
{
  return _mm_or_ps(m_v, rhs.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4b::operator!() const
{
  __m128 allTrue = _mm_cmpeq_ps(_mm_setzero_ps(), _mm_setzero_ps());
  return _mm_xor_ps(m_v, allTrue);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4b::operator==(const nsSimdVec4b& rhs) const
{
  return !(*this != rhs);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4b::operator!=(const nsSimdVec4b& rhs) const
{
  return _mm_xor_ps(m_v, rhs.m_v);
}

template <int N>
NS_ALWAYS_INLINE bool nsSimdVec4b::AllSet() const
{
  const int mask = NS_BIT(N) - 1;
  return (_mm_movemask_ps(m_v) & mask) == mask;
}

template <int N>
NS_ALWAYS_INLINE bool nsSimdVec4b::AnySet() const
{
  const int mask = NS_BIT(N) - 1;
  return (_mm_movemask_ps(m_v) & mask) != 0;
}

template <int N>
NS_ALWAYS_INLINE bool nsSimdVec4b::NoneSet() const
{
  const int mask = NS_BIT(N) - 1;
  return (_mm_movemask_ps(m_v) & mask) == 0;
}

// static
NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4b::Select(const nsSimdVec4b& vCmp, const nsSimdVec4b& vTrue, const nsSimdVec4b& vFalse)
{
#if NS_SSE_LEVEL >= NS_SSE_41
  return _mm_blendv_ps(vFalse.m_v, vTrue.m_v, vCmp.m_v);
#else
  return _mm_or_ps(_mm_andnot_ps(cmp.m_v, ifFalse.m_v), _mm_and_ps(cmp.m_v, ifTrue.m_v));
#endif
}

#pragma once

WD_ALWAYS_INLINE wdSimdVec4b::wdSimdVec4b()
{
  WD_CHECK_SIMD_ALIGNMENT(this);
}

WD_ALWAYS_INLINE wdSimdVec4b::wdSimdVec4b(bool b)
{
  WD_CHECK_SIMD_ALIGNMENT(this);

  wdUInt32 mask = b ? 0xFFFFFFFF : 0;
  __m128 tmp = _mm_load_ss((float*)&mask);
  m_v = _mm_shuffle_ps(tmp, tmp, WD_TO_SHUFFLE(wdSwizzle::XXXX));
}

WD_ALWAYS_INLINE wdSimdVec4b::wdSimdVec4b(bool x, bool y, bool z, bool w)
{
  WD_CHECK_SIMD_ALIGNMENT(this);

  alignas(16) wdUInt32 mask[4] = {x ? 0xFFFFFFFF : 0, y ? 0xFFFFFFFF : 0, z ? 0xFFFFFFFF : 0, w ? 0xFFFFFFFF : 0};
  m_v = _mm_load_ps((float*)mask);
}

WD_ALWAYS_INLINE wdSimdVec4b::wdSimdVec4b(wdInternal::QuadBool v)
{
  m_v = v;
}

template <int N>
WD_ALWAYS_INLINE bool wdSimdVec4b::GetComponent() const
{
  return _mm_movemask_ps(_mm_shuffle_ps(m_v, m_v, WD_SHUFFLE(N, N, N, N))) != 0;
}

WD_ALWAYS_INLINE bool wdSimdVec4b::x() const
{
  return GetComponent<0>();
}

WD_ALWAYS_INLINE bool wdSimdVec4b::y() const
{
  return GetComponent<1>();
}

WD_ALWAYS_INLINE bool wdSimdVec4b::z() const
{
  return GetComponent<2>();
}

WD_ALWAYS_INLINE bool wdSimdVec4b::w() const
{
  return GetComponent<3>();
}

template <wdSwizzle::Enum s>
WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4b::Get() const
{
  return _mm_shuffle_ps(m_v, m_v, WD_TO_SHUFFLE(s));
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4b::operator&&(const wdSimdVec4b& rhs) const
{
  return _mm_and_ps(m_v, rhs.m_v);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4b::operator||(const wdSimdVec4b& rhs) const
{
  return _mm_or_ps(m_v, rhs.m_v);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4b::operator!() const
{
  __m128 allTrue = _mm_cmpeq_ps(_mm_setzero_ps(), _mm_setzero_ps());
  return _mm_xor_ps(m_v, allTrue);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4b::operator==(const wdSimdVec4b& rhs) const
{
  return !(*this != rhs);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4b::operator!=(const wdSimdVec4b& rhs) const
{
  return _mm_xor_ps(m_v, rhs.m_v);
}

template <int N>
WD_ALWAYS_INLINE bool wdSimdVec4b::AllSet() const
{
  const int mask = WD_BIT(N) - 1;
  return (_mm_movemask_ps(m_v) & mask) == mask;
}

template <int N>
WD_ALWAYS_INLINE bool wdSimdVec4b::AnySet() const
{
  const int mask = WD_BIT(N) - 1;
  return (_mm_movemask_ps(m_v) & mask) != 0;
}

template <int N>
WD_ALWAYS_INLINE bool wdSimdVec4b::NoneSet() const
{
  const int mask = WD_BIT(N) - 1;
  return (_mm_movemask_ps(m_v) & mask) == 0;
}

// static
WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4b::Select(const wdSimdVec4b& vCmp, const wdSimdVec4b& vTrue, const wdSimdVec4b& vFalse)
{
#if WD_SSE_LEVEL >= WD_SSE_41
  return _mm_blendv_ps(vFalse.m_v, vTrue.m_v, vCmp.m_v);
#else
  return _mm_or_ps(_mm_andnot_ps(cmp.m_v, ifFalse.m_v), _mm_and_ps(cmp.m_v, ifTrue.m_v));
#endif
}

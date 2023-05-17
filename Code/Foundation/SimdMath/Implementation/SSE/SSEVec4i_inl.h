#pragma once

WD_ALWAYS_INLINE wdSimdVec4i::wdSimdVec4i()
{
  WD_CHECK_SIMD_ALIGNMENT(this);

#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
  m_v = _mm_set1_epi32(0xCDCDCDCD);
#endif
}

WD_ALWAYS_INLINE wdSimdVec4i::wdSimdVec4i(wdInt32 iXyzw)
{
  WD_CHECK_SIMD_ALIGNMENT(this);

  m_v = _mm_set1_epi32(iXyzw);
}

WD_ALWAYS_INLINE wdSimdVec4i::wdSimdVec4i(wdInt32 x, wdInt32 y, wdInt32 z, wdInt32 w)
{
  WD_CHECK_SIMD_ALIGNMENT(this);

  m_v = _mm_setr_epi32(x, y, z, w);
}

WD_ALWAYS_INLINE wdSimdVec4i::wdSimdVec4i(wdInternal::QuadInt v)
{
  m_v = v;
}

WD_ALWAYS_INLINE void wdSimdVec4i::Set(wdInt32 iXyzw)
{
  m_v = _mm_set1_epi32(iXyzw);
}

WD_ALWAYS_INLINE void wdSimdVec4i::Set(wdInt32 x, wdInt32 y, wdInt32 z, wdInt32 w)
{
  m_v = _mm_setr_epi32(x, y, z, w);
}

WD_ALWAYS_INLINE void wdSimdVec4i::SetZero()
{
  m_v = _mm_setzero_si128();
}

template <>
WD_ALWAYS_INLINE void wdSimdVec4i::Load<1>(const wdInt32* pInts)
{
  m_v = _mm_loadu_si32(pInts);
}

template <>
WD_ALWAYS_INLINE void wdSimdVec4i::Load<2>(const wdInt32* pInts)
{
  m_v = _mm_loadu_si64(pInts);
}

template <>
WD_ALWAYS_INLINE void wdSimdVec4i::Load<3>(const wdInt32* pInts)
{
  m_v = _mm_setr_epi32(pInts[0], pInts[1], pInts[2], 0);
}

template <>
WD_ALWAYS_INLINE void wdSimdVec4i::Load<4>(const wdInt32* pInts)
{
  m_v = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pInts));
}

template <>
WD_ALWAYS_INLINE void wdSimdVec4i::Store<1>(wdInt32* pInts) const
{
  _mm_storeu_si32(pInts, m_v);
}

template <>
WD_ALWAYS_INLINE void wdSimdVec4i::Store<2>(wdInt32* pInts) const
{
  _mm_storeu_si64(pInts, m_v);
}

template <>
WD_ALWAYS_INLINE void wdSimdVec4i::Store<3>(wdInt32* pInts) const
{
  _mm_storeu_si64(pInts, m_v);
  _mm_storeu_si32(pInts + 2, _mm_castps_si128(_mm_movehl_ps(_mm_castsi128_ps(m_v), _mm_castsi128_ps(m_v))));
}

template <>
WD_ALWAYS_INLINE void wdSimdVec4i::Store<4>(wdInt32* pInts) const
{
  _mm_storeu_si128(reinterpret_cast<__m128i*>(pInts), m_v);
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4i::ToFloat() const
{
  return _mm_cvtepi32_ps(m_v);
}

// static
WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::Truncate(const wdSimdVec4f& f)
{
  return _mm_cvttps_epi32(f.m_v);
}

template <int N>
WD_ALWAYS_INLINE wdInt32 wdSimdVec4i::GetComponent() const
{
#if WD_SSE_LEVEL >= WD_SSE_41
  return _mm_extract_epi32(m_v, N);
#else
  return m_v.m128i_i32[N];
#endif
}

WD_ALWAYS_INLINE wdInt32 wdSimdVec4i::x() const
{
  return GetComponent<0>();
}

WD_ALWAYS_INLINE wdInt32 wdSimdVec4i::y() const
{
  return GetComponent<1>();
}

WD_ALWAYS_INLINE wdInt32 wdSimdVec4i::z() const
{
  return GetComponent<2>();
}

WD_ALWAYS_INLINE wdInt32 wdSimdVec4i::w() const
{
  return GetComponent<3>();
}

template <wdSwizzle::Enum s>
WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::Get() const
{
  return _mm_shuffle_epi32(m_v, WD_TO_SHUFFLE(s));
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::operator-() const
{
  return _mm_sub_epi32(_mm_setzero_si128(), m_v);
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::operator+(const wdSimdVec4i& v) const
{
  return _mm_add_epi32(m_v, v.m_v);
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::operator-(const wdSimdVec4i& v) const
{
  return _mm_sub_epi32(m_v, v.m_v);
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::CompMul(const wdSimdVec4i& v) const
{
#if WD_SSE_LEVEL >= WD_SSE_41
  return _mm_mullo_epi32(m_v, v.m_v);
#else
  WD_ASSERT_NOT_IMPLEMENTED; // not sure whether this code works so better assert
  __m128i tmp1 = _mm_mul_epu32(m_v, v.m_v);
  __m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(m_v, 4), _mm_srli_si128(v.m_v, 4));
  return _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, WD_SHUFFLE(0, 2, 0, 0)), _mm_shuffle_epi32(tmp2, WD_SHUFFLE(0, 2, 0, 0)));
#endif
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::CompDiv(const wdSimdVec4i& v) const
{
#if WD_ENABLED(WD_COMPILER_MSVC)
  return _mm_div_epi32(m_v, v.m_v);
#else
  int a[4];
  int b[4];
  Store<4>(a);
  v.Store<4>(b);

  for (wdUInt32 i = 0; i < 4; ++i)
  {
    a[i] = a[i] / b[i];
  }

  wdSimdVec4i r;
  r.Load<4>(a);
  return r;
#endif
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::operator|(const wdSimdVec4i& v) const
{
  return _mm_or_si128(m_v, v.m_v);
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::operator&(const wdSimdVec4i& v) const
{
  return _mm_and_si128(m_v, v.m_v);
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::operator^(const wdSimdVec4i& v) const
{
  return _mm_xor_si128(m_v, v.m_v);
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::operator~() const
{
  __m128i ones = _mm_cmpeq_epi8(_mm_setzero_si128(), _mm_setzero_si128());
  return _mm_xor_si128(ones, m_v);
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::operator<<(wdUInt32 uiShift) const
{
  return _mm_slli_epi32(m_v, uiShift);
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::operator>>(wdUInt32 uiShift) const
{
  return _mm_srai_epi32(m_v, uiShift);
}

WD_FORCE_INLINE wdSimdVec4i wdSimdVec4i::operator<<(const wdSimdVec4i& v) const
{
  int a[4];
  int b[4];
  Store<4>(a);
  v.Store<4>(b);

  for (wdUInt32 i = 0; i < 4; ++i)
  {
    a[i] = a[i] << b[i];
  }

  wdSimdVec4i r;
  r.Load<4>(a);
  return r;
}

WD_FORCE_INLINE wdSimdVec4i wdSimdVec4i::operator>>(const wdSimdVec4i& v) const
{
  int a[4];
  int b[4];
  Store<4>(a);
  v.Store<4>(b);

  for (wdUInt32 i = 0; i < 4; ++i)
  {
    a[i] = a[i] >> b[i];
  }

  wdSimdVec4i r;
  r.Load<4>(a);
  return r;
}

WD_ALWAYS_INLINE wdSimdVec4i& wdSimdVec4i::operator+=(const wdSimdVec4i& v)
{
  m_v = _mm_add_epi32(m_v, v.m_v);
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4i& wdSimdVec4i::operator-=(const wdSimdVec4i& v)
{
  m_v = _mm_sub_epi32(m_v, v.m_v);
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4i& wdSimdVec4i::operator|=(const wdSimdVec4i& v)
{
  m_v = _mm_or_si128(m_v, v.m_v);
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4i& wdSimdVec4i::operator&=(const wdSimdVec4i& v)
{
  m_v = _mm_and_si128(m_v, v.m_v);
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4i& wdSimdVec4i::operator^=(const wdSimdVec4i& v)
{
  m_v = _mm_xor_si128(m_v, v.m_v);
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4i& wdSimdVec4i::operator<<=(wdUInt32 uiShift)
{
  m_v = _mm_slli_epi32(m_v, uiShift);
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4i& wdSimdVec4i::operator>>=(wdUInt32 uiShift)
{
  m_v = _mm_srai_epi32(m_v, uiShift);
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::CompMin(const wdSimdVec4i& v) const
{
#if WD_SSE_LEVEL >= WD_SSE_41
  return _mm_min_epi32(m_v, v.m_v);
#else
  __m128i mask = _mm_cmplt_epi32(m_v, v.m_v);
  return _mm_or_si128(_mm_and_si128(mask, m_v), _mm_andnot_si128(mask, v.m_v));
#endif
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::CompMax(const wdSimdVec4i& v) const
{
#if WD_SSE_LEVEL >= WD_SSE_41
  return _mm_max_epi32(m_v, v.m_v);
#else
  __m128i mask = _mm_cmpgt_epi32(m_v, v.m_v);
  return _mm_or_si128(_mm_and_si128(mask, m_v), _mm_andnot_si128(mask, v.m_v));
#endif
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::Abs() const
{
#if WD_SSE_LEVEL >= WD_SSE_31
  return _mm_abs_epi32(m_v);
#else
  __m128i negMask = _mm_cmplt_epi32(m_v, _mm_setzero_si128());
  __m128i neg = _mm_sub_epi32(_mm_setzero_si128(), m_v);
  return _mm_or_si128(_mm_and_si128(negMask, neg), _mm_andnot_si128(negMask, m_v));
#endif
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4i::operator==(const wdSimdVec4i& v) const
{
  return _mm_castsi128_ps(_mm_cmpeq_epi32(m_v, v.m_v));
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4i::operator!=(const wdSimdVec4i& v) const
{
  return !(*this == v);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4i::operator<=(const wdSimdVec4i& v) const
{
  return !(*this > v);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4i::operator<(const wdSimdVec4i& v) const
{
  return _mm_castsi128_ps(_mm_cmplt_epi32(m_v, v.m_v));
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4i::operator>=(const wdSimdVec4i& v) const
{
  return !(*this < v);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4i::operator>(const wdSimdVec4i& v) const
{
  return _mm_castsi128_ps(_mm_cmpgt_epi32(m_v, v.m_v));
}

// static
WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::ZeroVector()
{
  return _mm_setzero_si128();
}

// static
WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::Select(const wdSimdVec4b& vCmp, const wdSimdVec4i& vTrue, const wdSimdVec4i& vFalse)
{
#if WD_SSE_LEVEL >= WD_SSE_41
  return _mm_castps_si128(_mm_blendv_ps(_mm_castsi128_ps(vFalse.m_v), _mm_castsi128_ps(vTrue.m_v), vCmp.m_v));
#else
  return _mm_castps_si128(_mm_or_ps(_mm_andnot_ps(cmp.m_v, _mm_castsi128_ps(ifFalse.m_v)), _mm_and_ps(cmp.m_v, _mm_castsi128_ps(ifTrue.m_v))));
#endif
}

// not needed atm
#if 0
void wdSimdVec4i::Transpose(wdSimdVec4i& v0, wdSimdVec4i& v1, wdSimdVec4i& v2, wdSimdVec4i& v3)
{
  __m128i T0 = _mm_unpacklo_epi32(v0.m_v, v1.m_v);
  __m128i T1 = _mm_unpacklo_epi32(v2.m_v, v3.m_v);
  __m128i T2 = _mm_unpackhi_epi32(v0.m_v, v1.m_v);
  __m128i T3 = _mm_unpackhi_epi32(v2.m_v, v3.m_v);

  v0.m_v = _mm_unpacklo_epi64(T0, T1);
  v1.m_v = _mm_unpackhi_epi64(T0, T1);
  v2.m_v = _mm_unpacklo_epi64(T2, T3);
  v3.m_v = _mm_unpackhi_epi64(T2, T3);
}
#endif

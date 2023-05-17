#pragma once

WD_ALWAYS_INLINE wdSimdVec4u::wdSimdVec4u()
{
  WD_CHECK_SIMD_ALIGNMENT(this);

#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
  m_v = _mm_set1_epi32(0xCDCDCDCD);
#endif
}

WD_ALWAYS_INLINE wdSimdVec4u::wdSimdVec4u(wdUInt32 uiXyzw)
{
  WD_CHECK_SIMD_ALIGNMENT(this);

  m_v = _mm_set1_epi32(uiXyzw);
}

WD_ALWAYS_INLINE wdSimdVec4u::wdSimdVec4u(wdUInt32 x, wdUInt32 y, wdUInt32 z, wdUInt32 w)
{
  WD_CHECK_SIMD_ALIGNMENT(this);

  m_v = _mm_setr_epi32(x, y, z, w);
}

WD_ALWAYS_INLINE wdSimdVec4u::wdSimdVec4u(wdInternal::QuadInt v)
{
  m_v = v;
}

WD_ALWAYS_INLINE void wdSimdVec4u::Set(wdUInt32 uiXyzw)
{
  m_v = _mm_set1_epi32(uiXyzw);
}

WD_ALWAYS_INLINE void wdSimdVec4u::Set(wdUInt32 x, wdUInt32 y, wdUInt32 z, wdUInt32 w)
{
  m_v = _mm_setr_epi32(x, y, z, w);
}

WD_ALWAYS_INLINE void wdSimdVec4u::SetZero()
{
  m_v = _mm_setzero_si128();
}

// needs to be implemented here because of include dependencies
WD_ALWAYS_INLINE wdSimdVec4i::wdSimdVec4i(const wdSimdVec4u& u)
  : m_v(u.m_v)
{
}

WD_ALWAYS_INLINE wdSimdVec4u::wdSimdVec4u(const wdSimdVec4i& i)
  : m_v(i.m_v)
{
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4u::ToFloat() const
{
  __m128 two16 = _mm_set1_ps((float)0x10000); // 2^16
  __m128i high = _mm_srli_epi32(m_v, 16);
  __m128i low = _mm_srli_epi32(_mm_slli_epi32(m_v, 16), 16);
  __m128 fHigh = _mm_mul_ps(_mm_cvtepi32_ps(high), two16);
  __m128 fLow = _mm_cvtepi32_ps(low);

  return _mm_add_ps(fHigh, fLow);
}

// static
WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::Truncate(const wdSimdVec4f& f)
{
  alignas(16) const float fmax[4] = {2.14748364e+009f, 2.14748364e+009f, 2.14748364e+009f, 2.14748364e+009f};
  alignas(16) const float fmax_unsigned[4] = {4.29496729e+009f, 4.29496729e+009f, 4.29496729e+009f, 4.29496729e+009f};
  __m128i zero = _mm_setzero_si128();
  __m128i mask = _mm_cmpgt_epi32(_mm_castps_si128(f.m_v), zero);
  __m128 min = _mm_and_ps(_mm_castsi128_ps(mask), f.m_v);
  __m128 max = _mm_min_ps(min, _mm_load_ps(fmax_unsigned)); // clamped in 0 - 4.29496729+009

  __m128 diff = _mm_sub_ps(max, _mm_load_ps(fmax));
  mask = _mm_cmpgt_epi32(_mm_castps_si128(diff), zero);
  diff = _mm_and_ps(_mm_castsi128_ps(mask), diff);

  __m128i res1 = _mm_cvttps_epi32(diff);
  __m128i res2 = _mm_cvttps_epi32(max);
  return _mm_add_epi32(res1, res2);
}

template <int N>
WD_ALWAYS_INLINE wdUInt32 wdSimdVec4u::GetComponent() const
{
#if WD_SSE_LEVEL >= WD_SSE_41
  return _mm_extract_epi32(m_v, N);
#else
  return m_v.m128i_i32[N];
#endif
}

WD_ALWAYS_INLINE wdUInt32 wdSimdVec4u::x() const
{
  return GetComponent<0>();
}

WD_ALWAYS_INLINE wdUInt32 wdSimdVec4u::y() const
{
  return GetComponent<1>();
}

WD_ALWAYS_INLINE wdUInt32 wdSimdVec4u::z() const
{
  return GetComponent<2>();
}

WD_ALWAYS_INLINE wdUInt32 wdSimdVec4u::w() const
{
  return GetComponent<3>();
}

template <wdSwizzle::Enum s>
WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::Get() const
{
  return _mm_shuffle_epi32(m_v, WD_TO_SHUFFLE(s));
}

WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::operator+(const wdSimdVec4u& v) const
{
  return _mm_add_epi32(m_v, v.m_v);
}

WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::operator-(const wdSimdVec4u& v) const
{
  return _mm_sub_epi32(m_v, v.m_v);
}

WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::CompMul(const wdSimdVec4u& v) const
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

WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::operator|(const wdSimdVec4u& v) const
{
  return _mm_or_si128(m_v, v.m_v);
}

WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::operator&(const wdSimdVec4u& v) const
{
  return _mm_and_si128(m_v, v.m_v);
}

WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::operator^(const wdSimdVec4u& v) const
{
  return _mm_xor_si128(m_v, v.m_v);
}

WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::operator~() const
{
  __m128i ones = _mm_cmpeq_epi8(_mm_setzero_si128(), _mm_setzero_si128());
  return _mm_xor_si128(ones, m_v);
}

WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::operator<<(wdUInt32 uiShift) const
{
  return _mm_slli_epi32(m_v, uiShift);
}

WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::operator>>(wdUInt32 uiShift) const
{
  return _mm_srli_epi32(m_v, uiShift);
}

WD_ALWAYS_INLINE wdSimdVec4u& wdSimdVec4u::operator+=(const wdSimdVec4u& v)
{
  m_v = _mm_add_epi32(m_v, v.m_v);
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4u& wdSimdVec4u::operator-=(const wdSimdVec4u& v)
{
  m_v = _mm_sub_epi32(m_v, v.m_v);
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4u& wdSimdVec4u::operator|=(const wdSimdVec4u& v)
{
  m_v = _mm_or_si128(m_v, v.m_v);
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4u& wdSimdVec4u::operator&=(const wdSimdVec4u& v)
{
  m_v = _mm_and_si128(m_v, v.m_v);
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4u& wdSimdVec4u::operator^=(const wdSimdVec4u& v)
{
  m_v = _mm_xor_si128(m_v, v.m_v);
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4u& wdSimdVec4u::operator<<=(wdUInt32 uiShift)
{
  m_v = _mm_slli_epi32(m_v, uiShift);
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4u& wdSimdVec4u::operator>>=(wdUInt32 uiShift)
{
  m_v = _mm_srli_epi32(m_v, uiShift);
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::CompMin(const wdSimdVec4u& v) const
{
#if WD_SSE_LEVEL >= WD_SSE_41
  return _mm_min_epu32(m_v, v.m_v);
#else
  __m128i mask = _mm_cmplt_epi32(m_v, v.m_v);
  return _mm_or_si128(_mm_and_si128(mask, m_v), _mm_andnot_si128(mask, v.m_v));
#endif
}

WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::CompMax(const wdSimdVec4u& v) const
{
#if WD_SSE_LEVEL >= WD_SSE_41
  return _mm_max_epu32(m_v, v.m_v);
#else
  __m128i mask = _mm_cmpgt_epi32(m_v, v.m_v);
  return _mm_or_si128(_mm_and_si128(mask, m_v), _mm_andnot_si128(mask, v.m_v));
#endif
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4u::operator==(const wdSimdVec4u& v) const
{
  return _mm_castsi128_ps(_mm_cmpeq_epi32(m_v, v.m_v));
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4u::operator!=(const wdSimdVec4u& v) const
{
  return !(*this == v);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4u::operator<=(const wdSimdVec4u& v) const
{
#if WD_SSE_LEVEL >= WD_SSE_41
  __m128i minValue = _mm_min_epu32(m_v, v.m_v);
  return _mm_castsi128_ps(_mm_cmpeq_epi32(minValue, m_v));
#else
  return !(*this > v);
#endif
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4u::operator<(const wdSimdVec4u& v) const
{
  __m128i signBit = _mm_set1_epi32(0x80000000);
  __m128i a = _mm_sub_epi32(m_v, signBit);
  __m128i b = _mm_sub_epi32(v.m_v, signBit);
  return _mm_castsi128_ps(_mm_cmplt_epi32(a, b));
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4u::operator>=(const wdSimdVec4u& v) const
{
#if WD_SSE_LEVEL >= WD_SSE_41
  __m128i maxValue = _mm_max_epu32(m_v, v.m_v);
  return _mm_castsi128_ps(_mm_cmpeq_epi32(maxValue, m_v));
#else
  return !(*this < v);
#endif
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4u::operator>(const wdSimdVec4u& v) const
{
  __m128i signBit = _mm_set1_epi32(0x80000000);
  __m128i a = _mm_sub_epi32(m_v, signBit);
  __m128i b = _mm_sub_epi32(v.m_v, signBit);
  return _mm_castsi128_ps(_mm_cmpgt_epi32(a, b));
}

// static
WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::ZeroVector()
{
  return _mm_setzero_si128();
}

// not needed atm
#if 0
void wdSimdVec4u::Transpose(wdSimdVec4u& v0, wdSimdVec4u& v1, wdSimdVec4u& v2, wdSimdVec4u& v3)
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

#pragma once

NS_ALWAYS_INLINE nsSimdVec4u::nsSimdVec4u()
{
  NS_CHECK_SIMD_ALIGNMENT(this);

#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
  m_v = _mm_set1_epi32(0xCDCDCDCD);
#endif
}

NS_ALWAYS_INLINE nsSimdVec4u::nsSimdVec4u(nsUInt32 uiXyzw)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  m_v = _mm_set1_epi32(uiXyzw);
}

NS_ALWAYS_INLINE nsSimdVec4u::nsSimdVec4u(nsUInt32 x, nsUInt32 y, nsUInt32 z, nsUInt32 w)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  m_v = _mm_setr_epi32(x, y, z, w);
}

NS_ALWAYS_INLINE nsSimdVec4u::nsSimdVec4u(nsInternal::QuadInt v)
{
  m_v = v;
}

NS_ALWAYS_INLINE void nsSimdVec4u::Set(nsUInt32 uiXyzw)
{
  m_v = _mm_set1_epi32(uiXyzw);
}

NS_ALWAYS_INLINE void nsSimdVec4u::Set(nsUInt32 x, nsUInt32 y, nsUInt32 z, nsUInt32 w)
{
  m_v = _mm_setr_epi32(x, y, z, w);
}

NS_ALWAYS_INLINE void nsSimdVec4u::SetZero()
{
  m_v = _mm_setzero_si128();
}

// needs to be implemented here because of include dependencies
NS_ALWAYS_INLINE nsSimdVec4i::nsSimdVec4i(const nsSimdVec4u& u)
  : m_v(u.m_v)
{
}

NS_ALWAYS_INLINE nsSimdVec4u::nsSimdVec4u(const nsSimdVec4i& i)
  : m_v(i.m_v)
{
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4u::ToFloat() const
{
  __m128 two16 = _mm_set1_ps((float)0x10000); // 2^16
  __m128i high = _mm_srli_epi32(m_v, 16);
  __m128i low = _mm_srli_epi32(_mm_slli_epi32(m_v, 16), 16);
  __m128 fHigh = _mm_mul_ps(_mm_cvtepi32_ps(high), two16);
  __m128 fLow = _mm_cvtepi32_ps(low);

  return _mm_add_ps(fHigh, fLow);
}

// static
NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::Truncate(const nsSimdVec4f& f)
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
NS_ALWAYS_INLINE nsUInt32 nsSimdVec4u::GetComponent() const
{
#if NS_SSE_LEVEL >= NS_SSE_41
  return _mm_extract_epi32(m_v, N);
#else
  return m_v.m128i_i32[N];
#endif
}

NS_ALWAYS_INLINE nsUInt32 nsSimdVec4u::x() const
{
  return GetComponent<0>();
}

NS_ALWAYS_INLINE nsUInt32 nsSimdVec4u::y() const
{
  return GetComponent<1>();
}

NS_ALWAYS_INLINE nsUInt32 nsSimdVec4u::z() const
{
  return GetComponent<2>();
}

NS_ALWAYS_INLINE nsUInt32 nsSimdVec4u::w() const
{
  return GetComponent<3>();
}

template <nsSwizzle::Enum s>
NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::Get() const
{
  return _mm_shuffle_epi32(m_v, NS_TO_SHUFFLE(s));
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::operator+(const nsSimdVec4u& v) const
{
  return _mm_add_epi32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::operator-(const nsSimdVec4u& v) const
{
  return _mm_sub_epi32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::CompMul(const nsSimdVec4u& v) const
{
#if NS_SSE_LEVEL >= NS_SSE_41
  return _mm_mullo_epi32(m_v, v.m_v);
#else
  NS_ASSERT_NOT_IMPLEMENTED; // not sure whether this code works so better assert
  __m128i tmp1 = _mm_mul_epu32(m_v, v.m_v);
  __m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(m_v, 4), _mm_srli_si128(v.m_v, 4));
  return _mm_unpacklo_epi32(_mm_shuffle_epi32(tmp1, NS_SHUFFLE(0, 2, 0, 0)), _mm_shuffle_epi32(tmp2, NS_SHUFFLE(0, 2, 0, 0)));
#endif
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::operator|(const nsSimdVec4u& v) const
{
  return _mm_or_si128(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::operator&(const nsSimdVec4u& v) const
{
  return _mm_and_si128(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::operator^(const nsSimdVec4u& v) const
{
  return _mm_xor_si128(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::operator~() const
{
  __m128i ones = _mm_cmpeq_epi8(_mm_setzero_si128(), _mm_setzero_si128());
  return _mm_xor_si128(ones, m_v);
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::operator<<(nsUInt32 uiShift) const
{
  return _mm_slli_epi32(m_v, uiShift);
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::operator>>(nsUInt32 uiShift) const
{
  return _mm_srli_epi32(m_v, uiShift);
}

NS_ALWAYS_INLINE nsSimdVec4u& nsSimdVec4u::operator+=(const nsSimdVec4u& v)
{
  m_v = _mm_add_epi32(m_v, v.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4u& nsSimdVec4u::operator-=(const nsSimdVec4u& v)
{
  m_v = _mm_sub_epi32(m_v, v.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4u& nsSimdVec4u::operator|=(const nsSimdVec4u& v)
{
  m_v = _mm_or_si128(m_v, v.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4u& nsSimdVec4u::operator&=(const nsSimdVec4u& v)
{
  m_v = _mm_and_si128(m_v, v.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4u& nsSimdVec4u::operator^=(const nsSimdVec4u& v)
{
  m_v = _mm_xor_si128(m_v, v.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4u& nsSimdVec4u::operator<<=(nsUInt32 uiShift)
{
  m_v = _mm_slli_epi32(m_v, uiShift);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4u& nsSimdVec4u::operator>>=(nsUInt32 uiShift)
{
  m_v = _mm_srli_epi32(m_v, uiShift);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::CompMin(const nsSimdVec4u& v) const
{
#if NS_SSE_LEVEL >= NS_SSE_41
  return _mm_min_epu32(m_v, v.m_v);
#else
  __m128i mask = _mm_cmplt_epi32(m_v, v.m_v);
  return _mm_or_si128(_mm_and_si128(mask, m_v), _mm_andnot_si128(mask, v.m_v));
#endif
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::CompMax(const nsSimdVec4u& v) const
{
#if NS_SSE_LEVEL >= NS_SSE_41
  return _mm_max_epu32(m_v, v.m_v);
#else
  __m128i mask = _mm_cmpgt_epi32(m_v, v.m_v);
  return _mm_or_si128(_mm_and_si128(mask, m_v), _mm_andnot_si128(mask, v.m_v));
#endif
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4u::operator==(const nsSimdVec4u& v) const
{
  return _mm_castsi128_ps(_mm_cmpeq_epi32(m_v, v.m_v));
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4u::operator!=(const nsSimdVec4u& v) const
{
  return !(*this == v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4u::operator<=(const nsSimdVec4u& v) const
{
#if NS_SSE_LEVEL >= NS_SSE_41
  __m128i minValue = _mm_min_epu32(m_v, v.m_v);
  return _mm_castsi128_ps(_mm_cmpeq_epi32(minValue, m_v));
#else
  return !(*this > v);
#endif
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4u::operator<(const nsSimdVec4u& v) const
{
  __m128i signBit = _mm_set1_epi32(0x80000000);
  __m128i a = _mm_sub_epi32(m_v, signBit);
  __m128i b = _mm_sub_epi32(v.m_v, signBit);
  return _mm_castsi128_ps(_mm_cmplt_epi32(a, b));
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4u::operator>=(const nsSimdVec4u& v) const
{
#if NS_SSE_LEVEL >= NS_SSE_41
  __m128i maxValue = _mm_max_epu32(m_v, v.m_v);
  return _mm_castsi128_ps(_mm_cmpeq_epi32(maxValue, m_v));
#else
  return !(*this < v);
#endif
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4u::operator>(const nsSimdVec4u& v) const
{
  __m128i signBit = _mm_set1_epi32(0x80000000);
  __m128i a = _mm_sub_epi32(m_v, signBit);
  __m128i b = _mm_sub_epi32(v.m_v, signBit);
  return _mm_castsi128_ps(_mm_cmpgt_epi32(a, b));
}

// static
NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::MakeZero()
{
  return _mm_setzero_si128();
}

// not needed atm
#if 0
void nsSimdVec4u::Transpose(nsSimdVec4u& v0, nsSimdVec4u& v1, nsSimdVec4u& v2, nsSimdVec4u& v3)
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

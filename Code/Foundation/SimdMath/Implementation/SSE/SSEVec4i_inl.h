#pragma once

NS_ALWAYS_INLINE nsSimdVec4i::nsSimdVec4i()
{
  NS_CHECK_SIMD_ALIGNMENT(this);

#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
  m_v = _mm_set1_epi32(0xCDCDCDCD);
#endif
}

NS_ALWAYS_INLINE nsSimdVec4i::nsSimdVec4i(nsInt32 iXyzw)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  m_v = _mm_set1_epi32(iXyzw);
}

NS_ALWAYS_INLINE nsSimdVec4i::nsSimdVec4i(nsInt32 x, nsInt32 y, nsInt32 z, nsInt32 w)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  m_v = _mm_setr_epi32(x, y, z, w);
}

NS_ALWAYS_INLINE nsSimdVec4i::nsSimdVec4i(nsInternal::QuadInt v)
{
  m_v = v;
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::MakeZero()
{
  return _mm_setzero_si128();
}

NS_ALWAYS_INLINE void nsSimdVec4i::Set(nsInt32 iXyzw)
{
  m_v = _mm_set1_epi32(iXyzw);
}

NS_ALWAYS_INLINE void nsSimdVec4i::Set(nsInt32 x, nsInt32 y, nsInt32 z, nsInt32 w)
{
  m_v = _mm_setr_epi32(x, y, z, w);
}

NS_ALWAYS_INLINE void nsSimdVec4i::SetZero()
{
  m_v = _mm_setzero_si128();
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4i::Load<1>(const nsInt32* pInts)
{
  m_v = _mm_loadu_si32(pInts);
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4i::Load<2>(const nsInt32* pInts)
{
  m_v = _mm_loadu_si64(pInts);
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4i::Load<3>(const nsInt32* pInts)
{
  m_v = _mm_setr_epi32(pInts[0], pInts[1], pInts[2], 0);
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4i::Load<4>(const nsInt32* pInts)
{
  m_v = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pInts));
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4i::Store<1>(nsInt32* pInts) const
{
  _mm_storeu_si32(pInts, m_v);
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4i::Store<2>(nsInt32* pInts) const
{
  _mm_storeu_si64(pInts, m_v);
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4i::Store<3>(nsInt32* pInts) const
{
  _mm_storeu_si64(pInts, m_v);
  _mm_storeu_si32(pInts + 2, _mm_castps_si128(_mm_movehl_ps(_mm_castsi128_ps(m_v), _mm_castsi128_ps(m_v))));
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4i::Store<4>(nsInt32* pInts) const
{
  _mm_storeu_si128(reinterpret_cast<__m128i*>(pInts), m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4i::ToFloat() const
{
  return _mm_cvtepi32_ps(m_v);
}

// static
NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::Truncate(const nsSimdVec4f& f)
{
  return _mm_cvttps_epi32(f.m_v);
}

template <int N>
NS_ALWAYS_INLINE nsInt32 nsSimdVec4i::GetComponent() const
{
#if NS_SSE_LEVEL >= NS_SSE_41
  return _mm_extract_epi32(m_v, N);
#else
  return m_v.m128i_i32[N];
#endif
}

NS_ALWAYS_INLINE nsInt32 nsSimdVec4i::x() const
{
  return GetComponent<0>();
}

NS_ALWAYS_INLINE nsInt32 nsSimdVec4i::y() const
{
  return GetComponent<1>();
}

NS_ALWAYS_INLINE nsInt32 nsSimdVec4i::z() const
{
  return GetComponent<2>();
}

NS_ALWAYS_INLINE nsInt32 nsSimdVec4i::w() const
{
  return GetComponent<3>();
}

template <nsSwizzle::Enum s>
NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::Get() const
{
  return _mm_shuffle_epi32(m_v, NS_TO_SHUFFLE(s));
}

template <nsSwizzle::Enum s>
NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::GetCombined(const nsSimdVec4i& other) const
{
  return _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(m_v), _mm_castsi128_ps(other.m_v), NS_TO_SHUFFLE(s)));
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator-() const
{
  return _mm_sub_epi32(_mm_setzero_si128(), m_v);
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator+(const nsSimdVec4i& v) const
{
  return _mm_add_epi32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator-(const nsSimdVec4i& v) const
{
  return _mm_sub_epi32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::CompMul(const nsSimdVec4i& v) const
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

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::CompDiv(const nsSimdVec4i& v) const
{
#if NS_ENABLED(NS_COMPILER_MSVC)
  return _mm_div_epi32(m_v, v.m_v);
#else
  int a[4];
  int b[4];
  Store<4>(a);
  v.Store<4>(b);

  for (nsUInt32 i = 0; i < 4; ++i)
  {
    a[i] = a[i] / b[i];
  }

  nsSimdVec4i r;
  r.Load<4>(a);
  return r;
#endif
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator|(const nsSimdVec4i& v) const
{
  return _mm_or_si128(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator&(const nsSimdVec4i& v) const
{
  return _mm_and_si128(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator^(const nsSimdVec4i& v) const
{
  return _mm_xor_si128(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator~() const
{
  __m128i ones = _mm_cmpeq_epi8(_mm_setzero_si128(), _mm_setzero_si128());
  return _mm_xor_si128(ones, m_v);
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator<<(nsUInt32 uiShift) const
{
  return _mm_slli_epi32(m_v, uiShift);
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator>>(nsUInt32 uiShift) const
{
  return _mm_srai_epi32(m_v, uiShift);
}

NS_FORCE_INLINE nsSimdVec4i nsSimdVec4i::operator<<(const nsSimdVec4i& v) const
{
  int a[4];
  int b[4];
  Store<4>(a);
  v.Store<4>(b);

  for (nsUInt32 i = 0; i < 4; ++i)
  {
    a[i] = a[i] << b[i];
  }

  nsSimdVec4i r;
  r.Load<4>(a);
  return r;
}

NS_FORCE_INLINE nsSimdVec4i nsSimdVec4i::operator>>(const nsSimdVec4i& v) const
{
  int a[4];
  int b[4];
  Store<4>(a);
  v.Store<4>(b);

  for (nsUInt32 i = 0; i < 4; ++i)
  {
    a[i] = a[i] >> b[i];
  }

  nsSimdVec4i r;
  r.Load<4>(a);
  return r;
}

NS_ALWAYS_INLINE nsSimdVec4i& nsSimdVec4i::operator+=(const nsSimdVec4i& v)
{
  m_v = _mm_add_epi32(m_v, v.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4i& nsSimdVec4i::operator-=(const nsSimdVec4i& v)
{
  m_v = _mm_sub_epi32(m_v, v.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4i& nsSimdVec4i::operator|=(const nsSimdVec4i& v)
{
  m_v = _mm_or_si128(m_v, v.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4i& nsSimdVec4i::operator&=(const nsSimdVec4i& v)
{
  m_v = _mm_and_si128(m_v, v.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4i& nsSimdVec4i::operator^=(const nsSimdVec4i& v)
{
  m_v = _mm_xor_si128(m_v, v.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4i& nsSimdVec4i::operator<<=(nsUInt32 uiShift)
{
  m_v = _mm_slli_epi32(m_v, uiShift);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4i& nsSimdVec4i::operator>>=(nsUInt32 uiShift)
{
  m_v = _mm_srai_epi32(m_v, uiShift);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::CompMin(const nsSimdVec4i& v) const
{
#if NS_SSE_LEVEL >= NS_SSE_41
  return _mm_min_epi32(m_v, v.m_v);
#else
  __m128i mask = _mm_cmplt_epi32(m_v, v.m_v);
  return _mm_or_si128(_mm_and_si128(mask, m_v), _mm_andnot_si128(mask, v.m_v));
#endif
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::CompMax(const nsSimdVec4i& v) const
{
#if NS_SSE_LEVEL >= NS_SSE_41
  return _mm_max_epi32(m_v, v.m_v);
#else
  __m128i mask = _mm_cmpgt_epi32(m_v, v.m_v);
  return _mm_or_si128(_mm_and_si128(mask, m_v), _mm_andnot_si128(mask, v.m_v));
#endif
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::Abs() const
{
#if NS_SSE_LEVEL >= NS_SSE_31
  return _mm_abs_epi32(m_v);
#else
  __m128i negMask = _mm_cmplt_epi32(m_v, _mm_setzero_si128());
  __m128i neg = _mm_sub_epi32(_mm_setzero_si128(), m_v);
  return _mm_or_si128(_mm_and_si128(negMask, neg), _mm_andnot_si128(negMask, m_v));
#endif
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4i::operator==(const nsSimdVec4i& v) const
{
  return _mm_castsi128_ps(_mm_cmpeq_epi32(m_v, v.m_v));
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4i::operator!=(const nsSimdVec4i& v) const
{
  return !(*this == v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4i::operator<=(const nsSimdVec4i& v) const
{
  return !(*this > v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4i::operator<(const nsSimdVec4i& v) const
{
  return _mm_castsi128_ps(_mm_cmplt_epi32(m_v, v.m_v));
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4i::operator>=(const nsSimdVec4i& v) const
{
  return !(*this < v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4i::operator>(const nsSimdVec4i& v) const
{
  return _mm_castsi128_ps(_mm_cmpgt_epi32(m_v, v.m_v));
}

// static
NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::Select(const nsSimdVec4b& vCmp, const nsSimdVec4i& vTrue, const nsSimdVec4i& vFalse)
{
#if NS_SSE_LEVEL >= NS_SSE_41
  return _mm_castps_si128(_mm_blendv_ps(_mm_castsi128_ps(vFalse.m_v), _mm_castsi128_ps(vTrue.m_v), vCmp.m_v));
#else
  return _mm_castps_si128(_mm_or_ps(_mm_andnot_ps(cmp.m_v, _mm_castsi128_ps(ifFalse.m_v)), _mm_and_ps(cmp.m_v, _mm_castsi128_ps(ifTrue.m_v))));
#endif
}

// not needed atm
#if 0
void nsSimdVec4i::Transpose(nsSimdVec4i& v0, nsSimdVec4i& v1, nsSimdVec4i& v2, nsSimdVec4i& v3)
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

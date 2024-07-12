#pragma once

NS_ALWAYS_INLINE nsSimdVec4f::nsSimdVec4f()
{
  NS_CHECK_SIMD_ALIGNMENT(this);

#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
  // Initialize all data to NaN in debug mode to find problems with uninitialized data easier.
  m_v = _mm_set1_ps(nsMath::NaN<float>());
#endif
}

NS_ALWAYS_INLINE nsSimdVec4f::nsSimdVec4f(float fXyzw)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  m_v = _mm_set1_ps(fXyzw);
}

NS_ALWAYS_INLINE nsSimdVec4f::nsSimdVec4f(const nsSimdFloat& fXyzw)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  m_v = fXyzw.m_v;
}

NS_ALWAYS_INLINE nsSimdVec4f::nsSimdVec4f(float x, float y, float z, float w)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  m_v = _mm_setr_ps(x, y, z, w);
}

NS_ALWAYS_INLINE void nsSimdVec4f::Set(float fXyzw)
{
  m_v = _mm_set1_ps(fXyzw);
}

NS_ALWAYS_INLINE void nsSimdVec4f::Set(float x, float y, float z, float w)
{
  m_v = _mm_setr_ps(x, y, z, w);
}

NS_ALWAYS_INLINE void nsSimdVec4f::SetX(const nsSimdFloat& f)
{
  m_v = _mm_move_ss(m_v, f.m_v);
}

NS_ALWAYS_INLINE void nsSimdVec4f::SetY(const nsSimdFloat& f)
{
  m_v = _mm_shuffle_ps(_mm_unpacklo_ps(m_v, f.m_v), m_v, NS_TO_SHUFFLE(nsSwizzle::XYZW));
}

NS_ALWAYS_INLINE void nsSimdVec4f::SetZ(const nsSimdFloat& f)
{
  m_v = _mm_shuffle_ps(m_v, _mm_unpackhi_ps(f.m_v, m_v), NS_TO_SHUFFLE(nsSwizzle::XYZW));
}

NS_ALWAYS_INLINE void nsSimdVec4f::SetW(const nsSimdFloat& f)
{
  m_v = _mm_shuffle_ps(m_v, _mm_unpackhi_ps(m_v, f.m_v), NS_TO_SHUFFLE(nsSwizzle::XYXY));
}

NS_ALWAYS_INLINE void nsSimdVec4f::SetZero()
{
  m_v = _mm_setzero_ps();
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4f::Load<1>(const float* pFloat)
{
  m_v = _mm_load_ss(pFloat);
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4f::Load<2>(const float* pFloat)
{
  m_v = _mm_castpd_ps(_mm_load_sd(reinterpret_cast<const double*>(pFloat)));
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4f::Load<3>(const float* pFloat)
{
// There is a compiler bug in GCC where GCC will incorrectly optimize the alternative faster implementation.
#if NS_ENABLED(NS_COMPILER_GCC)
  m_v = _mm_set_ps(0.0f, pFloat[2], pFloat[1], pFloat[0]);
#else
  m_v = _mm_movelh_ps(_mm_castpd_ps(_mm_load_sd(reinterpret_cast<const double*>(pFloat))), _mm_load_ss(pFloat + 2));
#endif
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4f::Load<4>(const float* pFloat)
{
  m_v = _mm_loadu_ps(pFloat);
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4f::Store<1>(float* pFloat) const
{
  _mm_store_ss(pFloat, m_v);
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4f::Store<2>(float* pFloat) const
{
  _mm_store_sd(reinterpret_cast<double*>(pFloat), _mm_castps_pd(m_v));
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4f::Store<3>(float* pFloat) const
{
  _mm_store_sd(reinterpret_cast<double*>(pFloat), _mm_castps_pd(m_v));
  _mm_store_ss(pFloat + 2, _mm_movehl_ps(m_v, m_v));
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4f::Store<4>(float* pFloat) const
{
  _mm_storeu_ps(pFloat, m_v);
}

template <>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetReciprocal<nsMathAcc::BITS_12>() const
{
  return _mm_rcp_ps(m_v);
}

template <>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetReciprocal<nsMathAcc::BITS_23>() const
{
  __m128 x0 = _mm_rcp_ps(m_v);

  // One Newton-Raphson iteration
  __m128 x1 = _mm_mul_ps(x0, _mm_sub_ps(_mm_set1_ps(2.0f), _mm_mul_ps(m_v, x0)));

  return x1;
}

template <>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetReciprocal<nsMathAcc::FULL>() const
{
  return _mm_div_ps(_mm_set1_ps(1.0f), m_v);
}

template <>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetSqrt<nsMathAcc::BITS_12>() const
{
  return _mm_mul_ps(m_v, _mm_rsqrt_ps(m_v));
}

template <>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetSqrt<nsMathAcc::BITS_23>() const
{
  __m128 x0 = _mm_rsqrt_ps(m_v);

  // One iteration of Newton-Raphson
  __m128 x1 = _mm_mul_ps(_mm_mul_ps(_mm_set1_ps(0.5f), x0), _mm_sub_ps(_mm_set1_ps(3.0f), _mm_mul_ps(_mm_mul_ps(m_v, x0), x0)));

  return _mm_mul_ps(m_v, x1);
}

template <>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetSqrt<nsMathAcc::FULL>() const
{
  return _mm_sqrt_ps(m_v);
}

template <>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetInvSqrt<nsMathAcc::FULL>() const
{
  return _mm_div_ps(_mm_set1_ps(1.0f), _mm_sqrt_ps(m_v));
}

template <>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetInvSqrt<nsMathAcc::BITS_23>() const
{
  const __m128 x0 = _mm_rsqrt_ps(m_v);

  // One iteration of Newton-Raphson
  return _mm_mul_ps(_mm_mul_ps(_mm_set1_ps(0.5f), x0), _mm_sub_ps(_mm_set1_ps(3.0f), _mm_mul_ps(_mm_mul_ps(m_v, x0), x0)));
}

template <>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetInvSqrt<nsMathAcc::BITS_12>() const
{
  return _mm_rsqrt_ps(m_v);
}

template <int N, nsMathAcc::Enum acc>
void nsSimdVec4f::NormalizeIfNotZero(const nsSimdFloat& fEpsilon)
{
  nsSimdFloat sqLength = GetLengthSquared<N>();
  __m128 isNotZero = _mm_cmpgt_ps(sqLength.m_v, fEpsilon.m_v);
  m_v = _mm_mul_ps(m_v, sqLength.GetInvSqrt<acc>().m_v);
  m_v = _mm_and_ps(isNotZero, m_v);
}

template <int N>
NS_ALWAYS_INLINE bool nsSimdVec4f::IsZero() const
{
  const int mask = NS_BIT(N) - 1;
  return (_mm_movemask_ps(_mm_cmpeq_ps(m_v, _mm_setzero_ps())) & mask) == mask;
}

template <int N>
NS_ALWAYS_INLINE bool nsSimdVec4f::IsZero(const nsSimdFloat& fEpsilon) const
{
  const int mask = NS_BIT(N) - 1;
  __m128 absVal = Abs().m_v;
  return (_mm_movemask_ps(_mm_cmplt_ps(absVal, fEpsilon.m_v)) & mask) == mask;
}

template <int N>
inline bool nsSimdVec4f::IsNaN() const
{
  // NAN -> (exponent = all 1, mantissa = non-zero)

  alignas(16) const nsUInt32 s_exponentMask[4] = {0x7f800000, 0x7f800000, 0x7f800000, 0x7f800000};
  alignas(16) const nsUInt32 s_mantissaMask[4] = {0x7FFFFF, 0x7FFFFF, 0x7FFFFF, 0x7FFFFF};

  __m128 exponentMask = _mm_load_ps(reinterpret_cast<const float*>(s_exponentMask));
  __m128 mantissaMask = _mm_load_ps(reinterpret_cast<const float*>(s_mantissaMask));

  __m128 exponentAll1 = _mm_cmpeq_ps(_mm_and_ps(m_v, exponentMask), exponentMask);
  __m128 mantissaNon0 = _mm_cmpneq_ps(_mm_and_ps(m_v, mantissaMask), _mm_setzero_ps());

  const int mask = NS_BIT(N) - 1;
  return (_mm_movemask_ps(_mm_and_ps(exponentAll1, mantissaNon0)) & mask) != 0;
}

template <int N>
NS_ALWAYS_INLINE bool nsSimdVec4f::IsValid() const
{
  // Check the 8 exponent bits.
  // NAN -> (exponent = all 1, mantissa = non-zero)
  // INF -> (exponent = all 1, mantissa = zero)

  alignas(16) const nsUInt32 s_exponentMask[4] = {0x7f800000, 0x7f800000, 0x7f800000, 0x7f800000};

  __m128 exponentMask = _mm_load_ps(reinterpret_cast<const float*>(s_exponentMask));

  __m128 exponentNot1 = _mm_cmpneq_ps(_mm_and_ps(m_v, exponentMask), exponentMask);

  const int mask = NS_BIT(N) - 1;
  return (_mm_movemask_ps(exponentNot1) & mask) == mask;
}

template <int N>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::GetComponent() const
{
  return _mm_shuffle_ps(m_v, m_v, NS_SHUFFLE(N, N, N, N));
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::x() const
{
  return GetComponent<0>();
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::y() const
{
  return GetComponent<1>();
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::z() const
{
  return GetComponent<2>();
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::w() const
{
  return GetComponent<3>();
}

template <nsSwizzle::Enum s>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::Get() const
{
  return _mm_shuffle_ps(m_v, m_v, NS_TO_SHUFFLE(s));
}

template <nsSwizzle::Enum s>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetCombined(const nsSimdVec4f& other) const
{
  return _mm_shuffle_ps(m_v, other.m_v, NS_TO_SHUFFLE(s));
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::operator-() const
{
  return _mm_sub_ps(_mm_setzero_ps(), m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::operator+(const nsSimdVec4f& v) const
{
  return _mm_add_ps(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::operator-(const nsSimdVec4f& v) const
{
  return _mm_sub_ps(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::operator*(const nsSimdFloat& f) const
{
  return _mm_mul_ps(m_v, f.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::operator/(const nsSimdFloat& f) const
{
  return _mm_div_ps(m_v, f.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::CompMul(const nsSimdVec4f& v) const
{
  return _mm_mul_ps(m_v, v.m_v);
}

template <>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::CompDiv<nsMathAcc::FULL>(const nsSimdVec4f& v) const
{
  return _mm_div_ps(m_v, v.m_v);
}

template <>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::CompDiv<nsMathAcc::BITS_23>(const nsSimdVec4f& v) const
{
  __m128 x0 = _mm_rcp_ps(v.m_v);

  // One iteration of Newton-Raphson
  __m128 x1 = _mm_mul_ps(x0, _mm_sub_ps(_mm_set1_ps(2.0f), _mm_mul_ps(v.m_v, x0)));

  return _mm_mul_ps(m_v, x1);
}

template <>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::CompDiv<nsMathAcc::BITS_12>(const nsSimdVec4f& v) const
{
  return _mm_mul_ps(m_v, _mm_rcp_ps(v.m_v));
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::CompMin(const nsSimdVec4f& v) const
{
  return _mm_min_ps(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::CompMax(const nsSimdVec4f& v) const
{
  return _mm_max_ps(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::Abs() const
{
  return _mm_andnot_ps(_mm_set1_ps(-0.0f), m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::Round() const
{
#if NS_SSE_LEVEL >= NS_SSE_41
  return _mm_round_ps(m_v, _MM_FROUND_NINT);
#else
  NS_ASSERT_NOT_IMPLEMENTED;
#endif
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::Floor() const
{
#if NS_SSE_LEVEL >= NS_SSE_41
  return _mm_round_ps(m_v, _MM_FROUND_FLOOR);
#else
  NS_ASSERT_NOT_IMPLEMENTED;
#endif
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::Ceil() const
{
#if NS_SSE_LEVEL >= NS_SSE_41
  return _mm_round_ps(m_v, _MM_FROUND_CEIL);
#else
  NS_ASSERT_NOT_IMPLEMENTED;
#endif
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::Trunc() const
{
#if NS_SSE_LEVEL >= NS_SSE_41
  return _mm_round_ps(m_v, _MM_FROUND_TRUNC);
#else
  NS_ASSERT_NOT_IMPLEMENTED;
#endif
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::FlipSign(const nsSimdVec4b& vCmp) const
{
  return _mm_xor_ps(m_v, _mm_and_ps(vCmp.m_v, _mm_set1_ps(-0.0f)));
}

// static
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::Select(const nsSimdVec4b& vCmp, const nsSimdVec4f& vTrue, const nsSimdVec4f& vFalse)
{
#if NS_SSE_LEVEL >= NS_SSE_41
  return _mm_blendv_ps(vFalse.m_v, vTrue.m_v, vCmp.m_v);
#else
  return _mm_or_ps(_mm_andnot_ps(cmp.m_v, ifFalse.m_v), _mm_and_ps(cmp.m_v, ifTrue.m_v));
#endif
}

NS_ALWAYS_INLINE nsSimdVec4f& nsSimdVec4f::operator+=(const nsSimdVec4f& v)
{
  m_v = _mm_add_ps(m_v, v.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4f& nsSimdVec4f::operator-=(const nsSimdVec4f& v)
{
  m_v = _mm_sub_ps(m_v, v.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4f& nsSimdVec4f::operator*=(const nsSimdFloat& f)
{
  m_v = _mm_mul_ps(m_v, f.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4f& nsSimdVec4f::operator/=(const nsSimdFloat& f)
{
  m_v = _mm_div_ps(m_v, f.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4f::operator==(const nsSimdVec4f& v) const
{
  return _mm_cmpeq_ps(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4f::operator!=(const nsSimdVec4f& v) const
{
  return _mm_cmpneq_ps(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4f::operator<=(const nsSimdVec4f& v) const
{
  return _mm_cmple_ps(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4f::operator<(const nsSimdVec4f& v) const
{
  return _mm_cmplt_ps(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4f::operator>=(const nsSimdVec4f& v) const
{
  return _mm_cmpge_ps(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4f::operator>(const nsSimdVec4f& v) const
{
  return _mm_cmpgt_ps(m_v, v.m_v);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalSum<2>() const
{
#if NS_SSE_LEVEL >= NS_SSE_31
  __m128 a = _mm_hadd_ps(m_v, m_v);
  return _mm_shuffle_ps(a, a, NS_TO_SHUFFLE(nsSwizzle::XXXX));
#else
  return GetComponent<0>() + GetComponent<1>();
#endif
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalSum<3>() const
{
  return HorizontalSum<2>() + GetComponent<2>();
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalSum<4>() const
{
#if NS_SSE_LEVEL >= NS_SSE_31
  __m128 a = _mm_hadd_ps(m_v, m_v);
  return _mm_hadd_ps(a, a);
#else
  return (GetComponent<0>() + GetComponent<1>()) + (GetComponent<2>() + GetComponent<3>());
#endif
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalMin<2>() const
{
  return _mm_min_ps(GetComponent<0>().m_v, GetComponent<1>().m_v);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalMin<3>() const
{
  return _mm_min_ps(_mm_min_ps(GetComponent<0>().m_v, GetComponent<1>().m_v), GetComponent<2>().m_v);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalMin<4>() const
{
  __m128 xyxyzwzw = _mm_min_ps(_mm_shuffle_ps(m_v, m_v, NS_TO_SHUFFLE(nsSwizzle::ZWXY)), m_v);
  __m128 zwzwxyxy = _mm_shuffle_ps(xyxyzwzw, xyxyzwzw, NS_TO_SHUFFLE(nsSwizzle::YXWZ));
  return _mm_min_ps(xyxyzwzw, zwzwxyxy);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalMax<2>() const
{
  return _mm_max_ps(GetComponent<0>().m_v, GetComponent<1>().m_v);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalMax<3>() const
{
  return _mm_max_ps(_mm_max_ps(GetComponent<0>().m_v, GetComponent<1>().m_v), GetComponent<2>().m_v);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalMax<4>() const
{
  __m128 xyxyzwzw = _mm_max_ps(_mm_shuffle_ps(m_v, m_v, NS_TO_SHUFFLE(nsSwizzle::ZWXY)), m_v);
  __m128 zwzwxyxy = _mm_shuffle_ps(xyxyzwzw, xyxyzwzw, NS_TO_SHUFFLE(nsSwizzle::YXWZ));
  return _mm_max_ps(xyxyzwzw, zwzwxyxy);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::Dot<1>(const nsSimdVec4f& v) const
{
#if NS_SSE_LEVEL >= NS_SSE_41
  return _mm_dp_ps(m_v, v.m_v, 0x1f);
#else
  return CompMul(v).HorizontalSum<1>();
#endif
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::Dot<2>(const nsSimdVec4f& v) const
{
#if NS_SSE_LEVEL >= NS_SSE_41
  return _mm_dp_ps(m_v, v.m_v, 0x3f);
#else
  return CompMul(v).HorizontalSum<2>();
#endif
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::Dot<3>(const nsSimdVec4f& v) const
{
#if NS_SSE_LEVEL >= NS_SSE_41
  return _mm_dp_ps(m_v, v.m_v, 0x7f);
#else
  return CompMul(v).HorizontalSum<3>();
#endif
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::Dot<4>(const nsSimdVec4f& v) const
{
#if NS_SSE_LEVEL >= NS_SSE_41
  return _mm_dp_ps(m_v, v.m_v, 0xff);
#else
  return CompMul(v).HorizontalSum<4>();
#endif
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::CrossRH(const nsSimdVec4f& v) const
{
  __m128 a = _mm_mul_ps(m_v, _mm_shuffle_ps(v.m_v, v.m_v, NS_TO_SHUFFLE(nsSwizzle::YZXW)));
  __m128 b = _mm_mul_ps(v.m_v, _mm_shuffle_ps(m_v, m_v, NS_TO_SHUFFLE(nsSwizzle::YZXW)));
  __m128 c = _mm_sub_ps(a, b);

  return _mm_shuffle_ps(c, c, NS_TO_SHUFFLE(nsSwizzle::YZXW));
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetOrthogonalVector() const
{
  // See http://blog.selfshadow.com/2011/10/17/perp-vectors/ - this is Stark's first variant, SIMDified.
  return CrossRH(_mm_and_ps(m_v, _mm_cmpeq_ps(m_v, HorizontalMin<3>().m_v)));
}

// static
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::MulAdd(const nsSimdVec4f& a, const nsSimdVec4f& b, const nsSimdVec4f& c)
{
#if NS_SSE_LEVEL >= NS_SSE_AVX2
  return _mm_fmadd_ps(a.m_v, b.m_v, c.m_v);
#else
  return a.CompMul(b) + c;
#endif
}

// static
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::MulAdd(const nsSimdVec4f& a, const nsSimdFloat& b, const nsSimdVec4f& c)
{
#if NS_SSE_LEVEL >= NS_SSE_AVX2
  return _mm_fmadd_ps(a.m_v, b.m_v, c.m_v);
#else
  return a * b + c;
#endif
}

// static
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::MulSub(const nsSimdVec4f& a, const nsSimdVec4f& b, const nsSimdVec4f& c)
{
#if NS_SSE_LEVEL >= NS_SSE_AVX2
  return _mm_fmsub_ps(a.m_v, b.m_v, c.m_v);
#else
  return a.CompMul(b) - c;
#endif
}

// static
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::MulSub(const nsSimdVec4f& a, const nsSimdFloat& b, const nsSimdVec4f& c)
{
#if NS_SSE_LEVEL >= NS_SSE_AVX2
  return _mm_fmsub_ps(a.m_v, b.m_v, c.m_v);
#else
  return a * b - c;
#endif
}

// static
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::CopySign(const nsSimdVec4f& vMagnitude, const nsSimdVec4f& vSign)
{
  __m128 minusZero = _mm_set1_ps(-0.0f);
  return _mm_or_ps(_mm_andnot_ps(minusZero, vMagnitude.m_v), _mm_and_ps(minusZero, vSign.m_v));
}

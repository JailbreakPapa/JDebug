#pragma once

NS_ALWAYS_INLINE nsSimdFloat::nsSimdFloat()
{
  NS_CHECK_SIMD_ALIGNMENT(this);

#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
  // Initialize all data to NaN in debug mode to find problems with uninitialized data easier.
  m_v = _mm_set1_ps(nsMath::NaN<float>());
#endif
}

NS_ALWAYS_INLINE nsSimdFloat::nsSimdFloat(float f)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  m_v = _mm_set1_ps(f);
}

NS_ALWAYS_INLINE nsSimdFloat::nsSimdFloat(nsInt32 i)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  __m128 v = _mm_cvtsi32_ss(_mm_setzero_ps(), i);
  m_v = _mm_shuffle_ps(v, v, NS_TO_SHUFFLE(nsSwizzle::XXXX));
}

NS_ALWAYS_INLINE nsSimdFloat::nsSimdFloat(nsUInt32 i)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

#if NS_ENABLED(NS_PLATFORM_64BIT)
  __m128 v = _mm_cvtsi64_ss(_mm_setzero_ps(), i);
#else
  __m128 v = _mm_cvtsi32_ss(_mm_setzero_ps(), i);
#endif
  m_v = _mm_shuffle_ps(v, v, NS_TO_SHUFFLE(nsSwizzle::XXXX));
}

NS_ALWAYS_INLINE nsSimdFloat::nsSimdFloat(nsAngle a)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  m_v = _mm_set1_ps(a.GetRadian());
}

NS_ALWAYS_INLINE nsSimdFloat::nsSimdFloat(nsInternal::QuadFloat v)
{
  m_v = v;
}

NS_ALWAYS_INLINE nsSimdFloat::operator float() const
{
  float f;
  _mm_store_ss(&f, m_v);
  return f;
}

// static
NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::MakeZero()
{
  return _mm_setzero_ps();
}

// static
NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::MakeNaN()
{
  return _mm_set1_ps(nsMath::NaN<float>());
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::operator+(const nsSimdFloat& f) const
{
  return _mm_add_ps(m_v, f.m_v);
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::operator-(const nsSimdFloat& f) const
{
  return _mm_sub_ps(m_v, f.m_v);
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::operator*(const nsSimdFloat& f) const
{
  return _mm_mul_ps(m_v, f.m_v);
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::operator/(const nsSimdFloat& f) const
{
  return _mm_div_ps(m_v, f.m_v);
}

NS_ALWAYS_INLINE nsSimdFloat& nsSimdFloat::operator+=(const nsSimdFloat& f)
{
  m_v = _mm_add_ps(m_v, f.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdFloat& nsSimdFloat::operator-=(const nsSimdFloat& f)
{
  m_v = _mm_sub_ps(m_v, f.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdFloat& nsSimdFloat::operator*=(const nsSimdFloat& f)
{
  m_v = _mm_mul_ps(m_v, f.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdFloat& nsSimdFloat::operator/=(const nsSimdFloat& f)
{
  m_v = _mm_div_ps(m_v, f.m_v);
  return *this;
}

NS_ALWAYS_INLINE bool nsSimdFloat::IsEqual(const nsSimdFloat& rhs, const nsSimdFloat& fEpsilon) const
{
  nsSimdFloat minusEps = rhs - fEpsilon;
  nsSimdFloat plusEps = rhs + fEpsilon;
  return ((*this >= minusEps) && (*this <= plusEps));
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator==(const nsSimdFloat& f) const
{
  return _mm_comieq_ss(m_v, f.m_v) == 1;
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator!=(const nsSimdFloat& f) const
{
  return _mm_comineq_ss(m_v, f.m_v) == 1;
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator>=(const nsSimdFloat& f) const
{
  return _mm_comige_ss(m_v, f.m_v) == 1;
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator>(const nsSimdFloat& f) const
{
  return _mm_comigt_ss(m_v, f.m_v) == 1;
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator<=(const nsSimdFloat& f) const
{
  return _mm_comile_ss(m_v, f.m_v) == 1;
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator<(const nsSimdFloat& f) const
{
  return _mm_comilt_ss(m_v, f.m_v) == 1;
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator==(float f) const
{
  return (*this) == nsSimdFloat(f);
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator!=(float f) const
{
  return (*this) != nsSimdFloat(f);
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator>(float f) const
{
  return (*this) > nsSimdFloat(f);
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator>=(float f) const
{
  return (*this) >= nsSimdFloat(f);
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator<(float f) const
{
  return (*this) < nsSimdFloat(f);
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator<=(float f) const
{
  return (*this) <= nsSimdFloat(f);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::GetReciprocal<nsMathAcc::FULL>() const
{
  return _mm_div_ps(_mm_set1_ps(1.0f), m_v);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::GetReciprocal<nsMathAcc::BITS_23>() const
{
  __m128 x0 = _mm_rcp_ps(m_v);

  // One iteration of Newton-Raphson
  __m128 x1 = _mm_mul_ps(x0, _mm_sub_ps(_mm_set1_ps(2.0f), _mm_mul_ps(m_v, x0)));

  return x1;
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::GetReciprocal<nsMathAcc::BITS_12>() const
{
  return _mm_rcp_ps(m_v);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::GetInvSqrt<nsMathAcc::FULL>() const
{
  return _mm_div_ps(_mm_set1_ps(1.0f), _mm_sqrt_ps(m_v));
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::GetInvSqrt<nsMathAcc::BITS_23>() const
{
  const __m128 x0 = _mm_rsqrt_ps(m_v);

  // One iteration of Newton-Raphson
  return _mm_mul_ps(_mm_mul_ps(_mm_set1_ps(0.5f), x0), _mm_sub_ps(_mm_set1_ps(3.0f), _mm_mul_ps(_mm_mul_ps(m_v, x0), x0)));
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::GetInvSqrt<nsMathAcc::BITS_12>() const
{
  return _mm_rsqrt_ps(m_v);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::GetSqrt<nsMathAcc::FULL>() const
{
  return _mm_sqrt_ps(m_v);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::GetSqrt<nsMathAcc::BITS_23>() const
{
  return (*this) * GetInvSqrt<nsMathAcc::BITS_23>();
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::GetSqrt<nsMathAcc::BITS_12>() const
{
  return (*this) * GetInvSqrt<nsMathAcc::BITS_12>();
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::Max(const nsSimdFloat& f) const
{
  return _mm_max_ps(m_v, f.m_v);
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::Min(const nsSimdFloat& f) const
{
  return _mm_min_ps(m_v, f.m_v);
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::Abs() const
{
  return _mm_andnot_ps(_mm_set1_ps(-0.0f), m_v);
}

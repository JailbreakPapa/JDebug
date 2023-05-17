#pragma once

WD_ALWAYS_INLINE wdSimdFloat::wdSimdFloat()
{
  WD_CHECK_SIMD_ALIGNMENT(this);

#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
  // Initialize all data to NaN in debug mode to find problems with uninitialized data easier.
  m_v = _mm_set1_ps(wdMath::NaN<float>());
#endif
}

WD_ALWAYS_INLINE wdSimdFloat::wdSimdFloat(float f)
{
  WD_CHECK_SIMD_ALIGNMENT(this);

  m_v = _mm_set1_ps(f);
}

WD_ALWAYS_INLINE wdSimdFloat::wdSimdFloat(wdInt32 i)
{
  WD_CHECK_SIMD_ALIGNMENT(this);

  __m128 v = _mm_cvtsi32_ss(_mm_setzero_ps(), i);
  m_v = _mm_shuffle_ps(v, v, WD_TO_SHUFFLE(wdSwizzle::XXXX));
}

WD_ALWAYS_INLINE wdSimdFloat::wdSimdFloat(wdUInt32 i)
{
  WD_CHECK_SIMD_ALIGNMENT(this);

#if WD_ENABLED(WD_PLATFORM_64BIT)
  __m128 v = _mm_cvtsi64_ss(_mm_setzero_ps(), i);
#else
  __m128 v = _mm_cvtsi32_ss(_mm_setzero_ps(), i);
#endif
  m_v = _mm_shuffle_ps(v, v, WD_TO_SHUFFLE(wdSwizzle::XXXX));
}

WD_ALWAYS_INLINE wdSimdFloat::wdSimdFloat(wdAngle a)
{
  WD_CHECK_SIMD_ALIGNMENT(this);

  m_v = _mm_set1_ps(a.GetRadian());
}

WD_ALWAYS_INLINE wdSimdFloat::wdSimdFloat(wdInternal::QuadFloat v)
{
  m_v = v;
}

WD_ALWAYS_INLINE wdSimdFloat::operator float() const
{
  float f;
  _mm_store_ss(&f, m_v);
  return f;
}

// static
WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::Zero()
{
  return _mm_setzero_ps();
}

WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::operator+(const wdSimdFloat& f) const
{
  return _mm_add_ps(m_v, f.m_v);
}

WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::operator-(const wdSimdFloat& f) const
{
  return _mm_sub_ps(m_v, f.m_v);
}

WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::operator*(const wdSimdFloat& f) const
{
  return _mm_mul_ps(m_v, f.m_v);
}

WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::operator/(const wdSimdFloat& f) const
{
  return _mm_div_ps(m_v, f.m_v);
}

WD_ALWAYS_INLINE wdSimdFloat& wdSimdFloat::operator+=(const wdSimdFloat& f)
{
  m_v = _mm_add_ps(m_v, f.m_v);
  return *this;
}

WD_ALWAYS_INLINE wdSimdFloat& wdSimdFloat::operator-=(const wdSimdFloat& f)
{
  m_v = _mm_sub_ps(m_v, f.m_v);
  return *this;
}

WD_ALWAYS_INLINE wdSimdFloat& wdSimdFloat::operator*=(const wdSimdFloat& f)
{
  m_v = _mm_mul_ps(m_v, f.m_v);
  return *this;
}

WD_ALWAYS_INLINE wdSimdFloat& wdSimdFloat::operator/=(const wdSimdFloat& f)
{
  m_v = _mm_div_ps(m_v, f.m_v);
  return *this;
}

WD_ALWAYS_INLINE bool wdSimdFloat::IsEqual(const wdSimdFloat& rhs, const wdSimdFloat& fEpsilon) const
{
  wdSimdFloat minusEps = rhs - fEpsilon;
  wdSimdFloat plusEps = rhs + fEpsilon;
  return ((*this >= minusEps) && (*this <= plusEps));
}

WD_ALWAYS_INLINE bool wdSimdFloat::operator==(const wdSimdFloat& f) const
{
  return _mm_comieq_ss(m_v, f.m_v) == 1;
}

WD_ALWAYS_INLINE bool wdSimdFloat::operator!=(const wdSimdFloat& f) const
{
  return _mm_comineq_ss(m_v, f.m_v) == 1;
}

WD_ALWAYS_INLINE bool wdSimdFloat::operator>=(const wdSimdFloat& f) const
{
  return _mm_comige_ss(m_v, f.m_v) == 1;
}

WD_ALWAYS_INLINE bool wdSimdFloat::operator>(const wdSimdFloat& f) const
{
  return _mm_comigt_ss(m_v, f.m_v) == 1;
}

WD_ALWAYS_INLINE bool wdSimdFloat::operator<=(const wdSimdFloat& f) const
{
  return _mm_comile_ss(m_v, f.m_v) == 1;
}

WD_ALWAYS_INLINE bool wdSimdFloat::operator<(const wdSimdFloat& f) const
{
  return _mm_comilt_ss(m_v, f.m_v) == 1;
}

WD_ALWAYS_INLINE bool wdSimdFloat::operator==(float f) const
{
  return (*this) == wdSimdFloat(f);
}

WD_ALWAYS_INLINE bool wdSimdFloat::operator!=(float f) const
{
  return (*this) != wdSimdFloat(f);
}

WD_ALWAYS_INLINE bool wdSimdFloat::operator>(float f) const
{
  return (*this) > wdSimdFloat(f);
}

WD_ALWAYS_INLINE bool wdSimdFloat::operator>=(float f) const
{
  return (*this) >= wdSimdFloat(f);
}

WD_ALWAYS_INLINE bool wdSimdFloat::operator<(float f) const
{
  return (*this) < wdSimdFloat(f);
}

WD_ALWAYS_INLINE bool wdSimdFloat::operator<=(float f) const
{
  return (*this) <= wdSimdFloat(f);
}

template <>
WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::GetReciprocal<wdMathAcc::FULL>() const
{
  return _mm_div_ps(_mm_set1_ps(1.0f), m_v);
}

template <>
WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::GetReciprocal<wdMathAcc::BITS_23>() const
{
  __m128 x0 = _mm_rcp_ps(m_v);

  // One iteration of Newton-Raphson
  __m128 x1 = _mm_mul_ps(x0, _mm_sub_ps(_mm_set1_ps(2.0f), _mm_mul_ps(m_v, x0)));

  return x1;
}

template <>
WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::GetReciprocal<wdMathAcc::BITS_12>() const
{
  return _mm_rcp_ps(m_v);
}

template <>
WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::GetInvSqrt<wdMathAcc::FULL>() const
{
  return _mm_div_ps(_mm_set1_ps(1.0f), _mm_sqrt_ps(m_v));
}

template <>
WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::GetInvSqrt<wdMathAcc::BITS_23>() const
{
  const __m128 x0 = _mm_rsqrt_ps(m_v);

  // One iteration of Newton-Raphson
  return _mm_mul_ps(_mm_mul_ps(_mm_set1_ps(0.5f), x0), _mm_sub_ps(_mm_set1_ps(3.0f), _mm_mul_ps(_mm_mul_ps(m_v, x0), x0)));
}

template <>
WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::GetInvSqrt<wdMathAcc::BITS_12>() const
{
  return _mm_rsqrt_ps(m_v);
}

template <>
WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::GetSqrt<wdMathAcc::FULL>() const
{
  return _mm_sqrt_ps(m_v);
}

template <>
WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::GetSqrt<wdMathAcc::BITS_23>() const
{
  return (*this) * GetInvSqrt<wdMathAcc::BITS_23>();
}

template <>
WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::GetSqrt<wdMathAcc::BITS_12>() const
{
  return (*this) * GetInvSqrt<wdMathAcc::BITS_12>();
}

WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::Max(const wdSimdFloat& f) const
{
  return _mm_max_ps(m_v, f.m_v);
}

WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::Min(const wdSimdFloat& f) const
{
  return _mm_min_ps(m_v, f.m_v);
}

WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::Abs() const
{
  return _mm_andnot_ps(_mm_set1_ps(-0.0f), m_v);
}

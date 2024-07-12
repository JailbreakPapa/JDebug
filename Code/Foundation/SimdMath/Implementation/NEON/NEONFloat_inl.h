#pragma once

NS_ALWAYS_INLINE nsSimdFloat::nsSimdFloat()
{
  NS_CHECK_SIMD_ALIGNMENT(this);

#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
  // Initialize all data to NaN in debug mode to find problems with uninitialized data easier.
  m_v = vmovq_n_f32(nsMath::NaN<float>());
#endif
}

NS_ALWAYS_INLINE nsSimdFloat::nsSimdFloat(float f)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  m_v = vmovq_n_f32(f);
}

NS_ALWAYS_INLINE nsSimdFloat::nsSimdFloat(nsInt32 i)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  m_v = vcvtq_f32_s32(vmovq_n_s32(i));
}

NS_ALWAYS_INLINE nsSimdFloat::nsSimdFloat(nsUInt32 i)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  m_v = vcvtq_f32_u32(vmovq_n_u32(i));
}

NS_ALWAYS_INLINE nsSimdFloat::nsSimdFloat(nsAngle a)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  m_v = vmovq_n_f32(a.GetRadian());
}

NS_ALWAYS_INLINE nsSimdFloat::nsSimdFloat(nsInternal::QuadFloat v)
{
  m_v = v;
}

NS_ALWAYS_INLINE nsSimdFloat::operator float() const
{
  return vgetq_lane_f32(m_v, 0);
}

// static
NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::MakeZero()
{
  return vmovq_n_f32(0.0f);
}

// static
NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::MakeNaN()
{
  return vmovq_n_f32(nsMath::NaN<float>());
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::operator+(const nsSimdFloat& f) const
{
  return vaddq_f32(m_v, f.m_v);
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::operator-(const nsSimdFloat& f) const
{
  return vsubq_f32(m_v, f.m_v);
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::operator*(const nsSimdFloat& f) const
{
  return vmulq_f32(m_v, f.m_v);
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::operator/(const nsSimdFloat& f) const
{
  return vdivq_f32(m_v, f.m_v);
}

NS_ALWAYS_INLINE nsSimdFloat& nsSimdFloat::operator+=(const nsSimdFloat& f)
{
  m_v = vaddq_f32(m_v, f.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdFloat& nsSimdFloat::operator-=(const nsSimdFloat& f)
{
  m_v = vsubq_f32(m_v, f.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdFloat& nsSimdFloat::operator*=(const nsSimdFloat& f)
{
  m_v = vmulq_f32(m_v, f.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdFloat& nsSimdFloat::operator/=(const nsSimdFloat& f)
{
  m_v = vdivq_f32(m_v, f.m_v);
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
  return vgetq_lane_u32(vceqq_f32(m_v, f.m_v), 0) & 1;
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator!=(const nsSimdFloat& f) const
{
  return !operator==(f);
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator>=(const nsSimdFloat& f) const
{
  return vgetq_lane_u32(vcgeq_f32(m_v, f.m_v), 0) & 1;
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator>(const nsSimdFloat& f) const
{
  return vgetq_lane_u32(vcgtq_f32(m_v, f.m_v), 0) & 1;
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator<=(const nsSimdFloat& f) const
{
  return vgetq_lane_u32(vcleq_f32(m_v, f.m_v), 0) & 1;
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator<(const nsSimdFloat& f) const
{
  return vgetq_lane_u32(vcltq_f32(m_v, f.m_v), 0) & 1;
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
  return vdivq_f32(vmovq_n_f32(1.0f), m_v);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::GetReciprocal<nsMathAcc::BITS_23>() const
{
  float32x4_t x0 = vrecpeq_f32(m_v);

  // Two iterations of Newton-Raphson
  float32x4_t x1 = vmulq_f32(vrecpsq_f32(m_v, x0), x0);
  float32x4_t x2 = vmulq_f32(vrecpsq_f32(m_v, x1), x1);

  return x2;
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::GetReciprocal<nsMathAcc::BITS_12>() const
{
  float32x4_t x0 = vrecpeq_f32(m_v);

  // One iteration of Newton-Raphson
  float32x4_t x1 = vmulq_f32(vrecpsq_f32(m_v, x0), x0);

  return x1;
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::GetInvSqrt<nsMathAcc::FULL>() const
{
  return vdivq_f32(vmovq_n_f32(1.0f), vsqrtq_f32(m_v));
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::GetInvSqrt<nsMathAcc::BITS_23>() const
{
  const float32x4_t x0 = vrsqrteq_f32(m_v);

  // Two iterations of Newton-Raphson
  const float32x4_t x1 = vmulq_f32(vrsqrtsq_f32(vmulq_f32(x0, m_v), x0), x0);
  return vmulq_f32(vrsqrtsq_f32(vmulq_f32(x1, m_v), x1), x1);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::GetInvSqrt<nsMathAcc::BITS_12>() const
{
  const float32x4_t x0 = vrsqrteq_f32(m_v);

  // One iteration of Newton-Raphson
  return vmulq_f32(vrsqrtsq_f32(vmulq_f32(x0, m_v), x0), x0);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::GetSqrt<nsMathAcc::FULL>() const
{
  return vsqrtq_f32(m_v);
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
  return vmaxq_f32(m_v, f.m_v);
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::Min(const nsSimdFloat& f) const
{
  return vminq_f32(m_v, f.m_v);
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::Abs() const
{
  return vabsq_f32(m_v);
}

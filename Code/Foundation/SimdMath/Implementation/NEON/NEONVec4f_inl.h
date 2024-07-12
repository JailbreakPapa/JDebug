#pragma once

NS_ALWAYS_INLINE nsSimdVec4f::nsSimdVec4f()
{
  NS_CHECK_SIMD_ALIGNMENT(this);

#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
  // Initialize all data to NaN in debug mode to find problems with uninitialized data easier.
  m_v = vmovq_n_f32(nsMath::NaN<float>());
#endif
}

NS_ALWAYS_INLINE nsSimdVec4f::nsSimdVec4f(float xyzw)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  m_v = vmovq_n_f32(xyzw);
}

NS_ALWAYS_INLINE nsSimdVec4f::nsSimdVec4f(const nsSimdFloat& xyzw)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  m_v = xyzw.m_v;
}

NS_ALWAYS_INLINE nsSimdVec4f::nsSimdVec4f(float x, float y, float z, float w)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  alignas(16) float values[4] = {x, y, z, w};
  m_v = vld1q_f32(values);
}

NS_ALWAYS_INLINE void nsSimdVec4f::Set(float xyzw)
{
  m_v = vmovq_n_f32(xyzw);
}

NS_ALWAYS_INLINE void nsSimdVec4f::Set(float x, float y, float z, float w)
{
  alignas(16) float values[4] = {x, y, z, w};
  m_v = vld1q_f32(values);
}

NS_ALWAYS_INLINE void nsSimdVec4f::SetX(const nsSimdFloat& f)
{
  m_v = vsetq_lane_f32(f, m_v, 0);
}

NS_ALWAYS_INLINE void nsSimdVec4f::SetY(const nsSimdFloat& f)
{
  m_v = vsetq_lane_f32(f, m_v, 1);
}

NS_ALWAYS_INLINE void nsSimdVec4f::SetZ(const nsSimdFloat& f)
{
  m_v = vsetq_lane_f32(f, m_v, 2);
}

NS_ALWAYS_INLINE void nsSimdVec4f::SetW(const nsSimdFloat& f)
{
  m_v = vsetq_lane_f32(f, m_v, 3);
}

NS_ALWAYS_INLINE void nsSimdVec4f::SetZero()
{
  m_v = vmovq_n_f32(0.0f);
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4f::Load<1>(const float* pFloat)
{
  m_v = vld1q_lane_f32(pFloat, vmovq_n_f32(0.0f), 0);
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4f::Load<2>(const float* pFloat)
{
  m_v = vreinterpretq_f32_f64(vld1q_lane_f64(reinterpret_cast<const float64_t*>(pFloat), vmovq_n_f64(0.0), 0));
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4f::Load<3>(const float* pFloat)
{
  m_v = vcombine_f32(vld1_f32(pFloat), vld1_lane_f32(pFloat + 2, vmov_n_f32(0.0f), 0));
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4f::Load<4>(const float* pFloat)
{
  m_v = vld1q_f32(pFloat);
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4f::Store<1>(float* pFloat) const
{
  vst1q_lane_f32(pFloat, m_v, 0);
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4f::Store<2>(float* pFloat) const
{
  vst1q_lane_f64(reinterpret_cast<float64_t*>(pFloat), vreinterpretq_f64_f32(m_v), 0);
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4f::Store<3>(float* pFloat) const
{
  vst1q_lane_f64(reinterpret_cast<float64_t*>(pFloat), vreinterpretq_f64_f32(m_v), 0);
  vst1q_lane_f32(pFloat + 2, m_v, 2);
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4f::Store<4>(float* pFloat) const
{
  vst1q_f32(pFloat, m_v);
}

template <>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetReciprocal<nsMathAcc::BITS_12>() const
{
  float32x4_t x0 = vrecpeq_f32(m_v);

  // One iteration of Newton-Raphson
  float32x4_t x1 = vmulq_f32(vrecpsq_f32(m_v, x0), x0);

  return x1;
}

template <>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetReciprocal<nsMathAcc::BITS_23>() const
{
  float32x4_t x0 = vrecpeq_f32(m_v);

  // Two iterations of Newton-Raphson
  float32x4_t x1 = vmulq_f32(vrecpsq_f32(m_v, x0), x0);
  float32x4_t x2 = vmulq_f32(vrecpsq_f32(m_v, x1), x1);

  return x2;
}

template <>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetReciprocal<nsMathAcc::FULL>() const
{
  return vdivq_f32(vmovq_n_f32(1.0f), m_v);
}

template <>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetInvSqrt<nsMathAcc::FULL>() const
{
  return vdivq_f32(vmovq_n_f32(1.0f), vsqrtq_f32(m_v));
}

template <>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetInvSqrt<nsMathAcc::BITS_23>() const
{
  const float32x4_t x0 = vrsqrteq_f32(m_v);

  // Two iterations of Newton-Raphson
  const float32x4_t x1 = vmulq_f32(vrsqrtsq_f32(vmulq_f32(x0, m_v), x0), x0);
  return vmulq_f32(vrsqrtsq_f32(vmulq_f32(x1, m_v), x1), x1);
}

template <>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetInvSqrt<nsMathAcc::BITS_12>() const
{
  const float32x4_t x0 = vrsqrteq_f32(m_v);

  // One iteration of Newton-Raphson
  return vmulq_f32(vrsqrtsq_f32(vmulq_f32(x0, m_v), x0), x0);
}

template <>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetSqrt<nsMathAcc::BITS_12>() const
{
  return CompMul(GetInvSqrt<nsMathAcc::BITS_12>());
}

template <>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetSqrt<nsMathAcc::BITS_23>() const
{
  return CompMul(GetInvSqrt<nsMathAcc::BITS_23>());
}

template <>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetSqrt<nsMathAcc::FULL>() const
{
  return vsqrtq_f32(m_v);
}

template <int N, nsMathAcc::Enum acc>
void nsSimdVec4f::NormalizeIfNotZero(const nsSimdFloat& fEpsilon)
{
  nsSimdFloat sqLength = GetLengthSquared<N>();
  uint32x4_t isNotZero = vcgtq_f32(sqLength.m_v, fEpsilon.m_v);
  m_v = vmulq_f32(m_v, sqLength.GetInvSqrt<acc>().m_v);
  m_v = vreinterpretq_f32_u32(vandq_u32(isNotZero, vreinterpretq_u32_f32(m_v)));
}

template <int N>
NS_ALWAYS_INLINE bool nsSimdVec4f::IsZero() const
{
  const int mask = NS_BIT(N) - 1;
  return (nsInternal::NeonMoveMask(vceqzq_f32(m_v)) & mask) == mask;
}

template <int N>
NS_ALWAYS_INLINE bool nsSimdVec4f::IsZero(const nsSimdFloat& fEpsilon) const
{
  const int mask = NS_BIT(N) - 1;
  float32x4_t absVal = Abs().m_v;
  return (nsInternal::NeonMoveMask(vcltq_f32(absVal, fEpsilon.m_v)) & mask) == mask;
}

template <int N>
inline bool nsSimdVec4f::IsNaN() const
{
  const int mask = NS_BIT(N) - 1;
  return (nsInternal::NeonMoveMask(vceqq_f32(m_v, m_v)) & mask) != mask;
}

template <int N>
NS_ALWAYS_INLINE bool nsSimdVec4f::IsValid() const
{
  const int mask = NS_BIT(N) - 1;
  return (nsInternal::NeonMoveMask(vcgeq_u32(vreinterpretq_u32_f32(m_v), vmovq_n_u32(0x7f800000))) & mask) == 0;
}

template <int N>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::GetComponent() const
{
  return vdupq_laneq_f32(m_v, N);
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
  return __builtin_shufflevector(m_v, m_v, NS_TO_SHUFFLE(s));
}

template <nsSwizzle::Enum s>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetCombined(const nsSimdVec4f& other) const
{
  return __builtin_shufflevector(m_v, other.m_v, NS_TO_SHUFFLE(s));
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::operator-() const
{
  return vnegq_f32(m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::operator+(const nsSimdVec4f& v) const
{
  return vaddq_f32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::operator-(const nsSimdVec4f& v) const
{
  return vsubq_f32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::operator*(const nsSimdFloat& f) const
{
  return vmulq_f32(m_v, f.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::operator/(const nsSimdFloat& f) const
{
  return vdivq_f32(m_v, f.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::CompMul(const nsSimdVec4f& v) const
{
  return vmulq_f32(m_v, v.m_v);
}

template <>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::CompDiv<nsMathAcc::FULL>(const nsSimdVec4f& v) const
{
  return vdivq_f32(m_v, v.m_v);
}

template <>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::CompDiv<nsMathAcc::BITS_23>(const nsSimdVec4f& v) const
{
  return CompMul(v.GetReciprocal<nsMathAcc::BITS_23>());
}

template <>
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::CompDiv<nsMathAcc::BITS_12>(const nsSimdVec4f& v) const
{
  return CompMul(v.GetReciprocal<nsMathAcc::BITS_12>());
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::CompMin(const nsSimdVec4f& v) const
{
  return vminq_f32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::CompMax(const nsSimdVec4f& v) const
{
  return vmaxq_f32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::Abs() const
{
  return vabsq_f32(m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::Round() const
{
  return vrndnq_f32(m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::Floor() const
{
  return vrndmq_f32(m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::Ceil() const
{
  return vrndpq_f32(m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::Trunc() const
{
  return vrndq_f32(m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::FlipSign(const nsSimdVec4b& cmp) const
{
  return vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(m_v), vshlq_n_u32(cmp.m_v, 31)));
}

// static
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::Select(const nsSimdVec4b& cmp, const nsSimdVec4f& ifTrue, const nsSimdVec4f& ifFalse)
{
  return vbslq_f32(cmp.m_v, ifTrue.m_v, ifFalse.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f& nsSimdVec4f::operator+=(const nsSimdVec4f& v)
{
  m_v = vaddq_f32(m_v, v.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4f& nsSimdVec4f::operator-=(const nsSimdVec4f& v)
{
  m_v = vsubq_f32(m_v, v.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4f& nsSimdVec4f::operator*=(const nsSimdFloat& f)
{
  m_v = vmulq_f32(m_v, f.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4f& nsSimdVec4f::operator/=(const nsSimdFloat& f)
{
  m_v = vdivq_f32(m_v, f.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4f::operator==(const nsSimdVec4f& v) const
{
  return vceqq_f32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4f::operator!=(const nsSimdVec4f& v) const
{
  return vmvnq_u32(vceqq_f32(m_v, v.m_v));
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4f::operator<=(const nsSimdVec4f& v) const
{
  return vcleq_f32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4f::operator<(const nsSimdVec4f& v) const
{
  return vcltq_f32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4f::operator>=(const nsSimdVec4f& v) const
{
  return vcgeq_f32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4f::operator>(const nsSimdVec4f& v) const
{
  return vcgtq_f32(m_v, v.m_v);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalSum<2>() const
{
  return vpadds_f32(vget_low_f32(m_v));
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalSum<3>() const
{
  return HorizontalSum<2>() + GetComponent<2>();
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalSum<4>() const
{
  float32x2_t x0 = vpadd_f32(vget_low_f32(m_v), vget_high_f32(m_v));
  return vpadds_f32(x0);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalMin<2>() const
{
  return vpmins_f32(vget_low_f32(m_v));
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalMin<3>() const
{
  return vminq_f32(vmovq_n_f32(vpmins_f32(vget_low_f32(m_v))), vdupq_laneq_f32(m_v, 2));
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalMin<4>() const
{
  return vpmins_f32(vpmin_f32(vget_low_f32(m_v), vget_high_f32(m_v)));
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalMax<2>() const
{
  return vpmaxs_f32(vget_low_f32(m_v));
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalMax<3>() const
{
  return vmaxq_f32(vmovq_n_f32(vpmaxs_f32(vget_low_f32(m_v))), vdupq_laneq_f32(m_v, 2));
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::HorizontalMax<4>() const
{
  return vpmaxs_f32(vpmax_f32(vget_low_f32(m_v), vget_high_f32(m_v)));
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::Dot<1>(const nsSimdVec4f& v) const
{
  return vdupq_laneq_f32(vmulq_f32(m_v, v.m_v), 0);
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::Dot<2>(const nsSimdVec4f& v) const
{
  return vpadds_f32(vmul_f32(vget_low_f32(m_v), vget_low_f32(v.m_v)));
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::Dot<3>(const nsSimdVec4f& v) const
{
  return CompMul(v).HorizontalSum<3>();
}

template <>
NS_ALWAYS_INLINE nsSimdFloat nsSimdVec4f::Dot<4>(const nsSimdVec4f& v) const
{
  return CompMul(v).HorizontalSum<4>();
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::CrossRH(const nsSimdVec4f& v) const
{
  float32x4_t a = vmulq_f32(m_v, __builtin_shufflevector(v.m_v, v.m_v, NS_TO_SHUFFLE(nsSwizzle::YZXW)));
  float32x4_t b = vmulq_f32(v.m_v, __builtin_shufflevector(m_v, m_v, NS_TO_SHUFFLE(nsSwizzle::YZXW)));
  float32x4_t c = vsubq_f32(a, b);

  return __builtin_shufflevector(c, c, NS_TO_SHUFFLE(nsSwizzle::YZXW));
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::GetOrthogonalVector() const
{
  // See http://blog.selfshadow.com/2011/10/17/perp-vectors/ - this is Stark's first variant, SIMDified.
  return CrossRH(vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(m_v), vceqq_f32(m_v, HorizontalMin<3>().m_v))));
}

// static
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::MulAdd(const nsSimdVec4f& a, const nsSimdVec4f& b, const nsSimdVec4f& c)
{
  return vfmaq_f32(c.m_v, a.m_v, b.m_v);
}

// static
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::MulAdd(const nsSimdVec4f& a, const nsSimdFloat& b, const nsSimdVec4f& c)
{
  return vfmaq_f32(c.m_v, a.m_v, b.m_v);
}

// static
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::MulSub(const nsSimdVec4f& a, const nsSimdVec4f& b, const nsSimdVec4f& c)
{
  return vnegq_f32(vfmsq_f32(c.m_v, a.m_v, b.m_v));
}

// static
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::MulSub(const nsSimdVec4f& a, const nsSimdFloat& b, const nsSimdVec4f& c)
{
  return vnegq_f32(vfmsq_f32(c.m_v, a.m_v, b.m_v));
}

// static
NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4f::CopySign(const nsSimdVec4f& magnitude, const nsSimdVec4f& sign)
{
  return vbslq_f32(vmovq_n_u32(0x80000000), sign.m_v, magnitude.m_v);
}

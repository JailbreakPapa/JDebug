#pragma once

NS_ALWAYS_INLINE nsSimdVec4i::nsSimdVec4i()
{
  NS_CHECK_SIMD_ALIGNMENT(this);

#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
  m_v = vmovq_n_u32(0xCDCDCDCD);
#endif
}

NS_ALWAYS_INLINE nsSimdVec4i::nsSimdVec4i(nsInt32 xyzw)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  m_v = vmovq_n_s32(xyzw);
}

NS_ALWAYS_INLINE nsSimdVec4i::nsSimdVec4i(nsInt32 x, nsInt32 y, nsInt32 z, nsInt32 w)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  alignas(16) nsInt32 values[4] = {x, y, z, w};
  m_v = vld1q_s32(values);
}

NS_ALWAYS_INLINE nsSimdVec4i::nsSimdVec4i(nsInternal::QuadInt v)
{
  m_v = v;
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::MakeZero()
{
  return vmovq_n_s32(0);
}

NS_ALWAYS_INLINE void nsSimdVec4i::Set(nsInt32 xyzw)
{
  m_v = vmovq_n_s32(xyzw);
}

NS_ALWAYS_INLINE void nsSimdVec4i::Set(nsInt32 x, nsInt32 y, nsInt32 z, nsInt32 w)
{
  alignas(16) nsInt32 values[4] = {x, y, z, w};
  m_v = vld1q_s32(values);
}

NS_ALWAYS_INLINE void nsSimdVec4i::SetZero()
{
  m_v = vmovq_n_s32(0);
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4i::Load<1>(const nsInt32* pInts)
{
  m_v = vld1q_lane_s32(pInts, vmovq_n_s32(0), 0);
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4i::Load<2>(const nsInt32* pInts)
{
  m_v = vreinterpretq_s32_s64(vld1q_lane_s64(reinterpret_cast<const int64_t*>(pInts), vmovq_n_s64(0), 0));
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4i::Load<3>(const nsInt32* pInts)
{
  m_v = vcombine_s32(vld1_s32(pInts), vld1_lane_s32(pInts + 2, vmov_n_s32(0), 0));
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4i::Load<4>(const nsInt32* pInts)
{
  m_v = vld1q_s32(pInts);
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4i::Store<1>(nsInt32* pInts) const
{
  vst1q_lane_s32(pInts, m_v, 0);
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4i::Store<2>(nsInt32* pInts) const
{
  vst1q_lane_s64(reinterpret_cast<int64_t*>(pInts), vreinterpretq_s64_s32(m_v), 0);
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4i::Store<3>(nsInt32* pInts) const
{
  vst1q_lane_s64(reinterpret_cast<int64_t*>(pInts), vreinterpretq_s64_s32(m_v), 0);
  vst1q_lane_s32(pInts + 2, m_v, 2);
}

template <>
NS_ALWAYS_INLINE void nsSimdVec4i::Store<4>(nsInt32* pInts) const
{
  vst1q_s32(pInts, m_v);
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4i::ToFloat() const
{
  return vcvtq_f32_s32(m_v);
}

// static
NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::Truncate(const nsSimdVec4f& f)
{
  return vcvtq_s32_f32(f.m_v);
}

template <int N>
NS_ALWAYS_INLINE nsInt32 nsSimdVec4i::GetComponent() const
{
  return vgetq_lane_s32(m_v, N);
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
  return __builtin_shufflevector(m_v, m_v, NS_TO_SHUFFLE(s));
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator-() const
{
  return vnegq_s32(m_v);
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator+(const nsSimdVec4i& v) const
{
  return vaddq_s32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator-(const nsSimdVec4i& v) const
{
  return vsubq_s32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::CompMul(const nsSimdVec4i& v) const
{
  return vmulq_s32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::CompDiv(const nsSimdVec4i& v) const
{
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
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator|(const nsSimdVec4i& v) const
{
  return vorrq_s32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator&(const nsSimdVec4i& v) const
{
  return vandq_s32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator^(const nsSimdVec4i& v) const
{
  return veorq_s32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator~() const
{
  return vmvnq_s32(m_v);
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator<<(nsUInt32 uiShift) const
{
  return vshlq_s32(m_v, vmovq_n_s32(uiShift));
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator>>(nsUInt32 uiShift) const
{
  return vshlq_s32(m_v, vmovq_n_s32(-uiShift));
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator<<(const nsSimdVec4i& v) const
{
  return vshlq_s32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator>>(const nsSimdVec4i& v) const
{
  return vshlq_s32(m_v, vnegq_s32(v.m_v));
}

NS_ALWAYS_INLINE nsSimdVec4i& nsSimdVec4i::operator+=(const nsSimdVec4i& v)
{
  m_v = vaddq_s32(m_v, v.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4i& nsSimdVec4i::operator-=(const nsSimdVec4i& v)
{
  m_v = vsubq_s32(m_v, v.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4i& nsSimdVec4i::operator|=(const nsSimdVec4i& v)
{
  m_v = vorrq_s32(m_v, v.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4i& nsSimdVec4i::operator&=(const nsSimdVec4i& v)
{
  m_v = vandq_s32(m_v, v.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4i& nsSimdVec4i::operator^=(const nsSimdVec4i& v)
{
  m_v = veorq_s32(m_v, v.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4i& nsSimdVec4i::operator<<=(nsUInt32 uiShift)
{
  m_v = vshlq_s32(m_v, vmovq_n_s32(uiShift));
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4i& nsSimdVec4i::operator>>=(nsUInt32 uiShift)
{
  m_v = vshlq_s32(m_v, vmovq_n_s32(-uiShift));
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::CompMin(const nsSimdVec4i& v) const
{
  return vminq_s32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::CompMax(const nsSimdVec4i& v) const
{
  return vmaxq_s32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::Abs() const
{
  return vabsq_s32(m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4i::operator==(const nsSimdVec4i& v) const
{
  return vceqq_s32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4i::operator!=(const nsSimdVec4i& v) const
{
  return vmvnq_u32(vceqq_s32(m_v, v.m_v));
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4i::operator<=(const nsSimdVec4i& v) const
{
  return vcleq_s32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4i::operator<(const nsSimdVec4i& v) const
{
  return vcltq_s32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4i::operator>=(const nsSimdVec4i& v) const
{
  return vcgeq_s32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4i::operator>(const nsSimdVec4i& v) const
{
  return vcgtq_s32(m_v, v.m_v);
}

// static
NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::Select(const nsSimdVec4b& vCmp, const nsSimdVec4i& vTrue, const nsSimdVec4i& vFalse)
{
  return vbslq_s32(vCmp.m_v, vTrue.m_v, vFalse.m_v);
}

// not needed atm
#if 0
void nsSimdVec4i::Transpose(nsSimdVec4i& v0, nsSimdVec4i& v1, nsSimdVec4i& v2, nsSimdVec4i& v3)
{
  int32x4x2_t P0 = vzipq_s32(v0.m_v, v2.m_v);
  int32x4x2_t P1 = vzipq_s32(v1.m_v, v3.m_v);

  int32x4x2_t T0 = vzipq_s32(P0.val[0], P1.val[0]);
  int32x4x2_t T1 = vzipq_s32(P0.val[1], P1.val[1]);

  v0.m_v = T0.val[0];
  v1.m_v = T0.val[1];
  v2.m_v = T1.val[0];
  v3.m_v = T1.val[1];
}
#endif

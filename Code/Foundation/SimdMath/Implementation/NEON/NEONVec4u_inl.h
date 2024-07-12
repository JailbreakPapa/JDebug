#pragma once

NS_ALWAYS_INLINE nsSimdVec4u::nsSimdVec4u()
{
  NS_CHECK_SIMD_ALIGNMENT(this);

#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
  m_v = vmovq_n_u32(0xCDCDCDCD);
#endif
}

NS_ALWAYS_INLINE nsSimdVec4u::nsSimdVec4u(nsUInt32 xyzw)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  m_v = vmovq_n_u32(xyzw);
}

NS_ALWAYS_INLINE nsSimdVec4u::nsSimdVec4u(nsUInt32 x, nsUInt32 y, nsUInt32 z, nsUInt32 w)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  alignas(16) nsUInt32 values[4] = {x, y, z, w};
  m_v = vld1q_u32(values);
}

NS_ALWAYS_INLINE nsSimdVec4u::nsSimdVec4u(nsInternal::QuadUInt v)
{
  m_v = v;
}

NS_ALWAYS_INLINE void nsSimdVec4u::Set(nsUInt32 xyzw)
{
  m_v = vmovq_n_u32(xyzw);
}

NS_ALWAYS_INLINE void nsSimdVec4u::Set(nsUInt32 x, nsUInt32 y, nsUInt32 z, nsUInt32 w)
{
  alignas(16) nsUInt32 values[4] = {x, y, z, w};
  m_v = vld1q_u32(values);
}

NS_ALWAYS_INLINE void nsSimdVec4u::SetZero()
{
  m_v = vmovq_n_u32(0);
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
  return vcvtq_f32_u32(m_v);
}

// static
NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::Truncate(const nsSimdVec4f& f)
{
  return vcvtq_u32_f32(f.m_v);
}

template <int N>
NS_ALWAYS_INLINE nsUInt32 nsSimdVec4u::GetComponent() const
{
  return vgetq_lane_u32(m_v, N);
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
  return __builtin_shufflevector(m_v, m_v, NS_TO_SHUFFLE(s));
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::operator+(const nsSimdVec4u& v) const
{
  return vaddq_u32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::operator-(const nsSimdVec4u& v) const
{
  return vsubq_u32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::CompMul(const nsSimdVec4u& v) const
{
  return vmulq_u32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::operator|(const nsSimdVec4u& v) const
{
  return vorrq_u32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::operator&(const nsSimdVec4u& v) const
{
  return vandq_u32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::operator^(const nsSimdVec4u& v) const
{
  return veorq_u32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::operator~() const
{
  return vmvnq_u32(m_v);
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::operator<<(nsUInt32 uiShift) const
{
  return vshlq_u32(m_v, vmovq_n_u32(uiShift));
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::operator>>(nsUInt32 uiShift) const
{
  return vshlq_u32(m_v, vmovq_n_u32(-uiShift));
}

NS_ALWAYS_INLINE nsSimdVec4u& nsSimdVec4u::operator+=(const nsSimdVec4u& v)
{
  m_v = vaddq_u32(m_v, v.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4u& nsSimdVec4u::operator-=(const nsSimdVec4u& v)
{
  m_v = vsubq_u32(m_v, v.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4u& nsSimdVec4u::operator|=(const nsSimdVec4u& v)
{
  m_v = vorrq_u32(m_v, v.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4u& nsSimdVec4u::operator&=(const nsSimdVec4u& v)
{
  m_v = vandq_u32(m_v, v.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4u& nsSimdVec4u::operator^=(const nsSimdVec4u& v)
{
  m_v = veorq_u32(m_v, v.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4u& nsSimdVec4u::operator<<=(nsUInt32 uiShift)
{
  m_v = vshlq_u32(m_v, vmovq_n_u32(uiShift));
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4u& nsSimdVec4u::operator>>=(nsUInt32 uiShift)
{
  m_v = vshlq_u32(m_v, vmovq_n_u32(-uiShift));
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::CompMin(const nsSimdVec4u& v) const
{
  return vminq_u32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::CompMax(const nsSimdVec4u& v) const
{
  return vmaxq_u32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4u::operator==(const nsSimdVec4u& v) const
{
  return vceqq_u32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4u::operator!=(const nsSimdVec4u& v) const
{
  return vmvnq_u32(vceqq_u32(m_v, v.m_v));
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4u::operator<=(const nsSimdVec4u& v) const
{
  return vcleq_u32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4u::operator<(const nsSimdVec4u& v) const
{
  return vcltq_u32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4u::operator>=(const nsSimdVec4u& v) const
{
  return vcgeq_u32(m_v, v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4u::operator>(const nsSimdVec4u& v) const
{
  return vcgtq_u32(m_v, v.m_v);
}

// static
NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::MakeZero()
{
  return vmovq_n_u32(0);
}

// not needed atm
#if 0
void nsSimdVec4u::Transpose(nsSimdVec4u& v0, nsSimdVec4u& v1, nsSimdVec4u& v2, nsSimdVec4u& v3)
{
  uint32x4x2_t P0 = vzipq_u32(v0.m_v, v2.m_v);
  uint32x4x2_t P1 = vzipq_u32(v1.m_v, v3.m_v);

  uint32x4x2_t T0 = vzipq_u32(P0.val[0], P1.val[0]);
  uint32x4x2_t T1 = vzipq_u32(P0.val[1], P1.val[1]);

  v0.m_v = T0.val[0];
  v1.m_v = T0.val[1];
  v2.m_v = T1.val[0];
  v3.m_v = T1.val[1];
}
#endif

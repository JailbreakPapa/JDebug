#pragma once

NS_ALWAYS_INLINE nsSimdVec4b::nsSimdVec4b()
{
  NS_CHECK_SIMD_ALIGNMENT(this);
}

NS_ALWAYS_INLINE nsSimdVec4b::nsSimdVec4b(bool b)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  m_v = vmovq_n_u32(b ? 0xFFFFFFFF : 0);
}

NS_ALWAYS_INLINE nsSimdVec4b::nsSimdVec4b(bool x, bool y, bool z, bool w)
{
  NS_CHECK_SIMD_ALIGNMENT(this);

  alignas(16) nsUInt32 mask[4] = {x ? 0xFFFFFFFF : 0, y ? 0xFFFFFFFF : 0, z ? 0xFFFFFFFF : 0, w ? 0xFFFFFFFF : 0};
  m_v = vld1q_u32(mask);
}

NS_ALWAYS_INLINE nsSimdVec4b::nsSimdVec4b(nsInternal::QuadBool v)
{
  m_v = v;
}

template <int N>
NS_ALWAYS_INLINE bool nsSimdVec4b::GetComponent() const
{
  return vgetq_lane_u32(m_v, N) & 1;
}

NS_ALWAYS_INLINE bool nsSimdVec4b::x() const
{
  return GetComponent<0>();
}

NS_ALWAYS_INLINE bool nsSimdVec4b::y() const
{
  return GetComponent<1>();
}

NS_ALWAYS_INLINE bool nsSimdVec4b::z() const
{
  return GetComponent<2>();
}

NS_ALWAYS_INLINE bool nsSimdVec4b::w() const
{
  return GetComponent<3>();
}

template <nsSwizzle::Enum s>
NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4b::Get() const
{
  return __builtin_shufflevector(m_v, m_v, NS_TO_SHUFFLE(s));
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4b::operator&&(const nsSimdVec4b& rhs) const
{
  return vandq_u32(m_v, rhs.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4b::operator||(const nsSimdVec4b& rhs) const
{
  return vorrq_u32(m_v, rhs.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4b::operator!() const
{
  return vmvnq_u32(m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4b::operator==(const nsSimdVec4b& rhs) const
{
  return vceqq_u32(m_v, rhs.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4b::operator!=(const nsSimdVec4b& rhs) const
{
  return veorq_u32(m_v, rhs.m_v);
}

template <int N>
NS_ALWAYS_INLINE bool nsSimdVec4b::AllSet() const
{
  const int mask = NS_BIT(N) - 1;
  return (nsInternal::NeonMoveMask(m_v) & mask) == mask;
}

template <int N>
NS_ALWAYS_INLINE bool nsSimdVec4b::AnySet() const
{
  const int mask = NS_BIT(N) - 1;
  return (nsInternal::NeonMoveMask(m_v) & mask) != 0;
}

template <int N>
NS_ALWAYS_INLINE bool nsSimdVec4b::NoneSet() const
{
  const int mask = NS_BIT(N) - 1;
  return (nsInternal::NeonMoveMask(m_v) & mask) == 0;
}

// static
NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4b::Select(const nsSimdVec4b& vCmp, const nsSimdVec4b& vTrue, const nsSimdVec4b& vFalse)
{
  return vbslq_u32(vCmp.m_v, vTrue.m_v, vFalse.m_v);
}

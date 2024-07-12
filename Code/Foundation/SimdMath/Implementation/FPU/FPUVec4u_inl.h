#pragma once

NS_ALWAYS_INLINE nsSimdVec4u::nsSimdVec4u()
{
#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
  m_v.Set(0xCDCDCDCD);
#endif
}

NS_ALWAYS_INLINE nsSimdVec4u::nsSimdVec4u(nsUInt32 xyzw)
{
  m_v.Set(xyzw);
}

NS_ALWAYS_INLINE nsSimdVec4u::nsSimdVec4u(nsUInt32 x, nsUInt32 y, nsUInt32 z, nsUInt32 w)
{
  m_v.Set(x, y, z, w);
}

NS_ALWAYS_INLINE nsSimdVec4u::nsSimdVec4u(nsInternal::QuadUInt v)
{
  m_v = v;
}

NS_ALWAYS_INLINE void nsSimdVec4u::Set(nsUInt32 xyzw)
{
  m_v.Set(xyzw);
}

NS_ALWAYS_INLINE void nsSimdVec4u::Set(nsUInt32 x, nsUInt32 y, nsUInt32 z, nsUInt32 w)
{
  m_v.Set(x, y, z, w);
}

NS_ALWAYS_INLINE void nsSimdVec4u::SetZero()
{
  m_v.SetZero();
}

// needs to be implemented here because of include dependencies
NS_ALWAYS_INLINE nsSimdVec4i::nsSimdVec4i(const nsSimdVec4u& u)
  : m_v(u.m_v.x, u.m_v.y, u.m_v.z, u.m_v.w)
{
}

NS_ALWAYS_INLINE nsSimdVec4u::nsSimdVec4u(const nsSimdVec4i& i)
  : m_v(i.m_v.x, i.m_v.y, i.m_v.z, i.m_v.w)
{
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4u::ToFloat() const
{
  nsSimdVec4f result;
  result.m_v.x = (float)m_v.x;
  result.m_v.y = (float)m_v.y;
  result.m_v.z = (float)m_v.z;
  result.m_v.w = (float)m_v.w;

  return result;
}

// static
NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::Truncate(const nsSimdVec4f& f)
{
  nsSimdVec4f clampedF = f.CompMax(nsSimdVec4f::MakeZero());

  nsSimdVec4u result;
  result.m_v.x = (nsUInt32)clampedF.m_v.x;
  result.m_v.y = (nsUInt32)clampedF.m_v.y;
  result.m_v.z = (nsUInt32)clampedF.m_v.z;
  result.m_v.w = (nsUInt32)clampedF.m_v.w;

  return result;
}

template <int N>
NS_ALWAYS_INLINE nsUInt32 nsSimdVec4u::GetComponent() const
{
  return (&m_v.x)[N];
}

NS_ALWAYS_INLINE nsUInt32 nsSimdVec4u::x() const
{
  return m_v.x;
}

NS_ALWAYS_INLINE nsUInt32 nsSimdVec4u::y() const
{
  return m_v.y;
}

NS_ALWAYS_INLINE nsUInt32 nsSimdVec4u::z() const
{
  return m_v.z;
}

NS_ALWAYS_INLINE nsUInt32 nsSimdVec4u::w() const
{
  return m_v.w;
}

template <nsSwizzle::Enum s>
NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::Get() const
{
  nsSimdVec4u result;

  const nsUInt32* v = &m_v.x;
  result.m_v.x = v[(s & 0x3000) >> 12];
  result.m_v.y = v[(s & 0x0300) >> 8];
  result.m_v.z = v[(s & 0x0030) >> 4];
  result.m_v.w = v[(s & 0x0003)];

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::operator+(const nsSimdVec4u& v) const
{
  return m_v + v.m_v;
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::operator-(const nsSimdVec4u& v) const
{
  return m_v - v.m_v;
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::CompMul(const nsSimdVec4u& v) const
{
  return m_v.CompMul(v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::operator|(const nsSimdVec4u& v) const
{
  nsSimdVec4u result;
  result.m_v.x = m_v.x | v.m_v.x;
  result.m_v.y = m_v.y | v.m_v.y;
  result.m_v.z = m_v.z | v.m_v.z;
  result.m_v.w = m_v.w | v.m_v.w;

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::operator&(const nsSimdVec4u& v) const
{
  nsSimdVec4u result;
  result.m_v.x = m_v.x & v.m_v.x;
  result.m_v.y = m_v.y & v.m_v.y;
  result.m_v.z = m_v.z & v.m_v.z;
  result.m_v.w = m_v.w & v.m_v.w;

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::operator^(const nsSimdVec4u& v) const
{
  nsSimdVec4u result;
  result.m_v.x = m_v.x ^ v.m_v.x;
  result.m_v.y = m_v.y ^ v.m_v.y;
  result.m_v.z = m_v.z ^ v.m_v.z;
  result.m_v.w = m_v.w ^ v.m_v.w;

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::operator~() const
{
  nsSimdVec4u result;
  result.m_v.x = ~m_v.x;
  result.m_v.y = ~m_v.y;
  result.m_v.z = ~m_v.z;
  result.m_v.w = ~m_v.w;

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::operator<<(nsUInt32 uiShift) const
{
  nsSimdVec4u result;
  result.m_v.x = m_v.x << uiShift;
  result.m_v.y = m_v.y << uiShift;
  result.m_v.z = m_v.z << uiShift;
  result.m_v.w = m_v.w << uiShift;

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::operator>>(nsUInt32 uiShift) const
{
  nsSimdVec4u result;
  result.m_v.x = m_v.x >> uiShift;
  result.m_v.y = m_v.y >> uiShift;
  result.m_v.z = m_v.z >> uiShift;
  result.m_v.w = m_v.w >> uiShift;

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4u& nsSimdVec4u::operator+=(const nsSimdVec4u& v)
{
  m_v += v.m_v;
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4u& nsSimdVec4u::operator-=(const nsSimdVec4u& v)
{
  m_v -= v.m_v;
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4u& nsSimdVec4u::operator|=(const nsSimdVec4u& v)
{
  m_v.x |= v.m_v.x;
  m_v.y |= v.m_v.y;
  m_v.z |= v.m_v.z;
  m_v.w |= v.m_v.w;
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4u& nsSimdVec4u::operator&=(const nsSimdVec4u& v)
{
  m_v.x &= v.m_v.x;
  m_v.y &= v.m_v.y;
  m_v.z &= v.m_v.z;
  m_v.w &= v.m_v.w;
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4u& nsSimdVec4u::operator^=(const nsSimdVec4u& v)
{
  m_v.x ^= v.m_v.x;
  m_v.y ^= v.m_v.y;
  m_v.z ^= v.m_v.z;
  m_v.w ^= v.m_v.w;
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4u& nsSimdVec4u::operator<<=(nsUInt32 uiShift)
{
  m_v.x <<= uiShift;
  m_v.y <<= uiShift;
  m_v.z <<= uiShift;
  m_v.w <<= uiShift;
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4u& nsSimdVec4u::operator>>=(nsUInt32 uiShift)
{
  m_v.x >>= uiShift;
  m_v.y >>= uiShift;
  m_v.z >>= uiShift;
  m_v.w >>= uiShift;
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::CompMin(const nsSimdVec4u& v) const
{
  return m_v.CompMin(v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::CompMax(const nsSimdVec4u& v) const
{
  return m_v.CompMax(v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4u::operator==(const nsSimdVec4u& v) const
{
  bool result[4];
  result[0] = m_v.x == v.m_v.x;
  result[1] = m_v.y == v.m_v.y;
  result[2] = m_v.z == v.m_v.z;
  result[3] = m_v.w == v.m_v.w;

  return nsSimdVec4b(result[0], result[1], result[2], result[3]);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4u::operator!=(const nsSimdVec4u& v) const
{
  return !(*this == v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4u::operator<=(const nsSimdVec4u& v) const
{
  return !(*this > v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4u::operator<(const nsSimdVec4u& v) const
{
  bool result[4];
  result[0] = m_v.x < v.m_v.x;
  result[1] = m_v.y < v.m_v.y;
  result[2] = m_v.z < v.m_v.z;
  result[3] = m_v.w < v.m_v.w;

  return nsSimdVec4b(result[0], result[1], result[2], result[3]);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4u::operator>=(const nsSimdVec4u& v) const
{
  return !(*this < v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4u::operator>(const nsSimdVec4u& v) const
{
  bool result[4];
  result[0] = m_v.x > v.m_v.x;
  result[1] = m_v.y > v.m_v.y;
  result[2] = m_v.z > v.m_v.z;
  result[3] = m_v.w > v.m_v.w;

  return nsSimdVec4b(result[0], result[1], result[2], result[3]);
}

// static
NS_ALWAYS_INLINE nsSimdVec4u nsSimdVec4u::MakeZero()
{
  return nsVec4U32::MakeZero();
}

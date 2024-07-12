#pragma once

NS_ALWAYS_INLINE nsSimdVec4i::nsSimdVec4i()
{
#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
  m_v.Set(0xCDCDCDCD);
#endif
}

NS_ALWAYS_INLINE nsSimdVec4i::nsSimdVec4i(nsInt32 xyzw)
{
  m_v.Set(xyzw);
}

NS_ALWAYS_INLINE nsSimdVec4i::nsSimdVec4i(nsInt32 x, nsInt32 y, nsInt32 z, nsInt32 w)
{
  m_v.Set(x, y, z, w);
}

NS_ALWAYS_INLINE nsSimdVec4i::nsSimdVec4i(nsInternal::QuadInt v)
{
  m_v = v;
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::MakeZero()
{
  return nsSimdVec4i(0);
}

NS_ALWAYS_INLINE void nsSimdVec4i::Set(nsInt32 xyzw)
{
  m_v.Set(xyzw);
}

NS_ALWAYS_INLINE void nsSimdVec4i::Set(nsInt32 x, nsInt32 y, nsInt32 z, nsInt32 w)
{
  m_v.Set(x, y, z, w);
}

NS_ALWAYS_INLINE void nsSimdVec4i::SetZero()
{
  m_v.SetZero();
}

template <int N>
NS_ALWAYS_INLINE void nsSimdVec4i::Load(const nsInt32* pInts)
{
  m_v.SetZero();
  for (int i = 0; i < N; ++i)
  {
    (&m_v.x)[i] = pInts[i];
  }
}

template <int N>
NS_ALWAYS_INLINE void nsSimdVec4i::Store(nsInt32* pInts) const
{
  for (int i = 0; i < N; ++i)
  {
    pInts[i] = (&m_v.x)[i];
  }
}

NS_ALWAYS_INLINE nsSimdVec4f nsSimdVec4i::ToFloat() const
{
  nsSimdVec4f result;
  result.m_v.x = (float)m_v.x;
  result.m_v.y = (float)m_v.y;
  result.m_v.z = (float)m_v.z;
  result.m_v.w = (float)m_v.w;

  return result;
}

// static
NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::Truncate(const nsSimdVec4f& f)
{
  nsSimdVec4i result;
  result.m_v.x = (nsInt32)f.m_v.x;
  result.m_v.y = (nsInt32)f.m_v.y;
  result.m_v.z = (nsInt32)f.m_v.z;
  result.m_v.w = (nsInt32)f.m_v.w;

  return result;
}

template <int N>
NS_ALWAYS_INLINE nsInt32 nsSimdVec4i::GetComponent() const
{
  return (&m_v.x)[N];
}

NS_ALWAYS_INLINE nsInt32 nsSimdVec4i::x() const
{
  return m_v.x;
}

NS_ALWAYS_INLINE nsInt32 nsSimdVec4i::y() const
{
  return m_v.y;
}

NS_ALWAYS_INLINE nsInt32 nsSimdVec4i::z() const
{
  return m_v.z;
}

NS_ALWAYS_INLINE nsInt32 nsSimdVec4i::w() const
{
  return m_v.w;
}

template <nsSwizzle::Enum s>
NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::Get() const
{
  nsSimdVec4i result;

  const nsInt32* v = &m_v.x;
  result.m_v.x = v[(s & 0x3000) >> 12];
  result.m_v.y = v[(s & 0x0300) >> 8];
  result.m_v.z = v[(s & 0x0030) >> 4];
  result.m_v.w = v[(s & 0x0003)];

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator-() const
{
  return -m_v;
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator+(const nsSimdVec4i& v) const
{
  return m_v + v.m_v;
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator-(const nsSimdVec4i& v) const
{
  return m_v - v.m_v;
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::CompMul(const nsSimdVec4i& v) const
{
  return m_v.CompMul(v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::CompDiv(const nsSimdVec4i& v) const
{
  return m_v.CompDiv(v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator|(const nsSimdVec4i& v) const
{
  nsSimdVec4i result;
  result.m_v.x = m_v.x | v.m_v.x;
  result.m_v.y = m_v.y | v.m_v.y;
  result.m_v.z = m_v.z | v.m_v.z;
  result.m_v.w = m_v.w | v.m_v.w;

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator&(const nsSimdVec4i& v) const
{
  nsSimdVec4i result;
  result.m_v.x = m_v.x & v.m_v.x;
  result.m_v.y = m_v.y & v.m_v.y;
  result.m_v.z = m_v.z & v.m_v.z;
  result.m_v.w = m_v.w & v.m_v.w;

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator^(const nsSimdVec4i& v) const
{
  nsSimdVec4i result;
  result.m_v.x = m_v.x ^ v.m_v.x;
  result.m_v.y = m_v.y ^ v.m_v.y;
  result.m_v.z = m_v.z ^ v.m_v.z;
  result.m_v.w = m_v.w ^ v.m_v.w;

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator~() const
{
  nsSimdVec4i result;
  result.m_v.x = ~m_v.x;
  result.m_v.y = ~m_v.y;
  result.m_v.z = ~m_v.z;
  result.m_v.w = ~m_v.w;

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator<<(nsUInt32 uiShift) const
{
  nsSimdVec4i result;
  result.m_v.x = m_v.x << uiShift;
  result.m_v.y = m_v.y << uiShift;
  result.m_v.z = m_v.z << uiShift;
  result.m_v.w = m_v.w << uiShift;

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator>>(nsUInt32 uiShift) const
{
  nsSimdVec4i result;
  result.m_v.x = m_v.x >> uiShift;
  result.m_v.y = m_v.y >> uiShift;
  result.m_v.z = m_v.z >> uiShift;
  result.m_v.w = m_v.w >> uiShift;

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator<<(const nsSimdVec4i& v) const
{
  nsSimdVec4i result;
  result.m_v.x = m_v.x << v.m_v.x;
  result.m_v.y = m_v.y << v.m_v.y;
  result.m_v.z = m_v.z << v.m_v.z;
  result.m_v.w = m_v.w << v.m_v.w;

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::operator>>(const nsSimdVec4i& v) const
{
  nsSimdVec4i result;
  result.m_v.x = m_v.x >> v.m_v.x;
  result.m_v.y = m_v.y >> v.m_v.y;
  result.m_v.z = m_v.z >> v.m_v.z;
  result.m_v.w = m_v.w >> v.m_v.w;

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4i& nsSimdVec4i::operator+=(const nsSimdVec4i& v)
{
  m_v += v.m_v;
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4i& nsSimdVec4i::operator-=(const nsSimdVec4i& v)
{
  m_v -= v.m_v;
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4i& nsSimdVec4i::operator|=(const nsSimdVec4i& v)
{
  m_v.x |= v.m_v.x;
  m_v.y |= v.m_v.y;
  m_v.z |= v.m_v.z;
  m_v.w |= v.m_v.w;
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4i& nsSimdVec4i::operator&=(const nsSimdVec4i& v)
{
  m_v.x &= v.m_v.x;
  m_v.y &= v.m_v.y;
  m_v.z &= v.m_v.z;
  m_v.w &= v.m_v.w;
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4i& nsSimdVec4i::operator^=(const nsSimdVec4i& v)
{
  m_v.x ^= v.m_v.x;
  m_v.y ^= v.m_v.y;
  m_v.z ^= v.m_v.z;
  m_v.w ^= v.m_v.w;
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4i& nsSimdVec4i::operator<<=(nsUInt32 uiShift)
{
  m_v.x <<= uiShift;
  m_v.y <<= uiShift;
  m_v.z <<= uiShift;
  m_v.w <<= uiShift;
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4i& nsSimdVec4i::operator>>=(nsUInt32 uiShift)
{
  m_v.x >>= uiShift;
  m_v.y >>= uiShift;
  m_v.z >>= uiShift;
  m_v.w >>= uiShift;
  return *this;
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::CompMin(const nsSimdVec4i& v) const
{
  return m_v.CompMin(v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::CompMax(const nsSimdVec4i& v) const
{
  return m_v.CompMax(v.m_v);
}

NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::Abs() const
{
  nsSimdVec4i result;
  result.m_v.x = nsMath::Abs(m_v.x);
  result.m_v.y = nsMath::Abs(m_v.y);
  result.m_v.z = nsMath::Abs(m_v.z);
  result.m_v.w = nsMath::Abs(m_v.w);

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4i::operator==(const nsSimdVec4i& v) const
{
  bool result[4];
  result[0] = m_v.x == v.m_v.x;
  result[1] = m_v.y == v.m_v.y;
  result[2] = m_v.z == v.m_v.z;
  result[3] = m_v.w == v.m_v.w;

  return nsSimdVec4b(result[0], result[1], result[2], result[3]);
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
  bool result[4];
  result[0] = m_v.x < v.m_v.x;
  result[1] = m_v.y < v.m_v.y;
  result[2] = m_v.z < v.m_v.z;
  result[3] = m_v.w < v.m_v.w;

  return nsSimdVec4b(result[0], result[1], result[2], result[3]);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4i::operator>=(const nsSimdVec4i& v) const
{
  return !(*this < v);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4i::operator>(const nsSimdVec4i& v) const
{
  bool result[4];
  result[0] = m_v.x > v.m_v.x;
  result[1] = m_v.y > v.m_v.y;
  result[2] = m_v.z > v.m_v.z;
  result[3] = m_v.w > v.m_v.w;

  return nsSimdVec4b(result[0], result[1], result[2], result[3]);
}

// static
NS_ALWAYS_INLINE nsSimdVec4i nsSimdVec4i::Select(const nsSimdVec4b& cmp, const nsSimdVec4i& ifTrue, const nsSimdVec4i& ifFalse)
{
  nsSimdVec4i result;
  result.m_v.x = cmp.m_v.x ? ifTrue.m_v.x : ifFalse.m_v.x;
  result.m_v.y = cmp.m_v.y ? ifTrue.m_v.y : ifFalse.m_v.y;
  result.m_v.z = cmp.m_v.z ? ifTrue.m_v.z : ifFalse.m_v.z;
  result.m_v.w = cmp.m_v.w ? ifTrue.m_v.w : ifFalse.m_v.w;

  return result;
}

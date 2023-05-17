#pragma once

WD_ALWAYS_INLINE wdSimdVec4u::wdSimdVec4u()
{
#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
  m_v.Set(0xCDCDCDCD);
#endif
}

WD_ALWAYS_INLINE wdSimdVec4u::wdSimdVec4u(wdUInt32 xyzw)
{
  m_v.Set(xyzw);
}

WD_ALWAYS_INLINE wdSimdVec4u::wdSimdVec4u(wdUInt32 x, wdUInt32 y, wdUInt32 z, wdUInt32 w)
{
  m_v.Set(x, y, z, w);
}

WD_ALWAYS_INLINE wdSimdVec4u::wdSimdVec4u(wdInternal::QuadUInt v)
{
  m_v = v;
}

WD_ALWAYS_INLINE void wdSimdVec4u::Set(wdUInt32 xyzw)
{
  m_v.Set(xyzw);
}

WD_ALWAYS_INLINE void wdSimdVec4u::Set(wdUInt32 x, wdUInt32 y, wdUInt32 z, wdUInt32 w)
{
  m_v.Set(x, y, z, w);
}

WD_ALWAYS_INLINE void wdSimdVec4u::SetZero()
{
  m_v.SetZero();
}

// needs to be implemented here because of include dependencies
WD_ALWAYS_INLINE wdSimdVec4i::wdSimdVec4i(const wdSimdVec4u& u)
  : m_v(u.m_v.x, u.m_v.y, u.m_v.z, u.m_v.w)
{
}

WD_ALWAYS_INLINE wdSimdVec4u::wdSimdVec4u(const wdSimdVec4i& i)
  : m_v(i.m_v.x, i.m_v.y, i.m_v.z, i.m_v.w)
{
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4u::ToFloat() const
{
  wdSimdVec4f result;
  result.m_v.x = (float)m_v.x;
  result.m_v.y = (float)m_v.y;
  result.m_v.z = (float)m_v.z;
  result.m_v.w = (float)m_v.w;

  return result;
}

// static
WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::Truncate(const wdSimdVec4f& f)
{
  wdSimdVec4f clampedF = f.CompMax(wdSimdVec4f::ZeroVector());

  wdSimdVec4u result;
  result.m_v.x = (wdUInt32)clampedF.m_v.x;
  result.m_v.y = (wdUInt32)clampedF.m_v.y;
  result.m_v.z = (wdUInt32)clampedF.m_v.z;
  result.m_v.w = (wdUInt32)clampedF.m_v.w;

  return result;
}

template <int N>
WD_ALWAYS_INLINE wdUInt32 wdSimdVec4u::GetComponent() const
{
  return (&m_v.x)[N];
}

WD_ALWAYS_INLINE wdUInt32 wdSimdVec4u::x() const
{
  return m_v.x;
}

WD_ALWAYS_INLINE wdUInt32 wdSimdVec4u::y() const
{
  return m_v.y;
}

WD_ALWAYS_INLINE wdUInt32 wdSimdVec4u::z() const
{
  return m_v.z;
}

WD_ALWAYS_INLINE wdUInt32 wdSimdVec4u::w() const
{
  return m_v.w;
}

template <wdSwizzle::Enum s>
WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::Get() const
{
  wdSimdVec4u result;

  const wdUInt32* v = &m_v.x;
  result.m_v.x = v[(s & 0x3000) >> 12];
  result.m_v.y = v[(s & 0x0300) >> 8];
  result.m_v.z = v[(s & 0x0030) >> 4];
  result.m_v.w = v[(s & 0x0003)];

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::operator+(const wdSimdVec4u& v) const
{
  return m_v + v.m_v;
}

WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::operator-(const wdSimdVec4u& v) const
{
  return m_v - v.m_v;
}

WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::CompMul(const wdSimdVec4u& v) const
{
  return m_v.CompMul(v.m_v);
}

WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::operator|(const wdSimdVec4u& v) const
{
  wdSimdVec4u result;
  result.m_v.x = m_v.x | v.m_v.x;
  result.m_v.y = m_v.y | v.m_v.y;
  result.m_v.z = m_v.z | v.m_v.z;
  result.m_v.w = m_v.w | v.m_v.w;

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::operator&(const wdSimdVec4u& v) const
{
  wdSimdVec4u result;
  result.m_v.x = m_v.x & v.m_v.x;
  result.m_v.y = m_v.y & v.m_v.y;
  result.m_v.z = m_v.z & v.m_v.z;
  result.m_v.w = m_v.w & v.m_v.w;

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::operator^(const wdSimdVec4u& v) const
{
  wdSimdVec4u result;
  result.m_v.x = m_v.x ^ v.m_v.x;
  result.m_v.y = m_v.y ^ v.m_v.y;
  result.m_v.z = m_v.z ^ v.m_v.z;
  result.m_v.w = m_v.w ^ v.m_v.w;

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::operator~() const
{
  wdSimdVec4u result;
  result.m_v.x = ~m_v.x;
  result.m_v.y = ~m_v.y;
  result.m_v.z = ~m_v.z;
  result.m_v.w = ~m_v.w;

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::operator<<(wdUInt32 uiShift) const
{
  wdSimdVec4u result;
  result.m_v.x = m_v.x << uiShift;
  result.m_v.y = m_v.y << uiShift;
  result.m_v.z = m_v.z << uiShift;
  result.m_v.w = m_v.w << uiShift;

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::operator>>(wdUInt32 uiShift) const
{
  wdSimdVec4u result;
  result.m_v.x = m_v.x >> uiShift;
  result.m_v.y = m_v.y >> uiShift;
  result.m_v.z = m_v.z >> uiShift;
  result.m_v.w = m_v.w >> uiShift;

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4u& wdSimdVec4u::operator+=(const wdSimdVec4u& v)
{
  m_v += v.m_v;
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4u& wdSimdVec4u::operator-=(const wdSimdVec4u& v)
{
  m_v -= v.m_v;
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4u& wdSimdVec4u::operator|=(const wdSimdVec4u& v)
{
  m_v.x |= v.m_v.x;
  m_v.y |= v.m_v.y;
  m_v.z |= v.m_v.z;
  m_v.w |= v.m_v.w;
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4u& wdSimdVec4u::operator&=(const wdSimdVec4u& v)
{
  m_v.x &= v.m_v.x;
  m_v.y &= v.m_v.y;
  m_v.z &= v.m_v.z;
  m_v.w &= v.m_v.w;
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4u& wdSimdVec4u::operator^=(const wdSimdVec4u& v)
{
  m_v.x ^= v.m_v.x;
  m_v.y ^= v.m_v.y;
  m_v.z ^= v.m_v.z;
  m_v.w ^= v.m_v.w;
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4u& wdSimdVec4u::operator<<=(wdUInt32 uiShift)
{
  m_v.x <<= uiShift;
  m_v.y <<= uiShift;
  m_v.z <<= uiShift;
  m_v.w <<= uiShift;
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4u& wdSimdVec4u::operator>>=(wdUInt32 uiShift)
{
  m_v.x >>= uiShift;
  m_v.y >>= uiShift;
  m_v.z >>= uiShift;
  m_v.w >>= uiShift;
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::CompMin(const wdSimdVec4u& v) const
{
  return m_v.CompMin(v.m_v);
}

WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::CompMax(const wdSimdVec4u& v) const
{
  return m_v.CompMax(v.m_v);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4u::operator==(const wdSimdVec4u& v) const
{
  bool result[4];
  result[0] = m_v.x == v.m_v.x;
  result[1] = m_v.y == v.m_v.y;
  result[2] = m_v.z == v.m_v.z;
  result[3] = m_v.w == v.m_v.w;

  return wdSimdVec4b(result[0], result[1], result[2], result[3]);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4u::operator!=(const wdSimdVec4u& v) const
{
  return !(*this == v);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4u::operator<=(const wdSimdVec4u& v) const
{
  return !(*this > v);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4u::operator<(const wdSimdVec4u& v) const
{
  bool result[4];
  result[0] = m_v.x < v.m_v.x;
  result[1] = m_v.y < v.m_v.y;
  result[2] = m_v.z < v.m_v.z;
  result[3] = m_v.w < v.m_v.w;

  return wdSimdVec4b(result[0], result[1], result[2], result[3]);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4u::operator>=(const wdSimdVec4u& v) const
{
  return !(*this < v);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4u::operator>(const wdSimdVec4u& v) const
{
  bool result[4];
  result[0] = m_v.x > v.m_v.x;
  result[1] = m_v.y > v.m_v.y;
  result[2] = m_v.z > v.m_v.z;
  result[3] = m_v.w > v.m_v.w;

  return wdSimdVec4b(result[0], result[1], result[2], result[3]);
}

// static
WD_ALWAYS_INLINE wdSimdVec4u wdSimdVec4u::ZeroVector()
{
  return wdVec4U32::ZeroVector();
}

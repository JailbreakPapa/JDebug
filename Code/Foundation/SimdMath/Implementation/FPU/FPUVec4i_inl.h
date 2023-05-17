#pragma once

WD_ALWAYS_INLINE wdSimdVec4i::wdSimdVec4i()
{
#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
  m_v.Set(0xCDCDCDCD);
#endif
}

WD_ALWAYS_INLINE wdSimdVec4i::wdSimdVec4i(wdInt32 xyzw)
{
  m_v.Set(xyzw);
}

WD_ALWAYS_INLINE wdSimdVec4i::wdSimdVec4i(wdInt32 x, wdInt32 y, wdInt32 z, wdInt32 w)
{
  m_v.Set(x, y, z, w);
}

WD_ALWAYS_INLINE wdSimdVec4i::wdSimdVec4i(wdInternal::QuadInt v)
{
  m_v = v;
}

WD_ALWAYS_INLINE void wdSimdVec4i::Set(wdInt32 xyzw)
{
  m_v.Set(xyzw);
}

WD_ALWAYS_INLINE void wdSimdVec4i::Set(wdInt32 x, wdInt32 y, wdInt32 z, wdInt32 w)
{
  m_v.Set(x, y, z, w);
}

WD_ALWAYS_INLINE void wdSimdVec4i::SetZero()
{
  m_v.SetZero();
}

template <int N>
WD_ALWAYS_INLINE void wdSimdVec4i::Load(const wdInt32* pInts)
{
  m_v.SetZero();
  for (int i = 0; i < N; ++i)
  {
    (&m_v.x)[i] = pInts[i];
  }
}

template <int N>
WD_ALWAYS_INLINE void wdSimdVec4i::Store(wdInt32* pInts) const
{
  for (int i = 0; i < N; ++i)
  {
    pInts[i] = (&m_v.x)[i];
  }
}

WD_ALWAYS_INLINE wdSimdVec4f wdSimdVec4i::ToFloat() const
{
  wdSimdVec4f result;
  result.m_v.x = (float)m_v.x;
  result.m_v.y = (float)m_v.y;
  result.m_v.z = (float)m_v.z;
  result.m_v.w = (float)m_v.w;

  return result;
}

// static
WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::Truncate(const wdSimdVec4f& f)
{
  wdSimdVec4i result;
  result.m_v.x = (wdInt32)f.m_v.x;
  result.m_v.y = (wdInt32)f.m_v.y;
  result.m_v.z = (wdInt32)f.m_v.z;
  result.m_v.w = (wdInt32)f.m_v.w;

  return result;
}

template <int N>
WD_ALWAYS_INLINE wdInt32 wdSimdVec4i::GetComponent() const
{
  return (&m_v.x)[N];
}

WD_ALWAYS_INLINE wdInt32 wdSimdVec4i::x() const
{
  return m_v.x;
}

WD_ALWAYS_INLINE wdInt32 wdSimdVec4i::y() const
{
  return m_v.y;
}

WD_ALWAYS_INLINE wdInt32 wdSimdVec4i::z() const
{
  return m_v.z;
}

WD_ALWAYS_INLINE wdInt32 wdSimdVec4i::w() const
{
  return m_v.w;
}

template <wdSwizzle::Enum s>
WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::Get() const
{
  wdSimdVec4i result;

  const wdInt32* v = &m_v.x;
  result.m_v.x = v[(s & 0x3000) >> 12];
  result.m_v.y = v[(s & 0x0300) >> 8];
  result.m_v.z = v[(s & 0x0030) >> 4];
  result.m_v.w = v[(s & 0x0003)];

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::operator-() const
{
  return -m_v;
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::operator+(const wdSimdVec4i& v) const
{
  return m_v + v.m_v;
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::operator-(const wdSimdVec4i& v) const
{
  return m_v - v.m_v;
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::CompMul(const wdSimdVec4i& v) const
{
  return m_v.CompMul(v.m_v);
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::CompDiv(const wdSimdVec4i& v) const
{
  return m_v.CompDiv(v.m_v);
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::operator|(const wdSimdVec4i& v) const
{
  wdSimdVec4i result;
  result.m_v.x = m_v.x | v.m_v.x;
  result.m_v.y = m_v.y | v.m_v.y;
  result.m_v.z = m_v.z | v.m_v.z;
  result.m_v.w = m_v.w | v.m_v.w;

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::operator&(const wdSimdVec4i& v) const
{
  wdSimdVec4i result;
  result.m_v.x = m_v.x & v.m_v.x;
  result.m_v.y = m_v.y & v.m_v.y;
  result.m_v.z = m_v.z & v.m_v.z;
  result.m_v.w = m_v.w & v.m_v.w;

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::operator^(const wdSimdVec4i& v) const
{
  wdSimdVec4i result;
  result.m_v.x = m_v.x ^ v.m_v.x;
  result.m_v.y = m_v.y ^ v.m_v.y;
  result.m_v.z = m_v.z ^ v.m_v.z;
  result.m_v.w = m_v.w ^ v.m_v.w;

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::operator~() const
{
  wdSimdVec4i result;
  result.m_v.x = ~m_v.x;
  result.m_v.y = ~m_v.y;
  result.m_v.z = ~m_v.z;
  result.m_v.w = ~m_v.w;

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::operator<<(wdUInt32 uiShift) const
{
  wdSimdVec4i result;
  result.m_v.x = m_v.x << uiShift;
  result.m_v.y = m_v.y << uiShift;
  result.m_v.z = m_v.z << uiShift;
  result.m_v.w = m_v.w << uiShift;

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::operator>>(wdUInt32 uiShift) const
{
  wdSimdVec4i result;
  result.m_v.x = m_v.x >> uiShift;
  result.m_v.y = m_v.y >> uiShift;
  result.m_v.z = m_v.z >> uiShift;
  result.m_v.w = m_v.w >> uiShift;

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::operator<<(const wdSimdVec4i& v) const
{
  wdSimdVec4i result;
  result.m_v.x = m_v.x << v.m_v.x;
  result.m_v.y = m_v.y << v.m_v.y;
  result.m_v.z = m_v.z << v.m_v.z;
  result.m_v.w = m_v.w << v.m_v.w;

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::operator>>(const wdSimdVec4i& v) const
{
  wdSimdVec4i result;
  result.m_v.x = m_v.x >> v.m_v.x;
  result.m_v.y = m_v.y >> v.m_v.y;
  result.m_v.z = m_v.z >> v.m_v.z;
  result.m_v.w = m_v.w >> v.m_v.w;

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4i& wdSimdVec4i::operator+=(const wdSimdVec4i& v)
{
  m_v += v.m_v;
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4i& wdSimdVec4i::operator-=(const wdSimdVec4i& v)
{
  m_v -= v.m_v;
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4i& wdSimdVec4i::operator|=(const wdSimdVec4i& v)
{
  m_v.x |= v.m_v.x;
  m_v.y |= v.m_v.y;
  m_v.z |= v.m_v.z;
  m_v.w |= v.m_v.w;
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4i& wdSimdVec4i::operator&=(const wdSimdVec4i& v)
{
  m_v.x &= v.m_v.x;
  m_v.y &= v.m_v.y;
  m_v.z &= v.m_v.z;
  m_v.w &= v.m_v.w;
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4i& wdSimdVec4i::operator^=(const wdSimdVec4i& v)
{
  m_v.x ^= v.m_v.x;
  m_v.y ^= v.m_v.y;
  m_v.z ^= v.m_v.z;
  m_v.w ^= v.m_v.w;
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4i& wdSimdVec4i::operator<<=(wdUInt32 uiShift)
{
  m_v.x <<= uiShift;
  m_v.y <<= uiShift;
  m_v.z <<= uiShift;
  m_v.w <<= uiShift;
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4i& wdSimdVec4i::operator>>=(wdUInt32 uiShift)
{
  m_v.x >>= uiShift;
  m_v.y >>= uiShift;
  m_v.z >>= uiShift;
  m_v.w >>= uiShift;
  return *this;
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::CompMin(const wdSimdVec4i& v) const
{
  return m_v.CompMin(v.m_v);
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::CompMax(const wdSimdVec4i& v) const
{
  return m_v.CompMax(v.m_v);
}

WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::Abs() const
{
  wdSimdVec4i result;
  result.m_v.x = wdMath::Abs(m_v.x);
  result.m_v.y = wdMath::Abs(m_v.y);
  result.m_v.z = wdMath::Abs(m_v.z);
  result.m_v.w = wdMath::Abs(m_v.w);

  return result;
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4i::operator==(const wdSimdVec4i& v) const
{
  bool result[4];
  result[0] = m_v.x == v.m_v.x;
  result[1] = m_v.y == v.m_v.y;
  result[2] = m_v.z == v.m_v.z;
  result[3] = m_v.w == v.m_v.w;

  return wdSimdVec4b(result[0], result[1], result[2], result[3]);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4i::operator!=(const wdSimdVec4i& v) const
{
  return !(*this == v);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4i::operator<=(const wdSimdVec4i& v) const
{
  return !(*this > v);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4i::operator<(const wdSimdVec4i& v) const
{
  bool result[4];
  result[0] = m_v.x < v.m_v.x;
  result[1] = m_v.y < v.m_v.y;
  result[2] = m_v.z < v.m_v.z;
  result[3] = m_v.w < v.m_v.w;

  return wdSimdVec4b(result[0], result[1], result[2], result[3]);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4i::operator>=(const wdSimdVec4i& v) const
{
  return !(*this < v);
}

WD_ALWAYS_INLINE wdSimdVec4b wdSimdVec4i::operator>(const wdSimdVec4i& v) const
{
  bool result[4];
  result[0] = m_v.x > v.m_v.x;
  result[1] = m_v.y > v.m_v.y;
  result[2] = m_v.z > v.m_v.z;
  result[3] = m_v.w > v.m_v.w;

  return wdSimdVec4b(result[0], result[1], result[2], result[3]);
}

// static
WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::ZeroVector()
{
  return wdVec4I32::ZeroVector();
}

// static
WD_ALWAYS_INLINE wdSimdVec4i wdSimdVec4i::Select(const wdSimdVec4b& cmp, const wdSimdVec4i& ifTrue, const wdSimdVec4i& ifFalse)
{
  wdSimdVec4i result;
  result.m_v.x = cmp.m_v.x ? ifTrue.m_v.x : ifFalse.m_v.x;
  result.m_v.y = cmp.m_v.y ? ifTrue.m_v.y : ifFalse.m_v.y;
  result.m_v.z = cmp.m_v.z ? ifTrue.m_v.z : ifFalse.m_v.z;
  result.m_v.w = cmp.m_v.w ? ifTrue.m_v.w : ifFalse.m_v.w;

  return result;
}

#pragma once

NS_ALWAYS_INLINE nsSimdVec4b::nsSimdVec4b() {}

NS_ALWAYS_INLINE nsSimdVec4b::nsSimdVec4b(bool b)
{
  m_v.x = b ? 0xFFFFFFFF : 0;
  m_v.y = b ? 0xFFFFFFFF : 0;
  m_v.z = b ? 0xFFFFFFFF : 0;
  m_v.w = b ? 0xFFFFFFFF : 0;
}

NS_ALWAYS_INLINE nsSimdVec4b::nsSimdVec4b(bool x, bool y, bool z, bool w)
{
  m_v.x = x ? 0xFFFFFFFF : 0;
  m_v.y = y ? 0xFFFFFFFF : 0;
  m_v.z = z ? 0xFFFFFFFF : 0;
  m_v.w = w ? 0xFFFFFFFF : 0;
}

NS_ALWAYS_INLINE nsSimdVec4b::nsSimdVec4b(nsInternal::QuadBool v)
{
  m_v = v;
}

template <int N>
NS_ALWAYS_INLINE bool nsSimdVec4b::GetComponent() const
{
  return (&m_v.x)[N] != 0;
}

NS_ALWAYS_INLINE bool nsSimdVec4b::x() const
{
  return m_v.x != 0;
}

NS_ALWAYS_INLINE bool nsSimdVec4b::y() const
{
  return m_v.y != 0;
}

NS_ALWAYS_INLINE bool nsSimdVec4b::z() const
{
  return m_v.z != 0;
}

NS_ALWAYS_INLINE bool nsSimdVec4b::w() const
{
  return m_v.w != 0;
}

template <nsSwizzle::Enum s>
NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4b::Get() const
{
  nsSimdVec4b result;

  const nsUInt32* v = &m_v.x;
  result.m_v.x = v[(s & 0x3000) >> 12];
  result.m_v.y = v[(s & 0x0300) >> 8];
  result.m_v.z = v[(s & 0x0030) >> 4];
  result.m_v.w = v[(s & 0x0003)];

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4b::operator&&(const nsSimdVec4b& rhs) const
{
  nsSimdVec4b result;
  result.m_v.x = m_v.x & rhs.m_v.x;
  result.m_v.y = m_v.y & rhs.m_v.y;
  result.m_v.z = m_v.z & rhs.m_v.z;
  result.m_v.w = m_v.w & rhs.m_v.w;

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4b::operator||(const nsSimdVec4b& rhs) const
{
  nsSimdVec4b result;
  result.m_v.x = m_v.x | rhs.m_v.x;
  result.m_v.y = m_v.y | rhs.m_v.y;
  result.m_v.z = m_v.z | rhs.m_v.z;
  result.m_v.w = m_v.w | rhs.m_v.w;

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4b::operator!() const
{
  nsSimdVec4b result;
  result.m_v.x = m_v.x ^ 0xFFFFFFFF;
  result.m_v.y = m_v.y ^ 0xFFFFFFFF;
  result.m_v.z = m_v.z ^ 0xFFFFFFFF;
  result.m_v.w = m_v.w ^ 0xFFFFFFFF;

  return result;
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4b::operator==(const nsSimdVec4b& rhs) const
{
  return !(*this != rhs);
}

NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4b::operator!=(const nsSimdVec4b& rhs) const
{
  nsSimdVec4b result;
  result.m_v.x = m_v.x ^ rhs.m_v.x;
  result.m_v.y = m_v.y ^ rhs.m_v.y;
  result.m_v.z = m_v.z ^ rhs.m_v.z;
  result.m_v.w = m_v.w ^ rhs.m_v.w;

  return result;
}

template <int N>
NS_ALWAYS_INLINE bool nsSimdVec4b::AllSet() const
{
  for (int i = 0; i < N; ++i)
  {
    if (!(&m_v.x)[i])
      return false;
  }

  return true;
}

template <int N>
NS_ALWAYS_INLINE bool nsSimdVec4b::AnySet() const
{
  for (int i = 0; i < N; ++i)
  {
    if ((&m_v.x)[i])
      return true;
  }

  return false;
}

template <int N>
NS_ALWAYS_INLINE bool nsSimdVec4b::NoneSet() const
{
  return !AnySet<N>();
}

// static
NS_ALWAYS_INLINE nsSimdVec4b nsSimdVec4b::Select(const nsSimdVec4b& cmp, const nsSimdVec4b& ifTrue, const nsSimdVec4b& ifFalse)
{
  nsSimdVec4b result;
  result.m_v.x = (cmp.m_v.x != 0) ? ifTrue.m_v.x : ifFalse.m_v.x;
  result.m_v.y = (cmp.m_v.y != 0) ? ifTrue.m_v.y : ifFalse.m_v.y;
  result.m_v.z = (cmp.m_v.z != 0) ? ifTrue.m_v.z : ifFalse.m_v.z;
  result.m_v.w = (cmp.m_v.w != 0) ? ifTrue.m_v.w : ifFalse.m_v.w;

  return result;
}

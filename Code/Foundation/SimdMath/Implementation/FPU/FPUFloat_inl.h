#pragma once

NS_ALWAYS_INLINE nsSimdFloat::nsSimdFloat() {}

NS_ALWAYS_INLINE nsSimdFloat::nsSimdFloat(float f)
{
  m_v.Set(f);
}

NS_ALWAYS_INLINE nsSimdFloat::nsSimdFloat(nsInt32 i)
{
  m_v.Set((float)i);
}

NS_ALWAYS_INLINE nsSimdFloat::nsSimdFloat(nsUInt32 i)
{
  m_v.Set((float)i);
}

NS_ALWAYS_INLINE nsSimdFloat::nsSimdFloat(nsAngle a)
{
  m_v.Set(a.GetRadian());
}

NS_ALWAYS_INLINE nsSimdFloat::nsSimdFloat(nsInternal::QuadFloat v)
{
  m_v = v;
}

NS_ALWAYS_INLINE nsSimdFloat::operator float() const
{
  return m_v.x;
}

// static
NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::MakeZero()
{
  return nsSimdFloat(0.0f);
}

// static
NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::MakeNaN()
{
  return nsSimdFloat(nsMath::NaN<float>());
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::operator+(const nsSimdFloat& f) const
{
  return m_v + f.m_v;
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::operator-(const nsSimdFloat& f) const
{
  return m_v - f.m_v;
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::operator*(const nsSimdFloat& f) const
{
  return m_v.CompMul(f.m_v);
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::operator/(const nsSimdFloat& f) const
{
  return m_v.CompDiv(f.m_v);
}

NS_ALWAYS_INLINE nsSimdFloat& nsSimdFloat::operator+=(const nsSimdFloat& f)
{
  m_v += f.m_v;
  return *this;
}

NS_ALWAYS_INLINE nsSimdFloat& nsSimdFloat::operator-=(const nsSimdFloat& f)
{
  m_v -= f.m_v;
  return *this;
}

NS_ALWAYS_INLINE nsSimdFloat& nsSimdFloat::operator*=(const nsSimdFloat& f)
{
  m_v = m_v.CompMul(f.m_v);
  return *this;
}

NS_ALWAYS_INLINE nsSimdFloat& nsSimdFloat::operator/=(const nsSimdFloat& f)
{
  m_v = m_v.CompDiv(f.m_v);
  return *this;
}

NS_ALWAYS_INLINE bool nsSimdFloat::IsEqual(const nsSimdFloat& rhs, const nsSimdFloat& fEpsilon) const
{
  return m_v.IsEqual(rhs.m_v, fEpsilon);
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator==(const nsSimdFloat& f) const
{
  return m_v.x == f.m_v.x;
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator!=(const nsSimdFloat& f) const
{
  return m_v.x != f.m_v.x;
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator>=(const nsSimdFloat& f) const
{
  return m_v.x >= f.m_v.x;
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator>(const nsSimdFloat& f) const
{
  return m_v.x > f.m_v.x;
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator<=(const nsSimdFloat& f) const
{
  return m_v.x <= f.m_v.x;
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator<(const nsSimdFloat& f) const
{
  return m_v.x < f.m_v.x;
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator==(float f) const
{
  return m_v.x == f;
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator!=(float f) const
{
  return m_v.x != f;
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator>(float f) const
{
  return m_v.x > f;
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator>=(float f) const
{
  return m_v.x >= f;
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator<(float f) const
{
  return m_v.x < f;
}

NS_ALWAYS_INLINE bool nsSimdFloat::operator<=(float f) const
{
  return m_v.x <= f;
}

template <nsMathAcc::Enum acc>
NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::GetReciprocal() const
{
  return nsSimdFloat(1.0f / m_v.x);
}

template <nsMathAcc::Enum acc>
NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::GetSqrt() const
{
  return nsSimdFloat(nsMath::Sqrt(m_v.x));
}

template <nsMathAcc::Enum acc>
NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::GetInvSqrt() const
{
  return nsSimdFloat(1.0f / nsMath::Sqrt(m_v.x));
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::Max(const nsSimdFloat& f) const
{
  return m_v.CompMax(f.m_v);
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::Min(const nsSimdFloat& f) const
{
  return m_v.CompMin(f.m_v);
}

NS_ALWAYS_INLINE nsSimdFloat nsSimdFloat::Abs() const
{
  return nsSimdFloat(nsMath::Abs(m_v.x));
}

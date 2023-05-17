#pragma once

WD_ALWAYS_INLINE wdSimdFloat::wdSimdFloat() {}

WD_ALWAYS_INLINE wdSimdFloat::wdSimdFloat(float f)
{
  m_v.Set(f);
}

WD_ALWAYS_INLINE wdSimdFloat::wdSimdFloat(wdInt32 i)
{
  m_v.Set((float)i);
}

WD_ALWAYS_INLINE wdSimdFloat::wdSimdFloat(wdUInt32 i)
{
  m_v.Set((float)i);
}

WD_ALWAYS_INLINE wdSimdFloat::wdSimdFloat(wdAngle a)
{
  m_v.Set(a.GetRadian());
}

WD_ALWAYS_INLINE wdSimdFloat::wdSimdFloat(wdInternal::QuadFloat v)
{
  m_v = v;
}

WD_ALWAYS_INLINE wdSimdFloat::operator float() const
{
  return m_v.x;
}

// static
WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::Zero()
{
  return wdSimdFloat(0.0f);
}

WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::operator+(const wdSimdFloat& f) const
{
  return m_v + f.m_v;
}

WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::operator-(const wdSimdFloat& f) const
{
  return m_v - f.m_v;
}

WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::operator*(const wdSimdFloat& f) const
{
  return m_v.CompMul(f.m_v);
}

WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::operator/(const wdSimdFloat& f) const
{
  return m_v.CompDiv(f.m_v);
}

WD_ALWAYS_INLINE wdSimdFloat& wdSimdFloat::operator+=(const wdSimdFloat& f)
{
  m_v += f.m_v;
  return *this;
}

WD_ALWAYS_INLINE wdSimdFloat& wdSimdFloat::operator-=(const wdSimdFloat& f)
{
  m_v -= f.m_v;
  return *this;
}

WD_ALWAYS_INLINE wdSimdFloat& wdSimdFloat::operator*=(const wdSimdFloat& f)
{
  m_v = m_v.CompMul(f.m_v);
  return *this;
}

WD_ALWAYS_INLINE wdSimdFloat& wdSimdFloat::operator/=(const wdSimdFloat& f)
{
  m_v = m_v.CompDiv(f.m_v);
  return *this;
}

WD_ALWAYS_INLINE bool wdSimdFloat::IsEqual(const wdSimdFloat& rhs, const wdSimdFloat& fEpsilon) const
{
  return m_v.IsEqual(rhs.m_v, fEpsilon);
}

WD_ALWAYS_INLINE bool wdSimdFloat::operator==(const wdSimdFloat& f) const
{
  return m_v.x == f.m_v.x;
}

WD_ALWAYS_INLINE bool wdSimdFloat::operator!=(const wdSimdFloat& f) const
{
  return m_v.x != f.m_v.x;
}

WD_ALWAYS_INLINE bool wdSimdFloat::operator>=(const wdSimdFloat& f) const
{
  return m_v.x >= f.m_v.x;
}

WD_ALWAYS_INLINE bool wdSimdFloat::operator>(const wdSimdFloat& f) const
{
  return m_v.x > f.m_v.x;
}

WD_ALWAYS_INLINE bool wdSimdFloat::operator<=(const wdSimdFloat& f) const
{
  return m_v.x <= f.m_v.x;
}

WD_ALWAYS_INLINE bool wdSimdFloat::operator<(const wdSimdFloat& f) const
{
  return m_v.x < f.m_v.x;
}

WD_ALWAYS_INLINE bool wdSimdFloat::operator==(float f) const
{
  return m_v.x == f;
}

WD_ALWAYS_INLINE bool wdSimdFloat::operator!=(float f) const
{
  return m_v.x != f;
}

WD_ALWAYS_INLINE bool wdSimdFloat::operator>(float f) const
{
  return m_v.x > f;
}

WD_ALWAYS_INLINE bool wdSimdFloat::operator>=(float f) const
{
  return m_v.x >= f;
}

WD_ALWAYS_INLINE bool wdSimdFloat::operator<(float f) const
{
  return m_v.x < f;
}

WD_ALWAYS_INLINE bool wdSimdFloat::operator<=(float f) const
{
  return m_v.x <= f;
}

template <wdMathAcc::Enum acc>
WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::GetReciprocal() const
{
  return wdSimdFloat(1.0f / m_v.x);
}

template <wdMathAcc::Enum acc>
WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::GetSqrt() const
{
  return wdSimdFloat(wdMath::Sqrt(m_v.x));
}

template <wdMathAcc::Enum acc>
WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::GetInvSqrt() const
{
  return wdSimdFloat(1.0f / wdMath::Sqrt(m_v.x));
}

WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::Max(const wdSimdFloat& f) const
{
  return m_v.CompMax(f.m_v);
}

WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::Min(const wdSimdFloat& f) const
{
  return m_v.CompMin(f.m_v);
}

WD_ALWAYS_INLINE wdSimdFloat wdSimdFloat::Abs() const
{
  return wdSimdFloat(wdMath::Abs(m_v.x));
}

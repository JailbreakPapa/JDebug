#pragma once

template <typename Type>
constexpr WD_ALWAYS_INLINE Type wdAngle::Pi()
{
  return static_cast<Type>(3.1415926535897932384626433832795);
}

template <typename Type>
constexpr WD_ALWAYS_INLINE Type wdAngle::DegToRadMultiplier()
{
  return Pi<Type>() / (Type)180;
}

template <typename Type>
constexpr WD_ALWAYS_INLINE Type wdAngle::RadToDegMultiplier()
{
  return ((Type)180) / Pi<Type>();
}

template <typename Type>
constexpr Type wdAngle::DegToRad(Type f)
{
  return f * DegToRadMultiplier<Type>();
}

template <typename Type>
constexpr Type wdAngle::RadToDeg(Type f)
{
  return f * RadToDegMultiplier<Type>();
}

constexpr inline wdAngle wdAngle::Degree(float fDegree)
{
  return wdAngle(DegToRad(fDegree));
}

constexpr WD_ALWAYS_INLINE wdAngle wdAngle::Radian(float fRadian)
{
  return wdAngle(fRadian);
}

constexpr inline float wdAngle::GetDegree() const
{
  return RadToDeg(m_fRadian);
}

constexpr WD_ALWAYS_INLINE float wdAngle::GetRadian() const
{
  return m_fRadian;
}

inline wdAngle wdAngle::GetNormalizedRange() const
{
  wdAngle out(m_fRadian);
  out.NormalizeRange();
  return out;
}

inline bool wdAngle::IsEqualSimple(wdAngle rhs, wdAngle epsilon) const
{
  const wdAngle diff = AngleBetween(*this, rhs);

  return ((diff.m_fRadian >= -epsilon.m_fRadian) && (diff.m_fRadian <= epsilon.m_fRadian));
}

inline bool wdAngle::IsEqualNormalized(wdAngle rhs, wdAngle epsilon) const
{
  // equality between normalized angles
  const wdAngle aNorm = GetNormalizedRange();
  const wdAngle bNorm = rhs.GetNormalizedRange();

  return aNorm.IsEqualSimple(bNorm, epsilon);
}

constexpr WD_ALWAYS_INLINE wdAngle wdAngle::operator-() const
{
  return wdAngle(-m_fRadian);
}

WD_ALWAYS_INLINE void wdAngle::operator+=(wdAngle r)
{
  m_fRadian += r.m_fRadian;
}

WD_ALWAYS_INLINE void wdAngle::operator-=(wdAngle r)
{
  m_fRadian -= r.m_fRadian;
}

constexpr inline wdAngle wdAngle::operator+(wdAngle r) const
{
  return wdAngle(m_fRadian + r.m_fRadian);
}

constexpr inline wdAngle wdAngle::operator-(wdAngle r) const
{
  return wdAngle(m_fRadian - r.m_fRadian);
}

constexpr WD_ALWAYS_INLINE bool wdAngle::operator==(const wdAngle& r) const
{
  return m_fRadian == r.m_fRadian;
}

constexpr WD_ALWAYS_INLINE bool wdAngle::operator!=(const wdAngle& r) const
{
  return m_fRadian != r.m_fRadian;
}

constexpr WD_ALWAYS_INLINE bool wdAngle::operator<(const wdAngle& r) const
{
  return m_fRadian < r.m_fRadian;
}

constexpr WD_ALWAYS_INLINE bool wdAngle::operator>(const wdAngle& r) const
{
  return m_fRadian > r.m_fRadian;
}

constexpr WD_ALWAYS_INLINE bool wdAngle::operator<=(const wdAngle& r) const
{
  return m_fRadian <= r.m_fRadian;
}

constexpr WD_ALWAYS_INLINE bool wdAngle::operator>=(const wdAngle& r) const
{
  return m_fRadian >= r.m_fRadian;
}

constexpr inline wdAngle operator*(wdAngle a, float f)
{
  return wdAngle::Radian(a.GetRadian() * f);
}

constexpr inline wdAngle operator*(float f, wdAngle a)
{
  return wdAngle::Radian(a.GetRadian() * f);
}

constexpr inline wdAngle operator/(wdAngle a, float f)
{
  return wdAngle::Radian(a.GetRadian() / f);
}

constexpr inline float operator/(wdAngle a, wdAngle b)
{
  return a.GetRadian() / b.GetRadian();
}

#pragma once

template <typename Type>
constexpr NS_ALWAYS_INLINE Type nsAngle::Pi()
{
  return static_cast<Type>(3.1415926535897932384626433832795);
}

template <typename Type>
constexpr NS_ALWAYS_INLINE Type nsAngle::DegToRadMultiplier()
{
  return Pi<Type>() / (Type)180;
}

template <typename Type>
constexpr NS_ALWAYS_INLINE Type nsAngle::RadToDegMultiplier()
{
  return ((Type)180) / Pi<Type>();
}

template <typename Type>
constexpr Type nsAngle::DegToRad(Type f)
{
  return f * DegToRadMultiplier<Type>();
}

template <typename Type>
constexpr Type nsAngle::RadToDeg(Type f)
{
  return f * RadToDegMultiplier<Type>();
}

constexpr inline nsAngle nsAngle::MakeFromDegree(float fDegree)
{
  return nsAngle(DegToRad(fDegree));
}

constexpr NS_ALWAYS_INLINE nsAngle nsAngle::MakeFromRadian(float fRadian)
{
  return nsAngle(fRadian);
}

constexpr inline float nsAngle::GetDegree() const
{
  return RadToDeg(m_fRadian);
}

constexpr NS_ALWAYS_INLINE float nsAngle::GetRadian() const
{
  return m_fRadian;
}

inline nsAngle nsAngle::GetNormalizedRange() const
{
  nsAngle out(m_fRadian);
  out.NormalizeRange();
  return out;
}

inline bool nsAngle::IsEqualSimple(nsAngle rhs, nsAngle epsilon) const
{
  const nsAngle diff = AngleBetween(*this, rhs);

  return ((diff.m_fRadian >= -epsilon.m_fRadian) && (diff.m_fRadian <= epsilon.m_fRadian));
}

inline bool nsAngle::IsEqualNormalized(nsAngle rhs, nsAngle epsilon) const
{
  // equality between normalized angles
  const nsAngle aNorm = GetNormalizedRange();
  const nsAngle bNorm = rhs.GetNormalizedRange();

  return aNorm.IsEqualSimple(bNorm, epsilon);
}

constexpr NS_ALWAYS_INLINE nsAngle nsAngle::operator-() const
{
  return nsAngle(-m_fRadian);
}

NS_ALWAYS_INLINE void nsAngle::operator+=(nsAngle r)
{
  m_fRadian += r.m_fRadian;
}

NS_ALWAYS_INLINE void nsAngle::operator-=(nsAngle r)
{
  m_fRadian -= r.m_fRadian;
}

constexpr inline nsAngle nsAngle::operator+(nsAngle r) const
{
  return nsAngle(m_fRadian + r.m_fRadian);
}

constexpr inline nsAngle nsAngle::operator-(nsAngle r) const
{
  return nsAngle(m_fRadian - r.m_fRadian);
}

constexpr NS_ALWAYS_INLINE bool nsAngle::operator==(const nsAngle& r) const
{
  return m_fRadian == r.m_fRadian;
}

constexpr NS_ALWAYS_INLINE bool nsAngle::operator!=(const nsAngle& r) const
{
  return m_fRadian != r.m_fRadian;
}

constexpr NS_ALWAYS_INLINE bool nsAngle::operator<(const nsAngle& r) const
{
  return m_fRadian < r.m_fRadian;
}

constexpr NS_ALWAYS_INLINE bool nsAngle::operator>(const nsAngle& r) const
{
  return m_fRadian > r.m_fRadian;
}

constexpr NS_ALWAYS_INLINE bool nsAngle::operator<=(const nsAngle& r) const
{
  return m_fRadian <= r.m_fRadian;
}

constexpr NS_ALWAYS_INLINE bool nsAngle::operator>=(const nsAngle& r) const
{
  return m_fRadian >= r.m_fRadian;
}

constexpr inline nsAngle operator*(nsAngle a, float f)
{
  return nsAngle::MakeFromRadian(a.GetRadian() * f);
}

constexpr inline nsAngle operator*(float f, nsAngle a)
{
  return nsAngle::MakeFromRadian(a.GetRadian() * f);
}

constexpr inline nsAngle operator/(nsAngle a, float f)
{
  return nsAngle::MakeFromRadian(a.GetRadian() / f);
}

constexpr inline float operator/(nsAngle a, nsAngle b)
{
  return a.GetRadian() / b.GetRadian();
}

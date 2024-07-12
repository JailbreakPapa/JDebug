#pragma once

#include <Foundation/Basics.h>

constexpr NS_ALWAYS_INLINE nsTime::nsTime(double fTime)
  : m_fTime(fTime)
{
}

constexpr NS_ALWAYS_INLINE float nsTime::AsFloatInSeconds() const
{
  return static_cast<float>(m_fTime);
}

constexpr NS_ALWAYS_INLINE double nsTime::GetNanoseconds() const
{
  return m_fTime * 1000000000.0;
}

constexpr NS_ALWAYS_INLINE double nsTime::GetMicroseconds() const
{
  return m_fTime * 1000000.0;
}

constexpr NS_ALWAYS_INLINE double nsTime::GetMilliseconds() const
{
  return m_fTime * 1000.0;
}

constexpr NS_ALWAYS_INLINE double nsTime::GetSeconds() const
{
  return m_fTime;
}

constexpr NS_ALWAYS_INLINE double nsTime::GetMinutes() const
{
  return m_fTime / 60.0;
}

constexpr NS_ALWAYS_INLINE double nsTime::GetHours() const
{
  return m_fTime / (60.0 * 60.0);
}

constexpr NS_ALWAYS_INLINE void nsTime::operator-=(const nsTime& other)
{
  m_fTime -= other.m_fTime;
}

constexpr NS_ALWAYS_INLINE void nsTime::operator+=(const nsTime& other)
{
  m_fTime += other.m_fTime;
}

constexpr NS_ALWAYS_INLINE void nsTime::operator*=(double fFactor)
{
  m_fTime *= fFactor;
}

constexpr NS_ALWAYS_INLINE void nsTime::operator/=(double fFactor)
{
  m_fTime /= fFactor;
}

constexpr NS_ALWAYS_INLINE nsTime nsTime::operator-() const
{
  return nsTime(-m_fTime);
}

constexpr NS_ALWAYS_INLINE nsTime nsTime::operator-(const nsTime& other) const
{
  return nsTime(m_fTime - other.m_fTime);
}

constexpr NS_ALWAYS_INLINE nsTime nsTime::operator+(const nsTime& other) const
{
  return nsTime(m_fTime + other.m_fTime);
}

constexpr NS_ALWAYS_INLINE nsTime operator*(nsTime t, double f)
{
  return nsTime::MakeFromSeconds(t.GetSeconds() * f);
}

constexpr NS_ALWAYS_INLINE nsTime operator*(double f, nsTime t)
{
  return nsTime::MakeFromSeconds(t.GetSeconds() * f);
}

constexpr NS_ALWAYS_INLINE nsTime operator*(nsTime f, nsTime t)
{
  return nsTime::MakeFromSeconds(t.GetSeconds() * f.GetSeconds());
}

constexpr NS_ALWAYS_INLINE nsTime operator/(nsTime t, double f)
{
  return nsTime::MakeFromSeconds(t.GetSeconds() / f);
}

constexpr NS_ALWAYS_INLINE nsTime operator/(double f, nsTime t)
{
  return nsTime::MakeFromSeconds(f / t.GetSeconds());
}

constexpr NS_ALWAYS_INLINE nsTime operator/(nsTime f, nsTime t)
{
  return nsTime::MakeFromSeconds(f.GetSeconds() / t.GetSeconds());
}

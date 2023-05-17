#pragma once

#include <Foundation/Basics.h>

constexpr WD_ALWAYS_INLINE wdTime::wdTime(double fTime)
  : m_fTime(fTime)
{
}

WD_ALWAYS_INLINE void wdTime::SetZero()
{
  m_fTime = 0.0;
}

constexpr WD_ALWAYS_INLINE float wdTime::AsFloatInSeconds() const
{
  return static_cast<float>(m_fTime);
}

constexpr WD_ALWAYS_INLINE double wdTime::GetNanoseconds() const
{
  return m_fTime * 1000000000.0;
}

constexpr WD_ALWAYS_INLINE double wdTime::GetMicroseconds() const
{
  return m_fTime * 1000000.0;
}

constexpr WD_ALWAYS_INLINE double wdTime::GetMilliseconds() const
{
  return m_fTime * 1000.0;
}

constexpr WD_ALWAYS_INLINE double wdTime::GetSeconds() const
{
  return m_fTime;
}

constexpr WD_ALWAYS_INLINE double wdTime::GetMinutes() const
{
  return m_fTime / 60.0;
}

constexpr WD_ALWAYS_INLINE double wdTime::GetHours() const
{
  return m_fTime / (60.0 * 60.0);
}

constexpr WD_ALWAYS_INLINE void wdTime::operator-=(const wdTime& other)
{
  m_fTime -= other.m_fTime;
}

constexpr WD_ALWAYS_INLINE void wdTime::operator+=(const wdTime& other)
{
  m_fTime += other.m_fTime;
}

constexpr WD_ALWAYS_INLINE void wdTime::operator*=(double fFactor)
{
  m_fTime *= fFactor;
}

constexpr WD_ALWAYS_INLINE void wdTime::operator/=(double fFactor)
{
  m_fTime /= fFactor;
}

constexpr WD_ALWAYS_INLINE wdTime wdTime::operator-() const
{
  return wdTime(-m_fTime);
}

constexpr WD_ALWAYS_INLINE wdTime wdTime::operator-(const wdTime& other) const
{
  return wdTime(m_fTime - other.m_fTime);
}

constexpr WD_ALWAYS_INLINE wdTime wdTime::operator+(const wdTime& other) const
{
  return wdTime(m_fTime + other.m_fTime);
}

constexpr WD_ALWAYS_INLINE wdTime operator*(wdTime t, double f)
{
  return wdTime::Seconds(t.GetSeconds() * f);
}

constexpr WD_ALWAYS_INLINE wdTime operator*(double f, wdTime t)
{
  return wdTime::Seconds(t.GetSeconds() * f);
}

constexpr WD_ALWAYS_INLINE wdTime operator*(wdTime f, wdTime t)
{
  return wdTime::Seconds(t.GetSeconds() * f.GetSeconds());
}

constexpr WD_ALWAYS_INLINE wdTime operator/(wdTime t, double f)
{
  return wdTime::Seconds(t.GetSeconds() / f);
}

constexpr WD_ALWAYS_INLINE wdTime operator/(double f, wdTime t)
{
  return wdTime::Seconds(f / t.GetSeconds());
}

constexpr WD_ALWAYS_INLINE wdTime operator/(wdTime f, wdTime t)
{
  return wdTime::Seconds(f.GetSeconds() / t.GetSeconds());
}

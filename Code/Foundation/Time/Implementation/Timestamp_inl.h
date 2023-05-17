#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Math/Math.h>

static const wdInt64 WD_INVALID_TIME_STAMP = 0x7FFFFFFFFFFFFFFFLL;

inline wdTimestamp::wdTimestamp()
{
  Invalidate();
}

inline wdTimestamp::wdTimestamp(wdInt64 iTimeValue, wdSIUnitOfTime::Enum unitOfTime)
{
  SetInt64(iTimeValue, unitOfTime);
}

inline void wdTimestamp::Invalidate()
{
  m_iTimestamp = WD_INVALID_TIME_STAMP;
}

inline bool wdTimestamp::IsValid() const
{
  return m_iTimestamp != WD_INVALID_TIME_STAMP;
}

inline void wdTimestamp::operator+=(const wdTime& timeSpan)
{
  WD_ASSERT_DEBUG(IsValid(), "Arithmetics on invalid time stamps are not allowed!");
  m_iTimestamp += (wdInt64)timeSpan.GetMicroseconds();
}

inline void wdTimestamp::operator-=(const wdTime& timeSpan)
{
  WD_ASSERT_DEBUG(IsValid(), "Arithmetics on invalid time stamps are not allowed!");
  m_iTimestamp -= (wdInt64)timeSpan.GetMicroseconds();
}

inline const wdTime wdTimestamp::operator-(const wdTimestamp& other) const
{
  WD_ASSERT_DEBUG(IsValid(), "Arithmetics on invalid time stamps are not allowed!");
  WD_ASSERT_DEBUG(other.IsValid(), "Arithmetics on invalid time stamps are not allowed!");
  return wdTime::Microseconds((double)(m_iTimestamp - other.m_iTimestamp));
}

inline const wdTimestamp wdTimestamp::operator+(const wdTime& timeSpan) const
{
  WD_ASSERT_DEBUG(IsValid(), "Arithmetics on invalid time stamps are not allowed!");
  return wdTimestamp(m_iTimestamp + (wdInt64)timeSpan.GetMicroseconds(), wdSIUnitOfTime::Microsecond);
}

inline const wdTimestamp wdTimestamp::operator-(const wdTime& timeSpan) const
{
  WD_ASSERT_DEBUG(IsValid(), "Arithmetics on invalid time stamps are not allowed!");
  return wdTimestamp(m_iTimestamp - (wdInt64)timeSpan.GetMicroseconds(), wdSIUnitOfTime::Microsecond);
}

inline const wdTimestamp operator+(const wdTime& timeSpan, const wdTimestamp& timestamp)
{
  WD_ASSERT_DEBUG(timestamp.IsValid(), "Arithmetics on invalid time stamps are not allowed!");
  return wdTimestamp(timestamp.GetInt64(wdSIUnitOfTime::Microsecond) + (wdInt64)timeSpan.GetMicroseconds(), wdSIUnitOfTime::Microsecond);
}



inline wdUInt32 wdDateTime::GetYear() const
{
  return m_iYear;
}

inline void wdDateTime::SetYear(wdInt16 iYear)
{
  m_iYear = iYear;
}

inline wdUInt8 wdDateTime::GetMonth() const
{
  return m_uiMonth;
}

inline void wdDateTime::SetMonth(wdUInt8 uiMonth)
{
  m_uiMonth = wdMath::Clamp<wdUInt8>(uiMonth, 1, 12);
}

inline wdUInt8 wdDateTime::GetDay() const
{
  return m_uiDay;
}

inline void wdDateTime::SetDay(wdUInt8 uiDay)
{
  m_uiDay = wdMath::Clamp<wdUInt8>(uiDay, 1u, 31u);
}

inline wdUInt8 wdDateTime::GetDayOfWeek() const
{
  return m_uiDayOfWeek;
}

inline void wdDateTime::SetDayOfWeek(wdUInt8 uiDayOfWeek)
{
  m_uiDayOfWeek = wdMath::Clamp<wdUInt8>(uiDayOfWeek, 0u, 6u);
}

inline wdUInt8 wdDateTime::GetHour() const
{
  return m_uiHour;
}

inline void wdDateTime::SetHour(wdUInt8 uiHour)
{
  m_uiHour = wdMath::Clamp<wdUInt8>(uiHour, 0u, 23u);
}

inline wdUInt8 wdDateTime::GetMinute() const
{
  return m_uiMinute;
}

inline void wdDateTime::SetMinute(wdUInt8 uiMinute)
{
  m_uiMinute = wdMath::Clamp<wdUInt8>(uiMinute, 0u, 59u);
}

inline wdUInt8 wdDateTime::GetSecond() const
{
  return m_uiSecond;
}

inline void wdDateTime::SetSecond(wdUInt8 uiSecond)
{
  m_uiSecond = wdMath::Clamp<wdUInt8>(uiSecond, 0u, 59u);
}

inline wdUInt32 wdDateTime::GetMicroseconds() const
{
  return m_uiMicroseconds;
}

inline void wdDateTime::SetMicroseconds(wdUInt32 uiMicroSeconds)
{
  m_uiMicroseconds = wdMath::Clamp<wdUInt32>(uiMicroSeconds, 0u, 999999u);
}

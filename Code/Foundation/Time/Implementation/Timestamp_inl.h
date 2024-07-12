#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Math/Math.h>

inline nsTimestamp::nsTimestamp() = default;

inline bool nsTimestamp::IsValid() const
{
  return m_iTimestamp != NS_INVALID_TIME_STAMP;
}

inline void nsTimestamp::operator+=(const nsTime& timeSpan)
{
  NS_ASSERT_DEBUG(IsValid(), "Arithmetics on invalid time stamps are not allowed!");
  m_iTimestamp += (nsInt64)timeSpan.GetMicroseconds();
}

inline void nsTimestamp::operator-=(const nsTime& timeSpan)
{
  NS_ASSERT_DEBUG(IsValid(), "Arithmetics on invalid time stamps are not allowed!");
  m_iTimestamp -= (nsInt64)timeSpan.GetMicroseconds();
}

inline const nsTime nsTimestamp::operator-(const nsTimestamp& other) const
{
  NS_ASSERT_DEBUG(IsValid(), "Arithmetics on invalid time stamps are not allowed!");
  NS_ASSERT_DEBUG(other.IsValid(), "Arithmetics on invalid time stamps are not allowed!");
  return nsTime::MakeFromMicroseconds((double)(m_iTimestamp - other.m_iTimestamp));
}

inline const nsTimestamp nsTimestamp::operator+(const nsTime& timeSpan) const
{
  NS_ASSERT_DEBUG(IsValid(), "Arithmetics on invalid time stamps are not allowed!");
  return nsTimestamp::MakeFromInt(m_iTimestamp + (nsInt64)timeSpan.GetMicroseconds(), nsSIUnitOfTime::Microsecond);
}

inline const nsTimestamp nsTimestamp::operator-(const nsTime& timeSpan) const
{
  NS_ASSERT_DEBUG(IsValid(), "Arithmetics on invalid time stamps are not allowed!");
  return nsTimestamp::MakeFromInt(m_iTimestamp - (nsInt64)timeSpan.GetMicroseconds(), nsSIUnitOfTime::Microsecond);
}

inline const nsTimestamp operator+(const nsTime& timeSpan, const nsTimestamp& timestamp)
{
  NS_ASSERT_DEBUG(timestamp.IsValid(), "Arithmetics on invalid time stamps are not allowed!");
  return nsTimestamp::MakeFromInt(timestamp.GetInt64(nsSIUnitOfTime::Microsecond) + (nsInt64)timeSpan.GetMicroseconds(), nsSIUnitOfTime::Microsecond);
}



inline nsUInt32 nsDateTime::GetYear() const
{
  return m_iYear;
}

inline void nsDateTime::SetYear(nsInt16 iYear)
{
  m_iYear = iYear;
}

inline nsUInt8 nsDateTime::GetMonth() const
{
  return m_uiMonth;
}

inline void nsDateTime::SetMonth(nsUInt8 uiMonth)
{
  NS_ASSERT_DEBUG(uiMonth >= 1 && uiMonth <= 12, "Invalid month value");
  m_uiMonth = uiMonth;
}

inline nsUInt8 nsDateTime::GetDay() const
{
  return m_uiDay;
}

inline void nsDateTime::SetDay(nsUInt8 uiDay)
{
  NS_ASSERT_DEBUG(uiDay >= 1 && uiDay <= 31, "Invalid day value");
  m_uiDay = uiDay;
}

inline nsUInt8 nsDateTime::GetDayOfWeek() const
{
  return m_uiDayOfWeek;
}

inline void nsDateTime::SetDayOfWeek(nsUInt8 uiDayOfWeek)
{
  NS_ASSERT_DEBUG(uiDayOfWeek <= 6, "Invalid day of week value");
  m_uiDayOfWeek = uiDayOfWeek;
}

inline nsUInt8 nsDateTime::GetHour() const
{
  return m_uiHour;
}

inline void nsDateTime::SetHour(nsUInt8 uiHour)
{
  NS_ASSERT_DEBUG(uiHour <= 23, "Invalid hour value");
  m_uiHour = uiHour;
}

inline nsUInt8 nsDateTime::GetMinute() const
{
  return m_uiMinute;
}

inline void nsDateTime::SetMinute(nsUInt8 uiMinute)
{
  NS_ASSERT_DEBUG(uiMinute <= 59, "Invalid minute value");
  m_uiMinute = uiMinute;
}

inline nsUInt8 nsDateTime::GetSecond() const
{
  return m_uiSecond;
}

inline void nsDateTime::SetSecond(nsUInt8 uiSecond)
{
  NS_ASSERT_DEBUG(uiSecond <= 59, "Invalid second value");
  m_uiSecond = uiSecond;
}

inline nsUInt32 nsDateTime::GetMicroseconds() const
{
  return m_uiMicroseconds;
}

inline void nsDateTime::SetMicroseconds(nsUInt32 uiMicroSeconds)
{
  NS_ASSERT_DEBUG(uiMicroSeconds <= 999999u, "Invalid micro-second value");
  m_uiMicroseconds = uiMicroSeconds;
}

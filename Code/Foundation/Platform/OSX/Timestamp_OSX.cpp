#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_OSX)

#  include <Foundation/Time/Timestamp.h>

#  include <Foundation/Basics/Platform/OSX/ScopedCFRef.h>

#  include <CoreFoundation/CFCalendar.h>
#  include <CoreFoundation/CoreFoundation.h>

const nsTimestamp nsTimestamp::CurrentTimestamp()
{
  timeval currentTime;
  gettimeofday(&currentTime, nullptr);

  return nsTimestamp::MakeFromInt(currentTime.tv_sec * 1000000LL + currentTime.tv_usec, nsSIUnitOfTime::Microsecond);
}

const nsTimestamp nsDateTime::GetTimestamp() const
{
  nsScopedCFRef<CFTimeZoneRef> timnsone(CFTimeZoneCreateWithTimeIntervalFromGMT(kCFAllocatorDefault, 0));
  nsScopedCFRef<CFCalendarRef> calendar(CFCalendarCreateWithIdentifier(kCFAllocatorSystemDefault, kCFGregorianCalendar));
  CFCalendarSetTimeZone(calendar, timnsone);

  int year = m_iYear, month = m_uiMonth, day = m_uiDay, hour = m_uiHour, minute = m_uiMinute, second = m_uiSecond;

  // Validate the year against the valid range of the calendar
  {
    auto yearMin = CFCalendarGetMinimumRangeOfUnit(calendar, kCFCalendarUnitYear), yearMax = CFCalendarGetMaximumRangeOfUnit(calendar, kCFCalendarUnitYear);

    if (year < yearMin.location || year > yearMax.length)
    {
      return nsTimestamp::MakeInvalid();
    }
  }

  // Validate the month against the valid range of the calendar
  {
    auto monthMin = CFCalendarGetMinimumRangeOfUnit(calendar, kCFCalendarUnitMonth), monthMax = CFCalendarGetMaximumRangeOfUnit(calendar, kCFCalendarUnitMonth);

    if (month < monthMin.location || month > monthMax.length)
    {
      return nsTimestamp::MakeInvalid();
    }
  }

  // Validate the day against the valid range of the calendar
  {
    auto dayMin = CFCalendarGetMinimumRangeOfUnit(calendar, kCFCalendarUnitDay), dayMax = CFCalendarGetMaximumRangeOfUnit(calendar, kCFCalendarUnitDay);

    if (day < dayMin.location || day > dayMax.length)
    {
      return nsTimestamp::MakeInvalid();
    }
  }

  CFAbsoluteTime absTime;
  if (CFCalendarComposeAbsoluteTime(calendar, &absTime, "yMdHms", year, month, day, hour, minute, second) == FALSE)
  {
    return nsTimestamp::MakeInvalid();
  }

  return nsTimestamp::MakeFromInt(static_cast<nsInt64>((absTime + kCFAbsoluteTimeIntervalSince1970) * 1000000.0), nsSIUnitOfTime::Microsecond);
}

nsResult nsDateTime::SetFromTimestamp(nsTimestamp timestamp)
{
  // Round the microseconds to the full second so that we can reconstruct the right date / time afterwards
  nsInt64 us = timestamp.GetInt64(nsSIUnitOfTime::Microsecond);
  nsInt64 microseconds = us % (1000 * 1000);

  CFAbsoluteTime at = (static_cast<CFAbsoluteTime>((us - microseconds) / 1000000.0)) - kCFAbsoluteTimeIntervalSince1970;

  nsScopedCFRef<CFTimeZoneRef> timnsone(CFTimeZoneCreateWithTimeIntervalFromGMT(kCFAllocatorDefault, 0));
  nsScopedCFRef<CFCalendarRef> calendar(CFCalendarCreateWithIdentifier(kCFAllocatorSystemDefault, kCFGregorianCalendar));
  CFCalendarSetTimeZone(calendar, timnsone);

  int year, month, day, dayOfWeek, hour, minute, second;

  if (CFCalendarDecomposeAbsoluteTime(calendar, at, "yMdHmsE", &year, &month, &day, &hour, &minute, &second, &dayOfWeek) == FALSE)
  {
    return NS_FAILURE;
  }

  m_iYear = (nsInt16)year;
  m_uiMonth = (nsUInt8)month;
  m_uiDay = (nsUInt8)day;
  m_uiDayOfWeek = (nsUInt8)(dayOfWeek - 1);
  m_uiHour = (nsUInt8)hour;
  m_uiMinute = (nsUInt8)minute;
  m_uiSecond = (nsUInt8)second;
  m_uiMicroseconds = (nsUInt32)microseconds;
  return NS_SUCCESS;
}

#endif

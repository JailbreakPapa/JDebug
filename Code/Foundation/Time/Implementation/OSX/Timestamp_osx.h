#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <Foundation/Basics/Platform/OSX/ScopedCFRef.h>

#include <CoreFoundation/CFCalendar.h>
#include <CoreFoundation/CoreFoundation.h>

const wdTimestamp wdTimestamp::CurrentTimestamp()
{
  timeval currentTime;
  gettimeofday(&currentTime, nullptr);

  return wdTimestamp(currentTime.tv_sec * 1000000LL + currentTime.tv_usec, wdSIUnitOfTime::Microsecond);
}

const wdTimestamp wdDateTime::GetTimestamp() const
{
  wdScopedCFRef<CFTimeZoneRef> timwdone(CFTimeZoneCreateWithTimeIntervalFromGMT(kCFAllocatorDefault, 0));
  wdScopedCFRef<CFCalendarRef> calendar(CFCalendarCreateWithIdentifier(kCFAllocatorSystemDefault, kCFGregorianCalendar));
  CFCalendarSetTimeZone(calendar, timwdone);

  int year = m_iYear, month = m_uiMonth, day = m_uiDay, hour = m_uiHour, minute = m_uiMinute, second = m_uiSecond;

  // Validate the year against the valid range of the calendar
  {
    auto yearMin = CFCalendarGetMinimumRangeOfUnit(calendar, kCFCalendarUnitYear), yearMax = CFCalendarGetMaximumRangeOfUnit(calendar, kCFCalendarUnitYear);

    if (year < yearMin.location || year > yearMax.length)
    {
      return wdTimestamp();
    }
  }

  // Validate the month against the valid range of the calendar
  {
    auto monthMin = CFCalendarGetMinimumRangeOfUnit(calendar, kCFCalendarUnitMonth), monthMax = CFCalendarGetMaximumRangeOfUnit(calendar, kCFCalendarUnitMonth);

    if (month < monthMin.location || month > monthMax.length)
    {
      return wdTimestamp();
    }
  }

  // Validate the day against the valid range of the calendar
  {
    auto dayMin = CFCalendarGetMinimumRangeOfUnit(calendar, kCFCalendarUnitDay), dayMax = CFCalendarGetMaximumRangeOfUnit(calendar, kCFCalendarUnitDay);

    if (day < dayMin.location || day > dayMax.length)
    {
      return wdTimestamp();
    }
  }

  CFAbsoluteTime absTime;
  if (CFCalendarComposeAbsoluteTime(calendar, &absTime, "yMdHms", year, month, day, hour, minute, second) == FALSE)
  {
    return wdTimestamp();
  }

  return wdTimestamp(static_cast<wdInt64>((absTime + kCFAbsoluteTimeIntervalSince1970) * 1000000.0), wdSIUnitOfTime::Microsecond);
}

bool wdDateTime::SetTimestamp(wdTimestamp timestamp)
{
  // Round the microseconds to the full second so that we can reconstruct the right date / time afterwards
  wdInt64 us = timestamp.GetInt64(wdSIUnitOfTime::Microsecond);
  wdInt64 microseconds = us % (1000 * 1000);

  CFAbsoluteTime at = (static_cast<CFAbsoluteTime>((us - microseconds) / 1000000.0)) - kCFAbsoluteTimeIntervalSince1970;

  wdScopedCFRef<CFTimeZoneRef> timwdone(CFTimeZoneCreateWithTimeIntervalFromGMT(kCFAllocatorDefault, 0));
  wdScopedCFRef<CFCalendarRef> calendar(CFCalendarCreateWithIdentifier(kCFAllocatorSystemDefault, kCFGregorianCalendar));
  CFCalendarSetTimeZone(calendar, timwdone);

  int year, month, day, dayOfWeek, hour, minute, second;

  if (CFCalendarDecomposeAbsoluteTime(calendar, at, "yMdHmsE", &year, &month, &day, &hour, &minute, &second, &dayOfWeek) == FALSE)
  {
    return false;
  }

  m_iYear = (wdInt16)year;
  m_uiMonth = (wdUInt8)month;
  m_uiDay = (wdUInt8)day;
  m_uiDayOfWeek = (wdUInt8)(dayOfWeek - 1);
  m_uiHour = (wdUInt8)hour;
  m_uiMinute = (wdUInt8)minute;
  m_uiSecond = (wdUInt8)second;
  m_uiMicroseconds = (wdUInt32)microseconds;
  return true;
}

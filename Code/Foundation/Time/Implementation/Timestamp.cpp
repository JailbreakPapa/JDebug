#include <Foundation/FoundationPCH.h>

#include <Foundation/Time/Timestamp.h>

// clang-format off
WD_BEGIN_STATIC_REFLECTED_TYPE(wdTimestamp, wdNoBase, 1, wdRTTINoAllocator)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("time", m_iTimestamp),
  }
  WD_END_PROPERTIES;
}
WD_END_STATIC_REFLECTED_TYPE;
// clang-format on

wdInt64 wdTimestamp::GetInt64(wdSIUnitOfTime::Enum unitOfTime) const
{
  WD_ASSERT_DEV(IsValid(), "Can't retrieve timestamp of invalid values!");
  WD_ASSERT_DEV(unitOfTime >= wdSIUnitOfTime::Nanosecond && unitOfTime <= wdSIUnitOfTime::Second, "Invalid wdSIUnitOfTime value ({0})", unitOfTime);

  switch (unitOfTime)
  {
    case wdSIUnitOfTime::Nanosecond:
      return m_iTimestamp * 1000LL;
    case wdSIUnitOfTime::Microsecond:
      return m_iTimestamp;
    case wdSIUnitOfTime::Millisecond:
      return m_iTimestamp / 1000LL;
    case wdSIUnitOfTime::Second:
      return m_iTimestamp / 1000000LL;
  }
  return WD_INVALID_TIME_STAMP;
}

void wdTimestamp::SetInt64(wdInt64 iTimeValue, wdSIUnitOfTime::Enum unitOfTime)
{
  WD_ASSERT_DEV(unitOfTime >= wdSIUnitOfTime::Nanosecond && unitOfTime <= wdSIUnitOfTime::Second, "Invalid wdSIUnitOfTime value ({0})", unitOfTime);

  switch (unitOfTime)
  {
    case wdSIUnitOfTime::Nanosecond:
      m_iTimestamp = iTimeValue / 1000LL;
      break;
    case wdSIUnitOfTime::Microsecond:
      m_iTimestamp = iTimeValue;
      break;
    case wdSIUnitOfTime::Millisecond:
      m_iTimestamp = iTimeValue * 1000LL;
      break;
    case wdSIUnitOfTime::Second:
      m_iTimestamp = iTimeValue * 1000000LL;
      break;
  }
}

bool wdTimestamp::Compare(const wdTimestamp& rhs, CompareMode::Enum mode) const
{
  switch (mode)
  {
    case CompareMode::FileTimeEqual:
      // Resolution of seconds until all platforms are tuned to milliseconds.
      return (m_iTimestamp / 1000000LL) == (rhs.m_iTimestamp / 1000000LL);

    case CompareMode::Identical:
      return m_iTimestamp == rhs.m_iTimestamp;

    case CompareMode::Newer:
      // Resolution of seconds until all platforms are tuned to milliseconds.
      return (m_iTimestamp / 1000000LL) > (rhs.m_iTimestamp / 1000000LL);
  }

  WD_ASSERT_NOT_IMPLEMENTED;
  return false;
}

wdDateTime::wdDateTime()
  : m_uiMicroseconds(0)
  , m_iYear(0)
  , m_uiMonth(0)
  , m_uiDay(0)
  , m_uiDayOfWeek(0)
  , m_uiHour(0)
  , m_uiMinute(0)
  , m_uiSecond(0)
{
}

wdDateTime::wdDateTime(wdTimestamp timestamp)
  : wdDateTime()
{
  SetTimestamp(timestamp);
}

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdDateTime& arg)
{
  wdStringUtils::snprintf(szTmp, uiLength, "%04u-%02u-%02u_%02u-%02u-%02u-%03u", arg.GetYear(), arg.GetMonth(), arg.GetDay(), arg.GetHour(),
    arg.GetMinute(), arg.GetSecond(), arg.GetMicroseconds() / 1000);

  return szTmp;
}

namespace
{
  // This implementation chooses a 3-character-long short name for each of the twelve months
  // for consistency reasons. Mind, that other, potentially more widely-spread stylist
  // alternatives may exist.
  const char* GetMonthShortName(const wdDateTime& dateTime)
  {
    switch (dateTime.GetMonth())
    {
      case 1:
        return "Jan";
      case 2:
        return "Feb";
      case 3:
        return "Mar";
      case 4:
        return "Apr";
      case 5:
        return "May";
      case 6:
        return "Jun";
      case 7:
        return "Jul";
      case 8:
        return "Aug";
      case 9:
        return "Sep";
      case 10:
        return "Oct";
      case 11:
        return "Nov";
      case 12:
        return "Dec";
      default:
        WD_ASSERT_DEV(false, "Unknown month.");
        return "Unknown Month";
    }
  }

  // This implementation chooses a 3-character-long short name for each of the seven days
  // of the week for consistency reasons. Mind, that other, potentially more widely-spread
  // stylistic alternatives may exist.
  const char* GetDayOfWeekShortName(const wdDateTime& dateTime)
  {
    switch (dateTime.GetDayOfWeek())
    {
      case 0:
        return "Sun";
      case 1:
        return "Mon";
      case 2:
        return "Tue";
      case 3:
        return "Wed";
      case 4:
        return "Thu";
      case 5:
        return "Fri";
      case 6:
        return "Sat";
      default:
        WD_ASSERT_DEV(false, "Unknown day of week.");
        return "Unknown Day of Week";
    }
  }
} // namespace

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdArgDateTime& arg)
{
  const wdDateTime& dateTime = arg.m_Value;

  wdUInt32 offset = 0;

  if ((arg.m_uiFormattingFlags & wdArgDateTime::ShowDate) == wdArgDateTime::ShowDate)
  {
    if ((arg.m_uiFormattingFlags & wdArgDateTime::TextualDate) == wdArgDateTime::TextualDate)
    {
      offset += wdStringUtils::snprintf(
        szTmp + offset, uiLength - offset, "%04u %s %02u", dateTime.GetYear(), ::GetMonthShortName(dateTime), dateTime.GetDay());
    }
    else
    {
      offset +=
        wdStringUtils::snprintf(szTmp + offset, uiLength - offset, "%04u-%02u-%02u", dateTime.GetYear(), dateTime.GetMonth(), dateTime.GetDay());
    }
  }

  if ((arg.m_uiFormattingFlags & wdArgDateTime::ShowWeekday) == wdArgDateTime::ShowWeekday)
  {
    // add a space
    if (offset != 0)
    {
      szTmp[offset] = ' ';
      ++offset;
      szTmp[offset] = '\0';
    }

    offset += wdStringUtils::snprintf(szTmp + offset, uiLength - offset, "(%s)", ::GetDayOfWeekShortName(dateTime));
  }

  if ((arg.m_uiFormattingFlags & wdArgDateTime::ShowTime) == wdArgDateTime::ShowTime)
  {
    // add a space
    if (offset != 0)
    {
      szTmp[offset] = ' ';
      szTmp[offset + 1] = '-';
      szTmp[offset + 2] = ' ';
      szTmp[offset + 3] = '\0';
      offset += 3;
    }

    offset += wdStringUtils::snprintf(szTmp + offset, uiLength - offset, "%02u:%02u", dateTime.GetHour(), dateTime.GetMinute());

    if ((arg.m_uiFormattingFlags & wdArgDateTime::ShowSeconds) == wdArgDateTime::ShowSeconds)
    {
      offset += wdStringUtils::snprintf(szTmp + offset, uiLength - offset, ":%02u", dateTime.GetSecond());
    }

    if ((arg.m_uiFormattingFlags & wdArgDateTime::ShowMilliseconds) == wdArgDateTime::ShowMilliseconds)
    {
      offset += wdStringUtils::snprintf(szTmp + offset, uiLength - offset, ".%03u", dateTime.GetMicroseconds() / 1000);
    }

    if ((arg.m_uiFormattingFlags & wdArgDateTime::ShowTimeZone) == wdArgDateTime::ShowTimeZone)
    {
      offset += wdStringUtils::snprintf(szTmp + offset, uiLength - offset, " (UTC)");
    }
  }

  return szTmp;
}

// Include inline file
#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/Time/Implementation/Win/Timestamp_win.h>
#elif WD_ENABLED(WD_PLATFORM_OSX)
#  include <Foundation/Time/Implementation/OSX/Timestamp_osx.h>
#elif WD_ENABLED(WD_PLATFORM_ANDROID)
#  include <Foundation/Time/Implementation/Android/Timestamp_android.h>
#elif WD_ENABLED(WD_PLATFORM_LINUX)
#  include <Foundation/Time/Implementation/Posix/Timestamp_posix.h>
#else
#  error "Time functions are not implemented on current platform"
#endif

WD_STATICLINK_FILE(Foundation, Foundation_Time_Implementation_Timestamp);

#include <Foundation/FoundationPCH.h>

#include <Foundation/Time/Timestamp.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsTimestamp, nsNoBase, 1, nsRTTINoAllocator)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("time", m_iTimestamp),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

nsInt64 nsTimestamp::GetInt64(nsSIUnitOfTime::Enum unitOfTime) const
{
  NS_ASSERT_DEV(IsValid(), "Can't retrieve timestamp of invalid values!");
  NS_ASSERT_DEV(unitOfTime >= nsSIUnitOfTime::Nanosecond && unitOfTime <= nsSIUnitOfTime::Second, "Invalid nsSIUnitOfTime value ({0})", unitOfTime);

  switch (unitOfTime)
  {
    case nsSIUnitOfTime::Nanosecond:
      return m_iTimestamp * 1000LL;
    case nsSIUnitOfTime::Microsecond:
      return m_iTimestamp;
    case nsSIUnitOfTime::Millisecond:
      return m_iTimestamp / 1000LL;
    case nsSIUnitOfTime::Second:
      return m_iTimestamp / 1000000LL;
  }
  return NS_INVALID_TIME_STAMP;
}

nsTimestamp nsTimestamp::MakeFromInt(nsInt64 iTimeValue, nsSIUnitOfTime::Enum unitOfTime)
{
  NS_ASSERT_DEV(unitOfTime >= nsSIUnitOfTime::Nanosecond && unitOfTime <= nsSIUnitOfTime::Second, "Invalid nsSIUnitOfTime value ({0})", unitOfTime);

  nsTimestamp ts;

  switch (unitOfTime)
  {
    case nsSIUnitOfTime::Nanosecond:
      ts.m_iTimestamp = iTimeValue / 1000LL;
      break;
    case nsSIUnitOfTime::Microsecond:
      ts.m_iTimestamp = iTimeValue;
      break;
    case nsSIUnitOfTime::Millisecond:
      ts.m_iTimestamp = iTimeValue * 1000LL;
      break;
    case nsSIUnitOfTime::Second:
      ts.m_iTimestamp = iTimeValue * 1000000LL;
      break;

      NS_DEFAULT_CASE_NOT_IMPLEMENTED;
  }

  return ts;
}

bool nsTimestamp::Compare(const nsTimestamp& rhs, CompareMode::Enum mode) const
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

  NS_ASSERT_NOT_IMPLEMENTED;
  return false;
}

nsDateTime::nsDateTime() = default;
nsDateTime::~nsDateTime() = default;

nsDateTime nsDateTime::MakeFromTimestamp(nsTimestamp timestamp)
{
  nsDateTime res;
  res.SetFromTimestamp(timestamp).AssertSuccess("Invalid timestamp");
  return res;
}

bool nsDateTime::IsValid() const
{
  if (m_uiMonth <= 0 || m_uiMonth > 12)
    return false;

  if (m_uiDay <= 0 || m_uiDay > 31)
    return false;

  if (m_uiDayOfWeek > 6)
    return false;

  if (m_uiHour > 23)
    return false;

  if (m_uiMinute > 59)
    return false;

  if (m_uiSecond > 59)
    return false;

  return true;
}

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsDateTime& arg)
{
  nsStringUtils::snprintf(szTmp, uiLength, "%04u-%02u-%02u_%02u-%02u-%02u-%03u", arg.GetYear(), arg.GetMonth(), arg.GetDay(), arg.GetHour(),
    arg.GetMinute(), arg.GetSecond(), arg.GetMicroseconds() / 1000);

  return szTmp;
}

namespace
{
  // This implementation chooses a 3-character-long short name for each of the twelve months
  // for consistency reasons. Mind, that other, potentially more widely-spread stylist
  // alternatives may exist.
  const char* GetMonthShortName(const nsDateTime& dateTime)
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
        NS_ASSERT_DEV(false, "Unknown month.");
        return "Unknown Month";
    }
  }

  // This implementation chooses a 3-character-long short name for each of the seven days
  // of the week for consistency reasons. Mind, that other, potentially more widely-spread
  // stylistic alternatives may exist.
  const char* GetDayOfWeekShortName(const nsDateTime& dateTime)
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
        NS_ASSERT_DEV(false, "Unknown day of week.");
        return "Unknown Day of Week";
    }
  }
} // namespace

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsArgDateTime& arg)
{
  const nsDateTime& dateTime = arg.m_Value;

  nsUInt32 offset = 0;

  if ((arg.m_uiFormattingFlags & nsArgDateTime::ShowDate) == nsArgDateTime::ShowDate)
  {
    if ((arg.m_uiFormattingFlags & nsArgDateTime::TextualDate) == nsArgDateTime::TextualDate)
    {
      offset += nsStringUtils::snprintf(
        szTmp + offset, uiLength - offset, "%04u %s %02u", dateTime.GetYear(), ::GetMonthShortName(dateTime), dateTime.GetDay());
    }
    else
    {
      offset +=
        nsStringUtils::snprintf(szTmp + offset, uiLength - offset, "%04u-%02u-%02u", dateTime.GetYear(), dateTime.GetMonth(), dateTime.GetDay());
    }
  }

  if ((arg.m_uiFormattingFlags & nsArgDateTime::ShowWeekday) == nsArgDateTime::ShowWeekday)
  {
    // add a space
    if (offset != 0)
    {
      szTmp[offset] = ' ';
      ++offset;
      szTmp[offset] = '\0';
    }

    offset += nsStringUtils::snprintf(szTmp + offset, uiLength - offset, "(%s)", ::GetDayOfWeekShortName(dateTime));
  }

  if ((arg.m_uiFormattingFlags & nsArgDateTime::ShowTime) == nsArgDateTime::ShowTime)
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

    offset += nsStringUtils::snprintf(szTmp + offset, uiLength - offset, "%02u:%02u", dateTime.GetHour(), dateTime.GetMinute());

    if ((arg.m_uiFormattingFlags & nsArgDateTime::ShowSeconds) == nsArgDateTime::ShowSeconds)
    {
      offset += nsStringUtils::snprintf(szTmp + offset, uiLength - offset, ":%02u", dateTime.GetSecond());
    }

    if ((arg.m_uiFormattingFlags & nsArgDateTime::ShowMilliseconds) == nsArgDateTime::ShowMilliseconds)
    {
      offset += nsStringUtils::snprintf(szTmp + offset, uiLength - offset, ".%03u", dateTime.GetMicroseconds() / 1000);
    }

    if ((arg.m_uiFormattingFlags & nsArgDateTime::ShowTimeZone) == nsArgDateTime::ShowTimeZone)
    {
      nsStringUtils::snprintf(szTmp + offset, uiLength - offset, " (UTC)");
    }
  }

  return szTmp;
}

NS_STATICLINK_FILE(Foundation, Foundation_Time_Implementation_Timestamp);

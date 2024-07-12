#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS)

#  include <Foundation/Time/Timestamp.h>

// Helper function to shift windows file time into Unix epoch (in microseconds).
nsInt64 FileTimeToEpoch(FILETIME fileTime)
{
  ULARGE_INTEGER currentTime;
  currentTime.LowPart = fileTime.dwLowDateTime;
  currentTime.HighPart = fileTime.dwHighDateTime;

  nsInt64 iTemp = currentTime.QuadPart / 10;
  iTemp -= 11644473600000000LL;
  return iTemp;
}

// Helper function to shift Unix epoch (in microseconds) into windows file time.
FILETIME EpochToFileTime(nsInt64 iFileTime)
{
  nsInt64 iTemp = iFileTime + 11644473600000000LL;
  iTemp *= 10;

  FILETIME fileTime;
  ULARGE_INTEGER currentTime;
  currentTime.QuadPart = iTemp;
  fileTime.dwLowDateTime = currentTime.LowPart;
  fileTime.dwHighDateTime = currentTime.HighPart;
  return fileTime;
}

const nsTimestamp nsTimestamp::CurrentTimestamp()
{
  FILETIME fileTime;
  GetSystemTimeAsFileTime(&fileTime);
  return nsTimestamp::MakeFromInt(FileTimeToEpoch(fileTime), nsSIUnitOfTime::Microsecond);
}

const nsTimestamp nsDateTime::GetTimestamp() const
{
  SYSTEMTIME st;
  FILETIME fileTime;
  memset(&st, 0, sizeof(SYSTEMTIME));
  st.wYear = (WORD)m_iYear;
  st.wMonth = m_uiMonth;
  st.wDay = m_uiDay;
  st.wDayOfWeek = m_uiDayOfWeek;
  st.wHour = m_uiHour;
  st.wMinute = m_uiMinute;
  st.wSecond = m_uiSecond;
  st.wMilliseconds = (WORD)(m_uiMicroseconds / 1000);
  BOOL res = SystemTimeToFileTime(&st, &fileTime);
  nsTimestamp timestamp;
  if (res != 0)
    timestamp = nsTimestamp::MakeFromInt(FileTimeToEpoch(fileTime), nsSIUnitOfTime::Microsecond);

  return timestamp;
}

nsResult nsDateTime::SetFromTimestamp(nsTimestamp timestamp)
{
  FILETIME fileTime = EpochToFileTime(timestamp.GetInt64(nsSIUnitOfTime::Microsecond));

  SYSTEMTIME st;
  BOOL res = FileTimeToSystemTime(&fileTime, &st);
  if (res == 0)
    return NS_FAILURE;

  m_iYear = (nsInt16)st.wYear;
  m_uiMonth = (nsUInt8)st.wMonth;
  m_uiDay = (nsUInt8)st.wDay;
  m_uiDayOfWeek = (nsUInt8)st.wDayOfWeek;
  m_uiHour = (nsUInt8)st.wHour;
  m_uiMinute = (nsUInt8)st.wMinute;
  m_uiSecond = (nsUInt8)st.wSecond;
  m_uiMicroseconds = nsUInt32(st.wMilliseconds * 1000);
  return NS_SUCCESS;
}

#endif

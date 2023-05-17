#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

// Helper function to shift windows file time into Unix epoch (in microseconds).
wdInt64 FileTimeToEpoch(FILETIME fileTime)
{
  ULARGE_INTEGER currentTime;
  currentTime.LowPart = fileTime.dwLowDateTime;
  currentTime.HighPart = fileTime.dwHighDateTime;

  wdInt64 iTemp = currentTime.QuadPart / 10;
  iTemp -= 11644473600000000LL;
  return iTemp;
}

// Helper function to shift Unix epoch (in microseconds) into windows file time.
FILETIME EpochToFileTime(wdInt64 iFileTime)
{
  wdInt64 iTemp = iFileTime + 11644473600000000LL;
  iTemp *= 10;

  FILETIME fileTime;
  ULARGE_INTEGER currentTime;
  currentTime.QuadPart = iTemp;
  fileTime.dwLowDateTime = currentTime.LowPart;
  fileTime.dwHighDateTime = currentTime.HighPart;
  return fileTime;
}

const wdTimestamp wdTimestamp::CurrentTimestamp()
{
  FILETIME fileTime;
  GetSystemTimeAsFileTime(&fileTime);
  return wdTimestamp(FileTimeToEpoch(fileTime), wdSIUnitOfTime::Microsecond);
}

const wdTimestamp wdDateTime::GetTimestamp() const
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
  wdTimestamp timestamp;
  if (res != 0)
    timestamp.SetInt64(FileTimeToEpoch(fileTime), wdSIUnitOfTime::Microsecond);

  return timestamp;
}

bool wdDateTime::SetTimestamp(wdTimestamp timestamp)
{
  FILETIME fileTime = EpochToFileTime(timestamp.GetInt64(wdSIUnitOfTime::Microsecond));

  SYSTEMTIME st;
  BOOL res = FileTimeToSystemTime(&fileTime, &st);
  if (res == 0)
    return false;

  m_iYear = (wdInt16)st.wYear;
  m_uiMonth = (wdUInt8)st.wMonth;
  m_uiDay = (wdUInt8)st.wDay;
  m_uiDayOfWeek = (wdUInt8)st.wDayOfWeek;
  m_uiHour = (wdUInt8)st.wHour;
  m_uiMinute = (wdUInt8)st.wMinute;
  m_uiSecond = (wdUInt8)st.wSecond;
  m_uiMicroseconds = wdUInt32(st.wMilliseconds * 1000);
  return true;
}

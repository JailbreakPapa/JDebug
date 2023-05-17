#include <Foundation/FoundationPCH.h>

#include <Foundation/Time/Stopwatch.h>

wdStopwatch::wdStopwatch()
{
  m_LastCheckpoint = wdTime::Now();

  StopAndReset();
  Resume();
}

void wdStopwatch::StopAndReset()
{
  m_TotalDuration.SetZero();
  m_bRunning = false;
}

void wdStopwatch::Resume()
{
  if (m_bRunning)
    return;

  m_bRunning = true;
  m_LastUpdate = wdTime::Now();
}

void wdStopwatch::Pause()
{
  if (!m_bRunning)
    return;

  m_bRunning = false;

  m_TotalDuration += wdTime::Now() - m_LastUpdate;
}

wdTime wdStopwatch::GetRunningTotal() const
{
  if (m_bRunning)
  {
    const wdTime tNow = wdTime::Now();

    m_TotalDuration += tNow - m_LastUpdate;
    m_LastUpdate = tNow;
  }

  return m_TotalDuration;
}

wdTime wdStopwatch::Checkpoint()
{
  const wdTime tNow = wdTime::Now();

  const wdTime tDiff = tNow - m_LastCheckpoint;
  m_LastCheckpoint = tNow;

  return tDiff;
}



WD_STATICLINK_FILE(Foundation, Foundation_Time_Implementation_Stopwatch);

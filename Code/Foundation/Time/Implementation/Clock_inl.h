#pragma once

#include <Foundation/Time/Clock.h>

inline void wdClock::SetClockName(const char* szName)
{
  m_sName = szName;
}

inline const char* wdClock::GetClockName() const
{
  return m_sName.GetData();
}

inline void wdClock::SetTimeStepSmoothing(wdTimeStepSmoothing* pSmoother)
{
  m_pTimeStepSmoother = pSmoother;

  if (m_pTimeStepSmoother)
    m_pTimeStepSmoother->Reset(this);
}

inline wdTimeStepSmoothing* wdClock::GetTimeStepSmoothing() const
{
  return m_pTimeStepSmoother;
}

inline void wdClock::SetPaused(bool bPaused)
{
  m_bPaused = bPaused;

  // when we enter a pause, inform the time step smoother to throw away his statistics
  if (bPaused && m_pTimeStepSmoother)
    m_pTimeStepSmoother->Reset(this);
}

inline bool wdClock::GetPaused() const
{
  return m_bPaused;
}

inline wdTime wdClock::GetFixedTimeStep() const
{
  return m_FixedTimeStep;
}

inline wdTime wdClock::GetAccumulatedTime() const
{
  return m_AccumulatedTime;
}

inline wdTime wdClock::GetTimeDiff() const
{
  return m_LastTimeDiff;
}

inline double wdClock::GetSpeed() const
{
  return m_fSpeed;
}

inline void wdClock::SetMinimumTimeStep(wdTime min)
{
  WD_ASSERT_DEV(min >= wdTime::Seconds(0.0), "Time flows in one direction only.");

  m_MinTimeStep = min;
}

inline void wdClock::SetMaximumTimeStep(wdTime max)
{
  WD_ASSERT_DEV(max >= wdTime::Seconds(0.0), "Time flows in one direction only.");

  m_MaxTimeStep = max;
}

inline wdTime wdClock::GetMinimumTimeStep() const
{
  return m_MinTimeStep;
}

inline wdTime wdClock::GetMaximumTimeStep() const
{
  return m_MaxTimeStep;
}

inline void wdClock::SetFixedTimeStep(wdTime diff)
{
  WD_ASSERT_DEV(m_FixedTimeStep.GetSeconds() >= 0.0, "Fixed Time Stepping cannot reverse time!");

  m_FixedTimeStep = diff;
}

inline void wdClock::SetSpeed(double fFactor)
{
  WD_ASSERT_DEV(fFactor >= 0.0, "Time cannot run backwards.");

  m_fSpeed = fFactor;
}

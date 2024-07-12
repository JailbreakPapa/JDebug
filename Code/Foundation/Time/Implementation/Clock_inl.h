#pragma once

#include <Foundation/Time/Clock.h>

inline void nsClock::SetClockName(nsStringView sName)
{
  m_sName = sName;
}

inline nsStringView nsClock::GetClockName() const
{
  return m_sName;
}

inline void nsClock::SetTimeStepSmoothing(nsTimeStepSmoothing* pSmoother)
{
  m_pTimeStepSmoother = pSmoother;

  if (m_pTimeStepSmoother)
    m_pTimeStepSmoother->Reset(this);
}

inline nsTimeStepSmoothing* nsClock::GetTimeStepSmoothing() const
{
  return m_pTimeStepSmoother;
}

inline void nsClock::SetPaused(bool bPaused)
{
  m_bPaused = bPaused;

  // when we enter a pause, inform the time step smoother to throw away his statistics
  if (bPaused && m_pTimeStepSmoother)
    m_pTimeStepSmoother->Reset(this);
}

inline bool nsClock::GetPaused() const
{
  return m_bPaused;
}

inline nsTime nsClock::GetFixedTimeStep() const
{
  return m_FixedTimeStep;
}

inline nsTime nsClock::GetAccumulatedTime() const
{
  return m_AccumulatedTime;
}

inline nsTime nsClock::GetTimeDiff() const
{
  return m_LastTimeDiff;
}

inline double nsClock::GetSpeed() const
{
  return m_fSpeed;
}

inline void nsClock::SetMinimumTimeStep(nsTime min)
{
  NS_ASSERT_DEV(min >= nsTime::MakeFromSeconds(0.0), "Time flows in one direction only.");

  m_MinTimeStep = min;
}

inline void nsClock::SetMaximumTimeStep(nsTime max)
{
  NS_ASSERT_DEV(max >= nsTime::MakeFromSeconds(0.0), "Time flows in one direction only.");

  m_MaxTimeStep = max;
}

inline nsTime nsClock::GetMinimumTimeStep() const
{
  return m_MinTimeStep;
}

inline nsTime nsClock::GetMaximumTimeStep() const
{
  return m_MaxTimeStep;
}

inline void nsClock::SetFixedTimeStep(nsTime diff)
{
  NS_ASSERT_DEV(m_FixedTimeStep.GetSeconds() >= 0.0, "Fixed Time Stepping cannot reverse time!");

  m_FixedTimeStep = diff;
}

inline void nsClock::SetSpeed(double fFactor)
{
  NS_ASSERT_DEV(fFactor >= 0.0, "Time cannot run backwards.");

  m_fSpeed = fFactor;
}

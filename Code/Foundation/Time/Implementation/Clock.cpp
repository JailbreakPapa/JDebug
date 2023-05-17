#include <Foundation/FoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/Time/Clock.h>

wdClock::Event wdClock::s_TimeEvents;
wdClock* wdClock::s_pGlobalClock = nullptr;

// clang-format off
WD_BEGIN_SUBSYSTEM_DECLARATION(Foundation, Clock)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Time"
  END_SUBSYSTEM_DEPENDENCIES

  ON_BASESYSTEMS_STARTUP
  {
    wdClock::s_pGlobalClock = new wdClock("Global");
  }

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

wdClock::wdClock(const char* szName)
{
  SetClockName(szName);

  Reset(true);
}

void wdClock::Reset(bool bEverything)
{
  if (bEverything)
  {
    m_pTimeStepSmoother = nullptr;
    m_MinTimeStep = wdTime::Seconds(0.001); // 1000 FPS
    m_MaxTimeStep = wdTime::Seconds(0.1);   //   10 FPS, many simulations will be instable at that rate already
    m_FixedTimeStep = wdTime::Seconds(0.0);
  }

  m_AccumulatedTime = wdTime::Seconds(0.0);
  m_fSpeed = 1.0;
  m_bPaused = false;

  // this is to prevent having a time difference of zero (which might not work with some code)
  // in case the next Update() call is done right after this
  m_LastTimeUpdate = wdTime::Now() - m_MinTimeStep;
  m_LastTimeDiff = m_MinTimeStep;

  if (m_pTimeStepSmoother)
    m_pTimeStepSmoother->Reset(this);
}

void wdClock::Update()
{
  const wdTime tNow = wdTime::Now();
  const wdTime tDiff = tNow - m_LastTimeUpdate;
  m_LastTimeUpdate = tNow;

  if (m_bPaused)
  {
    // no change during pause
    m_LastTimeDiff = wdTime::Seconds(0.0);
  }
  else if (m_FixedTimeStep > wdTime::Seconds(0.0))
  {
    // scale the time step by the speed factor
    m_LastTimeDiff = m_FixedTimeStep * m_fSpeed;
  }
  else
  {
    // in variable time step mode, apply the time step smoother, if available
    if (m_pTimeStepSmoother)
      m_LastTimeDiff = m_pTimeStepSmoother->GetSmoothedTimeStep(tDiff, this);
    else
    {
      // scale the time step by the speed factor
      // and make sure the time step does not leave the predetermined bounds
      m_LastTimeDiff = wdMath::Clamp(tDiff * m_fSpeed, m_MinTimeStep, m_MaxTimeStep);
    }
  }

  m_AccumulatedTime += m_LastTimeDiff;

  EventData ed;
  ed.m_szClockName = m_sName.GetData();
  ed.m_RawTimeStep = tDiff;
  ed.m_SmoothedTimeStep = m_LastTimeDiff;

  s_TimeEvents.Broadcast(ed);
}

void wdClock::SetAccumulatedTime(wdTime t)
{
  m_AccumulatedTime = t;

  // this is to prevent having a time difference of zero (which might not work with some code)
  // in case the next Update() call is done right after this
  m_LastTimeUpdate = wdTime::Now() - wdTime::Seconds(0.01);
  m_LastTimeDiff = wdTime::Seconds(0.01);
}

void wdClock::Save(wdStreamWriter& inout_stream) const
{
  const wdUInt8 uiVersion = 1;

  inout_stream << uiVersion;
  inout_stream << m_AccumulatedTime;
  inout_stream << m_LastTimeDiff;
  inout_stream << m_FixedTimeStep;
  inout_stream << m_MinTimeStep;
  inout_stream << m_MaxTimeStep;
  inout_stream << m_fSpeed;
  inout_stream << m_bPaused;
}

void wdClock::Load(wdStreamReader& inout_stream)
{
  wdUInt8 uiVersion = 0;
  inout_stream >> uiVersion;

  WD_ASSERT_DEV(uiVersion == 1, "Wrong version for wdClock: {0}", uiVersion);

  inout_stream >> m_AccumulatedTime;
  inout_stream >> m_LastTimeDiff;
  inout_stream >> m_FixedTimeStep;
  inout_stream >> m_MinTimeStep;
  inout_stream >> m_MaxTimeStep;
  inout_stream >> m_fSpeed;
  inout_stream >> m_bPaused;

  // make sure we continue properly
  m_LastTimeUpdate = wdTime::Now() - m_MinTimeStep;

  if (m_pTimeStepSmoother)
    m_pTimeStepSmoother->Reset(this);
}



WD_STATICLINK_FILE(Foundation, Foundation_Time_Implementation_Clock);

#include <Foundation/FoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/Time/Clock.h>

nsClock::Event nsClock::s_TimeEvents;
nsClock* nsClock::s_pGlobalClock = nullptr;

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(Foundation, Clock)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Time"
  END_SUBSYSTEM_DEPENDENCIES

  ON_BASESYSTEMS_STARTUP
  {
    nsClock::s_pGlobalClock = new nsClock("Global");
  }

NS_END_SUBSYSTEM_DECLARATION;

NS_BEGIN_STATIC_REFLECTED_TYPE(nsClock, nsNoBase, 1, nsRTTINoAllocator)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Paused", GetPaused, SetPaused),
    NS_ACCESSOR_PROPERTY("Speed", GetSpeed, SetSpeed),
  }
  NS_END_PROPERTIES;

  NS_BEGIN_FUNCTIONS
  {
    NS_SCRIPT_FUNCTION_PROPERTY(GetGlobalClock),
    NS_SCRIPT_FUNCTION_PROPERTY(GetAccumulatedTime),
    NS_SCRIPT_FUNCTION_PROPERTY(GetTimeDiff)
  }
  NS_END_FUNCTIONS;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

nsClock::nsClock(nsStringView sName)
{
  SetClockName(sName);

  Reset(true);
}

void nsClock::Reset(bool bEverything)
{
  if (bEverything)
  {
    m_pTimeStepSmoother = nullptr;
    m_MinTimeStep = nsTime::MakeFromSeconds(0.001); // 1000 FPS
    m_MaxTimeStep = nsTime::MakeFromSeconds(0.1);   //   10 FPS, many simulations will be instable at that rate already
    m_FixedTimeStep = nsTime::MakeFromSeconds(0.0);
  }

  m_AccumulatedTime = nsTime::MakeFromSeconds(0.0);
  m_fSpeed = 1.0;
  m_bPaused = false;

  // this is to prevent having a time difference of zero (which might not work with some code)
  // in case the next Update() call is done right after this
  m_LastTimeUpdate = nsTime::Now() - m_MinTimeStep;
  m_LastTimeDiff = m_MinTimeStep;

  if (m_pTimeStepSmoother)
    m_pTimeStepSmoother->Reset(this);
}

void nsClock::Update()
{
  const nsTime tNow = nsTime::Now();
  const nsTime tDiff = tNow - m_LastTimeUpdate;
  m_LastTimeUpdate = tNow;

  if (m_bPaused)
  {
    // no change during pause
    m_LastTimeDiff = nsTime::MakeFromSeconds(0.0);
  }
  else if (m_FixedTimeStep > nsTime::MakeFromSeconds(0.0))
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
      m_LastTimeDiff = nsMath::Clamp(tDiff * m_fSpeed, m_MinTimeStep, m_MaxTimeStep);
    }
  }

  m_AccumulatedTime += m_LastTimeDiff;

  EventData ed;
  ed.m_sClockName = m_sName;
  ed.m_RawTimeStep = tDiff;
  ed.m_SmoothedTimeStep = m_LastTimeDiff;

  s_TimeEvents.Broadcast(ed);
}

void nsClock::SetAccumulatedTime(nsTime t)
{
  m_AccumulatedTime = t;

  // this is to prevent having a time difference of zero (which might not work with some code)
  // in case the next Update() call is done right after this
  m_LastTimeUpdate = nsTime::Now() - nsTime::MakeFromSeconds(0.01);
  m_LastTimeDiff = nsTime::MakeFromSeconds(0.01);
}

void nsClock::Save(nsStreamWriter& inout_stream) const
{
  const nsUInt8 uiVersion = 1;

  inout_stream << uiVersion;
  inout_stream << m_AccumulatedTime;
  inout_stream << m_LastTimeDiff;
  inout_stream << m_FixedTimeStep;
  inout_stream << m_MinTimeStep;
  inout_stream << m_MaxTimeStep;
  inout_stream << m_fSpeed;
  inout_stream << m_bPaused;
}

void nsClock::Load(nsStreamReader& inout_stream)
{
  nsUInt8 uiVersion = 0;
  inout_stream >> uiVersion;

  NS_ASSERT_DEV(uiVersion == 1, "Wrong version for nsClock: {0}", uiVersion);

  inout_stream >> m_AccumulatedTime;
  inout_stream >> m_LastTimeDiff;
  inout_stream >> m_FixedTimeStep;
  inout_stream >> m_MinTimeStep;
  inout_stream >> m_MaxTimeStep;
  inout_stream >> m_fSpeed;
  inout_stream >> m_bPaused;

  // make sure we continue properly
  m_LastTimeUpdate = nsTime::Now() - m_MinTimeStep;

  if (m_pTimeStepSmoother)
    m_pTimeStepSmoother->Reset(this);
}



NS_STATICLINK_FILE(Foundation, Foundation_Time_Implementation_Clock);

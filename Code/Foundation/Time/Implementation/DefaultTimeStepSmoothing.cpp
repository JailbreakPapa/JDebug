#include <Foundation/FoundationPCH.h>

#include <Foundation/Time/DefaultTimeStepSmoothing.h>

wdDefaultTimeStepSmoothing::wdDefaultTimeStepSmoothing()
{
  m_fLerpFactor = 0.2f;
}

void wdDefaultTimeStepSmoothing::Reset(const wdClock* pClock)
{
  m_LastTimeSteps.Clear();
}

wdTime wdDefaultTimeStepSmoothing::GetSmoothedTimeStep(wdTime rawTimeStep, const wdClock* pClock)
{
  rawTimeStep = wdMath::Clamp(rawTimeStep * pClock->GetSpeed(), pClock->GetMinimumTimeStep(), pClock->GetMaximumTimeStep());

  if (m_LastTimeSteps.GetCount() < 10)
  {
    m_LastTimeSteps.PushBack(rawTimeStep);
    m_LastTimeStepTaken = rawTimeStep;
    return m_LastTimeStepTaken;
  }

  if (!m_LastTimeSteps.CanAppend(1))
    m_LastTimeSteps.PopFront(1);

  m_LastTimeSteps.PushBack(rawTimeStep);

  wdStaticArray<wdTime, 11> Sorted;
  Sorted.SetCountUninitialized(m_LastTimeSteps.GetCount());

  for (wdUInt32 i = 0; i < m_LastTimeSteps.GetCount(); ++i)
    Sorted[i] = m_LastTimeSteps[i];

  Sorted.Sort();

  wdUInt32 uiFirstSample = 2;
  wdUInt32 uiLastSample = 8;

  wdTime tAvg;

  for (wdUInt32 i = uiFirstSample; i <= uiLastSample; ++i)
  {
    tAvg = tAvg + Sorted[i];
  }

  tAvg = tAvg / (double)((uiLastSample - uiFirstSample) + 1.0);


  m_LastTimeStepTaken = wdMath::Lerp(m_LastTimeStepTaken, tAvg, m_fLerpFactor);

  return m_LastTimeStepTaken;
}



WD_STATICLINK_FILE(Foundation, Foundation_Time_Implementation_DefaultTimeStepSmoothing);

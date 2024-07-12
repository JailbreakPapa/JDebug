#include <Foundation/FoundationPCH.h>

#include <Foundation/Time/DefaultTimeStepSmoothing.h>

nsDefaultTimeStepSmoothing::nsDefaultTimeStepSmoothing()
{
  m_fLerpFactor = 0.2f;
}

void nsDefaultTimeStepSmoothing::Reset(const nsClock* pClock)
{
  m_LastTimeSteps.Clear();
}

nsTime nsDefaultTimeStepSmoothing::GetSmoothedTimeStep(nsTime rawTimeStep, const nsClock* pClock)
{
  rawTimeStep = nsMath::Clamp(rawTimeStep * pClock->GetSpeed(), pClock->GetMinimumTimeStep(), pClock->GetMaximumTimeStep());

  if (m_LastTimeSteps.GetCount() < 10)
  {
    m_LastTimeSteps.PushBack(rawTimeStep);
    m_LastTimeStepTaken = rawTimeStep;
    return m_LastTimeStepTaken;
  }

  if (!m_LastTimeSteps.CanAppend(1))
    m_LastTimeSteps.PopFront(1);

  m_LastTimeSteps.PushBack(rawTimeStep);

  nsStaticArray<nsTime, 11> Sorted;
  Sorted.SetCountUninitialized(m_LastTimeSteps.GetCount());

  for (nsUInt32 i = 0; i < m_LastTimeSteps.GetCount(); ++i)
    Sorted[i] = m_LastTimeSteps[i];

  Sorted.Sort();

  nsUInt32 uiFirstSample = 2;
  nsUInt32 uiLastSample = 8;

  nsTime tAvg;

  for (nsUInt32 i = uiFirstSample; i <= uiLastSample; ++i)
  {
    tAvg = tAvg + Sorted[i];
  }

  tAvg = tAvg / (double)((uiLastSample - uiFirstSample) + 1.0);


  m_LastTimeStepTaken = nsMath::Lerp(m_LastTimeStepTaken, tAvg, m_fLerpFactor);

  return m_LastTimeStepTaken;
}

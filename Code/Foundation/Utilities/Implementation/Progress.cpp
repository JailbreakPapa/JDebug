#include <Foundation/FoundationPCH.h>

#include <Foundation/Utilities/Progress.h>

static wdProgress* s_pGlobal = nullptr;

wdProgress::wdProgress() = default;

wdProgress::~wdProgress()
{
  if (s_pGlobal == this)
  {
    s_pGlobal = nullptr;
  }
}

float wdProgress::GetCompletion() const
{
  return m_fCurrentCompletion;
}

void wdProgress::SetCompletion(float fCompletion)
{
  WD_ASSERT_DEV(fCompletion >= 0.0f && fCompletion <= 1.0f, "Completion value {0} is out of valid range", fCompletion);

  m_fCurrentCompletion = fCompletion;

  if (fCompletion > m_fLastReportedCompletion + 0.001f)
  {
    m_fLastReportedCompletion = fCompletion;

    wdProgressEvent e;
    e.m_pProgressbar = this;
    e.m_Type = wdProgressEvent::Type::ProgressChanged;

    m_Events.Broadcast(e, 1);
  }
}

void wdProgress::SetActiveRange(wdProgressRange* pRange)
{
  if (m_pActiveRange == nullptr && pRange != nullptr)
  {
    m_fLastReportedCompletion = 0.0;
    m_fCurrentCompletion = 0.0;
    m_bCancelClicked = false;
    m_bEnableCancel = pRange->m_bAllowCancel;

    wdProgressEvent e;
    e.m_pProgressbar = this;
    e.m_Type = wdProgressEvent::Type::ProgressStarted;

    m_Events.Broadcast(e);
  }

  if (m_pActiveRange != nullptr && pRange == nullptr)
  {
    wdProgressEvent e;
    e.m_pProgressbar = this;
    e.m_Type = wdProgressEvent::Type::ProgressEnded;

    m_Events.Broadcast(e);
  }

  m_pActiveRange = pRange;
}

wdStringView wdProgress::GetMainDisplayText() const
{
  if (m_pActiveRange == nullptr)
    return {};

  return m_pActiveRange->m_sDisplayText;
}

wdStringView wdProgress::GetStepDisplayText() const
{
  if (m_pActiveRange == nullptr)
    return {};

  return m_pActiveRange->m_sStepDisplayText;
}

void wdProgress::UserClickedCancel()
{
  if (m_bCancelClicked)
    return;

  m_bCancelClicked = true;

  wdProgressEvent e;
  e.m_Type = wdProgressEvent::Type::CancelClicked;
  e.m_pProgressbar = this;

  m_Events.Broadcast(e, 1);
}

bool wdProgress::WasCanceled() const
{
  return m_bCancelClicked;
}

bool wdProgress::AllowUserCancel() const
{
  return m_bEnableCancel;
}

wdProgress* wdProgress::GetGlobalProgressbar()
{
  if (!s_pGlobal)
  {
    static wdProgress s_Global;
    return &s_Global;
  }

  return s_pGlobal;
}

void wdProgress::SetGlobalProgressbar(wdProgress* pProgress)
{
  s_pGlobal = pProgress;
}

//////////////////////////////////////////////////////////////////////////

wdProgressRange::wdProgressRange(wdStringView sDisplayText, wdUInt32 uiSteps, bool bAllowCancel, wdProgress* pProgressbar /*= nullptr*/)
{
  WD_ASSERT_DEV(uiSteps > 0, "Every progress range must have at least one step to complete");

  m_iCurrentStep = -1;
  m_fWeightedCompletion = -1.0;
  m_fSummedWeight = (double)uiSteps;

  Init(sDisplayText, bAllowCancel, pProgressbar);
}

wdProgressRange::wdProgressRange(wdStringView sDisplayText, bool bAllowCancel, wdProgress* pProgressbar /*= nullptr*/)
{
  Init(sDisplayText, bAllowCancel, pProgressbar);
}

void wdProgressRange::Init(wdStringView sDisplayText, bool bAllowCancel, wdProgress* pProgressbar)
{
  if (pProgressbar == nullptr)
    m_pProgressbar = wdProgress::GetGlobalProgressbar();
  else
    m_pProgressbar = pProgressbar;

  WD_ASSERT_DEV(m_pProgressbar != nullptr, "No global progress-bar context available.");

  m_bAllowCancel = bAllowCancel;
  m_sDisplayText = sDisplayText;

  m_pParentRange = m_pProgressbar->m_pActiveRange;

  if (m_pParentRange == nullptr)
  {
    m_fPercentageBase = 0.0;
    m_fPercentageRange = 1.0;
  }
  else
  {
    m_pParentRange->ComputeCurStepBaseAndRange(m_fPercentageBase, m_fPercentageRange);
  }

  m_pProgressbar->SetActiveRange(this);
}

wdProgressRange::~wdProgressRange()
{
  m_pProgressbar->SetCompletion((float)(m_fPercentageBase + m_fPercentageRange));
  m_pProgressbar->SetActiveRange(m_pParentRange);
}

wdProgress* wdProgressRange::GetProgressbar() const
{
  return m_pProgressbar;
}

void wdProgressRange::SetStepWeighting(wdUInt32 uiStep, float fWeight)
{
  WD_ASSERT_DEV(m_fSummedWeight > 0.0, "This function is only supported if ProgressRange was initialized with steps");

  m_fSummedWeight -= GetStepWeight(uiStep);
  m_fSummedWeight += fWeight;
  m_StepWeights[uiStep] = fWeight;
}

float wdProgressRange::GetStepWeight(wdUInt32 uiStep) const
{
  const float* pOldWeight = m_StepWeights.GetValue(uiStep);
  return pOldWeight != nullptr ? *pOldWeight : 1.0f;
}

void wdProgressRange::ComputeCurStepBaseAndRange(double& out_base, double& out_range)
{
  const double internalBase = wdMath::Max(m_fWeightedCompletion, 0.0) / m_fSummedWeight;
  const double internalRange = GetStepWeight(wdMath::Max(m_iCurrentStep, 0)) / m_fSummedWeight;

  out_range = internalRange * m_fPercentageRange;
  out_base = m_fPercentageBase + (internalBase * m_fPercentageRange);

  WD_ASSERT_DEBUG(out_base <= 1.0f, "Invalid range");
  WD_ASSERT_DEBUG(out_range <= 1.0f, "Invalid range");
  WD_ASSERT_DEBUG(out_base + out_range <= 1.0f, "Invalid range");
}

bool wdProgressRange::BeginNextStep(wdStringView sStepDisplayText, wdUInt32 uiNumSteps)
{
  WD_ASSERT_DEV(m_fSummedWeight > 0.0, "This function is only supported if ProgressRange was initialized with steps");

  m_sStepDisplayText = sStepDisplayText;

  for (wdUInt32 i = 0; i < uiNumSteps; ++i)
  {
    m_fWeightedCompletion += GetStepWeight(m_iCurrentStep + i);
  }
  m_iCurrentStep += uiNumSteps;

  const double internalCompletion = m_fWeightedCompletion / m_fSummedWeight;
  const double finalCompletion = m_fPercentageBase + internalCompletion * m_fPercentageRange;

  m_pProgressbar->SetCompletion((float)finalCompletion);

  return !m_pProgressbar->WasCanceled();
}

bool wdProgressRange::SetCompletion(double fCompletionFactor)
{
  WD_ASSERT_DEV(m_fSummedWeight == 0.0, "This function is only supported if ProgressRange was initialized without steps");

  const double finalCompletion = m_fPercentageBase + fCompletionFactor * m_fPercentageRange;

  m_pProgressbar->SetCompletion((float)finalCompletion);

  return !m_pProgressbar->WasCanceled();
}

bool wdProgressRange::WasCanceled() const
{
  if (!m_pProgressbar->m_bCancelClicked)
    return false;

  const wdProgressRange* pCur = this;

  // if there is any action in the stack above, that cannot be canceled
  // all sub actions should be fully executed, even if they could be canceled
  while (pCur)
  {
    if (!pCur->m_bAllowCancel)
      return false;

    pCur = pCur->m_pParentRange;
  }

  return true;
}



WD_STATICLINK_FILE(Foundation, Foundation_Utilities_Implementation_Progress);

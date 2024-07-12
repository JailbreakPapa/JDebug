#include <Foundation/FoundationPCH.h>

#include <Foundation/Utilities/Progress.h>

static nsProgress* s_pGlobal = nullptr;

nsProgress::nsProgress() = default;

nsProgress::~nsProgress()
{
  if (s_pGlobal == this)
  {
    s_pGlobal = nullptr;
  }
}

float nsProgress::GetCompletion() const
{
  return m_fCurrentCompletion;
}

void nsProgress::SetCompletion(float fCompletion)
{
  NS_ASSERT_DEV(fCompletion >= 0.0f && fCompletion <= 1.0f, "Completion value {0} is out of valid range", fCompletion);

  m_fCurrentCompletion = fCompletion;

  if (fCompletion > m_fLastReportedCompletion + 0.001f)
  {
    m_fLastReportedCompletion = fCompletion;

    nsProgressEvent e;
    e.m_pProgressbar = this;
    e.m_Type = nsProgressEvent::Type::ProgressChanged;

    m_Events.Broadcast(e, 1);
  }
}

void nsProgress::SetActiveRange(nsProgressRange* pRange)
{
  if (m_pActiveRange == nullptr && pRange != nullptr)
  {
    m_fLastReportedCompletion = 0.0;
    m_fCurrentCompletion = 0.0;
    m_bCancelClicked = false;
    m_bEnableCancel = pRange->m_bAllowCancel;

    nsProgressEvent e;
    e.m_pProgressbar = this;
    e.m_Type = nsProgressEvent::Type::ProgressStarted;

    m_Events.Broadcast(e);
  }

  if (m_pActiveRange != nullptr && pRange == nullptr)
  {
    nsProgressEvent e;
    e.m_pProgressbar = this;
    e.m_Type = nsProgressEvent::Type::ProgressEnded;

    m_Events.Broadcast(e);
  }

  m_pActiveRange = pRange;
}

nsStringView nsProgress::GetMainDisplayText() const
{
  if (m_pActiveRange == nullptr)
    return {};

  return m_pActiveRange->m_sDisplayText;
}

nsStringView nsProgress::GetStepDisplayText() const
{
  if (m_pActiveRange == nullptr)
    return {};

  return m_pActiveRange->m_sStepDisplayText;
}

void nsProgress::UserClickedCancel()
{
  if (m_bCancelClicked)
    return;

  m_bCancelClicked = true;

  nsProgressEvent e;
  e.m_Type = nsProgressEvent::Type::CancelClicked;
  e.m_pProgressbar = this;

  m_Events.Broadcast(e, 1);
}

bool nsProgress::WasCanceled() const
{
  return m_bCancelClicked;
}

bool nsProgress::AllowUserCancel() const
{
  return m_bEnableCancel;
}

nsProgress* nsProgress::GetGlobalProgressbar()
{
  if (!s_pGlobal)
  {
    static nsProgress s_Global;
    return &s_Global;
  }

  return s_pGlobal;
}

void nsProgress::SetGlobalProgressbar(nsProgress* pProgress)
{
  s_pGlobal = pProgress;
}

//////////////////////////////////////////////////////////////////////////

nsProgressRange::nsProgressRange(nsStringView sDisplayText, nsUInt32 uiSteps, bool bAllowCancel, nsProgress* pProgressbar /*= nullptr*/)
{
  NS_ASSERT_DEV(uiSteps > 0, "Every progress range must have at least one step to complete");

  m_iCurrentStep = -1;
  m_fWeightedCompletion = -1.0;
  m_fSummedWeight = (double)uiSteps;

  Init(sDisplayText, bAllowCancel, pProgressbar);
}

nsProgressRange::nsProgressRange(nsStringView sDisplayText, bool bAllowCancel, nsProgress* pProgressbar /*= nullptr*/)
{
  Init(sDisplayText, bAllowCancel, pProgressbar);
}

void nsProgressRange::Init(nsStringView sDisplayText, bool bAllowCancel, nsProgress* pProgressbar)
{
  if (pProgressbar == nullptr)
    m_pProgressbar = nsProgress::GetGlobalProgressbar();
  else
    m_pProgressbar = pProgressbar;

  NS_ASSERT_DEV(m_pProgressbar != nullptr, "No global progress-bar context available.");

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

nsProgressRange::~nsProgressRange()
{
  m_pProgressbar->SetCompletion((float)(m_fPercentageBase + m_fPercentageRange));
  m_pProgressbar->SetActiveRange(m_pParentRange);
}

nsProgress* nsProgressRange::GetProgressbar() const
{
  return m_pProgressbar;
}

void nsProgressRange::SetStepWeighting(nsUInt32 uiStep, float fWeight)
{
  NS_ASSERT_DEV(m_fSummedWeight > 0.0, "This function is only supported if ProgressRange was initialized with steps");

  m_fSummedWeight -= GetStepWeight(uiStep);
  m_fSummedWeight += fWeight;
  m_StepWeights[uiStep] = fWeight;
}

float nsProgressRange::GetStepWeight(nsUInt32 uiStep) const
{
  const float* pOldWeight = m_StepWeights.GetValue(uiStep);
  return pOldWeight != nullptr ? *pOldWeight : 1.0f;
}

void nsProgressRange::ComputeCurStepBaseAndRange(double& out_base, double& out_range)
{
  const double internalBase = nsMath::Max(m_fWeightedCompletion, 0.0) / m_fSummedWeight;
  const double internalRange = GetStepWeight(nsMath::Max(m_iCurrentStep, 0)) / m_fSummedWeight;

  out_range = internalRange * m_fPercentageRange;
  out_base = m_fPercentageBase + (internalBase * m_fPercentageRange);

  NS_ASSERT_DEBUG(out_base <= 1.0f, "Invalid range");
  NS_ASSERT_DEBUG(out_range <= 1.0f, "Invalid range");
  NS_ASSERT_DEBUG(out_base + out_range <= 1.0f, "Invalid range");
}

bool nsProgressRange::BeginNextStep(nsStringView sStepDisplayText, nsUInt32 uiNumSteps)
{
  NS_ASSERT_DEV(m_fSummedWeight > 0.0, "This function is only supported if ProgressRange was initialized with steps");

  m_sStepDisplayText = sStepDisplayText;

  for (nsUInt32 i = 0; i < uiNumSteps; ++i)
  {
    m_fWeightedCompletion += GetStepWeight(m_iCurrentStep + i);
  }
  m_iCurrentStep += uiNumSteps;

  const double internalCompletion = m_fWeightedCompletion / m_fSummedWeight;
  const double finalCompletion = m_fPercentageBase + internalCompletion * m_fPercentageRange;

  m_pProgressbar->SetCompletion((float)finalCompletion);

  return !m_pProgressbar->WasCanceled();
}

bool nsProgressRange::SetCompletion(double fCompletionFactor)
{
  NS_ASSERT_DEV(m_fSummedWeight == 0.0, "This function is only supported if ProgressRange was initialized without steps");

  const double finalCompletion = m_fPercentageBase + fCompletionFactor * m_fPercentageRange;

  m_pProgressbar->SetCompletion((float)finalCompletion);

  return !m_pProgressbar->WasCanceled();
}

bool nsProgressRange::WasCanceled() const
{
  if (!m_pProgressbar->m_bCancelClicked)
    return false;

  const nsProgressRange* pCur = this;

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

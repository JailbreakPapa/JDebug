#include <Foundation/FoundationPCH.h>

#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Threading/Implementation/Task.h>

nsTask::nsTask() = default;
nsTask::~nsTask() = default;

void nsTask::Reset()
{
  m_iRemainingRuns = (int)nsMath::Max(1u, m_uiMultiplicity);
  m_bCancelExecution = false;
  m_bTaskIsScheduled = false;
  m_bUsesMultiplicity = m_uiMultiplicity > 0;
}

void nsTask::ConfigureTask(const char* szTaskName, nsTaskNesting nestingMode, nsOnTaskFinishedCallback callback /*= nsOnTaskFinishedCallback()*/)
{
  NS_ASSERT_DEV(IsTaskFinished(), "This function must be called before the task is started.");

  m_sTaskName = szTaskName;
  m_NestingMode = nestingMode;
  m_OnTaskFinished = callback;
}

void nsTask::SetMultiplicity(nsUInt32 uiMultiplicity)
{
  m_uiMultiplicity = uiMultiplicity;
  m_bUsesMultiplicity = m_uiMultiplicity > 0;
}

void nsTask::Run(nsUInt32 uiInvocation)
{
  // actually this should not be possible to happen
  if (m_iRemainingRuns == 0 || m_bCancelExecution)
  {
    m_iRemainingRuns = 0;
    return;
  }

  {
    nsStringBuilder scopeName = m_sTaskName;

    if (m_bUsesMultiplicity)
      scopeName.AppendFormat("-{}", uiInvocation);

    NS_PROFILE_SCOPE(scopeName.GetData());

    if (m_bUsesMultiplicity)
    {
      ExecuteWithMultiplicity(uiInvocation);
    }
    else
    {
      Execute();
    }
  }

  m_iRemainingRuns.Decrement();
}

#include <Foundation/FoundationPCH.h>

#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Threading/Implementation/Task.h>

wdTask::wdTask() = default;
wdTask::~wdTask() = default;

void wdTask::Reset()
{
  m_iRemainingRuns = (int)wdMath::Max(1u, m_uiMultiplicity);
  m_bCancelExecution = false;
  m_bTaskIsScheduled = false;
  m_bUsesMultiplicity = m_uiMultiplicity > 0;
}

void wdTask::ConfigureTask(const char* szTaskName, wdTaskNesting nestingMode, wdOnTaskFinishedCallback callback /*= wdOnTaskFinishedCallback()*/)
{
  WD_ASSERT_DEV(IsTaskFinished(), "This function must be called before the task is started.");

  m_sTaskName = szTaskName;
  m_NestingMode = nestingMode;
  m_OnTaskFinished = callback;
}

void wdTask::SetMultiplicity(wdUInt32 uiMultiplicity)
{
  m_uiMultiplicity = uiMultiplicity;
  m_bUsesMultiplicity = m_uiMultiplicity > 0;
}

void wdTask::Run(wdUInt32 uiInvocation)
{
  // actually this should not be possible to happen
  if (m_iRemainingRuns == 0 || m_bCancelExecution)
  {
    m_iRemainingRuns = 0;
    return;
  }

  {
    wdStringBuilder scopeName = m_sTaskName;

    if (m_bUsesMultiplicity)
      scopeName.AppendFormat("-{}", uiInvocation);

    WD_PROFILE_SCOPE(scopeName.GetData());

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


WD_STATICLINK_FILE(Foundation, Foundation_Threading_Implementation_Task);

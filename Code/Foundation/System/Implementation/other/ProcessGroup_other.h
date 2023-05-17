
///
/// Implements wdProcessGroup by using wdProcess
///

#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <Foundation/System/ProcessGroup.h>

struct wdProcessGroupImpl
{
  WD_DECLARE_POD_TYPE();
};

wdProcessGroup::wdProcessGroup(wdStringView sGroupName)
{
}

wdProcessGroup::~wdProcessGroup()
{
  TerminateAll().IgnoreResult();
}

wdResult wdProcessGroup::Launch(const wdProcessOptions& opt)
{
  wdProcess& process = m_Processes.ExpandAndGetRef();
  return process.Launch(opt);
}

wdResult wdProcessGroup::WaitToFinish(wdTime timeout /*= wdTime::Zero()*/)
{
  for (auto& process : m_Processes)
  {
    if (process.GetState() != wdProcessState::Finished && process.WaitToFinish(timeout).Failed())
    {
      return WD_FAILURE;
    }
  }

  return WD_SUCCESS;
}

wdResult wdProcessGroup::TerminateAll(wdInt32 iForcedExitCode /*= -2*/)
{
  auto result = WD_SUCCESS;
  for (auto& process : m_Processes)
  {
    if (process.GetState() == wdProcessState::Running && process.Terminate().Failed())
    {
      result = WD_FAILURE;
    }
  }

  return result;
}

#include <Foundation/FoundationPCH.h>

#if WD_ENABLED(WD_SUPPORTS_PROCESSES)
// Include inline file
#  if WD_ENABLED(WD_PLATFORM_WINDOWS)
#    include <Foundation/System/Implementation/Win/ProcessGroup_win.h>
#  else
#    include <Foundation/System/Implementation/other/ProcessGroup_other.h>
#  endif

const wdHybridArray<wdProcess, 8>& wdProcessGroup::GetProcesses() const
{
  return m_Processes;
}

#endif

WD_STATICLINK_FILE(Foundation, Foundation_System_Implementation_ProcessGroup);

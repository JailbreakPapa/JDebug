#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_SUPPORTS_PROCESSES)

#  include <Foundation/System/ProcessGroup.h>

const nsHybridArray<nsProcess, 8>& nsProcessGroup::GetProcesses() const
{
  return m_Processes;
}

#endif

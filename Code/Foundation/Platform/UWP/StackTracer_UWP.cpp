#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)

#  include <Foundation/System/StackTracer.h>

void nsStackTracer::OnPluginEvent(const nsPluginEvent& e)
{
}

// static
nsUInt32 nsStackTracer::GetStackTrace(nsArrayPtr<void*>& trace, void* pContext)
{
  return 0;
}

// static
void nsStackTracer::ResolveStackTrace(const nsArrayPtr<void*>& trace, PrintFunc printFunc)
{
  char szBuffer[512] = "Stack Traces are currently not supported on UWP";

  printFunc(szBuffer);
}

#endif

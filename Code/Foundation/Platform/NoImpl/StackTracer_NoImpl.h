#include <Foundation/System/StackTracer.h>

void nsStackTracer::OnPluginEvent(const nsPluginEvent& e)
{
}

nsUInt32 nsStackTracer::GetStackTrace(nsArrayPtr<void*>& trace, void* pContext)
{
  return 0;
}

void nsStackTracer::ResolveStackTrace(const nsArrayPtr<void*>& trace, PrintFunc printFunc)
{
}

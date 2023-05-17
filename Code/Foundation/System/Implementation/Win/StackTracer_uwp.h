#ifdef WD_STACKTRACER_UWP_INL_H_INCLUDED
#  error "This file must not be included twice."
#endif

#define WD_STACKTRACER_UWP_INL_H_INCLUDED

#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

void wdStackTracer::OnPluginEvent(const wdPluginEvent& e) {}

// static
wdUInt32 wdStackTracer::GetStackTrace(wdArrayPtr<void*>& trace, void* pContext)
{
  return 0;
}

// static
void wdStackTracer::ResolveStackTrace(const wdArrayPtr<void*>& trace, PrintFunc printFunc)
{
  char szBuffer[512] = "Stack Traces are currently not supported on UWP";

  printFunc(szBuffer);
}

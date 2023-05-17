#ifdef WD_STACKTRACER_POSIX_INL_H_INCLUDED
#  error "This file must not be included twice."
#endif

#define WD_STACKTRACER_POSIX_INL_H_INCLUDED

#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <Foundation/Math/Math.h>
#include <execinfo.h>

void wdStackTracer::OnPluginEvent(const wdPluginEvent& e) {}

// static
wdUInt32 wdStackTracer::GetStackTrace(wdArrayPtr<void*>& trace, void* pContext)
{
  int iSymbols = backtrace(trace.GetPtr(), trace.GetCount());

  return iSymbols;
}

// static
void wdStackTracer::ResolveStackTrace(const wdArrayPtr<void*>& trace, PrintFunc printFunc)
{
  char szBuffer[512];

  char** ppSymbols = backtrace_symbols(trace.GetPtr(), trace.GetCount());

  if (ppSymbols != nullptr)
  {
    for (wdUInt32 i = 0; i < trace.GetCount(); i++)
    {
      int iLen = wdMath::Min(strlen(ppSymbols[i]), (size_t)WD_ARRAY_SIZE(szBuffer) - 2);
      memcpy(szBuffer, ppSymbols[i], iLen);
      szBuffer[iLen] = '\n';
      szBuffer[iLen + 1] = '\0';

      printFunc(szBuffer);
    }

    free(ppSymbols);
  }
}

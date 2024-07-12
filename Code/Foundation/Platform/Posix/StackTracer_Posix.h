#include <Foundation/FoundationInternal.h>
NS_FOUNDATION_INTERNAL_HEADER

#include <Foundation/System/StackTracer.h>

#include <Foundation/Math/Math.h>

#if __has_include(<execinfo.h>)
#  include <execinfo.h>
#  define HAS_EXECINFO 1
#endif

void nsStackTracer::OnPluginEvent(const nsPluginEvent& e)
{
}

// static
nsUInt32 nsStackTracer::GetStackTrace(nsArrayPtr<void*>& trace, void* pContext)
{
#if HAS_EXECINFO
  return backtrace(trace.GetPtr(), trace.GetCount());
#else
  return 0;
#endif
}

// static
void nsStackTracer::ResolveStackTrace(const nsArrayPtr<void*>& trace, PrintFunc printFunc)
{
#if HAS_EXECINFO
  char szBuffer[512];

  char** ppSymbols = backtrace_symbols(trace.GetPtr(), trace.GetCount());

  if (ppSymbols != nullptr)
  {
    for (nsUInt32 i = 0; i < trace.GetCount(); i++)
    {
      size_t uiLen = nsMath::Min(strlen(ppSymbols[i]), static_cast<size_t>(NS_ARRAY_SIZE(szBuffer)) - 2);
      memcpy(szBuffer, ppSymbols[i], uiLen);
      szBuffer[uiLen] = '\n';
      szBuffer[uiLen + 1] = '\0';

      printFunc(szBuffer);
    }

    free(ppSymbols);
  }
#else
  printFunc("Could not record stack trace on this Linux system, because execinfo.h is not available.");
#endif
}

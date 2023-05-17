#ifdef WD_STACKTRACER_ANDROID_INL_H_INCLUDED
#  error "This file must not be included twice."
#endif

#define WD_STACKTRACER_ANDROID_INL_H_INCLUDED

#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <dlfcn.h>
#include <unwind.h>

void wdStackTracer::OnPluginEvent(const wdPluginEvent& e) {}

struct Backtrace
{
  wdUInt32 uiPos = 0;
  wdArrayPtr<void*> trace;
};

static _Unwind_Reason_Code BacktraceCallback(struct _Unwind_Context* pContext, void* pData)
{
  Backtrace& backtrace = *(Backtrace*)pData;

  if (backtrace.uiPos < backtrace.trace.GetCount())
  {
    backtrace.trace[backtrace.uiPos] = reinterpret_cast<void*>(_Unwind_GetIP(pContext));
    backtrace.uiPos++;
    return _URC_NO_REASON;
  }
  else
  {
    return _URC_END_OF_STACK;
  }
}
// static
wdUInt32 wdStackTracer::GetStackTrace(wdArrayPtr<void*>& trace, void* pContext)
{
  Backtrace backtrace;
  backtrace.trace = trace;
  _Unwind_Reason_Code res = _Unwind_Backtrace(BacktraceCallback, &backtrace);
  return backtrace.uiPos;
}

// static
void wdStackTracer::ResolveStackTrace(const wdArrayPtr<void*>& trace, PrintFunc printFunc)
{
  char szBuffer[512];
  for (wdUInt32 i = 0; i < trace.GetCount(); i++)
  {
    Dl_info info;
    if (dladdr(trace[i], &info) && info.dli_sname)
    {
      int iLen = wdMath::Min(strlen(info.dli_sname), (size_t)WD_ARRAY_SIZE(szBuffer) - 2);
      memcpy(szBuffer, info.dli_sname, iLen);
      szBuffer[iLen] = '\n';
      szBuffer[iLen + 1] = '\0';
      printFunc(szBuffer);
    }
    else
    {
      printFunc("Unresolved stack.\n");
    }
  }
}

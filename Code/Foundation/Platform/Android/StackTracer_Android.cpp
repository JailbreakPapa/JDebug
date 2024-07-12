#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_ANDROID)

#  include <Foundation/System/StackTracer.h>

#  include <dlfcn.h>
#  include <unwind.h>

void nsStackTracer::OnPluginEvent(const nsPluginEvent& e) {}

struct Backtrace
{
  nsUInt32 uiPos = 0;
  nsArrayPtr<void*> trace;
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
nsUInt32 nsStackTracer::GetStackTrace(nsArrayPtr<void*>& trace, void* pContext)
{
  Backtrace backtrace;
  backtrace.trace = trace;
  _Unwind_Reason_Code res = _Unwind_Backtrace(BacktraceCallback, &backtrace);
  NS_IGNORE_UNUSED(res);
  return backtrace.uiPos;
}

// static
void nsStackTracer::ResolveStackTrace(const nsArrayPtr<void*>& trace, PrintFunc printFunc)
{
  char szBuffer[512];
  for (nsUInt32 i = 0; i < trace.GetCount(); i++)
  {
    Dl_info info;
    if (dladdr(trace[i], &info) && info.dli_sname)
    {
      int iLen = nsMath::Min(strlen(info.dli_sname), (size_t)NS_ARRAY_SIZE(szBuffer) - 2);
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

#endif

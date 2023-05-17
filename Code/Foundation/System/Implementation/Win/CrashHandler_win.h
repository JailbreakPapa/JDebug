#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <Foundation/System/StackTracer.h>

static LONG WINAPI wdCrashHandlerFunc(struct _EXCEPTION_POINTERS* pExceptionInfo)
{
  static wdMutex s_CrashMutex;
  WD_LOCK(s_CrashMutex);

  static bool s_bAlreadyHandled = false;

  if (s_bAlreadyHandled == false)
  {
    if (wdCrashHandler::GetCrashHandler() != nullptr)
    {
      s_bAlreadyHandled = true;
      wdCrashHandler::GetCrashHandler()->HandleCrash(pExceptionInfo);
    }
  }

  return EXCEPTION_CONTINUE_SEARCH;
}

void wdCrashHandler::SetCrashHandler(wdCrashHandler* pHandler)
{
  s_pActiveHandler = pHandler;

  if (s_pActiveHandler != nullptr)
  {
    SetUnhandledExceptionFilter(wdCrashHandlerFunc);
  }
  else
  {
    SetUnhandledExceptionFilter(nullptr);
  }
}

bool wdCrashHandler_WriteMiniDump::WriteOwnProcessMiniDump(void* pOsSpecificData)
{
#if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)
  wdStatus res = wdMiniDumpUtils::WriteOwnProcessMiniDump(m_sDumpFilePath, (_EXCEPTION_POINTERS*)pOsSpecificData);
  if (res.Failed())
    wdLog::Printf("WriteOwnProcessMiniDump failed: %s\n", res.m_sMessage.GetData());
  return res.Succeeded();
#else
  return false;
#endif
}

void wdCrashHandler_WriteMiniDump::PrintStackTrace(void* pOsSpecificData)
{
  _EXCEPTION_POINTERS* pExceptionInfo = (_EXCEPTION_POINTERS*)pOsSpecificData;

  wdLog::Printf("***Unhandled Exception:***\n");
  wdLog::Printf("Exception: %08x", (wdUInt32)pExceptionInfo->ExceptionRecord->ExceptionCode);

  {
    wdLog::Printf("\n\n***Stack Trace:***\n");
    void* pBuffer[64];
    wdArrayPtr<void*> tempTrace(pBuffer);
    const wdUInt32 uiNumTraces = wdStackTracer::GetStackTrace(tempTrace, pExceptionInfo->ContextRecord);

    wdStackTracer::ResolveStackTrace(tempTrace.GetSubArray(0, uiNumTraces), &PrintHelper);
  }
}

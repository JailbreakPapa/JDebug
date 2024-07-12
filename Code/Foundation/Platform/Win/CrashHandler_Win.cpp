#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS)

#  include <Foundation/Logging/Log.h>
#  include <Foundation/System/CrashHandler.h>
#  include <Foundation/System/MiniDumpUtils.h>
#  include <Foundation/System/StackTracer.h>

static void PrintHelper(const char* szString)
{
  nsLog::Printf("%s", szString);
}

static LONG WINAPI nsCrashHandlerFunc(struct _EXCEPTION_POINTERS* pExceptionInfo)
{
  static nsMutex s_CrashMutex;
  NS_LOCK(s_CrashMutex);

  static bool s_bAlreadyHandled = false;

  if (s_bAlreadyHandled == false)
  {
    if (nsCrashHandler::GetCrashHandler() != nullptr)
    {
      s_bAlreadyHandled = true;
      nsCrashHandler::GetCrashHandler()->HandleCrash(pExceptionInfo);
    }
  }

  return EXCEPTION_CONTINUE_SEARCH;
}

void nsCrashHandler::SetCrashHandler(nsCrashHandler* pHandler)
{
  s_pActiveHandler = pHandler;

  if (s_pActiveHandler != nullptr)
  {
    SetUnhandledExceptionFilter(nsCrashHandlerFunc);
  }
  else
  {
    SetUnhandledExceptionFilter(nullptr);
  }
}

bool nsCrashHandler_WriteMiniDump::WriteOwnProcessMiniDump(void* pOsSpecificData)
{
#  if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)
  nsStatus res = nsMiniDumpUtils::WriteOwnProcessMiniDump(m_sDumpFilePath, (_EXCEPTION_POINTERS*)pOsSpecificData);
  if (res.Failed())
    nsLog::Printf("WriteOwnProcessMiniDump failed: %s\n", res.m_sMessage.GetData());
  return res.Succeeded();
#  else
  return false;
#  endif
}

void nsCrashHandler_WriteMiniDump::PrintStackTrace(void* pOsSpecificData)
{
  _EXCEPTION_POINTERS* pExceptionInfo = (_EXCEPTION_POINTERS*)pOsSpecificData;

  nsLog::Printf("***Unhandled Exception:***\n");
  nsLog::Printf("Exception: %08x", (nsUInt32)pExceptionInfo->ExceptionRecord->ExceptionCode);

  {
    nsLog::Printf("\n\n***Stack Trace:***\n");
    void* pBuffer[64];
    nsArrayPtr<void*> tempTrace(pBuffer);
    const nsUInt32 uiNumTraces = nsStackTracer::GetStackTrace(tempTrace, pExceptionInfo->ContextRecord);

    nsStackTracer::ResolveStackTrace(tempTrace.GetSubArray(0, uiNumTraces), &PrintHelper);
  }
}

#endif

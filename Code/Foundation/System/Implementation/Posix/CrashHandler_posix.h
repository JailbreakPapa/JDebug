#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <Foundation/Logging/Log.h>
#include <Foundation/System/CrashHandler.h>
#include <Foundation/System/StackTracer.h>

#include <csignal>
#include <cxxabi.h>
#include <unistd.h>

static void wdCrashHandlerFunc() noexcept
{
  if (wdCrashHandler::GetCrashHandler() != nullptr)
  {
    wdCrashHandler::GetCrashHandler()->HandleCrash(nullptr);
  }

  // restore the original signal handler for the abort signal and raise one so the kernel can do a core dump
  std::signal(SIGABRT, SIG_DFL);
  std::raise(SIGABRT);
}

static void wdSignalHandler(int signum)
{
  wdLog::Printf("***Unhandled Signal:***\n");
  switch (signum)
  {
    case SIGINT:
      wdLog::Printf("Signal SIGINT: interrupt\n");
      break;
    case SIGILL:
      wdLog::Printf("Signal SIGILL: illegal instruction - invalid function image\n");
      break;
    case SIGFPE:
      wdLog::Printf("Signal SIGFPE: floating point exception\n");
      break;
    case SIGSEGV:
      wdLog::Printf("Signal SIGSEGV: segment violation\n");
      break;
    case SIGTERM:
      wdLog::Printf("Signal SIGTERM: Software termination signal from kill\n");
      break;
    case SIGABRT:
      wdLog::Printf("Signal SIGABRT: abnormal termination triggered by abort call\n");
      break;
    default:
      wdLog::Printf("Signal %i: unknown signal\n", signal);
      break;
  }

  if (wdCrashHandler::GetCrashHandler() != nullptr)
  {
    wdCrashHandler::GetCrashHandler()->HandleCrash(nullptr);
  }

  // forward the signal back to the OS so that it can write a core dump
  std::signal(signum, SIG_DFL);
  kill(getpid(), signum);
}

void wdCrashHandler::SetCrashHandler(wdCrashHandler* pHandler)
{
  s_pActiveHandler = pHandler;

  if (s_pActiveHandler != nullptr)
  {
    std::signal(SIGINT, wdSignalHandler);
    std::signal(SIGILL, wdSignalHandler);
    std::signal(SIGFPE, wdSignalHandler);
    std::signal(SIGSEGV, wdSignalHandler);
    std::signal(SIGTERM, wdSignalHandler);
    std::signal(SIGABRT, wdSignalHandler);
    std::set_terminate(wdCrashHandlerFunc);
  }
  else
  {
    std::signal(SIGINT, nullptr);
    std::signal(SIGILL, nullptr);
    std::signal(SIGFPE, nullptr);
    std::signal(SIGSEGV, nullptr);
    std::signal(SIGTERM, nullptr);
    std::signal(SIGABRT, nullptr);
    std::set_terminate(nullptr);
  }
}

bool wdCrashHandler_WriteMiniDump::WriteOwnProcessMiniDump(void* pOsSpecificData)
{
  return false;
}

void wdCrashHandler_WriteMiniDump::PrintStackTrace(void* pOsSpecificData)
{
  wdLog::Printf("***Unhandled Exception:***\n");

  // wdLog::Printf exception type
  if (std::type_info* type = abi::__cxa_current_exception_type())
  {
    if (const char* szName = type->name())
    {
      int status = -1;
      // Try to print nice name
      if (char* szNiceName = abi::__cxa_demangle(szName, 0, 0, &status))
        wdLog::Printf("Exception: %s\n", szNiceName);
      else
        wdLog::Printf("Exception: %s\n", szName);
    }
  }

  {
    wdLog::Printf("\n\n***Stack Trace:***\n");

    void* pBuffer[64];
    wdArrayPtr<void*> tempTrace(pBuffer);
    const wdUInt32 uiNumTraces = wdStackTracer::GetStackTrace(tempTrace);

    wdStackTracer::ResolveStackTrace(tempTrace.GetSubArray(0, uiNumTraces), &PrintHelper);
  }
}

#include <Foundation/FoundationInternal.h>
NS_FOUNDATION_INTERNAL_HEADER

#include <Foundation/Logging/Log.h>
#include <Foundation/System/CrashHandler.h>
#include <Foundation/System/MiniDumpUtils.h>
#include <Foundation/System/StackTracer.h>

#include <csignal>
#include <cxxabi.h>
#include <unistd.h>

static void PrintHelper(const char* szString)
{
  nsLog::Printf("%s", szString);
}

static void nsCrashHandlerFunc() noexcept
{
  if (nsCrashHandler::GetCrashHandler() != nullptr)
  {
    nsCrashHandler::GetCrashHandler()->HandleCrash(nullptr);
  }

  // restore the original signal handler for the abort signal and raise one so the kernel can do a core dump
  std::signal(SIGABRT, SIG_DFL);
  std::raise(SIGABRT);
}

static void nsSignalHandler(int signum)
{
  nsLog::Printf("***Unhandled Signal:***\n");
  switch (signum)
  {
    case SIGINT:
      nsLog::Printf("Signal SIGINT: interrupt\n");
      break;
    case SIGILL:
      nsLog::Printf("Signal SIGILL: illegal instruction - invalid function image\n");
      break;
    case SIGFPE:
      nsLog::Printf("Signal SIGFPE: floating point exception\n");
      break;
    case SIGSEGV:
      nsLog::Printf("Signal SIGSEGV: segment violation\n");
      break;
    case SIGTERM:
      nsLog::Printf("Signal SIGTERM: Software termination signal from kill\n");
      break;
    case SIGABRT:
      nsLog::Printf("Signal SIGABRT: abnormal termination triggered by abort call\n");
      break;
    default:
      nsLog::Printf("Signal %i: unknown signal\n", signal);
      break;
  }

  if (nsCrashHandler::GetCrashHandler() != nullptr)
  {
    nsCrashHandler::GetCrashHandler()->HandleCrash(nullptr);
  }

  // forward the signal back to the OS so that it can write a core dump
  std::signal(signum, SIG_DFL);
  kill(getpid(), signum);
}

void nsCrashHandler::SetCrashHandler(nsCrashHandler* pHandler)
{
  s_pActiveHandler = pHandler;

  if (s_pActiveHandler != nullptr)
  {
    std::signal(SIGINT, nsSignalHandler);
    std::signal(SIGILL, nsSignalHandler);
    std::signal(SIGFPE, nsSignalHandler);
    std::signal(SIGSEGV, nsSignalHandler);
    std::signal(SIGTERM, nsSignalHandler);
    std::signal(SIGABRT, nsSignalHandler);
    std::set_terminate(nsCrashHandlerFunc);
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

bool nsCrashHandler_WriteMiniDump::WriteOwnProcessMiniDump(void* pOsSpecificData)
{
  return false;
}

void nsCrashHandler_WriteMiniDump::PrintStackTrace(void* pOsSpecificData)
{
  nsLog::Printf("***Unhandled Exception:***\n");

  // nsLog::Printf exception type
  if (std::type_info* type = abi::__cxa_current_exception_type())
  {
    if (const char* szName = type->name())
    {
      int status = -1;
      // Try to print nice name
      if (char* szNiceName = abi::__cxa_demangle(szName, 0, 0, &status))
        nsLog::Printf("Exception: %s\n", szNiceName);
      else
        nsLog::Printf("Exception: %s\n", szName);
    }
  }

  {
    nsLog::Printf("\n\n***Stack Trace:***\n");

    void* pBuffer[64];
    nsArrayPtr<void*> tempTrace(pBuffer);
    const nsUInt32 uiNumTraces = nsStackTracer::GetStackTrace(tempTrace);

    nsStackTracer::ResolveStackTrace(tempTrace.GetSubArray(0, uiNumTraces), &PrintHelper);
  }
}

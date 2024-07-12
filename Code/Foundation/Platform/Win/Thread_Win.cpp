#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS)

#  include <Foundation/Threading/Thread.h>

// Thread entry point used to launch nsRunnable instances
DWORD __stdcall nsThreadClassEntryPoint(LPVOID pThreadParameter)
{
  NS_ASSERT_RELEASE(pThreadParameter != nullptr, "thread parameter in thread entry point must not be nullptr!");

  nsThread* pThread = reinterpret_cast<nsThread*>(pThreadParameter);

  return RunThread(pThread);
}

#endif

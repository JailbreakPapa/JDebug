#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

// Deactivate Doxygen document generation for the following block.
/// \cond

// Thread entry point used to launch wdRunnable instances
DWORD __stdcall wdThreadClassEntryPoint(LPVOID pThreadParameter)
{
  WD_ASSERT_RELEASE(pThreadParameter != nullptr, "thread parameter in thread entry point must not be nullptr!");

  wdThread* pThread = reinterpret_cast<wdThread*>(pThreadParameter);

  return RunThread(pThread);
}


/// \endcond

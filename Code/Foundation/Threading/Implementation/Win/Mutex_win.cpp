#include <Foundation/FoundationPCH.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/Threading/Mutex.h>

#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>

template <wdUInt32 a, wdUInt32 b>
struct SameSize
{
  static_assert(a == b, "Critical section has incorrect size");
};

template <wdUInt32 a, wdUInt32 b>
struct SameAlignment
{
  static_assert(a == b, "Critical section has incorrect alignment");
};


wdMutex::wdMutex()
{
  SameSize<sizeof(wdMutexHandle), sizeof(CRITICAL_SECTION)> check1;
  (void)check1;
  SameAlignment<alignof(wdMutexHandle), alignof(CRITICAL_SECTION)> check2;
  (void)check2;
  InitializeCriticalSection((CRITICAL_SECTION*)&m_hHandle);
}

wdMutex::~wdMutex()
{
  DeleteCriticalSection((CRITICAL_SECTION*)&m_hHandle);
}
#endif


WD_STATICLINK_FILE(Foundation, Foundation_Threading_Implementation_Win_Mutex_win);

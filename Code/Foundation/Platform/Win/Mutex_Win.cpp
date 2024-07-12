#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
#  include <Foundation/Threading/Mutex.h>

#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>

template <nsUInt32 a, nsUInt32 b>
struct SameSize
{
  static_assert(a == b, "Critical section has incorrect size");
};

template <nsUInt32 a, nsUInt32 b>
struct SameAlignment
{
  static_assert(a == b, "Critical section has incorrect alignment");
};


nsMutex::nsMutex()
{
  SameSize<sizeof(nsMutexHandle), sizeof(CRITICAL_SECTION)> check1;
  (void)check1;
  SameAlignment<alignof(nsMutexHandle), alignof(CRITICAL_SECTION)> check2;
  (void)check2;
  InitializeCriticalSection((CRITICAL_SECTION*)&m_hHandle);
}

nsMutex::~nsMutex()
{
  DeleteCriticalSection((CRITICAL_SECTION*)&m_hHandle);
}
#endif

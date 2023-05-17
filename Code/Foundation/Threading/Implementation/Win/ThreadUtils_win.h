#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

// Windows implementation of thread helper functions

static DWORD g_uiMainThreadID = 0xFFFFFFFF;

void wdThreadUtils::Initialize()
{
  g_uiMainThreadID = GetCurrentThreadId();
}

void wdThreadUtils::YieldTimeSlice()
{
  ::Sleep(0);
}

void wdThreadUtils::YieldHardwareThread()
{
  YieldProcessor();
}

void wdThreadUtils::Sleep(const wdTime& duration)
{
  ::Sleep((DWORD)duration.GetMilliseconds());
}

wdThreadID wdThreadUtils::GetCurrentThreadID()
{
  return ::GetCurrentThreadId();
}

bool wdThreadUtils::IsMainThread()
{
  return GetCurrentThreadID() == g_uiMainThreadID;
}

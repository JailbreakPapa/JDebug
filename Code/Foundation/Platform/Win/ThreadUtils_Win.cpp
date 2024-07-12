#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS)

#  include <Foundation/Threading/ThreadUtils.h>

static DWORD g_uiMainThreadID = 0xFFFFFFFF;

void nsThreadUtils::Initialize()
{
  g_uiMainThreadID = GetCurrentThreadId();
}

void nsThreadUtils::YieldTimeSlice()
{
  ::Sleep(0);
}

void nsThreadUtils::YieldHardwareThread()
{
  YieldProcessor();
}

void nsThreadUtils::Sleep(const nsTime& duration)
{
  ::Sleep((DWORD)duration.GetMilliseconds());
}

nsThreadID nsThreadUtils::GetCurrentThreadID()
{
  return ::GetCurrentThreadId();
}

bool nsThreadUtils::IsMainThread()
{
  return GetCurrentThreadID() == g_uiMainThreadID;
}

#endif

#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

// Posix implementation of thread helper functions

#include <pthread.h>

static pthread_t g_MainThread = (pthread_t)0;

void wdThreadUtils::Initialize()
{
  g_MainThread = pthread_self();
}

void wdThreadUtils::YieldTimeSlice()
{
  sched_yield();
}

void wdThreadUtils::YieldHardwareThread()
{
  // No equivalent to mm_pause on linux
}

void wdThreadUtils::Sleep(const wdTime& duration)
{
  timespec SleepTime;
  SleepTime.tv_sec = duration.GetSeconds();
  SleepTime.tv_nsec = ((wdInt64)duration.GetMilliseconds() * 1000000LL) % 1000000000LL;
  nanosleep(&SleepTime, nullptr);
}

// wdThreadHandle wdThreadUtils::GetCurrentThreadHandle()
//{
//  return pthread_self();
//}

wdThreadID wdThreadUtils::GetCurrentThreadID()
{
  return pthread_self();
}

bool wdThreadUtils::IsMainThread()
{
  return pthread_self() == g_MainThread;
}

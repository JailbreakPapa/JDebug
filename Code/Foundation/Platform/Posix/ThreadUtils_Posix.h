#include <Foundation/FoundationInternal.h>
NS_FOUNDATION_INTERNAL_HEADER

#include <Foundation/Threading/ThreadUtils.h>

// Posix implementation of thread helper functions

#include <pthread.h>

static pthread_t g_MainThread = (pthread_t)0;

void nsThreadUtils::Initialize()
{
  g_MainThread = pthread_self();
}

void nsThreadUtils::YieldTimeSlice()
{
  sched_yield();
}

void nsThreadUtils::YieldHardwareThread()
{
  // No equivalent to mm_pause on linux
}

void nsThreadUtils::Sleep(const nsTime& duration)
{
  timespec SleepTime;
  SleepTime.tv_sec = duration.GetSeconds();
  SleepTime.tv_nsec = ((nsInt64)duration.GetMilliseconds() * 1000000LL) % 1000000000LL;
  nanosleep(&SleepTime, nullptr);
}

// nsThreadHandle nsThreadUtils::GetCurrentThreadHandle()
//{
//  return pthread_self();
//}

nsThreadID nsThreadUtils::GetCurrentThreadID()
{
  return pthread_self();
}

bool nsThreadUtils::IsMainThread()
{
  return pthread_self() == g_MainThread;
}

#include <Foundation/FoundationInternal.h>
NS_FOUNDATION_INTERNAL_HEADER

#include <Foundation/Time/Time.h>

#include <time.h>

void nsTime::Initialize()
{
}

nsTime nsTime::Now()
{
  struct timespec sp;
  clock_gettime(CLOCK_MONOTONIC_RAW, &sp);

  return nsTime::MakeFromSeconds((double)sp.tv_sec + (double)(sp.tv_nsec / 1000000000.0));
}

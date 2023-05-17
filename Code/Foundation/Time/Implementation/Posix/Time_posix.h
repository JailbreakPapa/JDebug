#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER
#include <time.h>

void wdTime::Initialize() {}

wdTime wdTime::Now()
{
  struct timespec sp;
  clock_gettime(CLOCK_MONOTONIC_RAW, &sp);

  return wdTime::Seconds((double)sp.tv_sec + (double)(sp.tv_nsec / 1000000000.0));
}

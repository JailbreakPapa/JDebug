#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>

static double g_TimeFactor = 0;

void wdTime::Initialize()
{
  mach_timebase_info_data_t TimebaseInfo;
  mach_timebase_info(&TimebaseInfo);
  g_TimeFactor = (double)TimebaseInfo.numer / (double)TimebaseInfo.denom / (double)1000000000LL;
}

wdTime wdTime::Now()
{
  // mach_absolute_time() returns nanoseconds after factoring in the mach_timebase_info_data_t
  return wdTime::Seconds((double)mach_absolute_time() * g_TimeFactor);
}

#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <Foundation/Basics/Platform/Win/IncludeWindows.h>

static double g_fInvQpcFrequency;

void wdTime::Initialize()
{
  LARGE_INTEGER frequency;
  QueryPerformanceFrequency(&frequency);

  g_fInvQpcFrequency = 1.0 / double(frequency.QuadPart);
}

#if WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT)
thread_local wdInt64 s_LastTime;
#endif

wdTime wdTime::Now()
{
  LARGE_INTEGER temp;
  QueryPerformanceCounter(&temp);

#if WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT)
  WD_ASSERT_DEV(s_LastTime <= temp.QuadPart, "Serious problem, Steve. This is like \"Houston, forget that other thing\".\n\n\n"
                                             "When this happens the PC timer is unreliable. It was probably called from different threads "
                                             "and the clocks on different CPU cores seem to return different results.\n"
                                             "Under these conditions the engine cannot run reliably, it might crash or act weird.");
  s_LastTime = temp.QuadPart;
#endif

  return wdTime::Seconds(double(temp.QuadPart) * g_fInvQpcFrequency);
}

#include <Foundation/FoundationPCH.h>

#include <Foundation/Memory/MemoryTracker.h>
#include <Foundation/Memory/PageAllocator.h>
#include <Foundation/System/SystemInformation.h>

static wdAllocatorId GetPageAllocatorId()
{
  static wdAllocatorId id;

  if (id.IsInvalidated())
  {
    id = wdMemoryTracker::RegisterAllocator("Page", wdMemoryTrackingFlags::Default, wdAllocatorId());
  }

  return id;
}

wdAllocatorId wdPageAllocator::GetId()
{
  return GetPageAllocatorId();
}

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/Memory/Implementation/Win/PageAllocator_win.h>
#elif WD_ENABLED(WD_PLATFORM_OSX) || WD_ENABLED(WD_PLATFORM_LINUX) || WD_ENABLED(WD_PLATFORM_ANDROID)
#  include <Foundation/Memory/Implementation/Posix/PageAllocator_posix.h>
#else
#  error "wdPageAllocator is not implemented on current platform"
#endif

WD_STATICLINK_FILE(Foundation, Foundation_Memory_Implementation_PageAllocator);


#include <Foundation/Time/Time.h>

// static
void* wdPageAllocator::AllocatePage(size_t uiSize)
{
  wdTime fAllocationTime = wdTime::Now();

  void* ptr = nullptr;
  size_t uiAlign = wdSystemInformation::Get().GetMemoryPageSize();
  const int res = posix_memalign(&ptr, uiAlign, uiSize);
  WD_ASSERT_DEBUG(res == 0, "Failed to align pointer");
  WD_IGNORE_UNUSED(res);

  WD_CHECK_ALIGNMENT(ptr, uiAlign);

  if ((wdMemoryTrackingFlags::Default & wdMemoryTrackingFlags::EnableAllocationTracking) != 0)
  {
    wdMemoryTracker::AddAllocation(GetPageAllocatorId(), wdMemoryTrackingFlags::Default, ptr, uiSize, uiAlign, wdTime::Now() - fAllocationTime);
  }

  return ptr;
}

// static
void wdPageAllocator::DeallocatePage(void* ptr)
{
  if ((wdMemoryTrackingFlags::Default & wdMemoryTrackingFlags::EnableAllocationTracking) != 0)
  {
    wdMemoryTracker::RemoveAllocation(GetPageAllocatorId(), ptr);
  }

  free(ptr);
}

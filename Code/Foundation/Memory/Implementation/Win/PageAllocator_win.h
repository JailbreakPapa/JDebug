#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <Foundation/Basics/Platform/Win/Platform_win.h>
#include <Foundation/Time/Time.h>

// static
void* wdPageAllocator::AllocatePage(size_t uiSize)
{
  wdTime fAllocationTime = wdTime::Now();

  void* ptr = ::VirtualAlloc(nullptr, uiSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  WD_ASSERT_DEV(ptr != nullptr, "Could not allocate memory pages. Error Code '{0}'", wdArgErrorCode(::GetLastError()));

  size_t uiAlign = wdSystemInformation::Get().GetMemoryPageSize();
  WD_CHECK_ALIGNMENT(ptr, uiAlign);

  if ((wdMemoryTrackingFlags::Default & wdMemoryTrackingFlags::EnableAllocationTracking) != 0)
  {
    wdMemoryTracker::AddAllocation(GetPageAllocatorId(), wdMemoryTrackingFlags::Default, ptr, uiSize, uiAlign, wdTime::Now() - fAllocationTime);
  }

  return ptr;
}

// static
void wdPageAllocator::DeallocatePage(void* pPtr)
{
  if ((wdMemoryTrackingFlags::Default & wdMemoryTrackingFlags::EnableAllocationTracking) != 0)
  {
    wdMemoryTracker::RemoveAllocation(GetPageAllocatorId(), pPtr);
  }

  WD_VERIFY(::VirtualFree(pPtr, 0, MEM_RELEASE), "Could not free memory pages. Error Code '{0}'", wdArgErrorCode(::GetLastError()));
}

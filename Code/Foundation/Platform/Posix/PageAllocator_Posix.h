#include <Foundation/Memory/MemoryTracker.h>
#include <Foundation/Memory/PageAllocator.h>
#include <Foundation/System/SystemInformation.h>
#include <Foundation/Time/Time.h>

// static
void* nsPageAllocator::AllocatePage(size_t uiSize)
{
  nsTime fAllocationTime = nsTime::Now();

  void* ptr = nullptr;
  size_t uiAlign = nsSystemInformation::Get().GetMemoryPageSize();
  const int res = posix_memalign(&ptr, uiAlign, uiSize);
  NS_ASSERT_DEBUG(res == 0, "Failed to align pointer");
  NS_IGNORE_UNUSED(res);

  NS_CHECK_ALIGNMENT(ptr, uiAlign);

  if constexpr (nsAllocatorTrackingMode::Default >= nsAllocatorTrackingMode::AllocationStats)
  {
    nsMemoryTracker::AddAllocation(nsPageAllocator::GetId(), nsAllocatorTrackingMode::Default, ptr, uiSize, uiAlign, nsTime::Now() - fAllocationTime);
  }

  return ptr;
}

// static
void nsPageAllocator::DeallocatePage(void* ptr)
{
  if constexpr (nsAllocatorTrackingMode::Default >= nsAllocatorTrackingMode::AllocationStats)
  {
    nsMemoryTracker::RemoveAllocation(nsPageAllocator::GetId(), ptr);
  }

  free(ptr);
}

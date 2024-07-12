#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS)

#  include <Foundation/Basics/Platform/Win/Platform_win.h>
#  include <Foundation/Memory/MemoryTracker.h>
#  include <Foundation/Memory/PageAllocator.h>
#  include <Foundation/System/SystemInformation.h>
#  include <Foundation/Time/Time.h>

// static
void* nsPageAllocator::AllocatePage(size_t uiSize)
{
  nsTime fAllocationTime = nsTime::Now();

  void* ptr = ::VirtualAlloc(nullptr, uiSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  NS_ASSERT_DEV(ptr != nullptr, "Could not allocate memory pages. Error Code '{0}'", nsArgErrorCode(::GetLastError()));

  size_t uiAlign = nsSystemInformation::Get().GetMemoryPageSize();
  NS_CHECK_ALIGNMENT(ptr, uiAlign);

  if constexpr (nsAllocatorTrackingMode::Default >= nsAllocatorTrackingMode::AllocationStats)
  {
    nsMemoryTracker::AddAllocation(nsPageAllocator::GetId(), nsAllocatorTrackingMode::Default, ptr, uiSize, uiAlign, nsTime::Now() - fAllocationTime);
  }

  return ptr;
}

// static
void nsPageAllocator::DeallocatePage(void* pPtr)
{
  if constexpr (nsAllocatorTrackingMode::Default >= nsAllocatorTrackingMode::AllocationStats)
  {
    nsMemoryTracker::RemoveAllocation(nsPageAllocator::GetId(), pPtr);
  }

  NS_VERIFY(::VirtualFree(pPtr, 0, MEM_RELEASE), "Could not free memory pages. Error Code '{0}'", nsArgErrorCode(::GetLastError()));
}

#endif

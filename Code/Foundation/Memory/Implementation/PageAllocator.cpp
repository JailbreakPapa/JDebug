#include <Foundation/FoundationPCH.h>

#include <Foundation/Memory/MemoryTracker.h>
#include <Foundation/Memory/PageAllocator.h>

nsAllocatorId nsPageAllocator::GetId()
{
  static nsAllocatorId id;

  if (id.IsInvalidated())
  {
    id = nsMemoryTracker::RegisterAllocator("Page", nsAllocatorTrackingMode::Default, nsAllocatorId());
  }

  return id;
}

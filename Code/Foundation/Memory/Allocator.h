#pragma once

#include <Foundation/Math/Math.h>
#include <Foundation/Memory/AllocatorBase.h>
#include <Foundation/Memory/MemoryTracker.h>
#include <Foundation/Threading/ThreadUtils.h>

WD_MAKE_MEMBERFUNCTION_CHECKER(Reallocate, wdHasReallocate);

#include <Foundation/Memory/Implementation/Allocator_inl.h>

/// \brief Policy based allocator implementation of the wdAllocatorBase interface.
///
/// AllocationPolicy defines how the actual memory is allocated.\n
/// TrackingFlags defines how stats about allocations are tracked.\n
template <typename AllocationPolicy, wdUInt32 TrackingFlags = wdMemoryTrackingFlags::Default>
class wdAllocator : public wdInternal::wdAllocatorMixinReallocate<AllocationPolicy, TrackingFlags,
                      wdHasReallocate<AllocationPolicy, void* (AllocationPolicy::*)(void*, size_t, size_t, size_t)>::value>
{
public:
  wdAllocator(const char* szName, wdAllocatorBase* pParent = nullptr)
    : wdInternal::wdAllocatorMixinReallocate<AllocationPolicy, TrackingFlags,
        wdHasReallocate<AllocationPolicy, void* (AllocationPolicy::*)(void*, size_t, size_t, size_t)>::value>(szName, pParent)
  {
  }
};

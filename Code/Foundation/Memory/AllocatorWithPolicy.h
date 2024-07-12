#pragma once

#include <Foundation/Math/Math.h>
#include <Foundation/Memory/Allocator.h>
#include <Foundation/Memory/MemoryTracker.h>
#include <Foundation/Threading/ThreadUtils.h>

NS_MAKE_MEMBERFUNCTION_CHECKER(Reallocate, nsHasReallocate);

#include <Foundation/Memory/Implementation/AllocatorMixin_inl.h>

/// \brief Policy based allocator implementation of the nsAllocator interface.
///
/// AllocationPolicy defines how the actual memory is allocated.\n
/// TrackingFlags defines how stats about allocations are tracked.\n
template <typename AllocationPolicy, nsAllocatorTrackingMode TrackingMode = nsAllocatorTrackingMode::Default>
class nsAllocatorWithPolicy : public nsInternal::nsAllocatorMixinReallocate<AllocationPolicy, TrackingMode,
                                nsHasReallocate<AllocationPolicy, void* (AllocationPolicy::*)(void*, size_t, size_t, size_t)>::value>
{
public:
  nsAllocatorWithPolicy(nsStringView sName, nsAllocator* pParent = nullptr)
    : nsInternal::nsAllocatorMixinReallocate<AllocationPolicy, TrackingMode,
        nsHasReallocate<AllocationPolicy, void* (AllocationPolicy::*)(void*, size_t, size_t, size_t)>::value>(sName, pParent)
  {
  }
};

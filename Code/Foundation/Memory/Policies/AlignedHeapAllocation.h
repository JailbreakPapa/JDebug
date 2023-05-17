#pragma once

#include <Foundation/Basics.h>

namespace wdMemoryPolicies
{
  /// \brief Aligned Heap memory allocation policy.
  ///
  /// \see wdAllocator
  class wdAlignedHeapAllocation
  {
  public:
    WD_ALWAYS_INLINE wdAlignedHeapAllocation(wdAllocatorBase* pParent) {}
    WD_ALWAYS_INLINE ~wdAlignedHeapAllocation() {}

    void* Allocate(size_t uiSize, size_t uiAlign);
    void Deallocate(void* pPtr);

    WD_ALWAYS_INLINE wdAllocatorBase* GetParent() const { return nullptr; }
  };

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/Memory/Policies/Win/AlignedHeapAllocation_win.h>
#elif WD_ENABLED(WD_PLATFORM_OSX) || WD_ENABLED(WD_PLATFORM_LINUX) || WD_ENABLED(WD_PLATFORM_ANDROID)
#  include <Foundation/Memory/Policies/Posix/AlignedHeapAllocation_posix.h>
#else
#  error "wdAlignedHeapAllocation is not implemented on current platform"
#endif
} // namespace wdMemoryPolicies

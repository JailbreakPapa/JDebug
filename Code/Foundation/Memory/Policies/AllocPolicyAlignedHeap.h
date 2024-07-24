#pragma once

#include <Foundation/Basics.h>

/// \brief Aligned Heap memory allocation policy.
///
/// \see nsAllocatorWithPolicy
class nsAllocPolicyAlignedHeap
{
public:
  NS_ALWAYS_INLINE nsAllocPolicyAlignedHeap(nsAllocator* pParent) {}
  NS_ALWAYS_INLINE ~nsAllocPolicyAlignedHeap() = default;

  void* Allocate(size_t uiSize, size_t uiAlign);
  void Deallocate(void* pPtr);

  NS_ALWAYS_INLINE nsAllocator* GetParent() const { return nullptr; }
};

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
#  include <Foundation/Memory/Policies/Win/AllocPolicyAlignedHeap_win.h>
#elif NS_ENABLED(NS_PLATFORM_OSX) || NS_ENABLED(NS_PLATFORM_LINUX) || NS_ENABLED(NS_PLATFORM_ANDROID)
#  include <Foundation/Memory/Policies/Posix/AllocPolicyAlignedHeap_posix.h>
#else
#  error "nsAllocPolicyAlignedHeap is not implemented on current platform"
#endif

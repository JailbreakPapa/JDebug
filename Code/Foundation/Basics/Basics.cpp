#include <Foundation/FoundationPCH.h>

#include <Foundation/Memory/CommonAllocators.h>

#if NS_ENABLED(NS_ALLOC_GUARD_ALLOCATIONS)
using DefaultHeapType = nsGuardingAllocator;
using DefaultAlignedHeapType = nsGuardingAllocator;
using DefaultStaticsHeapType = nsAllocatorWithPolicy<nsAllocPolicyGuarding, nsAllocatorTrackingMode::AllocationStatsIgnoreLeaks>;
#else
using DefaultHeapType = nsHeapAllocator;
using DefaultAlignedHeapType = nsAlignedHeapAllocator;
using DefaultStaticsHeapType = nsAllocatorWithPolicy<nsAllocPolicyHeap, nsAllocatorTrackingMode::AllocationStatsIgnoreLeaks>;
#endif

enum
{
  HEAP_ALLOCATOR_BUFFER_SIZE = sizeof(DefaultHeapType),
  ALIGNED_ALLOCATOR_BUFFER_SIZE = sizeof(DefaultAlignedHeapType)
};

alignas(NS_ALIGNMENT_MINIMUM) static nsUInt8 s_DefaultAllocatorBuffer[HEAP_ALLOCATOR_BUFFER_SIZE];
alignas(NS_ALIGNMENT_MINIMUM) static nsUInt8 s_StaticAllocatorBuffer[HEAP_ALLOCATOR_BUFFER_SIZE];

alignas(NS_ALIGNMENT_MINIMUM) static nsUInt8 s_AlignedAllocatorBuffer[ALIGNED_ALLOCATOR_BUFFER_SIZE];

bool nsFoundation::s_bIsInitialized = false;
nsAllocator* nsFoundation::s_pDefaultAllocator = nullptr;
nsAllocator* nsFoundation::s_pAlignedAllocator = nullptr;

void nsFoundation::Initialize()
{
  if (s_bIsInitialized)
    return;

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  nsMemoryUtils::ReserveLower4GBAddressSpace();
#endif

  if (s_pDefaultAllocator == nullptr)
  {
    s_pDefaultAllocator = new (s_DefaultAllocatorBuffer) DefaultHeapType("DefaultHeap");
  }

  if (s_pAlignedAllocator == nullptr)
  {
    s_pAlignedAllocator = new (s_AlignedAllocatorBuffer) DefaultAlignedHeapType("AlignedHeap");
  }

  s_bIsInitialized = true;
}

#if defined(NS_CUSTOM_STATIC_ALLOCATOR_FUNC)
extern nsAllocator* NS_CUSTOM_STATIC_ALLOCATOR_FUNC();
#endif

nsAllocator* nsFoundation::GetStaticsAllocator()
{
  static nsAllocator* pStaticAllocator = nullptr;

  if (pStaticAllocator == nullptr)
  {
#if defined(NS_CUSTOM_STATIC_ALLOCATOR_FUNC)

#  if NS_ENABLED(NS_COMPILE_ENGINE_AS_DLL)

#    if NS_ENABLED(NS_PLATFORM_WINDOWS)
    using GetStaticAllocatorFunc = nsAllocator* (*)();

    HMODULE hThisModule = GetModuleHandle(nullptr);
    GetStaticAllocatorFunc func = (GetStaticAllocatorFunc)GetProcAddress(hThisModule, NS_CUSTOM_STATIC_ALLOCATOR_FUNC);
    if (func != nullptr)
    {
      pStaticAllocator = (*func)();
      return pStaticAllocator;
    }
#    else
#      error "Customizing static allocator not implemented"
#    endif

#  else
    return NS_CUSTOM_STATIC_ALLOCATOR_FUNC();
#  endif

#endif

    pStaticAllocator = new (s_StaticAllocatorBuffer) DefaultStaticsHeapType("Statics");
  }

  return pStaticAllocator;
}

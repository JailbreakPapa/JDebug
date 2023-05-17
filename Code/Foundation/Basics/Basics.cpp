#include <Foundation/FoundationPCH.h>

#include <Foundation/Memory/CommonAllocators.h>

#if WD_ENABLED(WD_USE_GUARDED_ALLOCATIONS)
typedef wdGuardedAllocator DefaultHeapType;
typedef wdGuardedAllocator DefaultAlignedHeapType;
typedef wdGuardedAllocator DefaultStaticHeapType;
#else
typedef wdHeapAllocator DefaultHeapType;
typedef wdAlignedHeapAllocator DefaultAlignedHeapType;
typedef wdHeapAllocator DefaultStaticHeapType;
#endif

enum
{
  HEAP_ALLOCATOR_BUFFER_SIZE = sizeof(DefaultHeapType),
  ALIGNED_ALLOCATOR_BUFFER_SIZE = sizeof(DefaultAlignedHeapType)
};

alignas(WD_ALIGNMENT_MINIMUM) static wdUInt8 s_DefaultAllocatorBuffer[HEAP_ALLOCATOR_BUFFER_SIZE];
alignas(WD_ALIGNMENT_MINIMUM) static wdUInt8 s_StaticAllocatorBuffer[HEAP_ALLOCATOR_BUFFER_SIZE];

alignas(WD_ALIGNMENT_MINIMUM) static wdUInt8 s_AlignedAllocatorBuffer[ALIGNED_ALLOCATOR_BUFFER_SIZE];

bool wdFoundation::s_bIsInitialized = false;
wdAllocatorBase* wdFoundation::s_pDefaultAllocator = nullptr;
wdAllocatorBase* wdFoundation::s_pAlignedAllocator = nullptr;

void wdFoundation::Initialize()
{
  if (s_bIsInitialized)
    return;

#if WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT)
  wdMemoryUtils::ReserveLower4GBAddressSpace();
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

#if defined(WD_CUSTOM_STATIC_ALLOCATOR_FUNC)
extern wdAllocatorBase* WD_CUSTOM_STATIC_ALLOCATOR_FUNC();
#endif

wdAllocatorBase* wdFoundation::GetStaticAllocator()
{
  static wdAllocatorBase* pStaticAllocator = nullptr;

  if (pStaticAllocator == nullptr)
  {
#if defined(WD_CUSTOM_STATIC_ALLOCATOR_FUNC)

#  if WD_ENABLED(WD_COMPILE_ENGINE_AS_DLL)

#    if WD_ENABLED(WD_PLATFORM_WINDOWS)
    typedef wdAllocatorBase* (*GetStaticAllocatorFunc)();

    HMODULE hThisModule = GetModuleHandle(nullptr);
    GetStaticAllocatorFunc func = (GetStaticAllocatorFunc)GetProcAddress(hThisModule, WD_CUSTOM_STATIC_ALLOCATOR_FUNC);
    if (func != nullptr)
    {
      pStaticAllocator = (*func)();
      return pStaticAllocator;
    }
#    else
#      error "Customizing static allocator not implemented"
#    endif

#  else
    return WD_CUSTOM_STATIC_ALLOCATOR_FUNC();
#  endif

#endif

    pStaticAllocator = new (s_StaticAllocatorBuffer) DefaultStaticHeapType(WD_STATIC_ALLOCATOR_NAME);
  }

  return pStaticAllocator;
}



WD_STATICLINK_FILE(Foundation, Foundation_Basics_Basics);

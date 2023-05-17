#pragma once

#include <Foundation/Basics.h>

namespace wdMemoryPolicies
{
  /// \brief Default heap memory allocation policy.
  ///
  /// \see wdAllocator
  class wdHeapAllocation
  {
  public:
    WD_ALWAYS_INLINE wdHeapAllocation(wdAllocatorBase* pParent) {}
    WD_ALWAYS_INLINE ~wdHeapAllocation() {}

    WD_FORCE_INLINE void* Allocate(size_t uiSize, size_t uiAlign)
    {
      // malloc has no alignment guarantees, even though on many systems it returns 16 byte aligned data
      // if these asserts fail, you need to check what container made the allocation and change it
      // to use an aligned allocator, e.g. wdAlignedAllocatorWrapper

      // unfortunately using WD_ALIGNMENT_MINIMUM doesn't work, because even on 32 Bit systems we try to do allocations with 8 Byte
      // alignment interestingly, the code that does that, seems to work fine anyway
      WD_ASSERT_DEBUG(
        uiAlign <= 8, "This allocator does not guarantee alignments larger than 8. Use an aligned allocator to allocate the desired data type.");

      void* ptr = malloc(PadSize(uiSize));
      WD_CHECK_ALIGNMENT(ptr, uiAlign);

      return OffsetPtr(ptr);
    }

    WD_FORCE_INLINE void* Reallocate(void* pCurrentPtr, size_t uiCurrentSize, size_t uiNewSize, size_t uiAlign)
    {
      void* ptr = realloc(RestorePtr(pCurrentPtr), PadSize(uiNewSize));
      WD_CHECK_ALIGNMENT(ptr, uiAlign);

      return OffsetPtr(ptr);
    }

    WD_ALWAYS_INLINE void Deallocate(void* pPtr) { free(RestorePtr(pPtr)); }

    WD_ALWAYS_INLINE wdAllocatorBase* GetParent() const { return nullptr; }

  private:
    WD_ALWAYS_INLINE size_t PadSize(size_t uiSize)
    {
#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
      return uiSize + 2 * WD_ALIGNMENT_MINIMUM;
#else
      return uiSize;
#endif
    }

    WD_ALWAYS_INLINE void* OffsetPtr(void* ptr)
    {
#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
      wdUInt32 uiOffset = wdMemoryUtils::IsAligned(ptr, 2 * WD_ALIGNMENT_MINIMUM) ? WD_ALIGNMENT_MINIMUM : 2 * WD_ALIGNMENT_MINIMUM;
      ptr = wdMemoryUtils::AddByteOffset(ptr, uiOffset - 4);
      *static_cast<wdUInt32*>(ptr) = uiOffset;
      return wdMemoryUtils::AddByteOffset(ptr, 4);
#else
      return ptr;
#endif
    }

    WD_ALWAYS_INLINE void* RestorePtr(void* ptr)
    {
#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
      ptr = wdMemoryUtils::AddByteOffset(ptr, -4);
      wdInt32 uiOffset = *static_cast<wdUInt32*>(ptr);
      return wdMemoryUtils::AddByteOffset(ptr, -uiOffset + 4);
#else
      return ptr;
#endif
    }
  };
} // namespace wdMemoryPolicies

#pragma once

#include <Foundation/Math/Math.h>

namespace wdMemoryPolicies
{
  /// \brief Allocation policy to support custom alignment per allocation.
  ///
  /// \see wdAllocator
  template <typename T>
  class wdAlignedAllocation
  {
  public:
    wdAlignedAllocation(wdAllocatorBase* pParent)
      : m_allocator(pParent)
    {
    }

    void* Allocate(size_t uiSize, size_t uiAlign)
    {
      WD_ASSERT_DEV(uiAlign < (1 << 24), "Alignment of {0} is too big. Maximum supported alignment is 16MB.", uiAlign);

      const wdUInt32 uiPadding = (wdUInt32)(uiAlign - 1 + MetadataSize);
      const size_t uiAlignedSize = uiSize + uiPadding;

      wdUInt8* pMemory = (wdUInt8*)m_allocator.Allocate(uiAlignedSize, WD_ALIGNMENT_MINIMUM);

      wdUInt8* pAlignedMemory = wdMemoryUtils::AlignBackwards(pMemory + uiPadding, uiAlign);

      wdUInt32* pMetadata = GetMetadataPtr(pAlignedMemory);
      *pMetadata = PackMetadata((wdUInt32)(pAlignedMemory - pMemory), (wdUInt32)uiAlign);

      return pAlignedMemory;
    }

    void Deallocate(void* pPtr)
    {
      const wdUInt32 uiOffset = UnpackOffset(GetMetadata(pPtr));
      wdUInt8* pMemory = static_cast<wdUInt8*>(pPtr) - uiOffset;
      m_allocator.Deallocate(pMemory);
    }

    size_t AllocatedSize(const void* pPtr)
    {
      const wdUInt32 uiMetadata = GetMetadata(pPtr);
      const wdUInt32 uiOffset = UnpackOffset(uiMetadata);
      const wdUInt32 uiAlign = UnpackAlignment(uiMetadata);
      const wdUInt32 uiPadding = uiAlign - 1 + MetadataSize;

      const wdUInt8* pMemory = static_cast<const wdUInt8*>(pPtr) - uiOffset;
      return m_allocator.AllocatedSize(pMemory) - uiPadding;
    }

    size_t UsedMemorySize(const void* pPtr)
    {
      const wdUInt32 uiOffset = UnpackOffset(GetMetadata(pPtr));
      const wdUInt8* pMemory = static_cast<const wdUInt8*>(pPtr) - uiOffset;
      return m_allocator.UsedMemorySize(pMemory);
    }

    WD_ALWAYS_INLINE wdAllocatorBase* GetParent() const { return m_allocator.GetParent(); }

  private:
    enum
    {
      MetadataSize = sizeof(wdUInt32)
    };

    // Meta-data is stored 4 bytes before the aligned memory
    inline wdUInt32* GetMetadataPtr(void* pAlignedMemory)
    {
      return static_cast<wdUInt32*>(wdMemoryUtils::AddByteOffset(pAlignedMemory, -MetadataSize));
    }

    inline wdUInt32 GetMetadata(const void* pAlignedMemory)
    {
      return *static_cast<const wdUInt32*>(wdMemoryUtils::AddByteOffset(pAlignedMemory, -MetadataSize));
    }

    // Store offset between pMemory and pAlignedMemory in the lower 24 bit of meta-data.
    // The upper 8 bit are used to store the Log2 of the alignment.
    WD_ALWAYS_INLINE wdUInt32 PackMetadata(wdUInt32 uiOffset, wdUInt32 uiAlignment) { return uiOffset | (wdMath::Log2i(uiAlignment) << 24); }

    WD_ALWAYS_INLINE wdUInt32 UnpackOffset(wdUInt32 uiMetadata) { return uiMetadata & 0x00FFFFFF; }

    WD_ALWAYS_INLINE wdUInt32 UnpackAlignment(wdUInt32 uiMetadata) { return 1 << (uiMetadata >> 24); }

    T m_allocator;
  };
} // namespace wdMemoryPolicies

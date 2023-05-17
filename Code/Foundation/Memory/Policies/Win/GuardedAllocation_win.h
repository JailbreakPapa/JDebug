
#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <Foundation/Basics/Platform/Win/IncludeWindows.h>

namespace wdMemoryPolicies
{
  struct AlloctionMetaData
  {
    AlloctionMetaData()
    {
      m_uiSize = 0;

      for (wdUInt32 i = 0; i < WD_ARRAY_SIZE(m_magic); ++i)
      {
        m_magic[i] = 0x12345678;
      }
    }

    ~AlloctionMetaData()
    {
      for (wdUInt32 i = 0; i < WD_ARRAY_SIZE(m_magic); ++i)
      {
        WD_ASSERT_DEV(m_magic[i] == 0x12345678, "Magic value has been overwritten. This might be the result of a buffer underrun!");
      }
    }

    size_t m_uiSize;
    wdUInt32 m_magic[32];
  };

  wdGuardedAllocation::wdGuardedAllocation(wdAllocatorBase* pParent)
  {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    m_uiPageSize = sysInfo.dwPageSize;
  }


  void* wdGuardedAllocation::Allocate(size_t uiSize, size_t uiAlign)
  {
    WD_ASSERT_DEV(wdMath::IsPowerOf2((wdUInt32)uiAlign), "Alignment must be power of two");
    uiAlign = wdMath::Max<size_t>(uiAlign, WD_ALIGNMENT_MINIMUM);

    size_t uiAlignedSize = wdMemoryUtils::AlignSize(uiSize, uiAlign);
    size_t uiTotalSize = uiAlignedSize + sizeof(AlloctionMetaData);

    // align to full pages and add one page in front and one in back
    size_t uiPageSize = m_uiPageSize;
    size_t uiFullPageSize = wdMemoryUtils::AlignSize(uiTotalSize, uiPageSize);
    void* pMemory = VirtualAlloc(nullptr, uiFullPageSize + 2 * uiPageSize, MEM_RESERVE, PAGE_NOACCESS);
    WD_ASSERT_DEV(pMemory != nullptr, "Could not reserve memory pages. Error Code '{0}'", wdArgErrorCode(::GetLastError()));

    // add one page and commit the payload pages
    pMemory = wdMemoryUtils::AddByteOffset(pMemory, uiPageSize);
    void* ptr = VirtualAlloc(pMemory, uiFullPageSize, MEM_COMMIT, PAGE_READWRITE);
    WD_ASSERT_DEV(ptr != nullptr, "Could not commit memory pages. Error Code '{0}'", wdArgErrorCode(::GetLastError()));

    // store information in meta data
    AlloctionMetaData* metaData = wdMemoryUtils::AddByteOffset(static_cast<AlloctionMetaData*>(ptr), uiFullPageSize - uiTotalSize);
    wdMemoryUtils::Construct(metaData, 1);
    metaData->m_uiSize = uiAlignedSize;

    // finally add offset to the actual payload
    ptr = wdMemoryUtils::AddByteOffset(metaData, sizeof(AlloctionMetaData));
    return ptr;
  }

  // deactivate analysis warning for VirtualFree flags, it is needed for the specific functionality
  WD_MSVC_ANALYSIS_WARNING_PUSH
  WD_MSVC_ANALYSIS_WARNING_DISABLE(6250)

  void wdGuardedAllocation::Deallocate(void* pPtr)
  {
    wdLock<wdMutex> lock(m_Mutex);

    if (!m_AllocationsToFreeLater.CanAppend())
    {
      void* pMemory = m_AllocationsToFreeLater.PeekFront();
      WD_VERIFY(::VirtualFree(pMemory, 0, MEM_RELEASE), "Could not free memory pages. Error Code '{0}'", wdArgErrorCode(::GetLastError()));

      m_AllocationsToFreeLater.PopFront();
    }

    // Retrieve info from meta data first.
    AlloctionMetaData* metaData = wdMemoryUtils::AddByteOffset(static_cast<AlloctionMetaData*>(pPtr), -((ptrdiff_t)sizeof(AlloctionMetaData)));
    size_t uiAlignedSize = metaData->m_uiSize;

    wdMemoryUtils::Destruct(metaData, 1);

    // Decommit the pages but do not release the memory yet so use-after-free can be detected.
    size_t uiPageSize = m_uiPageSize;
    size_t uiTotalSize = uiAlignedSize + sizeof(AlloctionMetaData);
    size_t uiFullPageSize = wdMemoryUtils::AlignSize(uiTotalSize, uiPageSize);
    pPtr = wdMemoryUtils::AddByteOffset(pPtr, ((ptrdiff_t)uiAlignedSize) - uiFullPageSize);

    WD_VERIFY(
      ::VirtualFree(pPtr, uiFullPageSize, MEM_DECOMMIT), "Could not decommit memory pages. Error Code '{0}'", wdArgErrorCode(::GetLastError()));

    // Finally store the allocation so we can release it later
    void* pMemory = wdMemoryUtils::AddByteOffset(pPtr, -((ptrdiff_t)uiPageSize));
    m_AllocationsToFreeLater.PushBack(pMemory);
  }

  WD_MSVC_ANALYSIS_WARNING_POP
} // namespace wdMemoryPolicies

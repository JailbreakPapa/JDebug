#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS)

#  include <Foundation/Memory/Policies/AllocPolicyGuarding.h>

#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>

struct AlloctionMetaData
{
  AlloctionMetaData()
  {
    m_uiSize = 0;

    for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(m_magic); ++i)
    {
      m_magic[i] = 0x12345678;
    }
  }

  ~AlloctionMetaData()
  {
    for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(m_magic); ++i)
    {
      NS_ASSERT_DEV(m_magic[i] == 0x12345678, "Magic value has been overwritten. This might be the result of a buffer underrun!");
    }
  }

  size_t m_uiSize;
  nsUInt32 m_magic[32];
};

nsAllocPolicyGuarding::nsAllocPolicyGuarding(nsAllocator* pParent)
{
  SYSTEM_INFO sysInfo;
  GetSystemInfo(&sysInfo);
  m_uiPageSize = sysInfo.dwPageSize;
}


void* nsAllocPolicyGuarding::Allocate(size_t uiSize, size_t uiAlign)
{
  NS_ASSERT_DEV(nsMath::IsPowerOf2((nsUInt32)uiAlign), "Alignment must be power of two");
  uiAlign = nsMath::Max<size_t>(uiAlign, NS_ALIGNMENT_MINIMUM);

  size_t uiAlignedSize = nsMemoryUtils::AlignSize(uiSize, uiAlign);
  size_t uiTotalSize = uiAlignedSize + sizeof(AlloctionMetaData);

  // align to full pages and add one page in front and one in back
  size_t uiPageSize = m_uiPageSize;
  size_t uiFullPageSize = nsMemoryUtils::AlignSize(uiTotalSize, uiPageSize);
  void* pMemory = VirtualAlloc(nullptr, uiFullPageSize + 2 * uiPageSize, MEM_RESERVE, PAGE_NOACCESS);
  NS_ASSERT_DEV(pMemory != nullptr, "Could not reserve memory pages. Error Code '{0}'", nsArgErrorCode(::GetLastError()));

  // add one page and commit the payload pages
  pMemory = nsMemoryUtils::AddByteOffset(pMemory, uiPageSize);
  void* ptr = VirtualAlloc(pMemory, uiFullPageSize, MEM_COMMIT, PAGE_READWRITE);
  NS_ASSERT_DEV(ptr != nullptr, "Could not commit memory pages. Error Code '{0}'", nsArgErrorCode(::GetLastError()));

  // store information in meta data
  AlloctionMetaData* metaData = nsMemoryUtils::AddByteOffset(static_cast<AlloctionMetaData*>(ptr), uiFullPageSize - uiTotalSize);
  nsMemoryUtils::Construct<SkipTrivialTypes>(metaData, 1);
  metaData->m_uiSize = uiAlignedSize;

  // finally add offset to the actual payload
  ptr = nsMemoryUtils::AddByteOffset(metaData, sizeof(AlloctionMetaData));
  return ptr;
}

// deactivate analysis warning for VirtualFree flags, it is needed for the specific functionality
NS_MSVC_ANALYSIS_WARNING_PUSH
NS_MSVC_ANALYSIS_WARNING_DISABLE(6250)

void nsAllocPolicyGuarding::Deallocate(void* pPtr)
{
  nsLock<nsMutex> lock(m_Mutex);

  if (!m_AllocationsToFreeLater.CanAppend())
  {
    void* pMemory = m_AllocationsToFreeLater.PeekFront();
    NS_VERIFY(::VirtualFree(pMemory, 0, MEM_RELEASE), "Could not free memory pages. Error Code '{0}'", nsArgErrorCode(::GetLastError()));

    m_AllocationsToFreeLater.PopFront();
  }

  // Retrieve info from meta data first.
  AlloctionMetaData* metaData = nsMemoryUtils::AddByteOffset(static_cast<AlloctionMetaData*>(pPtr), -((std::ptrdiff_t)sizeof(AlloctionMetaData)));
  size_t uiAlignedSize = metaData->m_uiSize;

  nsMemoryUtils::Destruct(metaData, 1);

  // Decommit the pages but do not release the memory yet so use-after-free can be detected.
  size_t uiPageSize = m_uiPageSize;
  size_t uiTotalSize = uiAlignedSize + sizeof(AlloctionMetaData);
  size_t uiFullPageSize = nsMemoryUtils::AlignSize(uiTotalSize, uiPageSize);
  pPtr = nsMemoryUtils::AddByteOffset(pPtr, ((std::ptrdiff_t)uiAlignedSize) - uiFullPageSize);

  NS_VERIFY(
    ::VirtualFree(pPtr, uiFullPageSize, MEM_DECOMMIT), "Could not decommit memory pages. Error Code '{0}'", nsArgErrorCode(::GetLastError()));

  // Finally store the allocation so we can release it later
  void* pMemory = nsMemoryUtils::AddByteOffset(pPtr, -((std::ptrdiff_t)uiPageSize));
  m_AllocationsToFreeLater.PushBack(pMemory);
}

NS_MSVC_ANALYSIS_WARNING_POP

#endif

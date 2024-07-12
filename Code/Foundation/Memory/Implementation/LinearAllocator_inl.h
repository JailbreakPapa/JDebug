template <nsAllocatorTrackingMode TrackingMode, bool OverwriteMemoryOnReset>
nsLinearAllocator<TrackingMode, OverwriteMemoryOnReset>::nsLinearAllocator(nsStringView sName, nsAllocator* pParent)
  : nsAllocatorWithPolicy<typename nsLinearAllocator<TrackingMode, OverwriteMemoryOnReset>::PolicyStack, TrackingMode>(sName, pParent)
  , m_DestructData(pParent)
  , m_PtrToDestructDataIndexTable(pParent)
{
}

template <nsAllocatorTrackingMode TrackingMode, bool OverwriteMemoryOnReset>
nsLinearAllocator<TrackingMode, OverwriteMemoryOnReset>::~nsLinearAllocator()
{
  Reset();
}

template <nsAllocatorTrackingMode TrackingMode, bool OverwriteMemoryOnReset>
void* nsLinearAllocator<TrackingMode, OverwriteMemoryOnReset>::Allocate(size_t uiSize, size_t uiAlign, nsMemoryUtils::DestructorFunction destructorFunc)
{
  NS_LOCK(m_Mutex);

  void* ptr = nsAllocatorWithPolicy<typename nsLinearAllocator<TrackingMode, OverwriteMemoryOnReset>::PolicyStack, TrackingMode>::Allocate(uiSize, uiAlign, destructorFunc);

  if (destructorFunc != nullptr)
  {
    nsUInt32 uiIndex = m_DestructData.GetCount();
    m_PtrToDestructDataIndexTable.Insert(ptr, uiIndex);

    auto& data = m_DestructData.ExpandAndGetRef();
    data.m_Func = destructorFunc;
    data.m_Ptr = ptr;
  }

  return ptr;
}

template <nsAllocatorTrackingMode TrackingMode, bool OverwriteMemoryOnReset>
void nsLinearAllocator<TrackingMode, OverwriteMemoryOnReset>::Deallocate(void* pPtr)
{
  NS_LOCK(m_Mutex);

  nsUInt32 uiIndex;
  if (m_PtrToDestructDataIndexTable.Remove(pPtr, &uiIndex))
  {
    auto& data = m_DestructData[uiIndex];
    data.m_Func = nullptr;
    data.m_Ptr = nullptr;
  }

  nsAllocatorWithPolicy<typename nsLinearAllocator<TrackingMode, OverwriteMemoryOnReset>::PolicyStack, TrackingMode>::Deallocate(pPtr);
}

NS_MSVC_ANALYSIS_WARNING_PUSH

// Disable warning for incorrect operator (compiler complains about the TrackingMode bitwise and in the case that flags = None)
// even with the added guard of a check that it can't be 0.
NS_MSVC_ANALYSIS_WARNING_DISABLE(6313)

template <nsAllocatorTrackingMode TrackingMode, bool OverwriteMemoryOnReset>
void nsLinearAllocator<TrackingMode, OverwriteMemoryOnReset>::Reset()
{
  NS_LOCK(m_Mutex);

  for (nsUInt32 i = m_DestructData.GetCount(); i-- > 0;)
  {
    auto& data = m_DestructData[i];
    if (data.m_Func != nullptr)
      data.m_Func(data.m_Ptr);
  }

  m_DestructData.Clear();
  m_PtrToDestructDataIndexTable.Clear();

  this->m_allocator.Reset();
  if constexpr (TrackingMode >= nsAllocatorTrackingMode::AllocationStats)
  {
    nsMemoryTracker::RemoveAllAllocations(this->m_Id);
  }
  else if constexpr (TrackingMode >= nsAllocatorTrackingMode::Basics)
  {
    nsAllocator::Stats stats;
    this->m_allocator.FillStats(stats);

    nsMemoryTracker::SetAllocatorStats(this->m_Id, stats);
  }
}
NS_MSVC_ANALYSIS_WARNING_POP

template <wdUInt32 TrackingFlags>
wdStackAllocator<TrackingFlags>::wdStackAllocator(const char* szName, wdAllocatorBase* pParent)
  : wdAllocator<wdMemoryPolicies::wdStackAllocation, TrackingFlags>(szName, pParent)
  , m_DestructData(pParent)
  , m_PtrToDestructDataIndexTable(pParent)
{
}

template <wdUInt32 TrackingFlags>
wdStackAllocator<TrackingFlags>::~wdStackAllocator()
{
  Reset();
}

template <wdUInt32 TrackingFlags>
void* wdStackAllocator<TrackingFlags>::Allocate(size_t uiSize, size_t uiAlign, wdMemoryUtils::DestructorFunction destructorFunc)
{
  WD_LOCK(m_Mutex);

  void* ptr = wdAllocator<wdMemoryPolicies::wdStackAllocation, TrackingFlags>::Allocate(uiSize, uiAlign, destructorFunc);

  if (destructorFunc != nullptr)
  {
    wdUInt32 uiIndex = m_DestructData.GetCount();
    m_PtrToDestructDataIndexTable.Insert(ptr, uiIndex);

    auto& data = m_DestructData.ExpandAndGetRef();
    data.m_Func = destructorFunc;
    data.m_Ptr = ptr;
  }

  return ptr;
}

template <wdUInt32 TrackingFlags>
void wdStackAllocator<TrackingFlags>::Deallocate(void* pPtr)
{
  WD_LOCK(m_Mutex);

  wdUInt32 uiIndex;
  if (m_PtrToDestructDataIndexTable.Remove(pPtr, &uiIndex))
  {
    auto& data = m_DestructData[uiIndex];
    data.m_Func = nullptr;
    data.m_Ptr = nullptr;
  }

  wdAllocator<wdMemoryPolicies::wdStackAllocation, TrackingFlags>::Deallocate(pPtr);
}

WD_MSVC_ANALYSIS_WARNING_PUSH

// Disable warning for incorrect operator (compiler complains about the TrackingFlags bitwise and in the case that flags = None)
// even with the added guard of a check that it can't be 0.
WD_MSVC_ANALYSIS_WARNING_DISABLE(6313)

template <wdUInt32 TrackingFlags>
void wdStackAllocator<TrackingFlags>::Reset()
{
  WD_LOCK(m_Mutex);

  for (wdUInt32 i = m_DestructData.GetCount(); i-- > 0;)
  {
    auto& data = m_DestructData[i];
    if (data.m_Func != nullptr)
      data.m_Func(data.m_Ptr);
  }
  m_DestructData.Clear();
  m_PtrToDestructDataIndexTable.Clear();

  this->m_allocator.Reset();
  if ((TrackingFlags & wdMemoryTrackingFlags::EnableAllocationTracking) != 0)
  {
    wdMemoryTracker::RemoveAllAllocations(this->m_Id);
  }
  else if ((TrackingFlags & wdMemoryTrackingFlags::RegisterAllocator) != 0)
  {
    wdAllocatorBase::Stats stats;
    this->m_allocator.FillStats(stats);

    wdMemoryTracker::SetAllocatorStats(this->m_Id, stats);
  }
}
WD_MSVC_ANALYSIS_WARNING_POP

namespace wdInternal
{
  template <typename AllocationPolicy, wdUInt32 TrackingFlags>
  class wdAllocatorImpl : public wdAllocatorBase
  {
  public:
    wdAllocatorImpl(const char* szName, wdAllocatorBase* pParent);
    ~wdAllocatorImpl();

    // wdAllocatorBase implementation
    virtual void* Allocate(size_t uiSize, size_t uiAlign, wdMemoryUtils::DestructorFunction destructorFunc = nullptr) override;
    virtual void Deallocate(void* pPtr) override;
    virtual size_t AllocatedSize(const void* pPtr) override;
    virtual wdAllocatorId GetId() const override;
    virtual Stats GetStats() const override;

    wdAllocatorBase* GetParent() const;

  protected:
    AllocationPolicy m_allocator;

    wdAllocatorId m_Id;
    wdThreadID m_ThreadID;
  };

  template <typename AllocationPolicy, wdUInt32 TrackingFlags, bool HasReallocate>
  class wdAllocatorMixinReallocate : public wdAllocatorImpl<AllocationPolicy, TrackingFlags>
  {
  public:
    wdAllocatorMixinReallocate(const char* szName, wdAllocatorBase* pParent);
  };

  template <typename AllocationPolicy, wdUInt32 TrackingFlags>
  class wdAllocatorMixinReallocate<AllocationPolicy, TrackingFlags, true> : public wdAllocatorImpl<AllocationPolicy, TrackingFlags>
  {
  public:
    wdAllocatorMixinReallocate(const char* szName, wdAllocatorBase* pParent);
    virtual void* Reallocate(void* pPtr, size_t uiCurrentSize, size_t uiNewSize, size_t uiAlign) override;
  };
}; // namespace wdInternal

template <typename A, wdUInt32 TrackingFlags>
WD_FORCE_INLINE wdInternal::wdAllocatorImpl<A, TrackingFlags>::wdAllocatorImpl(const char* szName, wdAllocatorBase* pParent /* = nullptr */)
  : m_allocator(pParent)
  , m_ThreadID(wdThreadUtils::GetCurrentThreadID())
{
  if ((TrackingFlags & wdMemoryTrackingFlags::RegisterAllocator) != 0)
  {
    WD_CHECK_AT_COMPILETIME_MSG((TrackingFlags & ~wdMemoryTrackingFlags::All) == 0, "Invalid tracking flags");
    const wdUInt32 uiTrackingFlags = TrackingFlags;
    wdBitflags<wdMemoryTrackingFlags> flags = *reinterpret_cast<const wdBitflags<wdMemoryTrackingFlags>*>(&uiTrackingFlags);
    this->m_Id = wdMemoryTracker::RegisterAllocator(szName, flags, pParent != nullptr ? pParent->GetId() : wdAllocatorId());
  }
}

template <typename A, wdUInt32 TrackingFlags>
wdInternal::wdAllocatorImpl<A, TrackingFlags>::~wdAllocatorImpl()
{
  // WD_ASSERT_RELEASE(m_ThreadID == wdThreadUtils::GetCurrentThreadID(), "Allocator is deleted from another thread");

  if ((TrackingFlags & wdMemoryTrackingFlags::RegisterAllocator) != 0)
  {
    wdMemoryTracker::DeregisterAllocator(this->m_Id);
  }
}

template <typename A, wdUInt32 TrackingFlags>
void* wdInternal::wdAllocatorImpl<A, TrackingFlags>::Allocate(size_t uiSize, size_t uiAlign, wdMemoryUtils::DestructorFunction destructorFunc)
{
  // zero size allocations always return nullptr without tracking (since deallocate nullptr is ignored)
  if (uiSize == 0)
    return nullptr;

  WD_ASSERT_DEBUG(wdMath::IsPowerOf2((wdUInt32)uiAlign), "Alignment must be power of two");

  wdTime fAllocationTime = wdTime::Now();

  void* ptr = m_allocator.Allocate(uiSize, uiAlign);
  WD_ASSERT_DEV(ptr != nullptr, "Could not allocate {0} bytes. Out of memory?", uiSize);

  if ((TrackingFlags & wdMemoryTrackingFlags::EnableAllocationTracking) != 0)
  {
    wdBitflags<wdMemoryTrackingFlags> flags;
    flags.SetValue(TrackingFlags);

    wdMemoryTracker::AddAllocation(this->m_Id, flags, ptr, uiSize, uiAlign, wdTime::Now() - fAllocationTime);
  }

  return ptr;
}

template <typename A, wdUInt32 TrackingFlags>
void wdInternal::wdAllocatorImpl<A, TrackingFlags>::Deallocate(void* pPtr)
{
  if ((TrackingFlags & wdMemoryTrackingFlags::EnableAllocationTracking) != 0)
  {
    wdMemoryTracker::RemoveAllocation(this->m_Id, pPtr);
  }

  m_allocator.Deallocate(pPtr);
}

template <typename A, wdUInt32 TrackingFlags>
size_t wdInternal::wdAllocatorImpl<A, TrackingFlags>::AllocatedSize(const void* pPtr)
{
  if ((TrackingFlags & wdMemoryTrackingFlags::EnableAllocationTracking) != 0)
  {
    return wdMemoryTracker::GetAllocationInfo(this->m_Id, pPtr).m_uiSize;
  }

  return 0;
}

template <typename A, wdUInt32 TrackingFlags>
wdAllocatorId wdInternal::wdAllocatorImpl<A, TrackingFlags>::GetId() const
{
  return this->m_Id;
}

template <typename A, wdUInt32 TrackingFlags>
wdAllocatorBase::Stats wdInternal::wdAllocatorImpl<A, TrackingFlags>::GetStats() const
{
  if ((TrackingFlags & wdMemoryTrackingFlags::RegisterAllocator) != 0)
  {
    return wdMemoryTracker::GetAllocatorStats(this->m_Id);
  }

  return Stats();
}

template <typename A, wdUInt32 TrackingFlags>
WD_ALWAYS_INLINE wdAllocatorBase* wdInternal::wdAllocatorImpl<A, TrackingFlags>::GetParent() const
{
  return m_allocator.GetParent();
}

template <typename A, wdUInt32 TrackingFlags, bool HasReallocate>
wdInternal::wdAllocatorMixinReallocate<A, TrackingFlags, HasReallocate>::wdAllocatorMixinReallocate(const char* szName, wdAllocatorBase* pParent)
  : wdAllocatorImpl<A, TrackingFlags>(szName, pParent)
{
}

template <typename A, wdUInt32 TrackingFlags>
wdInternal::wdAllocatorMixinReallocate<A, TrackingFlags, true>::wdAllocatorMixinReallocate(const char* szName, wdAllocatorBase* pParent)
  : wdAllocatorImpl<A, TrackingFlags>(szName, pParent)
{
}

template <typename A, wdUInt32 TrackingFlags>
void* wdInternal::wdAllocatorMixinReallocate<A, TrackingFlags, true>::Reallocate(void* pPtr, size_t uiCurrentSize, size_t uiNewSize, size_t uiAlign)
{
  if ((TrackingFlags & wdMemoryTrackingFlags::EnableAllocationTracking) != 0)
  {
    wdMemoryTracker::RemoveAllocation(this->m_Id, pPtr);
  }

  wdTime fAllocationTime = wdTime::Now();

  void* pNewMem = this->m_allocator.Reallocate(pPtr, uiCurrentSize, uiNewSize, uiAlign);

  if ((TrackingFlags & wdMemoryTrackingFlags::EnableAllocationTracking) != 0)
  {
    wdBitflags<wdMemoryTrackingFlags> flags;
    flags.SetValue(TrackingFlags);

    wdMemoryTracker::AddAllocation(this->m_Id, flags, pNewMem, uiNewSize, uiAlign, wdTime::Now() - fAllocationTime);
  }
  return pNewMem;
}

namespace nsInternal
{
  template <typename AllocationPolicy, nsAllocatorTrackingMode TrackingMode>
  class nsAllocatorImpl : public nsAllocator
  {
  public:
    nsAllocatorImpl(nsStringView sName, nsAllocator* pParent);
    ~nsAllocatorImpl();

    // nsAllocator implementation
    virtual void* Allocate(size_t uiSize, size_t uiAlign, nsMemoryUtils::DestructorFunction destructorFunc = nullptr) override;
    virtual void Deallocate(void* pPtr) override;
    virtual size_t AllocatedSize(const void* pPtr) override;
    virtual nsAllocatorId GetId() const override;
    virtual Stats GetStats() const override;

    nsAllocator* GetParent() const;

  protected:
    AllocationPolicy m_allocator;

    nsAllocatorId m_Id;
    nsThreadID m_ThreadID;
  };

  template <typename AllocationPolicy, nsAllocatorTrackingMode TrackingMode, bool HasReallocate>
  class nsAllocatorMixinReallocate : public nsAllocatorImpl<AllocationPolicy, TrackingMode>
  {
  public:
    nsAllocatorMixinReallocate(nsStringView sName, nsAllocator* pParent);
  };

  template <typename AllocationPolicy, nsAllocatorTrackingMode TrackingMode>
  class nsAllocatorMixinReallocate<AllocationPolicy, TrackingMode, true> : public nsAllocatorImpl<AllocationPolicy, TrackingMode>
  {
  public:
    nsAllocatorMixinReallocate(nsStringView sName, nsAllocator* pParent);
    virtual void* Reallocate(void* pPtr, size_t uiCurrentSize, size_t uiNewSize, size_t uiAlign) override;
  };
}; // namespace nsInternal

template <typename A, nsAllocatorTrackingMode TrackingMode>
NS_FORCE_INLINE nsInternal::nsAllocatorImpl<A, TrackingMode>::nsAllocatorImpl(nsStringView sName, nsAllocator* pParent /* = nullptr */)
  : m_allocator(pParent)
  , m_ThreadID(nsThreadUtils::GetCurrentThreadID())
{
  if constexpr (TrackingMode >= nsAllocatorTrackingMode::Basics)
  {
    this->m_Id = nsMemoryTracker::RegisterAllocator(sName, TrackingMode, pParent != nullptr ? pParent->GetId() : nsAllocatorId());
  }
}

template <typename A, nsAllocatorTrackingMode TrackingMode>
nsInternal::nsAllocatorImpl<A, TrackingMode>::~nsAllocatorImpl()
{
  if constexpr (TrackingMode >= nsAllocatorTrackingMode::Basics)
  {
    nsMemoryTracker::DeregisterAllocator(this->m_Id);
  }
}

template <typename A, nsAllocatorTrackingMode TrackingMode>
void* nsInternal::nsAllocatorImpl<A, TrackingMode>::Allocate(size_t uiSize, size_t uiAlign, nsMemoryUtils::DestructorFunction destructorFunc)
{
  // zero size allocations always return nullptr without tracking (since deallocate nullptr is ignored)
  if (uiSize == 0)
    return nullptr;

  NS_ASSERT_DEBUG(nsMath::IsPowerOf2((nsUInt32)uiAlign), "Alignment must be power of two");

  nsTime fAllocationTime = nsTime::Now();

  void* ptr = m_allocator.Allocate(uiSize, uiAlign);
  NS_ASSERT_DEV(ptr != nullptr, "Could not allocate {0} bytes. Out of memory?", uiSize);

  if constexpr (TrackingMode >= nsAllocatorTrackingMode::AllocationStats)
  {
    nsMemoryTracker::AddAllocation(this->m_Id, TrackingMode, ptr, uiSize, uiAlign, nsTime::Now() - fAllocationTime);
  }

  return ptr;
}

template <typename A, nsAllocatorTrackingMode TrackingMode>
void nsInternal::nsAllocatorImpl<A, TrackingMode>::Deallocate(void* pPtr)
{
  if constexpr (TrackingMode >= nsAllocatorTrackingMode::AllocationStats)
  {
    nsMemoryTracker::RemoveAllocation(this->m_Id, pPtr);
  }

  m_allocator.Deallocate(pPtr);
}

template <typename A, nsAllocatorTrackingMode TrackingMode>
size_t nsInternal::nsAllocatorImpl<A, TrackingMode>::AllocatedSize(const void* pPtr)
{
  if constexpr (TrackingMode >= nsAllocatorTrackingMode::AllocationStats)
  {
    return nsMemoryTracker::GetAllocationInfo(this->m_Id, pPtr).m_uiSize;
  }
  else
  {
    return 0;
  }
}

template <typename A, nsAllocatorTrackingMode TrackingMode>
nsAllocatorId nsInternal::nsAllocatorImpl<A, TrackingMode>::GetId() const
{
  return this->m_Id;
}

template <typename A, nsAllocatorTrackingMode TrackingMode>
nsAllocator::Stats nsInternal::nsAllocatorImpl<A, TrackingMode>::GetStats() const
{
  if constexpr (TrackingMode >= nsAllocatorTrackingMode::Basics)
  {
    return nsMemoryTracker::GetAllocatorStats(this->m_Id);
  }
  else
  {
    return Stats();
  }
}

template <typename A, nsAllocatorTrackingMode TrackingMode>
NS_ALWAYS_INLINE nsAllocator* nsInternal::nsAllocatorImpl<A, TrackingMode>::GetParent() const
{
  return m_allocator.GetParent();
}

template <typename A, nsAllocatorTrackingMode TrackingMode, bool HasReallocate>
nsInternal::nsAllocatorMixinReallocate<A, TrackingMode, HasReallocate>::nsAllocatorMixinReallocate(nsStringView sName, nsAllocator* pParent)
  : nsAllocatorImpl<A, TrackingMode>(sName, pParent)
{
}

template <typename A, nsAllocatorTrackingMode TrackingMode>
nsInternal::nsAllocatorMixinReallocate<A, TrackingMode, true>::nsAllocatorMixinReallocate(nsStringView sName, nsAllocator* pParent)
  : nsAllocatorImpl<A, TrackingMode>(sName, pParent)
{
}

template <typename A, nsAllocatorTrackingMode TrackingMode>
void* nsInternal::nsAllocatorMixinReallocate<A, TrackingMode, true>::Reallocate(void* pPtr, size_t uiCurrentSize, size_t uiNewSize, size_t uiAlign)
{
  if constexpr (TrackingMode >= nsAllocatorTrackingMode::AllocationStats)
  {
    nsMemoryTracker::RemoveAllocation(this->m_Id, pPtr);
  }

  nsTime fAllocationTime = nsTime::Now();

  void* pNewMem = this->m_allocator.Reallocate(pPtr, uiCurrentSize, uiNewSize, uiAlign);

  if constexpr (TrackingMode >= nsAllocatorTrackingMode::AllocationStats)
  {
    nsMemoryTracker::AddAllocation(this->m_Id, TrackingMode, pNewMem, uiNewSize, uiAlign, nsTime::Now() - fAllocationTime);
  }

  return pNewMem;
}

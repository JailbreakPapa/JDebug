
template <typename T, wdUInt32 SizeInBytes>
WD_ALWAYS_INLINE wdDataBlock<T, SizeInBytes>::wdDataBlock(T* pData, wdUInt32 uiCount)
{
  m_pData = pData;
  m_uiCount = uiCount;
}

template <typename T, wdUInt32 SizeInBytes>
WD_FORCE_INLINE T* wdDataBlock<T, SizeInBytes>::ReserveBack()
{
  WD_ASSERT_DEV(m_uiCount < CAPACITY, "Block is full.");
  return m_pData + m_uiCount++;
}

template <typename T, wdUInt32 SizeInBytes>
WD_FORCE_INLINE T* wdDataBlock<T, SizeInBytes>::PopBack()
{
  WD_ASSERT_DEV(m_uiCount > 0, "Block is empty");
  --m_uiCount;
  return m_pData + m_uiCount;
}

template <typename T, wdUInt32 SizeInBytes>
WD_ALWAYS_INLINE bool wdDataBlock<T, SizeInBytes>::IsEmpty() const
{
  return m_uiCount == 0;
}

template <typename T, wdUInt32 SizeInBytes>
WD_ALWAYS_INLINE bool wdDataBlock<T, SizeInBytes>::IsFull() const
{
  return m_uiCount == CAPACITY;
}

template <typename T, wdUInt32 SizeInBytes>
WD_FORCE_INLINE T& wdDataBlock<T, SizeInBytes>::operator[](wdUInt32 uiIndex) const
{
  WD_ASSERT_DEV(uiIndex < m_uiCount, "Out of bounds access. Data block has {0} elements, trying to access element at index {1}.", m_uiCount, uiIndex);
  return m_pData[uiIndex];
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <wdUInt32 BlockSize>
wdLargeBlockAllocator<BlockSize>::wdLargeBlockAllocator(const char* szName, wdAllocatorBase* pParent, wdBitflags<wdMemoryTrackingFlags> flags)
  : m_TrackingFlags(flags)
  , m_SuperBlocks(pParent)
  , m_FreeBlocks(pParent)
{
  WD_CHECK_AT_COMPILETIME_MSG(BlockSize >= 4096, "Block size must be 4096 or bigger");

  m_Id = wdMemoryTracker::RegisterAllocator(szName, flags, wdPageAllocator::GetId());
  m_ThreadID = wdThreadUtils::GetCurrentThreadID();

  const wdUInt32 uiPageSize = wdSystemInformation::Get().GetMemoryPageSize();
  WD_IGNORE_UNUSED(uiPageSize);
  WD_ASSERT_DEV(uiPageSize <= BlockSize, "Memory Page size is bigger than block size.");
  WD_ASSERT_DEV(BlockSize % uiPageSize == 0, "Blocksize ({0}) must be a multiple of page size ({1})", BlockSize, uiPageSize);
}

template <wdUInt32 BlockSize>
wdLargeBlockAllocator<BlockSize>::~wdLargeBlockAllocator()
{
  WD_ASSERT_RELEASE(m_ThreadID == wdThreadUtils::GetCurrentThreadID(), "Allocator is deleted from another thread");
  wdMemoryTracker::DeregisterAllocator(m_Id);

  for (wdUInt32 i = 0; i < m_SuperBlocks.GetCount(); ++i)
  {
    wdPageAllocator::DeallocatePage(m_SuperBlocks[i].m_pBasePtr);
  }
}

template <wdUInt32 BlockSize>
template <typename T>
WD_FORCE_INLINE wdDataBlock<T, BlockSize> wdLargeBlockAllocator<BlockSize>::AllocateBlock()
{
  struct Helper
  {
    enum
    {
      BLOCK_CAPACITY = wdDataBlock<T, BlockSize>::CAPACITY
    };
  };

  WD_CHECK_AT_COMPILETIME_MSG(
    Helper::BLOCK_CAPACITY >= 1, "Type is too big for block allocation. Consider using regular heap allocation instead or increase the block size.");

  wdDataBlock<T, BlockSize> block(static_cast<T*>(Allocate(WD_ALIGNMENT_OF(T))), 0);
  return block;
}

template <wdUInt32 BlockSize>
template <typename T>
WD_FORCE_INLINE void wdLargeBlockAllocator<BlockSize>::DeallocateBlock(wdDataBlock<T, BlockSize>& inout_block)
{
  Deallocate(inout_block.m_pData);
  inout_block.m_pData = nullptr;
  inout_block.m_uiCount = 0;
}

template <wdUInt32 BlockSize>
WD_ALWAYS_INLINE const char* wdLargeBlockAllocator<BlockSize>::GetName() const
{
  return wdMemoryTracker::GetAllocatorName(m_Id);
}

template <wdUInt32 BlockSize>
WD_ALWAYS_INLINE wdAllocatorId wdLargeBlockAllocator<BlockSize>::GetId() const
{
  return m_Id;
}

template <wdUInt32 BlockSize>
WD_ALWAYS_INLINE const wdAllocatorBase::Stats& wdLargeBlockAllocator<BlockSize>::GetStats() const
{
  return wdMemoryTracker::GetAllocatorStats(m_Id);
}

template <wdUInt32 BlockSize>
void* wdLargeBlockAllocator<BlockSize>::Allocate(size_t uiAlign)
{
  WD_ASSERT_RELEASE(wdMath::IsPowerOf2((wdUInt32)uiAlign), "Alignment must be power of two");

  wdTime fAllocationTime = wdTime::Now();

  WD_LOCK(m_Mutex);

  void* ptr = nullptr;

  if (!m_FreeBlocks.IsEmpty())
  {
    // Re-use a super block
    wdUInt32 uiFreeBlockIndex = m_FreeBlocks.PeekBack();
    m_FreeBlocks.PopBack();

    const wdUInt32 uiSuperBlockIndex = uiFreeBlockIndex / SuperBlock::NUM_BLOCKS;
    const wdUInt32 uiInnerBlockIndex = uiFreeBlockIndex & (SuperBlock::NUM_BLOCKS - 1);
    SuperBlock& superBlock = m_SuperBlocks[uiSuperBlockIndex];
    ++superBlock.m_uiUsedBlocks;

    ptr = wdMemoryUtils::AddByteOffset(superBlock.m_pBasePtr, uiInnerBlockIndex * BlockSize);
  }
  else
  {
    // Allocate a new super block
    void* pMemory = wdPageAllocator::AllocatePage(SuperBlock::SIZE_IN_BYTES);
    WD_CHECK_ALIGNMENT(pMemory, uiAlign);

    SuperBlock superBlock;
    superBlock.m_pBasePtr = pMemory;
    superBlock.m_uiUsedBlocks = 1;

    m_SuperBlocks.PushBack(superBlock);

    const wdUInt32 uiBlockBaseIndex = (m_SuperBlocks.GetCount() - 1) * SuperBlock::NUM_BLOCKS;
    for (wdUInt32 i = SuperBlock::NUM_BLOCKS - 1; i > 0; --i)
    {
      m_FreeBlocks.PushBack(uiBlockBaseIndex + i);
    }

    ptr = pMemory;
  }

  if ((m_TrackingFlags & wdMemoryTrackingFlags::EnableAllocationTracking) != 0)
  {
    wdMemoryTracker::AddAllocation(m_Id, m_TrackingFlags, ptr, BlockSize, uiAlign, wdTime::Now() - fAllocationTime);
  }

  return ptr;
}

template <wdUInt32 BlockSize>
void wdLargeBlockAllocator<BlockSize>::Deallocate(void* ptr)
{
  WD_LOCK(m_Mutex);

  if ((m_TrackingFlags & wdMemoryTrackingFlags::EnableAllocationTracking) != 0)
  {
    wdMemoryTracker::RemoveAllocation(m_Id, ptr);
  }

  // find super block
  bool bFound = false;
  wdUInt32 uiSuperBlockIndex = m_SuperBlocks.GetCount();
  ptrdiff_t diff = 0;

  for (; uiSuperBlockIndex-- > 0;)
  {
    diff = (char*)ptr - (char*)m_SuperBlocks[uiSuperBlockIndex].m_pBasePtr;
    if (diff >= 0 && diff < SuperBlock::SIZE_IN_BYTES)
    {
      bFound = true;
      break;
    }
  }

  WD_ASSERT_DEV(bFound, "'{0}' was not allocated with this allocator", wdArgP(ptr));

  SuperBlock& superBlock = m_SuperBlocks[uiSuperBlockIndex];
  --superBlock.m_uiUsedBlocks;

  if (superBlock.m_uiUsedBlocks == 0 && m_FreeBlocks.GetCount() > SuperBlock::NUM_BLOCKS * 4)
  {
    // give memory back
    wdPageAllocator::DeallocatePage(superBlock.m_pBasePtr);

    m_SuperBlocks.RemoveAtAndSwap(uiSuperBlockIndex);
    const wdUInt32 uiLastSuperBlockIndex = m_SuperBlocks.GetCount();

    // patch free list
    for (wdUInt32 i = 0; i < m_FreeBlocks.GetCount(); ++i)
    {
      const wdUInt32 uiIndex = m_FreeBlocks[i];
      const wdUInt32 uiSBIndex = uiIndex / SuperBlock::NUM_BLOCKS;

      if (uiSBIndex == uiSuperBlockIndex)
      {
        // points to the block we just removed
        m_FreeBlocks.RemoveAtAndSwap(i);
        --i;
      }
      else if (uiSBIndex == uiLastSuperBlockIndex)
      {
        // points to the block we just swapped
        m_FreeBlocks[i] = uiSuperBlockIndex * SuperBlock::NUM_BLOCKS + (uiIndex & (SuperBlock::NUM_BLOCKS - 1));
      }
    }
  }
  else
  {
    // add block to free list
    const wdUInt32 uiInnerBlockIndex = (wdUInt32)(diff / BlockSize);
    m_FreeBlocks.PushBack(uiSuperBlockIndex * SuperBlock::NUM_BLOCKS + uiInnerBlockIndex);
  }
}

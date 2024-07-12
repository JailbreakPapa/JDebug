
template <typename T, nsUInt32 SizeInBytes>
NS_ALWAYS_INLINE nsDataBlock<T, SizeInBytes>::nsDataBlock(T* pData, nsUInt32 uiCount)
{
  m_pData = pData;
  m_uiCount = uiCount;
}

template <typename T, nsUInt32 SizeInBytes>
NS_FORCE_INLINE T* nsDataBlock<T, SizeInBytes>::ReserveBack()
{
  NS_ASSERT_DEV(m_uiCount < CAPACITY, "Block is full.");
  return m_pData + m_uiCount++;
}

template <typename T, nsUInt32 SizeInBytes>
NS_FORCE_INLINE T* nsDataBlock<T, SizeInBytes>::PopBack()
{
  NS_ASSERT_DEV(m_uiCount > 0, "Block is empty");
  --m_uiCount;
  return m_pData + m_uiCount;
}

template <typename T, nsUInt32 SizeInBytes>
NS_ALWAYS_INLINE bool nsDataBlock<T, SizeInBytes>::IsEmpty() const
{
  return m_uiCount == 0;
}

template <typename T, nsUInt32 SizeInBytes>
NS_ALWAYS_INLINE bool nsDataBlock<T, SizeInBytes>::IsFull() const
{
  return m_uiCount == CAPACITY;
}

template <typename T, nsUInt32 SizeInBytes>
NS_FORCE_INLINE T& nsDataBlock<T, SizeInBytes>::operator[](nsUInt32 uiIndex) const
{
  NS_ASSERT_DEV(uiIndex < m_uiCount, "Out of bounds access. Data block has {0} elements, trying to access element at index {1}.", m_uiCount, uiIndex);
  return m_pData[uiIndex];
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <nsUInt32 BlockSize>
nsLargeBlockAllocator<BlockSize>::nsLargeBlockAllocator(nsStringView sName, nsAllocator* pParent, nsAllocatorTrackingMode mode)
  : m_TrackingMode(mode)
  , m_SuperBlocks(pParent)
  , m_FreeBlocks(pParent)
{
  NS_CHECK_AT_COMPILETIME_MSG(BlockSize >= 4096, "Block size must be 4096 or bigger");

  m_Id = nsMemoryTracker::RegisterAllocator(sName, mode, nsPageAllocator::GetId());
  m_ThreadID = nsThreadUtils::GetCurrentThreadID();

  const nsUInt32 uiPageSize = nsSystemInformation::Get().GetMemoryPageSize();
  NS_IGNORE_UNUSED(uiPageSize);
  NS_ASSERT_DEV(uiPageSize <= BlockSize, "Memory Page size is bigger than block size.");
  NS_ASSERT_DEV(BlockSize % uiPageSize == 0, "Blocksize ({0}) must be a multiple of page size ({1})", BlockSize, uiPageSize);
}

template <nsUInt32 BlockSize>
nsLargeBlockAllocator<BlockSize>::~nsLargeBlockAllocator()
{
  NS_ASSERT_RELEASE(m_ThreadID == nsThreadUtils::GetCurrentThreadID(), "Allocator is deleted from another thread");
  nsMemoryTracker::DeregisterAllocator(m_Id);

  for (nsUInt32 i = 0; i < m_SuperBlocks.GetCount(); ++i)
  {
    nsPageAllocator::DeallocatePage(m_SuperBlocks[i].m_pBasePtr);
  }
}

template <nsUInt32 BlockSize>
template <typename T>
NS_FORCE_INLINE nsDataBlock<T, BlockSize> nsLargeBlockAllocator<BlockSize>::AllocateBlock()
{
  struct Helper
  {
    enum
    {
      BLOCK_CAPACITY = nsDataBlock<T, BlockSize>::CAPACITY
    };
  };

  NS_CHECK_AT_COMPILETIME_MSG(
    Helper::BLOCK_CAPACITY >= 1, "Type is too big for block allocation. Consider using regular heap allocation instead or increase the block size.");

  nsDataBlock<T, BlockSize> block(static_cast<T*>(Allocate(NS_ALIGNMENT_OF(T))), 0);
  return block;
}

template <nsUInt32 BlockSize>
template <typename T>
NS_FORCE_INLINE void nsLargeBlockAllocator<BlockSize>::DeallocateBlock(nsDataBlock<T, BlockSize>& inout_block)
{
  Deallocate(inout_block.m_pData);
  inout_block.m_pData = nullptr;
  inout_block.m_uiCount = 0;
}

template <nsUInt32 BlockSize>
NS_ALWAYS_INLINE nsStringView nsLargeBlockAllocator<BlockSize>::GetName() const
{
  return nsMemoryTracker::GetAllocatorName(m_Id);
}

template <nsUInt32 BlockSize>
NS_ALWAYS_INLINE nsAllocatorId nsLargeBlockAllocator<BlockSize>::GetId() const
{
  return m_Id;
}

template <nsUInt32 BlockSize>
NS_ALWAYS_INLINE const nsAllocator::Stats& nsLargeBlockAllocator<BlockSize>::GetStats() const
{
  return nsMemoryTracker::GetAllocatorStats(m_Id);
}

template <nsUInt32 BlockSize>
void* nsLargeBlockAllocator<BlockSize>::Allocate(size_t uiAlign)
{
  NS_ASSERT_RELEASE(nsMath::IsPowerOf2((nsUInt32)uiAlign), "Alignment must be power of two");

  nsTime fAllocationTime = nsTime::Now();

  NS_LOCK(m_Mutex);

  void* ptr = nullptr;

  if (!m_FreeBlocks.IsEmpty())
  {
    // Re-use a super block
    nsUInt32 uiFreeBlockIndex = m_FreeBlocks.PeekBack();
    m_FreeBlocks.PopBack();

    const nsUInt32 uiSuperBlockIndex = uiFreeBlockIndex / SuperBlock::NUM_BLOCKS;
    const nsUInt32 uiInnerBlockIndex = uiFreeBlockIndex & (SuperBlock::NUM_BLOCKS - 1);
    SuperBlock& superBlock = m_SuperBlocks[uiSuperBlockIndex];
    ++superBlock.m_uiUsedBlocks;

    ptr = nsMemoryUtils::AddByteOffset(superBlock.m_pBasePtr, uiInnerBlockIndex * BlockSize);
  }
  else
  {
    // Allocate a new super block
    void* pMemory = nsPageAllocator::AllocatePage(SuperBlock::SIZE_IN_BYTES);
    NS_CHECK_ALIGNMENT(pMemory, uiAlign);

    SuperBlock superBlock;
    superBlock.m_pBasePtr = pMemory;
    superBlock.m_uiUsedBlocks = 1;

    m_SuperBlocks.PushBack(superBlock);

    const nsUInt32 uiBlockBaseIndex = (m_SuperBlocks.GetCount() - 1) * SuperBlock::NUM_BLOCKS;
    for (nsUInt32 i = SuperBlock::NUM_BLOCKS - 1; i > 0; --i)
    {
      m_FreeBlocks.PushBack(uiBlockBaseIndex + i);
    }

    ptr = pMemory;
  }

  if (m_TrackingMode >= nsAllocatorTrackingMode::AllocationStats)
  {
    nsMemoryTracker::AddAllocation(m_Id, m_TrackingMode, ptr, BlockSize, uiAlign, nsTime::Now() - fAllocationTime);
  }

  return ptr;
}

template <nsUInt32 BlockSize>
void nsLargeBlockAllocator<BlockSize>::Deallocate(void* ptr)
{
  NS_LOCK(m_Mutex);

  if (m_TrackingMode >= nsAllocatorTrackingMode::AllocationStats)
  {
    nsMemoryTracker::RemoveAllocation(m_Id, ptr);
  }

  // find super block
  bool bFound = false;
  nsUInt32 uiSuperBlockIndex = m_SuperBlocks.GetCount();
  std::ptrdiff_t diff = 0;

  for (; uiSuperBlockIndex-- > 0;)
  {
    diff = (char*)ptr - (char*)m_SuperBlocks[uiSuperBlockIndex].m_pBasePtr;
    if (diff >= 0 && diff < SuperBlock::SIZE_IN_BYTES)
    {
      bFound = true;
      break;
    }
  }

  NS_IGNORE_UNUSED(bFound);
  NS_ASSERT_DEV(bFound, "'{0}' was not allocated with this allocator", nsArgP(ptr));

  SuperBlock& superBlock = m_SuperBlocks[uiSuperBlockIndex];
  --superBlock.m_uiUsedBlocks;

  if (superBlock.m_uiUsedBlocks == 0 && m_FreeBlocks.GetCount() > SuperBlock::NUM_BLOCKS * 4)
  {
    // give memory back
    nsPageAllocator::DeallocatePage(superBlock.m_pBasePtr);

    m_SuperBlocks.RemoveAtAndSwap(uiSuperBlockIndex);
    const nsUInt32 uiLastSuperBlockIndex = m_SuperBlocks.GetCount();

    // patch free list
    for (nsUInt32 i = 0; i < m_FreeBlocks.GetCount(); ++i)
    {
      const nsUInt32 uiIndex = m_FreeBlocks[i];
      const nsUInt32 uiSBIndex = uiIndex / SuperBlock::NUM_BLOCKS;

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
    const nsUInt32 uiInnerBlockIndex = (nsUInt32)(diff / BlockSize);
    m_FreeBlocks.PushBack(uiSuperBlockIndex * SuperBlock::NUM_BLOCKS + uiInnerBlockIndex);
  }
}

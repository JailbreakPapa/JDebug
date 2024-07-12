
template <typename T, nsUInt32 BlockSize, nsBlockStorageType::Enum StorageType>
NS_FORCE_INLINE nsBlockStorage<T, BlockSize, StorageType>::ConstIterator::ConstIterator(
  const nsBlockStorage<T, BlockSize, StorageType>& storage, nsUInt32 uiStartIndex, nsUInt32 uiCount)
  : m_Storage(storage)
{
  m_uiCurrentIndex = uiStartIndex;
  m_uiEndIndex = nsMath::Max(uiStartIndex + uiCount, uiCount);

  if (StorageType == nsBlockStorageType::FreeList)
  {
    nsUInt32 uiEndIndex = nsMath::Min(m_uiEndIndex, m_Storage.m_uiCount);
    while (m_uiCurrentIndex < uiEndIndex && !m_Storage.m_UsedEntries.IsBitSet(m_uiCurrentIndex))
    {
      ++m_uiCurrentIndex;
    }
  }
}

template <typename T, nsUInt32 BlockSize, nsBlockStorageType::Enum StorageType>
NS_FORCE_INLINE T& nsBlockStorage<T, BlockSize, StorageType>::ConstIterator::CurrentElement() const
{
  const nsUInt32 uiBlockIndex = m_uiCurrentIndex / nsDataBlock<T, BlockSize>::CAPACITY;
  const nsUInt32 uiInnerIndex = m_uiCurrentIndex - uiBlockIndex * nsDataBlock<T, BlockSize>::CAPACITY;
  return m_Storage.m_Blocks[uiBlockIndex][uiInnerIndex];
}

template <typename T, nsUInt32 BlockSize, nsBlockStorageType::Enum StorageType>
NS_ALWAYS_INLINE const T& nsBlockStorage<T, BlockSize, StorageType>::ConstIterator::operator*() const
{
  return CurrentElement();
}

template <typename T, nsUInt32 BlockSize, nsBlockStorageType::Enum StorageType>
NS_ALWAYS_INLINE const T* nsBlockStorage<T, BlockSize, StorageType>::ConstIterator::operator->() const
{
  return &CurrentElement();
}

template <typename T, nsUInt32 BlockSize, nsBlockStorageType::Enum StorageType>
NS_ALWAYS_INLINE nsBlockStorage<T, BlockSize, StorageType>::ConstIterator::operator const T*() const
{
  return &CurrentElement();
}

template <typename T, nsUInt32 BlockSize, nsBlockStorageType::Enum StorageType>
NS_FORCE_INLINE void nsBlockStorage<T, BlockSize, StorageType>::ConstIterator::Next()
{
  ++m_uiCurrentIndex;

  if (StorageType == nsBlockStorageType::FreeList)
  {
    nsUInt32 uiEndIndex = nsMath::Min(m_uiEndIndex, m_Storage.m_uiCount);
    while (m_uiCurrentIndex < uiEndIndex && !m_Storage.m_UsedEntries.IsBitSet(m_uiCurrentIndex))
    {
      ++m_uiCurrentIndex;
    }
  }
}

template <typename T, nsUInt32 BlockSize, nsBlockStorageType::Enum StorageType>
NS_FORCE_INLINE bool nsBlockStorage<T, BlockSize, StorageType>::ConstIterator::IsValid() const
{
  return m_uiCurrentIndex < nsMath::Min(m_uiEndIndex, m_Storage.m_uiCount);
}

template <typename T, nsUInt32 BlockSize, nsBlockStorageType::Enum StorageType>
NS_ALWAYS_INLINE void nsBlockStorage<T, BlockSize, StorageType>::ConstIterator::operator++()
{
  Next();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T, nsUInt32 BlockSize, nsBlockStorageType::Enum StorageType>
NS_FORCE_INLINE nsBlockStorage<T, BlockSize, StorageType>::Iterator::Iterator(
  const nsBlockStorage<T, BlockSize, StorageType>& storage, nsUInt32 uiStartIndex, nsUInt32 uiCount)
  : ConstIterator(storage, uiStartIndex, uiCount)
{
}

template <typename T, nsUInt32 BlockSize, nsBlockStorageType::Enum StorageType>
NS_ALWAYS_INLINE T& nsBlockStorage<T, BlockSize, StorageType>::Iterator::operator*()
{
  return this->CurrentElement();
}

template <typename T, nsUInt32 BlockSize, nsBlockStorageType::Enum StorageType>
NS_ALWAYS_INLINE T* nsBlockStorage<T, BlockSize, StorageType>::Iterator::operator->()
{
  return &(this->CurrentElement());
}

template <typename T, nsUInt32 BlockSize, nsBlockStorageType::Enum StorageType>
NS_ALWAYS_INLINE nsBlockStorage<T, BlockSize, StorageType>::Iterator::operator T*()
{
  return &(this->CurrentElement());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T, nsUInt32 BlockSize, nsBlockStorageType::Enum StorageType>
NS_FORCE_INLINE nsBlockStorage<T, BlockSize, StorageType>::nsBlockStorage(
  nsLargeBlockAllocator<BlockSize>* pBlockAllocator, nsAllocator* pAllocator)
  : m_pBlockAllocator(pBlockAllocator)
  , m_Blocks(pAllocator)

{
}

template <typename T, nsUInt32 BlockSize, nsBlockStorageType::Enum StorageType>
nsBlockStorage<T, BlockSize, StorageType>::~nsBlockStorage()
{
  Clear();
}

template <typename T, nsUInt32 BlockSize, nsBlockStorageType::Enum StorageType>
void nsBlockStorage<T, BlockSize, StorageType>::Clear()
{
  for (nsUInt32 uiBlockIndex = 0; uiBlockIndex < m_Blocks.GetCount(); ++uiBlockIndex)
  {
    nsDataBlock<T, BlockSize>& block = m_Blocks[uiBlockIndex];

    if (StorageType == nsBlockStorageType::Compact)
    {
      nsMemoryUtils::Destruct(block.m_pData, block.m_uiCount);
    }
    else
    {
      for (nsUInt32 uiInnerIndex = 0; uiInnerIndex < block.m_uiCount; ++uiInnerIndex)
      {
        nsUInt32 uiIndex = uiBlockIndex * nsDataBlock<T, BlockSize>::CAPACITY + uiInnerIndex;
        if (m_UsedEntries.IsBitSet(uiIndex))
        {
          nsMemoryUtils::Destruct(&block.m_pData[uiInnerIndex], 1);
        }
      }
    }

    m_pBlockAllocator->DeallocateBlock(block);
  }

  m_Blocks.Clear();
}

template <typename T, nsUInt32 BlockSize, nsBlockStorageType::Enum StorageType>
T* nsBlockStorage<T, BlockSize, StorageType>::Create()
{
  T* pNewObject = nullptr;
  nsUInt32 uiNewIndex = nsInvalidIndex;

  if (StorageType == nsBlockStorageType::FreeList && m_uiFreelistStart != nsInvalidIndex)
  {
    uiNewIndex = m_uiFreelistStart;

    const nsUInt32 uiBlockIndex = uiNewIndex / nsDataBlock<T, BlockSize>::CAPACITY;
    const nsUInt32 uiInnerIndex = uiNewIndex - uiBlockIndex * nsDataBlock<T, BlockSize>::CAPACITY;

    pNewObject = &(m_Blocks[uiBlockIndex][uiInnerIndex]);

    m_uiFreelistStart = *reinterpret_cast<nsUInt32*>(pNewObject);
  }
  else
  {
    nsDataBlock<T, BlockSize>* pBlock = nullptr;

    if (m_Blocks.GetCount() > 0)
    {
      pBlock = &m_Blocks.PeekBack();
    }

    if (pBlock == nullptr || pBlock->IsFull())
    {
      m_Blocks.PushBack(m_pBlockAllocator->template AllocateBlock<T>());
      pBlock = &m_Blocks.PeekBack();
    }

    pNewObject = pBlock->ReserveBack();
    uiNewIndex = m_uiCount;

    ++m_uiCount;
  }

  nsMemoryUtils::Construct<SkipTrivialTypes>(pNewObject, 1);

  if (StorageType == nsBlockStorageType::FreeList)
  {
    m_UsedEntries.SetCount(m_uiCount);
    m_UsedEntries.SetBit(uiNewIndex);
  }

  return pNewObject;
}

template <typename T, nsUInt32 BlockSize, nsBlockStorageType::Enum StorageType>
NS_FORCE_INLINE void nsBlockStorage<T, BlockSize, StorageType>::Delete(T* pObject)
{
  T* pDummy;
  Delete(pObject, pDummy);
}

template <typename T, nsUInt32 BlockSize, nsBlockStorageType::Enum StorageType>
void nsBlockStorage<T, BlockSize, StorageType>::Delete(T* pObject, T*& out_pMovedObject)
{
  Delete(pObject, out_pMovedObject, nsTraitInt<StorageType>());
}

template <typename T, nsUInt32 BlockSize, nsBlockStorageType::Enum StorageType>
NS_ALWAYS_INLINE nsUInt32 nsBlockStorage<T, BlockSize, StorageType>::GetCount() const
{
  return m_uiCount;
}

template <typename T, nsUInt32 BlockSize, nsBlockStorageType::Enum StorageType>
NS_ALWAYS_INLINE typename nsBlockStorage<T, BlockSize, StorageType>::Iterator nsBlockStorage<T, BlockSize, StorageType>::GetIterator(
  nsUInt32 uiStartIndex /*= 0*/, nsUInt32 uiCount /*= nsInvalidIndex*/)
{
  return Iterator(*this, uiStartIndex, uiCount);
}

template <typename T, nsUInt32 BlockSize, nsBlockStorageType::Enum StorageType>
NS_ALWAYS_INLINE typename nsBlockStorage<T, BlockSize, StorageType>::ConstIterator nsBlockStorage<T, BlockSize, StorageType>::GetIterator(
  nsUInt32 uiStartIndex /*= 0*/, nsUInt32 uiCount /*= nsInvalidIndex*/) const
{
  return ConstIterator(*this, uiStartIndex, uiCount);
}

template <typename T, nsUInt32 BlockSize, nsBlockStorageType::Enum StorageType>
NS_FORCE_INLINE void nsBlockStorage<T, BlockSize, StorageType>::Delete(T* pObject, T*& out_pMovedObject, nsTraitInt<nsBlockStorageType::Compact>)
{
  nsDataBlock<T, BlockSize>& lastBlock = m_Blocks.PeekBack();
  T* pLast = lastBlock.PopBack();

  --m_uiCount;
  if (pObject != pLast)
  {
    nsMemoryUtils::Relocate(pObject, pLast, 1);
  }
  else
  {
    nsMemoryUtils::Destruct(pLast, 1);
  }

  out_pMovedObject = pLast;

  if (lastBlock.IsEmpty())
  {
    m_pBlockAllocator->DeallocateBlock(lastBlock);
    m_Blocks.PopBack();
  }
}

template <typename T, nsUInt32 BlockSize, nsBlockStorageType::Enum StorageType>
NS_FORCE_INLINE void nsBlockStorage<T, BlockSize, StorageType>::Delete(T* pObject, T*& out_pMovedObject, nsTraitInt<nsBlockStorageType::FreeList>)
{
  nsUInt32 uiIndex = nsInvalidIndex;
  for (nsUInt32 uiBlockIndex = 0; uiBlockIndex < m_Blocks.GetCount(); ++uiBlockIndex)
  {
    std::ptrdiff_t diff = pObject - m_Blocks[uiBlockIndex].m_pData;
    if (diff >= 0 && diff < nsDataBlock<T, BlockSize>::CAPACITY)
    {
      uiIndex = uiBlockIndex * nsDataBlock<T, BlockSize>::CAPACITY + (nsInt32)diff;
      break;
    }
  }

  NS_ASSERT_DEV(uiIndex != nsInvalidIndex, "Invalid object {0} was not found in block storage.", nsArgP(pObject));

  m_UsedEntries.ClearBit(uiIndex);

  out_pMovedObject = pObject;
  nsMemoryUtils::Destruct(pObject, 1);

  *reinterpret_cast<nsUInt32*>(pObject) = m_uiFreelistStart;
  m_uiFreelistStart = uiIndex;
}

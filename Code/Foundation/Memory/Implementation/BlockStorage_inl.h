
template <typename T, wdUInt32 BlockSize, wdBlockStorageType::Enum StorageType>
WD_FORCE_INLINE wdBlockStorage<T, BlockSize, StorageType>::ConstIterator::ConstIterator(
  const wdBlockStorage<T, BlockSize, StorageType>& storage, wdUInt32 uiStartIndex, wdUInt32 uiCount)
  : m_Storage(storage)
{
  m_uiCurrentIndex = uiStartIndex;
  m_uiEndIndex = wdMath::Max(uiStartIndex + uiCount, uiCount);

  if (StorageType == wdBlockStorageType::FreeList)
  {
    wdUInt32 uiEndIndex = wdMath::Min(m_uiEndIndex, m_Storage.m_uiCount);
    while (m_uiCurrentIndex < uiEndIndex && !m_Storage.m_UsedEntries.IsBitSet(m_uiCurrentIndex))
    {
      ++m_uiCurrentIndex;
    }
  }
}

template <typename T, wdUInt32 BlockSize, wdBlockStorageType::Enum StorageType>
WD_FORCE_INLINE T& wdBlockStorage<T, BlockSize, StorageType>::ConstIterator::CurrentElement() const
{
  const wdUInt32 uiBlockIndex = m_uiCurrentIndex / wdDataBlock<T, BlockSize>::CAPACITY;
  const wdUInt32 uiInnerIndex = m_uiCurrentIndex - uiBlockIndex * wdDataBlock<T, BlockSize>::CAPACITY;
  return m_Storage.m_Blocks[uiBlockIndex][uiInnerIndex];
}

template <typename T, wdUInt32 BlockSize, wdBlockStorageType::Enum StorageType>
WD_ALWAYS_INLINE const T& wdBlockStorage<T, BlockSize, StorageType>::ConstIterator::operator*() const
{
  return CurrentElement();
}

template <typename T, wdUInt32 BlockSize, wdBlockStorageType::Enum StorageType>
WD_ALWAYS_INLINE const T* wdBlockStorage<T, BlockSize, StorageType>::ConstIterator::operator->() const
{
  return &CurrentElement();
}

template <typename T, wdUInt32 BlockSize, wdBlockStorageType::Enum StorageType>
WD_ALWAYS_INLINE wdBlockStorage<T, BlockSize, StorageType>::ConstIterator::operator const T*() const
{
  return &CurrentElement();
}

template <typename T, wdUInt32 BlockSize, wdBlockStorageType::Enum StorageType>
WD_FORCE_INLINE void wdBlockStorage<T, BlockSize, StorageType>::ConstIterator::Next()
{
  ++m_uiCurrentIndex;

  if (StorageType == wdBlockStorageType::FreeList)
  {
    wdUInt32 uiEndIndex = wdMath::Min(m_uiEndIndex, m_Storage.m_uiCount);
    while (m_uiCurrentIndex < uiEndIndex && !m_Storage.m_UsedEntries.IsBitSet(m_uiCurrentIndex))
    {
      ++m_uiCurrentIndex;
    }
  }
}

template <typename T, wdUInt32 BlockSize, wdBlockStorageType::Enum StorageType>
WD_FORCE_INLINE bool wdBlockStorage<T, BlockSize, StorageType>::ConstIterator::IsValid() const
{
  return m_uiCurrentIndex < wdMath::Min(m_uiEndIndex, m_Storage.m_uiCount);
}

template <typename T, wdUInt32 BlockSize, wdBlockStorageType::Enum StorageType>
WD_ALWAYS_INLINE void wdBlockStorage<T, BlockSize, StorageType>::ConstIterator::operator++()
{
  Next();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T, wdUInt32 BlockSize, wdBlockStorageType::Enum StorageType>
WD_FORCE_INLINE wdBlockStorage<T, BlockSize, StorageType>::Iterator::Iterator(
  const wdBlockStorage<T, BlockSize, StorageType>& storage, wdUInt32 uiStartIndex, wdUInt32 uiCount)
  : ConstIterator(storage, uiStartIndex, uiCount)
{
}

template <typename T, wdUInt32 BlockSize, wdBlockStorageType::Enum StorageType>
WD_ALWAYS_INLINE T& wdBlockStorage<T, BlockSize, StorageType>::Iterator::operator*()
{
  return this->CurrentElement();
}

template <typename T, wdUInt32 BlockSize, wdBlockStorageType::Enum StorageType>
WD_ALWAYS_INLINE T* wdBlockStorage<T, BlockSize, StorageType>::Iterator::operator->()
{
  return &(this->CurrentElement());
}

template <typename T, wdUInt32 BlockSize, wdBlockStorageType::Enum StorageType>
WD_ALWAYS_INLINE wdBlockStorage<T, BlockSize, StorageType>::Iterator::operator T*()
{
  return &(this->CurrentElement());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T, wdUInt32 BlockSize, wdBlockStorageType::Enum StorageType>
WD_FORCE_INLINE wdBlockStorage<T, BlockSize, StorageType>::wdBlockStorage(
  wdLargeBlockAllocator<BlockSize>* pBlockAllocator, wdAllocatorBase* pAllocator)
  : m_pBlockAllocator(pBlockAllocator)
  , m_Blocks(pAllocator)
  , m_uiCount(0)
  , m_uiFreelistStart(wdInvalidIndex)
{
}

template <typename T, wdUInt32 BlockSize, wdBlockStorageType::Enum StorageType>
wdBlockStorage<T, BlockSize, StorageType>::~wdBlockStorage()
{
  Clear();
}

template <typename T, wdUInt32 BlockSize, wdBlockStorageType::Enum StorageType>
void wdBlockStorage<T, BlockSize, StorageType>::Clear()
{
  for (wdUInt32 uiBlockIndex = 0; uiBlockIndex < m_Blocks.GetCount(); ++uiBlockIndex)
  {
    wdDataBlock<T, BlockSize>& block = m_Blocks[uiBlockIndex];

    if (StorageType == wdBlockStorageType::Compact)
    {
      wdMemoryUtils::Destruct(block.m_pData, block.m_uiCount);
    }
    else
    {
      for (wdUInt32 uiInnerIndex = 0; uiInnerIndex < block.m_uiCount; ++uiInnerIndex)
      {
        wdUInt32 uiIndex = uiBlockIndex * wdDataBlock<T, BlockSize>::CAPACITY + uiInnerIndex;
        if (m_UsedEntries.IsBitSet(uiIndex))
        {
          wdMemoryUtils::Destruct(&block.m_pData[uiInnerIndex], 1);
        }
      }
    }

    m_pBlockAllocator->DeallocateBlock(block);
  }

  m_Blocks.Clear();
}

template <typename T, wdUInt32 BlockSize, wdBlockStorageType::Enum StorageType>
T* wdBlockStorage<T, BlockSize, StorageType>::Create()
{
  T* pNewObject = nullptr;
  wdUInt32 uiNewIndex = wdInvalidIndex;

  if (StorageType == wdBlockStorageType::FreeList && m_uiFreelistStart != wdInvalidIndex)
  {
    uiNewIndex = m_uiFreelistStart;

    const wdUInt32 uiBlockIndex = uiNewIndex / wdDataBlock<T, BlockSize>::CAPACITY;
    const wdUInt32 uiInnerIndex = uiNewIndex - uiBlockIndex * wdDataBlock<T, BlockSize>::CAPACITY;

    pNewObject = &(m_Blocks[uiBlockIndex][uiInnerIndex]);

    m_uiFreelistStart = *reinterpret_cast<wdUInt32*>(pNewObject);
  }
  else
  {
    wdDataBlock<T, BlockSize>* pBlock = nullptr;

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

  wdMemoryUtils::Construct(pNewObject, 1);

  if (StorageType == wdBlockStorageType::FreeList)
  {
    m_UsedEntries.SetCount(m_uiCount);
    m_UsedEntries.SetBit(uiNewIndex);
  }

  return pNewObject;
}

template <typename T, wdUInt32 BlockSize, wdBlockStorageType::Enum StorageType>
WD_FORCE_INLINE void wdBlockStorage<T, BlockSize, StorageType>::Delete(T* pObject)
{
  T* pDummy;
  Delete(pObject, pDummy);
}

template <typename T, wdUInt32 BlockSize, wdBlockStorageType::Enum StorageType>
void wdBlockStorage<T, BlockSize, StorageType>::Delete(T* pObject, T*& out_pMovedObject)
{
  Delete(pObject, out_pMovedObject, wdTraitInt<StorageType>());
}

template <typename T, wdUInt32 BlockSize, wdBlockStorageType::Enum StorageType>
WD_ALWAYS_INLINE wdUInt32 wdBlockStorage<T, BlockSize, StorageType>::GetCount() const
{
  return m_uiCount;
}

template <typename T, wdUInt32 BlockSize, wdBlockStorageType::Enum StorageType>
WD_ALWAYS_INLINE typename wdBlockStorage<T, BlockSize, StorageType>::Iterator wdBlockStorage<T, BlockSize, StorageType>::GetIterator(
  wdUInt32 uiStartIndex /*= 0*/, wdUInt32 uiCount /*= wdInvalidIndex*/)
{
  return Iterator(*this, uiStartIndex, uiCount);
}

template <typename T, wdUInt32 BlockSize, wdBlockStorageType::Enum StorageType>
WD_ALWAYS_INLINE typename wdBlockStorage<T, BlockSize, StorageType>::ConstIterator wdBlockStorage<T, BlockSize, StorageType>::GetIterator(
  wdUInt32 uiStartIndex /*= 0*/, wdUInt32 uiCount /*= wdInvalidIndex*/) const
{
  return ConstIterator(*this, uiStartIndex, uiCount);
}

template <typename T, wdUInt32 BlockSize, wdBlockStorageType::Enum StorageType>
WD_FORCE_INLINE void wdBlockStorage<T, BlockSize, StorageType>::Delete(T* pObject, T*& out_pMovedObject, wdTraitInt<wdBlockStorageType::Compact>)
{
  wdDataBlock<T, BlockSize>& lastBlock = m_Blocks.PeekBack();
  T* pLast = lastBlock.PopBack();

  --m_uiCount;
  if (pObject != pLast)
  {
    wdMemoryUtils::Relocate(pObject, pLast, 1);
  }
  else
  {
    wdMemoryUtils::Destruct(pLast, 1);
  }

  out_pMovedObject = pLast;

  if (lastBlock.IsEmpty())
  {
    m_pBlockAllocator->DeallocateBlock(lastBlock);
    m_Blocks.PopBack();
  }
}

template <typename T, wdUInt32 BlockSize, wdBlockStorageType::Enum StorageType>
WD_FORCE_INLINE void wdBlockStorage<T, BlockSize, StorageType>::Delete(T* pObject, T*& out_pMovedObject, wdTraitInt<wdBlockStorageType::FreeList>)
{
  wdUInt32 uiIndex = wdInvalidIndex;
  for (wdUInt32 uiBlockIndex = 0; uiBlockIndex < m_Blocks.GetCount(); ++uiBlockIndex)
  {
    ptrdiff_t diff = pObject - m_Blocks[uiBlockIndex].m_pData;
    if (diff >= 0 && diff < wdDataBlock<T, BlockSize>::CAPACITY)
    {
      uiIndex = uiBlockIndex * wdDataBlock<T, BlockSize>::CAPACITY + (wdInt32)diff;
      break;
    }
  }

  WD_ASSERT_DEV(uiIndex != wdInvalidIndex, "Invalid object {0} was not found in block storage.", wdArgP(pObject));

  m_UsedEntries.ClearBit(uiIndex);

  out_pMovedObject = pObject;
  wdMemoryUtils::Destruct(pObject, 1);

  *reinterpret_cast<wdUInt32*>(pObject) = m_uiFreelistStart;
  m_uiFreelistStart = uiIndex;
}

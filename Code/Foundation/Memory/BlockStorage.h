#pragma once

#include <Foundation/Containers/Bitfield.h>
#include <Foundation/Memory/LargeBlockAllocator.h>

struct wdBlockStorageType
{
  enum Enum
  {
    Compact,
    FreeList
  };
};

template <typename T, wdUInt32 BlockSizeInByte, wdBlockStorageType::Enum StorageType>
class wdBlockStorage
{
public:
  class ConstIterator
  {
  public:
    const T& operator*() const;
    const T* operator->() const;

    operator const T*() const;

    void Next();
    bool IsValid() const;

    void operator++();

  protected:
    friend class wdBlockStorage<T, BlockSizeInByte, StorageType>;

    ConstIterator(const wdBlockStorage<T, BlockSizeInByte, StorageType>& storage, wdUInt32 uiStartIndex, wdUInt32 uiCount);

    T& CurrentElement() const;

    const wdBlockStorage<T, BlockSizeInByte, StorageType>& m_Storage;
    wdUInt32 m_uiCurrentIndex;
    wdUInt32 m_uiEndIndex;
  };

  class Iterator : public ConstIterator
  {
  public:
    T& operator*();
    T* operator->();

    operator T*();

  private:
    friend class wdBlockStorage<T, BlockSizeInByte, StorageType>;

    Iterator(const wdBlockStorage<T, BlockSizeInByte, StorageType>& storage, wdUInt32 uiStartIndex, wdUInt32 uiCount);
  };

  wdBlockStorage(wdLargeBlockAllocator<BlockSizeInByte>* pBlockAllocator, wdAllocatorBase* pAllocator);
  ~wdBlockStorage();

  void Clear();

  T* Create();
  void Delete(T* pObject);
  void Delete(T* pObject, T*& out_pMovedObject);

  wdUInt32 GetCount() const;
  Iterator GetIterator(wdUInt32 uiStartIndex = 0, wdUInt32 uiCount = wdInvalidIndex);
  ConstIterator GetIterator(wdUInt32 uiStartIndex = 0, wdUInt32 uiCount = wdInvalidIndex) const;

private:
  void Delete(T* pObject, T*& out_pMovedObject, wdTraitInt<wdBlockStorageType::Compact>);
  void Delete(T* pObject, T*& out_pMovedObject, wdTraitInt<wdBlockStorageType::FreeList>);

  wdLargeBlockAllocator<BlockSizeInByte>* m_pBlockAllocator;

  wdDynamicArray<wdDataBlock<T, BlockSizeInByte>> m_Blocks;
  wdUInt32 m_uiCount;

  wdUInt32 m_uiFreelistStart;

  wdDynamicBitfield m_UsedEntries;
};

#include <Foundation/Memory/Implementation/BlockStorage_inl.h>

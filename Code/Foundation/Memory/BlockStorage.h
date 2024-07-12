#pragma once

#include <Foundation/Containers/Bitfield.h>
#include <Foundation/Memory/LargeBlockAllocator.h>

struct nsBlockStorageType
{
  enum Enum
  {
    Compact,
    FreeList
  };
};

template <typename T, nsUInt32 BlockSizeInByte, nsBlockStorageType::Enum StorageType>
class nsBlockStorage
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
    friend class nsBlockStorage<T, BlockSizeInByte, StorageType>;

    ConstIterator(const nsBlockStorage<T, BlockSizeInByte, StorageType>& storage, nsUInt32 uiStartIndex, nsUInt32 uiCount);

    T& CurrentElement() const;

    const nsBlockStorage<T, BlockSizeInByte, StorageType>& m_Storage;
    nsUInt32 m_uiCurrentIndex;
    nsUInt32 m_uiEndIndex;
  };

  class Iterator : public ConstIterator
  {
  public:
    T& operator*();
    T* operator->();

    operator T*();

  private:
    friend class nsBlockStorage<T, BlockSizeInByte, StorageType>;

    Iterator(const nsBlockStorage<T, BlockSizeInByte, StorageType>& storage, nsUInt32 uiStartIndex, nsUInt32 uiCount);
  };

  nsBlockStorage(nsLargeBlockAllocator<BlockSizeInByte>* pBlockAllocator, nsAllocator* pAllocator);
  ~nsBlockStorage();

  void Clear();

  T* Create();
  void Delete(T* pObject);
  void Delete(T* pObject, T*& out_pMovedObject);

  nsUInt32 GetCount() const;
  Iterator GetIterator(nsUInt32 uiStartIndex = 0, nsUInt32 uiCount = nsInvalidIndex);
  ConstIterator GetIterator(nsUInt32 uiStartIndex = 0, nsUInt32 uiCount = nsInvalidIndex) const;

private:
  void Delete(T* pObject, T*& out_pMovedObject, nsTraitInt<nsBlockStorageType::Compact>);
  void Delete(T* pObject, T*& out_pMovedObject, nsTraitInt<nsBlockStorageType::FreeList>);

  nsLargeBlockAllocator<BlockSizeInByte>* m_pBlockAllocator;

  nsDynamicArray<nsDataBlock<T, BlockSizeInByte>> m_Blocks;
  nsUInt32 m_uiCount = 0;

  nsUInt32 m_uiFreelistStart = nsInvalidIndex;

  nsDynamicBitfield m_UsedEntries;
};

#include <Foundation/Memory/Implementation/BlockStorage_inl.h>

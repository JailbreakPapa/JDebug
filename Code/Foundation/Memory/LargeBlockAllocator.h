#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Memory/MemoryTracker.h>
#include <Foundation/Memory/PageAllocator.h>
#include <Foundation/System/SystemInformation.h>
#include <Foundation/Threading/Lock.h>
#include <Foundation/Threading/Mutex.h>
#include <Foundation/Threading/ThreadUtils.h>

/// \brief This struct represents a block of type T, typically 4kb.
template <typename T, nsUInt32 SizeInBytes>
struct nsDataBlock
{
  NS_DECLARE_POD_TYPE();

  enum
  {
    SIZE_IN_BYTES = SizeInBytes,
    CAPACITY = SIZE_IN_BYTES / sizeof(T)
  };

  nsDataBlock(T* pData, nsUInt32 uiCount);

  T* ReserveBack();
  T* PopBack();

  bool IsEmpty() const;
  bool IsFull() const;

  T& operator[](nsUInt32 uiIndex) const;

  T* m_pData;
  nsUInt32 m_uiCount;
};

/// \brief A block allocator which can only allocates blocks of memory at once.
template <nsUInt32 BlockSizeInByte>
class nsLargeBlockAllocator
{
public:
  nsLargeBlockAllocator(nsStringView sName, nsAllocator* pParent, nsAllocatorTrackingMode mode = nsAllocatorTrackingMode::Default);
  ~nsLargeBlockAllocator();

  template <typename T>
  nsDataBlock<T, BlockSizeInByte> AllocateBlock();

  template <typename T>
  void DeallocateBlock(nsDataBlock<T, BlockSizeInByte>& ref_block);


  nsStringView GetName() const;

  nsAllocatorId GetId() const;

  const nsAllocator::Stats& GetStats() const;

private:
  void* Allocate(size_t uiAlign);
  void Deallocate(void* ptr);

  nsAllocatorId m_Id;
  nsAllocatorTrackingMode m_TrackingMode;

  nsMutex m_Mutex;
  nsThreadID m_ThreadID;

  struct SuperBlock
  {
    NS_DECLARE_POD_TYPE();

    enum
    {
      NUM_BLOCKS = 16,
      SIZE_IN_BYTES = BlockSizeInByte * NUM_BLOCKS
    };

    void* m_pBasePtr;

    nsUInt32 m_uiUsedBlocks;
  };

  nsDynamicArray<SuperBlock> m_SuperBlocks;
  nsDynamicArray<nsUInt32> m_FreeBlocks;
};

#include <Foundation/Memory/Implementation/LargeBlockAllocator_inl.h>

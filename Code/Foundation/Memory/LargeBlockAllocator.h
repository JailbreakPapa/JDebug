#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Memory/MemoryTracker.h>
#include <Foundation/Memory/PageAllocator.h>
#include <Foundation/System/SystemInformation.h>
#include <Foundation/Threading/Lock.h>
#include <Foundation/Threading/Mutex.h>
#include <Foundation/Threading/ThreadUtils.h>

/// \brief This struct represents a block of type T, typically 4kb.
template <typename T, wdUInt32 SizeInBytes>
struct wdDataBlock
{
  WD_DECLARE_POD_TYPE();

  enum
  {
    SIZE_IN_BYTES = SizeInBytes,
    CAPACITY = SIZE_IN_BYTES / sizeof(T)
  };

  wdDataBlock(T* pData, wdUInt32 uiCount);

  T* ReserveBack();
  T* PopBack();

  bool IsEmpty() const;
  bool IsFull() const;

  T& operator[](wdUInt32 uiIndex) const;

  T* m_pData;
  wdUInt32 m_uiCount;
};

/// \brief A block allocator which can only allocates blocks of memory at once.
template <wdUInt32 BlockSizeInByte>
class wdLargeBlockAllocator
{
public:
  wdLargeBlockAllocator(const char* szName, wdAllocatorBase* pParent, wdBitflags<wdMemoryTrackingFlags> flags = wdMemoryTrackingFlags::Default);
  ~wdLargeBlockAllocator();

  template <typename T>
  wdDataBlock<T, BlockSizeInByte> AllocateBlock();

  template <typename T>
  void DeallocateBlock(wdDataBlock<T, BlockSizeInByte>& ref_block);


  const char* GetName() const;

  wdAllocatorId GetId() const;

  const wdAllocatorBase::Stats& GetStats() const;

private:
  void* Allocate(size_t uiAlign);
  void Deallocate(void* ptr);

  wdAllocatorId m_Id;
  wdBitflags<wdMemoryTrackingFlags> m_TrackingFlags;

  wdMutex m_Mutex;
  wdThreadID m_ThreadID;

  struct SuperBlock
  {
    WD_DECLARE_POD_TYPE();

    enum
    {
      NUM_BLOCKS = 16,
      SIZE_IN_BYTES = BlockSizeInByte * NUM_BLOCKS
    };

    void* m_pBasePtr;

    wdUInt32 m_uiUsedBlocks;
  };

  wdDynamicArray<SuperBlock> m_SuperBlocks;
  wdDynamicArray<wdUInt32> m_FreeBlocks;
};

#include <Foundation/Memory/Implementation/LargeBlockAllocator_inl.h>

#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Memory/CommonAllocators.h>
#include <Foundation/Memory/LargeBlockAllocator.h>
#include <Foundation/Memory/LinearAllocator.h>

struct alignas(NS_ALIGNMENT_MINIMUM) NonAlignedVector
{
  NS_DECLARE_POD_TYPE();

  NonAlignedVector()
  {
    x = 5.0f;
    y = 6.0f;
    z = 8.0f;
  }

  float x;
  float y;
  float z;
};

struct alignas(16) AlignedVector
{
  NS_DECLARE_POD_TYPE();

  AlignedVector()
  {
    x = 5.0f;
    y = 6.0f;
    z = 8.0f;
  }

  float x;
  float y;
  float z;
  float w;
};

template <typename T>
void TestAlignmentHelper(size_t uiExpectedAlignment)
{
  nsAllocator* pAllocator = nsFoundation::GetAlignedAllocator();
  NS_TEST_BOOL(pAllocator != nullptr);

  size_t uiAlignment = NS_ALIGNMENT_OF(T);
  NS_TEST_INT(uiAlignment, uiExpectedAlignment);

  T testOnStack = T();
  NS_TEST_BOOL(nsMemoryUtils::IsAligned(&testOnStack, uiExpectedAlignment));

  T* pTestBuffer = NS_NEW_RAW_BUFFER(pAllocator, T, 32);
  nsArrayPtr<T> TestArray = NS_NEW_ARRAY(pAllocator, T, 32);

  // default constructor should be called even if we declare as a pod type
  NS_TEST_FLOAT(TestArray[0].x, 5.0f, 0.0f);
  NS_TEST_FLOAT(TestArray[0].y, 6.0f, 0.0f);
  NS_TEST_FLOAT(TestArray[0].z, 8.0f, 0.0f);

  NS_TEST_BOOL(nsMemoryUtils::IsAligned(pTestBuffer, uiExpectedAlignment));
  NS_TEST_BOOL(nsMemoryUtils::IsAligned(TestArray.GetPtr(), uiExpectedAlignment));

  size_t uiExpectedSize = sizeof(T) * 32;

  if constexpr (nsAllocatorTrackingMode::Default >= nsAllocatorTrackingMode::AllocationStats)
  {
    NS_TEST_INT(pAllocator->AllocatedSize(pTestBuffer), uiExpectedSize);

    nsAllocator::Stats stats = pAllocator->GetStats();
    NS_TEST_INT(stats.m_uiAllocationSize, uiExpectedSize * 2);
    NS_TEST_INT(stats.m_uiNumAllocations - stats.m_uiNumDeallocations, 2);
  }

  NS_DELETE_ARRAY(pAllocator, TestArray);
  NS_DELETE_RAW_BUFFER(pAllocator, pTestBuffer);

  if constexpr (nsAllocatorTrackingMode::Default >= nsAllocatorTrackingMode::Basics)
  {
    nsAllocator::Stats stats = pAllocator->GetStats();
    NS_TEST_INT(stats.m_uiAllocationSize, 0);
    NS_TEST_INT(stats.m_uiNumAllocations - stats.m_uiNumDeallocations, 0);
  }
}

NS_CREATE_SIMPLE_TEST_GROUP(Memory);

NS_CREATE_SIMPLE_TEST(Memory, Allocator)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Alignment")
  {
    TestAlignmentHelper<NonAlignedVector>(NS_ALIGNMENT_MINIMUM);
    TestAlignmentHelper<AlignedVector>(16);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "LargeBlockAllocator")
  {
    enum
    {
      BLOCK_SIZE_IN_BYTES = 4096 * 4
    };
    const nsUInt32 uiPageSize = nsSystemInformation::Get().GetMemoryPageSize();

    nsLargeBlockAllocator<BLOCK_SIZE_IN_BYTES> allocator("Test", nsFoundation::GetDefaultAllocator(), nsAllocatorTrackingMode::AllocationStats);

    nsDynamicArray<nsDataBlock<int, BLOCK_SIZE_IN_BYTES>> blocks;
    blocks.Reserve(1000);

    for (nsUInt32 i = 0; i < 17; ++i)
    {
      auto block = allocator.AllocateBlock<int>();
      NS_TEST_BOOL(nsMemoryUtils::IsAligned(block.m_pData, uiPageSize)); // test page alignment
      NS_TEST_INT(block.m_uiCount, 0);

      blocks.PushBack(block);
    }

    nsAllocator::Stats stats = allocator.GetStats();

    NS_TEST_BOOL(stats.m_uiNumAllocations == 17);
    NS_TEST_BOOL(stats.m_uiNumDeallocations == 0);
    NS_TEST_BOOL(stats.m_uiAllocationSize == 17 * BLOCK_SIZE_IN_BYTES);

    for (nsUInt32 i = 0; i < 200; ++i)
    {
      auto block = allocator.AllocateBlock<int>();
      blocks.PushBack(block);
    }

    for (nsUInt32 i = 0; i < 200; ++i)
    {
      allocator.DeallocateBlock(blocks.PeekBack());
      blocks.PopBack();
    }

    stats = allocator.GetStats();

    NS_TEST_BOOL(stats.m_uiNumAllocations == 217);
    NS_TEST_BOOL(stats.m_uiNumDeallocations == 200);
    NS_TEST_BOOL(stats.m_uiAllocationSize == 17 * BLOCK_SIZE_IN_BYTES);

    for (nsUInt32 i = 0; i < 2000; ++i)
    {
      nsUInt32 uiAction = rand() % 2;
      if (uiAction == 0)
      {
        blocks.PushBack(allocator.AllocateBlock<int>());
      }
      else if (blocks.GetCount() > 0)
      {
        nsUInt32 uiIndex = rand() % blocks.GetCount();
        auto block = blocks[uiIndex];

        allocator.DeallocateBlock(block);

        blocks.RemoveAtAndSwap(uiIndex);
      }
    }

    for (nsUInt32 i = 0; i < blocks.GetCount(); ++i)
    {
      allocator.DeallocateBlock(blocks[i]);
    }

    stats = allocator.GetStats();

    NS_TEST_BOOL(stats.m_uiNumAllocations - stats.m_uiNumDeallocations == 0);
    NS_TEST_BOOL(stats.m_uiAllocationSize == 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "StackAllocator")
  {
    nsLinearAllocator<> allocator("TestStackAllocator", nsFoundation::GetAlignedAllocator());

    void* blocks[8];
    for (size_t i = 0; i < NS_ARRAY_SIZE(blocks); i++)
    {
      size_t size = i + 1;
      blocks[i] = allocator.Allocate(size, sizeof(void*), nullptr);
      NS_TEST_BOOL(blocks[i] != nullptr);
      if (i > 0)
      {
        NS_TEST_BOOL((nsUInt8*)blocks[i - 1] + (size - 1) <= blocks[i]);
      }
    }

    for (size_t i = NS_ARRAY_SIZE(blocks); i--;)
    {
      allocator.Deallocate(blocks[i]);
    }

    size_t sizes[] = {128, 128, 4096, 1024, 1024, 16000, 512, 512, 768, 768, 16000, 16000, 16000, 16000};
    void* allocs[NS_ARRAY_SIZE(sizes)];
    for (size_t i = 0; i < NS_ARRAY_SIZE(sizes); i++)
    {
      allocs[i] = allocator.Allocate(sizes[i], sizeof(void*), nullptr);
      NS_TEST_BOOL(allocs[i] != nullptr);
    }
    for (size_t i = NS_ARRAY_SIZE(sizes); i--;)
    {
      allocator.Deallocate(allocs[i]);
    }
    allocator.Reset();

    for (size_t i = 0; i < NS_ARRAY_SIZE(sizes); i++)
    {
      allocs[i] = allocator.Allocate(sizes[i], sizeof(void*), nullptr);
      NS_TEST_BOOL(allocs[i] != nullptr);
    }
    allocator.Reset();
    allocs[0] = allocator.Allocate(8, sizeof(void*), nullptr);
    NS_TEST_BOOL(allocs[0] < allocs[1]);
    allocator.Deallocate(allocs[0]);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "StackAllocator with non-PODs")
  {
    nsLinearAllocator<> allocator("TestStackAllocator", nsFoundation::GetAlignedAllocator());

    nsDynamicArray<nsConstructionCounter*> counters;
    counters.Reserve(100);

    for (nsUInt32 i = 0; i < 100; ++i)
    {
      counters.PushBack(NS_NEW(&allocator, nsConstructionCounter));
    }

    for (nsUInt32 i = 0; i < 100; ++i)
    {
      NS_NEW(&allocator, NonAlignedVector);
    }

    NS_TEST_BOOL(nsConstructionCounter::HasConstructed(100));

    for (nsUInt32 i = 0; i < 50; ++i)
    {
      NS_DELETE(&allocator, counters[i * 2]);
    }

    NS_TEST_BOOL(nsConstructionCounter::HasDestructed(50));

    allocator.Reset();

    NS_TEST_BOOL(nsConstructionCounter::HasDestructed(50));
  }
}

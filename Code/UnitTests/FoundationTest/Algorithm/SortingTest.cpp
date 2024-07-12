#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Containers/DynamicArray.h>

namespace
{
  struct CustomComparer
  {
    NS_ALWAYS_INLINE bool Less(nsInt32 a, nsInt32 b) const { return a > b; }

    // Comparision via operator. Sorting algorithm should prefer Less operator
    bool operator()(nsInt32 a, nsInt32 b) const { return a < b; }
  };
} // namespace

NS_CREATE_SIMPLE_TEST(Algorithm, Sorting)
{
  nsDynamicArray<nsInt32> a1;

  for (nsUInt32 i = 0; i < 2000; ++i)
  {
    a1.PushBack(rand() % 100000);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "QuickSort")
  {
    nsDynamicArray<nsInt32> a2 = a1;

    nsSorting::QuickSort(a1, CustomComparer()); // quicksort uses insertion sort for partitions smaller than 16 elements

    for (nsUInt32 i = 1; i < a1.GetCount(); ++i)
    {
      NS_TEST_BOOL(a1[i - 1] >= a1[i]);
    }

    nsArrayPtr<nsInt32> arrayPtr = a2;
    nsSorting::QuickSort(arrayPtr, CustomComparer()); // quicksort uses insertion sort for partitions smaller than 16 elements

    for (nsUInt32 i = 1; i < arrayPtr.GetCount(); ++i)
    {
      NS_TEST_BOOL(arrayPtr[i - 1] >= arrayPtr[i]);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "QuickSort - Lambda")
  {
    nsDynamicArray<nsInt32> a2 = a1;
    nsSorting::QuickSort(a2, [](const auto& a, const auto& b)
      { return a > b; });

    for (nsUInt32 i = 1; i < a2.GetCount(); ++i)
    {
      NS_TEST_BOOL(a2[i - 1] >= a2[i]);
    }
  }
}

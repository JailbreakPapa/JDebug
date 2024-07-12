#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Memory/CommonAllocators.h>
#include <Foundation/Time/Stopwatch.h>
#include <Foundation/Types/UniquePtr.h>

static nsInt32 iCallPodConstructor = 0;
static nsInt32 iCallPodDestructor = 0;
static nsInt32 iCallNonPodConstructor = 0;
static nsInt32 iCallNonPodDestructor = 0;

namespace DynamicArrayTestDetail
{
  using st = nsConstructionCounter;

  static int g_iDummyCounter = 0;

  class Dummy
  {
  public:
    int a;
    int b;
    std::string s;

    Dummy()
      : a(0)
      , b(g_iDummyCounter++)
      , s("Test")
    {
    }
    Dummy(int a)
      : a(a)
      , b(g_iDummyCounter++)
      , s("Test")
    {
    }

    bool operator<=(const Dummy& dummy) const { return a <= dummy.a; }
    bool operator>=(const Dummy& dummy) const { return a >= dummy.a; }
    bool operator>(const Dummy& dummy) const { return a > dummy.a; }
    bool operator<(const Dummy& dummy) const { return a < dummy.a; }
    bool operator==(const Dummy& dummy) const { return a == dummy.a; }
  };

  nsAllocator* g_pTestAllocator;

  struct nsTestAllocatorWrapper
  {
    static nsAllocator* GetAllocator() { return g_pTestAllocator; }
  };

  template <typename T = st, typename AllocatorWrapper = nsTestAllocatorWrapper>
  static nsDynamicArray<T, AllocatorWrapper> CreateArray(nsUInt32 uiSize, nsUInt32 uiOffset)
  {
    nsDynamicArray<T, AllocatorWrapper> a;
    a.SetCount(uiSize);

    for (nsUInt32 i = 0; i < uiSize; ++i)
      a[i] = T(uiOffset + i);

    return a;
  }
} // namespace DynamicArrayTestDetail

#if NS_ENABLED(NS_PLATFORM_64BIT)
NS_CHECK_AT_COMPILETIME(sizeof(nsDynamicArray<nsInt32>) == 24);
#else
NS_CHECK_AT_COMPILETIME(sizeof(nsDynamicArray<nsInt32>) == 16);
#endif

NS_CREATE_SIMPLE_TEST_GROUP(Containers);

NS_CREATE_SIMPLE_TEST(Containers, DynamicArray)
{
  iCallPodConstructor = 0;
  iCallPodDestructor = 0;
  iCallNonPodConstructor = 0;
  iCallNonPodDestructor = 0;

  nsProxyAllocator proxy("DynamicArrayTestAllocator", nsFoundation::GetDefaultAllocator());
  DynamicArrayTestDetail::g_pTestAllocator = &proxy;

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
    nsDynamicArray<nsInt32> a1;
    nsDynamicArray<DynamicArrayTestDetail::st> a2;

    NS_TEST_BOOL(a1.GetCount() == 0);
    NS_TEST_BOOL(a2.GetCount() == 0);
    NS_TEST_BOOL(a1.IsEmpty());
    NS_TEST_BOOL(a2.IsEmpty());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Copy Constructor")
  {
    nsDynamicArray<nsInt32, DynamicArrayTestDetail::nsTestAllocatorWrapper> a1;

    NS_TEST_BOOL(a1.GetHeapMemoryUsage() == 0);

    for (nsInt32 i = 0; i < 32; ++i)
      a1.PushBack(rand() % 100000);

    NS_TEST_BOOL(a1.GetHeapMemoryUsage() >= 32 * sizeof(nsInt32));

    nsDynamicArray<nsInt32> a2 = a1;
    nsDynamicArray<nsInt32> a3(a1);

    NS_TEST_BOOL(a1 == a2);
    NS_TEST_BOOL(a1 == a3);
    NS_TEST_BOOL(a2 == a3);

    nsInt32 test[] = {1, 2, 3, 4};
    nsArrayPtr<nsInt32> aptr(test);

    nsDynamicArray<nsInt32> a4(aptr);

    NS_TEST_BOOL(a4 == aptr);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Move Constructor / Operator")
  {
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasAllDestructed());

    {
      // move constructor
      nsDynamicArray<DynamicArrayTestDetail::st, DynamicArrayTestDetail::nsTestAllocatorWrapper> a1(DynamicArrayTestDetail::CreateArray(100, 20));

      NS_TEST_INT(a1.GetCount(), 100);
      for (nsUInt32 i = 0; i < a1.GetCount(); ++i)
        NS_TEST_INT(a1[i].m_iData, 20 + i);

      // move operator
      a1 = DynamicArrayTestDetail::CreateArray(200, 50);

      NS_TEST_INT(a1.GetCount(), 200);
      for (nsUInt32 i = 0; i < a1.GetCount(); ++i)
        NS_TEST_INT(a1[i].m_iData, 50 + i);
    }

    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasAllDestructed());

    {
      // move assignment with different allocators
      nsConstructionCounterRelocatable::Reset();
      nsProxyAllocator proxyAllocator("test allocator", nsFoundation::GetDefaultAllocator());
      {
        nsDynamicArray<nsConstructionCounterRelocatable> a1(&proxyAllocator);

        a1 = DynamicArrayTestDetail::CreateArray<nsConstructionCounterRelocatable, nsDefaultAllocatorWrapper>(8, 70);
        NS_TEST_BOOL(nsConstructionCounterRelocatable::HasDone(8, 0));
        NS_TEST_BOOL(a1.GetAllocator() == &proxyAllocator); // allocator must not change

        NS_TEST_INT(a1.GetCount(), 8);
        for (nsUInt32 i = 0; i < a1.GetCount(); ++i)
          NS_TEST_INT(a1[i].m_iData, 70 + i);

        a1 = DynamicArrayTestDetail::CreateArray<nsConstructionCounterRelocatable, nsDefaultAllocatorWrapper>(32, 100);
        NS_TEST_BOOL(nsConstructionCounterRelocatable::HasDone(32, 8));
        NS_TEST_BOOL(a1.GetAllocator() == &proxyAllocator); // allocator must not change

        NS_TEST_INT(a1.GetCount(), 32);
        for (nsUInt32 i = 0; i < a1.GetCount(); ++i)
          NS_TEST_INT(a1[i].m_iData, 100 + i);
      }

      NS_TEST_BOOL(nsConstructionCounterRelocatable::HasAllDestructed());
      nsConstructionCounterRelocatable::Reset();

      auto allocatorStats = proxyAllocator.GetStats();
      NS_TEST_BOOL(allocatorStats.m_uiNumAllocations == allocatorStats.m_uiNumDeallocations); // check for memory leak?
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Convert to ArrayPtr")
  {
    nsDynamicArray<nsInt32> a1;

    for (nsInt32 i = 0; i < 100; ++i)
    {
      nsInt32 r = rand() % 100000;
      a1.PushBack(r);
    }

    nsArrayPtr<nsInt32> ap = a1;

    NS_TEST_BOOL(ap.GetCount() == a1.GetCount());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator =")
  {
    nsDynamicArray<nsInt32, DynamicArrayTestDetail::nsTestAllocatorWrapper> a1;
    nsDynamicArray<nsInt32> a2;

    for (nsInt32 i = 0; i < 100; ++i)
      a1.PushBack(i);

    a2 = a1;

    NS_TEST_BOOL(a1 == a2);

    nsArrayPtr<nsInt32> arrayPtr(a1);

    a2 = arrayPtr;

    NS_TEST_BOOL(a2 == arrayPtr);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator == / !=/ <")
  {
    nsDynamicArray<nsInt32> a1, a2;

    NS_TEST_BOOL(a1 == a1);
    NS_TEST_BOOL(a2 == a2);
    NS_TEST_BOOL(a1 == a2);

    NS_TEST_BOOL((a1 != a1) == false);
    NS_TEST_BOOL((a2 != a2) == false);
    NS_TEST_BOOL((a1 != a2) == false);

    for (nsInt32 i = 0; i < 100; ++i)
    {
      nsInt32 r = rand() % 100000;
      a1.PushBack(r);
      a2.PushBack(r);
    }

    NS_TEST_BOOL(a1 == a1);
    NS_TEST_BOOL(a2 == a2);
    NS_TEST_BOOL(a1 == a2);

    NS_TEST_BOOL((a1 != a2) == false);

    NS_TEST_BOOL((a1 < a2) == false);
    a2.PushBack(100);
    NS_TEST_BOOL(a1 < a2);
    a1.PushBack(99);
    NS_TEST_BOOL(a1 < a2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Index operator")
  {
    nsDynamicArray<nsInt32> a1;
    a1.SetCountUninitialized(100);

    for (nsInt32 i = 0; i < 100; ++i)
      a1[i] = i;

    for (nsInt32 i = 0; i < 100; ++i)
      NS_TEST_INT(a1[i], i);

    nsDynamicArray<nsInt32> ca1;
    ca1 = a1;

    for (nsInt32 i = 0; i < 100; ++i)
      NS_TEST_INT(ca1[i], i);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetCount / GetCount / IsEmpty")
  {
    nsDynamicArray<nsInt32> a1;

    NS_TEST_BOOL(a1.IsEmpty());

    for (nsInt32 i = 0; i < 128; ++i)
    {
      a1.SetCount(i + 1);
      NS_TEST_INT(a1[i], 0);
      a1[i] = i;

      NS_TEST_INT(a1.GetCount(), i + 1);
      NS_TEST_BOOL(!a1.IsEmpty());
    }

    for (nsInt32 i = 0; i < 128; ++i)
      NS_TEST_INT(a1[i], i);

    for (nsInt32 i = 128; i >= 0; --i)
    {
      a1.SetCount(i);

      NS_TEST_INT(a1.GetCount(), i);

      for (nsInt32 i2 = 0; i2 < i; ++i2)
        NS_TEST_INT(a1[i2], i2);
    }

    NS_TEST_BOOL(a1.IsEmpty());

    a1.SetCountUninitialized(32);
    NS_TEST_INT(a1.GetCount(), 32);
    a1[31] = 45;
    NS_TEST_INT(a1[31], 45);

    // Test SetCount with fill value
    {
      nsDynamicArray<nsInt32> a2;
      a2.PushBack(5);
      a2.PushBack(3);
      a2.SetCount(10, 42);

      if (NS_TEST_INT(a2.GetCount(), 10))
      {
        NS_TEST_INT(a2[0], 5);
        NS_TEST_INT(a2[1], 3);
        NS_TEST_INT(a2[4], 42);
        NS_TEST_INT(a2[9], 42);
      }

      a2.Clear();
      a2.PushBack(1);
      a2.PushBack(2);
      a2.PushBack(3);

      a2.SetCount(2, 10);
      if (NS_TEST_INT(a2.GetCount(), 2))
      {
        NS_TEST_INT(a2[0], 1);
        NS_TEST_INT(a2[1], 2);
      }
    }
  }

  // Test SetCount with fill value
  {
    nsDynamicArray<nsInt32> a2;
    a2.PushBack(5);
    a2.PushBack(3);
    a2.SetCount(10, 42);

    if (NS_TEST_INT(a2.GetCount(), 10))
    {
      NS_TEST_INT(a2[0], 5);
      NS_TEST_INT(a2[1], 3);
      NS_TEST_INT(a2[4], 42);
      NS_TEST_INT(a2[9], 42);
    }

    a2.Clear();
    a2.PushBack(1);
    a2.PushBack(2);
    a2.PushBack(3);

    a2.SetCount(2, 10);
    if (NS_TEST_INT(a2.GetCount(), 2))
    {
      NS_TEST_INT(a2[0], 1);
      NS_TEST_INT(a2[1], 2);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "EnsureCount")
  {
    nsDynamicArray<nsInt32> a1;

    NS_TEST_INT(a1.GetCount(), 0);

    a1.EnsureCount(0);
    NS_TEST_INT(a1.GetCount(), 0);

    a1.EnsureCount(1);
    NS_TEST_INT(a1.GetCount(), 1);

    a1.EnsureCount(2);
    NS_TEST_INT(a1.GetCount(), 2);

    a1.EnsureCount(1);
    NS_TEST_INT(a1.GetCount(), 2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Clear")
  {
    nsDynamicArray<nsInt32> a1;
    a1.Clear();

    a1.PushBack(3);
    a1.Clear();

    NS_TEST_BOOL(a1.IsEmpty());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Contains / IndexOf / LastIndexOf")
  {
    nsDynamicArray<nsInt32> a1;

    for (nsInt32 i = -100; i < 100; ++i)
      NS_TEST_BOOL(!a1.Contains(i));

    for (nsInt32 i = 0; i < 100; ++i)
      a1.PushBack(i);
    for (nsInt32 i = 0; i < 100; ++i)
      a1.PushBack(i);

    for (nsInt32 i = 0; i < 100; ++i)
    {
      NS_TEST_BOOL(a1.Contains(i));
      NS_TEST_INT(a1.IndexOf(i), i);
      NS_TEST_INT(a1.IndexOf(i, 100), i + 100);
      NS_TEST_INT(a1.LastIndexOf(i), i + 100);
      NS_TEST_INT(a1.LastIndexOf(i, 100), i);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "PushBackUnchecked / PushBackRange")
  {
    nsDynamicArray<nsInt32> a1;
    a1.Reserve(100);

    for (nsInt32 i = 0; i < 100; ++i)
      a1.PushBackUnchecked(i);

    for (nsInt32 i = 0; i < 100; ++i)
      NS_TEST_INT(a1[i], i);

    nsInt32 temp[] = {100, 101, 102, 103, 104};
    nsArrayPtr<nsInt32> range(temp);

    a1.PushBackRange(range);

    NS_TEST_INT(a1.GetCount(), 105);
    for (nsUInt32 i = 0; i < a1.GetCount(); ++i)
      NS_TEST_INT(a1[i], i);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Insert")
  {
    nsDynamicArray<nsInt32> a1;

    // always inserts at the front
    for (nsInt32 i = 0; i < 100; ++i)
      a1.InsertAt(0, i);

    for (nsInt32 i = 0; i < 100; ++i)
      NS_TEST_INT(a1[i], 99 - i);

    nsUniquePtr<DynamicArrayTestDetail::st> ptr = NS_DEFAULT_NEW(DynamicArrayTestDetail::st);
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasConstructed(1));

    {
      nsDynamicArray<nsUniquePtr<DynamicArrayTestDetail::st>> a2;
      for (nsUInt32 i = 0; i < 10; ++i)
        a2.InsertAt(0, nsUniquePtr<DynamicArrayTestDetail::st>());

      a2.InsertAt(0, std::move(ptr));
      NS_TEST_BOOL(ptr == nullptr);
      NS_TEST_BOOL(a2[0] != nullptr);

      for (nsUInt32 i = 1; i < a2.GetCount(); ++i)
        NS_TEST_BOOL(a2[i] == nullptr);
    }

    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasAllDestructed());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "InsertRange")
  {
    // Pod element tests
    nsDynamicArray<nsInt32> intTestRange;
    nsDynamicArray<nsInt32> a1;

    nsInt32 intTemp1[] = {91, 92, 93, 94, 95};
    nsArrayPtr<nsInt32> intRange1(intTemp1);

    nsInt32 intTemp2[] = {96, 97, 98, 99, 100};
    nsArrayPtr<nsInt32> intRange2(intTemp2);

    nsInt32 intTemp3[] = {100, 101, 102, 103, 104};
    nsArrayPtr<nsInt32> intRange3(intTemp3);

    {
      intTestRange.PushBackRange(intRange3);

      a1.InsertRange(intRange3, 0);

      NS_TEST_INT(a1.GetCount(), 5);

      for (nsUInt32 i = 0; i < a1.GetCount(); ++i)
        NS_TEST_INT(a1[i], intTestRange[i]);
    }

    {
      intTestRange.Clear();
      intTestRange.PushBackRange(intRange1);
      intTestRange.PushBackRange(intRange3);

      a1.InsertRange(intRange1, 0);

      NS_TEST_INT(a1.GetCount(), 10);
      for (nsUInt32 i = 0; i < a1.GetCount(); ++i)
        NS_TEST_INT(a1[i], intTestRange[i]);
    }

    {
      intTestRange.Clear();
      intTestRange.PushBackRange(intRange1);
      intTestRange.PushBackRange(intRange2);
      intTestRange.PushBackRange(intRange3);

      a1.InsertRange(intRange2, 5);

      NS_TEST_INT(a1.GetCount(), 15);
      for (nsUInt32 i = 0; i < a1.GetCount(); ++i)
        NS_TEST_INT(a1[i], intTestRange[i]);
    }

    // Class element tests
    nsDynamicArray<nsDeque<nsString>> classTestRange;
    nsDynamicArray<nsDeque<nsString>> a2;

    nsDeque<nsString> strTemp1[4];
    {
      strTemp1[0].PushBack("One");
      strTemp1[1].PushBack("Two");
      strTemp1[2].PushBack("Three");
      strTemp1[3].PushBack("Four");
    }
    nsArrayPtr<nsDeque<nsString>> classRange1(strTemp1);

    nsDeque<nsString> strTemp2[3];
    {
      strTemp2[0].PushBack("Five");
      strTemp2[1].PushBack("Six");
      strTemp2[2].PushBack("Seven");
    }
    nsArrayPtr<nsDeque<nsString>> classRange2(strTemp2);

    nsDeque<nsString> strTemp3[3];
    {
      strTemp3[0].PushBack("Eight");
      strTemp3[1].PushBack("Nine");
      strTemp3[2].PushBack("Ten");
    }
    nsArrayPtr<nsDeque<nsString>> classRange3(strTemp3);

    {
      classTestRange.PushBackRange(classRange3);

      a2.InsertRange(classRange3, 0);

      NS_TEST_INT(a2.GetCount(), 3);

      for (nsUInt32 i = 0; i < a2.GetCount(); ++i)
        NS_TEST_STRING(a2[i].PeekFront(), classTestRange[i].PeekFront());
    }

    {
      classTestRange.Clear();
      classTestRange.PushBackRange(classRange1);
      classTestRange.PushBackRange(classRange3);

      a2.InsertRange(classRange1, 0);

      NS_TEST_INT(a2.GetCount(), 7);
      for (nsUInt32 i = 0; i < a2.GetCount(); ++i)
        NS_TEST_STRING(a2[i].PeekFront(), classTestRange[i].PeekFront());
    }

    {
      classTestRange.Clear();
      classTestRange.PushBackRange(classRange1);
      classTestRange.PushBackRange(classRange2);
      classTestRange.PushBackRange(classRange3);

      a2.InsertRange(classRange2, 4);

      NS_TEST_INT(a2.GetCount(), 10);
      for (nsUInt32 i = 0; i < a2.GetCount(); ++i)
        NS_TEST_STRING(a2[i].PeekFront(), classTestRange[i].PeekFront());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RemoveAndCopy")
  {
    nsDynamicArray<nsInt32> a1;

    for (nsInt32 i = 0; i < 100; ++i)
      a1.PushBack(i % 2);

    while (a1.RemoveAndCopy(1))
    {
    }

    NS_TEST_BOOL(a1.GetCount() == 50);

    for (nsUInt32 i = 0; i < a1.GetCount(); ++i)
      NS_TEST_INT(a1[i], 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RemoveAndSwap")
  {
    nsDynamicArray<nsInt32> a1;

    for (nsInt32 i = 0; i < 10; ++i)
      a1.InsertAt(i, i); // inserts at the end

    a1.RemoveAndSwap(9);
    a1.RemoveAndSwap(7);
    a1.RemoveAndSwap(5);
    a1.RemoveAndSwap(3);
    a1.RemoveAndSwap(1);

    NS_TEST_INT(a1.GetCount(), 5);

    for (nsInt32 i = 0; i < 5; ++i)
      NS_TEST_BOOL(nsMath::IsEven(a1[i]));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RemoveAtAndCopy")
  {
    nsDynamicArray<nsInt32> a1;

    for (nsInt32 i = 0; i < 10; ++i)
      a1.InsertAt(i, i); // inserts at the end

    a1.RemoveAtAndCopy(9);
    a1.RemoveAtAndCopy(7);
    a1.RemoveAtAndCopy(5);
    a1.RemoveAtAndCopy(3);
    a1.RemoveAtAndCopy(1);

    NS_TEST_INT(a1.GetCount(), 5);

    for (nsInt32 i = 0; i < 5; ++i)
      NS_TEST_INT(a1[i], i * 2);

    nsUniquePtr<DynamicArrayTestDetail::st> ptr = NS_DEFAULT_NEW(DynamicArrayTestDetail::st);
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasConstructed(1));

    {
      nsDynamicArray<nsUniquePtr<DynamicArrayTestDetail::st>> a2;
      for (nsUInt32 i = 0; i < 10; ++i)
        a2.InsertAt(0, nsUniquePtr<DynamicArrayTestDetail::st>());

      a2.PushBack(std::move(ptr));
      NS_TEST_BOOL(ptr == nullptr);
      NS_TEST_BOOL(a2[10] != nullptr);

      a2.RemoveAtAndCopy(0);
      NS_TEST_BOOL(a2[9] != nullptr);
      NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(0, 0));
    }

    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasAllDestructed());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RemoveAtAndSwap")
  {
    nsDynamicArray<nsInt32> a1;

    for (nsInt32 i = 0; i < 10; ++i)
      a1.InsertAt(i, i); // inserts at the end

    a1.RemoveAtAndSwap(9);
    a1.RemoveAtAndSwap(7);
    a1.RemoveAtAndSwap(5);
    a1.RemoveAtAndSwap(3);
    a1.RemoveAtAndSwap(1);

    NS_TEST_INT(a1.GetCount(), 5);

    for (nsInt32 i = 0; i < 5; ++i)
      NS_TEST_BOOL(nsMath::IsEven(a1[i]));

    nsUniquePtr<DynamicArrayTestDetail::st> ptr = NS_DEFAULT_NEW(DynamicArrayTestDetail::st);
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasConstructed(1));

    {
      nsDynamicArray<nsUniquePtr<DynamicArrayTestDetail::st>> a2;
      for (nsUInt32 i = 0; i < 10; ++i)
        a2.InsertAt(0, nsUniquePtr<DynamicArrayTestDetail::st>());

      a2.PushBack(std::move(ptr));
      NS_TEST_BOOL(ptr == nullptr);
      NS_TEST_BOOL(a2[10] != nullptr);

      a2.RemoveAtAndSwap(0);
      NS_TEST_BOOL(a2[0] != nullptr);
    }

    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasAllDestructed());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "PushBack / PopBack / PeekBack")
  {
    nsDynamicArray<nsInt32> a1;

    for (nsInt32 i = 0; i < 10; ++i)
    {
      a1.PushBack(i);
      NS_TEST_INT(a1.PeekBack(), i);
    }

    for (nsInt32 i = 9; i >= 0; --i)
    {
      NS_TEST_INT(a1.PeekBack(), i);
      a1.PopBack();
    }

    a1.PushBack(23);
    a1.PushBack(2);
    a1.PushBack(3);

    a1.PopBack(2);
    NS_TEST_INT(a1.PeekBack(), 23);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExpandAndGetRef")
  {
    nsDynamicArray<nsInt32> a1;

    for (nsInt32 i = 0; i < 20; ++i)
    {
      nsInt32& intRef = a1.ExpandAndGetRef();
      intRef = i * 5;
    }


    NS_TEST_BOOL(a1.GetCount() == 20);

    for (nsInt32 i = 0; i < 20; ++i)
    {
      NS_TEST_INT(a1[i], i * 5);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Construction / Destruction")
  {
    {
      NS_TEST_BOOL(DynamicArrayTestDetail::st::HasAllDestructed());

      nsDynamicArray<DynamicArrayTestDetail::st> a1;
      nsDynamicArray<DynamicArrayTestDetail::st> a2;

      NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(0, 0)); // nothing has been constructed / destructed in between
      NS_TEST_BOOL(DynamicArrayTestDetail::st::HasAllDestructed());

      a1.PushBack(DynamicArrayTestDetail::st(1));
      NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(2, 1)); // one temporary, one final (copy constructed)

      a1.InsertAt(0, DynamicArrayTestDetail::st(2));
      NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(2, 1)); // one temporary, one final (copy constructed)

      a2 = a1;
      NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(2, 0)); // two copies

      a1.Clear();
      NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(0, 2));

      a1.PushBack(DynamicArrayTestDetail::st(3));
      a1.PushBack(DynamicArrayTestDetail::st(4));
      a1.PushBack(DynamicArrayTestDetail::st(5));
      a1.PushBack(DynamicArrayTestDetail::st(6));

      NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(8, 4)); // four temporaries

      a1.RemoveAndCopy(DynamicArrayTestDetail::st(3));
      NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(1, 2)); // one temporary, one destroyed

      a1.RemoveAndCopy(DynamicArrayTestDetail::st(3));
      NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(1, 1)); // one temporary, none destroyed

      a1.RemoveAtAndCopy(0);
      NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(0, 1)); // one destroyed

      a1.RemoveAtAndSwap(0);
      NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(0, 1)); // one destroyed
    }

    // tests the destructor of a2 and a1
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasAllDestructed());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SortingPrimitives")
  {
    nsDynamicArray<nsUInt32> list;

    list.Sort();

    for (nsUInt32 i = 0; i < 450; i++)
    {
      list.PushBack(std::rand());
    }
    list.Sort();

    for (nsUInt32 i = 1; i < list.GetCount(); i++)
    {
      NS_TEST_BOOL(list[i - 1] <= list[i]);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SortingObjects")
  {
    nsDynamicArray<DynamicArrayTestDetail::Dummy> list;
    list.Reserve(128);

    for (nsUInt32 i = 0; i < 100; i++)
    {
      list.PushBack(DynamicArrayTestDetail::Dummy(rand()));
    }
    list.Sort();

    for (nsUInt32 i = 1; i < list.GetCount(); i++)
    {
      NS_TEST_BOOL(list[i - 1] <= list[i]);
      NS_TEST_BOOL(list[i].s == "Test");
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SortingMovableObjects")
  {
    {
      nsDynamicArray<nsUniquePtr<DynamicArrayTestDetail::st>> list;
      list.Reserve(128);

      for (nsUInt32 i = 0; i < 100; i++)
      {
        list.PushBack(NS_DEFAULT_NEW(DynamicArrayTestDetail::st));
      }
      list.Sort();

      for (nsUInt32 i = 1; i < list.GetCount(); i++)
      {
        NS_TEST_BOOL(list[i - 1] <= list[i]);
      }
    }

    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasAllDestructed());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Various")
  {
    nsDynamicArray<DynamicArrayTestDetail::Dummy> list;
    list.PushBack(1);
    list.PushBack(2);
    list.PushBack(3);
    list.InsertAt(3, 4);
    list.InsertAt(1, 0);
    list.InsertAt(5, 0);

    NS_TEST_BOOL(list[0].a == 1);
    NS_TEST_BOOL(list[1].a == 0);
    NS_TEST_BOOL(list[2].a == 2);
    NS_TEST_BOOL(list[3].a == 3);
    NS_TEST_BOOL(list[4].a == 4);
    NS_TEST_BOOL(list[5].a == 0);
    NS_TEST_BOOL(list.GetCount() == 6);

    list.RemoveAtAndCopy(3);
    list.RemoveAtAndSwap(2);

    NS_TEST_BOOL(list[0].a == 1);
    NS_TEST_BOOL(list[1].a == 0);
    NS_TEST_BOOL(list[2].a == 0);
    NS_TEST_BOOL(list[3].a == 4);
    NS_TEST_BOOL(list.GetCount() == 4);
    NS_TEST_BOOL(list.IndexOf(0) == 1);
    NS_TEST_BOOL(list.LastIndexOf(0) == 2);

    list.PushBack(5);
    NS_TEST_BOOL(list[4].a == 5);
    DynamicArrayTestDetail::Dummy d = list.PeekBack();
    list.PopBack();
    NS_TEST_BOOL(d.a == 5);
    NS_TEST_BOOL(list.GetCount() == 4);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Assignment")
  {
    nsDynamicArray<DynamicArrayTestDetail::Dummy> list;
    for (int i = 0; i < 16; i++)
    {
      list.PushBack(DynamicArrayTestDetail::Dummy(rand()));
    }

    nsDynamicArray<DynamicArrayTestDetail::Dummy> list2;
    for (int i = 0; i < 8; i++)
    {
      list2.PushBack(DynamicArrayTestDetail::Dummy(rand()));
    }

    list = list2;
    NS_TEST_BOOL(list.GetCount() == list2.GetCount());

    list2.Clear();
    NS_TEST_BOOL(list2.GetCount() == 0);

    list2 = list;
    NS_TEST_BOOL(list.PeekBack() == list2.PeekBack());
    NS_TEST_BOOL(list == list2);

    for (int i = 0; i < 16; i++)
    {
      list2.PushBack(DynamicArrayTestDetail::Dummy(rand()));
    }

    list = list2;
    NS_TEST_BOOL(list.PeekBack() == list2.PeekBack());
    NS_TEST_BOOL(list == list2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Count")
  {
    nsDynamicArray<DynamicArrayTestDetail::Dummy> list;
    for (int i = 0; i < 16; i++)
    {
      list.PushBack(DynamicArrayTestDetail::Dummy(rand()));
    }
    list.SetCount(32);
    list.SetCount(16);

    list.Compact();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Reserve")
  {
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasAllDestructed());

    nsDynamicArray<DynamicArrayTestDetail::st> a;

    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(0, 0)); // nothing has been constructed / destructed in between
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasAllDestructed());

    a.Reserve(100);

    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(0, 0)); // nothing has been constructed / destructed in between
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasAllDestructed());

    a.SetCount(10);
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(10, 0));

    a.Reserve(100);
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(0, 0));

    a.SetCount(100);
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(90, 0));

    a.Reserve(200);
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(100, 100)); // had to copy some elements over

    a.SetCount(200);
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(100, 0));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Compact")
  {
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasAllDestructed());

    nsDynamicArray<DynamicArrayTestDetail::st> a;

    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(0, 0)); // nothing has been constructed / destructed in between
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasAllDestructed());

    a.SetCount(100);
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(100, 0));

    a.SetCount(200);
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(200, 100));

    a.SetCount(10);
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(0, 190));

    // no reallocations and copying, if the memory is already available
    a.SetCount(200);
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(190, 0));

    a.SetCount(10);
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(0, 190));

    // now we remove the spare memory
    a.Compact();
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(10, 10));

    // this time the array needs to be relocated, and thus the already present elements need to be copied
    a.SetCount(200);
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(200, 10));

    // this does not deallocate memory
    a.Clear();
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(0, 200));

    a.SetCount(100);
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(100, 0));

    // therefore no object relocation
    a.SetCount(200);
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(100, 0));

    a.Clear();
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(0, 200));

    // this will deallocate ALL memory
    NS_TEST_BOOL(a.GetHeapMemoryUsage() > 0);
    a.Compact();
    NS_TEST_BOOL(a.GetHeapMemoryUsage() == 0);

    a.SetCount(100);
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(100, 0));

    // this time objects need to be relocated
    a.SetCount(200);
    NS_TEST_BOOL(DynamicArrayTestDetail::st::HasDone(200, 100));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "STL Iterator")
  {
    nsDynamicArray<nsInt32> a1;

    for (nsInt32 i = 0; i < 1000; ++i)
      a1.PushBack(1000 - i - 1);

    // STL sort
    std::sort(begin(a1), end(a1));

    for (nsInt32 i = 1; i < 1000; ++i)
    {
      NS_TEST_BOOL(a1[i - 1] <= a1[i]);
    }

    // foreach
    nsInt32 prev = 0;
    nsInt32 sum1 = 0;
    for (nsInt32 val : a1)
    {
      NS_TEST_BOOL(prev <= val);
      prev = val;
      sum1 += val;
    }

    prev = 1000;
    const auto endIt = rend(a1);
    nsInt32 sum2 = 0;
    for (auto it = rbegin(a1); it != endIt; ++it)
    {
      NS_TEST_BOOL(prev > (*it));
      prev = (*it);
      sum2 += (*it);
    }

    NS_TEST_BOOL(sum1 == sum2);

    // const array
    const nsDynamicArray<nsInt32>& a2 = a1;

    // STL lower bound
    auto lb = std::lower_bound(begin(a2), end(a2), 400);
    NS_TEST_BOOL(*lb == a2[400]);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "STL Reverse Iterator")
  {
    nsDynamicArray<nsInt32> a1;

    for (nsInt32 i = 0; i < 1000; ++i)
      a1.PushBack(1000 - i - 1);

    // STL sort
    std::sort(rbegin(a1), rend(a1));

    for (nsInt32 i = 1; i < 1000; ++i)
    {
      NS_TEST_BOOL(a1[i - 1] >= a1[i]);
    }

    // foreach
    nsUInt32 prev = 1000;
    for (nsUInt32 val : a1)
    {
      NS_TEST_BOOL(prev >= val);
      prev = val;
    }

    // const array
    const nsDynamicArray<nsInt32>& a2 = a1;

    // STL lower bound
    auto lb = std::lower_bound(rbegin(a2), rend(a2), 400);
    NS_TEST_BOOL(*lb == a2[1000 - 400 - 1]);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetArrayPtr")
  {
    nsDynamicArray<nsInt32> a1;
    a1.SetCountUninitialized(10);

    NS_TEST_BOOL(a1.GetArrayPtr().GetCount() == 10);
    NS_TEST_BOOL(a1.GetArrayPtr().GetPtr() == a1.GetData());

    const nsDynamicArray<nsInt32>& a1ref = a1;

    NS_TEST_BOOL(a1ref.GetArrayPtr().GetCount() == 10);
    NS_TEST_BOOL(a1ref.GetArrayPtr().GetPtr() == a1ref.GetData());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Swap")
  {
    nsDynamicArray<nsInt32> a1, a2;

    nsInt32 content1[] = {1, 2, 3, 4};
    nsInt32 content2[] = {5, 6, 7, 8, 9};

    a1 = nsMakeArrayPtr(content1);
    a2 = nsMakeArrayPtr(content2);

    nsInt32* a1Ptr = a1.GetData();
    nsInt32* a2Ptr = a2.GetData();

    a1.Swap(a2);

    // The pointers should be simply swapped
    NS_TEST_BOOL(a2Ptr == a1.GetData());
    NS_TEST_BOOL(a1Ptr == a2.GetData());

    // The data should be swapped
    NS_TEST_BOOL(a1.GetArrayPtr() == nsMakeArrayPtr(content2));
    NS_TEST_BOOL(a2.GetArrayPtr() == nsMakeArrayPtr(content1));
  }

#if NS_ENABLED(NS_PLATFORM_64BIT)

  // disabled, because this is a very slow test
  NS_TEST_BLOCK(nsTestBlock::DisabledNoWarning, "Large Allocation")
  {
    const nsUInt32 uiMaxNumElements = 0xFFFFFFFF - 16; // max supported elements due to alignment restrictions

    // this will allocate about 16 GB memory, the pure allocation is really fast
    nsDynamicArray<nsUInt32> byteArray;
    byteArray.SetCountUninitialized(uiMaxNumElements);

    const nsUInt32 uiCheckElements = byteArray.GetCount();
    const nsUInt32 uiSkipElements = 1024;

    // this will touch the memory and thus enforce that it is indeed made available by the OS
    // this takes a while
    for (nsUInt64 i = 0; i < uiCheckElements; i += uiSkipElements)
    {
      const nsUInt32 idx = i & 0xFFFFFFFF;
      byteArray[idx] = idx;
    }

    // check that the assigned values are all correct
    // again, this takes quite a while
    for (nsUInt64 i = 0; i < uiCheckElements; i += uiSkipElements)
    {
      const nsUInt32 idx = i & 0xFFFFFFFF;
      NS_TEST_INT(byteArray[idx], idx);
    }
  }
#endif


  const nsUInt32 uiNumSortItems = 1'000'000;

  struct Item
  {
    bool operator<(const Item& rhs) const { return m_iKey < rhs.m_iKey; }

    nsInt32 m_iKey = 0;
    nsInt32 m_iIndex = 0;
  };

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SortLargeArray (ns-sort)")
  {
    nsDynamicArray<Item> list;
    list.Reserve(uiNumSortItems);

    for (nsUInt32 i = 0; i < uiNumSortItems; i++)
    {
      auto& item = list.ExpandAndGetRef();
      item.m_iIndex = i;
      item.m_iKey = std::rand();
    }

    nsStopwatch sw;
    list.Sort();

    nsTime t = sw.GetRunningTotal();
    nsStringBuilder s;
    s.SetFormat("ns-sort (random keys): {}", t);
    nsTestFramework::Output(nsTestOutput::Details, s);

    for (nsUInt32 i = 1; i < list.GetCount(); i++)
    {
      NS_TEST_BOOL(list[i - 1].m_iKey <= list[i].m_iKey);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SortLargeArray (std::sort)")
  {
    nsDynamicArray<Item> list;
    list.Reserve(uiNumSortItems);

    for (nsUInt32 i = 0; i < uiNumSortItems; i++)
    {
      auto& item = list.ExpandAndGetRef();
      item.m_iIndex = i;
      item.m_iKey = std::rand();
    }

    nsStopwatch sw;
    std::sort(begin(list), end(list));

    nsTime t = sw.GetRunningTotal();
    nsStringBuilder s;
    s.SetFormat("std::sort (random keys): {}", t);
    nsTestFramework::Output(nsTestOutput::Details, s);

    for (nsUInt32 i = 1; i < list.GetCount(); i++)
    {
      NS_TEST_BOOL(list[i - 1].m_iKey <= list[i].m_iKey);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SortLargeArray (equal keys) (ns-sort)")
  {
    nsDynamicArray<Item> list;
    list.Reserve(uiNumSortItems);

    for (nsUInt32 i = 0; i < uiNumSortItems; i++)
    {
      auto& item = list.ExpandAndGetRef();
      item.m_iIndex = i;
      item.m_iKey = 42;
    }

    nsStopwatch sw;
    list.Sort();

    nsTime t = sw.GetRunningTotal();
    nsStringBuilder s;
    s.SetFormat("ns-sort (equal keys): {}", t);
    nsTestFramework::Output(nsTestOutput::Details, s);

    for (nsUInt32 i = 1; i < list.GetCount(); i++)
    {
      NS_TEST_BOOL(list[i - 1].m_iKey <= list[i].m_iKey);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SortLargeArray (equal keys) (std::sort)")
  {
    nsDynamicArray<Item> list;
    list.Reserve(uiNumSortItems);

    for (nsUInt32 i = 0; i < uiNumSortItems; i++)
    {
      auto& item = list.ExpandAndGetRef();
      item.m_iIndex = i;
      item.m_iKey = 42;
    }

    nsStopwatch sw;
    std::sort(begin(list), end(list));

    nsTime t = sw.GetRunningTotal();
    nsStringBuilder s;
    s.SetFormat("std::sort (equal keys): {}", t);
    nsTestFramework::Output(nsTestOutput::Details, s);

    for (nsUInt32 i = 1; i < list.GetCount(); i++)
    {
      NS_TEST_BOOL(list[i - 1].m_iKey <= list[i].m_iKey);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetCountUninitialized")
  {
    struct POD
    {
      NS_DECLARE_POD_TYPE();

      nsUInt32 a = 2;
      nsUInt32 b = 4;

      POD()
      {
        iCallPodConstructor++;
      }

      // this isn't allowed anymore in types that use NS_DECLARE_POD_TYPE
      // unfortunately that means we can't do this kind of check either
      //~POD()
      //{
      //  iCallPodDestructor++;
      //}
    };

    static_assert(std::is_trivial<POD>::value == 0);
    static_assert(nsIsPodType<POD>::value == 1);

    struct NonPOD
    {
      nsUInt32 a = 3;
      nsUInt32 b = 5;

      NonPOD()
      {
        iCallNonPodConstructor++;
      }

      ~NonPOD()
      {
        iCallNonPodDestructor++;
      }
    };

    static_assert(std::is_trivial<NonPOD>::value == 0);
    static_assert(nsIsPodType<NonPOD>::value == 0);

    // check that SetCountUninitialized doesn't construct and Clear doesn't destruct POD types
    {
      nsDynamicArray<POD> s1a;

      s1a.SetCountUninitialized(16);
      NS_TEST_INT(iCallPodConstructor, 0);
      NS_TEST_INT(iCallPodDestructor, 0);

      s1a.Clear();
      NS_TEST_INT(iCallPodConstructor, 0);
      NS_TEST_INT(iCallPodDestructor, 0);
    }

    // check that SetCount constructs and Clear destructs Non-POD types
    {
      nsDynamicArray<NonPOD> s2a;

      s2a.SetCount(16);
      NS_TEST_INT(iCallNonPodConstructor, 16);
      NS_TEST_INT(iCallNonPodDestructor, 0);

      s2a.Clear();
      NS_TEST_INT(iCallNonPodConstructor, 16);
      NS_TEST_INT(iCallNonPodDestructor, 16);
    }
  }
}

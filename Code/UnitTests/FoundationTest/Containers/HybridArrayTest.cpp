#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Memory/CommonAllocators.h>
#include <Foundation/Strings/String.h>

namespace HybridArrayTestDetail
{

  class Dummy
  {
  public:
    int a = 0;
    std::string s = "Test";

    Dummy() = default;

    Dummy(int a)
      : a(a)
    {
    }

    Dummy(const Dummy& other) = default;
    ~Dummy() = default;

    Dummy& operator=(const Dummy& other) = default;

    bool operator<=(const Dummy& dummy) const { return a <= dummy.a; }
    bool operator>=(const Dummy& dummy) const { return a >= dummy.a; }
    bool operator>(const Dummy& dummy) const { return a > dummy.a; }
    bool operator<(const Dummy& dummy) const { return a < dummy.a; }
    bool operator==(const Dummy& dummy) const { return a == dummy.a; }
  };

  class NonMovableClass
  {
  public:
    NonMovableClass(int iVal)
    {
      m_val = iVal;
      m_pVal = &m_val;
    }

    NonMovableClass(const NonMovableClass& other)
    {
      m_val = other.m_val;
      m_pVal = &m_val;
    }

    void operator=(const NonMovableClass& other) { m_val = other.m_val; }

    int m_val = 0;
    int* m_pVal = nullptr;
  };

  template <typename T>
  static nsHybridArray<T, 16> CreateArray(nsUInt32 uiSize, nsUInt32 uiOffset)
  {
    nsHybridArray<T, 16> a;
    a.SetCount(uiSize);

    for (nsUInt32 i = 0; i < uiSize; ++i)
      a[i] = T(uiOffset + i);

    return a;
  }

  struct ExternalCounter
  {
    NS_DECLARE_MEM_RELOCATABLE_TYPE();

    ExternalCounter() = default;

    ExternalCounter(int& ref_iCounter)
      : m_counter{&ref_iCounter}
    {
    }

    ~ExternalCounter()
    {
      if (m_counter)
        (*m_counter)++;
    }

    int* m_counter{};
  };
} // namespace HybridArrayTestDetail

static void TakesDynamicArray(nsDynamicArray<int>& ref_ar, int iNum, int iStart);

#if NS_ENABLED(NS_PLATFORM_64BIT)
static_assert(sizeof(nsHybridArray<nsInt32, 1>) == 32);
#else
static_assert(sizeof(nsHybridArray<nsInt32, 1>) == 20);
#endif

static_assert(nsGetTypeClass<nsHybridArray<nsInt32, 1>>::value == nsTypeIsClass::value);
static_assert(nsGetTypeClass<nsHybridArray<HybridArrayTestDetail::NonMovableClass, 1>>::value == nsTypeIsClass::value);

NS_CREATE_SIMPLE_TEST(Containers, HybridArray)
{
  nsConstructionCounter::Reset();

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
    nsHybridArray<nsInt32, 16> a1;
    nsHybridArray<nsConstructionCounter, 16> a2;

    NS_TEST_BOOL(a1.GetCount() == 0);
    NS_TEST_BOOL(a2.GetCount() == 0);
    NS_TEST_BOOL(a1.IsEmpty());
    NS_TEST_BOOL(a2.IsEmpty());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Copy Constructor")
  {
    nsHybridArray<nsInt32, 16> a1;

    NS_TEST_BOOL(a1.GetHeapMemoryUsage() == 0);

    for (nsInt32 i = 0; i < 32; ++i)
    {
      a1.PushBack(rand() % 100000);

      if (i < 16)
      {
        NS_TEST_BOOL(a1.GetHeapMemoryUsage() == 0);
      }
      else
      {
        NS_TEST_BOOL(a1.GetHeapMemoryUsage() >= i * sizeof(nsInt32));
      }
    }

    nsHybridArray<nsInt32, 16> a2 = a1;
    nsHybridArray<nsInt32, 16> a3(a1);

    NS_TEST_BOOL(a1 == a2);
    NS_TEST_BOOL(a1 == a3);
    NS_TEST_BOOL(a2 == a3);

    nsInt32 test[] = {1, 2, 3, 4};
    nsArrayPtr<nsInt32> aptr(test);

    nsHybridArray<nsInt32, 16> a4(aptr);

    NS_TEST_BOOL(a4 == aptr);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Move Constructor / Operator")
  {
    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());

    {
      // move constructor external storage
      nsHybridArray<nsConstructionCounter, 16> a1(HybridArrayTestDetail::CreateArray<nsConstructionCounter>(100, 20));

      NS_TEST_INT(a1.GetCount(), 100);
      for (nsUInt32 i = 0; i < a1.GetCount(); ++i)
        NS_TEST_INT(a1[i].m_iData, 20 + i);

      // move operator external storage
      a1 = HybridArrayTestDetail::CreateArray<nsConstructionCounter>(200, 50);

      NS_TEST_INT(a1.GetCount(), 200);
      for (nsUInt32 i = 0; i < a1.GetCount(); ++i)
        NS_TEST_INT(a1[i].m_iData, 50 + i);
    }

    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());
    nsConstructionCounter::Reset();

    {
      // move constructor internal storage
      nsHybridArray<nsConstructionCounter, 16> a2(HybridArrayTestDetail::CreateArray<nsConstructionCounter>(10, 30));

      NS_TEST_INT(a2.GetCount(), 10);
      for (nsUInt32 i = 0; i < a2.GetCount(); ++i)
        NS_TEST_INT(a2[i].m_iData, 30 + i);

      // move operator internal storage
      a2 = HybridArrayTestDetail::CreateArray<nsConstructionCounter>(8, 70);

      NS_TEST_INT(a2.GetCount(), 8);
      for (nsUInt32 i = 0; i < a2.GetCount(); ++i)
        NS_TEST_INT(a2[i].m_iData, 70 + i);
    }

    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());
    nsConstructionCounter::Reset();

    nsConstructionCounterRelocatable::Reset();
    {
      // move constructor external storage relocatable
      nsHybridArray<nsConstructionCounterRelocatable, 16> a1(HybridArrayTestDetail::CreateArray<nsConstructionCounterRelocatable>(100, 20));

      NS_TEST_BOOL(nsConstructionCounterRelocatable::HasDone(100, 0));

      NS_TEST_INT(a1.GetCount(), 100);
      for (nsUInt32 i = 0; i < a1.GetCount(); ++i)
        NS_TEST_INT(a1[i].m_iData, 20 + i);

      // move operator external storage
      a1 = HybridArrayTestDetail::CreateArray<nsConstructionCounterRelocatable>(200, 50);
      NS_TEST_BOOL(nsConstructionCounterRelocatable::HasDone(200, 100));

      NS_TEST_INT(a1.GetCount(), 200);
      for (nsUInt32 i = 0; i < a1.GetCount(); ++i)
        NS_TEST_INT(a1[i].m_iData, 50 + i);
    }

    NS_TEST_BOOL(nsConstructionCounterRelocatable::HasAllDestructed());
    nsConstructionCounterRelocatable::Reset();

    {
      // move constructor internal storage relocatable
      nsHybridArray<nsConstructionCounterRelocatable, 16> a2(HybridArrayTestDetail::CreateArray<nsConstructionCounterRelocatable>(10, 30));
      NS_TEST_BOOL(nsConstructionCounterRelocatable::HasDone(10, 0));

      NS_TEST_INT(a2.GetCount(), 10);
      for (nsUInt32 i = 0; i < a2.GetCount(); ++i)
        NS_TEST_INT(a2[i].m_iData, 30 + i);

      // move operator internal storage
      a2 = HybridArrayTestDetail::CreateArray<nsConstructionCounterRelocatable>(8, 70);
      NS_TEST_BOOL(nsConstructionCounterRelocatable::HasDone(8, 10));

      NS_TEST_INT(a2.GetCount(), 8);
      for (nsUInt32 i = 0; i < a2.GetCount(); ++i)
        NS_TEST_INT(a2[i].m_iData, 70 + i);
    }

    NS_TEST_BOOL(nsConstructionCounterRelocatable::HasAllDestructed());
    nsConstructionCounterRelocatable::Reset();

    {
      // move constructor with different allocators
      nsProxyAllocator proxyAllocator("test allocator", nsFoundation::GetDefaultAllocator());
      {
        nsHybridArray<nsConstructionCounterRelocatable, 16> a1(&proxyAllocator);

        a1 = HybridArrayTestDetail::CreateArray<nsConstructionCounterRelocatable>(8, 70);
        NS_TEST_BOOL(nsConstructionCounterRelocatable::HasDone(8, 0));
        NS_TEST_BOOL(a1.GetAllocator() == &proxyAllocator); // allocator must not change

        NS_TEST_INT(a1.GetCount(), 8);
        for (nsUInt32 i = 0; i < a1.GetCount(); ++i)
          NS_TEST_INT(a1[i].m_iData, 70 + i);

        a1 = HybridArrayTestDetail::CreateArray<nsConstructionCounterRelocatable>(32, 100);
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
    nsHybridArray<nsInt32, 16> a1;

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
    nsHybridArray<nsInt32, 16> a1, a2;

    for (nsInt32 i = 0; i < 100; ++i)
      a1.PushBack(i);

    a2 = a1;

    NS_TEST_BOOL(a1 == a2);

    nsArrayPtr<nsInt32> arrayPtr(a1);

    a2 = arrayPtr;

    NS_TEST_BOOL(a2 == arrayPtr);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator == / !=")
  {
    nsHybridArray<nsInt32, 16> a1, a2;

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
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Index operator")
  {
    nsHybridArray<nsInt32, 16> a1;
    a1.SetCountUninitialized(100);

    for (nsInt32 i = 0; i < 100; ++i)
      a1[i] = i;

    for (nsInt32 i = 0; i < 100; ++i)
      NS_TEST_INT(a1[i], i);

    const nsHybridArray<nsInt32, 16> ca1 = a1;

    for (nsInt32 i = 0; i < 100; ++i)
      NS_TEST_INT(ca1[i], i);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetCount / GetCount / IsEmpty")
  {
    nsHybridArray<nsInt32, 16> a1;

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
      nsHybridArray<nsInt32, 2> a2;
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
    nsHybridArray<nsInt32, 2> a2;
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

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Clear")
  {
    nsHybridArray<nsInt32, 16> a1;
    a1.Clear();

    a1.PushBack(3);
    a1.Clear();

    NS_TEST_BOOL(a1.IsEmpty());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Contains / IndexOf / LastIndexOf")
  {
    nsHybridArray<nsInt32, 16> a1;

    for (nsInt32 i = -100; i < 100; ++i)
      NS_TEST_BOOL(!a1.Contains(i));

    for (nsInt32 i = 0; i < 100; ++i)
      a1.PushBack(i);

    for (nsInt32 i = 0; i < 100; ++i)
    {
      NS_TEST_BOOL(a1.Contains(i));
      NS_TEST_INT(a1.IndexOf(i), i);
      NS_TEST_INT(a1.LastIndexOf(i), i);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "InsertAt")
  {
    nsHybridArray<nsInt32, 16> a1;

    // always inserts at the front
    for (nsInt32 i = 0; i < 100; ++i)
      a1.InsertAt(0, i);

    for (nsInt32 i = 0; i < 100; ++i)
      NS_TEST_INT(a1[i], 99 - i);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RemoveAndCopy")
  {
    nsHybridArray<nsInt32, 16> a1;

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
    nsHybridArray<nsInt32, 16> a1;

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
    nsHybridArray<nsInt32, 16> a1;

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
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RemoveAtAndSwap")
  {
    nsHybridArray<nsInt32, 16> a1;

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
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "PushBack / PopBack / PeekBack")
  {
    nsHybridArray<nsInt32, 16> a1;

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
    nsHybridArray<nsInt32, 16> a1;

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
      NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());

      nsHybridArray<nsConstructionCounter, 16> a1;
      nsHybridArray<nsConstructionCounter, 16> a2;

      NS_TEST_BOOL(nsConstructionCounter::HasDone(0, 0)); // nothing has been constructed / destructed in between
      NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());

      a1.PushBack(nsConstructionCounter(1));
      NS_TEST_BOOL(nsConstructionCounter::HasDone(2, 1)); // one temporary, one final (copy constructed)

      a1.InsertAt(0, nsConstructionCounter(2));
      NS_TEST_BOOL(nsConstructionCounter::HasDone(2, 1)); // one temporary, one final (copy constructed)

      a2 = a1;
      NS_TEST_BOOL(nsConstructionCounter::HasDone(2, 0)); // two copies

      a1.Clear();
      NS_TEST_BOOL(nsConstructionCounter::HasDone(0, 2));

      a1.PushBack(nsConstructionCounter(3));
      a1.PushBack(nsConstructionCounter(4));
      a1.PushBack(nsConstructionCounter(5));
      a1.PushBack(nsConstructionCounter(6));

      NS_TEST_BOOL(nsConstructionCounter::HasDone(8, 4)); // four temporaries

      a1.RemoveAndCopy(nsConstructionCounter(3));
      NS_TEST_BOOL(nsConstructionCounter::HasDone(1, 2)); // one temporary, one destroyed

      a1.RemoveAndCopy(nsConstructionCounter(3));
      NS_TEST_BOOL(nsConstructionCounter::HasDone(1, 1)); // one temporary, none destroyed

      a1.RemoveAtAndCopy(0);
      NS_TEST_BOOL(nsConstructionCounter::HasDone(0, 1)); // one destroyed

      a1.RemoveAtAndSwap(0);
      NS_TEST_BOOL(nsConstructionCounter::HasDone(0, 1)); // one destroyed
    }

    // tests the destructor of a2 and a1
    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Compact")
  {
    nsHybridArray<nsInt32, 16> a;

    for (nsInt32 i = 0; i < 1008; ++i)
    {
      a.PushBack(i);
      NS_TEST_INT(a.GetCount(), i + 1);
    }

    NS_TEST_BOOL(a.GetHeapMemoryUsage() > 0);
    a.Compact();
    NS_TEST_BOOL(a.GetHeapMemoryUsage() > 0);

    for (nsInt32 i = 0; i < 1008; ++i)
      NS_TEST_INT(a[i], i);

    // this tests whether the static array is reused properly (not the case anymore with new implementation that derives from nsDynamicArray)
    a.SetCount(15);
    a.Compact();
    // NS_TEST_BOOL(a.GetHeapMemoryUsage() == 0);
    NS_TEST_BOOL(a.GetHeapMemoryUsage() > 0);

    for (nsInt32 i = 0; i < 15; ++i)
      NS_TEST_INT(a[i], i);

    a.Clear();
    a.Compact();
    NS_TEST_BOOL(a.GetHeapMemoryUsage() == 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SortingPrimitives")
  {
    nsHybridArray<nsUInt32, 16> list;

    list.Sort();

    for (nsUInt32 i = 0; i < 45; i++)
    {
      list.PushBack(std::rand());
    }
    list.Sort();

    nsUInt32 last = 0;
    for (nsUInt32 i = 0; i < list.GetCount(); i++)
    {
      NS_TEST_BOOL(last <= list[i]);
      last = list[i];
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SortingObjects")
  {
    nsHybridArray<HybridArrayTestDetail::Dummy, 16> list;
    list.Reserve(128);

    for (nsUInt32 i = 0; i < 100; i++)
    {
      list.PushBack(HybridArrayTestDetail::Dummy(rand()));
    }
    list.Sort();

    HybridArrayTestDetail::Dummy last = 0;
    for (nsUInt32 i = 0; i < list.GetCount(); i++)
    {
      NS_TEST_BOOL(last <= list[i]);
      last = list[i];
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Various")
  {
    nsHybridArray<HybridArrayTestDetail::Dummy, 16> list;
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
    HybridArrayTestDetail::Dummy d = list.PeekBack();
    list.PopBack();
    NS_TEST_BOOL(d.a == 5);
    NS_TEST_BOOL(list.GetCount() == 4);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Assignment")
  {
    nsHybridArray<HybridArrayTestDetail::Dummy, 16> list;
    for (int i = 0; i < 16; i++)
    {
      list.PushBack(HybridArrayTestDetail::Dummy(rand()));
    }

    nsHybridArray<HybridArrayTestDetail::Dummy, 16> list2;
    for (int i = 0; i < 8; i++)
    {
      list2.PushBack(HybridArrayTestDetail::Dummy(rand()));
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
      list2.PushBack(HybridArrayTestDetail::Dummy(rand()));
    }

    list = list2;
    NS_TEST_BOOL(list.PeekBack() == list2.PeekBack());
    NS_TEST_BOOL(list == list2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Count")
  {
    nsHybridArray<HybridArrayTestDetail::Dummy, 16> list;
    for (int i = 0; i < 16; i++)
    {
      list.PushBack(HybridArrayTestDetail::Dummy(rand()));
    }
    list.SetCount(32);
    list.SetCount(4);

    list.Compact();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Reserve")
  {
    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());

    nsHybridArray<nsConstructionCounter, 16> a;

    NS_TEST_BOOL(nsConstructionCounter::HasDone(0, 0)); // nothing has been constructed / destructed in between
    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());

    a.Reserve(100);

    NS_TEST_BOOL(nsConstructionCounter::HasDone(0, 0)); // nothing has been constructed / destructed in between
    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());

    a.SetCount(10);
    NS_TEST_BOOL(nsConstructionCounter::HasDone(10, 0));

    a.Reserve(100);
    NS_TEST_BOOL(nsConstructionCounter::HasDone(0, 0));

    a.SetCount(100);
    NS_TEST_BOOL(nsConstructionCounter::HasDone(90, 0));

    a.Reserve(200);
    NS_TEST_BOOL(nsConstructionCounter::HasDone(100, 100)); // had to copy some elements over

    a.SetCount(200);
    NS_TEST_BOOL(nsConstructionCounter::HasDone(100, 0));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Compact")
  {
    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());

    nsHybridArray<nsConstructionCounter, 16> a;

    NS_TEST_BOOL(nsConstructionCounter::HasDone(0, 0)); // nothing has been constructed / destructed in between
    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());

    a.SetCount(100);
    NS_TEST_BOOL(nsConstructionCounter::HasDone(100, 0));

    a.SetCount(200);
    NS_TEST_BOOL(nsConstructionCounter::HasDone(200, 100));

    a.SetCount(10);
    NS_TEST_BOOL(nsConstructionCounter::HasDone(0, 190));

    // no reallocations and copying, if the memory is already available
    a.SetCount(200);
    NS_TEST_BOOL(nsConstructionCounter::HasDone(190, 0));

    a.SetCount(10);
    NS_TEST_BOOL(nsConstructionCounter::HasDone(0, 190));

    // now we remove the spare memory
    a.Compact();
    NS_TEST_BOOL(nsConstructionCounter::HasDone(10, 10));

    // this time the array needs to be relocated, and thus the already present elements need to be copied
    a.SetCount(200);
    NS_TEST_BOOL(nsConstructionCounter::HasDone(200, 10));

    // this does not deallocate memory
    a.Clear();
    NS_TEST_BOOL(nsConstructionCounter::HasDone(0, 200));

    a.SetCount(100);
    NS_TEST_BOOL(nsConstructionCounter::HasDone(100, 0));

    // therefore no object relocation
    a.SetCount(200);
    NS_TEST_BOOL(nsConstructionCounter::HasDone(100, 0));

    a.Clear();
    NS_TEST_BOOL(nsConstructionCounter::HasDone(0, 200));

    // this will deallocate ALL memory
    a.Compact();

    a.SetCount(100);
    NS_TEST_BOOL(nsConstructionCounter::HasDone(100, 0));

    // this time objects need to be relocated
    a.SetCount(200);
    NS_TEST_BOOL(nsConstructionCounter::HasDone(200, 100));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "STL Iterator")
  {
    nsHybridArray<nsInt32, 16> a1;

    for (nsInt32 i = 0; i < 1000; ++i)
      a1.PushBack(1000 - i - 1);

    // STL sort
    std::sort(begin(a1), end(a1));

    for (nsInt32 i = 1; i < 1000; ++i)
    {
      NS_TEST_BOOL(a1[i - 1] <= a1[i]);
    }

    // foreach
    nsUInt32 prev = 0;
    for (nsUInt32 val : a1)
    {
      NS_TEST_BOOL(prev <= val);
      prev = val;
    }

    // const array
    const nsHybridArray<nsInt32, 16>& a2 = a1;

    // STL lower bound
    auto lb = std::lower_bound(begin(a2), end(a2), 400);
    NS_TEST_BOOL(*lb == a2[400]);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "STL Reverse Iterator")
  {
    nsHybridArray<nsInt32, 16> a1;

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
    const nsHybridArray<nsInt32, 16>& a2 = a1;

    // STL lower bound
    auto lb = std::lower_bound(rbegin(a2), rend(a2), 400);
    NS_TEST_BOOL(*lb == a2[1000 - 400 - 1]);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Swap")
  {

    nsInt32 content1[] = {1, 2, 3, 4};
    nsInt32 content2[] = {5, 6, 7, 8, 9};
    nsInt32 contentHeap1[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    nsInt32 contentHeap2[] = {11, 12, 13, 14, 15, 16, 17, 18, 19, 110, 111, 112, 113};

    {
      // local <-> local
      nsHybridArray<nsInt32, 8> a1;
      nsHybridArray<nsInt32, 16> a2;
      a1 = nsMakeArrayPtr(content1);
      a2 = nsMakeArrayPtr(content2);

      nsInt32* a1Ptr = a1.GetData();
      nsInt32* a2Ptr = a2.GetData();

      a1.Swap(a2);

      // Because the data points to the internal storage the pointers shouldn't change when swapping
      NS_TEST_BOOL(a1Ptr == a1.GetData());
      NS_TEST_BOOL(a2Ptr == a2.GetData());

      // The data however should be swapped
      NS_TEST_BOOL(a1.GetArrayPtr() == nsMakeArrayPtr(content2));
      NS_TEST_BOOL(a2.GetArrayPtr() == nsMakeArrayPtr(content1));

      NS_TEST_INT(a1.GetCapacity(), 8);
      NS_TEST_INT(a2.GetCapacity(), 16);
    }

    {
      // local <-> heap
      nsHybridArray<nsInt32, 8> a1;
      nsDynamicArray<nsInt32> a2;
      a1 = nsMakeArrayPtr(content1);
      a2 = nsMakeArrayPtr(contentHeap1);
      nsInt32* a1Ptr = a1.GetData();
      nsInt32* a2Ptr = a2.GetData();
      a1.Swap(a2);
      NS_TEST_BOOL(a1Ptr != a1.GetData());
      NS_TEST_BOOL(a2Ptr != a2.GetData());
      NS_TEST_BOOL(a1.GetArrayPtr() == nsMakeArrayPtr(contentHeap1));
      NS_TEST_BOOL(a2.GetArrayPtr() == nsMakeArrayPtr(content1));

      NS_TEST_INT(a1.GetCapacity(), 16);
      NS_TEST_INT(a2.GetCapacity(), 16);
    }

    {
      // heap <-> local
      nsHybridArray<nsInt32, 8> a1;
      nsHybridArray<nsInt32, 7> a2;
      a1 = nsMakeArrayPtr(content1);
      a2 = nsMakeArrayPtr(contentHeap1);
      nsInt32* a1Ptr = a1.GetData();
      nsInt32* a2Ptr = a2.GetData();
      a2.Swap(a1); // Swap is opposite direction as before
      NS_TEST_BOOL(a1Ptr != a1.GetData());
      NS_TEST_BOOL(a2Ptr != a2.GetData());
      NS_TEST_BOOL(a1.GetArrayPtr() == nsMakeArrayPtr(contentHeap1));
      NS_TEST_BOOL(a2.GetArrayPtr() == nsMakeArrayPtr(content1));

      NS_TEST_INT(a1.GetCapacity(), 16);
      NS_TEST_INT(a2.GetCapacity(), 16);
    }

    {
      // heap <-> heap
      nsDynamicArray<nsInt32> a1;
      nsHybridArray<nsInt32, 8> a2;
      a1 = nsMakeArrayPtr(contentHeap1);
      a2 = nsMakeArrayPtr(contentHeap2);
      nsInt32* a1Ptr = a1.GetData();
      nsInt32* a2Ptr = a2.GetData();
      a2.Swap(a1);
      NS_TEST_BOOL(a1Ptr != a1.GetData());
      NS_TEST_BOOL(a2Ptr != a2.GetData());
      NS_TEST_BOOL(a1.GetArrayPtr() == nsMakeArrayPtr(contentHeap2));
      NS_TEST_BOOL(a2.GetArrayPtr() == nsMakeArrayPtr(contentHeap1));

      NS_TEST_INT(a1.GetCapacity(), 16);
      NS_TEST_INT(a2.GetCapacity(), 16);
    }

    {
      // empty <-> local
      nsHybridArray<nsInt32, 8> a1, a2;
      a2 = nsMakeArrayPtr(content2);
      a1.Swap(a2);
      NS_TEST_BOOL(a1.GetArrayPtr() == nsMakeArrayPtr(content2));
      NS_TEST_BOOL(a2.IsEmpty());

      NS_TEST_INT(a1.GetCapacity(), 8);
      NS_TEST_INT(a2.GetCapacity(), 8);
    }

    {
      // empty <-> empty
      nsHybridArray<nsInt32, 8> a1, a2;
      a1.Swap(a2);
      NS_TEST_BOOL(a1.IsEmpty());
      NS_TEST_BOOL(a2.IsEmpty());

      NS_TEST_INT(a1.GetCapacity(), 8);
      NS_TEST_INT(a2.GetCapacity(), 8);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Move")
  {
    int counter = 0;
    {
      nsHybridArray<HybridArrayTestDetail::ExternalCounter, 2> a, b;
      NS_TEST_BOOL(counter == 0);

      a.PushBack(HybridArrayTestDetail::ExternalCounter(counter));
      NS_TEST_BOOL(counter == 1);

      b = std::move(a);
      NS_TEST_BOOL(counter == 1);
    }
    NS_TEST_BOOL(counter == 2);

    counter = 0;
    {
      nsHybridArray<HybridArrayTestDetail::ExternalCounter, 2> a, b;
      NS_TEST_BOOL(counter == 0);

      a.PushBack(HybridArrayTestDetail::ExternalCounter(counter));
      a.PushBack(HybridArrayTestDetail::ExternalCounter(counter));
      a.PushBack(HybridArrayTestDetail::ExternalCounter(counter));
      a.PushBack(HybridArrayTestDetail::ExternalCounter(counter));
      NS_TEST_BOOL(counter == 4);

      b = std::move(a);
      NS_TEST_BOOL(counter == 4);
    }
    NS_TEST_BOOL(counter == 8);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Use nsHybridArray with nsDynamicArray")
  {
    nsHybridArray<int, 16> a;

    TakesDynamicArray(a, 4, a.GetCount());
    NS_TEST_INT(a.GetCount(), 4);
    NS_TEST_INT(a.GetCapacity(), 16);

    for (int i = 0; i < (int)a.GetCount(); ++i)
    {
      NS_TEST_INT(a[i], i);
    }

    TakesDynamicArray(a, 12, a.GetCount());
    NS_TEST_INT(a.GetCount(), 16);
    NS_TEST_INT(a.GetCapacity(), 16);

    for (int i = 0; i < (int)a.GetCount(); ++i)
    {
      NS_TEST_INT(a[i], i);
    }

    TakesDynamicArray(a, 8, a.GetCount());
    NS_TEST_INT(a.GetCount(), 24);
    NS_TEST_INT(a.GetCapacity(), 32);

    for (int i = 0; i < (int)a.GetCount(); ++i)
    {
      NS_TEST_INT(a[i], i);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Nested arrays")
  {
    nsDynamicArray<nsHybridArray<HybridArrayTestDetail::NonMovableClass, 4>> a;

    for (int i = 0; i < 100; ++i)
    {
      nsHybridArray<HybridArrayTestDetail::NonMovableClass, 4> b;
      b.PushBack(HybridArrayTestDetail::NonMovableClass(i));

      a.PushBack(std::move(b));
    }

    for (int i = 0; i < 100; ++i)
    {
      auto& nonMoveable = a[i][0];

      NS_TEST_INT(nonMoveable.m_val, i);
      NS_TEST_BOOL(nonMoveable.m_pVal == &nonMoveable.m_val);
    }
  }
}

void TakesDynamicArray(nsDynamicArray<int>& ref_ar, int iNum, int iStart)
{
  for (int i = 0; i < iNum; ++i)
  {
    ref_ar.PushBack(iStart + i);
  }
}

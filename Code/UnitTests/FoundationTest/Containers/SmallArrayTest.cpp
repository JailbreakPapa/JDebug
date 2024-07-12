#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Containers/SmallArray.h>
#include <Foundation/Memory/CommonAllocators.h>
#include <Foundation/Strings/String.h>

namespace SmallArrayTestDetail
{

  class Dummy
  {
  public:
    int a;
    std::string s;

    Dummy()
      : a(0)
      , s("Test")
    {
    }
    Dummy(int a)
      : a(a)
      , s("Test")
    {
    }
    Dummy(const Dummy& other)

      = default;
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
  static nsSmallArray<T, 16> CreateArray(nsUInt32 uiSize, nsUInt32 uiOffset, nsUInt32 uiUserData)
  {
    nsSmallArray<T, 16> a;
    a.SetCount(static_cast<nsUInt16>(uiSize));

    for (nsUInt32 i = 0; i < uiSize; ++i)
    {
      a[i] = T(uiOffset + i);
    }

    a.template GetUserData<nsUInt32>() = uiUserData;

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
} // namespace SmallArrayTestDetail

static void TakesDynamicArray(nsDynamicArray<int>& ref_ar, int iNum, int iStart);

#if NS_ENABLED(NS_PLATFORM_64BIT)
static_assert(sizeof(nsSmallArray<nsInt32, 1>) == 16);
#else
static_assert(sizeof(nsSmallArray<nsInt32, 1>) == 12);
#endif

static_assert(nsGetTypeClass<nsSmallArray<nsInt32, 1>>::value == nsTypeIsMemRelocatable::value);
static_assert(nsGetTypeClass<nsSmallArray<SmallArrayTestDetail::NonMovableClass, 1>>::value == nsTypeIsClass::value);

NS_CREATE_SIMPLE_TEST(Containers, SmallArray)
{
  nsConstructionCounter::Reset();

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
    nsSmallArray<nsInt32, 16> a1;
    nsSmallArray<nsConstructionCounter, 16> a2;

    NS_TEST_BOOL(a1.GetCount() == 0);
    NS_TEST_BOOL(a2.GetCount() == 0);
    NS_TEST_BOOL(a1.IsEmpty());
    NS_TEST_BOOL(a2.IsEmpty());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Copy Constructor")
  {
    nsSmallArray<nsInt32, 16> a1;

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

    a1.GetUserData<nsUInt32>() = 11;

    nsSmallArray<nsInt32, 16> a2 = a1;
    nsSmallArray<nsInt32, 16> a3(a1);

    NS_TEST_BOOL(a1 == a2);
    NS_TEST_BOOL(a1 == a3);
    NS_TEST_BOOL(a2 == a3);

    NS_TEST_INT(a2.GetUserData<nsUInt32>(), 11);
    NS_TEST_INT(a3.GetUserData<nsUInt32>(), 11);

    nsInt32 test[] = {1, 2, 3, 4};
    nsArrayPtr<nsInt32> aptr(test);

    nsSmallArray<nsInt32, 16> a4(aptr);

    NS_TEST_BOOL(a4 == aptr);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Move Constructor / Operator")
  {
    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());

    {
      // move constructor external storage
      nsSmallArray<nsConstructionCounter, 16> a1(SmallArrayTestDetail::CreateArray<nsConstructionCounter>(100, 20, 11));

      NS_TEST_INT(a1.GetCount(), 100);
      for (nsUInt32 i = 0; i < a1.GetCount(); ++i)
        NS_TEST_INT(a1[i].m_iData, 20 + i);

      NS_TEST_INT(a1.GetUserData<nsUInt32>(), 11);

      // move operator external storage
      a1 = SmallArrayTestDetail::CreateArray<nsConstructionCounter>(200, 50, 22);

      NS_TEST_INT(a1.GetCount(), 200);
      for (nsUInt32 i = 0; i < a1.GetCount(); ++i)
        NS_TEST_INT(a1[i].m_iData, 50 + i);

      NS_TEST_INT(a1.GetUserData<nsUInt32>(), 22);
    }

    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());
    nsConstructionCounter::Reset();

    {
      // move constructor internal storage
      nsSmallArray<nsConstructionCounter, 16> a2(SmallArrayTestDetail::CreateArray<nsConstructionCounter>(10, 30, 11));

      NS_TEST_INT(a2.GetCount(), 10);
      for (nsUInt32 i = 0; i < a2.GetCount(); ++i)
        NS_TEST_INT(a2[i].m_iData, 30 + i);

      NS_TEST_INT(a2.GetUserData<nsUInt32>(), 11);

      // move operator internal storage
      a2 = SmallArrayTestDetail::CreateArray<nsConstructionCounter>(8, 70, 22);

      NS_TEST_INT(a2.GetCount(), 8);
      for (nsUInt32 i = 0; i < a2.GetCount(); ++i)
        NS_TEST_INT(a2[i].m_iData, 70 + i);

      NS_TEST_INT(a2.GetUserData<nsUInt32>(), 22);
    }

    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());
    nsConstructionCounter::Reset();

    nsConstructionCounterRelocatable::Reset();
    {
      // move constructor external storage relocatable
      nsSmallArray<nsConstructionCounterRelocatable, 16> a1(SmallArrayTestDetail::CreateArray<nsConstructionCounterRelocatable>(100, 20, 11));

      NS_TEST_BOOL(nsConstructionCounterRelocatable::HasDone(100, 0));

      NS_TEST_INT(a1.GetCount(), 100);
      for (nsUInt32 i = 0; i < a1.GetCount(); ++i)
        NS_TEST_INT(a1[i].m_iData, 20 + i);

      NS_TEST_INT(a1.GetUserData<nsUInt32>(), 11);

      // move operator external storage
      a1 = SmallArrayTestDetail::CreateArray<nsConstructionCounterRelocatable>(200, 50, 22);
      NS_TEST_BOOL(nsConstructionCounterRelocatable::HasDone(200, 100));

      NS_TEST_INT(a1.GetCount(), 200);
      for (nsUInt32 i = 0; i < a1.GetCount(); ++i)
        NS_TEST_INT(a1[i].m_iData, 50 + i);

      NS_TEST_INT(a1.GetUserData<nsUInt32>(), 22);
    }

    NS_TEST_BOOL(nsConstructionCounterRelocatable::HasAllDestructed());
    nsConstructionCounterRelocatable::Reset();

    {
      // move constructor internal storage relocatable
      nsSmallArray<nsConstructionCounterRelocatable, 16> a2(SmallArrayTestDetail::CreateArray<nsConstructionCounterRelocatable>(10, 30, 11));
      NS_TEST_BOOL(nsConstructionCounterRelocatable::HasDone(10, 0));

      NS_TEST_INT(a2.GetCount(), 10);
      for (nsUInt32 i = 0; i < a2.GetCount(); ++i)
        NS_TEST_INT(a2[i].m_iData, 30 + i);

      NS_TEST_INT(a2.GetUserData<nsUInt32>(), 11);

      // move operator internal storage
      a2 = SmallArrayTestDetail::CreateArray<nsConstructionCounterRelocatable>(8, 70, 22);
      NS_TEST_BOOL(nsConstructionCounterRelocatable::HasDone(8, 10));

      NS_TEST_INT(a2.GetCount(), 8);
      for (nsUInt32 i = 0; i < a2.GetCount(); ++i)
        NS_TEST_INT(a2[i].m_iData, 70 + i);

      NS_TEST_INT(a2.GetUserData<nsUInt32>(), 22);
    }

    NS_TEST_BOOL(nsConstructionCounterRelocatable::HasAllDestructed());
    nsConstructionCounterRelocatable::Reset();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Convert to ArrayPtr")
  {
    nsSmallArray<nsInt32, 16> a1;

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
    nsSmallArray<nsInt32, 16> a1, a2;

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
    nsSmallArray<nsInt32, 16> a1, a2;

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
    nsSmallArray<nsInt32, 16> a1;
    a1.SetCountUninitialized(100);

    for (nsInt32 i = 0; i < 100; ++i)
      a1[i] = i;

    for (nsInt32 i = 0; i < 100; ++i)
      NS_TEST_INT(a1[i], i);

    const nsSmallArray<nsInt32, 16> ca1 = a1;

    for (nsInt32 i = 0; i < 100; ++i)
      NS_TEST_INT(ca1[i], i);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetCount / GetCount / IsEmpty")
  {
    nsSmallArray<nsInt32, 16> a1;

    NS_TEST_BOOL(a1.IsEmpty());

    for (nsInt32 i = 0; i < 128; ++i)
    {
      a1.SetCount(static_cast<nsUInt16>(i + 1));
      NS_TEST_INT(a1[i], 0);
      a1[i] = i;

      NS_TEST_INT(a1.GetCount(), i + 1);
      NS_TEST_BOOL(!a1.IsEmpty());
    }

    for (nsInt32 i = 0; i < 128; ++i)
      NS_TEST_INT(a1[i], i);

    for (nsInt32 i = 128; i >= 0; --i)
    {
      a1.SetCount(static_cast<nsUInt16>(i));

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
      nsSmallArray<nsInt32, 2> a2;
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
    nsSmallArray<nsInt32, 2> a2;
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
    nsSmallArray<nsInt32, 16> a1;
    a1.Clear();

    a1.PushBack(3);
    a1.Clear();

    NS_TEST_BOOL(a1.IsEmpty());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Contains / IndexOf / LastIndexOf")
  {
    nsSmallArray<nsInt32, 16> a1;

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

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Insert")
  {
    nsSmallArray<nsInt32, 16> a1;

    // always inserts at the front
    for (nsInt32 i = 0; i < 100; ++i)
      a1.InsertAt(0, i);

    for (nsInt32 i = 0; i < 100; ++i)
      NS_TEST_INT(a1[i], 99 - i);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RemoveAndCopy")
  {
    nsSmallArray<nsInt32, 16> a1;

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
    nsSmallArray<nsInt32, 16> a1;

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
    nsSmallArray<nsInt32, 16> a1;

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
    nsSmallArray<nsInt32, 16> a1;

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
    nsSmallArray<nsInt32, 16> a1;

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
    nsSmallArray<nsInt32, 16> a1;

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

      nsSmallArray<nsConstructionCounter, 16> a1;
      nsSmallArray<nsConstructionCounter, 16> a2;

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
    nsSmallArray<nsInt32, 16> a;

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

    // this tests whether the static array is reused properly
    a.SetCount(15);
    a.Compact();
    NS_TEST_BOOL(a.GetHeapMemoryUsage() == 0);

    for (nsInt32 i = 0; i < 15; ++i)
      NS_TEST_INT(a[i], i);

    a.Clear();
    a.Compact();
    NS_TEST_BOOL(a.GetHeapMemoryUsage() == 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SortingPrimitives")
  {
    nsSmallArray<nsUInt32, 16> list;

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
    nsSmallArray<SmallArrayTestDetail::Dummy, 16> list;
    list.Reserve(128);

    for (nsUInt32 i = 0; i < 100; i++)
    {
      list.PushBack(SmallArrayTestDetail::Dummy(rand()));
    }
    list.Sort();

    SmallArrayTestDetail::Dummy last = 0;
    for (nsUInt32 i = 0; i < list.GetCount(); i++)
    {
      NS_TEST_BOOL(last <= list[i]);
      last = list[i];
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Various")
  {
    nsSmallArray<SmallArrayTestDetail::Dummy, 16> list;
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
    SmallArrayTestDetail::Dummy d = list.PeekBack();
    list.PopBack();
    NS_TEST_BOOL(d.a == 5);
    NS_TEST_BOOL(list.GetCount() == 4);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Assignment")
  {
    nsSmallArray<SmallArrayTestDetail::Dummy, 16> list;
    for (int i = 0; i < 16; i++)
    {
      list.PushBack(SmallArrayTestDetail::Dummy(rand()));
    }
    list.GetUserData<nsUInt32>() = 11;

    nsSmallArray<SmallArrayTestDetail::Dummy, 16> list2;
    for (int i = 0; i < 8; i++)
    {
      list2.PushBack(SmallArrayTestDetail::Dummy(rand()));
    }
    list2.GetUserData<nsUInt32>() = 22;

    list = list2;
    NS_TEST_INT(list.GetCount(), list2.GetCount());
    NS_TEST_INT(list.GetUserData<nsUInt32>(), list2.GetUserData<nsUInt32>());

    list2.Clear();
    NS_TEST_BOOL(list2.GetCount() == 0);

    list2 = list;
    NS_TEST_BOOL(list.PeekBack() == list2.PeekBack());
    NS_TEST_BOOL(list == list2);

    for (int i = 0; i < 16; i++)
    {
      list2.PushBack(SmallArrayTestDetail::Dummy(rand()));
    }

    list = list2;
    NS_TEST_BOOL(list.PeekBack() == list2.PeekBack());
    NS_TEST_BOOL(list == list2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Count")
  {
    nsSmallArray<SmallArrayTestDetail::Dummy, 16> list;
    for (int i = 0; i < 16; i++)
    {
      list.PushBack(SmallArrayTestDetail::Dummy(rand()));
    }
    list.SetCount(32);
    list.SetCount(4);

    list.Compact();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Reserve")
  {
    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());

    nsSmallArray<nsConstructionCounter, 16> a;

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

    nsSmallArray<nsConstructionCounter, 16> a;

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

    a.SetCount(10);
    NS_TEST_BOOL(nsConstructionCounter::HasDone(10, 0));

    // this time objects need to be relocated
    a.SetCount(200);
    NS_TEST_BOOL(nsConstructionCounter::HasDone(200, 10));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "STL Iterator")
  {
    nsSmallArray<nsInt32, 16> a1;

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
    const nsSmallArray<nsInt32, 16>& a2 = a1;

    // STL lower bound
    auto lb = std::lower_bound(begin(a2), end(a2), 400);
    NS_TEST_BOOL(*lb == a2[400]);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "STL Reverse Iterator")
  {
    nsSmallArray<nsInt32, 16> a1;

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
    const nsSmallArray<nsInt32, 16>& a2 = a1;

    // STL lower bound
    auto lb = std::lower_bound(rbegin(a2), rend(a2), 400);
    NS_TEST_BOOL(*lb == a2[1000 - 400 - 1]);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Move")
  {
    int counter = 0;
    {
      nsSmallArray<SmallArrayTestDetail::ExternalCounter, 2> a, b;
      NS_TEST_BOOL(counter == 0);

      a.PushBack(SmallArrayTestDetail::ExternalCounter(counter));
      NS_TEST_BOOL(counter == 1);

      b = std::move(a);
      NS_TEST_BOOL(counter == 1);
    }
    NS_TEST_BOOL(counter == 2);

    counter = 0;
    {
      nsSmallArray<SmallArrayTestDetail::ExternalCounter, 2> a, b;
      NS_TEST_BOOL(counter == 0);

      a.PushBack(SmallArrayTestDetail::ExternalCounter(counter));
      a.PushBack(SmallArrayTestDetail::ExternalCounter(counter));
      a.PushBack(SmallArrayTestDetail::ExternalCounter(counter));
      a.PushBack(SmallArrayTestDetail::ExternalCounter(counter));
      NS_TEST_BOOL(counter == 4);

      b = std::move(a);
      NS_TEST_BOOL(counter == 4);
    }
    NS_TEST_BOOL(counter == 8);
  }
}

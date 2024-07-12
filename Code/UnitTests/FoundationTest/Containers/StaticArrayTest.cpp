#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Containers/StaticArray.h>

namespace StaticArrayTestDetail
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
} // namespace StaticArrayTestDetail

#if NS_ENABLED(NS_PLATFORM_64BIT)
static_assert(sizeof(nsStaticArray<nsInt32, 1>) == 24);
#else
static_assert(sizeof(nsStaticArray<nsInt32, 1>) == 16);
#endif

static_assert(nsGetTypeClass<nsStaticArray<nsInt32, 1>>::value == nsTypeIsMemRelocatable::value);
static_assert(nsGetTypeClass<nsStaticArray<StaticArrayTestDetail::Dummy, 1>>::value == nsTypeIsClass::value);

NS_CREATE_SIMPLE_TEST(Containers, StaticArray)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
    nsStaticArray<nsInt32, 32> a1;
    nsStaticArray<nsConstructionCounter, 32> a2;

    NS_TEST_BOOL(a1.GetCount() == 0);
    NS_TEST_BOOL(a2.GetCount() == 0);
    NS_TEST_BOOL(a1.IsEmpty());
    NS_TEST_BOOL(a2.IsEmpty());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Copy Constructor")
  {
    nsStaticArray<nsInt32, 32> a1;

    for (nsInt32 i = 0; i < 32; ++i)
      a1.PushBack(rand() % 100000);

    nsStaticArray<nsInt32, 64> a2 = a1;
    nsStaticArray<nsInt32, 32> a3(a1);

    NS_TEST_BOOL(a1.GetArrayPtr() == a2);
    NS_TEST_BOOL(a1 == a3);
    NS_TEST_BOOL(a2 == a3.GetArrayPtr());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Convert to ArrayPtr")
  {
    nsStaticArray<nsInt32, 128> a1;

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
    nsStaticArray<nsInt32, 128> a1, a2;

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
    nsStaticArray<nsInt32, 128> a1, a2;

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
    nsStaticArray<nsInt32, 128> a1;
    a1.SetCountUninitialized(100);

    for (nsInt32 i = 0; i < 100; ++i)
      a1[i] = i;

    for (nsInt32 i = 0; i < 100; ++i)
      NS_TEST_INT(a1[i], i);

    const nsStaticArray<nsInt32, 128> ca1 = a1;

    for (nsInt32 i = 0; i < 100; ++i)
      NS_TEST_INT(ca1[i], i);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetCount / GetCount / IsEmpty")
  {
    nsStaticArray<nsInt32, 128> a1;

    NS_TEST_BOOL(a1.IsEmpty());

    for (nsInt32 i = 0; i < 128; ++i)
    {
      a1.SetCount(i + 1);
      NS_TEST_INT(a1[i], 0);
      a1[i] = i;

      NS_TEST_INT((int)a1.GetCount(), i + 1);
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
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Clear")
  {
    nsStaticArray<nsInt32, 128> a1;
    a1.Clear();

    a1.PushBack(3);
    a1.Clear();

    NS_TEST_BOOL(a1.IsEmpty());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Contains / IndexOf / LastIndexOf")
  {
    nsStaticArray<nsInt32, 128> a1;

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
    nsStaticArray<nsInt32, 128> a1;

    // always inserts at the front
    for (nsInt32 i = 0; i < 100; ++i)
      a1.InsertAt(0, i);

    for (nsInt32 i = 0; i < 100; ++i)
      NS_TEST_INT(a1[i], 99 - i);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RemoveAndCopy")
  {
    nsStaticArray<nsInt32, 128> a1;

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
    nsStaticArray<nsInt32, 128> a1;

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
    nsStaticArray<nsInt32, 128> a1;

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
    nsStaticArray<nsInt32, 128> a1;

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
    nsStaticArray<nsInt32, 128> a1;

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

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Construction / Destruction")
  {
    {
      NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());

      nsStaticArray<nsConstructionCounter, 128> a1;
      nsStaticArray<nsConstructionCounter, 100> a2;

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

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SortingPrimitives")
  {
    nsStaticArray<nsUInt32, 128> list;

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
    nsStaticArray<StaticArrayTestDetail::Dummy, 128> list;

    for (nsUInt32 i = 0; i < 100; i++)
    {
      list.PushBack(StaticArrayTestDetail::Dummy(rand()));
    }
    list.Sort();

    StaticArrayTestDetail::Dummy last = 0;
    for (nsUInt32 i = 0; i < list.GetCount(); i++)
    {
      NS_TEST_BOOL(last <= list[i]);
      last = list[i];
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Various")
  {
    nsStaticArray<StaticArrayTestDetail::Dummy, 32> list;
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
    StaticArrayTestDetail::Dummy d = list.PeekBack();
    list.PopBack();
    NS_TEST_BOOL(d.a == 5);
    NS_TEST_BOOL(list.GetCount() == 4);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Assignment")
  {
    nsStaticArray<StaticArrayTestDetail::Dummy, 32> list;
    for (int i = 0; i < 16; i++)
    {
      list.PushBack(StaticArrayTestDetail::Dummy(rand()));
    }

    nsStaticArray<StaticArrayTestDetail::Dummy, 32> list2;
    for (int i = 0; i < 8; i++)
    {
      list2.PushBack(StaticArrayTestDetail::Dummy(rand()));
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
      list2.PushBack(StaticArrayTestDetail::Dummy(rand()));
    }

    list = list2;
    NS_TEST_BOOL(list.PeekBack() == list2.PeekBack());
    NS_TEST_BOOL(list == list2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Count")
  {
    nsStaticArray<StaticArrayTestDetail::Dummy, 32> list;
    for (int i = 0; i < 16; i++)
    {
      list.PushBack(StaticArrayTestDetail::Dummy(rand()));
    }
    list.SetCount(32);
    list.SetCount(4);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "STL Iterator")
  {
    nsStaticArray<nsInt32, 1024> a1;

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
    const nsStaticArray<nsInt32, 1024>& a2 = a1;

    // STL lower bound
    auto lb = std::lower_bound(begin(a2), end(a2), 400);
    NS_TEST_BOOL(*lb == a2[400]);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "STL Reverse Iterator")
  {
    nsStaticArray<nsInt32, 1024> a1;

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
    const nsStaticArray<nsInt32, 1024>& a2 = a1;

    // STL lower bound
    auto lb = std::lower_bound(rbegin(a2), rend(a2), 400);
    NS_TEST_BOOL(*lb == a2[1000 - 400 - 1]);
  }
}

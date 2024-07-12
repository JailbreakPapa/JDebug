#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/Types/UniquePtr.h>

namespace DequeTestDetail
{
  using st = nsConstructionCounter;

  static nsDeque<st> CreateArray(nsUInt32 uiSize, nsUInt32 uiOffset)
  {
    nsDeque<st> a;
    a.SetCount(uiSize);

    for (nsUInt32 i = 0; i < uiSize; ++i)
      a[i] = uiOffset + i;

    return a;
  }
} // namespace DequeTestDetail

NS_CREATE_SIMPLE_TEST(Containers, Deque)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Fill / Empty (Sawtooth)")
  {
    nsDeque<nsInt32> d;

    nsUInt32 uiVal = 0;

    for (nsInt32 i = 0; i < 10000; ++i)
      d.PushBack(uiVal++);

    // this is kind of the worst case scenario, as it will deallocate and reallocate chunks in every loop
    // the smaller the chunk size, the more allocations will happen
    for (nsUInt32 s2 = 0; s2 < 10; ++s2)
    {
      for (nsInt32 i = 0; i < 1000; ++i)
        d.PopBack();
      for (nsInt32 i = 0; i < 1000; ++i)
        d.PushBack(uiVal++);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Fill / Empty")
  {
    nsDeque<nsInt32> d;

    NS_TEST_BOOL(d.GetHeapMemoryUsage() == 0);

    for (nsInt32 i = 0; i < 10000; ++i)
      d.PushBack(i);

    NS_TEST_BOOL(d.GetHeapMemoryUsage() > 10000 * sizeof(nsInt32));

    for (nsInt32 i = 0; i < 10000; ++i)
      d.PopFront();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Queue Back")
  {
    nsDeque<nsInt32> d;

    d.PushBack(0);

    for (nsInt32 i = 0; i < 10000; ++i)
    {
      d.PushBack(i);
      d.PopFront();
    }

    d.Compact();

    for (nsInt32 i = 0; i < 10000; ++i)
    {
      d.PushBack(i);
      d.PopFront();
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Queue Front")
  {
    nsDeque<nsInt32> d;

    d.PushBack(0);

    for (nsInt32 i = 0; i < 10000; ++i)
    {
      d.PushFront(i);
      d.PopBack();
    }

    d.Compact();

    for (nsInt32 i = 0; i < 10000; ++i)
    {
      d.PushFront(i);
      d.PopBack();
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "POD Types")
  {
    nsDeque<nsInt32> d1;
    d1.SetCount(5120);

    d1.Compact();

    NS_TEST_BOOL(d1.GetCount() == 5120);


    d1.SetCount(1);
    d1.Compact();

    NS_TEST_BOOL(d1.GetHeapMemoryUsage() > 0);

    d1.Clear();

    d1.Compact();

    NS_TEST_BOOL(d1.GetHeapMemoryUsage() == 0);
  }

  nsStartup::ShutdownCoreSystems();

  nsStartup::StartupCoreSystems();

  NS_TEST_BLOCK(nsTestBlock::Enabled, "POD Types")
  {
    nsDeque<nsInt32> d1;
    d1.SetCount(1000);

    NS_TEST_BOOL(d1.GetCount() == 1000);
    d1.Clear();
    NS_TEST_BOOL(d1.IsEmpty());

    for (nsInt32 i = 1; i < 1000; ++i)
    {
      d1.PushBack(i);

      NS_TEST_BOOL(d1.PeekBack() == i);
      NS_TEST_BOOL(d1.GetCount() == i);
      NS_TEST_BOOL(!d1.IsEmpty());
    }

    d1.Clear();
    NS_TEST_BOOL(d1.IsEmpty());

    for (nsInt32 i = 1; i < 1000; ++i)
    {
      d1.PushFront(i);

      NS_TEST_BOOL(d1.PeekFront() == i);
      NS_TEST_BOOL(d1.GetCount() == i);
      NS_TEST_BOOL(!d1.IsEmpty());
    }

    d1.Clear();
    NS_TEST_BOOL(d1.IsEmpty());

    for (nsInt32 i = 1; i <= 1000; ++i)
    {
      d1.PushFront(i);
      d1.PushBack(i);

      NS_TEST_BOOL(d1.PeekFront() == i);
      NS_TEST_BOOL(d1.PeekBack() == i);
      NS_TEST_BOOL(d1.GetCount() == i * 2);
      NS_TEST_BOOL(!d1.IsEmpty());
    }

    nsDeque<nsInt32> d2;
    d2 = d1;

    for (nsInt32 i = 1000; i >= 1; --i)
    {
      NS_TEST_BOOL(d1.PeekFront() == i);
      NS_TEST_BOOL(d1.PeekBack() == i);
      NS_TEST_BOOL(d1.GetCount() == i * 2);
      NS_TEST_BOOL(!d1.IsEmpty());

      d1.PopFront();
      d1.PopBack();


      NS_TEST_BOOL(d2.PeekFront() == i);
      NS_TEST_BOOL(d2.PeekBack() == i);
      NS_TEST_BOOL(d2.GetCount() == i * 2);
      NS_TEST_BOOL(!d2.IsEmpty());

      d2.PopFront();
      d2.PopBack();
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "STL Iterator")
    {
      nsDeque<nsInt32> a1;
      for (nsInt32 i = 0; i < 1000; ++i)
        a1.PushBack(1000 - i - 1);

      // STL sort
      std::sort(begin(a1), end(a1));

      nsUInt32 prev = 0;
      for (nsUInt32 val : a1)
      {
        NS_TEST_BOOL(prev <= val);
        prev = val;
      }

      // STL lower bound
      auto lb = std::lower_bound(begin(a1), end(a1), 400);
      NS_TEST_BOOL(*lb == a1[400]);
    }

    NS_TEST_BLOCK(nsTestBlock::Enabled, "STL Reverse Iterator")
    {
      nsDeque<nsInt32> a1;
      for (nsInt32 i = 0; i < 1000; ++i)
        a1.PushBack(1000 - i - 1);

      std::sort(rbegin(a1), rend(a1));

      // foreach
      nsUInt32 prev = 1000;
      for (nsUInt32 val : a1)
      {
        NS_TEST_BOOL(prev >= val);
        prev = val;
      }

      // const array
      const nsDeque<nsInt32>& a2 = a1;

      // STL lower bound
      auto lb2 = std::lower_bound(rbegin(a2), rend(a2), 400);
      NS_TEST_INT(*lb2, a2[1000 - 400 - 1]);
    }
  }

  nsStartup::ShutdownCoreSystems();

  nsStartup::StartupCoreSystems();

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Non-POD Types")
  {
    NS_TEST_BOOL(DequeTestDetail::st::HasAllDestructed());

    {
      nsDeque<DequeTestDetail::st> v1;
      NS_TEST_BOOL(DequeTestDetail::st::HasDone(0, 0));

      {
        v1.PushBack(DequeTestDetail::st(3));
        NS_TEST_BOOL(DequeTestDetail::st::HasDone(2, 1));

        v1.PushBack();
        NS_TEST_BOOL(DequeTestDetail::st::HasDone(1, 0));

        v1.PopBack();
        NS_TEST_BOOL(DequeTestDetail::st::HasDone(0, 1));
      }
      {
        v1.PushFront(DequeTestDetail::st(3));
        NS_TEST_BOOL(DequeTestDetail::st::HasDone(2, 1));

        v1.PushFront();
        NS_TEST_BOOL(DequeTestDetail::st::HasDone(1, 0));

        v1.PopFront();
        NS_TEST_BOOL(DequeTestDetail::st::HasDone(0, 1));
      }

      NS_TEST_BOOL(v1.GetCount() == 2);

      v1.SetCount(12);
      NS_TEST_BOOL(DequeTestDetail::st::HasDone(10, 0));

      {
        nsDeque<DequeTestDetail::st> v2;
        NS_TEST_BOOL(DequeTestDetail::st::HasDone(0, 0));

        v2 = v1;
        NS_TEST_BOOL(DequeTestDetail::st::HasDone(12, 0));

        v2.Clear();
        NS_TEST_BOOL(DequeTestDetail::st::HasDone(0, 12));

        nsDeque<DequeTestDetail::st> v3(v1);
        NS_TEST_BOOL(DequeTestDetail::st::HasDone(12, 0));

        nsDeque<DequeTestDetail::st> v4(v1);
        NS_TEST_BOOL(DequeTestDetail::st::HasDone(12, 0));

        v4.SetCount(0);
        NS_TEST_BOOL(DequeTestDetail::st::HasDone(0, 12));
      }

      NS_TEST_BOOL(DequeTestDetail::st::HasDone(0, 12));
    }

    NS_TEST_BOOL(DequeTestDetail::st::HasAllDestructed());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SortingPrimitives")
  {
    nsDeque<nsUInt32> list;

    list.Sort();

    for (nsUInt32 i = 0; i < 245; i++)
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



  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
    nsDeque<nsInt32> a1;
    nsDeque<DequeTestDetail::st> a2;

    NS_TEST_BOOL(a1.GetCount() == 0);
    NS_TEST_BOOL(a2.GetCount() == 0);
    NS_TEST_BOOL(a1.IsEmpty());
    NS_TEST_BOOL(a2.IsEmpty());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Copy Constructor")
  {
    nsDeque<nsInt32> a1;

    for (nsInt32 i = 0; i < 32; ++i)
      a1.PushBack(rand() % 100000);

    nsDeque<nsInt32> a2 = a1;
    nsDeque<nsInt32> a3(a1);

    NS_TEST_BOOL(a1.GetCount() == a2.GetCount());
    NS_TEST_BOOL(a1.GetCount() == a3.GetCount());

    for (nsUInt32 i = 0; i < a1.GetCount(); ++i)
    {
      NS_TEST_BOOL(a1[i] == a2[i]);
      NS_TEST_BOOL(a1[i] == a3[i]);
      NS_TEST_BOOL(a2[i] == a3[i]);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Move Constructor / Operator")
  {
    NS_TEST_BOOL(DequeTestDetail::st::HasAllDestructed());

    {
      // move constructor
      nsDeque<DequeTestDetail::st> a1(DequeTestDetail::CreateArray(100, 20));

      NS_TEST_INT(a1.GetCount(), 100);
      for (nsUInt32 i = 0; i < a1.GetCount(); ++i)
        NS_TEST_INT(a1[i].m_iData, 20 + i);

      // move operator
      a1 = DequeTestDetail::CreateArray(200, 50);

      NS_TEST_INT(a1.GetCount(), 200);
      for (nsUInt32 i = 0; i < a1.GetCount(); ++i)
        NS_TEST_INT(a1[i].m_iData, 50 + i);
    }

    NS_TEST_BOOL(DequeTestDetail::st::HasAllDestructed());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator = / operator == / operator !=")
  {
    nsDeque<nsInt32> a1;
    nsDeque<nsInt32> a2;

    NS_TEST_BOOL(a1 == a2);

    for (nsInt32 i = 0; i < 100; ++i)
      a1.PushBack(i);

    NS_TEST_BOOL(a1 != a2);

    a2 = a1;

    NS_TEST_BOOL(a1.GetCount() == a2.GetCount());

    for (nsUInt32 i = 0; i < a1.GetCount(); ++i)
      NS_TEST_BOOL(a1[i] == a2[i]);

    NS_TEST_BOOL(a1 == a2);
    NS_TEST_BOOL(a2 == a1);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Index operator")
  {
    nsDeque<nsInt32> a1;
    a1.SetCount(100);

    for (nsInt32 i = 0; i < 100; ++i)
      a1[i] = i;

    for (nsInt32 i = 0; i < 100; ++i)
      NS_TEST_INT(a1[i], i);

    nsDeque<nsInt32> ca1;
    ca1 = a1;

    for (nsInt32 i = 0; i < 100; ++i)
      NS_TEST_INT(ca1[i], i);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetCount / GetCount / IsEmpty")
  {
    nsDeque<nsInt32> a1;

    NS_TEST_BOOL(a1.IsEmpty());

    for (nsInt32 i = 0; i < 128; ++i)
    {
      a1.SetCount(i + 1);
      NS_TEST_INT(a1[i], 0); // default init
      a1[i] = i;
      NS_TEST_INT(a1[i], i);

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
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetCountUninitialized")
  {
    nsDeque<nsInt32> a1;

    NS_TEST_BOOL(a1.IsEmpty());

    for (nsInt32 i = 0; i < 128; ++i)
    {
      a1.SetCountUninitialized(i + 1);
      // no default init
      a1[i] = i;
      NS_TEST_INT(a1[i], i);

      NS_TEST_INT(a1.GetCount(), i + 1);
      NS_TEST_BOOL(!a1.IsEmpty());
    }

    for (nsInt32 i = 0; i < 128; ++i)
      NS_TEST_INT(a1[i], i);

    for (nsInt32 i = 128; i >= 0; --i)
    {
      a1.SetCountUninitialized(i);

      NS_TEST_INT(a1.GetCount(), i);

      for (nsInt32 i2 = 0; i2 < i; ++i2)
        NS_TEST_INT(a1[i2], i2);
    }

    NS_TEST_BOOL(a1.IsEmpty());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "EnsureCount")
  {
    nsDeque<nsInt32> a1;

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
    nsDeque<nsInt32> a1;
    a1.Clear();

    a1.PushBack(3);
    a1.Clear();

    NS_TEST_BOOL(a1.IsEmpty());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Contains / IndexOf / LastIndexOf")
  {
    nsDeque<nsInt32> a1;

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
    nsDeque<nsInt32> a1;

    // always inserts at the front
    for (nsInt32 i = 0; i < 100; ++i)
      a1.InsertAt(0, i);

    for (nsInt32 i = 0; i < 100; ++i)
      NS_TEST_INT(a1[i], 99 - i);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "RemoveAndCopy")
  {
    nsDeque<nsInt32> a1;

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
    nsDeque<nsInt32> a1;

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
    nsDeque<nsInt32> a1;

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
    nsDeque<nsInt32> a1;

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

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExpandAndGetRef")
  {
    nsDeque<nsInt32> a1;

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

  NS_TEST_BLOCK(nsTestBlock::Enabled, "PushBack / PopBack / PeekBack")
  {
    nsDeque<nsInt32> a1;

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

  NS_TEST_BLOCK(nsTestBlock::Enabled, "PushFront / PopFront / PeekFront")
  {
    nsDeque<nsInt32> a1;

    for (nsInt32 i = 0; i < 10; ++i)
    {
      a1.PushFront(i);
      NS_TEST_INT(a1.PeekFront(), i);
    }

    for (nsInt32 i = 9; i >= 0; --i)
    {
      NS_TEST_INT(a1.PeekFront(), i);
      a1.PopFront();
    }

    a1.PushFront(23);
    a1.PushFront(2);
    a1.PushFront(3);

    a1.PopFront(2);
    NS_TEST_INT(a1.PeekFront(), 23);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Construction / Destruction")
  {
    {
      NS_TEST_BOOL(DequeTestDetail::st::HasAllDestructed());

      nsDeque<DequeTestDetail::st> a1;
      nsDeque<DequeTestDetail::st> a2;

      NS_TEST_BOOL(DequeTestDetail::st::HasDone(0, 0)); // nothing has been constructed / destructed in between
      NS_TEST_BOOL(DequeTestDetail::st::HasAllDestructed());

      a1.PushBack(DequeTestDetail::st(1));
      NS_TEST_BOOL(DequeTestDetail::st::HasDone(2, 1)); // one temporary, one final (copy constructed)

      a1.InsertAt(0, DequeTestDetail::st(2));
      NS_TEST_BOOL(DequeTestDetail::st::HasDone(2, 1)); // one temporary, one final (copy constructed)

      a2 = a1;
      NS_TEST_BOOL(DequeTestDetail::st::HasDone(2, 0)); // two copies

      a1.Clear();
      NS_TEST_BOOL(DequeTestDetail::st::HasDone(0, 2));

      a1.PushBack(DequeTestDetail::st(3));
      a1.PushBack(DequeTestDetail::st(4));
      a1.PushBack(DequeTestDetail::st(5));
      a1.PushBack(DequeTestDetail::st(6));

      NS_TEST_BOOL(DequeTestDetail::st::HasDone(8, 4)); // four temporaries

      a1.RemoveAndCopy(DequeTestDetail::st(3));
      NS_TEST_BOOL(DequeTestDetail::st::HasDone(1, 2)); // one temporary, one destroyed

      a1.RemoveAndCopy(DequeTestDetail::st(3));
      NS_TEST_BOOL(DequeTestDetail::st::HasDone(1, 1)); // one temporary, none destroyed

      a1.RemoveAtAndCopy(0);
      NS_TEST_BOOL(DequeTestDetail::st::HasDone(0, 1)); // one destroyed

      a1.RemoveAtAndSwap(0);
      NS_TEST_BOOL(DequeTestDetail::st::HasDone(0, 1)); // one destroyed
    }

    // tests the destructor of a2 and a1
    NS_TEST_BOOL(DequeTestDetail::st::HasAllDestructed());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Reserve")
  {
    NS_TEST_BOOL(DequeTestDetail::st::HasAllDestructed());

    nsDeque<DequeTestDetail::st> a;

    NS_TEST_BOOL(DequeTestDetail::st::HasDone(0, 0)); // nothing has been constructed / destructed in between
    NS_TEST_BOOL(DequeTestDetail::st::HasAllDestructed());

    a.Reserve(100);

    NS_TEST_BOOL(DequeTestDetail::st::HasDone(0, 0)); // nothing has been constructed / destructed in between
    NS_TEST_BOOL(DequeTestDetail::st::HasAllDestructed());

    a.SetCount(10);
    NS_TEST_BOOL(DequeTestDetail::st::HasDone(10, 0));

    a.Reserve(100);
    NS_TEST_BOOL(DequeTestDetail::st::HasDone(0, 0));

    a.SetCount(100);
    NS_TEST_BOOL(DequeTestDetail::st::HasDone(90, 0));

    a.Reserve(200);
    NS_TEST_BOOL(DequeTestDetail::st::HasDone(0, 0)); // nothing had to be copied over

    a.SetCount(200);
    NS_TEST_BOOL(DequeTestDetail::st::HasDone(100, 0));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Compact")
  {
    NS_TEST_BOOL(DequeTestDetail::st::HasAllDestructed());

    nsDeque<DequeTestDetail::st> a;

    NS_TEST_BOOL(DequeTestDetail::st::HasDone(0, 0)); // nothing has been constructed / destructed in between
    NS_TEST_BOOL(DequeTestDetail::st::HasAllDestructed());

    a.SetCount(100);
    NS_TEST_BOOL(DequeTestDetail::st::HasDone(100, 0));

    a.SetCount(200);
    NS_TEST_BOOL(DequeTestDetail::st::HasDone(100, 0));

    a.SetCount(10);
    NS_TEST_BOOL(DequeTestDetail::st::HasDone(0, 190));

    // no reallocations and copying, if the memory is already available
    a.SetCount(200);
    NS_TEST_BOOL(DequeTestDetail::st::HasDone(190, 0));

    a.SetCount(10);
    NS_TEST_BOOL(DequeTestDetail::st::HasDone(0, 190));

    // now we remove the spare memory
    a.Compact();
    NS_TEST_BOOL(DequeTestDetail::st::HasDone(0, 0));

    // this time the array needs to be relocated, and thus the already present elements need to be copied
    a.SetCount(200);
    NS_TEST_BOOL(DequeTestDetail::st::HasDone(190, 0));

    // this does not deallocate memory
    a.Clear();
    NS_TEST_BOOL(DequeTestDetail::st::HasDone(0, 200));

    a.SetCount(100);
    NS_TEST_BOOL(DequeTestDetail::st::HasDone(100, 0));

    // therefore no object relocation
    a.SetCount(200);
    NS_TEST_BOOL(DequeTestDetail::st::HasDone(100, 0));

    a.Clear();
    NS_TEST_BOOL(DequeTestDetail::st::HasDone(0, 200));

    // this will deallocate ALL memory
    a.Compact();

    a.SetCount(100);
    NS_TEST_BOOL(DequeTestDetail::st::HasDone(100, 0));

    // this time objects need to be relocated
    a.SetCount(200);
    NS_TEST_BOOL(DequeTestDetail::st::HasDone(100, 0));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetContiguousRange")
  {
    // deques allocate data in 4 KB chunks, so an integer deque will have 1024 ints per chunk

    nsDeque<nsInt32> d;

    for (nsUInt32 i = 0; i < 100 * 1024; ++i)
      d.PushBack(i);

    nsDynamicArray<nsInt32> a;
    a.SetCountUninitialized(d.GetCount());

    nsUInt32 uiArrayPos = 0;

    for (nsUInt32 i = 0; i < 100; ++i)
    {
      const nsUInt32 uiOffset = i * 1024 + i;

      const nsUInt32 uiRange = d.GetContiguousRange(uiOffset);

      NS_TEST_INT(uiRange, 1024 - i);

      nsMemoryUtils::Copy(&a[uiArrayPos], &d[uiOffset], uiRange);

      uiArrayPos += uiRange;
    }

    a.SetCountUninitialized(uiArrayPos);

    uiArrayPos = 0;

    for (nsUInt32 i = 0; i < 100; ++i)
    {
      const nsUInt32 uiOffset = i * 1024 + i;
      const nsUInt32 uiRange = 1024 - i;

      for (nsUInt32 r = 0; r < uiRange; ++r)
      {
        NS_TEST_INT(a[uiArrayPos], uiOffset + r);
        ++uiArrayPos;
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Swap")
  {
    nsDeque<nsInt32> a1, a2;

    nsInt32 content1[] = {1, 2, 3, 4};
    nsInt32 content2[] = {5, 6, 7, 8, 9};
    for (nsInt32 i : content1)
    {
      a1.PushBack(i);
    }
    for (nsInt32 i : content2)
    {
      a2.PushBack(i);
    }

    nsInt32* a1Ptr = &a1[0];
    nsInt32* a2Ptr = &a2[0];

    a1.Swap(a2);

    // The pointers should be simply swapped
    NS_TEST_BOOL(a2Ptr == &a1[0]);
    NS_TEST_BOOL(a1Ptr == &a2[0]);

    NS_TEST_INT(NS_ARRAY_SIZE(content1), a2.GetCount());
    NS_TEST_INT(NS_ARRAY_SIZE(content2), a1.GetCount());

    // The data should be swapped
    for (int i = 0; i < NS_ARRAY_SIZE(content1); ++i)
    {
      NS_TEST_INT(content1[i], a2[i]);
    }
    for (int i = 0; i < NS_ARRAY_SIZE(content2); ++i)
    {
      NS_TEST_INT(content2[i], a1[i]);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Move PushBack / PushFront")
  {
    nsDeque<nsUniquePtr<nsUInt32>> a1, a2;
    a1.PushBack(nsUniquePtr<nsUInt32>(NS_DEFAULT_NEW(nsUInt32, 1)));
    a1.PushBack(nsUniquePtr<nsUInt32>(NS_DEFAULT_NEW(nsUInt32, 2)));

    a2.PushFront(nsUniquePtr<nsUInt32>(NS_DEFAULT_NEW(nsUInt32, 3)));
    a2.PushFront(nsUniquePtr<nsUInt32>(NS_DEFAULT_NEW(nsUInt32, 4)));

    a1.Swap(a2);

    NS_TEST_INT(*a1[0].Borrow(), 4);
    NS_TEST_INT(*a1[1].Borrow(), 3);

    NS_TEST_INT(*a2[0].Borrow(), 1);
    NS_TEST_INT(*a2[1].Borrow(), 2);
  }
}

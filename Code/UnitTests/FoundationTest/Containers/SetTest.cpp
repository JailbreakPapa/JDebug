#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Containers/Set.h>
#include <Foundation/Memory/CommonAllocators.h>

NS_CREATE_SIMPLE_TEST(Containers, Set)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
    nsSet<nsUInt32> m;
    nsSet<nsConstructionCounter, nsUInt32> m2;
    nsSet<nsConstructionCounter, nsConstructionCounter> m3;
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsEmpty")
  {
    nsSet<nsUInt32> m;
    NS_TEST_BOOL(m.IsEmpty());

    m.Insert(1);
    NS_TEST_BOOL(!m.IsEmpty());

    m.Clear();
    NS_TEST_BOOL(m.IsEmpty());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetCount")
  {
    nsSet<nsUInt32> m;
    NS_TEST_INT(m.GetCount(), 0);

    m.Insert(0);
    NS_TEST_INT(m.GetCount(), 1);

    m.Insert(1);
    NS_TEST_INT(m.GetCount(), 2);

    m.Insert(2);
    NS_TEST_INT(m.GetCount(), 3);

    m.Insert(1);
    NS_TEST_INT(m.GetCount(), 3);

    m.Clear();
    NS_TEST_INT(m.GetCount(), 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Clear")
  {
    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());

    {
      nsSet<nsConstructionCounter> m1;
      m1.Insert(nsConstructionCounter(1));
      NS_TEST_BOOL(nsConstructionCounter::HasDone(2, 1));

      m1.Insert(nsConstructionCounter(3));
      NS_TEST_BOOL(nsConstructionCounter::HasDone(2, 1));

      m1.Insert(nsConstructionCounter(1));
      NS_TEST_BOOL(nsConstructionCounter::HasDone(1, 1)); // nothing new to create, so only the one temporary is used

      m1.Clear();
      NS_TEST_BOOL(nsConstructionCounter::HasDone(0, 2));
      NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());
    }

    {
      nsSet<nsConstructionCounter> m1;
      m1.Insert(nsConstructionCounter(0));
      NS_TEST_BOOL(nsConstructionCounter::HasDone(2, 1)); // one temporary

      m1.Insert(nsConstructionCounter(1));
      NS_TEST_BOOL(nsConstructionCounter::HasDone(2, 1)); // one temporary

      m1.Insert(nsConstructionCounter(0));
      NS_TEST_BOOL(nsConstructionCounter::HasDone(1, 1)); // nothing new to create, so only the one temporary is used

      m1.Clear();
      NS_TEST_BOOL(nsConstructionCounter::HasDone(0, 2));
      NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Insert")
  {
    nsSet<nsUInt32> m;
    NS_TEST_BOOL(m.GetHeapMemoryUsage() == 0);

    NS_TEST_BOOL(m.Insert(1).IsValid());
    NS_TEST_BOOL(m.Insert(1).IsValid());

    m.Insert(3);
    auto it7 = m.Insert(7);
    m.Insert(9);
    m.Insert(4);
    m.Insert(2);
    m.Insert(8);
    m.Insert(5);
    m.Insert(6);

    NS_TEST_BOOL(m.Insert(1).Key() == 1);
    NS_TEST_BOOL(m.Insert(3).Key() == 3);
    NS_TEST_BOOL(m.Insert(7) == it7);

    NS_TEST_BOOL(m.GetHeapMemoryUsage() >= sizeof(nsUInt32) * 1 * 9);

    NS_TEST_BOOL(m.Find(1).IsValid());
    NS_TEST_BOOL(m.Find(2).IsValid());
    NS_TEST_BOOL(m.Find(3).IsValid());
    NS_TEST_BOOL(m.Find(4).IsValid());
    NS_TEST_BOOL(m.Find(5).IsValid());
    NS_TEST_BOOL(m.Find(6).IsValid());
    NS_TEST_BOOL(m.Find(7).IsValid());
    NS_TEST_BOOL(m.Find(8).IsValid());
    NS_TEST_BOOL(m.Find(9).IsValid());

    NS_TEST_BOOL(!m.Find(0).IsValid());
    NS_TEST_BOOL(!m.Find(10).IsValid());

    NS_TEST_INT(m.GetCount(), 9);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Contains")
  {
    nsSet<nsUInt32> m;
    m.Insert(1);
    m.Insert(3);
    m.Insert(7);
    m.Insert(9);
    m.Insert(4);
    m.Insert(2);
    m.Insert(8);
    m.Insert(5);
    m.Insert(6);

    NS_TEST_BOOL(m.Contains(1));
    NS_TEST_BOOL(m.Contains(2));
    NS_TEST_BOOL(m.Contains(3));
    NS_TEST_BOOL(m.Contains(4));
    NS_TEST_BOOL(m.Contains(5));
    NS_TEST_BOOL(m.Contains(6));
    NS_TEST_BOOL(m.Contains(7));
    NS_TEST_BOOL(m.Contains(8));
    NS_TEST_BOOL(m.Contains(9));

    NS_TEST_BOOL(!m.Contains(0));
    NS_TEST_BOOL(!m.Contains(10));

    NS_TEST_INT(m.GetCount(), 9);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Set Operations")
  {
    nsSet<nsUInt32> base;
    base.Insert(1);
    base.Insert(3);
    base.Insert(5);

    nsSet<nsUInt32> empty;

    nsSet<nsUInt32> disjunct;
    disjunct.Insert(2);
    disjunct.Insert(4);
    disjunct.Insert(6);

    nsSet<nsUInt32> subSet;
    subSet.Insert(1);
    subSet.Insert(5);

    nsSet<nsUInt32> superSet;
    superSet.Insert(1);
    superSet.Insert(3);
    superSet.Insert(5);
    superSet.Insert(7);

    nsSet<nsUInt32> nonDisjunctNonEmptySubSet;
    nonDisjunctNonEmptySubSet.Insert(1);
    nonDisjunctNonEmptySubSet.Insert(4);
    nonDisjunctNonEmptySubSet.Insert(5);

    // ContainsSet
    NS_TEST_BOOL(base.ContainsSet(base));

    NS_TEST_BOOL(base.ContainsSet(empty));
    NS_TEST_BOOL(!empty.ContainsSet(base));

    NS_TEST_BOOL(!base.ContainsSet(disjunct));
    NS_TEST_BOOL(!disjunct.ContainsSet(base));

    NS_TEST_BOOL(base.ContainsSet(subSet));
    NS_TEST_BOOL(!subSet.ContainsSet(base));

    NS_TEST_BOOL(!base.ContainsSet(superSet));
    NS_TEST_BOOL(superSet.ContainsSet(base));

    NS_TEST_BOOL(!base.ContainsSet(nonDisjunctNonEmptySubSet));
    NS_TEST_BOOL(!nonDisjunctNonEmptySubSet.ContainsSet(base));

    // Union
    {
      nsSet<nsUInt32> res;

      res.Union(base);
      NS_TEST_BOOL(res.ContainsSet(base));
      NS_TEST_BOOL(base.ContainsSet(res));
      res.Union(subSet);
      NS_TEST_BOOL(res.ContainsSet(base));
      NS_TEST_BOOL(res.ContainsSet(subSet));
      NS_TEST_BOOL(base.ContainsSet(res));
      res.Union(superSet);
      NS_TEST_BOOL(res.ContainsSet(base));
      NS_TEST_BOOL(res.ContainsSet(subSet));
      NS_TEST_BOOL(res.ContainsSet(superSet));
      NS_TEST_BOOL(superSet.ContainsSet(res));
    }

    // Difference
    {
      nsSet<nsUInt32> res;
      res.Union(base);
      res.Difference(empty);
      NS_TEST_BOOL(res.ContainsSet(base));
      NS_TEST_BOOL(base.ContainsSet(res));
      res.Difference(disjunct);
      NS_TEST_BOOL(res.ContainsSet(base));
      NS_TEST_BOOL(base.ContainsSet(res));
      res.Difference(subSet);
      NS_TEST_INT(res.GetCount(), 1);
      NS_TEST_BOOL(res.Contains(3));
    }

    // Intersection
    {
      nsSet<nsUInt32> res;
      res.Union(base);
      res.Intersection(disjunct);
      NS_TEST_BOOL(res.IsEmpty());
      res.Union(base);
      res.Intersection(subSet);
      NS_TEST_BOOL(base.ContainsSet(subSet));
      NS_TEST_BOOL(res.ContainsSet(subSet));
      NS_TEST_BOOL(subSet.ContainsSet(res));
      res.Intersection(superSet);
      NS_TEST_BOOL(superSet.ContainsSet(res));
      NS_TEST_BOOL(res.ContainsSet(subSet));
      NS_TEST_BOOL(subSet.ContainsSet(res));
      res.Intersection(empty);
      NS_TEST_BOOL(res.IsEmpty());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Find")
  {
    nsSet<nsUInt32> m;

    for (nsInt32 i = 0; i < 1000; ++i)
      m.Insert(i);

    for (nsInt32 i = 1000 - 1; i >= 0; --i)
      NS_TEST_INT(m.Find(i).Key(), i);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Remove (non-existing)")
  {
    nsSet<nsUInt32> m;

    for (nsInt32 i = 0; i < 1000; ++i)
      NS_TEST_BOOL(!m.Remove(i));

    for (nsInt32 i = 0; i < 1000; ++i)
      m.Insert(i);

    for (nsInt32 i = 0; i < 1000; ++i)
      NS_TEST_BOOL(m.Remove(i + 500) == (i < 500));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Remove (Iterator)")
  {
    nsSet<nsUInt32> m;

    for (nsInt32 i = 0; i < 1000; ++i)
      m.Insert(i);

    for (nsInt32 i = 0; i < 1000 - 1; ++i)
    {
      nsSet<nsUInt32>::Iterator itNext = m.Remove(m.Find(i));
      NS_TEST_BOOL(!m.Find(i).IsValid());
      NS_TEST_BOOL(itNext.Key() == i + 1);

      NS_TEST_INT(m.GetCount(), 1000 - 1 - i);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Remove (Key)")
  {
    nsSet<nsUInt32> m;

    for (nsInt32 i = 0; i < 1000; ++i)
      m.Insert(i);

    for (nsInt32 i = 0; i < 1000; ++i)
    {
      NS_TEST_BOOL(m.Remove(i));
      NS_TEST_BOOL(!m.Find(i).IsValid());

      NS_TEST_INT(m.GetCount(), 1000 - 1 - i);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator=")
  {
    nsSet<nsUInt32> m, m2;

    for (nsInt32 i = 0; i < 1000; ++i)
      m.Insert(i);

    m2 = m;

    for (nsInt32 i = 1000 - 1; i >= 0; --i)
      NS_TEST_BOOL(m2.Find(i).IsValid());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Copy Constructor")
  {
    nsSet<nsUInt32> m;

    for (nsInt32 i = 0; i < 1000; ++i)
      m.Insert(i);

    nsSet<nsUInt32> m2(m);

    for (nsInt32 i = 1000 - 1; i >= 0; --i)
      NS_TEST_BOOL(m2.Find(i).IsValid());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetIterator / Forward Iteration")
  {
    nsSet<nsUInt32> m;

    for (nsInt32 i = 0; i < 1000; ++i)
      m.Insert(i);

    nsInt32 i = 0;
    for (nsSet<nsUInt32>::Iterator it = m.GetIterator(); it.IsValid(); ++it)
    {
      NS_TEST_INT(it.Key(), i);
      ++i;
    }

    NS_TEST_INT(i, 1000);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetIterator / Forward Iteration (const)")
  {
    nsSet<nsUInt32> m;

    for (nsInt32 i = 0; i < 1000; ++i)
      m.Insert(i);

    const nsSet<nsUInt32> m2(m);

    nsInt32 i = 0;
    for (nsSet<nsUInt32>::Iterator it = m2.GetIterator(); it.IsValid(); ++it)
    {
      NS_TEST_INT(it.Key(), i);
      ++i;
    }

    NS_TEST_INT(i, 1000);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "LowerBound")
  {
    nsSet<nsInt32> m, m2;

    m.Insert(0);
    m.Insert(3);
    m.Insert(7);
    m.Insert(9);

    NS_TEST_INT(m.LowerBound(-1).Key(), 0);
    NS_TEST_INT(m.LowerBound(0).Key(), 0);
    NS_TEST_INT(m.LowerBound(1).Key(), 3);
    NS_TEST_INT(m.LowerBound(2).Key(), 3);
    NS_TEST_INT(m.LowerBound(3).Key(), 3);
    NS_TEST_INT(m.LowerBound(4).Key(), 7);
    NS_TEST_INT(m.LowerBound(5).Key(), 7);
    NS_TEST_INT(m.LowerBound(6).Key(), 7);
    NS_TEST_INT(m.LowerBound(7).Key(), 7);
    NS_TEST_INT(m.LowerBound(8).Key(), 9);
    NS_TEST_INT(m.LowerBound(9).Key(), 9);

    NS_TEST_BOOL(!m.LowerBound(10).IsValid());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "UpperBound")
  {
    nsSet<nsInt32> m, m2;

    m.Insert(0);
    m.Insert(3);
    m.Insert(7);
    m.Insert(9);

    NS_TEST_INT(m.UpperBound(-1).Key(), 0);
    NS_TEST_INT(m.UpperBound(0).Key(), 3);
    NS_TEST_INT(m.UpperBound(1).Key(), 3);
    NS_TEST_INT(m.UpperBound(2).Key(), 3);
    NS_TEST_INT(m.UpperBound(3).Key(), 7);
    NS_TEST_INT(m.UpperBound(4).Key(), 7);
    NS_TEST_INT(m.UpperBound(5).Key(), 7);
    NS_TEST_INT(m.UpperBound(6).Key(), 7);
    NS_TEST_INT(m.UpperBound(7).Key(), 9);
    NS_TEST_INT(m.UpperBound(8).Key(), 9);
    NS_TEST_BOOL(!m.UpperBound(9).IsValid());
    NS_TEST_BOOL(!m.UpperBound(10).IsValid());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Insert / Remove")
  {
    // Tests whether reusing of elements makes problems

    nsSet<nsInt32> m;

    for (nsUInt32 r = 0; r < 5; ++r)
    {
      // Insert
      for (nsUInt32 i = 0; i < 10000; ++i)
        m.Insert(i);

      NS_TEST_INT(m.GetCount(), 10000);

      // Remove
      for (nsUInt32 i = 0; i < 5000; ++i)
        NS_TEST_BOOL(m.Remove(i));

      // Insert others
      for (nsUInt32 j = 1; j < 1000; ++j)
        m.Insert(20000 * j);

      // Remove
      for (nsUInt32 i = 0; i < 5000; ++i)
        NS_TEST_BOOL(m.Remove(5000 + i));

      // Remove others
      for (nsUInt32 j = 1; j < 1000; ++j)
      {
        NS_TEST_BOOL(m.Find(20000 * j).IsValid());
        NS_TEST_BOOL(m.Remove(20000 * j));
      }
    }

    NS_TEST_BOOL(m.IsEmpty());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Iterator")
  {
    nsSet<nsUInt32> m;
    for (nsUInt32 i = 0; i < 1000; ++i)
      m.Insert(i + 1);

    NS_TEST_INT(std::find(begin(m), end(m), 500).Key(), 500);

    auto itfound = std::find_if(begin(m), end(m), [](nsUInt32 uiVal)
      { return uiVal == 500; });

    NS_TEST_BOOL(std::find(begin(m), end(m), 500) == itfound);

    nsUInt32 prev = *begin(m);
    for (nsUInt32 val : m)
    {
      NS_TEST_BOOL(val >= prev);
      prev = val;
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator == / !=")
  {
    nsSet<nsUInt32> m, m2;

    NS_TEST_BOOL(m == m2);

    for (nsInt32 i = 0; i < 1000; ++i)
      m.Insert(i * 10);

    NS_TEST_BOOL(m != m2);

    m2 = m;

    NS_TEST_BOOL(m == m2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CompatibleKeyType")
  {
    {
      nsSet<nsString> stringSet;
      const char* szChar = "Char";
      const char* szString = "ViewBla";
      nsStringView sView(szString, szString + 4);
      nsStringBuilder sBuilder("Builder");
      nsString sString("String");
      stringSet.Insert(szChar);
      stringSet.Insert(sView);
      stringSet.Insert(sBuilder);
      stringSet.Insert(sString);

      NS_TEST_BOOL(stringSet.Contains(szChar));
      NS_TEST_BOOL(stringSet.Contains(sView));
      NS_TEST_BOOL(stringSet.Contains(sBuilder));
      NS_TEST_BOOL(stringSet.Contains(sString));

      NS_TEST_BOOL(stringSet.Remove(szChar));
      NS_TEST_BOOL(stringSet.Remove(sView));
      NS_TEST_BOOL(stringSet.Remove(sBuilder));
      NS_TEST_BOOL(stringSet.Remove(sString));
    }

    // dynamic array as key, check for allocations in comparisons
    {
      nsProxyAllocator testAllocator("Test", nsFoundation::GetDefaultAllocator());
      nsLocalAllocatorWrapper allocWrapper(&testAllocator);
      using TestDynArray = nsDynamicArray<int, nsLocalAllocatorWrapper>;
      TestDynArray a;
      TestDynArray b;
      for (int i = 0; i < 10; ++i)
      {
        a.PushBack(i);
        b.PushBack(i * 2);
      }

      nsSet<TestDynArray> arraySet;
      arraySet.Insert(a);
      arraySet.Insert(b);

      nsArrayPtr<const int> aPtr = a.GetArrayPtr();
      nsArrayPtr<const int> bPtr = b.GetArrayPtr();

      nsUInt64 oldAllocCount = testAllocator.GetStats().m_uiNumAllocations;

      NS_TEST_BOOL(arraySet.Contains(aPtr));
      NS_TEST_BOOL(arraySet.Contains(bPtr));
      NS_TEST_BOOL(arraySet.Contains(a));

      NS_TEST_INT(testAllocator.GetStats().m_uiNumAllocations, oldAllocCount);

      NS_TEST_BOOL(arraySet.Remove(aPtr));
      NS_TEST_BOOL(arraySet.Remove(bPtr));

      NS_TEST_INT(testAllocator.GetStats().m_uiNumAllocations, oldAllocCount);
    }
  }

  constexpr nsUInt32 uiSetSize = sizeof(nsSet<nsString>);

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Swap")
  {
    nsUInt8 set1Mem[uiSetSize];
    nsUInt8 set2Mem[uiSetSize];
    nsMemoryUtils::PatternFill(set1Mem, 0xCA, uiSetSize);
    nsMemoryUtils::PatternFill(set2Mem, 0xCA, uiSetSize);

    nsStringBuilder tmp;
    nsSet<nsString>* set1 = new (set1Mem)(nsSet<nsString>);
    nsSet<nsString>* set2 = new (set2Mem)(nsSet<nsString>);

    for (nsUInt32 i = 0; i < 1000; ++i)
    {
      tmp.SetFormat("stuff{}bla", i);
      set1->Insert(tmp);

      tmp.SetFormat("{0}{0}{0}", i);
      set2->Insert(tmp);
    }

    set1->Swap(*set2);

    // test swapped elements
    for (nsUInt32 i = 0; i < 1000; ++i)
    {
      tmp.SetFormat("stuff{}bla", i);
      NS_TEST_BOOL(set2->Contains(tmp));

      tmp.SetFormat("{0}{0}{0}", i);
      NS_TEST_BOOL(set1->Contains(tmp));
    }

    // test iterators after swap
    {
      for (const auto& element : *set1)
      {
        NS_TEST_BOOL(!set2->Contains(element));
      }

      for (const auto& element : *set2)
      {
        NS_TEST_BOOL(!set1->Contains(element));
      }
    }

    // due to a compiler bug in VS 2017, PatternFill cannot be called here, because it will move the memset BEFORE the destructor call!
    // seems to be fixed in VS 2019 though

    set1->~nsSet<nsString>();
    // nsMemoryUtils::PatternFill(set1Mem, 0xBA, uiSetSize);

    set2->~nsSet<nsString>();
    nsMemoryUtils::PatternFill(set2Mem, 0xBA, uiSetSize);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Swap Empty")
  {
    nsUInt8 set1Mem[uiSetSize];
    nsUInt8 set2Mem[uiSetSize];
    nsMemoryUtils::PatternFill(set1Mem, 0xCA, uiSetSize);
    nsMemoryUtils::PatternFill(set2Mem, 0xCA, uiSetSize);

    nsStringBuilder tmp;
    nsSet<nsString>* set1 = new (set1Mem)(nsSet<nsString>);
    nsSet<nsString>* set2 = new (set2Mem)(nsSet<nsString>);

    for (nsUInt32 i = 0; i < 100; ++i)
    {
      tmp.SetFormat("stuff{}bla", i);
      set1->Insert(tmp);
    }

    set1->Swap(*set2);
    NS_TEST_BOOL(set1->IsEmpty());

    set1->~nsSet<nsString>();
    nsMemoryUtils::PatternFill(set1Mem, 0xBA, uiSetSize);

    // test swapped elements
    for (nsUInt32 i = 0; i < 100; ++i)
    {
      tmp.SetFormat("stuff{}bla", i);
      NS_TEST_BOOL(set2->Contains(tmp));
    }

    // test iterators after swap
    {
      for (const auto& element : *set2)
      {
        NS_TEST_BOOL(set2->Contains(element));
      }
    }

    set2->~nsSet<nsString>();
    nsMemoryUtils::PatternFill(set2Mem, 0xBA, uiSetSize);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetReverseIterator")
  {
    nsSet<nsUInt32> m;

    for (nsInt32 i = 0; i < 1000; ++i)
      m.Insert(i);

    nsInt32 i = 1000 - 1;
    for (nsSet<nsUInt32>::ReverseIterator it = m.GetReverseIterator(); it.IsValid(); ++it)
    {
      NS_TEST_INT(it.Key(), i);
      --i;
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetReverseIterator (const)")
  {
    nsSet<nsUInt32> m;

    for (nsInt32 i = 0; i < 1000; ++i)
      m.Insert(i);

    const nsSet<nsUInt32> m2(m);

    nsInt32 i = 1000 - 1;
    for (nsSet<nsUInt32>::ReverseIterator it = m2.GetReverseIterator(); it.IsValid(); ++it)
    {
      NS_TEST_INT(it.Key(), i);
      --i;
    }
  }
}

#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Containers/Map.h>
#include <Foundation/Memory/CommonAllocators.h>
#include <Foundation/Strings/String.h>
#include <algorithm>
#include <iterator>

NS_CREATE_SIMPLE_TEST(Containers, Map)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Iterator")
  {
    nsMap<nsUInt32, nsUInt32> m;
    for (nsUInt32 i = 0; i < 1000; ++i)
      m[i] = i + 1;

    // NS_TEST_INT(std::find(begin(m), end(m), 500).Key(), 499);

    auto itfound = std::find_if(begin(m), end(m), [](nsMap<nsUInt32, nsUInt32>::ConstIterator val)
      { return val.Value() == 500; });

    // NS_TEST_BOOL(std::find(begin(m), end(m), 500) == itfound);

    nsUInt32 prev = begin(m).Key();
    for (auto it : m)
    {
      NS_TEST_BOOL(it.Value() >= prev);
      prev = it.Value();
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
    nsMap<nsUInt32, nsUInt32> m;
    nsMap<nsConstructionCounter, nsUInt32> m2;
    nsMap<nsConstructionCounter, nsConstructionCounter> m3;
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "IsEmpty")
  {
    nsMap<nsUInt32, nsUInt32> m;
    NS_TEST_BOOL(m.IsEmpty());

    m[1] = 2;
    NS_TEST_BOOL(!m.IsEmpty());

    m.Clear();
    NS_TEST_BOOL(m.IsEmpty());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetCount")
  {
    nsMap<nsUInt32, nsUInt32> m;
    NS_TEST_INT(m.GetCount(), 0);

    m[0] = 1;
    NS_TEST_INT(m.GetCount(), 1);

    m[1] = 2;
    NS_TEST_INT(m.GetCount(), 2);

    m[2] = 3;
    NS_TEST_INT(m.GetCount(), 3);

    m[0] = 1;
    NS_TEST_INT(m.GetCount(), 3);

    m.Clear();
    NS_TEST_INT(m.GetCount(), 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Clear")
  {
    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());

    {
      nsMap<nsUInt32, nsConstructionCounter> m1;
      m1[0] = nsConstructionCounter(1);
      NS_TEST_BOOL(nsConstructionCounter::HasDone(3, 2)); // for inserting new elements 2 temporaries are created (and destroyed)

      m1[1] = nsConstructionCounter(3);
      NS_TEST_BOOL(nsConstructionCounter::HasDone(3, 2)); // for inserting new elements 2 temporaries are created (and destroyed)

      m1[0] = nsConstructionCounter(2);
      NS_TEST_BOOL(nsConstructionCounter::HasDone(1, 1)); // nothing new to create, so only the one temporary is used

      m1.Clear();
      NS_TEST_BOOL(nsConstructionCounter::HasDone(0, 2));
      NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());
    }

    {
      nsMap<nsConstructionCounter, nsUInt32> m1;
      m1[nsConstructionCounter(0)] = 1;
      NS_TEST_BOOL(nsConstructionCounter::HasDone(2, 1)); // one temporary

      m1[nsConstructionCounter(1)] = 3;
      NS_TEST_BOOL(nsConstructionCounter::HasDone(2, 1)); // one temporary

      m1[nsConstructionCounter(0)] = 2;
      NS_TEST_BOOL(nsConstructionCounter::HasDone(1, 1)); // nothing new to create, so only the one temporary is used

      m1.Clear();
      NS_TEST_BOOL(nsConstructionCounter::HasDone(0, 2));
      NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Insert")
  {
    nsMap<nsUInt32, nsUInt32> m;

    NS_TEST_BOOL(m.GetHeapMemoryUsage() == 0);

    NS_TEST_BOOL(m.Insert(1, 10).IsValid());
    NS_TEST_BOOL(m.Insert(1, 10).IsValid());
    m.Insert(3, 30);
    auto it7 = m.Insert(7, 70);
    m.Insert(9, 90);
    m.Insert(4, 40);
    m.Insert(2, 20);
    m.Insert(8, 80);
    m.Insert(5, 50);
    m.Insert(6, 60);

    NS_TEST_BOOL(m.Insert(7, 70).Value() == 70);
    NS_TEST_BOOL(m.Insert(7, 70) == it7);

    NS_TEST_BOOL(m.GetHeapMemoryUsage() >= sizeof(nsUInt32) * 2 * 9);

    NS_TEST_INT(m[1], 10);
    NS_TEST_INT(m[2], 20);
    NS_TEST_INT(m[3], 30);
    NS_TEST_INT(m[4], 40);
    NS_TEST_INT(m[5], 50);
    NS_TEST_INT(m[6], 60);
    NS_TEST_INT(m[7], 70);
    NS_TEST_INT(m[8], 80);
    NS_TEST_INT(m[9], 90);

    NS_TEST_INT(m.GetCount(), 9);

    for (nsUInt32 i = 0; i < 1000000; ++i)
      m[i] = i;

    NS_TEST_BOOL(m.GetHeapMemoryUsage() >= sizeof(nsUInt32) * 2 * 1000000);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Find")
  {
    nsMap<nsUInt32, nsUInt32> m;

    for (nsInt32 i = 0; i < 1000; ++i)
      m[i] = i * 10;

    for (nsInt32 i = 1000 - 1; i >= 0; --i)
      NS_TEST_INT(m.Find(i).Value(), i * 10);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetValue/TryGetValue")
  {
    nsMap<nsUInt32, nsUInt32> m;

    for (nsInt32 i = 0; i < 100; ++i)
      m[i] = i * 10;

    for (nsInt32 i = 100 - 1; i >= 0; --i)
    {
      NS_TEST_INT(*m.GetValue(i), i * 10);

      nsUInt32 v = 0;
      NS_TEST_BOOL(m.TryGetValue(i, v));
      NS_TEST_INT(v, i * 10);

      nsUInt32* pV = nullptr;
      NS_TEST_BOOL(m.TryGetValue(i, pV));
      NS_TEST_INT(*pV, i * 10);
    }

    NS_TEST_BOOL(m.GetValue(101) == nullptr);

    nsUInt32 v = 0;
    NS_TEST_BOOL(m.TryGetValue(101, v) == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetValue/TryGetValue (const)")
  {
    nsMap<nsUInt32, nsUInt32> m;

    for (nsInt32 i = 0; i < 100; ++i)
      m[i] = i * 10;

    const nsMap<nsUInt32, nsUInt32>& mConst = m;

    for (nsInt32 i = 100 - 1; i >= 0; --i)
    {
      NS_TEST_INT(*mConst.GetValue(i), i * 10);

      nsUInt32 v = 0;
      NS_TEST_BOOL(m.TryGetValue(i, v));
      NS_TEST_INT(v, i * 10);

      nsUInt32* pV = nullptr;
      NS_TEST_BOOL(m.TryGetValue(i, pV));
      NS_TEST_INT(*pV, i * 10);
    }

    NS_TEST_BOOL(mConst.GetValue(101) == nullptr);

    nsUInt32 v = 0;
    NS_TEST_BOOL(mConst.TryGetValue(101, v) == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetValueOrDefault")
  {
    nsMap<nsUInt32, nsUInt32> m;

    for (nsInt32 i = 0; i < 100; ++i)
      m[i] = i * 10;

    for (nsInt32 i = 100 - 1; i >= 0; --i)
      NS_TEST_INT(m.GetValueOrDefault(i, 999), i * 10);

    NS_TEST_BOOL(m.GetValueOrDefault(101, 999) == 999);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Contains")
  {
    nsMap<nsUInt32, nsUInt32> m;

    for (nsInt32 i = 0; i < 1000; i += 2)
      m[i] = i * 10;

    for (nsInt32 i = 0; i < 1000; i += 2)
    {
      NS_TEST_BOOL(m.Contains(i));
      NS_TEST_BOOL(!m.Contains(i + 1));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "FindOrAdd")
  {
    nsMap<nsUInt32, nsUInt32> m;

    for (nsInt32 i = 0; i < 1000; ++i)
    {
      bool bExisted = true;
      m.FindOrAdd(i, &bExisted).Value() = i * 10;
      NS_TEST_BOOL(!bExisted);
    }

    for (nsInt32 i = 1000 - 1; i >= 0; --i)
    {
      bool bExisted = false;
      NS_TEST_INT(m.FindOrAdd(i, &bExisted).Value(), i * 10);
      NS_TEST_BOOL(bExisted);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator[]")
  {
    nsMap<nsUInt32, nsUInt32> m;

    for (nsInt32 i = 0; i < 1000; ++i)
      m[i] = i * 10;

    for (nsInt32 i = 1000 - 1; i >= 0; --i)
      NS_TEST_INT(m[i], i * 10);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Remove (non-existing)")
  {
    nsMap<nsUInt32, nsUInt32> m;

    for (nsInt32 i = 0; i < 1000; ++i)
    {
      NS_TEST_BOOL(!m.Remove(i));
    }

    for (nsInt32 i = 0; i < 1000; ++i)
      m[i] = i * 10;

    for (nsInt32 i = 0; i < 1000; ++i)
    {
      NS_TEST_BOOL(m.Remove(i + 500) == (i < 500));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Remove (Iterator)")
  {
    nsMap<nsUInt32, nsUInt32> m;

    for (nsInt32 i = 0; i < 1000; ++i)
      m[i] = i * 10;

    for (nsInt32 i = 0; i < 1000 - 1; ++i)
    {
      nsMap<nsUInt32, nsUInt32>::Iterator itNext = m.Remove(m.Find(i));
      NS_TEST_BOOL(!m.Find(i).IsValid());
      NS_TEST_BOOL(itNext.Key() == i + 1);

      NS_TEST_INT(m.GetCount(), 1000 - 1 - i);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Remove (Key)")
  {
    nsMap<nsUInt32, nsUInt32> m;

    for (nsInt32 i = 0; i < 1000; ++i)
      m[i] = i * 10;

    for (nsInt32 i = 0; i < 1000; ++i)
    {
      NS_TEST_BOOL(m.Remove(i));
      NS_TEST_BOOL(!m.Find(i).IsValid());

      NS_TEST_INT(m.GetCount(), 1000 - 1 - i);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator=")
  {
    nsMap<nsUInt32, nsUInt32> m, m2;

    for (nsInt32 i = 0; i < 1000; ++i)
      m[i] = i * 10;

    m2 = m;

    for (nsInt32 i = 1000 - 1; i >= 0; --i)
      NS_TEST_INT(m2[i], i * 10);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Copy Constructor")
  {
    nsMap<nsUInt32, nsUInt32> m;

    for (nsInt32 i = 0; i < 1000; ++i)
      m[i] = i * 10;

    nsMap<nsUInt32, nsUInt32> m2(m);

    for (nsInt32 i = 1000 - 1; i >= 0; --i)
      NS_TEST_INT(m2[i], i * 10);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetIterator / Forward Iteration")
  {
    nsMap<nsUInt32, nsUInt32> m;

    for (nsInt32 i = 0; i < 1000; ++i)
      m[i] = i * 10;

    nsInt32 i = 0;
    for (nsMap<nsUInt32, nsUInt32>::Iterator it = m.GetIterator(); it.IsValid(); ++it)
    {
      NS_TEST_INT(it.Key(), i);
      NS_TEST_INT(it.Value(), i * 10);
      ++i;
    }

    NS_TEST_INT(i, 1000);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetIterator / Forward Iteration (const)")
  {
    nsMap<nsUInt32, nsUInt32> m;

    for (nsInt32 i = 0; i < 1000; ++i)
      m[i] = i * 10;

    const nsMap<nsUInt32, nsUInt32> m2(m);

    nsInt32 i = 0;
    for (nsMap<nsUInt32, nsUInt32>::ConstIterator it = m2.GetIterator(); it.IsValid(); ++it)
    {
      NS_TEST_INT(it.Key(), i);
      NS_TEST_INT(it.Value(), i * 10);
      ++i;
    }

    NS_TEST_INT(i, 1000);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "LowerBound")
  {
    nsMap<nsInt32, nsInt32> m, m2;

    m[0] = 0;
    m[3] = 30;
    m[7] = 70;
    m[9] = 90;

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
    nsMap<nsInt32, nsInt32> m, m2;

    m[0] = 0;
    m[3] = 30;
    m[7] = 70;
    m[9] = 90;

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

    nsMap<nsInt32, nsInt32> m;

    for (nsUInt32 r = 0; r < 5; ++r)
    {
      // Insert
      for (nsUInt32 i = 0; i < 10000; ++i)
        m.Insert(i, i * 10);

      NS_TEST_INT(m.GetCount(), 10000);

      // Remove
      for (nsUInt32 i = 0; i < 5000; ++i)
        NS_TEST_BOOL(m.Remove(i));

      // Insert others
      for (nsUInt32 j = 1; j < 1000; ++j)
        m.Insert(20000 * j, j);

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

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator == / !=")
  {
    nsMap<nsUInt32, nsUInt32> m, m2;

    NS_TEST_BOOL(m == m2);

    for (nsInt32 i = 0; i < 1000; ++i)
      m[i] = i * 10;

    NS_TEST_BOOL(m != m2);

    m2 = m;

    NS_TEST_BOOL(m == m2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CompatibleKeyType")
  {
    {
      nsMap<nsString, int> stringTable;
      const char* szChar = "Char";
      const char* szString = "ViewBla";
      nsStringView sView(szString, szString + 4);
      nsStringBuilder sBuilder("Builder");
      nsString sString("String");
      stringTable.Insert(szChar, 1);
      stringTable.Insert(sView, 2);
      stringTable.Insert(sBuilder, 3);
      stringTable.Insert(sString, 4);

      NS_TEST_BOOL(stringTable.Contains(szChar));
      NS_TEST_BOOL(stringTable.Contains(sView));
      NS_TEST_BOOL(stringTable.Contains(sBuilder));
      NS_TEST_BOOL(stringTable.Contains(sString));

      NS_TEST_INT(*stringTable.GetValue(szChar), 1);
      NS_TEST_INT(*stringTable.GetValue(sView), 2);
      NS_TEST_INT(*stringTable.GetValue(sBuilder), 3);
      NS_TEST_INT(*stringTable.GetValue(sString), 4);

      NS_TEST_BOOL(stringTable.Remove(szChar));
      NS_TEST_BOOL(stringTable.Remove(sView));
      NS_TEST_BOOL(stringTable.Remove(sBuilder));
      NS_TEST_BOOL(stringTable.Remove(sString));
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

      nsMap<TestDynArray, int> arrayTable;
      arrayTable.Insert(a, 1);
      arrayTable.Insert(b, 2);

      nsArrayPtr<const int> aPtr = a.GetArrayPtr();
      nsArrayPtr<const int> bPtr = b.GetArrayPtr();

      nsUInt64 oldAllocCount = testAllocator.GetStats().m_uiNumAllocations;

      bool existed;
      auto it = arrayTable.FindOrAdd(aPtr, &existed);
      NS_TEST_BOOL(existed);

      NS_TEST_INT(testAllocator.GetStats().m_uiNumAllocations, oldAllocCount);

      NS_TEST_BOOL(arrayTable.Contains(aPtr));
      NS_TEST_BOOL(arrayTable.Contains(bPtr));
      NS_TEST_BOOL(arrayTable.Contains(a));

      NS_TEST_INT(testAllocator.GetStats().m_uiNumAllocations, oldAllocCount);

      NS_TEST_INT(*arrayTable.GetValue(aPtr), 1);
      NS_TEST_INT(*arrayTable.GetValue(bPtr), 2);
      NS_TEST_INT(*arrayTable.GetValue(a), 1);

      NS_TEST_INT(testAllocator.GetStats().m_uiNumAllocations, oldAllocCount);

      NS_TEST_BOOL(arrayTable.Remove(aPtr));
      NS_TEST_BOOL(arrayTable.Remove(bPtr));

      NS_TEST_INT(testAllocator.GetStats().m_uiNumAllocations, oldAllocCount);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Swap")
  {
    nsStringBuilder tmp;
    nsMap<nsString, nsInt32> map1;
    nsMap<nsString, nsInt32> map2;

    for (nsUInt32 i = 0; i < 1000; ++i)
    {
      tmp.SetFormat("stuff{}bla", i);
      map1[tmp] = i;

      tmp.SetFormat("{0}{0}{0}", i);
      map2[tmp] = i;
    }

    map1.Swap(map2);

    for (nsUInt32 i = 0; i < 1000; ++i)
    {
      tmp.SetFormat("stuff{}bla", i);
      NS_TEST_BOOL(map2.Contains(tmp));
      NS_TEST_INT(map2[tmp], i);

      tmp.SetFormat("{0}{0}{0}", i);
      NS_TEST_BOOL(map1.Contains(tmp));
      NS_TEST_INT(map1[tmp], i);
    }
  }

  constexpr nsUInt32 uiMapSize = sizeof(nsMap<nsString, nsInt32>);

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Swap")
  {
    nsUInt8 map1Mem[uiMapSize];
    nsUInt8 map2Mem[uiMapSize];
    nsMemoryUtils::PatternFill(map1Mem, 0xCA, uiMapSize);
    nsMemoryUtils::PatternFill(map2Mem, 0xCA, uiMapSize);

    nsStringBuilder tmp;
    nsMap<nsString, nsInt32>* map1 = new (map1Mem)(nsMap<nsString, nsInt32>);
    nsMap<nsString, nsInt32>* map2 = new (map2Mem)(nsMap<nsString, nsInt32>);

    for (nsUInt32 i = 0; i < 1000; ++i)
    {
      tmp.SetFormat("stuff{}bla", i);
      map1->Insert(tmp, i);

      tmp.SetFormat("{0}{0}{0}", i);
      map2->Insert(tmp, i);
    }

    map1->Swap(*map2);

    // test swapped elements
    for (nsUInt32 i = 0; i < 1000; ++i)
    {
      tmp.SetFormat("stuff{}bla", i);
      NS_TEST_BOOL(map2->Contains(tmp));
      NS_TEST_INT((*map2)[tmp], i);

      tmp.SetFormat("{0}{0}{0}", i);
      NS_TEST_BOOL(map1->Contains(tmp));
      NS_TEST_INT((*map1)[tmp], i);
    }

    // test iterators after swap
    {
      for (auto it : *map1)
      {
        NS_TEST_BOOL(!map2->Contains(it.Key()));
      }

      for (auto it : *map2)
      {
        NS_TEST_BOOL(!map1->Contains(it.Key()));
      }
    }

    // due to a compiler bug in VS 2017, PatternFill cannot be called here, because it will move the memset BEFORE the destructor call!
    // seems to be fixed in VS 2019 though

    map1->~nsMap<nsString, nsInt32>();
    // nsMemoryUtils::PatternFill(map1Mem, 0xBA, uiSetSize);

    map2->~nsMap<nsString, nsInt32>();
    nsMemoryUtils::PatternFill(map2Mem, 0xBA, uiMapSize);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Swap Empty")
  {
    nsUInt8 map1Mem[uiMapSize];
    nsUInt8 map2Mem[uiMapSize];
    nsMemoryUtils::PatternFill(map1Mem, 0xCA, uiMapSize);
    nsMemoryUtils::PatternFill(map2Mem, 0xCA, uiMapSize);

    nsStringBuilder tmp;
    nsMap<nsString, nsInt32>* map1 = new (map1Mem)(nsMap<nsString, nsInt32>);
    nsMap<nsString, nsInt32>* map2 = new (map2Mem)(nsMap<nsString, nsInt32>);

    for (nsUInt32 i = 0; i < 100; ++i)
    {
      tmp.SetFormat("stuff{}bla", i);
      map1->Insert(tmp, i);
    }

    map1->Swap(*map2);
    NS_TEST_BOOL(map1->IsEmpty());

    map1->~nsMap<nsString, nsInt32>();
    nsMemoryUtils::PatternFill(map1Mem, 0xBA, uiMapSize);

    // test swapped elements
    for (nsUInt32 i = 0; i < 100; ++i)
    {
      tmp.SetFormat("stuff{}bla", i);
      NS_TEST_BOOL(map2->Contains(tmp));
    }

    // test iterators after swap
    {
      for (auto it : *map2)
      {
        NS_TEST_BOOL(map2->Contains(it.Key()));
      }
    }

    map2->~nsMap<nsString, nsInt32>();
    nsMemoryUtils::PatternFill(map2Mem, 0xBA, uiMapSize);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetReverseIterator")
  {
    nsMap<nsUInt32, nsUInt32> m;

    for (nsInt32 i = 0; i < 1000; ++i)
      m[i] = i * 10;

    nsInt32 i = 1000 - 1;
    for (nsMap<nsUInt32, nsUInt32>::ReverseIterator it = m.GetReverseIterator(); it.IsValid(); ++it)
    {
      NS_TEST_INT(it.Key(), i);
      NS_TEST_INT(it.Value(), i * 10);
      --i;
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetReverseIterator (const)")
  {
    nsMap<nsUInt32, nsUInt32> m;

    for (nsInt32 i = 0; i < 1000; ++i)
      m[i] = i * 10;

    const nsMap<nsUInt32, nsUInt32> m2(m);

    nsInt32 i = 1000 - 1;
    for (nsMap<nsUInt32, nsUInt32>::ConstReverseIterator it = m2.GetReverseIterator(); it.IsValid(); ++it)
    {
      NS_TEST_INT(it.Key(), i);
      NS_TEST_INT(it.Value(), i * 10);
      --i;
    }
  }
}

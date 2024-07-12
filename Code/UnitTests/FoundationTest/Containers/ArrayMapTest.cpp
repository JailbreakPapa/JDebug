#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Containers/ArrayMap.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Time/Stopwatch.h>

NS_CREATE_SIMPLE_TEST(Containers, ArrayMap)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Iterator")
  {
    nsArrayMap<nsUInt32, nsUInt32> m;
    for (nsUInt32 i = 0; i < 1000; ++i)
      m[i] = i + 1;

    // element with the given key (and such, value "key + 1")
    auto findable = m.Find(499u);

    // non-const
    {
      // findable
      auto itfound = std::find_if(begin(m), end(m), [](const nsArrayMap<nsUInt32, nsUInt32>::Pair& val)
        { return val.value == 500; });
      NS_TEST_BOOL(findable == itfound->key);

      // unfindable
      itfound = std::find_if(begin(m), end(m), [](const nsArrayMap<nsUInt32, nsUInt32>::Pair& val)
        { return val.value == 1001; });
      NS_TEST_BOOL(end(m) == itfound);
    }

    // const
    {
      // findable
      auto itfound = std::find_if(cbegin(m), cend(m), [](const nsArrayMap<nsUInt32, nsUInt32>::Pair& val)
        { return val.value == 500; });
      NS_TEST_BOOL(findable == itfound->key);

      // unfindable
      itfound = std::find_if(cbegin(m), cend(m), [](const nsArrayMap<nsUInt32, nsUInt32>::Pair& val)
        { return val.value == 1001; });
      NS_TEST_BOOL(cend(m) == itfound);
    }

    // non-const reverse
    {
      // findable
      auto itfound = std::find_if(rbegin(m), rend(m), [](const nsArrayMap<nsUInt32, nsUInt32>::Pair& val)
        { return val.value == 500; });
      NS_TEST_BOOL(findable == itfound->key);

      // unfindable
      itfound = std::find_if(rbegin(m), rend(m), [](const nsArrayMap<nsUInt32, nsUInt32>::Pair& val)
        { return val.value == 1001; });
      NS_TEST_BOOL(rend(m) == itfound);
    }

    // const reverse
    {
      // findable
      auto itfound = std::find_if(crbegin(m), crend(m), [](const nsArrayMap<nsUInt32, nsUInt32>::Pair& val)
        { return val.value == 500; });
      NS_TEST_BOOL(findable == itfound->key);

      // unfindable
      itfound = std::find_if(crbegin(m), crend(m), [](const nsArrayMap<nsUInt32, nsUInt32>::Pair& val)
        { return val.value == 1001; });
      NS_TEST_BOOL(crend(m) == itfound);
    }

    // forward
    nsUInt32 prev = begin(m)->key;
    for (const auto& elem : m)
    {
      NS_TEST_BOOL(elem.value == prev + 1);
      prev = elem.value;
    }

    NS_TEST_BOOL(prev == 1000);

    // backward
    prev = (rbegin(m))->value + 1;
    for (auto it = rbegin(m); it < rend(m); ++it)
    {
      NS_TEST_BOOL(it->value == prev - 1);
      prev = it->value;
    }

    NS_TEST_BOOL(prev == 1);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetData with Iterators")
  {
    nsArrayMap<nsUInt32, nsUInt32> m;
    for (nsUInt32 i = 0; i < 1000; ++i)
      m[i] = i + 1;

    // element with the given key (and such, value "key + 1")
    auto findable = m.Find(499u);

    // check if modification of the keys via direct data access
    // keeps iterability and access via keys intact

    // modify
    auto& data = m.GetData();
    for (auto& p : data)
    {
      p.key += 1000;
    }

    // ...and test with new key
    NS_TEST_BOOL(m[findable + 1000] == 500);

    // and index...
    NS_TEST_BOOL(m.GetValue(499u) == 500);

    // and old key.
    NS_TEST_BOOL(m.Find(499u) == nsInvalidIndex);

    // findable
    auto itfound = std::find_if(begin(m), end(m), [](const nsArrayMap<nsUInt32, nsUInt32>::Pair& val)
      { return val.value == 500; });
    NS_TEST_BOOL((findable + 1000) == itfound->key);

    // unfindable
    itfound = std::find_if(begin(m), end(m), [](const nsArrayMap<nsUInt32, nsUInt32>::Pair& val)
      { return val.value == 1001; });
    NS_TEST_BOOL(end(m) == itfound);

    // forward
    nsUInt32 prev = 0;
    for (const auto& elem : m)
    {
      NS_TEST_BOOL(elem.value == prev + 1);
      prev = elem.value;
    }

    NS_TEST_BOOL(prev == 1000);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Insert / Find / Reserve / Clear / IsEmpty / Compact / GetCount")
  {
    nsArrayMap<nsString, nsInt32> sa;

    NS_TEST_BOOL(sa.GetHeapMemoryUsage() == 0);

    NS_TEST_INT(sa.GetCount(), 0);
    NS_TEST_BOOL(sa.IsEmpty());

    sa.Reserve(10);

    NS_TEST_BOOL(sa.GetHeapMemoryUsage() >= 10 * (sizeof(nsString) + sizeof(nsInt32)));

    sa.Insert("z", 0);
    sa.Insert("y", 1);
    sa.Insert("x", 2);
    sa.Insert("c", 3);
    sa.Insert("b", 4);
    sa.Insert("a", 5);

    NS_TEST_INT(sa.GetCount(), 6);
    NS_TEST_BOOL(!sa.IsEmpty());

    NS_TEST_INT(sa.Find("a"), 0);
    NS_TEST_INT(sa.Find("b"), 1);
    NS_TEST_INT(sa.Find("c"), 2);
    NS_TEST_INT(sa.Find("x"), 3);
    NS_TEST_INT(sa.Find("y"), 4);
    NS_TEST_INT(sa.Find("z"), 5);

    NS_TEST_INT(sa.GetPair(sa.Find("a")).value, 5);
    NS_TEST_INT(sa.GetPair(sa.Find("b")).value, 4);
    NS_TEST_INT(sa.GetPair(sa.Find("c")).value, 3);
    NS_TEST_INT(sa.GetPair(sa.Find("x")).value, 2);
    NS_TEST_INT(sa.GetPair(sa.Find("y")).value, 1);
    NS_TEST_INT(sa.GetPair(sa.Find("z")).value, 0);

    sa.Clear();
    NS_TEST_BOOL(sa.IsEmpty());

    NS_TEST_BOOL(sa.GetHeapMemoryUsage() > 0);
    sa.Compact();
    NS_TEST_BOOL(sa.GetHeapMemoryUsage() == 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Insert / Find / = / == / != ")
  {
    nsArrayMap<nsInt32, nsInt32> sa, sa2;

    sa.Insert(20, 0);
    sa.Insert(19, 1);
    sa.Insert(18, 2);
    sa.Insert(12, 3);
    sa.Insert(11, 4);

    sa2 = sa;

    NS_TEST_BOOL(sa == sa2);

    sa.Insert(10, 5);

    NS_TEST_BOOL(sa != sa2);

    NS_TEST_INT(sa.Find(10), 0);
    NS_TEST_INT(sa.Find(11), 1);
    NS_TEST_INT(sa.Find(12), 2);
    NS_TEST_INT(sa.Find(18), 3);
    NS_TEST_INT(sa.Find(19), 4);
    NS_TEST_INT(sa.Find(20), 5);

    sa2.Insert(10, 5);

    NS_TEST_BOOL(sa == sa2);

    NS_TEST_INT(sa.GetValue(sa.Find(10)), 5);
    NS_TEST_INT(sa.GetValue(sa.Find(11)), 4);
    NS_TEST_INT(sa.GetValue(sa.Find(12)), 3);
    NS_TEST_INT(sa.GetValue(sa.Find(18)), 2);
    NS_TEST_INT(sa.GetValue(sa.Find(19)), 1);
    NS_TEST_INT(sa.GetValue(sa.Find(20)), 0);

    NS_TEST_BOOL(sa == sa2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Contains")
  {
    nsArrayMap<nsString, nsInt32> sa;

    NS_TEST_BOOL(!sa.Contains("a"));
    NS_TEST_BOOL(!sa.Contains("z"));

    sa.Insert("z", 0);
    sa.Insert("y", 1);
    sa.Insert("x", 2);

    NS_TEST_BOOL(!sa.Contains("a"));
    NS_TEST_BOOL(sa.Contains("z"));

    sa.Insert("c", 3);
    sa.Insert("b", 4);
    sa.Insert("a", 5);

    NS_TEST_BOOL(sa.Contains("a"));
    NS_TEST_BOOL(sa.Contains("z"));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Contains")
  {
    nsArrayMap<nsString, nsInt32> sa;

    NS_TEST_BOOL(!sa.Contains("a", 0));
    NS_TEST_BOOL(!sa.Contains("z", 0));

    sa.Insert("z", 0);
    sa.Insert("y", 1);
    sa.Insert("x", 2);

    NS_TEST_BOOL(!sa.Contains("a", 0));
    NS_TEST_BOOL(sa.Contains("z", 0));
    NS_TEST_BOOL(sa.Contains("y", 1));
    NS_TEST_BOOL(sa.Contains("x", 2));

    sa.Insert("c", 3);
    sa.Insert("b", 4);
    sa.Insert("a", 5);

    NS_TEST_BOOL(sa.Contains("a", 5));
    NS_TEST_BOOL(sa.Contains("b", 4));
    NS_TEST_BOOL(sa.Contains("c", 3));
    NS_TEST_BOOL(sa.Contains("z", 0));
    NS_TEST_BOOL(sa.Contains("y", 1));
    NS_TEST_BOOL(sa.Contains("x", 2));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetValue / GetKey / Copy Constructor")
  {
    nsArrayMap<nsString, nsInt32> sa;

    sa.Insert("z", 1);
    sa.Insert("y", 3);
    sa.Insert("x", 5);
    sa.Insert("c", 7);
    sa.Insert("b", 9);
    sa.Insert("a", 11);

    sa.Sort();

    const nsArrayMap<nsString, nsInt32> sa2(sa);

    NS_TEST_INT(sa.GetValue(0), 11);
    NS_TEST_INT(sa.GetValue(2), 7);

    NS_TEST_INT(sa2.GetValue(0), 11);
    NS_TEST_INT(sa2.GetValue(2), 7);

    NS_TEST_STRING(sa.GetKey(1), "b");
    NS_TEST_STRING(sa.GetKey(3), "x");

    NS_TEST_INT(sa["b"], 9);
    NS_TEST_INT(sa["y"], 3);

    NS_TEST_INT(sa.GetPair(2).value, 7);
    NS_TEST_STRING(sa.GetPair(4).key, "y");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Remove")
  {
    nsArrayMap<nsString, nsInt32> sa;

    bool bExisted = true;

    sa.FindOrAdd("a", &bExisted) = 2;
    NS_TEST_BOOL(!bExisted);

    sa.FindOrAdd("b", &bExisted) = 4;
    NS_TEST_BOOL(!bExisted);

    sa.FindOrAdd("c", &bExisted) = 6;
    NS_TEST_BOOL(!bExisted);

    sa.FindOrAdd("b", &bExisted) = 5;
    NS_TEST_BOOL(bExisted);

    NS_TEST_INT(sa.GetCount(), 3);

    NS_TEST_INT(sa.Find("a"), 0);
    NS_TEST_INT(sa.Find("c"), 2);

    sa.RemoveAndCopy("b");
    NS_TEST_INT(sa.GetCount(), 2);

    NS_TEST_INT(sa.Find("b"), nsInvalidIndex);

    NS_TEST_INT(sa.Find("a"), 0);
    NS_TEST_INT(sa.Find("c"), 1);

    sa.RemoveAtAndCopy(1);
    NS_TEST_INT(sa.GetCount(), 1);

    NS_TEST_INT(sa.Find("a"), 0);
    NS_TEST_INT(sa.Find("c"), nsInvalidIndex);

    sa.RemoveAtAndCopy(0);
    NS_TEST_INT(sa.GetCount(), 0);

    NS_TEST_INT(sa.Find("a"), nsInvalidIndex);
    NS_TEST_INT(sa.Find("c"), nsInvalidIndex);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Stresstest")
  {
    // Interestingly the map is not really slower than the sorted array, at least not in debug builds

    nsStopwatch s;
    nsArrayMap<nsInt32, nsInt32> sa;
    nsMap<nsInt32, nsInt32> map;

    const nsInt32 uiElements = 100000;

    // const nsTime t0 = s.Checkpoint();

    {
      sa.Reserve(uiElements);

      for (nsUInt32 i = 0; i < uiElements; ++i)
      {
        sa.Insert(uiElements - i, i * 2);
      }

      sa.Sort();
    }

    // const nsTime t1 = s.Checkpoint();

    {
      for (nsInt32 i = 0; i < uiElements; ++i)
      {
        NS_TEST_INT(sa.GetValue(sa.Find(uiElements - i)), i * 2);
      }
    }

    // const nsTime t2 = s.Checkpoint();

    {
      for (nsUInt32 i = 0; i < uiElements; ++i)
      {
        map.Insert(uiElements - i, i * 2);
      }
    }

    // const nsTime t3 = s.Checkpoint();

    {
      for (nsUInt32 i = 0; i < uiElements; ++i)
      {
        NS_TEST_INT(map[uiElements - i], i * 2);
      }
    }

    // const nsTime t4 = s.Checkpoint();

    // int breakpoint = 0;
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Lower Bound / Upper Bound")
  {
    nsArrayMap<nsInt32, nsInt32> sa;
    sa[1] = 23;
    sa[3] = 23;
    sa[4] = 23;
    sa[6] = 23;
    sa[7] = 23;
    sa[9] = 23;
    sa[11] = 23;
    sa[14] = 23;
    sa[17] = 23;

    NS_TEST_INT(sa.LowerBound(0), 0);
    NS_TEST_INT(sa.LowerBound(1), 0);
    NS_TEST_INT(sa.LowerBound(2), 1);
    NS_TEST_INT(sa.LowerBound(3), 1);
    NS_TEST_INT(sa.LowerBound(4), 2);
    NS_TEST_INT(sa.LowerBound(5), 3);
    NS_TEST_INT(sa.LowerBound(6), 3);
    NS_TEST_INT(sa.LowerBound(7), 4);
    NS_TEST_INT(sa.LowerBound(8), 5);
    NS_TEST_INT(sa.LowerBound(9), 5);
    NS_TEST_INT(sa.LowerBound(10), 6);
    NS_TEST_INT(sa.LowerBound(11), 6);
    NS_TEST_INT(sa.LowerBound(12), 7);
    NS_TEST_INT(sa.LowerBound(13), 7);
    NS_TEST_INT(sa.LowerBound(14), 7);
    NS_TEST_INT(sa.LowerBound(15), 8);
    NS_TEST_INT(sa.LowerBound(16), 8);
    NS_TEST_INT(sa.LowerBound(17), 8);
    NS_TEST_INT(sa.LowerBound(18), nsInvalidIndex);
    NS_TEST_INT(sa.LowerBound(19), nsInvalidIndex);
    NS_TEST_INT(sa.LowerBound(20), nsInvalidIndex);

    NS_TEST_INT(sa.UpperBound(0), 0);
    NS_TEST_INT(sa.UpperBound(1), 1);
    NS_TEST_INT(sa.UpperBound(2), 1);
    NS_TEST_INT(sa.UpperBound(3), 2);
    NS_TEST_INT(sa.UpperBound(4), 3);
    NS_TEST_INT(sa.UpperBound(5), 3);
    NS_TEST_INT(sa.UpperBound(6), 4);
    NS_TEST_INT(sa.UpperBound(7), 5);
    NS_TEST_INT(sa.UpperBound(8), 5);
    NS_TEST_INT(sa.UpperBound(9), 6);
    NS_TEST_INT(sa.UpperBound(10), 6);
    NS_TEST_INT(sa.UpperBound(11), 7);
    NS_TEST_INT(sa.UpperBound(12), 7);
    NS_TEST_INT(sa.UpperBound(13), 7);
    NS_TEST_INT(sa.UpperBound(14), 8);
    NS_TEST_INT(sa.UpperBound(15), 8);
    NS_TEST_INT(sa.UpperBound(16), 8);
    NS_TEST_INT(sa.UpperBound(17), nsInvalidIndex);
    NS_TEST_INT(sa.UpperBound(18), nsInvalidIndex);
    NS_TEST_INT(sa.UpperBound(19), nsInvalidIndex);
    NS_TEST_INT(sa.UpperBound(20), nsInvalidIndex);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Duplicate Keys")
  {
    nsArrayMap<nsInt32, nsInt32> sa;

    sa.Insert(32, 1);
    sa.Insert(31, 1);
    sa.Insert(33, 1);

    sa.Insert(40, 1);
    sa.Insert(44, 1);
    sa.Insert(46, 1);

    sa.Insert(11, 1);
    sa.Insert(15, 1);
    sa.Insert(19, 1);

    sa.Insert(11, 2);
    sa.Insert(15, 2);
    sa.Insert(31, 2);
    sa.Insert(44, 2);

    sa.Insert(11, 3);
    sa.Insert(15, 3);
    sa.Insert(44, 3);

    sa.Insert(60, 1);
    sa.Insert(60, 2);
    sa.Insert(60, 3);
    sa.Insert(60, 4);
    sa.Insert(60, 5);
    sa.Insert(60, 6);
    sa.Insert(60, 7);
    sa.Insert(60, 8);
    sa.Insert(60, 9);
    sa.Insert(60, 10);

    sa.Sort();

    NS_TEST_INT(sa.LowerBound(11), 0);
    NS_TEST_INT(sa.LowerBound(15), 3);
    NS_TEST_INT(sa.LowerBound(19), 6);

    NS_TEST_INT(sa.LowerBound(31), 7);
    NS_TEST_INT(sa.LowerBound(32), 9);
    NS_TEST_INT(sa.LowerBound(33), 10);

    NS_TEST_INT(sa.LowerBound(40), 11);
    NS_TEST_INT(sa.LowerBound(44), 12);
    NS_TEST_INT(sa.LowerBound(46), 15);

    NS_TEST_INT(sa.LowerBound(60), 16);


    NS_TEST_INT(sa.UpperBound(11), 3);
    NS_TEST_INT(sa.UpperBound(15), 6);
    NS_TEST_INT(sa.UpperBound(19), 7);

    NS_TEST_INT(sa.UpperBound(31), 9);
    NS_TEST_INT(sa.UpperBound(32), 10);
    NS_TEST_INT(sa.UpperBound(33), 11);

    NS_TEST_INT(sa.UpperBound(40), 12);
    NS_TEST_INT(sa.UpperBound(44), 15);
    NS_TEST_INT(sa.UpperBound(46), 16);

    NS_TEST_INT(sa.UpperBound(60), nsInvalidIndex);
  }
}

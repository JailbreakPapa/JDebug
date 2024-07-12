#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Containers/HashSet.h>
#include <Foundation/Containers/StaticArray.h>
#include <Foundation/Memory/CommonAllocators.h>

namespace
{
  using st = nsConstructionCounter;

  struct Collision
  {
    nsUInt32 hash;
    int key;

    inline Collision(nsUInt32 uiHash, int iKey)
    {
      this->hash = uiHash;
      this->key = iKey;
    }

    inline bool operator==(const Collision& other) const { return key == other.key; }

    NS_DECLARE_POD_TYPE();
  };

  class OnlyMovable
  {
  public:
    OnlyMovable(nsUInt32 uiHash)
      : hash(uiHash)

    {
    }
    OnlyMovable(OnlyMovable&& other) { *this = std::move(other); }

    void operator=(OnlyMovable&& other)
    {
      hash = other.hash;
      m_NumTimesMoved = 0;
      ++other.m_NumTimesMoved;
    }

    bool operator==(const OnlyMovable& other) const { return hash == other.hash; }

    int m_NumTimesMoved = 0;
    nsUInt32 hash;

  private:
    OnlyMovable(const OnlyMovable&);
    void operator=(const OnlyMovable&);
  };
} // namespace

template <>
struct nsHashHelper<Collision>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(const Collision& value) { return value.hash; }

  NS_ALWAYS_INLINE static bool Equal(const Collision& a, const Collision& b) { return a == b; }
};

template <>
struct nsHashHelper<OnlyMovable>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(const OnlyMovable& value) { return value.hash; }

  NS_ALWAYS_INLINE static bool Equal(const OnlyMovable& a, const OnlyMovable& b) { return a.hash == b.hash; }
};

NS_CREATE_SIMPLE_TEST(Containers, HashSet)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
    nsHashSet<nsInt32> table1;

    NS_TEST_BOOL(table1.GetCount() == 0);
    NS_TEST_BOOL(table1.IsEmpty());

    nsUInt32 counter = 0;
    for (auto it = table1.GetIterator(); it.IsValid(); ++it)
    {
      ++counter;
    }
    NS_TEST_INT(counter, 0);

    NS_TEST_BOOL(begin(table1) == end(table1));
    NS_TEST_BOOL(cbegin(table1) == cend(table1));
    table1.Reserve(10);
    NS_TEST_BOOL(begin(table1) == end(table1));
    NS_TEST_BOOL(cbegin(table1) == cend(table1));

    for (auto value : table1)
    {
      ++counter;
    }
    NS_TEST_INT(counter, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Copy Constructor/Assignment/Iterator")
  {
    nsHashSet<nsInt32> table1;

    for (nsInt32 i = 0; i < 64; ++i)
    {
      nsInt32 key;

      do
      {
        key = rand() % 100000;
      } while (table1.Contains(key));

      table1.Insert(key);
    }

    // insert an element at the very end
    table1.Insert(47);

    nsHashSet<nsInt32> table2;
    table2 = table1;
    nsHashSet<nsInt32> table3(table1);

    NS_TEST_INT(table1.GetCount(), 65);
    NS_TEST_INT(table2.GetCount(), 65);
    NS_TEST_INT(table3.GetCount(), 65);
    NS_TEST_BOOL(begin(table1) != end(table1));
    NS_TEST_BOOL(cbegin(table1) != cend(table1));

    nsUInt32 uiCounter = 0;
    for (auto it = table1.GetIterator(); it.IsValid(); ++it)
    {
      nsConstructionCounter value;
      NS_TEST_BOOL(table2.Contains(it.Key()));
      NS_TEST_BOOL(table3.Contains(it.Key()));
      ++uiCounter;
    }
    NS_TEST_INT(uiCounter, table1.GetCount());

    uiCounter = 0;
    for (const auto& value : table1)
    {
      NS_TEST_BOOL(table2.Contains(value));
      NS_TEST_BOOL(table3.Contains(value));
      ++uiCounter;
    }
    NS_TEST_INT(uiCounter, table1.GetCount());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Move Copy Constructor/Assignment")
  {
    nsHashSet<st> set1;
    for (nsInt32 i = 0; i < 64; ++i)
    {
      set1.Insert(nsConstructionCounter(i));
    }

    nsUInt64 memoryUsage = set1.GetHeapMemoryUsage();

    nsHashSet<st> set2;
    set2 = std::move(set1);

    NS_TEST_INT(set1.GetCount(), 0);
    NS_TEST_INT(set1.GetHeapMemoryUsage(), 0);
    NS_TEST_INT(set2.GetCount(), 64);
    NS_TEST_INT(set2.GetHeapMemoryUsage(), memoryUsage);

    nsHashSet<st> set3(std::move(set2));

    NS_TEST_INT(set2.GetCount(), 0);
    NS_TEST_INT(set2.GetHeapMemoryUsage(), 0);
    NS_TEST_INT(set3.GetCount(), 64);
    NS_TEST_INT(set3.GetHeapMemoryUsage(), memoryUsage);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Collision Tests")
  {
    nsHashSet<Collision> set2;

    set2.Insert(Collision(0, 0));
    set2.Insert(Collision(1, 1));
    set2.Insert(Collision(0, 2));
    set2.Insert(Collision(1, 3));
    set2.Insert(Collision(1, 4));
    set2.Insert(Collision(0, 5));

    NS_TEST_BOOL(set2.Contains(Collision(0, 0)));
    NS_TEST_BOOL(set2.Contains(Collision(1, 1)));
    NS_TEST_BOOL(set2.Contains(Collision(0, 2)));
    NS_TEST_BOOL(set2.Contains(Collision(1, 3)));
    NS_TEST_BOOL(set2.Contains(Collision(1, 4)));
    NS_TEST_BOOL(set2.Contains(Collision(0, 5)));

    NS_TEST_BOOL(set2.Remove(Collision(0, 0)));
    NS_TEST_BOOL(set2.Remove(Collision(1, 1)));

    NS_TEST_BOOL(!set2.Contains(Collision(0, 0)));
    NS_TEST_BOOL(!set2.Contains(Collision(1, 1)));
    NS_TEST_BOOL(set2.Contains(Collision(0, 2)));
    NS_TEST_BOOL(set2.Contains(Collision(1, 3)));
    NS_TEST_BOOL(set2.Contains(Collision(1, 4)));
    NS_TEST_BOOL(set2.Contains(Collision(0, 5)));

    set2.Insert(Collision(0, 6));
    set2.Insert(Collision(1, 7));

    NS_TEST_BOOL(set2.Contains(Collision(0, 2)));
    NS_TEST_BOOL(set2.Contains(Collision(1, 3)));
    NS_TEST_BOOL(set2.Contains(Collision(1, 4)));
    NS_TEST_BOOL(set2.Contains(Collision(0, 5)));
    NS_TEST_BOOL(set2.Contains(Collision(0, 6)));
    NS_TEST_BOOL(set2.Contains(Collision(1, 7)));

    NS_TEST_BOOL(set2.Remove(Collision(1, 4)));
    NS_TEST_BOOL(set2.Remove(Collision(0, 6)));

    NS_TEST_BOOL(!set2.Contains(Collision(1, 4)));
    NS_TEST_BOOL(!set2.Contains(Collision(0, 6)));
    NS_TEST_BOOL(set2.Contains(Collision(0, 2)));
    NS_TEST_BOOL(set2.Contains(Collision(1, 3)));
    NS_TEST_BOOL(set2.Contains(Collision(0, 5)));
    NS_TEST_BOOL(set2.Contains(Collision(1, 7)));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Clear")
  {
    NS_TEST_BOOL(st::HasAllDestructed());

    {
      nsHashSet<st> m1;
      m1.Insert(st(1));
      NS_TEST_BOOL(st::HasDone(2, 1)); // for inserting new elements 1 temporary is created (and destroyed)

      m1.Insert(st(3));
      NS_TEST_BOOL(st::HasDone(2, 1)); // for inserting new elements 2 temporary is created (and destroyed)

      m1.Insert(st(1));
      NS_TEST_BOOL(st::HasDone(1, 1)); // nothing new to create, so only the one temporary is used

      m1.Clear();
      NS_TEST_BOOL(st::HasDone(0, 2));
      NS_TEST_BOOL(st::HasAllDestructed());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Insert")
  {
    nsHashSet<nsInt32> a1;

    for (nsInt32 i = 0; i < 10; ++i)
    {
      NS_TEST_BOOL(!a1.Insert(i));
    }

    for (nsInt32 i = 0; i < 10; ++i)
    {
      NS_TEST_BOOL(a1.Insert(i));
    }
  }


  NS_TEST_BLOCK(nsTestBlock::Enabled, "Move Insert")
  {
    OnlyMovable noCopyObject(42);

    nsHashSet<OnlyMovable> noCopyKey;
    // noCopyKey.Insert(noCopyObject); // Should not compile
    noCopyKey.Insert(std::move(noCopyObject));
    NS_TEST_INT(noCopyObject.m_NumTimesMoved, 1);
    NS_TEST_BOOL(noCopyKey.Contains(noCopyObject));
  }


  NS_TEST_BLOCK(nsTestBlock::Enabled, "Remove/Compact")
  {
    nsHashSet<nsInt32> a;

    NS_TEST_BOOL(a.GetHeapMemoryUsage() == 0);

    for (nsInt32 i = 0; i < 1000; ++i)
    {
      a.Insert(i);
      NS_TEST_INT(a.GetCount(), i + 1);
    }

    NS_TEST_BOOL(a.GetHeapMemoryUsage() >= 1000 * (sizeof(nsInt32)));

    a.Compact();

    for (nsInt32 i = 0; i < 500; ++i)
    {
      NS_TEST_BOOL(a.Remove(i));
    }

    a.Compact();

    for (nsInt32 i = 500; i < 1000; ++i)
    {
      NS_TEST_BOOL(a.Contains(i));
    }

    a.Clear();
    a.Compact();

    NS_TEST_BOOL(a.GetHeapMemoryUsage() == 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Remove (Iterator)")
  {
    nsHashSet<nsInt32> a;

    NS_TEST_BOOL(a.GetHeapMemoryUsage() == 0);
    for (nsInt32 i = 0; i < 1000; ++i)
      a.Insert(i);

    nsHashSet<nsInt32>::ConstIterator it = a.GetIterator();

    for (nsInt32 i = 0; i < 1000 - 1; ++i)
    {
      nsInt32 value = it.Key();
      it = a.Remove(it);
      NS_TEST_BOOL(!a.Contains(value));
      NS_TEST_BOOL(it.IsValid());
      NS_TEST_INT(a.GetCount(), 1000 - 1 - i);
    }
    it = a.Remove(it);
    NS_TEST_BOOL(!it.IsValid());
    NS_TEST_BOOL(a.IsEmpty());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Set Operations")
  {
    nsHashSet<nsUInt32> base;
    base.Insert(1);
    base.Insert(3);
    base.Insert(5);

    nsHashSet<nsUInt32> empty;

    nsHashSet<nsUInt32> disjunct;
    disjunct.Insert(2);
    disjunct.Insert(4);
    disjunct.Insert(6);

    nsHashSet<nsUInt32> subSet;
    subSet.Insert(1);
    subSet.Insert(5);

    nsHashSet<nsUInt32> superSet;
    superSet.Insert(1);
    superSet.Insert(3);
    superSet.Insert(5);
    superSet.Insert(7);

    nsHashSet<nsUInt32> nonDisjunctNonEmptySubSet;
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
      nsHashSet<nsUInt32> res;

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
      nsHashSet<nsUInt32> res;
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
      nsHashSet<nsUInt32> res;
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

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator==/!=")
  {
    nsStaticArray<nsInt32, 64> keys[2];

    for (nsUInt32 i = 0; i < 64; ++i)
    {
      keys[0].PushBack(rand());
    }

    keys[1] = keys[0];

    nsHashSet<nsInt32> t[2];

    for (nsUInt32 i = 0; i < 2; ++i)
    {
      while (!keys[i].IsEmpty())
      {
        const nsUInt32 uiIndex = rand() % keys[i].GetCount();
        const nsInt32 key = keys[i][uiIndex];
        t[i].Insert(key);

        keys[i].RemoveAtAndSwap(uiIndex);
      }
    }

    NS_TEST_BOOL(t[0] == t[1]);

    t[0].Insert(32);
    NS_TEST_BOOL(t[0] != t[1]);

    t[1].Insert(32);
    NS_TEST_BOOL(t[0] == t[1]);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CompatibleKeyType")
  {
    nsProxyAllocator testAllocator("Test", nsFoundation::GetDefaultAllocator());
    nsLocalAllocatorWrapper allocWrapper(&testAllocator);
    using TestString = nsHybridString<32, nsLocalAllocatorWrapper>;

    nsHashSet<TestString> stringSet;
    const char* szChar = "VeryLongStringDefinitelyMoreThan32Chars1111elf!!!!";
    const char* szString = "AnotherVeryLongStringThisTimeUsedForStringView!!!!";
    nsStringView sView(szString);
    nsStringBuilder sBuilder("BuilderAlsoNeedsToBeAVeryLongStringToTriggerAllocation");
    nsString sString("String");
    NS_TEST_BOOL(!stringSet.Insert(szChar));
    NS_TEST_BOOL(!stringSet.Insert(sView));
    NS_TEST_BOOL(!stringSet.Insert(sBuilder));
    NS_TEST_BOOL(!stringSet.Insert(sString));
    NS_TEST_BOOL(stringSet.Insert(szString));

    nsUInt64 oldAllocCount = testAllocator.GetStats().m_uiNumAllocations;

    NS_TEST_BOOL(stringSet.Contains(szChar));
    NS_TEST_BOOL(stringSet.Contains(sView));
    NS_TEST_BOOL(stringSet.Contains(sBuilder));
    NS_TEST_BOOL(stringSet.Contains(sString));

    NS_TEST_INT(testAllocator.GetStats().m_uiNumAllocations, oldAllocCount);

    NS_TEST_BOOL(stringSet.Remove(szChar));
    NS_TEST_BOOL(stringSet.Remove(sView));
    NS_TEST_BOOL(stringSet.Remove(sBuilder));
    NS_TEST_BOOL(stringSet.Remove(sString));

    NS_TEST_INT(testAllocator.GetStats().m_uiNumAllocations, oldAllocCount);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Swap")
  {
    nsStringBuilder tmp;
    nsHashSet<nsString> set1;
    nsHashSet<nsString> set2;

    for (nsUInt32 i = 0; i < 1000; ++i)
    {
      tmp.SetFormat("stuff{}bla", i);
      set1.Insert(tmp);

      tmp.SetFormat("{0}{0}{0}", i);
      set2.Insert(tmp);
    }

    set1.Swap(set2);

    for (nsUInt32 i = 0; i < 1000; ++i)
    {
      tmp.SetFormat("stuff{}bla", i);
      NS_TEST_BOOL(set2.Contains(tmp));

      tmp.SetFormat("{0}{0}{0}", i);
      NS_TEST_BOOL(set1.Contains(tmp));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "foreach")
  {
    nsStringBuilder tmp;
    nsHashSet<nsString> set;
    nsHashSet<nsString> set2;

    for (nsUInt32 i = 0; i < 1000; ++i)
    {
      tmp.SetFormat("stuff{}bla", i);
      set.Insert(tmp);
    }

    NS_TEST_INT(set.GetCount(), 1000);

    set2 = set;
    NS_TEST_INT(set2.GetCount(), set.GetCount());

    for (nsHashSet<nsString>::ConstIterator it = begin(set); it != end(set); ++it)
    {
      const nsString& k = it.Key();
      set2.Remove(k);
    }

    NS_TEST_BOOL(set2.IsEmpty());
    set2 = set;

    for (auto key : set)
    {
      set2.Remove(key);
    }

    NS_TEST_BOOL(set2.IsEmpty());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Find")
  {
    nsStringBuilder tmp;
    nsHashSet<nsString> set;

    for (nsUInt32 i = 0; i < 1000; ++i)
    {
      tmp.SetFormat("stuff{}bla", i);
      set.Insert(tmp);
    }

    for (nsInt32 i = set.GetCount() - 1; i > 0; --i)
    {
      tmp.SetFormat("stuff{}bla", i);

      auto it = set.Find(tmp);

      NS_TEST_STRING(it.Key(), tmp);

      int allowedIterations = set.GetCount();
      for (auto it2 = it; it2.IsValid(); ++it2)
      {
        // just test that iteration is possible and terminates correctly
        --allowedIterations;
        NS_TEST_BOOL(allowedIterations >= 0);
      }

      set.Remove(it);
    }
  }
}

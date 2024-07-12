#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Containers/HashTable.h>
#include <Foundation/Containers/StaticArray.h>
#include <Foundation/Memory/CommonAllocators.h>
#include <Foundation/Strings/String.h>

namespace HashTableTestDetail
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
} // namespace HashTableTestDetail

template <>
struct nsHashHelper<HashTableTestDetail::Collision>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(const HashTableTestDetail::Collision& value) { return value.hash; }

  NS_ALWAYS_INLINE static bool Equal(const HashTableTestDetail::Collision& a, const HashTableTestDetail::Collision& b) { return a == b; }
};

template <>
struct nsHashHelper<HashTableTestDetail::OnlyMovable>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(const HashTableTestDetail::OnlyMovable& value) { return value.hash; }

  NS_ALWAYS_INLINE static bool Equal(const HashTableTestDetail::OnlyMovable& a, const HashTableTestDetail::OnlyMovable& b)
  {
    return a.hash == b.hash;
  }
};

NS_CREATE_SIMPLE_TEST(Containers, HashTable)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
    nsHashTable<nsInt32, HashTableTestDetail::st> table1;

    NS_TEST_BOOL(table1.GetCount() == 0);
    NS_TEST_BOOL(table1.IsEmpty());

    nsUInt32 counter = 0;
    for (nsHashTable<nsInt32, HashTableTestDetail::st>::ConstIterator it = table1.GetIterator(); it.IsValid(); ++it)
    {
      ++counter;
    }
    NS_TEST_INT(counter, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Copy Constructor/Assignment/Iterator")
  {
    nsHashTable<nsInt32, HashTableTestDetail::st> table1;

    for (nsInt32 i = 0; i < 64; ++i)
    {
      nsInt32 key;

      do
      {
        key = rand() % 100000;
      } while (table1.Contains(key));

      table1.Insert(key, nsConstructionCounter(i));
    }

    // insert an element at the very end
    table1.Insert(47, nsConstructionCounter(64));

    nsHashTable<nsInt32, HashTableTestDetail::st> table2;
    table2 = table1;
    nsHashTable<nsInt32, HashTableTestDetail::st> table3(table1);

    NS_TEST_INT(table1.GetCount(), 65);
    NS_TEST_INT(table2.GetCount(), 65);
    NS_TEST_INT(table3.GetCount(), 65);

    nsUInt32 uiCounter = 0;
    for (nsHashTable<nsInt32, HashTableTestDetail::st>::ConstIterator it = table1.GetIterator(); it.IsValid(); ++it)
    {
      nsConstructionCounter value;

      NS_TEST_BOOL(table2.TryGetValue(it.Key(), value));
      NS_TEST_BOOL(it.Value() == value);
      NS_TEST_BOOL(*table2.GetValue(it.Key()) == it.Value());

      NS_TEST_BOOL(table3.TryGetValue(it.Key(), value));
      NS_TEST_BOOL(it.Value() == value);
      NS_TEST_BOOL(*table3.GetValue(it.Key()) == it.Value());

      ++uiCounter;
    }
    NS_TEST_INT(uiCounter, table1.GetCount());

    for (nsHashTable<nsInt32, HashTableTestDetail::st>::Iterator it = table1.GetIterator(); it.IsValid(); ++it)
    {
      it.Value() = HashTableTestDetail::st(42);
    }

    for (nsHashTable<nsInt32, HashTableTestDetail::st>::ConstIterator it = table1.GetIterator(); it.IsValid(); ++it)
    {
      nsConstructionCounter value;

      NS_TEST_BOOL(table1.TryGetValue(it.Key(), value));
      NS_TEST_BOOL(it.Value() == value);
      NS_TEST_BOOL(value.m_iData == 42);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Move Copy Constructor/Assignment")
  {
    nsHashTable<nsInt32, HashTableTestDetail::st> table1;
    for (nsInt32 i = 0; i < 64; ++i)
    {
      table1.Insert(i, nsConstructionCounter(i));
    }

    nsUInt64 memoryUsage = table1.GetHeapMemoryUsage();

    nsHashTable<nsInt32, HashTableTestDetail::st> table2;
    table2 = std::move(table1);

    NS_TEST_INT(table1.GetCount(), 0);
    NS_TEST_INT(table1.GetHeapMemoryUsage(), 0);
    NS_TEST_INT(table2.GetCount(), 64);
    NS_TEST_INT(table2.GetHeapMemoryUsage(), memoryUsage);

    nsHashTable<nsInt32, HashTableTestDetail::st> table3(std::move(table2));

    NS_TEST_INT(table2.GetCount(), 0);
    NS_TEST_INT(table2.GetHeapMemoryUsage(), 0);
    NS_TEST_INT(table3.GetCount(), 64);
    NS_TEST_INT(table3.GetHeapMemoryUsage(), memoryUsage);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Move Insert")
  {
    HashTableTestDetail::OnlyMovable noCopyObject(42);

    {
      nsHashTable<HashTableTestDetail::OnlyMovable, int> noCopyKey;
      // noCopyKey.Insert(noCopyObject, 10); // Should not compile
      noCopyKey.Insert(std::move(noCopyObject), 10);
      NS_TEST_INT(noCopyObject.m_NumTimesMoved, 1);
      NS_TEST_BOOL(noCopyKey.Contains(noCopyObject));
    }

    {
      nsHashTable<int, HashTableTestDetail::OnlyMovable> noCopyValue;
      // noCopyValue.Insert(10, noCopyObject); // Should not compile
      noCopyValue.Insert(10, std::move(noCopyObject));
      NS_TEST_INT(noCopyObject.m_NumTimesMoved, 2);
      NS_TEST_BOOL(noCopyValue.Contains(10));
    }

    {
      nsHashTable<HashTableTestDetail::OnlyMovable, HashTableTestDetail::OnlyMovable> noCopyAnything;
      // noCopyAnything.Insert(10, noCopyObject); // Should not compile
      // noCopyAnything.Insert(noCopyObject, 10); // Should not compile
      noCopyAnything.Insert(std::move(noCopyObject), std::move(noCopyObject));
      NS_TEST_INT(noCopyObject.m_NumTimesMoved, 4);
      NS_TEST_BOOL(noCopyAnything.Contains(noCopyObject));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Collision Tests")
  {
    nsHashTable<HashTableTestDetail::Collision, int> map2;

    map2[HashTableTestDetail::Collision(0, 0)] = 0;
    map2[HashTableTestDetail::Collision(1, 1)] = 1;
    map2[HashTableTestDetail::Collision(0, 2)] = 2;
    map2[HashTableTestDetail::Collision(1, 3)] = 3;
    map2[HashTableTestDetail::Collision(1, 4)] = 4;
    map2[HashTableTestDetail::Collision(0, 5)] = 5;

    NS_TEST_BOOL(map2[HashTableTestDetail::Collision(0, 0)] == 0);
    NS_TEST_BOOL(map2[HashTableTestDetail::Collision(1, 1)] == 1);
    NS_TEST_BOOL(map2[HashTableTestDetail::Collision(0, 2)] == 2);
    NS_TEST_BOOL(map2[HashTableTestDetail::Collision(1, 3)] == 3);
    NS_TEST_BOOL(map2[HashTableTestDetail::Collision(1, 4)] == 4);
    NS_TEST_BOOL(map2[HashTableTestDetail::Collision(0, 5)] == 5);

    NS_TEST_BOOL(map2.Contains(HashTableTestDetail::Collision(0, 0)));
    NS_TEST_BOOL(map2.Contains(HashTableTestDetail::Collision(1, 1)));
    NS_TEST_BOOL(map2.Contains(HashTableTestDetail::Collision(0, 2)));
    NS_TEST_BOOL(map2.Contains(HashTableTestDetail::Collision(1, 3)));
    NS_TEST_BOOL(map2.Contains(HashTableTestDetail::Collision(1, 4)));
    NS_TEST_BOOL(map2.Contains(HashTableTestDetail::Collision(0, 5)));

    NS_TEST_BOOL(map2.Remove(HashTableTestDetail::Collision(0, 0)));
    NS_TEST_BOOL(map2.Remove(HashTableTestDetail::Collision(1, 1)));

    NS_TEST_BOOL(map2[HashTableTestDetail::Collision(0, 2)] == 2);
    NS_TEST_BOOL(map2[HashTableTestDetail::Collision(1, 3)] == 3);
    NS_TEST_BOOL(map2[HashTableTestDetail::Collision(1, 4)] == 4);
    NS_TEST_BOOL(map2[HashTableTestDetail::Collision(0, 5)] == 5);

    NS_TEST_BOOL(!map2.Contains(HashTableTestDetail::Collision(0, 0)));
    NS_TEST_BOOL(!map2.Contains(HashTableTestDetail::Collision(1, 1)));
    NS_TEST_BOOL(map2.Contains(HashTableTestDetail::Collision(0, 2)));
    NS_TEST_BOOL(map2.Contains(HashTableTestDetail::Collision(1, 3)));
    NS_TEST_BOOL(map2.Contains(HashTableTestDetail::Collision(1, 4)));
    NS_TEST_BOOL(map2.Contains(HashTableTestDetail::Collision(0, 5)));

    map2[HashTableTestDetail::Collision(0, 6)] = 6;
    map2[HashTableTestDetail::Collision(1, 7)] = 7;

    NS_TEST_BOOL(map2[HashTableTestDetail::Collision(0, 2)] == 2);
    NS_TEST_BOOL(map2[HashTableTestDetail::Collision(1, 3)] == 3);
    NS_TEST_BOOL(map2[HashTableTestDetail::Collision(1, 4)] == 4);
    NS_TEST_BOOL(map2[HashTableTestDetail::Collision(0, 5)] == 5);
    NS_TEST_BOOL(map2[HashTableTestDetail::Collision(0, 6)] == 6);
    NS_TEST_BOOL(map2[HashTableTestDetail::Collision(1, 7)] == 7);

    NS_TEST_BOOL(map2.Contains(HashTableTestDetail::Collision(0, 2)));
    NS_TEST_BOOL(map2.Contains(HashTableTestDetail::Collision(1, 3)));
    NS_TEST_BOOL(map2.Contains(HashTableTestDetail::Collision(1, 4)));
    NS_TEST_BOOL(map2.Contains(HashTableTestDetail::Collision(0, 5)));
    NS_TEST_BOOL(map2.Contains(HashTableTestDetail::Collision(0, 6)));
    NS_TEST_BOOL(map2.Contains(HashTableTestDetail::Collision(1, 7)));

    NS_TEST_BOOL(map2.Remove(HashTableTestDetail::Collision(1, 4)));
    NS_TEST_BOOL(map2.Remove(HashTableTestDetail::Collision(0, 6)));

    NS_TEST_BOOL(map2[HashTableTestDetail::Collision(0, 2)] == 2);
    NS_TEST_BOOL(map2[HashTableTestDetail::Collision(1, 3)] == 3);
    NS_TEST_BOOL(map2[HashTableTestDetail::Collision(0, 5)] == 5);
    NS_TEST_BOOL(map2[HashTableTestDetail::Collision(1, 7)] == 7);

    NS_TEST_BOOL(!map2.Contains(HashTableTestDetail::Collision(1, 4)));
    NS_TEST_BOOL(!map2.Contains(HashTableTestDetail::Collision(0, 6)));
    NS_TEST_BOOL(map2.Contains(HashTableTestDetail::Collision(0, 2)));
    NS_TEST_BOOL(map2.Contains(HashTableTestDetail::Collision(1, 3)));
    NS_TEST_BOOL(map2.Contains(HashTableTestDetail::Collision(0, 5)));
    NS_TEST_BOOL(map2.Contains(HashTableTestDetail::Collision(1, 7)));

    map2[HashTableTestDetail::Collision(0, 2)] = 3;
    map2[HashTableTestDetail::Collision(0, 5)] = 6;
    map2[HashTableTestDetail::Collision(1, 3)] = 4;

    NS_TEST_BOOL(map2[HashTableTestDetail::Collision(0, 2)] == 3);
    NS_TEST_BOOL(map2[HashTableTestDetail::Collision(0, 5)] == 6);
    NS_TEST_BOOL(map2[HashTableTestDetail::Collision(1, 3)] == 4);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Clear")
  {
    NS_TEST_BOOL(HashTableTestDetail::st::HasAllDestructed());

    {
      nsHashTable<nsUInt32, HashTableTestDetail::st> m1;
      m1[0] = HashTableTestDetail::st(1);
      NS_TEST_BOOL(HashTableTestDetail::st::HasDone(2, 1)); // for inserting new elements 1 temporary is created (and destroyed)

      m1[1] = HashTableTestDetail::st(3);
      NS_TEST_BOOL(HashTableTestDetail::st::HasDone(2, 1)); // for inserting new elements 2 temporary is created (and destroyed)

      m1[0] = HashTableTestDetail::st(2);
      NS_TEST_BOOL(HashTableTestDetail::st::HasDone(1, 1)); // nothing new to create, so only the one temporary is used

      m1.Clear();
      NS_TEST_BOOL(HashTableTestDetail::st::HasDone(0, 2));
      NS_TEST_BOOL(HashTableTestDetail::st::HasAllDestructed());
    }

    {
      nsHashTable<HashTableTestDetail::st, nsUInt32> m1;
      m1[HashTableTestDetail::st(0)] = 1;
      NS_TEST_BOOL(HashTableTestDetail::st::HasDone(2, 1)); // one temporary

      m1[HashTableTestDetail::st(1)] = 3;
      NS_TEST_BOOL(HashTableTestDetail::st::HasDone(2, 1)); // one temporary

      m1[HashTableTestDetail::st(0)] = 2;
      NS_TEST_BOOL(HashTableTestDetail::st::HasDone(1, 1)); // nothing new to create, so only the one temporary is used

      m1.Clear();
      NS_TEST_BOOL(HashTableTestDetail::st::HasDone(0, 2));
      NS_TEST_BOOL(HashTableTestDetail::st::HasAllDestructed());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Insert/TryGetValue/GetValue")
  {
    nsHashTable<nsInt32, HashTableTestDetail::st> a1;

    for (nsInt32 i = 0; i < 10; ++i)
    {
      NS_TEST_BOOL(!a1.Insert(i, i - 20));
    }

    for (nsInt32 i = 0; i < 10; ++i)
    {
      HashTableTestDetail::st oldValue;
      NS_TEST_BOOL(a1.Insert(i, i, &oldValue));
      NS_TEST_INT(oldValue.m_iData, i - 20);
    }

    HashTableTestDetail::st value;
    NS_TEST_BOOL(a1.TryGetValue(9, value));
    NS_TEST_INT(value.m_iData, 9);
    NS_TEST_INT(a1.GetValue(9)->m_iData, 9);

    NS_TEST_BOOL(!a1.TryGetValue(11, value));
    NS_TEST_INT(value.m_iData, 9);
    NS_TEST_BOOL(a1.GetValue(11) == nullptr);

    HashTableTestDetail::st* pValue;
    NS_TEST_BOOL(a1.TryGetValue(9, pValue));
    NS_TEST_INT(pValue->m_iData, 9);

    pValue->m_iData = 20;
    NS_TEST_INT(a1[9].m_iData, 20);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Remove/Compact")
  {
    nsHashTable<nsInt32, HashTableTestDetail::st> a;

    NS_TEST_BOOL(a.GetHeapMemoryUsage() == 0);

    for (nsInt32 i = 0; i < 1000; ++i)
    {
      a.Insert(i, i);
      NS_TEST_INT(a.GetCount(), i + 1);
    }

    NS_TEST_BOOL(a.GetHeapMemoryUsage() >= 1000 * (sizeof(nsInt32) + sizeof(HashTableTestDetail::st)));

    a.Compact();

    for (nsInt32 i = 0; i < 1000; ++i)
      NS_TEST_INT(a[i].m_iData, i);


    for (nsInt32 i = 0; i < 250; ++i)
    {
      HashTableTestDetail::st oldValue;
      NS_TEST_BOOL(a.Remove(i, &oldValue));
      NS_TEST_INT(oldValue.m_iData, i);
    }
    NS_TEST_INT(a.GetCount(), 750);

    for (nsHashTable<nsInt32, HashTableTestDetail::st>::Iterator it = a.GetIterator(); it.IsValid();)
    {
      if (it.Key() < 500)
        it = a.Remove(it);
      else
        ++it;
    }
    NS_TEST_INT(a.GetCount(), 500);
    a.Compact();

    for (nsInt32 i = 500; i < 1000; ++i)
      NS_TEST_INT(a[i].m_iData, i);

    a.Clear();
    a.Compact();

    NS_TEST_BOOL(a.GetHeapMemoryUsage() == 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator[]")
  {
    nsHashTable<nsInt32, nsInt32> a;

    a.Insert(4, 20);
    a[2] = 30;

    NS_TEST_INT(a[4], 20);
    NS_TEST_INT(a[2], 30);
    NS_TEST_INT(a[1], 0); // new values are default constructed
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator==/!=")
  {
    nsStaticArray<nsInt32, 64> keys[2];

    for (nsUInt32 i = 0; i < 64; ++i)
    {
      keys[0].PushBack(rand());
    }

    keys[1] = keys[0];

    nsHashTable<nsInt32, HashTableTestDetail::st> t[2];

    for (nsUInt32 i = 0; i < 2; ++i)
    {
      while (!keys[i].IsEmpty())
      {
        const nsUInt32 uiIndex = rand() % keys[i].GetCount();
        const nsInt32 key = keys[i][uiIndex];
        t[i].Insert(key, HashTableTestDetail::st(key * 3456));

        keys[i].RemoveAtAndSwap(uiIndex);
      }
    }

    NS_TEST_BOOL(t[0] == t[1]);

    t[0].Insert(32, HashTableTestDetail::st(64));
    NS_TEST_BOOL(t[0] != t[1]);

    t[1].Insert(32, HashTableTestDetail::st(47));
    NS_TEST_BOOL(t[0] != t[1]);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "CompatibleKeyType")
  {
    nsProxyAllocator testAllocator("Test", nsFoundation::GetDefaultAllocator());
    nsLocalAllocatorWrapper allocWrapper(&testAllocator);
    using TestString = nsHybridString<32, nsLocalAllocatorWrapper>;

    nsHashTable<TestString, int> stringTable;
    const char* szChar = "VeryLongStringDefinitelyMoreThan32Chars1111elf!!!!";
    const char* szString = "AnotherVeryLongStringThisTimeUsedForStringView!!!!";
    nsStringView sView(szString);
    nsStringBuilder sBuilder("BuilderAlsoNeedsToBeAVeryLongStringToTriggerAllocation");
    nsString sString("String");
    NS_TEST_BOOL(!stringTable.Insert(szChar, 1));
    NS_TEST_BOOL(!stringTable.Insert(sView, 2));
    NS_TEST_BOOL(!stringTable.Insert(sBuilder, 3));
    NS_TEST_BOOL(!stringTable.Insert(sString, 4));
    NS_TEST_BOOL(stringTable.Insert(szString, 2));

    nsUInt64 oldAllocCount = testAllocator.GetStats().m_uiNumAllocations;

    NS_TEST_BOOL(stringTable.Contains(szChar));
    NS_TEST_BOOL(stringTable.Contains(sView));
    NS_TEST_BOOL(stringTable.Contains(sBuilder));
    NS_TEST_BOOL(stringTable.Contains(sString));

    NS_TEST_INT(testAllocator.GetStats().m_uiNumAllocations, oldAllocCount);

    NS_TEST_INT(*stringTable.GetValue(szChar), 1);
    NS_TEST_INT(*stringTable.GetValue(sView), 2);
    NS_TEST_INT(*stringTable.GetValue(sBuilder), 3);
    NS_TEST_INT(*stringTable.GetValue(sString), 4);

    NS_TEST_INT(testAllocator.GetStats().m_uiNumAllocations, oldAllocCount);

    NS_TEST_BOOL(stringTable.Remove(szChar));
    NS_TEST_BOOL(stringTable.Remove(sView));
    NS_TEST_BOOL(stringTable.Remove(sBuilder));
    NS_TEST_BOOL(stringTable.Remove(sString));

    NS_TEST_INT(testAllocator.GetStats().m_uiNumAllocations, oldAllocCount);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Swap")
  {
    nsStringBuilder tmp;
    nsHashTable<nsString, nsInt32> map1;
    nsHashTable<nsString, nsInt32> map2;

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

  NS_TEST_BLOCK(nsTestBlock::Enabled, "foreach")
  {
    nsStringBuilder tmp;
    nsHashTable<nsString, nsInt32> map;
    nsHashTable<nsString, nsInt32> map2;

    for (nsUInt32 i = 0; i < 1000; ++i)
    {
      tmp.SetFormat("stuff{}bla", i);
      map[tmp] = i;
    }

    NS_TEST_INT(map.GetCount(), 1000);

    map2 = map;
    NS_TEST_INT(map2.GetCount(), map.GetCount());

    for (nsHashTable<nsString, nsInt32>::Iterator it = begin(map); it != end(map); ++it)
    {
      const nsString& k = it.Key();
      nsInt32 v = it.Value();

      map2.Remove(k);
    }

    NS_TEST_BOOL(map2.IsEmpty());
    map2 = map;

    for (auto it : map)
    {
      const nsString& k = it.Key();
      nsInt32 v = it.Value();

      map2.Remove(k);
    }

    NS_TEST_BOOL(map2.IsEmpty());
    map2 = map;

    // just check that this compiles
    for (auto it : static_cast<const nsHashTable<nsString, nsInt32>&>(map))
    {
      const nsString& k = it.Key();
      nsInt32 v = it.Value();

      map2.Remove(k);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Find")
  {
    nsStringBuilder tmp;
    nsHashTable<nsString, nsInt32> map;

    for (nsUInt32 i = 0; i < 1000; ++i)
    {
      tmp.SetFormat("stuff{}bla", i);
      map[tmp] = i;
    }

    for (nsInt32 i = map.GetCount() - 1; i > 0; --i)
    {
      tmp.SetFormat("stuff{}bla", i);

      auto it = map.Find(tmp);
      auto cit = static_cast<const nsHashTable<nsString, nsInt32>&>(map).Find(tmp);

      NS_TEST_STRING(it.Key(), tmp);
      NS_TEST_INT(it.Value(), i);

      NS_TEST_STRING(cit.Key(), tmp);
      NS_TEST_INT(cit.Value(), i);

      int allowedIterations = map.GetCount();
      for (auto it2 = it; it2.IsValid(); ++it2)
      {
        // just test that iteration is possible and terminates correctly
        --allowedIterations;
        NS_TEST_BOOL(allowedIterations >= 0);
      }

      allowedIterations = map.GetCount();
      for (auto cit2 = cit; cit2.IsValid(); ++cit2)
      {
        // just test that iteration is possible and terminates correctly
        --allowedIterations;
        NS_TEST_BOOL(allowedIterations >= 0);
      }

      map.Remove(it);
    }
  }
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Find")
  {
    nsStringBuilder tmp;
    nsHashTable<nsString, nsInt32> map;

    for (nsUInt32 i = 0; i < 1000; ++i)
    {
      tmp.SetFormat("stuff{}bla", i);
      map[tmp] = i;
    }

    for (nsInt32 i = map.GetCount() - 1; i > 0; --i)
    {
      tmp.SetFormat("stuff{}bla", i);

      auto it = map.Find(tmp);
      auto cit = static_cast<const nsHashTable<nsString, nsInt32>&>(map).Find(tmp);

      NS_TEST_STRING(it.Key(), tmp);
      NS_TEST_INT(it.Value(), i);

      NS_TEST_STRING(cit.Key(), tmp);
      NS_TEST_INT(cit.Value(), i);

      int allowedIterations = map.GetCount();
      for (auto it2 = it; it2.IsValid(); ++it2)
      {
        // just test that iteration is possible and terminates correctly
        --allowedIterations;
        NS_TEST_BOOL(allowedIterations >= 0);
      }

      allowedIterations = map.GetCount();
      for (auto cit2 = cit; cit2.IsValid(); ++cit2)
      {
        // just test that iteration is possible and terminates correctly
        --allowedIterations;
        NS_TEST_BOOL(allowedIterations >= 0);
      }

      map.Remove(it);
    }
  }
}

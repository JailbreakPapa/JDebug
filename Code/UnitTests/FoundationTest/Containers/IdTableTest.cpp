#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/IdTable.h>
#include <Foundation/Strings/String.h>

namespace
{
  using Id = nsGenericId<32, 16>;
  using st = nsConstructionCounter;

  struct TestObject
  {
    int x;
    nsString s;
  };
} // namespace

NS_CREATE_SIMPLE_TEST(Containers, IdTable)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
    nsIdTable<Id, nsInt32> table;

    NS_TEST_BOOL(table.GetCount() == 0);
    NS_TEST_BOOL(table.IsEmpty());

    nsUInt32 counter = 0;
    for (nsIdTable<Id, nsInt32>::ConstIterator it = table.GetIterator(); it.IsValid(); ++it)
    {
      ++counter;
    }
    NS_TEST_INT(counter, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Copy Constructor/Assignment/Iterator")
  {
    NS_TEST_BOOL(st::HasAllDestructed());
    {
      nsIdTable<Id, st> table1;

      for (nsInt32 i = 0; i < 200; ++i)
      {
        table1.Insert(st(i));
      }

      NS_TEST_BOOL(table1.Remove(Id(0, 1)));

      for (nsInt32 i = 0; i < 99; ++i)
      {
        Id id;
        id.m_Generation = 1;

        do
        {
          id.m_InstanceIndex = rand() % 200;
        } while (!table1.Contains(id));

        NS_TEST_BOOL(table1.Remove(id));
      }

      nsIdTable<Id, st> table2;
      table2 = table1;
      nsIdTable<Id, st> table3(table1);

      NS_TEST_BOOL(table2.IsFreelistValid());
      NS_TEST_BOOL(table3.IsFreelistValid());

      NS_TEST_INT(table1.GetCount(), 100);
      NS_TEST_INT(table2.GetCount(), 100);
      NS_TEST_INT(table3.GetCount(), 100);

      nsUInt32 uiCounter = 0;
      for (nsIdTable<Id, st>::ConstIterator it = table1.GetIterator(); it.IsValid(); ++it)
      {
        st value;

        NS_TEST_BOOL(table2.TryGetValue(it.Id(), value));
        NS_TEST_BOOL(it.Value() == value);

        NS_TEST_BOOL(table3.TryGetValue(it.Id(), value));
        NS_TEST_BOOL(it.Value() == value);

        ++uiCounter;
      }
      NS_TEST_INT(uiCounter, table1.GetCount());

      for (nsIdTable<Id, st>::Iterator it = table1.GetIterator(); it.IsValid(); ++it)
      {
        it.Value() = st(42);
      }

      for (nsIdTable<Id, st>::ConstIterator it = table1.GetIterator(); it.IsValid(); ++it)
      {
        st value;

        NS_TEST_BOOL(table1.TryGetValue(it.Id(), value));
        NS_TEST_BOOL(it.Value() == value);
        NS_TEST_BOOL(value.m_iData == 42);
      }
    }
    NS_TEST_BOOL(st::HasAllDestructed());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Verify 0 is never valid")
  {
    nsIdTable<Id, TestObject> table;

    nsUInt32 count1 = 0, count2 = 0;

    TestObject x = {11, "Test"};

    while (true)
    {
      Id id = table.Insert(x);
      NS_TEST_BOOL(id.m_Generation != 0);

      NS_TEST_BOOL(table.Remove(id));

      if (id.m_Generation > 1) // until all elements in generation 1 have been used up
        break;

      ++count1;
    }

    NS_TEST_BOOL(!table.Contains(Id(0, 0)));

    while (true)
    {
      Id id = table.Insert(x);
      NS_TEST_BOOL(id.m_Generation != 0);

      NS_TEST_BOOL(table.Remove(id));

      if (id.m_Generation == 1) // wrap around
        break;

      ++count2;
    }

    NS_TEST_BOOL(!table.Contains(Id(0, 0)));

    NS_TEST_INT(count1, 32);
    NS_TEST_INT(count2, 2097087);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Insert/Remove")
  {
    nsIdTable<Id, TestObject> table;

    for (int i = 0; i < 100; i++)
    {
      TestObject x = {rand(), "Test"};
      Id id = table.Insert(x);
      NS_TEST_INT(id.m_InstanceIndex, i);
      NS_TEST_INT(id.m_Generation, 1);

      NS_TEST_BOOL(table.Contains(id));

      TestObject y = table[id];
      NS_TEST_INT(x.x, y.x);
      NS_TEST_BOOL(x.s == y.s);
    }
    NS_TEST_INT(table.GetCount(), 100);

    Id ids[10] = {Id(13, 1), Id(0, 1), Id(16, 1), Id(34, 1), Id(56, 1), Id(57, 1), Id(79, 1), Id(85, 1), Id(91, 1), Id(97, 1)};


    for (int i = 0; i < 10; i++)
    {
      bool res = table.Remove(ids[i]);
      NS_TEST_BOOL(res);
      NS_TEST_BOOL(!table.Contains(ids[i]));
    }
    NS_TEST_INT(table.GetCount(), 90);

    for (int i = 0; i < 40; i++)
    {
      TestObject x = {1000, "Bla. This is a very long string which does not fit into 32 byte and will cause memory allocations."};
      Id newId = table.Insert(x);

      NS_TEST_BOOL(table.Contains(newId));

      TestObject y = table[newId];
      NS_TEST_INT(x.x, y.x);
      NS_TEST_BOOL(x.s == y.s);

      TestObject* pObj;
      NS_TEST_BOOL(table.TryGetValue(newId, pObj));
      NS_TEST_BOOL(pObj->s == x.s);
    }
    NS_TEST_INT(table.GetCount(), 130);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Crash test")
  {
    nsIdTable<Id, TestObject> table;
    nsDynamicArray<Id> ids;

    for (nsUInt32 i = 0; i < 100000; ++i)
    {
      int action = rand() % 2;
      if (action == 0)
      {
        TestObject x = {rand(), "Test"};
        ids.PushBack(table.Insert(x));
      }
      else
      {
        if (ids.GetCount() > 0)
        {
          nsUInt32 index = rand() % ids.GetCount();
          NS_TEST_BOOL(table.Remove(ids[index]));
          ids.RemoveAtAndSwap(index);
        }
      }

      NS_TEST_BOOL(table.IsFreelistValid());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Clear")
  {
    NS_TEST_BOOL(st::HasAllDestructed());

    nsIdTable<Id, st> m1;
    Id id0 = m1.Insert(st(1));
    NS_TEST_BOOL(st::HasDone(2, 1)); // for inserting new elements 1 temporary is created (and destroyed)

    Id id1 = m1.Insert(st(3));
    NS_TEST_BOOL(st::HasDone(2, 1)); // for inserting new elements 1 temporary is created (and destroyed)

    m1[id0] = st(2);
    NS_TEST_BOOL(st::HasDone(1, 1)); // nothing new to create, so only the one temporary is used

    m1.Clear();
    NS_TEST_BOOL(st::HasDone(0, 2));
    NS_TEST_BOOL(st::HasAllDestructed());

    NS_TEST_BOOL(!m1.Contains(id0));
    NS_TEST_BOOL(!m1.Contains(id1));
    NS_TEST_BOOL(m1.IsFreelistValid());
  }

  /*NS_TEST_BLOCK(nsTestBlock::Enabled, "Remove/Compact")
  {
    nsIdTable<Id, st> a;

    for (nsInt32 i = 0; i < 1000; ++i)
    {
      a.Insert(i);
      NS_TEST_INT(a.GetCount(), i + 1);
    }

    a.Compact();
    NS_TEST_BOOL(a.IsFreelistValid());

    {
      nsUInt32 i = 0;
      for (nsIdTable<Id, st>::Iterator it = a.GetIterator(); it.IsValid(); ++it)
      {
        NS_TEST_INT(a[it.Id()].m_iData, i);
        ++i;
      }
    }

    for (nsInt32 i = 500; i < 1000; ++i)
    {
      st oldValue;
      NS_TEST_BOOL(a.Remove(Id(i, 0), &oldValue));
      NS_TEST_INT(oldValue.m_iData, i);
    }

    a.Compact();
    NS_TEST_BOOL(a.IsFreelistValid());

    {
      nsUInt32 i = 0;
      for (nsIdTable<Id, st>::Iterator it = a.GetIterator(); it.IsValid(); ++it)
      {
        NS_TEST_INT(a[it.Id()].m_iData, i);
        ++i;
      }
    }
  }*/
}

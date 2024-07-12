#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Containers/List.h>

NS_CREATE_SIMPLE_TEST(Containers, List)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
    nsList<nsInt32> l;
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "PushBack() / PeekBack")
  {
    nsList<nsInt32> l;
    nsInt32& val = l.PushBack();

    NS_TEST_INT(val, 0);
    NS_TEST_INT(l.GetCount(), 1);
    NS_TEST_INT(l.PeekBack(), 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "PushBack(i) / GetCount")
  {
    nsList<nsInt32> l;
    NS_TEST_BOOL(l.GetHeapMemoryUsage() == 0);

    for (nsUInt32 i = 0; i < 1000; ++i)
    {
      l.PushBack(i);

      NS_TEST_INT(l.GetCount(), i + 1);
      NS_TEST_INT(l.PeekBack(), i);
    }

    NS_TEST_BOOL(l.GetHeapMemoryUsage() >= sizeof(nsInt32) * 1000);

    nsUInt32 i = 0;
    for (nsList<nsInt32>::Iterator it = l.GetIterator(); it != l.GetEndIterator(); ++it)
    {
      NS_TEST_INT(*it, i);
      ++i;
    }

    NS_TEST_INT(i, 1000);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "PopBack()")
  {
    nsList<nsInt32> l;

    nsInt32 i = 0;
    for (; i < 1000; ++i)
      l.PushBack(i);

    while (!l.IsEmpty())
    {
      --i;
      NS_TEST_INT(l.PeekBack(), i);
      l.PopBack();
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "PushFront() / PeekFront")
  {
    nsList<nsInt32> l;
    nsInt32& val = l.PushFront();

    NS_TEST_INT(val, 0);
    NS_TEST_INT(l.GetCount(), 1);
    NS_TEST_INT(l.PeekFront(), 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "PushFront(i) / PeekFront")
  {
    nsList<nsInt32> l;

    for (nsUInt32 i = 0; i < 1000; ++i)
    {
      l.PushFront(i);

      NS_TEST_INT(l.GetCount(), i + 1);
      NS_TEST_INT(l.PeekFront(), i);
    }

    nsUInt32 i2 = 1000;
    for (nsList<nsInt32>::Iterator it = l.GetIterator(); it != l.GetEndIterator(); ++it)
    {
      --i2;
      NS_TEST_INT(*it, i2);
    }

    NS_TEST_INT(i2, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "PopFront()")
  {
    nsList<nsInt32> l;

    nsInt32 i = 0;
    for (; i < 1000; ++i)
      l.PushFront(i);

    while (!l.IsEmpty())
    {
      --i;
      NS_TEST_INT(l.PeekFront(), i);
      l.PopFront();
    }
  }


  NS_TEST_BLOCK(nsTestBlock::Enabled, "Clear / IsEmpty")
  {
    nsList<nsInt32> l;

    NS_TEST_BOOL(l.IsEmpty());

    for (nsUInt32 i = 0; i < 1000; ++i)
      l.PushBack(i);

    NS_TEST_BOOL(!l.IsEmpty());

    l.Clear();
    NS_TEST_BOOL(l.IsEmpty());

    for (nsUInt32 i = 0; i < 1000; ++i)
    {
      l.PushBack(i);
      NS_TEST_BOOL(!l.IsEmpty());

      l.Clear();
      NS_TEST_BOOL(l.IsEmpty());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator=")
  {
    nsList<nsInt32> l, l2;

    for (nsUInt32 i = 0; i < 1000; ++i)
      l.PushBack(i);

    l2 = l;

    nsUInt32 i = 0;
    for (nsList<nsInt32>::Iterator it = l2.GetIterator(); it != l2.GetEndIterator(); ++it)
    {
      NS_TEST_INT(*it, i);
      ++i;
    }

    NS_TEST_INT(i, 1000);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Copy Constructor")
  {
    nsList<nsInt32> l;

    for (nsUInt32 i = 0; i < 1000; ++i)
      l.PushBack(i);

    nsList<nsInt32> l2(l);

    nsUInt32 i = 0;
    for (nsList<nsInt32>::Iterator it = l2.GetIterator(); it != l2.GetEndIterator(); ++it)
    {
      NS_TEST_INT(*it, i);
      ++i;
    }

    NS_TEST_INT(i, 1000);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetCount")
  {
    nsList<nsInt32> l;
    l.SetCount(1000);
    NS_TEST_INT(l.GetCount(), 1000);

    nsInt32 i = 1;
    for (nsList<nsInt32>::Iterator it = l.GetIterator(); it != l.GetEndIterator(); ++it)
    {
      NS_TEST_INT(*it, 0);
      *it = i;
      ++i;
    }

    l.SetCount(2000);
    i = 1;
    for (nsList<nsInt32>::Iterator it = l.GetIterator(); it != l.GetEndIterator(); ++it)
    {
      if (i > 1000)
        NS_TEST_INT(*it, 0);
      else
        NS_TEST_INT(*it, i);

      ++i;
    }

    l.SetCount(500);
    i = 1;
    for (nsList<nsInt32>::Iterator it = l.GetIterator(); it != l.GetEndIterator(); ++it)
    {
      NS_TEST_INT(*it, i);
      ++i;
    }

    NS_TEST_INT(i, 501);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Insert(item)")
  {
    nsList<nsInt32> l;

    for (nsUInt32 i = 1; i < 1000; ++i)
      l.PushBack(i);

    // create an interleaved array of values of i and i+10000
    for (nsList<nsInt32>::Iterator it = l.GetIterator(); it != l.GetEndIterator(); ++it)
    {
      // insert before this element
      l.Insert(it, *it + 10000);
    }

    nsInt32 i = 1;
    for (nsList<nsInt32>::Iterator it = l.GetIterator(); it != l.GetEndIterator(); ++it)
    {
      NS_TEST_INT(*it, i + 10000);
      ++it;

      NS_TEST_BOOL(it.IsValid());
      NS_TEST_INT(*it, i);

      ++i;
    }

    NS_TEST_INT(i, 1000);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Remove(item)")
  {
    nsList<nsInt32> l;

    nsUInt32 i = 1;
    for (; i < 1000; ++i)
      l.PushBack(i);

    // create an interleaved array of values of i and i+10000
    for (nsList<nsInt32>::Iterator it = l.GetIterator(); it != l.GetEndIterator(); ++it)
    {
      // insert before this element
      l.Insert(it, *it + 10000);
    }

    i = 1;

    // now remove every second element and only keep the larger values
    for (nsList<nsInt32>::Iterator it = l.GetIterator(); it.IsValid();)
    {
      NS_TEST_INT(*it, i + 10000);

      ++it;
      it = l.Remove(it);
      ++i;
    }

    i = 1;
    for (nsList<nsInt32>::Iterator it = l.GetIterator(); it.IsValid(); ++it)
    {
      NS_TEST_INT(*it, i + 10000);
      ++i;
    }

    NS_TEST_INT(i, 1000);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Iterator::IsValid")
  {
    nsList<nsInt32> l;

    for (nsUInt32 i = 0; i < 1000; ++i)
      l.PushBack(i);

    nsUInt32 i = 0;
    for (nsList<nsInt32>::Iterator it = l.GetIterator(); it.IsValid(); ++it)
    {
      NS_TEST_INT(*it, i);
      ++i;
    }

    NS_TEST_BOOL(!l.GetEndIterator().IsValid());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Element Constructions / Destructions")
  {
    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());

    nsList<nsConstructionCounter> l;

    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());

    l.PushBack();
    NS_TEST_BOOL(nsConstructionCounter::HasDone(1, 0));

    l.PushBack(nsConstructionCounter(1));
    NS_TEST_BOOL(nsConstructionCounter::HasDone(2, 1));

    l.SetCount(4);
    NS_TEST_BOOL(nsConstructionCounter::HasDone(2, 0));

    l.Clear();
    NS_TEST_BOOL(nsConstructionCounter::HasDone(0, 4));

    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator == / !=")
  {
    nsList<nsInt32> l, l2;

    NS_TEST_BOOL(l == l2);

    nsInt32 i = 0;
    for (; i < 1000; ++i)
      l.PushBack(i);

    NS_TEST_BOOL(l != l2);

    l2 = l;

    NS_TEST_BOOL(l == l2);
  }
}

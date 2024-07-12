#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Containers/StaticRingBuffer.h>

using cc = nsConstructionCounter;

NS_CREATE_SIMPLE_TEST(Containers, StaticRingBuffer)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor")
  {
    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());

    {
      nsStaticRingBuffer<nsInt32, 32> r1;
      nsStaticRingBuffer<nsInt32, 16> r2;
      nsStaticRingBuffer<cc, 2> r3;
    }

    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Copy Constructor / Operator=")
  {
    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());

    {
      nsStaticRingBuffer<cc, 16> r1;

      for (nsUInt32 i = 0; i < 16; ++i)
        r1.PushBack(cc(i));

      nsStaticRingBuffer<cc, 16> r2(r1);

      for (nsUInt32 i = 0; i < 16; ++i)
        NS_TEST_BOOL(r2[i] == cc(i));

      nsStaticRingBuffer<cc, 16> r3;
      r3 = r1;

      for (nsUInt32 i = 0; i < 16; ++i)
        NS_TEST_BOOL(r3[i] == cc(i));
    }

    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Operator==")
  {
    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());

    {
      nsStaticRingBuffer<cc, 16> r1;

      for (nsUInt32 i = 0; i < 16; ++i)
        r1.PushBack(cc(i));

      nsStaticRingBuffer<cc, 16> r2(r1);
      nsStaticRingBuffer<cc, 16> r3(r1);
      r3.PeekFront() = cc(3);

      NS_TEST_BOOL(r1 == r1);
      NS_TEST_BOOL(r2 == r2);
      NS_TEST_BOOL(r3 == r3);

      NS_TEST_BOOL(r1 == r2);
      NS_TEST_BOOL(r1 != r3);
      NS_TEST_BOOL(r2 != r3);
    }

    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "PushBack / operator[] / CanAppend")
  {
    nsStaticRingBuffer<nsInt32, 16> r;

    for (nsUInt32 i = 0; i < 16; ++i)
    {
      NS_TEST_BOOL(r.CanAppend());
      r.PushBack(i);
    }

    NS_TEST_BOOL(!r.CanAppend());

    for (nsUInt32 i = 0; i < 16; ++i)
      NS_TEST_INT(r[i], i);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetCount / IsEmpty")
  {
    nsStaticRingBuffer<nsInt32, 16> r;

    NS_TEST_BOOL(r.IsEmpty());

    for (nsUInt32 i = 0; i < 16; ++i)
    {
      NS_TEST_INT(r.GetCount(), i);
      r.PushBack(i);
      NS_TEST_INT(r.GetCount(), i + 1);

      NS_TEST_BOOL(!r.IsEmpty());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Clear / IsEmpty")
  {
    nsStaticRingBuffer<nsInt32, 16> r;

    NS_TEST_BOOL(r.IsEmpty());

    for (nsUInt32 i = 0; i < 16; ++i)
      r.PushBack(i);

    NS_TEST_BOOL(!r.IsEmpty());

    r.Clear();

    NS_TEST_BOOL(r.IsEmpty());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Cycle Items / PeekFront")
  {
    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());

    {
      nsStaticRingBuffer<nsConstructionCounter, 16> r;

      for (nsUInt32 i = 0; i < 16; ++i)
      {
        r.PushBack(nsConstructionCounter(i));
        NS_TEST_BOOL(nsConstructionCounter::HasDone(2, 1)); // one temporary
      }

      for (nsUInt32 i = 16; i < 1000; ++i)
      {
        NS_TEST_BOOL(r.PeekFront() == nsConstructionCounter(i - 16));
        NS_TEST_BOOL(nsConstructionCounter::HasDone(1, 1)); // one temporary

        NS_TEST_BOOL(!r.CanAppend());

        r.PopFront();
        NS_TEST_BOOL(nsConstructionCounter::HasDone(0, 1));

        NS_TEST_BOOL(r.CanAppend());

        r.PushBack(nsConstructionCounter(i));
        NS_TEST_BOOL(nsConstructionCounter::HasDone(2, 1)); // one temporary
      }

      for (nsUInt32 i = 1000; i < 1016; ++i)
      {
        NS_TEST_BOOL(r.PeekFront() == nsConstructionCounter(i - 16));
        NS_TEST_BOOL(nsConstructionCounter::HasDone(1, 1)); // one temporary

        r.PopFront();
        NS_TEST_BOOL(nsConstructionCounter::HasDone(0, 1)); // one temporary
      }

      NS_TEST_BOOL(r.IsEmpty());
    }

    NS_TEST_BOOL(nsConstructionCounter::HasAllDestructed());
  }
}

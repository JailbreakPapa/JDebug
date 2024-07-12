#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Communication/Event.h>

NS_CREATE_SIMPLE_TEST_GROUP(Communication);

namespace
{
  struct Test
  {
    void DoStuff(nsInt32* pEventData) { *pEventData += m_iData; }

    nsInt32 m_iData;
  };

  struct TestRecursion
  {
    TestRecursion() { m_uiRecursionCount = 0; }
    void DoStuff(nsUInt32 uiRecursions)
    {
      if (m_uiRecursionCount < uiRecursions)
      {
        m_uiRecursionCount++;
        m_Event.Broadcast(uiRecursions, 10);
      }
    }

    using Event = nsEvent<nsUInt32>;
    Event m_Event;
    nsUInt32 m_uiRecursionCount;
  };
} // namespace

NS_CREATE_SIMPLE_TEST(Communication, Event)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Basics")
  {
    using TestEvent = nsEvent<nsInt32*>;
    TestEvent e;

    Test test1;
    test1.m_iData = 3;

    Test test2;
    test2.m_iData = 5;

    nsInt32 iResult = 0;

    e.AddEventHandler(TestEvent::Handler(&Test::DoStuff, &test1));
    NS_TEST_BOOL(e.HasEventHandler(TestEvent::Handler(&Test::DoStuff, &test1)));

    iResult = 0;
    e.Broadcast(&iResult);

    NS_TEST_INT(iResult, 3);

    e.AddEventHandler(TestEvent::Handler(&Test::DoStuff, &test2));
    NS_TEST_BOOL(e.HasEventHandler(TestEvent::Handler(&Test::DoStuff, &test2)));

    iResult = 0;
    e.Broadcast(&iResult);

    NS_TEST_INT(iResult, 8);

    e.RemoveEventHandler(TestEvent::Handler(&Test::DoStuff, &test1));
    NS_TEST_BOOL(!e.HasEventHandler(TestEvent::Handler(&Test::DoStuff, &test1)));

    iResult = 0;
    e.Broadcast(&iResult);

    NS_TEST_INT(iResult, 5);

    e.RemoveEventHandler(TestEvent::Handler(&Test::DoStuff, &test2));
    NS_TEST_BOOL(!e.HasEventHandler(TestEvent::Handler(&Test::DoStuff, &test2)));

    iResult = 0;
    e.Broadcast(&iResult);

    NS_TEST_INT(iResult, 0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Unsubscribing via ID")
  {
    using TestEvent = nsEvent<nsInt32*>;
    TestEvent e;

    Test test1;
    Test test2;

    auto subId1 = e.AddEventHandler(TestEvent::Handler(&Test::DoStuff, &test1));
    NS_TEST_BOOL(e.HasEventHandler(TestEvent::Handler(&Test::DoStuff, &test1)));

    auto subId2 = e.AddEventHandler(TestEvent::Handler(&Test::DoStuff, &test2));
    NS_TEST_BOOL(e.HasEventHandler(TestEvent::Handler(&Test::DoStuff, &test2)));

    e.RemoveEventHandler(subId1);
    NS_TEST_BOOL(!e.HasEventHandler(TestEvent::Handler(&Test::DoStuff, &test1)));

    e.RemoveEventHandler(subId2);
    NS_TEST_BOOL(!e.HasEventHandler(TestEvent::Handler(&Test::DoStuff, &test2)));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Unsubscribing via Unsubscriber")
  {
    using TestEvent = nsEvent<nsInt32*>;
    TestEvent e;

    Test test1;
    Test test2;

    {
      TestEvent::Unsubscriber unsub1;

      {
        TestEvent::Unsubscriber unsub2;

        e.AddEventHandler(TestEvent::Handler(&Test::DoStuff, &test1), unsub1);
        NS_TEST_BOOL(e.HasEventHandler(TestEvent::Handler(&Test::DoStuff, &test1)));

        e.AddEventHandler(TestEvent::Handler(&Test::DoStuff, &test2), unsub2);
        NS_TEST_BOOL(e.HasEventHandler(TestEvent::Handler(&Test::DoStuff, &test2)));
      }

      NS_TEST_BOOL(!e.HasEventHandler(TestEvent::Handler(&Test::DoStuff, &test2)));
    }

    NS_TEST_BOOL(!e.HasEventHandler(TestEvent::Handler(&Test::DoStuff, &test1)));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Recursion")
  {
    for (nsUInt32 i = 0; i < 10; i++)
    {
      TestRecursion test;
      test.m_Event.AddEventHandler(TestRecursion::Event::Handler(&TestRecursion::DoStuff, &test));
      test.m_Event.Broadcast(i, 10);
      NS_TEST_INT(test.m_uiRecursionCount, i);
      test.m_Event.RemoveEventHandler(TestRecursion::Event::Handler(&TestRecursion::DoStuff, &test));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Remove while iterate")
  {
    using TestEvent = nsEvent<int, nsMutex, nsDefaultAllocatorWrapper, nsEventType::CopyOnBroadcast>;
    TestEvent e;

    nsUInt32 callMap = 0;

    nsEventSubscriptionID subscriptions[4] = {};

    subscriptions[0] = e.AddEventHandler(TestEvent::Handler([&](int i)
      { callMap |= NS_BIT(0); }));

    subscriptions[1] = e.AddEventHandler(TestEvent::Handler([&](int i)
      {
      callMap |= NS_BIT(1);
      e.RemoveEventHandler(subscriptions[1]); }));

    subscriptions[2] = e.AddEventHandler(TestEvent::Handler([&](int i)
      {
      callMap |= NS_BIT(2);
      e.RemoveEventHandler(subscriptions[2]);
      e.RemoveEventHandler(subscriptions[3]); }));

    subscriptions[3] = e.AddEventHandler(TestEvent::Handler([&](int i)
      { callMap |= NS_BIT(3); }));

    e.Broadcast(0);

    NS_TEST_BOOL(callMap == (NS_BIT(0) | NS_BIT(1) | NS_BIT(2) | NS_BIT(3)));

    callMap = 0;
    e.Broadcast(0);
    NS_TEST_BOOL(callMap == NS_BIT(0));

    e.RemoveEventHandler(subscriptions[0]);
  }
}

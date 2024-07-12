#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Communication/MessageQueue.h>

namespace
{
  struct nsMsgTest : public nsMessage
  {
    NS_DECLARE_MESSAGE_TYPE(nsMsgTest, nsMessage);
  };

  NS_IMPLEMENT_MESSAGE_TYPE(nsMsgTest);
  NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgTest, 1, nsRTTIDefaultAllocator<nsMsgTest>)
  NS_END_DYNAMIC_REFLECTED_TYPE;

  struct TestMessage : public nsMsgTest
  {
    NS_DECLARE_MESSAGE_TYPE(TestMessage, nsMsgTest);

    int x;
    int y;
  };

  struct MetaData
  {
    int receiver;
  };

  using TestMessageQueue = nsMessageQueue<MetaData>;

  NS_IMPLEMENT_MESSAGE_TYPE(TestMessage);
  NS_BEGIN_DYNAMIC_REFLECTED_TYPE(TestMessage, 1, nsRTTIDefaultAllocator<TestMessage>)
  NS_END_DYNAMIC_REFLECTED_TYPE;
} // namespace

NS_CREATE_SIMPLE_TEST(Communication, MessageQueue)
{
  {
    TestMessage msg;
    NS_TEST_INT(msg.GetSize(), sizeof(TestMessage));
  }

  TestMessageQueue q;

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Enqueue")
  {
    for (nsUInt32 i = 0; i < 100; ++i)
    {
      TestMessage* pMsg = NS_DEFAULT_NEW(TestMessage);
      pMsg->x = rand();
      pMsg->y = rand();

      MetaData md;
      md.receiver = rand() % 10;

      q.Enqueue(pMsg, md);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Sorting")
  {
    struct MessageComparer
    {
      bool Less(const TestMessageQueue::Entry& a, const TestMessageQueue::Entry& b) const
      {
        if (a.m_MetaData.receiver != b.m_MetaData.receiver)
          return a.m_MetaData.receiver < b.m_MetaData.receiver;

        return a.m_pMessage->GetHash() < b.m_pMessage->GetHash();
      }
    };

    q.Sort(MessageComparer());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "operator[]")
  {
    NS_LOCK(q);

    nsMessage* pLastMsg = q[0].m_pMessage;
    MetaData lastMd = q[0].m_MetaData;

    for (nsUInt32 i = 1; i < q.GetCount(); ++i)
    {
      nsMessage* pMsg = q[i].m_pMessage;
      MetaData md = q[i].m_MetaData;

      if (md.receiver == lastMd.receiver)
      {
        NS_TEST_BOOL(pMsg->GetHash() >= pLastMsg->GetHash());
      }
      else
      {
        NS_TEST_BOOL(md.receiver >= lastMd.receiver);
      }
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Dequeue")
  {
    nsMessage* pMsg = nullptr;
    MetaData md;

    while (q.TryDequeue(pMsg, md))
    {
      NS_DEFAULT_DELETE(pMsg);
    }
  }
}

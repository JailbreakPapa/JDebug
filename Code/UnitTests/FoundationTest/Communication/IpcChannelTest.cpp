#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Communication/IpcChannel.h>
#include <Foundation/Time/Stopwatch.h>
#include <optional>

#if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP) || NS_ENABLED(NS_PLATFORM_LINUX)

class ChannelTester
{
public:
  ChannelTester(nsIpcChannel* pChannel, bool bPing)
  {
    m_bPing = bPing;
    m_pChannel = pChannel;
    m_pChannel->SetReceiveCallback(nsMakeDelegate(&ChannelTester::ReceiveMessageData, this));
    m_pChannel->m_Events.AddEventHandler(nsMakeDelegate(&ChannelTester::OnIpcEventReceived, this));
  }
  ~ChannelTester()
  {
    m_pChannel->m_Events.RemoveEventHandler(nsMakeDelegate(&ChannelTester::OnIpcEventReceived, this));
    m_pChannel->SetReceiveCallback({});
  }

  void OnIpcEventReceived(const nsIpcChannelEvent& e)
  {
    NS_LOCK(m_Mutex);
    m_ReceivedEvents.ExpandAndGetRef() = e;
  }

  std::optional<nsIpcChannelEvent> WaitForEvents(nsTime timeout)
  {
    nsStopwatch sw;

    while (sw.GetRunningTotal() < timeout)
    {
      nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));
      NS_LOCK(m_Mutex);
      if (!m_ReceivedEvents.IsEmpty())
      {
        nsIpcChannelEvent e = m_ReceivedEvents.PeekFront();
        m_ReceivedEvents.PopFront();
        return e;
      }
    }

    return {};
  }

  void ReceiveMessageData(nsArrayPtr<const nsUInt8> data)
  {
    NS_LOCK(m_Mutex);
    if (m_bPing)
    {
      m_pChannel->Send(data);
    }
    else
    {
      m_ReceivedMessages.ExpandAndGetRef() = data;
    }
  }

  std::optional<nsDynamicArray<nsUInt8>> WaitForMessage(nsTime timeout)
  {
    nsResult res = m_pChannel->WaitForMessages(timeout);
    if (res.Succeeded())
    {
      NS_LOCK(m_Mutex);
      if (m_ReceivedMessages.GetCount() > 0)
      {
        auto res2 = m_ReceivedMessages.PeekFront();
        m_ReceivedMessages.PopFront();
        return res2;
      }
    }
    return {};
  }

private:
  bool m_bPing = false;
  nsMutex m_Mutex;
  nsIpcChannel* m_pChannel = nullptr;
  nsDeque<nsDynamicArray<nsUInt8>> m_ReceivedMessages;
  nsDeque<nsIpcChannelEvent> m_ReceivedEvents;
};

void TestIPCChannel(nsIpcChannel* pServer, ChannelTester* pServerTester, nsIpcChannel* pClient, ChannelTester* pClientTester)
{
  auto MessageMatches = [](const nsStringView& sReference, const nsDataBuffer& msg) -> bool
  {
    nsStringView sTemp(reinterpret_cast<const char*>(msg.GetData()), msg.GetCount());
    return sTemp == sReference;
  };

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Connect")
  {
    NS_TEST_BOOL(pServer->GetConnectionState() == nsIpcChannel::ConnectionState::Disconnected);
    NS_TEST_BOOL(pClient->GetConnectionState() == nsIpcChannel::ConnectionState::Disconnected);
    {
      auto res = pServerTester->WaitForEvents(nsTime::MakeFromMilliseconds(100));
      NS_TEST_BOOL(!res.has_value());
      auto res2 = pClientTester->WaitForEvents(nsTime::MakeFromMilliseconds(100));
      NS_TEST_BOOL(!res2.has_value());
    }
    {
      pServer->Connect();
      auto res = pServerTester->WaitForEvents(nsTime::MakeFromMilliseconds(100));
      NS_TEST_BOOL(res.has_value() && res->m_Type == nsIpcChannelEvent::Connecting);
      NS_TEST_BOOL(pServer->GetConnectionState() == nsIpcChannel::ConnectionState::Connecting);
    }
    {
      pClient->Connect();
      auto res = pClientTester->WaitForEvents(nsTime::MakeFromMilliseconds(100));
      NS_TEST_BOOL(res.has_value() && res->m_Type == nsIpcChannelEvent::Connecting);
    }
    auto res = pServerTester->WaitForEvents(nsTime::MakeFromSeconds(100));
    NS_TEST_BOOL(res.has_value() && res->m_Type == nsIpcChannelEvent::Connected);
    auto res2 = pClientTester->WaitForEvents(nsTime::MakeFromSeconds(100));
    NS_TEST_BOOL(res2.has_value() && res2->m_Type == nsIpcChannelEvent::Connected);

    NS_TEST_BOOL(pServer->GetConnectionState() == nsIpcChannel::ConnectionState::Connected);
    NS_TEST_BOOL(pClient->GetConnectionState() == nsIpcChannel::ConnectionState::Connected);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Connect When Already Connected")
  {
    pServer->Connect();
    pClient->Connect();
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ClientSend")
  {
    nsStringView sMsg = "TestMessage"_nssv;

    NS_TEST_BOOL(pClient->Send(nsConstByteArrayPtr(reinterpret_cast<const nsUInt8*>(sMsg.GetStartPointer()), sMsg.GetElementCount())));

    auto res = pServerTester->WaitForEvents(nsTime::MakeFromSeconds(1));
    NS_TEST_BOOL(res.has_value() && res->m_Type == nsIpcChannelEvent::NewMessages);
    auto res2 = pClientTester->WaitForEvents(nsTime::MakeFromSeconds(1));
    NS_TEST_BOOL(res2.has_value() && res2->m_Type == nsIpcChannelEvent::NewMessages);

    auto res3 = pClientTester->WaitForMessage(nsTime::MakeFromSeconds(1));
    NS_TEST_BOOL(res3.has_value() && MessageMatches(sMsg, res3.value()));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ServerSend")
  {
    nsStringView sMsg = "TestMessage2"_nssv;

    NS_TEST_BOOL(pServer->Send(nsConstByteArrayPtr(reinterpret_cast<const nsUInt8*>(sMsg.GetStartPointer()), sMsg.GetElementCount())));

    auto res2 = pClientTester->WaitForEvents(nsTime::MakeFromSeconds(1));
    NS_TEST_BOOL(res2.has_value() && res2->m_Type == nsIpcChannelEvent::NewMessages);

    auto res3 = pClientTester->WaitForMessage(nsTime::MakeFromSeconds(1));
    NS_TEST_BOOL(res3.has_value() && MessageMatches(sMsg, res3.value()));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ClientDisconnect")
  {
    pClient->Disconnect();
    pClient->Disconnect();

    auto res = pServerTester->WaitForEvents(nsTime::MakeFromSeconds(1));
    NS_TEST_BOOL(res.has_value() && res->m_Type == nsIpcChannelEvent::Disconnected);
    auto res2 = pClientTester->WaitForEvents(nsTime::MakeFromSeconds(1));
    NS_TEST_BOOL(res2.has_value() && res2->m_Type == nsIpcChannelEvent::Disconnected);

    NS_TEST_BOOL(pServer->GetConnectionState() == nsIpcChannel::ConnectionState::Disconnected);
    NS_TEST_BOOL(pClient->GetConnectionState() == nsIpcChannel::ConnectionState::Disconnected);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Reconnect")
  {
    {
      pServer->Connect();
      auto res = pServerTester->WaitForEvents(nsTime::MakeFromMilliseconds(100));
      NS_TEST_BOOL(res.has_value() && res->m_Type == nsIpcChannelEvent::Connecting);
      NS_TEST_BOOL(pServer->GetConnectionState() == nsIpcChannel::ConnectionState::Connecting);
    }
    {
      pClient->Connect();
      auto res = pClientTester->WaitForEvents(nsTime::MakeFromMilliseconds(100));
      NS_TEST_BOOL(res.has_value() && res->m_Type == nsIpcChannelEvent::Connecting);
    }

    auto res = pServerTester->WaitForEvents(nsTime::MakeFromSeconds(1));
    NS_TEST_BOOL(res.has_value() && res->m_Type == nsIpcChannelEvent::Connected);
    auto res2 = pClientTester->WaitForEvents(nsTime::MakeFromSeconds(1));
    NS_TEST_BOOL(res2.has_value() && res2->m_Type == nsIpcChannelEvent::Connected);

    NS_TEST_BOOL(pServer->GetConnectionState() == nsIpcChannel::ConnectionState::Connected);
    NS_TEST_BOOL(pClient->GetConnectionState() == nsIpcChannel::ConnectionState::Connected);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ClientSend after reconnect")
  {
    nsStringView sMsg = "TestMessage"_nssv;

    NS_TEST_BOOL(pClient->Send(nsConstByteArrayPtr(reinterpret_cast<const nsUInt8*>(sMsg.GetStartPointer()), sMsg.GetElementCount())));

    auto res = pServerTester->WaitForEvents(nsTime::MakeFromSeconds(1));
    NS_TEST_BOOL(res.has_value() && res->m_Type == nsIpcChannelEvent::NewMessages);
    auto res2 = pClientTester->WaitForEvents(nsTime::MakeFromSeconds(1));
    NS_TEST_BOOL(res2.has_value() && res2->m_Type == nsIpcChannelEvent::NewMessages);

    auto res3 = pClientTester->WaitForMessage(nsTime::MakeFromSeconds(1));
    NS_TEST_BOOL(res3.has_value() && MessageMatches(sMsg, res3.value()));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ServerDisconnect")
  {
    pServer->Disconnect();
    pServer->Disconnect();

    auto res = pServerTester->WaitForEvents(nsTime::MakeFromSeconds(1));
    NS_TEST_BOOL(res.has_value() && res->m_Type == nsIpcChannelEvent::Disconnected);
    auto res2 = pClientTester->WaitForEvents(nsTime::MakeFromSeconds(1));
    NS_TEST_BOOL(res2.has_value() && res2->m_Type == nsIpcChannelEvent::Disconnected);

    NS_TEST_BOOL(pServer->GetConnectionState() == nsIpcChannel::ConnectionState::Disconnected);
    NS_TEST_BOOL(pClient->GetConnectionState() == nsIpcChannel::ConnectionState::Disconnected);
  }
}

/* TODO: Enet does not connect in process.
NS_CREATE_SIMPLE_TEST(Communication, IpcChannel_Network)
{
  nsUniquePtr<nsIpcChannel> pServer = nsIpcChannel::CreateNetworkChannel("127.0.0.1:1050"_nssv, nsIpcChannel::Mode::Server);
  nsUniquePtr<ChannelTester> pServerTester = NS_DEFAULT_NEW(ChannelTester, pServer.Borrow(), true);

  nsUniquePtr<nsIpcChannel> pClient = nsIpcChannel::CreateNetworkChannel("127.0.0.1:1050"_nssv, nsIpcChannel::Mode::Client);
  nsUniquePtr<ChannelTester> pClientTester = NS_DEFAULT_NEW(ChannelTester, pClient.Borrow(), false);

  TestIPCChannel(pServer.Borrow(), pServerTester.Borrow(), pClient.Borrow(), pClientTester.Borrow());

  pClientTester.Clear();
  pClient.Clear();

  pServerTester.Clear();
  pServer.Clear();
}
*/

NS_CREATE_SIMPLE_TEST(Communication, IpcChannel_Pipe)
{
  nsUniquePtr<nsIpcChannel> pServer = nsIpcChannel::CreatePipeChannel("nsEngine_unit_test_channel", nsIpcChannel::Mode::Server);
  nsUniquePtr<ChannelTester> pServerTester = NS_DEFAULT_NEW(ChannelTester, pServer.Borrow(), true);

  nsUniquePtr<nsIpcChannel> pClient = nsIpcChannel::CreatePipeChannel("nsEngine_unit_test_channel", nsIpcChannel::Mode::Client);
  nsUniquePtr<ChannelTester> pClientTester = NS_DEFAULT_NEW(ChannelTester, pClient.Borrow(), false);

  TestIPCChannel(pServer.Borrow(), pServerTester.Borrow(), pClient.Borrow(), pClientTester.Borrow());

  pClientTester.Clear();
  pClient.Clear();

  pServerTester.Clear();
  pServer.Clear();
}

#endif

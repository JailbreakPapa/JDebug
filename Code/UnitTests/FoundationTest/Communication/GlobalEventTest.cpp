#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Communication/GlobalEvent.h>
#include <Foundation/Logging/ConsoleWriter.h>
#include <Foundation/Logging/Log.h>

static nsInt32 iTestData1 = 0;
static nsInt32 iTestData2 = 0;

// The following event handlers are automatically registered, nothing else needs to be done here

NS_ON_GLOBAL_EVENT(TestGlobalEvent1)
{
  iTestData1 += param0.Get<nsInt32>();
}

NS_ON_GLOBAL_EVENT(TestGlobalEvent2)
{
  iTestData2 += param0.Get<nsInt32>();
}

NS_ON_GLOBAL_EVENT_ONCE(TestGlobalEvent3)
{
  // this handler will be executed only once, even if the event is broadcast multiple times
  iTestData2 += 42;
}

static bool g_bFirstRun = true;

NS_CREATE_SIMPLE_TEST(Communication, GlobalEvent)
{
  iTestData1 = 0;
  iTestData2 = 0;

  NS_TEST_INT(iTestData1, 0);
  NS_TEST_INT(iTestData2, 0);

  nsGlobalEvent::Broadcast("TestGlobalEvent1", 1);

  NS_TEST_INT(iTestData1, 1);
  NS_TEST_INT(iTestData2, 0);

  nsGlobalEvent::Broadcast("TestGlobalEvent1", 2);

  NS_TEST_INT(iTestData1, 3);
  NS_TEST_INT(iTestData2, 0);

  nsGlobalEvent::Broadcast("TestGlobalEvent1", 3);

  NS_TEST_INT(iTestData1, 6);
  NS_TEST_INT(iTestData2, 0);

  nsGlobalEvent::Broadcast("TestGlobalEvent2", 4);

  NS_TEST_INT(iTestData1, 6);
  NS_TEST_INT(iTestData2, 4);

  nsGlobalEvent::Broadcast("TestGlobalEvent3", 4);

  NS_TEST_INT(iTestData1, 6);

  if (g_bFirstRun)
  {
    g_bFirstRun = false;
    NS_TEST_INT(iTestData2, 46);
  }
  else
  {
    NS_TEST_INT(iTestData2, 4);
    iTestData2 += 42;
  }

  nsGlobalEvent::Broadcast("TestGlobalEvent2", 5);

  NS_TEST_INT(iTestData1, 6);
  NS_TEST_INT(iTestData2, 51);

  nsGlobalEvent::Broadcast("TestGlobalEvent3", 4);

  NS_TEST_INT(iTestData1, 6);
  NS_TEST_INT(iTestData2, 51);

  nsGlobalEvent::Broadcast("TestGlobalEvent2", 6);

  NS_TEST_INT(iTestData1, 6);
  NS_TEST_INT(iTestData2, 57);

  nsGlobalEvent::Broadcast("TestGlobalEvent3", 4);

  NS_TEST_INT(iTestData1, 6);
  NS_TEST_INT(iTestData2, 57);

  nsGlobalLog::AddLogWriter(nsLogWriter::Console::LogMessageHandler);

  nsGlobalEvent::PrintGlobalEventStatistics();

  nsGlobalLog::RemoveLogWriter(nsLogWriter::Console::LogMessageHandler);
}

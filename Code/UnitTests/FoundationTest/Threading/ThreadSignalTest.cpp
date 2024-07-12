#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Threading/Thread.h>
#include <Foundation/Threading/ThreadSignal.h>
#include <Foundation/Types/UniquePtr.h>

namespace
{
  class TestThread2 : public nsThread
  {
  public:
    TestThread2()
      : nsThread("Test Thread")
    {
    }

    nsThreadSignal* m_pSignalAuto = nullptr;
    nsThreadSignal* m_pSignalManual = nullptr;
    nsAtomicInteger32* m_pCounter = nullptr;
    bool m_bTimeout = false;

    virtual nsUInt32 Run()
    {
      m_pCounter->Decrement();

      m_pSignalAuto->WaitForSignal();

      m_pCounter->Increment();

      if (m_bTimeout)
      {
        m_pSignalManual->WaitForSignal(nsTime::MakeFromSeconds(0.5));
      }
      else
      {
        m_pSignalManual->WaitForSignal();
      }

      m_pCounter->Increment();

      return 0;
    }
  };
} // namespace

NS_CREATE_SIMPLE_TEST(Threading, ThreadSignal)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Wait No Timeout")
  {
    constexpr nsUInt32 uiNumThreads = 32;

    nsUniquePtr<TestThread2> pTestThread2s[uiNumThreads];
    nsAtomicInteger32 iCounter = uiNumThreads;
    nsThreadSignal sigAuto(nsThreadSignal::Mode::AutoReset);
    nsThreadSignal sigManual(nsThreadSignal::Mode::ManualReset);

    for (nsUInt32 i = 0; i < uiNumThreads; ++i)
    {
      pTestThread2s[i] = NS_DEFAULT_NEW(TestThread2);
      pTestThread2s[i]->m_pCounter = &iCounter;
      pTestThread2s[i]->m_pSignalAuto = &sigAuto;
      pTestThread2s[i]->m_pSignalManual = &sigManual;
      pTestThread2s[i]->Start();
    }

    // wait until all threads are in waiting state
    while (iCounter > 0)
    {
      nsThreadUtils::YieldTimeSlice();
    }

    for (nsUInt32 t = 0; t < uiNumThreads; ++t)
    {
      const nsInt32 iExpected = t + 1;

      sigAuto.RaiseSignal();

      for (nsUInt32 a = 0; a < 1000; ++a)
      {
        nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(1));

        if (iCounter >= iExpected)
          break;
      }

      // theoretically this could fail, if the OS doesn't wake up any other thread in time
      // but with 1000 tries that is very unlikely
      NS_TEST_INT(iCounter, iExpected);
      NS_TEST_BOOL(iCounter <= iExpected); // THIS test must never fail!
    }

    // wake up the rest
    {
      sigManual.RaiseSignal();

      for (nsUInt32 a = 0; a < 1000; ++a)
      {
        nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(1));

        if (iCounter >= (nsInt32)uiNumThreads * 2)
          break;
      }

      // theoretically this could fail, if the OS doesn't wake up any other thread in time
      // but with 1000 tries that is very unlikely
      NS_TEST_INT(iCounter, (nsInt32)uiNumThreads * 2);
      NS_TEST_BOOL(iCounter <= (nsInt32)uiNumThreads * 2); // THIS test must never fail!
    }

    for (nsUInt32 i = 0; i < uiNumThreads; ++i)
    {
      pTestThread2s[i]->Join();
    }
  }


  NS_TEST_BLOCK(nsTestBlock::Enabled, "Wait With Timeout")
  {
    constexpr nsUInt32 uiNumThreads = 16;

    nsUniquePtr<TestThread2> pTestThread2s[uiNumThreads];
    nsAtomicInteger32 iCounter = uiNumThreads;
    nsThreadSignal sigAuto(nsThreadSignal::Mode::AutoReset);
    nsThreadSignal sigManual(nsThreadSignal::Mode::ManualReset);

    for (nsUInt32 i = 0; i < uiNumThreads; ++i)
    {
      pTestThread2s[i] = NS_DEFAULT_NEW(TestThread2);
      pTestThread2s[i]->m_pCounter = &iCounter;
      pTestThread2s[i]->m_pSignalAuto = &sigAuto;
      pTestThread2s[i]->m_pSignalManual = &sigManual;
      pTestThread2s[i]->m_bTimeout = true;
      pTestThread2s[i]->Start();
    }

    // wait until all threads are in waiting state
    while (iCounter > 0)
    {
      nsThreadUtils::YieldTimeSlice();
    }

    // raise the signal N times
    for (nsUInt32 t = 0; t < uiNumThreads; ++t)
    {
      sigAuto.RaiseSignal();

      for (nsUInt32 a = 0; a < 1000; ++a)
      {
        nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(1));

        if (iCounter >= (nsInt32)t + 1)
          break;
      }
    }

    // due to the wait timeout in the thread, testing this exact value here would be unreliable
    // NS_TEST_INT(iCounter, (nsInt32)uiNumThreads);

    // just wait for the rest
    {
      for (nsUInt32 a = 0; a < 100; ++a)
      {
        nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(50));

        if (iCounter >= (nsInt32)uiNumThreads * 2)
          break;
      }

      // theoretically this could fail, if the OS doesn't wake up any other thread in time
      // but with 1000 tries that is very unlikely
      NS_TEST_INT(iCounter, (nsInt32)uiNumThreads * 2);
      NS_TEST_BOOL(iCounter <= (nsInt32)uiNumThreads * 2); // THIS test must never fail!
    }

    for (nsUInt32 i = 0; i < uiNumThreads; ++i)
    {
      pTestThread2s[i]->Join();
    }
  }
}

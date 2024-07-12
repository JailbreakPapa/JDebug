#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Threading/ConditionVariable.h>

#include <Foundation/Threading/Mutex.h>
#include <Foundation/Threading/Thread.h>
#include <Foundation/Types/UniquePtr.h>

namespace
{
  class TestThread : public nsThread
  {
  public:
    TestThread()
      : nsThread("Test Thread")
    {
    }

    nsConditionVariable* m_pCV = nullptr;
    nsAtomicInteger32* m_pCounter = nullptr;

    virtual nsUInt32 Run()
    {
      NS_LOCK(*m_pCV);

      m_pCounter->Decrement();

      m_pCV->UnlockWaitForSignalAndLock();

      m_pCounter->Increment();
      return 0;
    }
  };

  class TestThreadTimeout : public nsThread
  {
  public:
    TestThreadTimeout()
      : nsThread("Test Thread Timeout")
    {
    }

    nsConditionVariable* m_pCV = nullptr;
    nsConditionVariable* m_pCVTimeout = nullptr;
    nsAtomicInteger32* m_pCounter = nullptr;

    virtual nsUInt32 Run()
    {
      // make sure all threads are put to sleep first
      {
        NS_LOCK(*m_pCV);
        m_pCounter->Decrement();
        m_pCV->UnlockWaitForSignalAndLock();
      }

      // this condition will never be met during the test
      // it should always run into the timeout
      NS_LOCK(*m_pCVTimeout);
      m_pCVTimeout->UnlockWaitForSignalAndLock(nsTime::MakeFromSeconds(0.5));

      m_pCounter->Increment();
      return 0;
    }
  };
} // namespace

NS_CREATE_SIMPLE_TEST(Threading, ConditionalVariable)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Wait No Timeout")
  {
    constexpr nsUInt32 uiNumThreads = 32;

    nsUniquePtr<TestThread> pTestThreads[uiNumThreads];
    nsAtomicInteger32 iCounter = uiNumThreads;
    nsConditionVariable cv;

    for (nsUInt32 i = 0; i < uiNumThreads; ++i)
    {
      pTestThreads[i] = NS_DEFAULT_NEW(TestThread);
      pTestThreads[i]->m_pCounter = &iCounter;
      pTestThreads[i]->m_pCV = &cv;
      pTestThreads[i]->Start();
    }

    // wait until all threads are in waiting state
    while (true)
    {
      // We need to lock here as otherwise we could signal
      // while a thread hasn't reached the wait yet.
      NS_LOCK(cv);
      if (iCounter == 0)
        break;

      nsThreadUtils::YieldTimeSlice();
    }

    for (nsUInt32 t = 0; t < uiNumThreads / 2; ++t)
    {
      const nsInt32 iExpected = iCounter + 1;

      cv.SignalOne();

      for (nsUInt32 a = 0; a < 1000; ++a)
      {
        nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(1));

        if (iCounter >= iExpected)
          break;
      }

      // Theoretically this could fail, if the OS doesn't wake up any other thread in time but with 1000 tries that is very unlikely.
      // On some platforms like posix it is not guaranteed that exactly one thread is woken up, so we check that at least one thread was woken up.
      NS_TEST_BOOL(iCounter >= iExpected);
    }

    // wake up the rest
    {
      cv.SignalAll();

      for (nsUInt32 a = 0; a < 1000; ++a)
      {
        nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(1));

        if (iCounter >= (nsInt32)uiNumThreads)
          break;
      }

      // theoretically this could fail, if the OS doesn't wake up any other thread in time
      // but with 1000 tries that is very unlikely
      NS_TEST_INT(iCounter, (nsInt32)uiNumThreads);
      NS_TEST_BOOL(iCounter <= (nsInt32)uiNumThreads); // THIS test must never fail!
    }

    for (nsUInt32 i = 0; i < uiNumThreads; ++i)
    {
      pTestThreads[i]->Join();
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Wait With timeout")
  {
    constexpr nsUInt32 uiNumThreads = 16;

    nsUniquePtr<TestThreadTimeout> pTestThreads[uiNumThreads];
    nsAtomicInteger32 iCounter = uiNumThreads;
    nsConditionVariable cv;
    nsConditionVariable cvt;

    for (nsUInt32 i = 0; i < uiNumThreads; ++i)
    {
      pTestThreads[i] = NS_DEFAULT_NEW(TestThreadTimeout);
      pTestThreads[i]->m_pCounter = &iCounter;
      pTestThreads[i]->m_pCV = &cv;
      pTestThreads[i]->m_pCVTimeout = &cvt;
      pTestThreads[i]->Start();
    }

    // wait until all threads are in waiting state
    while (true)
    {
      // We need to lock here as otherwise we could signal
      // while a thread hasn't reached the wait yet.
      NS_LOCK(cv);
      if (iCounter == 0)
        break;

      nsThreadUtils::YieldTimeSlice();
    }

    // open the flood gates
    cv.SignalAll();

    // all threads should run into their timeout now
    for (nsUInt32 a = 0; a < 100; ++a)
    {
      nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(50));

      if (iCounter >= (nsInt32)uiNumThreads)
        break;
    }

    // theoretically this could fail, if the OS doesn't wake up any other thread in time
    // but with 100 tries that is very unlikely
    NS_TEST_INT(iCounter, (nsInt32)uiNumThreads);
    NS_TEST_BOOL(iCounter <= (nsInt32)uiNumThreads); // THIS test must never fail!

    for (nsUInt32 i = 0; i < uiNumThreads; ++i)
    {
      pTestThreads[i]->Join();
    }
  }
}

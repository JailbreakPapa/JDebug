#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Threading/Mutex.h>
#include <Foundation/Threading/Thread.h>
#include <Foundation/Time/Time.h>

namespace
{
  nsInt32 g_iCrossThreadVariable = 0;
  const nsUInt32 g_uiIncrementSteps = 160000;

  class TestThread3 : public nsThread
  {
  public:
    TestThread3()
      : nsThread("Test Thread")
    {
    }

    nsMutex* m_pWaitMutex = nullptr;
    nsMutex* m_pBlockedMutex = nullptr;

    virtual nsUInt32 Run()
    {
      // test TryLock on a locked mutex
      NS_TEST_BOOL(m_pBlockedMutex->TryLock().Failed());

      {
        // enter and leave the mutex once
        NS_LOCK(*m_pWaitMutex);
      }

      NS_PROFILE_SCOPE("Test Thread::Run");

      for (nsUInt32 i = 0; i < g_uiIncrementSteps; i++)
      {
        nsAtomicUtils::Increment(g_iCrossThreadVariable);

        nsTime::Now();
        nsThreadUtils::YieldTimeSlice();
        nsTime::Now();
      }

      return 0;
    }
  };
} // namespace

NS_CREATE_SIMPLE_TEST_GROUP(Threading);

NS_CREATE_SIMPLE_TEST(Threading, Thread)
{
  g_iCrossThreadVariable = 0;


  NS_TEST_BLOCK(nsTestBlock::Enabled, "Thread")
  {
    TestThread3* pTestThread31 = nullptr;
    TestThread3* pTestThread32 = nullptr;

    /// the try-catch is necessary to quiet the static code analysis
    try
    {
      pTestThread31 = new TestThread3;
      pTestThread32 = new TestThread3;
    }
    catch (...)
    {
    }

    NS_TEST_BOOL(pTestThread31 != nullptr);
    NS_TEST_BOOL(pTestThread32 != nullptr);

    nsMutex waitMutex, blockedMutex;
    pTestThread31->m_pWaitMutex = &waitMutex;
    pTestThread32->m_pWaitMutex = &waitMutex;

    pTestThread31->m_pBlockedMutex = &blockedMutex;
    pTestThread32->m_pBlockedMutex = &blockedMutex;

    // no one holds these mutexes yet, must succeed
    NS_TEST_BOOL(blockedMutex.TryLock().Succeeded());
    NS_TEST_BOOL(waitMutex.TryLock().Succeeded());

    // Both thread will increment the global variable via atomic operations
    pTestThread31->Start();
    pTestThread32->Start();

    // give the threads a bit of time to start
    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(50));

    // allow the threads to run now
    waitMutex.Unlock();

    // Main thread will also increment the test variable
    nsAtomicUtils::Increment(g_iCrossThreadVariable);

    // Join with both threads
    pTestThread31->Join();
    pTestThread32->Join();

    // we are holding the mutex, another TryLock should work
    NS_TEST_BOOL(blockedMutex.TryLock().Succeeded());

    // The threads should have finished, no one holds the lock
    NS_TEST_BOOL(waitMutex.TryLock().Succeeded());

    // Test deletion
    delete pTestThread31;
    delete pTestThread32;

    NS_TEST_INT(g_iCrossThreadVariable, g_uiIncrementSteps * 2 + 1);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Thread Sleeping")
  {
    const nsTime start = nsTime::Now();

    nsTime sleepTime(nsTime::MakeFromSeconds(0.3));

    nsThreadUtils::Sleep(sleepTime);

    const nsTime stop = nsTime::Now();

    const nsTime duration = stop - start;

    // We test for 0.25 - 0.35 since the threading functions are a bit varying in their precision
    NS_TEST_BOOL(duration.GetSeconds() > 0.25);
    NS_TEST_BOOL_MSG(duration.GetSeconds() < 1.0, "This test can fail when the machine is under too much load and blocks the process for too long.");
  }
}

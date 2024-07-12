#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Threading/ThreadUtils.h>
#include <Foundation/Time/Stopwatch.h>

NS_CREATE_SIMPLE_TEST(Time, Stopwatch)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "General Functionality")
  {
    nsStopwatch sw;

    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(50));

    sw.StopAndReset();
    sw.Resume();

    const nsTime t0 = sw.Checkpoint();

    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));

    const nsTime t1 = sw.Checkpoint();

    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(20));

    const nsTime t2 = sw.Checkpoint();

    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(30));

    const nsTime t3 = sw.Checkpoint();

    const nsTime tTotal1 = sw.GetRunningTotal();

    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));

    sw.Pause(); // frense the current running total

    const nsTime tTotal2 = sw.GetRunningTotal();

    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10)); // should not affect the running total anymore

    const nsTime tTotal3 = sw.GetRunningTotal();


    // these tests are deliberately written such that they cannot fail,
    // even when the OS is under heavy load

    NS_TEST_BOOL(t0 > nsTime::MakeFromMilliseconds(5));
    NS_TEST_BOOL(t1 > nsTime::MakeFromMilliseconds(5));
    NS_TEST_BOOL(t2 > nsTime::MakeFromMilliseconds(5));
    NS_TEST_BOOL(t3 > nsTime::MakeFromMilliseconds(5));


    NS_TEST_BOOL(t1 + t2 + t3 <= tTotal1);
    NS_TEST_BOOL(t0 + t1 + t2 + t3 > tTotal1);

    NS_TEST_BOOL(tTotal1 < tTotal2);
    NS_TEST_BOOL(tTotal1 < tTotal3);
    NS_TEST_BOOL(tTotal2 == tTotal3);
  }
}

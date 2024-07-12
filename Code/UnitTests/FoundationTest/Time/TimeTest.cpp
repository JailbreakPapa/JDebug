#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Time/Time.h>

NS_CREATE_SIMPLE_TEST_GROUP(Time);

NS_CREATE_SIMPLE_TEST(Time, Timer)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Basics")
  {
    nsTime TestTime = nsTime::Now();

    NS_TEST_BOOL(TestTime.GetMicroseconds() > 0.0);

    volatile nsUInt32 testValue = 0;
    for (nsUInt32 i = 0; i < 42000; ++i)
    {
      testValue += 23;
    }

    nsTime TestTime2 = nsTime::Now();

    NS_TEST_BOOL(TestTime2.GetMicroseconds() > 0.0);

    TestTime2 -= TestTime;

    NS_TEST_BOOL(TestTime2.GetMicroseconds() > 0.0);
  }
}

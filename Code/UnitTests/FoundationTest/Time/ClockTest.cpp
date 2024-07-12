#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Threading/ThreadUtils.h>
#include <Foundation/Time/Clock.h>

class nsSimpleTimeStepSmoother : public nsTimeStepSmoothing
{
public:
  virtual nsTime GetSmoothedTimeStep(nsTime rawTimeStep, const nsClock* pClock) override { return nsTime::MakeFromSeconds(0.42); }

  virtual void Reset(const nsClock* pClock) override {}
};

NS_CREATE_SIMPLE_TEST(Time, Clock)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructor / Reset")
  {
    nsClock c("Test");                                 // calls 'Reset' internally

    NS_TEST_BOOL(c.GetTimeStepSmoothing() == nullptr); // after constructor

    NS_TEST_DOUBLE(c.GetAccumulatedTime().GetSeconds(), 0.0, 0.0);
    NS_TEST_DOUBLE(c.GetFixedTimeStep().GetSeconds(), 0.0, 0.0);
    NS_TEST_DOUBLE(c.GetSpeed(), 1.0, 0.0);
    NS_TEST_BOOL(c.GetPaused() == false);
    NS_TEST_DOUBLE(c.GetMinimumTimeStep().GetSeconds(), 0.001, 0.0); // to ensure the tests fail if somebody changes these constants
    NS_TEST_DOUBLE(c.GetMaximumTimeStep().GetSeconds(), 0.1, 0.0);   // to ensure the tests fail if somebody changes these constants
    NS_TEST_BOOL(c.GetTimeDiff() > nsTime::MakeFromSeconds(0.0));

    nsSimpleTimeStepSmoother s;

    c.SetTimeStepSmoothing(&s);

    NS_TEST_BOOL(c.GetTimeStepSmoothing() == &s);

    c.Reset(false);

    // does NOT reset which time step smoother to use
    NS_TEST_BOOL(c.GetTimeStepSmoothing() == &s);

    c.Reset(true);
    NS_TEST_BOOL(c.GetTimeStepSmoothing() == nullptr); // after constructor
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetPaused / GetPaused")
  {
    nsClock c("Test");
    NS_TEST_BOOL(!c.GetPaused());

    c.SetPaused(true);
    NS_TEST_BOOL(c.GetPaused());

    c.SetPaused(false);
    NS_TEST_BOOL(!c.GetPaused());

    c.SetPaused(true);
    NS_TEST_BOOL(c.GetPaused());

    c.Reset(false);
    NS_TEST_BOOL(!c.GetPaused());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Updates while Paused / Unpaused")
  {
    nsClock c("Test");

    c.SetPaused(false);

    const nsTime t0 = c.GetAccumulatedTime();

    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));
    c.Update();

    const nsTime t1 = c.GetAccumulatedTime();
    NS_TEST_BOOL(t0 < t1);

    c.SetPaused(true);

    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));
    c.Update();

    const nsTime t2 = c.GetAccumulatedTime();
    NS_TEST_BOOL(t1 == t2);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetFixedTimeStep / GetFixedTimeStep")
  {
    nsClock c("Test");

    NS_TEST_DOUBLE(c.GetFixedTimeStep().GetSeconds(), 0.0, 0.0);

    c.SetFixedTimeStep(nsTime::MakeFromSeconds(1.0 / 60.0));

    NS_TEST_DOUBLE(c.GetFixedTimeStep().GetSeconds(), 1.0 / 60.0, 0.000001);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Updates with fixed time step")
  {
    nsClock c("Test");
    c.SetFixedTimeStep(nsTime::MakeFromSeconds(1.0 / 60.0));
    c.Update();

    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));

    c.Update();
    NS_TEST_DOUBLE(c.GetTimeDiff().GetSeconds(), 1.0 / 60.0, 0.000001);

    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(50));

    c.Update();
    NS_TEST_DOUBLE(c.GetTimeDiff().GetSeconds(), 1.0 / 60.0, 0.000001);

    c.Update();
    NS_TEST_DOUBLE(c.GetTimeDiff().GetSeconds(), 1.0 / 60.0, 0.000001);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetAccumulatedTime / GetAccumulatedTime")
  {
    nsClock c("Test");

    c.SetAccumulatedTime(nsTime::MakeFromSeconds(23.42));

    NS_TEST_DOUBLE(c.GetAccumulatedTime().GetSeconds(), 23.42, 0.000001);

    c.Update(); // by default after a SetAccumulatedTime the time diff should always be > 0

    NS_TEST_BOOL(c.GetTimeDiff().GetSeconds() > 0.0);

    const nsTime t0 = c.GetAccumulatedTime();

    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(5));
    c.Update();

    const nsTime t1 = c.GetAccumulatedTime();

    NS_TEST_BOOL(t1 > t0);
    NS_TEST_BOOL(c.GetTimeDiff().GetSeconds() > 0.0);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetSpeed / GetSpeed / GetTimeDiff")
  {
    nsClock c("Test");
    NS_TEST_DOUBLE(c.GetSpeed(), 1.0, 0.0);

    c.SetFixedTimeStep(nsTime::MakeFromSeconds(0.01));

    c.SetSpeed(10.0);
    NS_TEST_DOUBLE(c.GetSpeed(), 10.0, 0.000001);

    c.Update();
    const nsTime t0 = c.GetTimeDiff();
    NS_TEST_DOUBLE(t0.GetSeconds(), 0.1, 0.00001);

    c.SetSpeed(0.1);

    c.Update();
    const nsTime t1 = c.GetTimeDiff();
    NS_TEST_DOUBLE(t1.GetSeconds(), 0.001, 0.00001);

    c.Reset(false);

    c.Update();
    const nsTime t2 = c.GetTimeDiff();
    NS_TEST_DOUBLE(t2.GetSeconds(), 0.01, 0.00001);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetMinimumTimeStep / GetMinimumTimeStep")
  {
    nsClock c("Test");
    NS_TEST_DOUBLE(c.GetMinimumTimeStep().GetSeconds(), 0.001, 0.0); // to ensure the tests fail if somebody changes these constants

    c.Update();
    c.Update();

    NS_TEST_DOUBLE(c.GetTimeDiff().GetSeconds(), c.GetMinimumTimeStep().GetSeconds(), 0.0000000001);

    c.SetMinimumTimeStep(nsTime::MakeFromSeconds(0.1));
    c.SetMaximumTimeStep(nsTime::MakeFromSeconds(1.0));

    NS_TEST_DOUBLE(c.GetMinimumTimeStep().GetSeconds(), 0.1, 0.0);

    c.Update();
    c.Update();

    NS_TEST_DOUBLE(c.GetTimeDiff().GetSeconds(), c.GetMinimumTimeStep().GetSeconds(), 0.0000000001);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetMaximumTimeStep / GetMaximumTimeStep")
  {
    nsClock c("Test");
    NS_TEST_DOUBLE(c.GetMaximumTimeStep().GetSeconds(), 0.1, 0.0); // to ensure the tests fail if somebody changes these constants

    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(200));
    c.Update();

    NS_TEST_DOUBLE(c.GetTimeDiff().GetSeconds(), c.GetMaximumTimeStep().GetSeconds(), 0.0000000001);

    c.SetMaximumTimeStep(nsTime::MakeFromSeconds(0.2));

    NS_TEST_DOUBLE(c.GetMaximumTimeStep().GetSeconds(), 0.2, 0.0);

    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(400));
    c.Update();

    NS_TEST_DOUBLE(c.GetTimeDiff().GetSeconds(), c.GetMaximumTimeStep().GetSeconds(), 0.0000000001);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "SetTimeStepSmoothing / GetTimeStepSmoothing")
  {
    nsClock c("Test");

    NS_TEST_BOOL(c.GetTimeStepSmoothing() == nullptr);

    nsSimpleTimeStepSmoother s;
    c.SetTimeStepSmoothing(&s);

    NS_TEST_BOOL(c.GetTimeStepSmoothing() == &s);

    c.SetMaximumTimeStep(nsTime::MakeFromSeconds(10.0)); // this would limit the time step even after smoothing
    c.Update();

    NS_TEST_DOUBLE(c.GetTimeDiff().GetSeconds(), 0.42, 0.0);
  }
}

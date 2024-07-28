#include <Core/CorePCH.h>

#include <Core/Utils/IntervalScheduler.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_ENUM(nsUpdateRate, 1)
  NS_ENUM_CONSTANTS(nsUpdateRate::EveryFrame)
  NS_ENUM_CONSTANTS(nsUpdateRate::Max30fps, nsUpdateRate::Max20fps, nsUpdateRate::Max10fps)
  NS_ENUM_CONSTANTS(nsUpdateRate::Max5fps, nsUpdateRate::Max2fps, nsUpdateRate::Max1fps)
  NS_ENUM_CONSTANTS(nsUpdateRate::Never)
NS_END_STATIC_REFLECTED_ENUM;
// clang-format on

static nsTime s_Intervals[] = {
  nsTime::MakeZero(),                  // EveryFrame
  nsTime::MakeFromSeconds(1.0 / 30.0), // Max30fps
  nsTime::MakeFromSeconds(1.0 / 20.0), // Max20fps
  nsTime::MakeFromSeconds(1.0 / 10.0), // Max10fps
  nsTime::MakeFromSeconds(1.0 / 5.0),  // Max5fps
  nsTime::MakeFromSeconds(1.0 / 2.0),  // Max2fps
  nsTime::MakeFromSeconds(1.0 / 1.0),  // Max1fps
};

static_assert(NS_ARRAY_SIZE(s_Intervals) == nsUpdateRate::Max1fps + 1);

nsTime nsUpdateRate::GetInterval(Enum updateRate)
{
  return s_Intervals[updateRate];
}

//////////////////////////////////////////////////////////////////////////

nsIntervalSchedulerBase::nsIntervalSchedulerBase(nsTime minInterval, nsTime maxInterval)
  : m_MinInterval(minInterval)
  , m_MaxInterval(maxInterval)
{
  NS_ASSERT_DEV(m_MinInterval.IsPositive(), "Min interval must be greater than zero");
  NS_ASSERT_DEV(m_MaxInterval > m_MinInterval, "Max interval must be greater than min interval");

  m_fInvIntervalRange = 1.0 / (m_MaxInterval - m_MinInterval).GetSeconds();

  for (nsUInt32 i = 0; i < HistogramSize; ++i)
  {
    m_HistogramSlotValues[i] = GetHistogramSlotValue(i);
  }
}

nsIntervalSchedulerBase::~nsIntervalSchedulerBase() = default;


NS_STATICLINK_FILE(Core, Core_Utils_Implementation_IntervalScheduler);

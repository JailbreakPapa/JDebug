#pragma once

#include <Core/CoreDLL.h>
#include <Foundation/Reflection/Reflection.h>

struct NS_CORE_DLL nsUpdateRate
{
  using StorageType = nsUInt8;

  enum Enum
  {
    EveryFrame,
    Max30fps,
    Max20fps,
    Max10fps,
    Max5fps,
    Max2fps,
    Max1fps,
    Never,

    Default = Max30fps
  };

  static nsTime GetInterval(Enum updateRate);
};

NS_DECLARE_REFLECTABLE_TYPE(NS_CORE_DLL, nsUpdateRate);

//////////////////////////////////////////////////////////////////////////

/// \brief Helper class to schedule work in intervals typically larger than the duration of one frame
///
/// Tries to maintain an even workload per frame and also keep the given interval for a work as best as possible.
/// A typical use case would be e.g. component update functions that don't need to be called every frame.
class NS_CORE_DLL nsIntervalSchedulerBase
{
protected:
  nsIntervalSchedulerBase(nsTime minInterval, nsTime maxInterval);
  ~nsIntervalSchedulerBase();

  nsUInt32 GetHistogramIndex(nsTime value);
  nsTime GetHistogramSlotValue(nsUInt32 uiIndex);

  static float GetRandomZeroToOne(int pos, nsUInt32& seed);
  static nsTime GetRandomTimeJitter(int pos, nsUInt32& seed);

  nsTime m_MinInterval;
  nsTime m_MaxInterval;
  double m_fInvIntervalRange;

  nsTime m_CurrentTime;

  nsUInt32 m_uiSeed = 0;

  static constexpr nsUInt32 HistogramSize = 32;
  nsUInt32 m_Histogram[HistogramSize] = {};
  nsTime m_HistogramSlotValues[HistogramSize] = {};
};

//////////////////////////////////////////////////////////////////////////

/// \brief \see nsIntervalSchedulerBase
template <typename T>
class nsIntervalScheduler : public nsIntervalSchedulerBase
{
  using SUPER = nsIntervalSchedulerBase;

public:
  NS_ALWAYS_INLINE nsIntervalScheduler(nsTime minInterval = nsTime::MakeFromMilliseconds(1), nsTime maxInterval = nsTime::MakeFromSeconds(1))
    : SUPER(minInterval, maxInterval)
  {
  }

  void AddOrUpdateWork(const T& work, nsTime interval);
  void RemoveWork(const T& work);

  nsTime GetInterval(const T& work) const;

  // reference to the work that should be run and time passed since this work has been last run.
  using RunWorkCallback = nsDelegate<void(const T&, nsTime)>;

  /// \brief Advances the scheduler by deltaTime and triggers runWorkCallback for each work that should be run during this update step.
  /// Since it is not possible to maintain the exact interval all the time the actual delta time for the work is also passed to runWorkCallback.
  void Update(nsTime deltaTime, RunWorkCallback runWorkCallback);

  void Clear();

private:
  struct Data
  {
    T m_Work;
    nsTime m_Interval;
    nsTime m_DueTime;
    nsTime m_LastScheduledTime;

    bool IsValid() const;
    void MarkAsInvalid();
  };

  using DataMap = nsMap<nsTime, Data>;
  DataMap m_Data;
  nsHashTable<T, typename DataMap::Iterator> m_WorkIdToData;

  typename DataMap::Iterator InsertData(Data& data);
  nsDynamicArray<typename DataMap::Iterator> m_ScheduledWork;
};

#include <Core/Utils/Implementation/IntervalScheduler_inl.h>

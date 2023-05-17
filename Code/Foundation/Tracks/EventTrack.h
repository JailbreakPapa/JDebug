#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Time/Time.h>

/// \brief An event track is a time line that contains named events.
///
/// The time line can be sampled to query all events that occurred during a time period.
/// There is no way to sample an event track at a fixed point in time, because events occur at specific time points and thus
/// only range queries make sense.
class WD_FOUNDATION_DLL wdEventTrack
{
public:
  wdEventTrack();
  ~wdEventTrack();

  /// \brief Removes all control points.
  void Clear();

  /// \brief Checks whether there are any control points in the track.
  bool IsEmpty() const;

  /// \brief Adds a named event into the track at the given time.
  void AddControlPoint(wdTime time, wdStringView sEvent);

  /// \brief Samples the event track from range [start; end) and adds all events that occured in that time period to the array.
  ///
  /// Note that the range is inclusive for the start time, and exclusive for the end time.
  ///
  /// If rangeStart is larger than rangeEnd, the events are returned in reverse order (backwards traversal).
  void Sample(wdTime rangeStart, wdTime rangeEnd, wdDynamicArray<wdHashedString>& out_events) const;

  void Save(wdStreamWriter& inout_stream) const;
  void Load(wdStreamReader& inout_stream);

private:
  struct ControlPoint
  {
    WD_ALWAYS_INLINE bool operator<(const ControlPoint& rhs) const { return m_Time < rhs.m_Time; }

    wdTime m_Time;
    wdUInt32 m_uiEvent;
  };

  wdUInt32 FindControlPointAfter(wdTime x) const;
  wdInt32 FindControlPointBefore(wdTime x) const;

  mutable bool m_bSort = false;
  mutable wdDynamicArray<ControlPoint> m_ControlPoints;
  wdHybridArray<wdHashedString, 4> m_Events;
};

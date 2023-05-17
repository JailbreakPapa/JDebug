#include <Foundation/FoundationPCH.h>

#include <Foundation/Tracks/EventTrack.h>

wdEventTrack::wdEventTrack() {}

wdEventTrack::~wdEventTrack() {}

void wdEventTrack::Clear()
{
  m_Events.Clear();
  m_ControlPoints.Clear();
}

bool wdEventTrack::IsEmpty() const
{
  return m_ControlPoints.IsEmpty();
}

void wdEventTrack::AddControlPoint(wdTime time, wdStringView sEvent)
{
  m_bSort = true;

  const wdUInt32 uiNumEvents = m_Events.GetCount();

  auto& cp = m_ControlPoints.ExpandAndGetRef();
  cp.m_Time = time;

  // search for existing event
  {
    wdTempHashedString tmp(sEvent);

    for (wdUInt32 i = 0; i < uiNumEvents; ++i)
    {
      if (m_Events[i] == tmp)
      {
        cp.m_uiEvent = i;
        return;
      }
    }
  }

  // not found -> add event name
  {
    cp.m_uiEvent = uiNumEvents;

    wdHashedString hs;
    hs.Assign(sEvent);

    m_Events.PushBack(hs);
  }
}

wdUInt32 wdEventTrack::FindControlPointAfter(wdTime x) const
{
  // searches for a control point after OR AT x

  WD_ASSERT_DEBUG(!m_ControlPoints.IsEmpty(), "");

  wdUInt32 uiLowIdx = 0;
  wdUInt32 uiHighIdx = m_ControlPoints.GetCount() - 1;

  // do a binary search to reduce the search space
  while (uiHighIdx - uiLowIdx > 8)
  {
    const wdUInt32 uiMidIdx = uiLowIdx + ((uiHighIdx - uiLowIdx) >> 1); // lerp

    if (m_ControlPoints[uiMidIdx].m_Time >= x)
      uiHighIdx = uiMidIdx;
    else
      uiLowIdx = uiMidIdx;
  }

  // now do a linear search to find the final item
  for (wdUInt32 idx = uiLowIdx; idx <= uiHighIdx; ++idx)
  {
    if (m_ControlPoints[idx].m_Time >= x)
    {
      return idx;
    }
  }

  WD_ASSERT_DEBUG(uiHighIdx + 1 == m_ControlPoints.GetCount(), "Unexpected event track entry index");
  return m_ControlPoints.GetCount();
}

wdInt32 wdEventTrack::FindControlPointBefore(wdTime x) const
{
  // searches for a control point before OR AT x

  WD_ASSERT_DEBUG(!m_ControlPoints.IsEmpty(), "");

  wdInt32 iLowIdx = 0;
  wdInt32 iHighIdx = (wdInt32)m_ControlPoints.GetCount() - 1;

  // do a binary search to reduce the search space
  while (iHighIdx - iLowIdx > 8)
  {
    const wdInt32 uiMidIdx = iLowIdx + ((iHighIdx - iLowIdx) >> 1); // lerp

    if (m_ControlPoints[uiMidIdx].m_Time >= x)
      iHighIdx = uiMidIdx;
    else
      iLowIdx = uiMidIdx;
  }

  // now do a linear search to find the final item
  for (wdInt32 idx = iHighIdx; idx >= iLowIdx; --idx)
  {
    if (m_ControlPoints[idx].m_Time <= x)
    {
      return idx;
    }
  }

  WD_ASSERT_DEBUG(iLowIdx == 0, "Unexpected event track entry index");
  return -1;
}

void wdEventTrack::Sample(wdTime rangeStart, wdTime rangeEnd, wdDynamicArray<wdHashedString>& out_events) const
{
  if (m_ControlPoints.IsEmpty())
    return;

  if (m_bSort)
  {
    m_bSort = false;
    m_ControlPoints.Sort();
  }

  if (rangeStart <= rangeEnd)
  {
    wdUInt32 curCpIdx = FindControlPointAfter(rangeStart);

    const wdUInt32 uiNumCPs = m_ControlPoints.GetCount();
    while (curCpIdx < uiNumCPs && m_ControlPoints[curCpIdx].m_Time < rangeEnd)
    {
      const wdHashedString& sEvent = m_Events[m_ControlPoints[curCpIdx].m_uiEvent];

      out_events.PushBack(sEvent);

      ++curCpIdx;
    }
  }
  else
  {
    wdInt32 curCpIdx = FindControlPointBefore(rangeStart);

    while (curCpIdx >= 0 && m_ControlPoints[curCpIdx].m_Time > rangeEnd)
    {
      const wdHashedString& sEvent = m_Events[m_ControlPoints[curCpIdx].m_uiEvent];

      out_events.PushBack(sEvent);

      --curCpIdx;
    }
  }
}

void wdEventTrack::Save(wdStreamWriter& inout_stream) const
{
  if (m_bSort)
  {
    m_bSort = false;
    m_ControlPoints.Sort();
  }

  wdUInt8 uiVersion = 1;
  inout_stream << uiVersion;

  inout_stream << m_Events.GetCount();
  for (const wdHashedString& name : m_Events)
  {
    inout_stream << name.GetString();
  }

  inout_stream << m_ControlPoints.GetCount();
  for (const ControlPoint& cp : m_ControlPoints)
  {
    inout_stream << cp.m_Time;
    inout_stream << cp.m_uiEvent;
  }
}

void wdEventTrack::Load(wdStreamReader& inout_stream)
{
  // don't rely on the data being sorted
  m_bSort = true;

  wdUInt8 uiVersion = 0;
  inout_stream >> uiVersion;

  WD_ASSERT_DEV(uiVersion == 1, "Invalid event track version {0}", uiVersion);

  wdUInt32 count = 0;
  wdStringBuilder tmp;

  inout_stream >> count;
  m_Events.SetCount(count);
  for (wdHashedString& name : m_Events)
  {
    inout_stream >> tmp;
    name.Assign(tmp);
  }

  inout_stream >> count;
  m_ControlPoints.SetCount(count);
  for (ControlPoint& cp : m_ControlPoints)
  {
    inout_stream >> cp.m_Time;
    inout_stream >> cp.m_uiEvent;
  }
}



WD_STATICLINK_FILE(Foundation, Foundation_Tracks_Implementation_EventTrack);

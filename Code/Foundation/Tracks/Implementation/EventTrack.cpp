#include <Foundation/FoundationPCH.h>

#include <Foundation/Tracks/EventTrack.h>

nsEventTrack::nsEventTrack() = default;

nsEventTrack::~nsEventTrack() = default;

void nsEventTrack::Clear()
{
  m_Events.Clear();
  m_ControlPoints.Clear();
}

bool nsEventTrack::IsEmpty() const
{
  return m_ControlPoints.IsEmpty();
}

void nsEventTrack::AddControlPoint(nsTime time, nsStringView sEvent)
{
  m_bSort = true;

  const nsUInt32 uiNumEvents = m_Events.GetCount();

  auto& cp = m_ControlPoints.ExpandAndGetRef();
  cp.m_Time = time;

  // search for existing event
  {
    nsTempHashedString tmp(sEvent);

    for (nsUInt32 i = 0; i < uiNumEvents; ++i)
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

    nsHashedString hs;
    hs.Assign(sEvent);

    m_Events.PushBack(hs);
  }
}

nsUInt32 nsEventTrack::FindControlPointAfter(nsTime x) const
{
  // searches for a control point after OR AT x

  NS_ASSERT_DEBUG(!m_ControlPoints.IsEmpty(), "");

  nsUInt32 uiLowIdx = 0;
  nsUInt32 uiHighIdx = m_ControlPoints.GetCount() - 1;

  // do a binary search to reduce the search space
  while (uiHighIdx - uiLowIdx > 8)
  {
    const nsUInt32 uiMidIdx = uiLowIdx + ((uiHighIdx - uiLowIdx) >> 1); // lerp

    if (m_ControlPoints[uiMidIdx].m_Time >= x)
      uiHighIdx = uiMidIdx;
    else
      uiLowIdx = uiMidIdx;
  }

  // now do a linear search to find the final item
  for (nsUInt32 idx = uiLowIdx; idx <= uiHighIdx; ++idx)
  {
    if (m_ControlPoints[idx].m_Time >= x)
    {
      return idx;
    }
  }

  NS_ASSERT_DEBUG(uiHighIdx + 1 == m_ControlPoints.GetCount(), "Unexpected event track entry index");
  return m_ControlPoints.GetCount();
}

nsInt32 nsEventTrack::FindControlPointBefore(nsTime x) const
{
  // searches for a control point before OR AT x

  NS_ASSERT_DEBUG(!m_ControlPoints.IsEmpty(), "");

  nsInt32 iLowIdx = 0;
  nsInt32 iHighIdx = (nsInt32)m_ControlPoints.GetCount() - 1;

  // do a binary search to reduce the search space
  while (iHighIdx - iLowIdx > 8)
  {
    const nsInt32 uiMidIdx = iLowIdx + ((iHighIdx - iLowIdx) >> 1); // lerp

    if (m_ControlPoints[uiMidIdx].m_Time >= x)
      iHighIdx = uiMidIdx;
    else
      iLowIdx = uiMidIdx;
  }

  // now do a linear search to find the final item
  for (nsInt32 idx = iHighIdx; idx >= iLowIdx; --idx)
  {
    if (m_ControlPoints[idx].m_Time <= x)
    {
      return idx;
    }
  }

  NS_ASSERT_DEBUG(iLowIdx == 0, "Unexpected event track entry index");
  return -1;
}

void nsEventTrack::Sample(nsTime rangeStart, nsTime rangeEnd, nsDynamicArray<nsHashedString>& out_events) const
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
    nsUInt32 curCpIdx = FindControlPointAfter(rangeStart);

    const nsUInt32 uiNumCPs = m_ControlPoints.GetCount();
    while (curCpIdx < uiNumCPs && m_ControlPoints[curCpIdx].m_Time < rangeEnd)
    {
      const nsHashedString& sEvent = m_Events[m_ControlPoints[curCpIdx].m_uiEvent];

      out_events.PushBack(sEvent);

      ++curCpIdx;
    }
  }
  else
  {
    nsInt32 curCpIdx = FindControlPointBefore(rangeStart);

    while (curCpIdx >= 0 && m_ControlPoints[curCpIdx].m_Time > rangeEnd)
    {
      const nsHashedString& sEvent = m_Events[m_ControlPoints[curCpIdx].m_uiEvent];

      out_events.PushBack(sEvent);

      --curCpIdx;
    }
  }
}

void nsEventTrack::Save(nsStreamWriter& inout_stream) const
{
  if (m_bSort)
  {
    m_bSort = false;
    m_ControlPoints.Sort();
  }

  nsUInt8 uiVersion = 1;
  inout_stream << uiVersion;

  inout_stream << m_Events.GetCount();
  for (const nsHashedString& name : m_Events)
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

void nsEventTrack::Load(nsStreamReader& inout_stream)
{
  // don't rely on the data being sorted
  m_bSort = true;

  nsUInt8 uiVersion = 0;
  inout_stream >> uiVersion;

  NS_ASSERT_DEV(uiVersion == 1, "Invalid event track version {0}", uiVersion);

  nsUInt32 count = 0;
  nsStringBuilder tmp;

  inout_stream >> count;
  m_Events.SetCount(count);
  for (nsHashedString& name : m_Events)
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

#include <Foundation/FoundationPCH.h>

#include <Foundation/Utilities/Stats.h>

nsMutex nsStats::s_Mutex;
nsStats::MapType nsStats::s_Stats;
nsStats::nsEventStats nsStats::s_StatsEvents;

void nsStats::RemoveStat(nsStringView sStatName)
{
  NS_LOCK(s_Mutex);

  MapType::Iterator it = s_Stats.Find(sStatName);

  if (!it.IsValid())
    return;

  s_Stats.Remove(it);

  StatsEventData e;
  e.m_EventType = StatsEventData::Remove;
  e.m_sStatName = sStatName;

  s_StatsEvents.Broadcast(e);
}

void nsStats::SetStat(nsStringView sStatName, const nsVariant& value)
{
  NS_LOCK(s_Mutex);

  bool bExisted = false;
  auto it = s_Stats.FindOrAdd(sStatName, &bExisted);

  if (it.Value() == value)
    return;

  it.Value() = value;

  StatsEventData e;
  e.m_EventType = bExisted ? StatsEventData::Set : StatsEventData::Add;
  e.m_sStatName = sStatName;
  e.m_NewStatValue = value;

  s_StatsEvents.Broadcast(e);
}

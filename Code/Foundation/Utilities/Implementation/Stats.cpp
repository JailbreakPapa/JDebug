#include <Foundation/FoundationPCH.h>

#include <Foundation/Utilities/Stats.h>

wdMutex wdStats::s_Mutex;
wdStats::MapType wdStats::s_Stats;
wdStats::wdEventStats wdStats::s_StatsEvents;

void wdStats::RemoveStat(const char* szStatName)
{
  WD_LOCK(s_Mutex);

  MapType::Iterator it = s_Stats.Find(szStatName);

  if (!it.IsValid())
    return;

  s_Stats.Remove(it);

  StatsEventData e;
  e.m_EventType = StatsEventData::Remove;
  e.m_szStatName = szStatName;

  s_StatsEvents.Broadcast(e);
}

void wdStats::SetStat(const char* szStatName, const wdVariant& value)
{
  WD_LOCK(s_Mutex);

  bool bExisted = false;
  auto it = s_Stats.FindOrAdd(szStatName, &bExisted);

  if (it.Value() == value)
    return;

  it.Value() = value;

  StatsEventData e;
  e.m_EventType = bExisted ? StatsEventData::Set : StatsEventData::Add;
  e.m_szStatName = szStatName;
  e.m_NewStatValue = value;

  s_StatsEvents.Broadcast(e);
}


WD_STATICLINK_FILE(Foundation, Foundation_Utilities_Implementation_Stats);

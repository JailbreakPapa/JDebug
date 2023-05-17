#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/GlobalEvent.h>
#include <Foundation/Logging/Log.h>

WD_ENUMERABLE_CLASS_IMPLEMENTATION(wdGlobalEvent);

wdGlobalEvent::EventMap wdGlobalEvent::s_KnownEvents;

wdGlobalEvent::EventData::EventData()
{
  m_uiNumTimesFired = 0;
  m_uiNumEventHandlersOnce = 0;
  m_uiNumEventHandlersRegular = 0;
}

wdGlobalEvent::wdGlobalEvent(const char* szEventName, WD_GLOBAL_EVENT_HANDLER handler, bool bOnlyOnce)
{
  m_szEventName = szEventName;
  m_bOnlyOnce = bOnlyOnce;
  m_bHasBeenFired = false;
  m_EventHandler = handler;
}

void wdGlobalEvent::Broadcast(const char* szEventName, wdVariant p1, wdVariant p2, wdVariant p3, wdVariant p4)
{
  wdGlobalEvent* pHandler = wdGlobalEvent::GetFirstInstance();

  while (pHandler)
  {
    if (wdStringUtils::IsEqual(pHandler->m_szEventName, szEventName))
    {
      if (!pHandler->m_bOnlyOnce || !pHandler->m_bHasBeenFired)
      {
        pHandler->m_bHasBeenFired = true;

        pHandler->m_EventHandler(p1, p2, p3, p4);
      }
    }

    pHandler = pHandler->GetNextInstance();
  }


  EventData& ed = s_KnownEvents[szEventName]; // this will make sure to record all fired events, even if there are no handlers for them
  ed.m_uiNumTimesFired++;
}

void wdGlobalEvent::UpdateGlobalEventStatistics()
{
  for (EventMap::Iterator it = s_KnownEvents.GetIterator(); it.IsValid(); ++it)
  {
    it.Value().m_uiNumEventHandlersRegular = 0;
    it.Value().m_uiNumEventHandlersOnce = 0;
  }

  wdGlobalEvent* pHandler = wdGlobalEvent::GetFirstInstance();

  while (pHandler)
  {
    EventData& ed = s_KnownEvents[pHandler->m_szEventName];

    if (pHandler->m_bOnlyOnce)
      ++ed.m_uiNumEventHandlersOnce;
    else
      ++ed.m_uiNumEventHandlersRegular;

    pHandler = pHandler->GetNextInstance();
  }
}

void wdGlobalEvent::PrintGlobalEventStatistics()
{
  UpdateGlobalEventStatistics();

  WD_LOG_BLOCK("Global Event Statistics");

  EventMap::Iterator it = s_KnownEvents.GetIterator();

  while (it.IsValid())
  {
    wdLog::Info("Event: '{0}', Num Handlers Regular / Once: {1} / {2}, Num Times Fired: {3}", it.Key(), it.Value().m_uiNumEventHandlersRegular,
      it.Value().m_uiNumEventHandlersOnce, it.Value().m_uiNumTimesFired);

    ++it;
  }
}



WD_STATICLINK_FILE(Foundation, Foundation_Communication_Implementation_GlobalEvent);

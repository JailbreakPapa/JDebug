#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/GlobalEvent.h>
#include <Foundation/Logging/Log.h>

NS_ENUMERABLE_CLASS_IMPLEMENTATION(nsGlobalEvent);

nsGlobalEvent::EventMap nsGlobalEvent::s_KnownEvents;

nsGlobalEvent::EventData::EventData()
{
  m_uiNumTimesFired = 0;
  m_uiNumEventHandlersOnce = 0;
  m_uiNumEventHandlersRegular = 0;
}

nsGlobalEvent::nsGlobalEvent(nsStringView sEventName, NS_GLOBAL_EVENT_HANDLER handler, bool bOnlyOnce)
{
  m_sEventName = sEventName;
  m_bOnlyOnce = bOnlyOnce;
  m_bHasBeenFired = false;
  m_EventHandler = handler;
}

void nsGlobalEvent::Broadcast(nsStringView sEventName, nsVariant p1, nsVariant p2, nsVariant p3, nsVariant p4)
{
  nsGlobalEvent* pHandler = nsGlobalEvent::GetFirstInstance();

  while (pHandler)
  {
    if (pHandler->m_sEventName == sEventName)
    {
      if (!pHandler->m_bOnlyOnce || !pHandler->m_bHasBeenFired)
      {
        pHandler->m_bHasBeenFired = true;

        pHandler->m_EventHandler(p1, p2, p3, p4);
      }
    }

    pHandler = pHandler->GetNextInstance();
  }


  EventData& ed = s_KnownEvents[sEventName]; // this will make sure to record all fired events, even if there are no handlers for them
  ed.m_uiNumTimesFired++;
}

void nsGlobalEvent::UpdateGlobalEventStatistics()
{
  for (EventMap::Iterator it = s_KnownEvents.GetIterator(); it.IsValid(); ++it)
  {
    it.Value().m_uiNumEventHandlersRegular = 0;
    it.Value().m_uiNumEventHandlersOnce = 0;
  }

  nsGlobalEvent* pHandler = nsGlobalEvent::GetFirstInstance();

  while (pHandler)
  {
    EventData& ed = s_KnownEvents[pHandler->m_sEventName];

    if (pHandler->m_bOnlyOnce)
      ++ed.m_uiNumEventHandlersOnce;
    else
      ++ed.m_uiNumEventHandlersRegular;

    pHandler = pHandler->GetNextInstance();
  }
}

void nsGlobalEvent::PrintGlobalEventStatistics()
{
  UpdateGlobalEventStatistics();

  NS_LOG_BLOCK("Global Event Statistics");

  EventMap::Iterator it = s_KnownEvents.GetIterator();

  while (it.IsValid())
  {
    nsLog::Info("Event: '{0}', Num Handlers Regular / Once: {1} / {2}, Num Times Fired: {3}", it.Key(), it.Value().m_uiNumEventHandlersRegular,
      it.Value().m_uiNumEventHandlersOnce, it.Value().m_uiNumTimesFired);

    ++it;
  }
}

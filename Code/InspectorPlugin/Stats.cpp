#include <InspectorPlugin/InspectorPluginPCH.h>

#include <Core/GameApplication/GameApplicationBase.h>
#include <Foundation/Communication/Telemetry.h>
#include <Foundation/Utilities/Stats.h>

static void StatsEventHandler(const nsStats::StatsEventData& e)
{
  if (!nsTelemetry::IsConnectedToClient())
    return;

  nsTelemetry::TransmitMode Mode = nsTelemetry::Reliable;

  switch (e.m_EventType)
  {
    case nsStats::StatsEventData::Set:
      Mode = nsTelemetry::Unreliable;
      // fall-through
    case nsStats::StatsEventData::Add:
    {
      nsTelemetryMessage msg;
      msg.SetMessageID('STAT', ' SET');
      msg.GetWriter() << e.m_sStatName;
      msg.GetWriter() << e.m_NewStatValue;
      msg.GetWriter() << nsTime::Now();

      nsTelemetry::Broadcast(Mode, msg);
    }
    break;
    case nsStats::StatsEventData::Remove:
    {
      nsTelemetryMessage msg;
      msg.SetMessageID('STAT', ' DEL');
      msg.GetWriter() << e.m_sStatName;
      msg.GetWriter() << nsTime::Now();

      nsTelemetry::Broadcast(nsTelemetry::Reliable, msg);
    }
    break;
  }
}


static void SendAllStatsTelemetry()
{
  if (!nsTelemetry::IsConnectedToClient())
    return;

  for (nsStats::MapType::ConstIterator it = nsStats::GetAllStats().GetIterator(); it.IsValid(); ++it)
  {
    nsTelemetryMessage msg;
    msg.SetMessageID('STAT', ' SET');
    msg.GetWriter() << it.Key().GetData();
    msg.GetWriter() << it.Value();

    nsTelemetry::Broadcast(nsTelemetry::Reliable, msg);
  }
}

static void TelemetryEventsHandler(const nsTelemetry::TelemetryEventData& e)
{
  switch (e.m_EventType)
  {
    case nsTelemetry::TelemetryEventData::ConnectedToClient:
      SendAllStatsTelemetry();
      break;

    default:
      break;
  }
}

static void PerFrameUpdateHandler(const nsGameApplicationExecutionEvent& e)
{
  switch (e.m_Type)
  {
    case nsGameApplicationExecutionEvent::Type::AfterPresent:
    {
      nsTime FrameTime;

      if (nsGameApplicationBase::GetGameApplicationBaseInstance() != nullptr)
      {
        FrameTime = nsGameApplicationBase::GetGameApplicationBaseInstance()->GetFrameTime();
      }

      nsStringBuilder s;
      nsStats::SetStat("App/FrameTime[ms]", FrameTime.GetMilliseconds());
      nsStats::SetStat("App/FPS", 1.0 / FrameTime.GetSeconds());

      nsStats::SetStat("App/Active Threads", nsOSThread::GetThreadCount());

      // Tasksystem Thread Utilization
      {
        for (nsUInt32 t = 0; t < nsTaskSystem::GetWorkerThreadCount(nsWorkerThreadType::ShortTasks); ++t)
        {
          nsUInt32 uiNumTasks = 0;
          const double Utilization = nsTaskSystem::GetThreadUtilization(nsWorkerThreadType::ShortTasks, t, &uiNumTasks);

          s.SetFormat("Utilization/Short{0}_Load[%%]", nsArgI(t, 2, true));
          nsStats::SetStat(s.GetData(), Utilization * 100.0);

          s.SetFormat("Utilization/Short{0}_Tasks", nsArgI(t, 2, true));
          nsStats::SetStat(s.GetData(), uiNumTasks);
        }

        for (nsUInt32 t = 0; t < nsTaskSystem::GetWorkerThreadCount(nsWorkerThreadType::LongTasks); ++t)
        {
          nsUInt32 uiNumTasks = 0;
          const double Utilization = nsTaskSystem::GetThreadUtilization(nsWorkerThreadType::LongTasks, t, &uiNumTasks);

          s.SetFormat("Utilization/Long{0}_Load[%%]", nsArgI(t, 2, true));
          nsStats::SetStat(s.GetData(), Utilization * 100.0);

          s.SetFormat("Utilization/Long{0}_Tasks", nsArgI(t, 2, true));
          nsStats::SetStat(s.GetData(), uiNumTasks);
        }

        for (nsUInt32 t = 0; t < nsTaskSystem::GetWorkerThreadCount(nsWorkerThreadType::FileAccess); ++t)
        {
          nsUInt32 uiNumTasks = 0;
          const double Utilization = nsTaskSystem::GetThreadUtilization(nsWorkerThreadType::FileAccess, t, &uiNumTasks);

          s.SetFormat("Utilization/File{0}_Load[%%]", nsArgI(t, 2, true));
          nsStats::SetStat(s.GetData(), Utilization * 100.0);

          s.SetFormat("Utilization/File{0}_Tasks", nsArgI(t, 2, true));
          nsStats::SetStat(s.GetData(), uiNumTasks);
        }
      }
    }
    break;

    default:
      break;
  }
}

void AddStatsEventHandler()
{
  nsStats::AddEventHandler(StatsEventHandler);

  nsTelemetry::AddEventHandler(TelemetryEventsHandler);

  // We're handling the per frame update by a different event since
  // using nsTelemetry::TelemetryEventData::PerFrameUpdate can lead
  // to deadlocks between the nsStats and nsTelemetry system.
  if (nsGameApplicationBase::GetGameApplicationBaseInstance() != nullptr)
  {
    nsGameApplicationBase::GetGameApplicationBaseInstance()->m_ExecutionEvents.AddEventHandler(PerFrameUpdateHandler);
  }
}

void RemoveStatsEventHandler()
{
  if (nsGameApplicationBase::GetGameApplicationBaseInstance() != nullptr)
  {
    nsGameApplicationBase::GetGameApplicationBaseInstance()->m_ExecutionEvents.RemoveEventHandler(PerFrameUpdateHandler);
  }

  nsTelemetry::RemoveEventHandler(TelemetryEventsHandler);

  nsStats::RemoveEventHandler(StatsEventHandler);
}



NS_STATICLINK_FILE(InspectorPlugin, InspectorPlugin_Stats);

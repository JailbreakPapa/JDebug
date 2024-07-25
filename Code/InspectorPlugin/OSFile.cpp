#include <InspectorPlugin/InspectorPluginPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Threading/TaskSystem.h>
#include <Foundation/Threading/ThreadUtils.h>

static void OSFileEventHandler(const nsOSFile::EventData& e)
{
  if (!nsTelemetry::IsConnectedToClient())
    return;

  nsTelemetryMessage Msg;
  Msg.GetWriter() << e.m_iFileID;

  switch (e.m_EventType)
  {
    case nsOSFile::EventType::FileOpen:
    {
      Msg.SetMessageID('FILE', 'OPEN');
      Msg.GetWriter() << e.m_sFile;
      Msg.GetWriter() << (nsUInt8)e.m_FileMode;
      Msg.GetWriter() << e.m_bSuccess;
    }
    break;

    case nsOSFile::EventType::FileRead:
    {
      Msg.SetMessageID('FILE', 'READ');
      Msg.GetWriter() << e.m_uiBytesAccessed;
    }
    break;

    case nsOSFile::EventType::FileWrite:
    {
      Msg.SetMessageID('FILE', 'WRIT');
      Msg.GetWriter() << e.m_uiBytesAccessed;
      Msg.GetWriter() << e.m_bSuccess;
    }
    break;

    case nsOSFile::EventType::FileClose:
    {
      Msg.SetMessageID('FILE', 'CLOS');
    }
    break;

    case nsOSFile::EventType::FileExists:
    case nsOSFile::EventType::DirectoryExists:
    {
      Msg.SetMessageID('FILE', 'EXST');
      Msg.GetWriter() << e.m_sFile;
      Msg.GetWriter() << e.m_bSuccess;
    }
    break;

    case nsOSFile::EventType::FileDelete:
    {
      Msg.SetMessageID('FILE', ' DEL');
      Msg.GetWriter() << e.m_sFile;
      Msg.GetWriter() << e.m_bSuccess;
    }
    break;

    case nsOSFile::EventType::MakeDir:
    {
      Msg.SetMessageID('FILE', 'CDIR');
      Msg.GetWriter() << e.m_sFile;
      Msg.GetWriter() << e.m_bSuccess;
    }
    break;

    case nsOSFile::EventType::FileCopy:
    {
      Msg.SetMessageID('FILE', 'COPY');
      Msg.GetWriter() << e.m_sFile;
      Msg.GetWriter() << e.m_sFile2;
      Msg.GetWriter() << e.m_bSuccess;
    }
    break;

    case nsOSFile::EventType::FileStat:
    {
      Msg.SetMessageID('FILE', 'STAT');
      Msg.GetWriter() << e.m_sFile;
      Msg.GetWriter() << e.m_bSuccess;
    }
    break;

    case nsOSFile::EventType::FileCasing:
    {
      Msg.SetMessageID('FILE', 'CASE');
      Msg.GetWriter() << e.m_sFile;
      Msg.GetWriter() << e.m_bSuccess;
    }
    break;

    case nsOSFile::EventType::None:
      break;
  }

  nsUInt8 uiThreadType = 0;

  if (nsThreadUtils::IsMainThread())
    uiThreadType = 1 << 0;
  else if (nsTaskSystem::GetCurrentThreadWorkerType() == nsWorkerThreadType::FileAccess)
    uiThreadType = 1 << 1;
  else
    uiThreadType = 1 << 2;

  Msg.GetWriter() << e.m_Duration.GetSeconds();
  Msg.GetWriter() << uiThreadType;

  nsTelemetry::Broadcast(nsTelemetry::Reliable, Msg);
}

void AddOSFileEventHandler()
{
  nsOSFile::AddEventHandler(OSFileEventHandler);
}

void RemoveOSFileEventHandler()
{
  nsOSFile::RemoveEventHandler(OSFileEventHandler);
}



NS_STATICLINK_FILE(InspectorPlugin, InspectorPlugin_OSFile);

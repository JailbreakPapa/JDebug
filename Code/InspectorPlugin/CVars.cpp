#include <InspectorPlugin/InspectorPluginPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <Foundation/Configuration/CVar.h>

static void TelemetryMessage(void* pPassThrough)
{
  nsTelemetryMessage Msg;

  while (nsTelemetry::RetrieveMessage('SVAR', Msg) == NS_SUCCESS)
  {
    if (Msg.GetMessageID() == ' SET')
    {
      nsString sCVar;
      nsUInt8 uiType;

      float fValue;
      nsInt32 iValue;
      bool bValue;
      nsString sValue;

      Msg.GetReader() >> sCVar;
      Msg.GetReader() >> uiType;

      switch (uiType)
      {
        case nsCVarType::Float:
          Msg.GetReader() >> fValue;
          break;
        case nsCVarType::Int:
          Msg.GetReader() >> iValue;
          break;
        case nsCVarType::Bool:
          Msg.GetReader() >> bValue;
          break;
        case nsCVarType::String:
          Msg.GetReader() >> sValue;
          break;
      }

      nsCVar* pCVar = nsCVar::GetFirstInstance();

      while (pCVar)
      {
        if (((nsUInt8)pCVar->GetType() == uiType) && (pCVar->GetName() == sCVar))
        {
          switch (uiType)
          {
            case nsCVarType::Float:
              *((nsCVarFloat*)pCVar) = fValue;
              break;
            case nsCVarType::Int:
              *((nsCVarInt*)pCVar) = iValue;
              break;
            case nsCVarType::Bool:
              *((nsCVarBool*)pCVar) = bValue;
              break;
            case nsCVarType::String:
              *((nsCVarString*)pCVar) = sValue;
              break;
          }
        }

        pCVar = pCVar->GetNextInstance();
      }
    }
  }
}

static void SendCVarTelemetry(nsCVar* pCVar)
{
  nsTelemetryMessage msg;
  msg.SetMessageID('CVAR', 'DATA');
  msg.GetWriter() << pCVar->GetName();
  msg.GetWriter() << pCVar->GetPluginName();
  // msg.GetWriter() << (nsUInt8) pCVar->GetFlags().GetValue(); // currently not used
  msg.GetWriter() << (nsUInt8)pCVar->GetType();
  msg.GetWriter() << pCVar->GetDescription();

  switch (pCVar->GetType())
  {
    case nsCVarType::Float:
    {
      const float val = ((nsCVarFloat*)pCVar)->GetValue();
      msg.GetWriter() << val;
    }
    break;
    case nsCVarType::Int:
    {
      const int val = ((nsCVarInt*)pCVar)->GetValue();
      msg.GetWriter() << val;
    }
    break;
    case nsCVarType::Bool:
    {
      const bool val = ((nsCVarBool*)pCVar)->GetValue();
      msg.GetWriter() << val;
    }
    break;
    case nsCVarType::String:
    {
      nsStringView val = ((nsCVarString*)pCVar)->GetValue();
      msg.GetWriter() << val;
    }
    break;

    case nsCVarType::ENUM_COUNT:
      NS_ASSERT_NOT_IMPLEMENTED;
      break;
  }

  nsTelemetry::Broadcast(nsTelemetry::Reliable, msg);
}

static void SendAllCVarTelemetry()
{
  if (!nsTelemetry::IsConnectedToClient())
    return;

  // clear
  {
    nsTelemetryMessage msg;
    nsTelemetry::Broadcast(nsTelemetry::Reliable, 'CVAR', ' CLR', nullptr, 0);
  }

  nsCVar* pCVar = nsCVar::GetFirstInstance();

  while (pCVar)
  {
    SendCVarTelemetry(pCVar);

    pCVar = pCVar->GetNextInstance();
  }

  {
    nsTelemetryMessage msg;
    nsTelemetry::Broadcast(nsTelemetry::Reliable, 'CVAR', 'SYNC', nullptr, 0);
  }
}

namespace CVarsDetail
{

  static void TelemetryEventsHandler(const nsTelemetry::TelemetryEventData& e)
  {
    switch (e.m_EventType)
    {
      case nsTelemetry::TelemetryEventData::ConnectedToClient:
        SendAllCVarTelemetry();
        break;

      default:
        break;
    }
  }

  static void CVarEventHandler(const nsCVarEvent& e)
  {
    if (!nsTelemetry::IsConnectedToClient())
      return;

    switch (e.m_EventType)
    {
      case nsCVarEvent::ValueChanged:
        SendCVarTelemetry(e.m_pCVar);
        break;

      case nsCVarEvent::ListOfVarsChanged:
        SendAllCVarTelemetry();
        break;

      default:
        break;
    }
  }

  static void PluginEventHandler(const nsPluginEvent& e)
  {
    switch (e.m_EventType)
    {
      case nsPluginEvent::AfterPluginChanges:
        SendAllCVarTelemetry();
        break;

      default:
        break;
    }
  }
} // namespace CVarsDetail

void AddCVarEventHandler()
{
  nsTelemetry::AddEventHandler(CVarsDetail::TelemetryEventsHandler);
  nsTelemetry::AcceptMessagesForSystem('SVAR', true, TelemetryMessage, nullptr);

  nsCVar::s_AllCVarEvents.AddEventHandler(CVarsDetail::CVarEventHandler);
  nsPlugin::Events().AddEventHandler(CVarsDetail::PluginEventHandler);
}

void RemoveCVarEventHandler()
{
  nsPlugin::Events().RemoveEventHandler(CVarsDetail::PluginEventHandler);
  nsCVar::s_AllCVarEvents.RemoveEventHandler(CVarsDetail::CVarEventHandler);

  nsTelemetry::RemoveEventHandler(CVarsDetail::TelemetryEventsHandler);
  nsTelemetry::AcceptMessagesForSystem('SVAR', false);
}



NS_STATICLINK_FILE(InspectorPlugin, InspectorPlugin_CVars);

#include <InspectorPlugin/InspectorPluginPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <Foundation/Reflection/Reflection.h>

namespace ReflectionDetail
{

  static void SendBasicTypesGroup()
  {
    nsTelemetryMessage msg;
    msg.SetMessageID('RFLC', 'DATA');
    msg.GetWriter() << "Basic Types";
    msg.GetWriter() << "";
    msg.GetWriter() << 0;
    msg.GetWriter() << "";
    msg.GetWriter() << (nsUInt32)0U;
    msg.GetWriter() << (nsUInt32)0U;

    nsTelemetry::Broadcast(nsTelemetry::Reliable, msg);
  }

  static nsStringView GetParentType(const nsRTTI* pRTTI)
  {
    if (pRTTI->GetParentType())
    {
      return pRTTI->GetParentType()->GetTypeName();
    }

    if ((pRTTI->GetTypeName() == "bool") || (pRTTI->GetTypeName() == "float") ||
        (pRTTI->GetTypeName() == "double") || (pRTTI->GetTypeName() == "nsInt8") ||
        (pRTTI->GetTypeName() == "nsUInt8") || (pRTTI->GetTypeName() == "nsInt16") ||
        (pRTTI->GetTypeName() == "nsUInt16") || (pRTTI->GetTypeName() == "nsInt32") ||
        (pRTTI->GetTypeName() == "nsUInt32") || (pRTTI->GetTypeName() == "nsInt64") ||
        (pRTTI->GetTypeName() == "nsUInt64") || (pRTTI->GetTypeName() == "nsConstCharPtr") ||
        (pRTTI->GetTypeName() == "nsVec2") || (pRTTI->GetTypeName() == "nsVec3") ||
        (pRTTI->GetTypeName() == "nsVec4") || (pRTTI->GetTypeName() == "nsMat3") ||
        (pRTTI->GetTypeName() == "nsMat4") || (pRTTI->GetTypeName() == "nsTime") ||
        (pRTTI->GetTypeName() == "nsUuid") || (pRTTI->GetTypeName() == "nsColor") ||
        (pRTTI->GetTypeName() == "nsVariant") || (pRTTI->GetTypeName() == "nsQuat"))
    {
      return "Basic Types";
    }

    return {};
  }

  static void SendReflectionTelemetry(const nsRTTI* pRTTI)
  {
    nsTelemetryMessage msg;
    msg.SetMessageID('RFLC', 'DATA');
    msg.GetWriter() << pRTTI->GetTypeName();
    msg.GetWriter() << GetParentType(pRTTI);
    msg.GetWriter() << pRTTI->GetTypeSize();
    msg.GetWriter() << pRTTI->GetPluginName();

    {
      auto properties = pRTTI->GetProperties();

      msg.GetWriter() << properties.GetCount();

      for (auto& prop : properties)
      {
        msg.GetWriter() << prop->GetPropertyName();
        msg.GetWriter() << (nsInt8)prop->GetCategory();

        const nsRTTI* pType = prop->GetSpecificType();
        msg.GetWriter() << (pType ? pType->GetTypeName() : "<Unknown Type>");
      }
    }

    {
      const nsArrayPtr<nsAbstractMessageHandler*>& Messages = pRTTI->GetMessageHandlers();

      msg.GetWriter() << Messages.GetCount();

      for (nsUInt32 i = 0; i < Messages.GetCount(); ++i)
      {
        msg.GetWriter() << Messages[i]->GetMessageId();
      }
    }

    nsTelemetry::Broadcast(nsTelemetry::Reliable, msg);
  }

  static void SendAllReflectionTelemetry()
  {
    if (!nsTelemetry::IsConnectedToClient())
      return;

    // clear
    {
      nsTelemetryMessage msg;
      nsTelemetry::Broadcast(nsTelemetry::Reliable, 'RFLC', ' CLR', nullptr, 0);
    }

    SendBasicTypesGroup();

    nsRTTI::ForEachType([](const nsRTTI* pRtti)
      { SendReflectionTelemetry(pRtti); });
  }


  static void TelemetryEventsHandler(const nsTelemetry::TelemetryEventData& e)
  {
    switch (e.m_EventType)
    {
      case nsTelemetry::TelemetryEventData::ConnectedToClient:
        SendAllReflectionTelemetry();
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
        SendAllReflectionTelemetry();
        break;

      default:
        break;
    }
  }
} // namespace ReflectionDetail

void AddReflectionEventHandler()
{
  nsTelemetry::AddEventHandler(ReflectionDetail::TelemetryEventsHandler);

  nsPlugin::Events().AddEventHandler(ReflectionDetail::PluginEventHandler);
}

void RemoveReflectionEventHandler()
{
  nsPlugin::Events().RemoveEventHandler(ReflectionDetail::PluginEventHandler);

  nsTelemetry::RemoveEventHandler(ReflectionDetail::TelemetryEventsHandler);
}



NS_STATICLINK_FILE(InspectorPlugin, InspectorPlugin_Reflection);

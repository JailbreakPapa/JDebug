#include <InspectorPlugin/InspectorPluginPCH.h>

#include <Core/Input/InputManager.h>
#include <Foundation/Communication/Telemetry.h>

namespace InputDetail
{

  static void SendInputSlotData(nsStringView sInputSlot)
  {
    float fValue = 0.0f;

    nsTelemetryMessage msg;
    msg.SetMessageID('INPT', 'SLOT');
    msg.GetWriter() << sInputSlot;
    msg.GetWriter() << nsInputManager::GetInputSlotFlags(sInputSlot).GetValue();
    msg.GetWriter() << (nsUInt8)nsInputManager::GetInputSlotState(sInputSlot, &fValue);
    msg.GetWriter() << fValue;
    msg.GetWriter() << nsInputManager::GetInputSlotDeadZone(sInputSlot);

    nsTelemetry::Broadcast(nsTelemetry::Reliable, msg);
  }

  static void SendInputActionData(nsStringView sInputSet, nsStringView sInputAction)
  {
    float fValue = 0.0f;

    const nsInputActionConfig cfg = nsInputManager::GetInputActionConfig(sInputSet, sInputAction);

    nsTelemetryMessage msg;
    msg.SetMessageID('INPT', 'ACTN');
    msg.GetWriter() << sInputSet;
    msg.GetWriter() << sInputAction;
    msg.GetWriter() << (nsUInt8)nsInputManager::GetInputActionState(sInputSet, sInputAction, &fValue);
    msg.GetWriter() << fValue;
    msg.GetWriter() << cfg.m_bApplyTimeScaling;

    for (nsUInt32 i = 0; i < nsInputActionConfig::MaxInputSlotAlternatives; ++i)
    {
      msg.GetWriter() << cfg.m_sInputSlotTrigger[i];
      msg.GetWriter() << cfg.m_fInputSlotScale[i];
    }

    nsTelemetry::Broadcast(nsTelemetry::Reliable, msg);
  }

  static void SendAllInputSlots()
  {
    nsDynamicArray<nsStringView> InputSlots;
    nsInputManager::RetrieveAllKnownInputSlots(InputSlots);

    for (nsUInt32 i = 0; i < InputSlots.GetCount(); ++i)
    {
      SendInputSlotData(InputSlots[i]);
    }
  }

  static void SendAllInputActions()
  {
    nsDynamicArray<nsString> InputSetNames;
    nsInputManager::GetAllInputSets(InputSetNames);

    for (nsUInt32 s = 0; s < InputSetNames.GetCount(); ++s)
    {
      nsHybridArray<nsString, 24> InputActions;

      nsInputManager::GetAllInputActions(InputSetNames[s].GetData(), InputActions);

      for (nsUInt32 a = 0; a < InputActions.GetCount(); ++a)
        SendInputActionData(InputSetNames[s].GetData(), InputActions[a].GetData());
    }
  }

  static void TelemetryEventsHandler(const nsTelemetry::TelemetryEventData& e)
  {
    if (!nsTelemetry::IsConnectedToClient())
      return;

    switch (e.m_EventType)
    {
      case nsTelemetry::TelemetryEventData::ConnectedToClient:
        SendAllInputSlots();
        SendAllInputActions();
        break;

      default:
        break;
    }
  }

  static void InputManagerEventHandler(const nsInputManager::InputEventData& e)
  {
    if (!nsTelemetry::IsConnectedToClient())
      return;

    switch (e.m_EventType)
    {
      case nsInputManager::InputEventData::InputActionChanged:
        SendInputActionData(e.m_sInputSet, e.m_sInputAction);
        break;
      case nsInputManager::InputEventData::InputSlotChanged:
        SendInputSlotData(e.m_sInputSlot);
        break;

      default:
        break;
    }
  }
} // namespace InputDetail

void AddInputEventHandler()
{
  nsTelemetry::AddEventHandler(InputDetail::TelemetryEventsHandler);
  nsInputManager::AddEventHandler(InputDetail::InputManagerEventHandler);
}

void RemoveInputEventHandler()
{
  nsInputManager::RemoveEventHandler(InputDetail::InputManagerEventHandler);
  nsTelemetry::RemoveEventHandler(InputDetail::TelemetryEventsHandler);
}



NS_STATICLINK_FILE(InspectorPlugin, InspectorPlugin_Input);

#include <Core/CorePCH.h>

#include <Core/Input/InputManager.h>

nsInputManager::nsEventInput nsInputManager::s_InputEvents;
nsInputManager::InternalData* nsInputManager::s_pData = nullptr;
nsUInt32 nsInputManager::s_uiLastCharacter = '\0';
bool nsInputManager::s_bInputSlotResetRequired = true;
nsString nsInputManager::s_sExclusiveInputSet;

nsInputManager::InternalData& nsInputManager::GetInternals()
{
  if (s_pData == nullptr)
    s_pData = NS_DEFAULT_NEW(InternalData);

  return *s_pData;
}

void nsInputManager::DeallocateInternals()
{
  NS_DEFAULT_DELETE(s_pData);
}

nsInputManager::nsInputSlot::nsInputSlot()
{
  m_fValue = 0.0f;
  m_State = nsKeyState::Up;
  m_fDeadZone = 0.0f;
}

void nsInputManager::RegisterInputSlot(nsStringView sInputSlot, nsStringView sDefaultDisplayName, nsBitflags<nsInputSlotFlags> SlotFlags)
{
  nsMap<nsString, nsInputSlot>::Iterator it = GetInternals().s_InputSlots.Find(sInputSlot);

  if (it.IsValid())
  {
    if (it.Value().m_SlotFlags != SlotFlags)
    {
      if ((it.Value().m_SlotFlags != nsInputSlotFlags::Default) && (SlotFlags != nsInputSlotFlags::Default))
      {
        nsStringBuilder tmp, tmp2;
        tmp.SetPrintf("Different devices register Input Slot '%s' with different Slot Flags: %16b vs. %16b",
          sInputSlot.GetData(tmp2), it.Value().m_SlotFlags.GetValue(), SlotFlags.GetValue());

        nsLog::Warning(tmp);
      }

      it.Value().m_SlotFlags |= SlotFlags;
    }

    // If the key already exists, but key and display string are identical, then overwrite the display string with the incoming string
    if (it.Value().m_sDisplayName != it.Key())
      return;
  }

  // nsLog::Debug("Registered Input Slot: '{0}'", sInputSlot);

  nsInputSlot& sm = GetInternals().s_InputSlots[sInputSlot];

  sm.m_sDisplayName = sDefaultDisplayName;
  sm.m_SlotFlags = SlotFlags;

  InputEventData e;
  e.m_EventType = InputEventData::InputSlotChanged;
  e.m_sInputSlot = sInputSlot;

  s_InputEvents.Broadcast(e);
}

nsBitflags<nsInputSlotFlags> nsInputManager::GetInputSlotFlags(nsStringView sInputSlot)
{
  nsMap<nsString, nsInputSlot>::ConstIterator it = GetInternals().s_InputSlots.Find(sInputSlot);

  if (it.IsValid())
    return it.Value().m_SlotFlags;

  nsLog::Warning("nsInputManager::GetInputSlotFlags: Input Slot '{0}' does not exist (yet).", sInputSlot);

  return nsInputSlotFlags::Default;
}

void nsInputManager::SetInputSlotDisplayName(nsStringView sInputSlot, nsStringView sDefaultDisplayName)
{
  RegisterInputSlot(sInputSlot, sDefaultDisplayName, nsInputSlotFlags::Default);
  GetInternals().s_InputSlots[sInputSlot].m_sDisplayName = sDefaultDisplayName;

  InputEventData e;
  e.m_EventType = InputEventData::InputSlotChanged;
  e.m_sInputSlot = sInputSlot;

  s_InputEvents.Broadcast(e);
}

nsStringView nsInputManager::GetInputSlotDisplayName(nsStringView sInputSlot)
{
  nsMap<nsString, nsInputSlot>::ConstIterator it = GetInternals().s_InputSlots.Find(sInputSlot);

  if (it.IsValid())
    return it.Value().m_sDisplayName.GetData();

  nsLog::Warning("nsInputManager::GetInputSlotDisplayName: Input Slot '{0}' does not exist (yet).", sInputSlot);
  return sInputSlot;
}

nsStringView nsInputManager::GetInputSlotDisplayName(nsStringView sInputSet, nsStringView sAction, nsInt32 iTrigger)
{
  /// \test This is new

  const auto cfg = GetInputActionConfig(sInputSet, sAction);

  if (iTrigger < 0)
  {
    for (iTrigger = 0; iTrigger < nsInputActionConfig::MaxInputSlotAlternatives; ++iTrigger)
    {
      if (!cfg.m_sInputSlotTrigger[iTrigger].IsEmpty())
        break;
    }
  }

  if (iTrigger >= nsInputActionConfig::MaxInputSlotAlternatives)
    return nullptr;

  return GetInputSlotDisplayName(cfg.m_sInputSlotTrigger[iTrigger]);
}

void nsInputManager::SetInputSlotDeadZone(nsStringView sInputSlot, float fDeadZone)
{
  RegisterInputSlot(sInputSlot, sInputSlot, nsInputSlotFlags::Default);
  GetInternals().s_InputSlots[sInputSlot].m_fDeadZone = nsMath::Max(fDeadZone, 0.0001f);

  InputEventData e;
  e.m_EventType = InputEventData::InputSlotChanged;
  e.m_sInputSlot = sInputSlot;

  s_InputEvents.Broadcast(e);
}

float nsInputManager::GetInputSlotDeadZone(nsStringView sInputSlot)
{
  nsMap<nsString, nsInputSlot>::ConstIterator it = GetInternals().s_InputSlots.Find(sInputSlot);

  if (it.IsValid())
    return it.Value().m_fDeadZone;

  nsLog::Warning("nsInputManager::GetInputSlotDeadZone: Input Slot '{0}' does not exist (yet).", sInputSlot);

  nsInputSlot s;
  return s.m_fDeadZone; // return the default value
}

nsKeyState::Enum nsInputManager::GetInputSlotState(nsStringView sInputSlot, float* pValue)
{
  nsMap<nsString, nsInputSlot>::ConstIterator it = GetInternals().s_InputSlots.Find(sInputSlot);

  if (it.IsValid())
  {
    if (pValue)
    {
      *pValue = s_bInputSlotResetRequired ? it.Value().m_fValue : it.Value().m_fValueOld;
    }
    return it.Value().m_State;
  }

  if (pValue)
    *pValue = 0.0f;

  nsLog::Warning("nsInputManager::GetInputSlotState: Input Slot '{0}' does not exist (yet). To ensure all devices are initialized, call "
                 "nsInputManager::Update before querying device states, or at least call nsInputManager::PollHardware.",
    sInputSlot);

  RegisterInputSlot(sInputSlot, sInputSlot, nsInputSlotFlags::None);

  return nsKeyState::Up;
}

void nsInputManager::PollHardware()
{
  if (s_bInputSlotResetRequired)
  {
    s_bInputSlotResetRequired = false;
    ResetInputSlotValues();
  }

  nsInputDevice::UpdateAllDevices();

  GatherDeviceInputSlotValues();
}

void nsInputManager::Update(nsTime timeDifference)
{
  PollHardware();

  UpdateInputSlotStates();

  s_uiLastCharacter = nsInputDevice::RetrieveLastCharacterFromAllDevices();

  UpdateInputActions(timeDifference);

  nsInputDevice::ResetAllDevices();

  nsInputDevice::UpdateAllHardwareStates(timeDifference);

  s_bInputSlotResetRequired = true;
}

void nsInputManager::ResetInputSlotValues()
{
  // set all input slot values to zero
  // this is crucial for accumulating the new values and for resetting the input state later
  for (nsInputSlotsMap::Iterator it = GetInternals().s_InputSlots.GetIterator(); it.IsValid(); it.Next())
  {
    it.Value().m_fValueOld = it.Value().m_fValue;
    it.Value().m_fValue = 0.0f;
  }
}

void nsInputManager::GatherDeviceInputSlotValues()
{
  for (nsInputDevice* pDevice = nsInputDevice::GetFirstInstance(); pDevice != nullptr; pDevice = pDevice->GetNextInstance())
  {
    pDevice->m_bGeneratedInputRecently = false;

    // iterate over all the input slots that this device provides
    for (auto it = pDevice->m_InputSlotValues.GetIterator(); it.IsValid(); it.Next())
    {
      if (it.Value() > 0.0f)
      {
        nsInputManager::nsInputSlot& Slot = GetInternals().s_InputSlots[it.Key()];

        // do not store a value larger than 0 unless it exceeds the dead-zone threshold
        if (it.Value() > Slot.m_fDeadZone)
        {
          Slot.m_fValue = nsMath::Max(Slot.m_fValue, it.Value()); // 'accumulate' the values for one slot from all the connected devices

          pDevice->m_bGeneratedInputRecently = true;
        }
      }
    }
  }

  nsMap<nsString, float>::Iterator it = GetInternals().s_InjectedInputSlots.GetIterator();

  for (; it.IsValid(); ++it)
  {
    nsInputManager::nsInputSlot& Slot = GetInternals().s_InputSlots[it.Key()];

    // do not store a value larger than 0 unless it exceeds the dead-zone threshold
    if (it.Value() > Slot.m_fDeadZone)
      Slot.m_fValue = nsMath::Max(Slot.m_fValue, it.Value()); // 'accumulate' the values for one slot from all the connected devices
  }

  GetInternals().s_InjectedInputSlots.Clear();
}

void nsInputManager::UpdateInputSlotStates()
{
  for (nsInputSlotsMap::Iterator it = GetInternals().s_InputSlots.GetIterator(); it.IsValid(); it.Next())
  {
    // update the state of the input slot, depending on its current value
    // its value will only be larger than zero, if it is also larger than its dead-zone value
    const nsKeyState::Enum NewState = nsKeyState::GetNewKeyState(it.Value().m_State, it.Value().m_fValue > 0.0f);

    if ((it.Value().m_State != NewState) || (NewState != nsKeyState::Up))
    {
      it.Value().m_State = NewState;

      InputEventData e;
      e.m_EventType = InputEventData::InputSlotChanged;
      e.m_sInputSlot = it.Key().GetData();

      s_InputEvents.Broadcast(e);
    }
  }
}

void nsInputManager::RetrieveAllKnownInputSlots(nsDynamicArray<nsStringView>& out_inputSlots)
{
  out_inputSlots.Clear();
  out_inputSlots.Reserve(GetInternals().s_InputSlots.GetCount());

  // just copy all slot names into the given array
  for (nsInputSlotsMap::Iterator it = GetInternals().s_InputSlots.GetIterator(); it.IsValid(); it.Next())
  {
    out_inputSlots.PushBack(it.Key().GetData());
  }
}

nsUInt32 nsInputManager::RetrieveLastCharacter(bool bResetCurrent)
{
  if (!bResetCurrent)
    return s_uiLastCharacter;

  nsUInt32 Temp = s_uiLastCharacter;
  s_uiLastCharacter = L'\0';
  return Temp;
}

void nsInputManager::InjectInputSlotValue(nsStringView sInputSlot, float fValue)
{
  GetInternals().s_InjectedInputSlots[sInputSlot] = nsMath::Max(GetInternals().s_InjectedInputSlots[sInputSlot], fValue);
}

nsStringView nsInputManager::GetPressedInputSlot(nsInputSlotFlags::Enum mustHaveFlags, nsInputSlotFlags::Enum mustNotHaveFlags)
{
  for (nsInputSlotsMap::Iterator it = GetInternals().s_InputSlots.GetIterator(); it.IsValid(); ++it)
  {
    if (it.Value().m_State != nsKeyState::Pressed)
      continue;

    if (it.Value().m_SlotFlags.IsAnySet(mustNotHaveFlags))
      continue;

    if (it.Value().m_SlotFlags.AreAllSet(mustHaveFlags))
      return it.Key().GetData();
  }

  return nsInputSlot_None;
}

nsStringView nsInputManager::GetInputSlotTouchPoint(nsUInt32 uiIndex)
{
  switch (uiIndex)
  {
    case 0:
      return nsInputSlot_TouchPoint0;
    case 1:
      return nsInputSlot_TouchPoint1;
    case 2:
      return nsInputSlot_TouchPoint2;
    case 3:
      return nsInputSlot_TouchPoint3;
    case 4:
      return nsInputSlot_TouchPoint4;
    case 5:
      return nsInputSlot_TouchPoint5;
    case 6:
      return nsInputSlot_TouchPoint6;
    case 7:
      return nsInputSlot_TouchPoint7;
    case 8:
      return nsInputSlot_TouchPoint8;
    case 9:
      return nsInputSlot_TouchPoint9;
    default:
      NS_REPORT_FAILURE("Maximum number of supported input touch points is 10");
      return "";
  }
}

nsStringView nsInputManager::GetInputSlotTouchPointPositionX(nsUInt32 uiIndex)
{
  switch (uiIndex)
  {
    case 0:
      return nsInputSlot_TouchPoint0_PositionX;
    case 1:
      return nsInputSlot_TouchPoint1_PositionX;
    case 2:
      return nsInputSlot_TouchPoint2_PositionX;
    case 3:
      return nsInputSlot_TouchPoint3_PositionX;
    case 4:
      return nsInputSlot_TouchPoint4_PositionX;
    case 5:
      return nsInputSlot_TouchPoint5_PositionX;
    case 6:
      return nsInputSlot_TouchPoint6_PositionX;
    case 7:
      return nsInputSlot_TouchPoint7_PositionX;
    case 8:
      return nsInputSlot_TouchPoint8_PositionX;
    case 9:
      return nsInputSlot_TouchPoint9_PositionX;
    default:
      NS_REPORT_FAILURE("Maximum number of supported input touch points is 10");
      return "";
  }
}

nsStringView nsInputManager::GetInputSlotTouchPointPositionY(nsUInt32 uiIndex)
{
  switch (uiIndex)
  {
    case 0:
      return nsInputSlot_TouchPoint0_PositionY;
    case 1:
      return nsInputSlot_TouchPoint1_PositionY;
    case 2:
      return nsInputSlot_TouchPoint2_PositionY;
    case 3:
      return nsInputSlot_TouchPoint3_PositionY;
    case 4:
      return nsInputSlot_TouchPoint4_PositionY;
    case 5:
      return nsInputSlot_TouchPoint5_PositionY;
    case 6:
      return nsInputSlot_TouchPoint6_PositionY;
    case 7:
      return nsInputSlot_TouchPoint7_PositionY;
    case 8:
      return nsInputSlot_TouchPoint8_PositionY;
    case 9:
      return nsInputSlot_TouchPoint9_PositionY;
    default:
      NS_REPORT_FAILURE("Maximum number of supported input touch points is 10");
      return "";
  }
}

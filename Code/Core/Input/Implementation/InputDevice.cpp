#include <Core/CorePCH.h>

#include <Core/Input/InputManager.h>

NS_ENUMERABLE_CLASS_IMPLEMENTATION(nsInputDevice);

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsInputDevice, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsKeyState::Enum nsKeyState::GetNewKeyState(nsKeyState::Enum prevState, bool bKeyDown)
{
  switch (prevState)
  {
    case nsKeyState::Down:
    case nsKeyState::Pressed:
      return bKeyDown ? nsKeyState::Down : nsKeyState::Released;
    case nsKeyState::Released:
    case nsKeyState::Up:
      return bKeyDown ? nsKeyState::Pressed : nsKeyState::Up;
  }

  return nsKeyState::Up;
}

nsInputDevice::nsInputDevice()
{
  m_bInitialized = false;
  m_uiLastCharacter = '\0';
}

void nsInputDevice::RegisterInputSlot(nsStringView sName, nsStringView sDefaultDisplayName, nsBitflags<nsInputSlotFlags> SlotFlags)
{
  nsInputManager::RegisterInputSlot(sName, sDefaultDisplayName, SlotFlags);
}

void nsInputDevice::Initialize()
{
  if (m_bInitialized)
    return;

  NS_LOG_BLOCK("Initializing Input Device", GetDynamicRTTI()->GetTypeName());

  nsLog::Dev("Input Device Type: {0}, Device Name: {1}", GetDynamicRTTI()->GetParentType()->GetTypeName(), GetDynamicRTTI()->GetTypeName());

  m_bInitialized = true;

  RegisterInputSlots();
  InitializeDevice();
}


void nsInputDevice::UpdateAllHardwareStates(nsTime tTimeDifference)
{
  // tell each device to update its hardware
  for (nsInputDevice* pDevice = nsInputDevice::GetFirstInstance(); pDevice != nullptr; pDevice = pDevice->GetNextInstance())
  {
    pDevice->UpdateHardwareState(tTimeDifference);
  }
}

void nsInputDevice::UpdateAllDevices()
{
  // tell each device to update its current input slot values
  for (nsInputDevice* pDevice = nsInputDevice::GetFirstInstance(); pDevice != nullptr; pDevice = pDevice->GetNextInstance())
  {
    pDevice->Initialize();
    pDevice->UpdateInputSlotValues();
  }
}

void nsInputDevice::ResetAllDevices()
{
  // tell all devices that the input update is through and they might need to reset some values now
  // this is especially important for device types that will get input messages at some undefined time after this call
  // but not during 'UpdateInputSlotValues'
  for (nsInputDevice* pDevice = nsInputDevice::GetFirstInstance(); pDevice != nullptr; pDevice = pDevice->GetNextInstance())
  {
    pDevice->ResetInputSlotValues();
  }
}

nsUInt32 nsInputDevice::RetrieveLastCharacter()
{
  nsUInt32 Temp = m_uiLastCharacter;
  m_uiLastCharacter = L'\0';
  return Temp;
}

nsUInt32 nsInputDevice::RetrieveLastCharacterFromAllDevices()
{
  for (nsInputDevice* pDevice = nsInputDevice::GetFirstInstance(); pDevice != nullptr; pDevice = pDevice->GetNextInstance())
  {
    const nsUInt32 Char = pDevice->RetrieveLastCharacter();

    if (Char != L'\0')
      return Char;
  }

  return '\0';
}

float nsInputDevice::GetInputSlotState(nsStringView sSlot) const
{
  return m_InputSlotValues.GetValueOrDefault(sSlot, 0.f);
}

bool nsInputDevice::HasDeviceBeenUsedLastFrame() const
{
  return m_bGeneratedInputRecently;
}

NS_STATICLINK_FILE(Core, Core_Input_Implementation_InputDevice);

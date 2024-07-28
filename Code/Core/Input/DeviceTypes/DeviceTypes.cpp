#include <Core/CorePCH.h>

#include <Core/Input/DeviceTypes/Controller.h>
#include <Core/Input/DeviceTypes/MouseKeyboard.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsInputDeviceMouseKeyboard, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsInputDeviceController, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsInt32 nsInputDeviceMouseKeyboard::s_iMouseIsOverWindowNumber = -1;

nsInputDeviceController::nsInputDeviceController()
{
  m_uiVibrationTrackPos = 0;

  for (nsInt8 c = 0; c < MaxControllers; ++c)
  {
    m_bVibrationEnabled[c] = false;
    m_iControllerMapping[c] = c;

    for (nsInt8 m = 0; m < Motor::ENUM_COUNT; ++m)
    {
      m_fVibrationStrength[c][m] = 0.0f;

      for (nsUInt8 t = 0; t < MaxVibrationSamples; ++t)
        m_fVibrationTracks[c][m][t] = 0.0f;
    }
  }
}

void nsInputDeviceController::EnableVibration(nsUInt8 uiVirtual, bool bEnable)
{
  NS_ASSERT_DEV(uiVirtual < MaxControllers, "Controller Index {0} is larger than allowed ({1}).", uiVirtual, MaxControllers);

  m_bVibrationEnabled[uiVirtual] = bEnable;
}

bool nsInputDeviceController::IsVibrationEnabled(nsUInt8 uiVirtual) const
{
  NS_ASSERT_DEV(uiVirtual < MaxControllers, "Controller Index {0} is larger than allowed ({1}).", uiVirtual, MaxControllers);

  return m_bVibrationEnabled[uiVirtual];
}

void nsInputDeviceController::SetVibrationStrength(nsUInt8 uiVirtual, Motor::Enum motor, float fValue)
{
  NS_ASSERT_DEV(uiVirtual < MaxControllers, "Controller Index {0} is larger than allowed ({1}).", uiVirtual, MaxControllers);
  NS_ASSERT_DEV(motor < Motor::ENUM_COUNT, "Invalid Vibration Motor Index.");

  m_fVibrationStrength[uiVirtual][motor] = nsMath::Clamp(fValue, 0.0f, 1.0f);
}

float nsInputDeviceController::GetVibrationStrength(nsUInt8 uiVirtual, Motor::Enum motor)
{
  NS_ASSERT_DEV(uiVirtual < MaxControllers, "Controller Index {0} is larger than allowed ({1}).", uiVirtual, MaxControllers);
  NS_ASSERT_DEV(motor < Motor::ENUM_COUNT, "Invalid Vibration Motor Index.");

  return m_fVibrationStrength[uiVirtual][motor];
}

void nsInputDeviceController::SetControllerMapping(nsUInt8 uiVirtualController, nsInt8 iTakeInputFromPhysical)
{
  NS_ASSERT_DEV(
    uiVirtualController < MaxControllers, "Virtual Controller Index {0} is larger than allowed ({1}).", uiVirtualController, MaxControllers);
  NS_ASSERT_DEV(
    iTakeInputFromPhysical < MaxControllers, "Physical Controller Index {0} is larger than allowed ({1}).", iTakeInputFromPhysical, MaxControllers);

  if (iTakeInputFromPhysical < 0)
  {
    // deactivates this virtual controller
    m_iControllerMapping[uiVirtualController] = -1;
  }
  else
  {
    // if any virtual controller already maps to the given physical controller, let it use the physical controller that
    // uiVirtualController is currently mapped to
    for (nsInt32 c = 0; c < MaxControllers; ++c)
    {
      if (m_iControllerMapping[c] == iTakeInputFromPhysical)
      {
        m_iControllerMapping[c] = m_iControllerMapping[uiVirtualController];
        break;
      }
    }

    m_iControllerMapping[uiVirtualController] = iTakeInputFromPhysical;
  }
}

nsInt8 nsInputDeviceController::GetControllerMapping(nsUInt8 uiVirtual) const
{
  NS_ASSERT_DEV(uiVirtual < MaxControllers, "Virtual Controller Index {0} is larger than allowed ({1}).", uiVirtual, MaxControllers);

  return m_iControllerMapping[uiVirtual];
}

void nsInputDeviceController::AddVibrationTrack(
  nsUInt8 uiVirtual, Motor::Enum motor, float* pVibrationTrackValue, nsUInt32 uiSamples, float fScalingFactor)
{
  uiSamples = nsMath::Min<nsUInt32>(uiSamples, MaxVibrationSamples);

  for (nsUInt32 s = 0; s < uiSamples; ++s)
  {
    float& fVal = m_fVibrationTracks[uiVirtual][motor][(m_uiVibrationTrackPos + 1 + s) % MaxVibrationSamples];

    fVal = nsMath::Max(fVal, pVibrationTrackValue[s] * fScalingFactor);
    fVal = nsMath::Clamp(fVal, 0.0f, 1.0f);
  }
}

void nsInputDeviceController::UpdateVibration(nsTime tTimeDifference)
{
  static nsTime tElapsedTime;
  tElapsedTime += tTimeDifference;

  const nsTime tTimePerSample = nsTime::MakeFromSeconds(1.0 / (double)VibrationSamplesPerSecond);

  // advance the vibration track sampling
  while (tElapsedTime >= tTimePerSample)
  {
    tElapsedTime -= tTimePerSample;

    for (nsUInt32 c = 0; c < MaxControllers; ++c)
    {
      for (nsUInt32 m = 0; m < Motor::ENUM_COUNT; ++m)
        m_fVibrationTracks[c][m][m_uiVibrationTrackPos] = 0.0f;
    }

    m_uiVibrationTrackPos = (m_uiVibrationTrackPos + 1) % MaxVibrationSamples;
  }

  // we will temporarily store how much vibration is to be applied on each physical controller
  float fVibrationToApply[MaxControllers][Motor::ENUM_COUNT];

  // Initialize with zero (we might not set all values later)
  for (nsUInt32 c = 0; c < MaxControllers; ++c)
  {
    for (nsUInt32 m = 0; m < Motor::ENUM_COUNT; ++m)
      fVibrationToApply[c][m] = 0.0f;
  }

  // go through all controllers and motors
  for (nsUInt8 c = 0; c < MaxControllers; ++c)
  {
    // ignore if vibration is disabled on this controller
    if (!m_bVibrationEnabled[c])
      continue;

    // check which physical controller this virtual controller is attached to
    const nsInt8 iPhysical = GetControllerMapping(c);

    // if it is attached to any physical controller, store the vibration value
    if (iPhysical >= 0)
    {
      for (nsUInt32 m = 0; m < Motor::ENUM_COUNT; ++m)
        fVibrationToApply[(nsUInt8)iPhysical][m] = nsMath::Max(m_fVibrationStrength[c][m], m_fVibrationTracks[c][m][m_uiVibrationTrackPos]);
    }
  }

  // now send the back-end all the information about how to vibrate which physical controller
  // this also always resets vibration to zero for controllers that might have been changed to another virtual controller etc.
  for (nsUInt8 c = 0; c < MaxControllers; ++c)
  {
    for (nsUInt32 m = 0; m < Motor::ENUM_COUNT; ++m)
    {
      ApplyVibration(c, (Motor::Enum)m, fVibrationToApply[c][m]);
    }
  }
}

void nsInputDeviceMouseKeyboard::UpdateInputSlotValues()
{
  const char* slots[3] = {nsInputSlot_MouseButton0, nsInputSlot_MouseButton1, nsInputSlot_MouseButton2};
  const char* dlbSlots[3] = {nsInputSlot_MouseDblClick0, nsInputSlot_MouseDblClick1, nsInputSlot_MouseDblClick2};

  const nsTime tNow = nsTime::Now();

  for (int i = 0; i < 3; ++i)
  {
    m_InputSlotValues[dlbSlots[i]] = 0.0f;

    const bool bDown = m_InputSlotValues[slots[i]] > 0;
    if (bDown)
    {
      if (!m_bMouseDown[i])
      {
        if (tNow - m_LastMouseClick[i] <= m_DoubleClickTime)
        {
          m_InputSlotValues[dlbSlots[i]] = 1.0f;
          m_LastMouseClick[i] = nsTime::MakeZero(); // this prevents triple-clicks from appearing as two double clicks
        }
        else
        {
          m_LastMouseClick[i] = tNow;
        }
      }
    }

    m_bMouseDown[i] = bDown;
  }
}

NS_STATICLINK_FILE(Core, Core_Input_DeviceTypes_DeviceTypes);

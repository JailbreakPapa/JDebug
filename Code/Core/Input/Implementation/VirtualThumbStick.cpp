#include <Core/CorePCH.h>

#include <Core/Input/VirtualThumbStick.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsVirtualThumbStick, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsInt32 nsVirtualThumbStick::s_iThumbsticks = 0;

nsVirtualThumbStick::nsVirtualThumbStick()
{
  SetAreaFocusMode(nsInputActionConfig::RequireKeyUp, nsInputActionConfig::KeepFocus);
  SetTriggerInputSlot(nsVirtualThumbStick::Input::Touchpoint);
  SetThumbstickOutput(nsVirtualThumbStick::Output::Controller0_LeftStick);

  SetInputArea(nsVec2(0.0f), nsVec2(0.0f), 0.0f, 0.0f);

  nsStringBuilder s;
  s.SetFormat("Thumbstick_{0}", s_iThumbsticks);
  m_sName = s;

  ++s_iThumbsticks;

  m_bEnabled = false;
  m_bConfigChanged = false;
  m_bIsActive = false;
}

nsVirtualThumbStick::~nsVirtualThumbStick()
{
  nsInputManager::RemoveInputAction(GetDynamicRTTI()->GetTypeName(), m_sName.GetData());
}

void nsVirtualThumbStick::SetTriggerInputSlot(nsVirtualThumbStick::Input::Enum input, const nsInputActionConfig* pCustomConfig)
{
  for (nsInt32 i = 0; i < nsInputActionConfig::MaxInputSlotAlternatives; ++i)
  {
    m_ActionConfig.m_sFilterByInputSlotX[i] = nsInputSlot_None;
    m_ActionConfig.m_sFilterByInputSlotY[i] = nsInputSlot_None;
    m_ActionConfig.m_sInputSlotTrigger[i] = nsInputSlot_None;
  }

  switch (input)
  {
    case nsVirtualThumbStick::Input::Touchpoint:
    {
      m_ActionConfig.m_sFilterByInputSlotX[0] = nsInputSlot_TouchPoint0_PositionX;
      m_ActionConfig.m_sFilterByInputSlotY[0] = nsInputSlot_TouchPoint0_PositionY;
      m_ActionConfig.m_sInputSlotTrigger[0] = nsInputSlot_TouchPoint0;

      m_ActionConfig.m_sFilterByInputSlotX[1] = nsInputSlot_TouchPoint1_PositionX;
      m_ActionConfig.m_sFilterByInputSlotY[1] = nsInputSlot_TouchPoint1_PositionY;
      m_ActionConfig.m_sInputSlotTrigger[1] = nsInputSlot_TouchPoint1;

      m_ActionConfig.m_sFilterByInputSlotX[2] = nsInputSlot_TouchPoint2_PositionX;
      m_ActionConfig.m_sFilterByInputSlotY[2] = nsInputSlot_TouchPoint2_PositionY;
      m_ActionConfig.m_sInputSlotTrigger[2] = nsInputSlot_TouchPoint2;
    }
    break;
    case nsVirtualThumbStick::Input::MousePosition:
    {
      m_ActionConfig.m_sFilterByInputSlotX[0] = nsInputSlot_MousePositionX;
      m_ActionConfig.m_sFilterByInputSlotY[0] = nsInputSlot_MousePositionY;
      m_ActionConfig.m_sInputSlotTrigger[0] = nsInputSlot_MouseButton0;
    }
    break;
    case nsVirtualThumbStick::Input::Custom:
    {
      NS_ASSERT_DEV(pCustomConfig != nullptr, "Must pass a custom config, if you want to have a custom config.");

      for (nsInt32 i = 0; i < nsInputActionConfig::MaxInputSlotAlternatives; ++i)
      {
        m_ActionConfig.m_sFilterByInputSlotX[i] = pCustomConfig->m_sFilterByInputSlotX[i];
        m_ActionConfig.m_sFilterByInputSlotY[i] = pCustomConfig->m_sFilterByInputSlotY[i];
        m_ActionConfig.m_sInputSlotTrigger[i] = pCustomConfig->m_sInputSlotTrigger[i];
      }
    }
    break;
  }

  m_bConfigChanged = true;
}

void nsVirtualThumbStick::SetThumbstickOutput(nsVirtualThumbStick::Output::Enum output, nsStringView sOutputLeft, nsStringView sOutputRight, nsStringView sOutputUp, nsStringView sOutputDown)
{
  switch (output)
  {
    case nsVirtualThumbStick::Output::Controller0_LeftStick:
    {
      m_sOutputLeft = nsInputSlot_Controller0_LeftStick_NegX;
      m_sOutputRight = nsInputSlot_Controller0_LeftStick_PosX;
      m_sOutputUp = nsInputSlot_Controller0_LeftStick_PosY;
      m_sOutputDown = nsInputSlot_Controller0_LeftStick_NegY;
    }
    break;
    case nsVirtualThumbStick::Output::Controller0_RightStick:
    {
      m_sOutputLeft = nsInputSlot_Controller0_RightStick_NegX;
      m_sOutputRight = nsInputSlot_Controller0_RightStick_PosX;
      m_sOutputUp = nsInputSlot_Controller0_RightStick_PosY;
      m_sOutputDown = nsInputSlot_Controller0_RightStick_NegY;
    }
    break;
    case nsVirtualThumbStick::Output::Controller1_LeftStick:
    {
      m_sOutputLeft = nsInputSlot_Controller1_LeftStick_NegX;
      m_sOutputRight = nsInputSlot_Controller1_LeftStick_PosX;
      m_sOutputUp = nsInputSlot_Controller1_LeftStick_PosY;
      m_sOutputDown = nsInputSlot_Controller1_LeftStick_NegY;
    }
    break;
    case nsVirtualThumbStick::Output::Controller1_RightStick:
    {
      m_sOutputLeft = nsInputSlot_Controller1_RightStick_NegX;
      m_sOutputRight = nsInputSlot_Controller1_RightStick_PosX;
      m_sOutputUp = nsInputSlot_Controller1_RightStick_PosY;
      m_sOutputDown = nsInputSlot_Controller1_RightStick_NegY;
    }
    break;
    case nsVirtualThumbStick::Output::Controller2_LeftStick:
    {
      m_sOutputLeft = nsInputSlot_Controller2_LeftStick_NegX;
      m_sOutputRight = nsInputSlot_Controller2_LeftStick_PosX;
      m_sOutputUp = nsInputSlot_Controller2_LeftStick_PosY;
      m_sOutputDown = nsInputSlot_Controller2_LeftStick_NegY;
    }
    break;
    case nsVirtualThumbStick::Output::Controller2_RightStick:
    {
      m_sOutputLeft = nsInputSlot_Controller2_RightStick_NegX;
      m_sOutputRight = nsInputSlot_Controller2_RightStick_PosX;
      m_sOutputUp = nsInputSlot_Controller2_RightStick_PosY;
      m_sOutputDown = nsInputSlot_Controller2_RightStick_NegY;
    }
    break;
    case nsVirtualThumbStick::Output::Controller3_LeftStick:
    {
      m_sOutputLeft = nsInputSlot_Controller3_LeftStick_NegX;
      m_sOutputRight = nsInputSlot_Controller3_LeftStick_PosX;
      m_sOutputUp = nsInputSlot_Controller3_LeftStick_PosY;
      m_sOutputDown = nsInputSlot_Controller3_LeftStick_NegY;
    }
    break;
    case nsVirtualThumbStick::Output::Controller3_RightStick:
    {
      m_sOutputLeft = nsInputSlot_Controller3_RightStick_NegX;
      m_sOutputRight = nsInputSlot_Controller3_RightStick_PosX;
      m_sOutputUp = nsInputSlot_Controller3_RightStick_PosY;
      m_sOutputDown = nsInputSlot_Controller3_RightStick_NegY;
    }
    break;
    case nsVirtualThumbStick::Output::Custom:
    {
      m_sOutputLeft = sOutputLeft;
      m_sOutputRight = sOutputRight;
      m_sOutputUp = sOutputUp;
      m_sOutputDown = sOutputDown;
    }
    break;
  }

  m_bConfigChanged = true;
}

void nsVirtualThumbStick::SetAreaFocusMode(nsInputActionConfig::OnEnterArea onEnter, nsInputActionConfig::OnLeaveArea onLeave)
{
  m_bConfigChanged = true;

  m_ActionConfig.m_OnEnterArea = onEnter;
  m_ActionConfig.m_OnLeaveArea = onLeave;
}

void nsVirtualThumbStick::SetInputArea(
  const nsVec2& vLowerLeft, const nsVec2& vUpperRight, float fThumbstickRadius, float fPriority, CenterMode::Enum center)
{
  m_bConfigChanged = true;

  m_vLowerLeft = vLowerLeft;
  m_vUpperRight = vUpperRight;
  m_fRadius = fThumbstickRadius;
  m_ActionConfig.m_fFilteredPriority = fPriority;
  m_CenterMode = center;
}

void nsVirtualThumbStick::GetInputArea(nsVec2& out_vLowerLeft, nsVec2& out_vUpperRight)
{
  out_vLowerLeft = m_vLowerLeft;
  out_vUpperRight = m_vUpperRight;
}

void nsVirtualThumbStick::UpdateActionMapping()
{
  if (!m_bConfigChanged)
    return;

  m_ActionConfig.m_fFilterXMinValue = m_vLowerLeft.x;
  m_ActionConfig.m_fFilterXMaxValue = m_vUpperRight.x;
  m_ActionConfig.m_fFilterYMinValue = m_vLowerLeft.y;
  m_ActionConfig.m_fFilterYMaxValue = m_vUpperRight.y;

  nsInputManager::SetInputActionConfig(GetDynamicRTTI()->GetTypeName(), m_sName.GetData(), m_ActionConfig, false);

  m_bConfigChanged = false;
}

void nsVirtualThumbStick::UpdateInputSlotValues()
{
  m_bIsActive = false;

  m_InputSlotValues[m_sOutputLeft] = 0.0f;
  m_InputSlotValues[m_sOutputRight] = 0.0f;
  m_InputSlotValues[m_sOutputUp] = 0.0f;
  m_InputSlotValues[m_sOutputDown] = 0.0f;

  if (!m_bEnabled)
  {
    nsInputManager::RemoveInputAction(GetDynamicRTTI()->GetTypeName(), m_sName.GetData());
    return;
  }

  UpdateActionMapping();

  float fValue;
  nsInt8 iTriggerAlt;

  const nsKeyState::Enum ks = nsInputManager::GetInputActionState(GetDynamicRTTI()->GetTypeName(), m_sName.GetData(), &fValue, &iTriggerAlt);

  if (ks != nsKeyState::Up)
  {
    m_bIsActive = true;

    nsVec2 vTouchPos(0.0f);

    nsInputManager::GetInputSlotState(m_ActionConfig.m_sFilterByInputSlotX[(nsUInt32)iTriggerAlt].GetData(), &vTouchPos.x);
    nsInputManager::GetInputSlotState(m_ActionConfig.m_sFilterByInputSlotY[(nsUInt32)iTriggerAlt].GetData(), &vTouchPos.y);

    if (ks == nsKeyState::Pressed)
    {
      switch (m_CenterMode)
      {
        case CenterMode::InputArea:
          m_vCenter = m_vLowerLeft + (m_vUpperRight - m_vLowerLeft) * 0.5f;
          break;
        case CenterMode::ActivationPoint:
          m_vCenter = vTouchPos;
          break;
      }
    }

    nsVec2 vDir = vTouchPos - m_vCenter;
    vDir.y *= -1;

    const float fLength = nsMath::Min(vDir.GetLength(), m_fRadius) / m_fRadius;
    vDir.NormalizeIfNotZero(nsVec2::MakeZero()).IgnoreResult();

    m_InputSlotValues[m_sOutputLeft] = nsMath::Max(0.0f, -vDir.x) * fLength;
    m_InputSlotValues[m_sOutputRight] = nsMath::Max(0.0f, vDir.x) * fLength;
    m_InputSlotValues[m_sOutputUp] = nsMath::Max(0.0f, vDir.y) * fLength;
    m_InputSlotValues[m_sOutputDown] = nsMath::Max(0.0f, -vDir.y) * fLength;
  }
}

void nsVirtualThumbStick::RegisterInputSlots()
{
  RegisterInputSlot(nsInputSlot_Controller0_LeftStick_NegX, "Left Stick Left", nsInputSlotFlags::IsAnalogStick);
  RegisterInputSlot(nsInputSlot_Controller0_LeftStick_PosX, "Left Stick Right", nsInputSlotFlags::IsAnalogStick);
  RegisterInputSlot(nsInputSlot_Controller0_LeftStick_NegY, "Left Stick Down", nsInputSlotFlags::IsAnalogStick);
  RegisterInputSlot(nsInputSlot_Controller0_LeftStick_PosY, "Left Stick Up", nsInputSlotFlags::IsAnalogStick);

  RegisterInputSlot(nsInputSlot_Controller0_RightStick_NegX, "Right Stick Left", nsInputSlotFlags::IsAnalogStick);
  RegisterInputSlot(nsInputSlot_Controller0_RightStick_PosX, "Right Stick Right", nsInputSlotFlags::IsAnalogStick);
  RegisterInputSlot(nsInputSlot_Controller0_RightStick_NegY, "Right Stick Down", nsInputSlotFlags::IsAnalogStick);
  RegisterInputSlot(nsInputSlot_Controller0_RightStick_PosY, "Right Stick Up", nsInputSlotFlags::IsAnalogStick);
}


NS_STATICLINK_FILE(Core, Core_Input_Implementation_VirtualThumbStick);

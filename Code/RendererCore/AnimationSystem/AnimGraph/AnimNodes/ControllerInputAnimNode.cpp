#include <RendererCore/RendererCorePCH.h>

#include <Core/Input/InputManager.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimNodes/ControllerInputAnimNode.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsControllerInputAnimNode, 1, nsRTTIDefaultAllocator<nsControllerInputAnimNode>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("LeftStickX", m_OutLeftStickX)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("LeftStickY", m_OutLeftStickY)->AddAttributes(new nsHiddenAttribute()),

    NS_MEMBER_PROPERTY("RightStickX", m_OutRightStickX)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("RightStickY", m_OutRightStickY)->AddAttributes(new nsHiddenAttribute()),

    NS_MEMBER_PROPERTY("LeftTrigger", m_OutLeftTrigger)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("RightTrigger", m_OutRightTrigger)->AddAttributes(new nsHiddenAttribute()),

    NS_MEMBER_PROPERTY("ButtonA", m_OutButtonA)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("ButtonB", m_OutButtonB)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("ButtonX", m_OutButtonX)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("ButtonY", m_OutButtonY)->AddAttributes(new nsHiddenAttribute()),

    NS_MEMBER_PROPERTY("LeftShoulder", m_OutLeftShoulder)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("RightShoulder", m_OutRightShoulder)->AddAttributes(new nsHiddenAttribute()),

    NS_MEMBER_PROPERTY("PadLeft", m_OutPadLeft)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("PadRight", m_OutPadRight)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("PadUp", m_OutPadUp)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("PadDown", m_OutPadDown)->AddAttributes(new nsHiddenAttribute()),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Input"),
    new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Pink)),
    new nsTitleAttribute("Controller"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsResult nsControllerInputAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  NS_SUCCEED_OR_RETURN(m_OutLeftStickX.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutLeftStickY.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutRightStickX.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutRightStickY.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutLeftTrigger.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutRightTrigger.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutButtonA.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutButtonB.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutButtonX.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutButtonY.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutLeftShoulder.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutRightShoulder.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutPadLeft.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutPadRight.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutPadUp.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutPadDown.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsControllerInputAnimNode::DeserializeNode(nsStreamReader& stream)
{
  stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  NS_SUCCEED_OR_RETURN(m_OutLeftStickX.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutLeftStickY.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutRightStickX.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutRightStickY.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutLeftTrigger.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutRightTrigger.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutButtonA.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutButtonB.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutButtonX.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutButtonY.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutLeftShoulder.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutRightShoulder.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutPadLeft.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutPadRight.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutPadUp.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutPadDown.Deserialize(stream));

  return NS_SUCCESS;
}

void nsControllerInputAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  {
    float fValue1 = 0.0f;
    float fValue2 = 0.0f;

    nsInputManager::GetInputSlotState(nsInputSlot_Controller0_LeftStick_NegX, &fValue1);
    nsInputManager::GetInputSlotState(nsInputSlot_Controller0_LeftStick_PosX, &fValue2);
    m_OutLeftStickX.SetNumber(ref_graph, -fValue1 + fValue2);

    nsInputManager::GetInputSlotState(nsInputSlot_Controller0_LeftStick_NegY, &fValue1);
    nsInputManager::GetInputSlotState(nsInputSlot_Controller0_LeftStick_PosY, &fValue2);
    m_OutLeftStickY.SetNumber(ref_graph, -fValue1 + fValue2);

    nsInputManager::GetInputSlotState(nsInputSlot_Controller0_RightStick_NegX, &fValue1);
    nsInputManager::GetInputSlotState(nsInputSlot_Controller0_RightStick_PosX, &fValue2);
    m_OutRightStickX.SetNumber(ref_graph, -fValue1 + fValue2);

    nsInputManager::GetInputSlotState(nsInputSlot_Controller0_RightStick_NegY, &fValue1);
    nsInputManager::GetInputSlotState(nsInputSlot_Controller0_RightStick_PosY, &fValue2);
    m_OutRightStickY.SetNumber(ref_graph, -fValue1 + fValue2);
  }

  {
    float fValue = 0.0f;
    nsInputManager::GetInputSlotState(nsInputSlot_Controller0_ButtonA, &fValue);
    m_OutButtonA.SetBool(ref_graph, fValue > 0);

    nsInputManager::GetInputSlotState(nsInputSlot_Controller0_ButtonB, &fValue);
    m_OutButtonB.SetBool(ref_graph, fValue > 0);

    nsInputManager::GetInputSlotState(nsInputSlot_Controller0_ButtonX, &fValue);
    m_OutButtonX.SetBool(ref_graph, fValue > 0);

    nsInputManager::GetInputSlotState(nsInputSlot_Controller0_ButtonY, &fValue);
    m_OutButtonY.SetBool(ref_graph, fValue > 0);

    nsInputManager::GetInputSlotState(nsInputSlot_Controller0_LeftShoulder, &fValue);
    m_OutLeftShoulder.SetBool(ref_graph, fValue > 0);

    nsInputManager::GetInputSlotState(nsInputSlot_Controller0_LeftTrigger, &fValue);
    m_OutLeftTrigger.SetNumber(ref_graph, fValue);

    nsInputManager::GetInputSlotState(nsInputSlot_Controller0_RightShoulder, &fValue);
    m_OutRightShoulder.SetBool(ref_graph, fValue > 0);

    nsInputManager::GetInputSlotState(nsInputSlot_Controller0_RightTrigger, &fValue);
    m_OutRightTrigger.SetNumber(ref_graph, fValue);

    nsInputManager::GetInputSlotState(nsInputSlot_Controller0_PadLeft, &fValue);
    m_OutPadLeft.SetBool(ref_graph, fValue > 0);

    nsInputManager::GetInputSlotState(nsInputSlot_Controller0_PadRight, &fValue);
    m_OutPadRight.SetBool(ref_graph, fValue > 0);

    nsInputManager::GetInputSlotState(nsInputSlot_Controller0_PadUp, &fValue);
    m_OutPadUp.SetBool(ref_graph, fValue > 0);

    nsInputManager::GetInputSlotState(nsInputSlot_Controller0_PadDown, &fValue);
    m_OutPadDown.SetBool(ref_graph, fValue > 0);
  }
}

NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_AnimGraph_AnimNodes_ControllerInputAnimNode);

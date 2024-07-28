#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/AnimationSystem/AnimGraph/AnimController.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphInstance.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphPins.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAnimGraphPin, 1, nsRTTINoAllocator)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("PinIdx", m_iPinIndex)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("NumConnections", m_uiNumConnections)->AddAttributes(new nsHiddenAttribute()),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAnimGraphInputPin, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAnimGraphOutputPin, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsResult nsAnimGraphPin::Serialize(nsStreamWriter& inout_stream) const
{
  inout_stream << m_iPinIndex;
  inout_stream << m_uiNumConnections;
  return NS_SUCCESS;
}

nsResult nsAnimGraphPin::Deserialize(nsStreamReader& inout_stream)
{
  inout_stream >> m_iPinIndex;
  inout_stream >> m_uiNumConnections;
  return NS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAnimGraphTriggerInputPin, 1, nsRTTIDefaultAllocator<nsAnimGraphTriggerInputPin>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAnimGraphTriggerOutputPin, 1, nsRTTIDefaultAllocator<nsAnimGraphTriggerOutputPin>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void nsAnimGraphTriggerOutputPin::SetTriggered(nsAnimGraphInstance& ref_graph) const
{
  if (m_iPinIndex < 0)
    return;

  const auto& map = ref_graph.m_pAnimGraph->m_OutputPinToInputPinMapping[nsAnimGraphPin::Trigger][m_iPinIndex];


  const nsInt8 offset = +1; // bTriggered ? +1 : -1;

  // trigger or reset all input pins that are connected to this output pin
  for (nsUInt16 idx : map)
  {
    ref_graph.m_pTriggerInputPinStates[idx] += offset;
  }
}

bool nsAnimGraphTriggerInputPin::IsTriggered(nsAnimGraphInstance& ref_graph) const
{
  if (m_iPinIndex < 0)
    return false;

  return ref_graph.m_pTriggerInputPinStates[m_iPinIndex] > 0;
}

bool nsAnimGraphTriggerInputPin::AreAllTriggered(nsAnimGraphInstance& ref_graph) const
{
  return ref_graph.m_pTriggerInputPinStates[m_iPinIndex] == m_uiNumConnections;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAnimGraphNumberInputPin, 1, nsRTTIDefaultAllocator<nsAnimGraphNumberInputPin>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAnimGraphNumberOutputPin, 1, nsRTTIDefaultAllocator<nsAnimGraphNumberOutputPin>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

double nsAnimGraphNumberInputPin::GetNumber(nsAnimGraphInstance& ref_graph, double fFallback /*= 0.0*/) const
{
  if (m_iPinIndex < 0)
    return fFallback;

  return ref_graph.m_pNumberInputPinStates[m_iPinIndex];
}

void nsAnimGraphNumberOutputPin::SetNumber(nsAnimGraphInstance& ref_graph, double value) const
{
  if (m_iPinIndex < 0)
    return;

  const auto& map = ref_graph.m_pAnimGraph->m_OutputPinToInputPinMapping[nsAnimGraphPin::Number][m_iPinIndex];

  // set all input pins that are connected to this output pin
  for (nsUInt16 idx : map)
  {
    ref_graph.m_pNumberInputPinStates[idx] = value;
  }
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAnimGraphBoolInputPin, 1, nsRTTIDefaultAllocator<nsAnimGraphBoolInputPin>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAnimGraphBoolOutputPin, 1, nsRTTIDefaultAllocator<nsAnimGraphBoolOutputPin>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

bool nsAnimGraphBoolInputPin::GetBool(nsAnimGraphInstance& ref_graph, bool bFallback /*= false */) const
{
  if (m_iPinIndex < 0)
    return bFallback;

  return ref_graph.m_pBoolInputPinStates[m_iPinIndex];
}

void nsAnimGraphBoolOutputPin::SetBool(nsAnimGraphInstance& ref_graph, bool bValue) const
{
  if (m_iPinIndex < 0)
    return;

  const auto& map = ref_graph.m_pAnimGraph->m_OutputPinToInputPinMapping[nsAnimGraphPin::Bool][m_iPinIndex];

  // set all input pins that are connected to this output pin
  for (nsUInt16 idx : map)
  {
    ref_graph.m_pBoolInputPinStates[idx] = bValue;
  }
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAnimGraphBoneWeightsInputPin, 1, nsRTTIDefaultAllocator<nsAnimGraphBoneWeightsInputPin>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAnimGraphBoneWeightsOutputPin, 1, nsRTTIDefaultAllocator<nsAnimGraphBoneWeightsOutputPin>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsAnimGraphPinDataBoneWeights* nsAnimGraphBoneWeightsInputPin::GetWeights(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph) const
{
  if (m_iPinIndex < 0 || ref_graph.m_pBoneWeightInputPinStates[m_iPinIndex] == 0xFFFF)
    return nullptr;

  return &ref_controller.m_PinDataBoneWeights[ref_graph.m_pBoneWeightInputPinStates[m_iPinIndex]];
}

void nsAnimGraphBoneWeightsOutputPin::SetWeights(nsAnimGraphInstance& ref_graph, nsAnimGraphPinDataBoneWeights* pWeights) const
{
  if (m_iPinIndex < 0)
    return;

  const auto& map = ref_graph.m_pAnimGraph->m_OutputPinToInputPinMapping[nsAnimGraphPin::BoneWeights][m_iPinIndex];

  // set all input pins that are connected to this output pin
  for (nsUInt16 idx : map)
  {
    ref_graph.m_pBoneWeightInputPinStates[idx] = pWeights->m_uiOwnIndex;
  }
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAnimGraphLocalPoseInputPin, 1, nsRTTIDefaultAllocator<nsAnimGraphLocalPoseInputPin>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAnimGraphLocalPoseMultiInputPin, 1, nsRTTIDefaultAllocator<nsAnimGraphLocalPoseMultiInputPin>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAnimGraphLocalPoseOutputPin, 1, nsRTTIDefaultAllocator<nsAnimGraphLocalPoseOutputPin>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsAnimGraphPinDataLocalTransforms* nsAnimGraphLocalPoseInputPin::GetPose(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph) const
{
  if (m_iPinIndex < 0)
    return nullptr;

  if (ref_graph.m_LocalPoseInputPinStates[m_iPinIndex].IsEmpty())
    return nullptr;

  return &ref_controller.m_PinDataLocalTransforms[ref_graph.m_LocalPoseInputPinStates[m_iPinIndex][0]];
}

void nsAnimGraphLocalPoseMultiInputPin::GetPoses(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsDynamicArray<nsAnimGraphPinDataLocalTransforms*>& out_poses) const
{
  out_poses.Clear();

  if (m_iPinIndex < 0)
    return;

  out_poses.SetCountUninitialized(ref_graph.m_LocalPoseInputPinStates[m_iPinIndex].GetCount());
  for (nsUInt32 i = 0; i < ref_graph.m_LocalPoseInputPinStates[m_iPinIndex].GetCount(); ++i)
  {
    out_poses[i] = &ref_controller.m_PinDataLocalTransforms[ref_graph.m_LocalPoseInputPinStates[m_iPinIndex][i]];
  }
}

void nsAnimGraphLocalPoseOutputPin::SetPose(nsAnimGraphInstance& ref_graph, nsAnimGraphPinDataLocalTransforms* pPose) const
{
  if (m_iPinIndex < 0)
    return;

  const auto& map = ref_graph.m_pAnimGraph->m_OutputPinToInputPinMapping[nsAnimGraphPin::LocalPose][m_iPinIndex];

  // set all input pins that are connected to this output pin
  for (nsUInt16 idx : map)
  {
    ref_graph.m_LocalPoseInputPinStates[idx].PushBack(pPose->m_uiOwnIndex);
  }
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAnimGraphModelPoseInputPin, 1, nsRTTIDefaultAllocator<nsAnimGraphModelPoseInputPin>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAnimGraphModelPoseOutputPin, 1, nsRTTIDefaultAllocator<nsAnimGraphModelPoseOutputPin>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsAnimGraphPinDataModelTransforms* nsAnimGraphModelPoseInputPin::GetPose(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph) const
{
  if (m_iPinIndex < 0 || ref_graph.m_pModelPoseInputPinStates[m_iPinIndex] == 0xFFFF)
    return nullptr;

  return &ref_controller.m_PinDataModelTransforms[ref_graph.m_pModelPoseInputPinStates[m_iPinIndex]];
}

void nsAnimGraphModelPoseOutputPin::SetPose(nsAnimGraphInstance& ref_graph, nsAnimGraphPinDataModelTransforms* pPose) const
{
  if (m_iPinIndex < 0)
    return;

  const auto& map = ref_graph.m_pAnimGraph->m_OutputPinToInputPinMapping[nsAnimGraphPin::ModelPose][m_iPinIndex];

  // set all input pins that are connected to this output pin
  for (nsUInt16 idx : map)
  {
    ref_graph.m_pModelPoseInputPinStates[idx] = pPose->m_uiOwnIndex;
  }
}


NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_AnimGraph_Implementation_AnimGraphPins);

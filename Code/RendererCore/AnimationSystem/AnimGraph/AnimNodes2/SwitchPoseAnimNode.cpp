#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/AnimationSystem/AnimGraph/AnimController.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphInstance.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimNodes2/SwitchPoseAnimNode.h>
#include <RendererCore/AnimationSystem/AnimationClipResource.h>
#include <RendererCore/AnimationSystem/SkeletonResource.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSwitchPoseAnimNode, 1, nsRTTIDefaultAllocator<nsSwitchPoseAnimNode>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("TransitionDuration", m_TransitionDuration)->AddAttributes(new nsDefaultValueAttribute(nsTime::MakeFromMilliseconds(200))),
    NS_MEMBER_PROPERTY("InIndex", m_InIndex)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("PosesCount", m_uiPosesCount)->AddAttributes(new nsNoTemporaryTransactionsAttribute(), new nsDynamicPinAttribute(), new nsDefaultValueAttribute(2)),
    NS_ARRAY_MEMBER_PROPERTY("InPoses", m_InPoses)->AddAttributes(new nsHiddenAttribute(), new nsDynamicPinAttribute("PosesCount")),
    NS_MEMBER_PROPERTY("OutPose", m_OutPose)->AddAttributes(new nsHiddenAttribute()),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Pose Blending"),
    new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Yellow)),
    new nsTitleAttribute("Pose Switch"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsResult nsSwitchPoseAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  stream << m_TransitionDuration;
  stream << m_uiPosesCount;

  NS_SUCCEED_OR_RETURN(m_InIndex.Serialize(stream));
  NS_SUCCEED_OR_RETURN(stream.WriteArray(m_InPoses));
  NS_SUCCEED_OR_RETURN(m_OutPose.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsSwitchPoseAnimNode::DeserializeNode(nsStreamReader& stream)
{
  const auto version = stream.ReadVersion(1);
  NS_IGNORE_UNUSED(version);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  stream >> m_TransitionDuration;
  stream >> m_uiPosesCount;

  NS_SUCCEED_OR_RETURN(m_InIndex.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(stream.ReadArray(m_InPoses));
  NS_SUCCEED_OR_RETURN(m_OutPose.Deserialize(stream));

  return NS_SUCCESS;
}

void nsSwitchPoseAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  if (!m_OutPose.IsConnected() || !m_InIndex.IsConnected())
    return;

  nsHybridArray<const nsAnimGraphLocalPoseInputPin*, 12> pPins;
  for (nsUInt32 i = 0; i < m_InPoses.GetCount(); ++i)
  {
    pPins.PushBack(&m_InPoses[i]);
  }

  // duplicate pin connections to fill up holes
  for (nsUInt32 i = 1; i < pPins.GetCount(); ++i)
  {
    if (!pPins[i]->IsConnected())
      pPins[i] = pPins[i - 1];
  }
  for (nsUInt32 i = pPins.GetCount(); i > 1; --i)
  {
    if (!pPins[i - 2]->IsConnected())
      pPins[i - 2] = pPins[i - 1];
  }

  if (pPins.IsEmpty() || !pPins[0]->IsConnected())
  {
    // this can only be the case if no pin is connected, at all
    return;
  }

  InstanceData* pInstance = ref_graph.GetAnimNodeInstanceData<InstanceData>(*this);

  const nsInt8 iDstIdx = nsMath::Clamp<nsInt8>((nsInt8)m_InIndex.GetNumber(ref_graph, 0), 0, pPins.GetCount() - 1);

  if (pInstance->m_iTransitionToIndex < 0)
  {
    pInstance->m_iTransitionToIndex = iDstIdx;
    pInstance->m_iTransitionFromIndex = iDstIdx;
  }

  pInstance->m_TransitionTime += tDiff;

  if (iDstIdx != pInstance->m_iTransitionToIndex)
  {
    pInstance->m_iTransitionFromIndex = pInstance->m_iTransitionToIndex;
    pInstance->m_iTransitionToIndex = iDstIdx;
    pInstance->m_TransitionTime = nsTime::MakeZero();
  }

  if (pInstance->m_TransitionTime >= m_TransitionDuration)
  {
    pInstance->m_iTransitionFromIndex = pInstance->m_iTransitionToIndex;
  }

  NS_ASSERT_DEBUG(pInstance->m_iTransitionToIndex >= 0 && pInstance->m_iTransitionToIndex < pPins.GetCount(), "Invalid pose index");
  NS_ASSERT_DEBUG(pInstance->m_iTransitionToIndex >= 0 && pInstance->m_iTransitionToIndex < pPins.GetCount(), "Invalid pose index");

  nsInt8 iTransitionFromIndex = pInstance->m_iTransitionFromIndex;
  nsInt8 iTransitionToIndex = pInstance->m_iTransitionToIndex;

  if (pPins[iTransitionFromIndex]->GetPose(ref_controller, ref_graph) == nullptr)
  {
    // if the 'from' pose already stopped, just jump to the 'to' pose
    iTransitionFromIndex = iTransitionToIndex;
  }

  if (iTransitionFromIndex == iTransitionToIndex)
  {
    const nsAnimGraphLocalPoseInputPin* pPinToForward = pPins[iTransitionToIndex];
    nsAnimGraphPinDataLocalTransforms* pDataToForward = pPinToForward->GetPose(ref_controller, ref_graph);

    if (pDataToForward == nullptr)
      return;

    nsAnimGraphPinDataLocalTransforms* pLocalTransforms = ref_controller.AddPinDataLocalTransforms();
    pLocalTransforms->m_CommandID = pDataToForward->m_CommandID;
    pLocalTransforms->m_pWeights = pDataToForward->m_pWeights;
    pLocalTransforms->m_fOverallWeight = pDataToForward->m_fOverallWeight;
    pLocalTransforms->m_vRootMotion = pDataToForward->m_vRootMotion;
    pLocalTransforms->m_bUseRootMotion = pDataToForward->m_bUseRootMotion;

    m_OutPose.SetPose(ref_graph, pLocalTransforms);
  }
  else
  {
    auto pPose0 = pPins[iTransitionFromIndex]->GetPose(ref_controller, ref_graph);
    auto pPose1 = pPins[iTransitionToIndex]->GetPose(ref_controller, ref_graph);

    if (pPose0 == nullptr || pPose1 == nullptr)
      return;

    nsAnimGraphPinDataLocalTransforms* pPinData = ref_controller.AddPinDataLocalTransforms();

    const float fLerp = (float)nsMath::Clamp(pInstance->m_TransitionTime.GetSeconds() / m_TransitionDuration.GetSeconds(), 0.0, 1.0);

    auto& cmd = ref_controller.GetPoseGenerator().AllocCommandCombinePoses();
    cmd.m_InputWeights.SetCount(2);
    cmd.m_InputWeights[0] = 1.0f - fLerp;
    cmd.m_InputWeights[1] = fLerp;
    cmd.m_Inputs.SetCount(2);
    cmd.m_Inputs[0] = pPose0->m_CommandID;
    cmd.m_Inputs[1] = pPose1->m_CommandID;

    pPinData->m_CommandID = cmd.GetCommandID();
    pPinData->m_bUseRootMotion = pPose0->m_bUseRootMotion || pPose1->m_bUseRootMotion;
    pPinData->m_vRootMotion = nsMath::Lerp(pPose0->m_vRootMotion, pPose1->m_vRootMotion, fLerp);

    m_OutPose.SetPose(ref_graph, pPinData);
  }
}

bool nsSwitchPoseAnimNode::GetInstanceDataDesc(nsInstanceDataDesc& out_desc) const
{
  out_desc.FillFromType<InstanceData>();
  return true;
}


NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_AnimGraph_AnimNodes2_SwitchPoseAnimNode);

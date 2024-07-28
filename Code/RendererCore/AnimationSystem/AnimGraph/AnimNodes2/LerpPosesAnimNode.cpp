#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Containers/HybridArray.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimController.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphInstance.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimNodes2/LerpPosesAnimNode.h>
#include <RendererCore/AnimationSystem/SkeletonResource.h>
#include <ozz/animation/runtime/skeleton.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsLerpPosesAnimNode, 1, nsRTTIDefaultAllocator<nsLerpPosesAnimNode>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Lerp", m_fLerp)->AddAttributes(new nsDefaultValueAttribute(0.5f), new nsClampValueAttribute(0.0f, 3.0f)),
    NS_MEMBER_PROPERTY("InLerp", m_InLerp)->AddAttributes(new nsHiddenAttribute),
    NS_MEMBER_PROPERTY("PosesCount", m_uiPosesCount)->AddAttributes(new nsNoTemporaryTransactionsAttribute(), new nsDynamicPinAttribute(), new nsDefaultValueAttribute(2)),
    NS_ARRAY_MEMBER_PROPERTY("InPoses", m_InPoses)->AddAttributes(new nsHiddenAttribute(), new nsDynamicPinAttribute("PosesCount")),
    NS_MEMBER_PROPERTY("OutPose", m_OutPose)->AddAttributes(new nsHiddenAttribute()),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Pose Blending"),
    new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Violet)),
    new nsTitleAttribute("Lerp Poses"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsLerpPosesAnimNode::nsLerpPosesAnimNode() = default;
nsLerpPosesAnimNode::~nsLerpPosesAnimNode() = default;

nsResult nsLerpPosesAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  stream << m_fLerp;
  stream << m_uiPosesCount;

  NS_SUCCEED_OR_RETURN(m_InLerp.Serialize(stream));
  NS_SUCCEED_OR_RETURN(stream.WriteArray(m_InPoses));
  NS_SUCCEED_OR_RETURN(m_OutPose.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsLerpPosesAnimNode::DeserializeNode(nsStreamReader& stream)
{
  stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  stream >> m_fLerp;
  stream >> m_uiPosesCount;

  NS_SUCCEED_OR_RETURN(m_InLerp.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(stream.ReadArray(m_InPoses));
  NS_SUCCEED_OR_RETURN(m_OutPose.Deserialize(stream));

  return NS_SUCCESS;
}

void nsLerpPosesAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  if (!m_OutPose.IsConnected())
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

  const float fIndex = nsMath::Clamp((float)m_InLerp.GetNumber(ref_graph, m_fLerp), 0.0f, (float)pPins.GetCount() - 1.0f);

  if (nsMath::Fraction(fIndex) == 0.0f)
  {
    const nsAnimGraphLocalPoseInputPin* pPinToForward = pPins[(nsInt32)nsMath::Trunc(fIndex)];
    nsAnimGraphPinDataLocalTransforms* pDataToForward = pPinToForward->GetPose(ref_controller, ref_graph);

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
    nsAnimGraphPinDataLocalTransforms* pPinData = ref_controller.AddPinDataLocalTransforms();

    const float fLerp = nsMath::Fraction(fIndex);

    auto pPose0 = pPins[(nsInt32)nsMath::Trunc(fIndex)]->GetPose(ref_controller, ref_graph);
    auto pPose1 = pPins[(nsInt32)nsMath::Trunc(fIndex) + 1]->GetPose(ref_controller, ref_graph);

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


NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_AnimGraph_AnimNodes2_LerpPosesAnimNode);

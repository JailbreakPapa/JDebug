#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/AnimationSystem/AnimGraph/AnimController.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphInstance.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimNodes2/PoseResultAnimNode.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsPoseResultAnimNode, 1, nsRTTIDefaultAllocator<nsPoseResultAnimNode>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("FadeDuration", m_FadeDuration)->AddAttributes(new nsDefaultValueAttribute(nsTime::MakeFromMilliseconds(200)), new nsClampValueAttribute(nsTime::MakeZero(), nsTime::MakeFromSeconds(10))),
    NS_MEMBER_PROPERTY("InPose", m_InPose)->AddAttributes(new nsHiddenAttribute),
    NS_MEMBER_PROPERTY("InTargetWeight", m_InTargetWeight)->AddAttributes(new nsHiddenAttribute),
    NS_MEMBER_PROPERTY("InFadeDuration", m_InFadeDuration)->AddAttributes(new nsHiddenAttribute),
    NS_MEMBER_PROPERTY("InWeights", m_InWeights)->AddAttributes(new nsHiddenAttribute),
    NS_MEMBER_PROPERTY("OutOnFadedOut", m_OutOnFadedOut)->AddAttributes(new nsHiddenAttribute),
    NS_MEMBER_PROPERTY("OutOnFadedIn", m_OutOnFadedIn)->AddAttributes(new nsHiddenAttribute),
    NS_MEMBER_PROPERTY("OutCurrentWeight", m_OutCurrentWeight)->AddAttributes(new nsHiddenAttribute),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Output"),
    new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Grape)),
    new nsTitleAttribute("Pose Result"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsPoseResultAnimNode::nsPoseResultAnimNode() = default;
nsPoseResultAnimNode::~nsPoseResultAnimNode() = default;

nsResult nsPoseResultAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  stream << m_FadeDuration;

  NS_SUCCEED_OR_RETURN(m_InPose.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_InTargetWeight.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_InFadeDuration.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_InWeights.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnFadedOut.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnFadedIn.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutCurrentWeight.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsPoseResultAnimNode::DeserializeNode(nsStreamReader& stream)
{
  stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  stream >> m_FadeDuration;

  NS_SUCCEED_OR_RETURN(m_InPose.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_InTargetWeight.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_InFadeDuration.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_InWeights.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnFadedOut.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnFadedIn.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutCurrentWeight.Deserialize(stream));

  return NS_SUCCESS;
}

void nsPoseResultAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  if (!m_InPose.IsConnected())
    return;

  InstanceData* pInstance = ref_graph.GetAnimNodeInstanceData<InstanceData>(*this);

  const bool bWasInterpolating = pInstance->m_PlayTime < pInstance->m_EndTime;
  const float fNewTargetWeight = m_InTargetWeight.GetNumber(ref_graph, 1.0f);

  if (pInstance->m_fEndWeight != fNewTargetWeight)
  {
    // compute weight from previous frame
    if (bWasInterpolating)
    {
      const float f = (float)(pInstance->m_PlayTime.GetSeconds() / pInstance->m_EndTime.GetSeconds());
      pInstance->m_fStartWeight = nsMath::Lerp(pInstance->m_fStartWeight, pInstance->m_fEndWeight, f);
    }
    else
    {
      pInstance->m_fStartWeight = pInstance->m_fEndWeight;
    }

    pInstance->m_fEndWeight = fNewTargetWeight;
    pInstance->m_PlayTime = nsTime::MakeZero();
    pInstance->m_EndTime = nsTime::MakeFromSeconds(m_InFadeDuration.GetNumber(ref_graph, m_FadeDuration.GetSeconds()));
  }

  float fCurrentWeight = 0.0f;
  pInstance->m_PlayTime += tDiff;

  if (pInstance->m_PlayTime >= pInstance->m_EndTime)
  {
    fCurrentWeight = pInstance->m_fEndWeight;

    if (bWasInterpolating && fCurrentWeight <= 0.0f)
    {
      m_OutOnFadedOut.SetTriggered(ref_graph);
    }
    if (bWasInterpolating && fCurrentWeight >= 1.0f)
    {
      m_OutOnFadedIn.SetTriggered(ref_graph);
    }
  }
  else
  {
    const float f = (float)(pInstance->m_PlayTime.GetSeconds() / pInstance->m_EndTime.GetSeconds());
    fCurrentWeight = nsMath::Lerp(pInstance->m_fStartWeight, pInstance->m_fEndWeight, f);
  }

  m_OutCurrentWeight.SetNumber(ref_graph, fCurrentWeight);

  if (fCurrentWeight <= 0.0f)
    return;

  if (auto pCurrentLocalTransforms = m_InPose.GetPose(ref_controller, ref_graph))
  {
    if (pCurrentLocalTransforms->m_CommandID != nsInvalidIndex)
    {
      nsAnimGraphPinDataLocalTransforms* pLocalTransforms = ref_controller.AddPinDataLocalTransforms();

      pLocalTransforms->m_CommandID = pCurrentLocalTransforms->m_CommandID;
      pLocalTransforms->m_pWeights = m_InWeights.GetWeights(ref_controller, ref_graph);
      pLocalTransforms->m_fOverallWeight = pCurrentLocalTransforms->m_fOverallWeight * fCurrentWeight;
      pLocalTransforms->m_bUseRootMotion = pCurrentLocalTransforms->m_bUseRootMotion;
      pLocalTransforms->m_vRootMotion = pCurrentLocalTransforms->m_vRootMotion;

      ref_controller.AddOutputLocalTransforms(pLocalTransforms);
    }
  }
  else
  {
    // if we are active, but the incoming pose isn't valid (anymore), use a rest pose as placeholder
    // this assumes that many animations return to the rest pose and if they are played up to the very end before fading out
    // they can be faded out by using the rest pose

    const void* pThis = this;
    auto& cmd = ref_controller.GetPoseGenerator().AllocCommandRestPose();

    {
      nsAnimGraphPinDataLocalTransforms* pLocalTransforms = ref_controller.AddPinDataLocalTransforms();

      pLocalTransforms->m_CommandID = cmd.GetCommandID();
      pLocalTransforms->m_pWeights = m_InWeights.GetWeights(ref_controller, ref_graph);
      pLocalTransforms->m_fOverallWeight = fCurrentWeight;
      pLocalTransforms->m_bUseRootMotion = false;

      ref_controller.AddOutputLocalTransforms(pLocalTransforms);
    }
  }
}

bool nsPoseResultAnimNode::GetInstanceDataDesc(nsInstanceDataDesc& out_desc) const
{
  out_desc.FillFromType<InstanceData>();
  return true;
}


NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_AnimGraph_AnimNodes2_PoseResultAnimNode);

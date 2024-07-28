#include <RendererCore/RendererCorePCH.h>

#include <Core/World/GameObject.h>
#include <Core/World/World.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimController.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphInstance.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimNodes2/SampleFrameAnimNode.h>
#include <RendererCore/AnimationSystem/AnimPoseGenerator.h>
#include <RendererCore/AnimationSystem/AnimationClipResource.h>
#include <RendererCore/AnimationSystem/SkeletonResource.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSampleFrameAnimNode, 1, nsRTTIDefaultAllocator<nsSampleFrameAnimNode>)
  {
    NS_BEGIN_PROPERTIES
    {
      NS_ACCESSOR_PROPERTY("Clip", GetClip, SetClip)->AddAttributes(new nsDynamicStringEnumAttribute("AnimationClipMappingEnum")),
      NS_MEMBER_PROPERTY("NormPos", m_fNormalizedSamplePosition)->AddAttributes(new nsDefaultValueAttribute(0.0f), new nsClampValueAttribute(0.0f, 1.0f)),

      NS_MEMBER_PROPERTY("InNormPos", m_InNormalizedSamplePosition)->AddAttributes(new nsHiddenAttribute()),
      NS_MEMBER_PROPERTY("InAbsPos", m_InAbsoluteSamplePosition)->AddAttributes(new nsHiddenAttribute()),

      NS_MEMBER_PROPERTY("OutPose", m_OutPose)->AddAttributes(new nsHiddenAttribute()),
    }
    NS_END_PROPERTIES;
    NS_BEGIN_ATTRIBUTES
    {
      new nsCategoryAttribute("Pose Generation"),
      new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Blue)),
      new nsTitleAttribute("Sample Frame: '{Clip}'"),
    }
    NS_END_ATTRIBUTES;
  }
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsResult nsSampleFrameAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  stream << m_sClip;
  stream << m_fNormalizedSamplePosition;

  NS_SUCCEED_OR_RETURN(m_InNormalizedSamplePosition.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_InAbsoluteSamplePosition.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutPose.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsSampleFrameAnimNode::DeserializeNode(nsStreamReader& stream)
{
  const auto version = stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  stream >> m_sClip;
  stream >> m_fNormalizedSamplePosition;

  NS_SUCCEED_OR_RETURN(m_InNormalizedSamplePosition.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_InAbsoluteSamplePosition.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutPose.Deserialize(stream));

  return NS_SUCCESS;
}

void nsSampleFrameAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  if (!m_OutPose.IsConnected())
    return;

  const auto& clip = ref_controller.GetAnimationClipInfo(m_sClip);

  if (clip.m_hClip.IsValid())
  {
    nsResourceLock<nsAnimationClipResource> pAnimClip(clip.m_hClip, nsResourceAcquireMode::BlockTillLoaded_NeverFail);
    if (pAnimClip.GetAcquireResult() != nsResourceAcquireResult::Final)
      return;

    float fNormPos = fNormPos = m_InNormalizedSamplePosition.GetNumber(ref_graph, m_fNormalizedSamplePosition);

    if (m_InAbsoluteSamplePosition.IsConnected())
    {
      const nsTime tDuration = pAnimClip->GetDescriptor().GetDuration();
      const float fInvDuration = 1.0f / tDuration.AsFloatInSeconds();
      fNormPos = m_InAbsoluteSamplePosition.GetNumber(ref_graph) * fInvDuration;
    }

    fNormPos = nsMath::Clamp(fNormPos, 0.0f, 1.0f);

    const void* pThis = this;
    auto& cmd = ref_controller.GetPoseGenerator().AllocCommandSampleTrack(nsHashingUtils::xxHash32(&pThis, sizeof(pThis)));

    cmd.m_hAnimationClip = clip.m_hClip;
    cmd.m_fPreviousNormalizedSamplePos = fNormPos;
    cmd.m_fNormalizedSamplePos = fNormPos;
    cmd.m_EventSampling = nsAnimPoseEventTrackSampleMode::None;

    {
      nsAnimGraphPinDataLocalTransforms* pLocalTransforms = ref_controller.AddPinDataLocalTransforms();

      pLocalTransforms->m_pWeights = nullptr;
      pLocalTransforms->m_bUseRootMotion = false;
      pLocalTransforms->m_fOverallWeight = 1.0f;
      pLocalTransforms->m_CommandID = cmd.GetCommandID();

      m_OutPose.SetPose(ref_graph, pLocalTransforms);
    }
  }
  else
  {
    const void* pThis = this;
    auto& cmd = ref_controller.GetPoseGenerator().AllocCommandRestPose();

    {
      nsAnimGraphPinDataLocalTransforms* pLocalTransforms = ref_controller.AddPinDataLocalTransforms();

      pLocalTransforms->m_pWeights = nullptr;
      pLocalTransforms->m_bUseRootMotion = false;
      pLocalTransforms->m_fOverallWeight = 1.0f;
      pLocalTransforms->m_CommandID = cmd.GetCommandID();

      m_OutPose.SetPose(ref_graph, pLocalTransforms);
    }
  }
}

void nsSampleFrameAnimNode::SetClip(const char* szClip)
{
  m_sClip.Assign(szClip);
}

const char* nsSampleFrameAnimNode::GetClip() const
{
  return m_sClip.GetData();
}


NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_AnimGraph_AnimNodes2_SampleFrameAnimNode);

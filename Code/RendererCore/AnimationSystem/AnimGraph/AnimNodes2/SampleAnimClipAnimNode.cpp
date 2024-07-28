#include <RendererCore/RendererCorePCH.h>

#include <Core/World/GameObject.h>
#include <Core/World/World.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimController.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphInstance.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimNodes2/SampleAnimClipAnimNode.h>
#include <RendererCore/AnimationSystem/SkeletonResource.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSampleAnimClipAnimNode, 1, nsRTTIDefaultAllocator<nsSampleAnimClipAnimNode>)
  {
    NS_BEGIN_PROPERTIES
    {
      NS_MEMBER_PROPERTY("Loop", m_bLoop)->AddAttributes(new nsDefaultValueAttribute(true)),
      NS_MEMBER_PROPERTY("PlaybackSpeed", m_fPlaybackSpeed)->AddAttributes(new nsDefaultValueAttribute(1.0f), new nsClampValueAttribute(0.0f, {})),
      NS_MEMBER_PROPERTY("ApplyRootMotion", m_bApplyRootMotion),
      NS_ACCESSOR_PROPERTY("Clip", GetClip, SetClip)->AddAttributes(new nsDynamicStringEnumAttribute("AnimationClipMappingEnum")),

      NS_MEMBER_PROPERTY("InStart", m_InStart)->AddAttributes(new nsHiddenAttribute()),
      NS_MEMBER_PROPERTY("InLoop", m_InLoop)->AddAttributes(new nsHiddenAttribute()),
      NS_MEMBER_PROPERTY("InSpeed", m_InSpeed)->AddAttributes(new nsHiddenAttribute()),

      NS_MEMBER_PROPERTY("OutPose", m_OutPose)->AddAttributes(new nsHiddenAttribute()),
      NS_MEMBER_PROPERTY("OutOnStarted", m_OutOnStarted)->AddAttributes(new nsHiddenAttribute()),
      NS_MEMBER_PROPERTY("OutOnFinished", m_OutOnFinished)->AddAttributes(new nsHiddenAttribute()),
    }
    NS_END_PROPERTIES;
    NS_BEGIN_ATTRIBUTES
    {
      new nsCategoryAttribute("Pose Generation"),
      new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Blue)),
      new nsTitleAttribute("Sample Clip: '{Clip}'"),
    }
    NS_END_ATTRIBUTES;
  }
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsSampleAnimClipAnimNode::nsSampleAnimClipAnimNode() = default;
nsSampleAnimClipAnimNode::~nsSampleAnimClipAnimNode() = default;

nsResult nsSampleAnimClipAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  stream << m_sClip;
  stream << m_bLoop;
  stream << m_bApplyRootMotion;
  stream << m_fPlaybackSpeed;

  NS_SUCCEED_OR_RETURN(m_InStart.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_InLoop.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_InSpeed.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutPose.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnStarted.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnFinished.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsSampleAnimClipAnimNode::DeserializeNode(nsStreamReader& stream)
{
  const auto version = stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  stream >> m_sClip;
  stream >> m_bLoop;
  stream >> m_bApplyRootMotion;
  stream >> m_fPlaybackSpeed;

  NS_SUCCEED_OR_RETURN(m_InStart.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_InLoop.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_InSpeed.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutPose.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnStarted.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnFinished.Deserialize(stream));

  return NS_SUCCESS;
}

void nsSampleAnimClipAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  const auto& clipInfo = ref_controller.GetAnimationClipInfo(m_sClip);

  if (!clipInfo.m_hClip.IsValid() || !m_OutPose.IsConnected())
    return;

  nsResourceLock<nsAnimationClipResource> pAnimClip(clipInfo.m_hClip, nsResourceAcquireMode::BlockTillLoaded_NeverFail);
  if (pAnimClip.GetAcquireResult() != nsResourceAcquireResult::Final)
    return;

  InstanceState* pState = ref_graph.GetAnimNodeInstanceData<InstanceState>(*this);

  if (m_InStart.IsTriggered(ref_graph))
  {
    pState->m_PlaybackTime = nsTime::MakeZero();

    m_OutOnStarted.SetTriggered(ref_graph);
  }

  const nsTime tDuration = pAnimClip->GetDescriptor().GetDuration();
  const float fInvDuration = 1.0f / tDuration.AsFloatInSeconds();

  // currently we only support playing clips forwards
  const float fPlaySpeed = nsMath::Max(0.0f, static_cast<float>(m_InSpeed.GetNumber(ref_graph, m_fPlaybackSpeed)));

  const nsTime tPrevSamplePos = pState->m_PlaybackTime;
  pState->m_PlaybackTime += tDiff * fPlaySpeed;

  const bool bLoop = m_InLoop.GetBool(ref_graph, m_bLoop);

  const void* pThis = this;
  auto& cmd = ref_controller.GetPoseGenerator().AllocCommandSampleTrack(nsHashingUtils::xxHash32(&pThis, sizeof(pThis)));
  cmd.m_EventSampling = nsAnimPoseEventTrackSampleMode::OnlyBetween;

  if (pState->m_PlaybackTime >= tDuration)
  {
    if (bLoop)
    {
      pState->m_PlaybackTime -= tDuration;
      cmd.m_EventSampling = nsAnimPoseEventTrackSampleMode::LoopAtEnd;
      m_OutOnStarted.SetTriggered(ref_graph);
    }
    else
    {
      if (tPrevSamplePos < tDuration)
      {
        m_OutOnFinished.SetTriggered(ref_graph);
      }
      else
      {
        // if we are already holding the last frame, we can skip event sampling
        cmd.m_EventSampling = nsAnimPoseEventTrackSampleMode::None;
      }
    }
  }

  cmd.m_hAnimationClip = clipInfo.m_hClip;
  cmd.m_fPreviousNormalizedSamplePos = nsMath::Clamp(tPrevSamplePos.AsFloatInSeconds() * fInvDuration, 0.0f, 1.0f);
  cmd.m_fNormalizedSamplePos = nsMath::Clamp(pState->m_PlaybackTime.AsFloatInSeconds() * fInvDuration, 0.0f, 1.0f);

  {
    nsAnimGraphPinDataLocalTransforms* pLocalTransforms = ref_controller.AddPinDataLocalTransforms();

    pLocalTransforms->m_pWeights = nullptr;
    pLocalTransforms->m_bUseRootMotion = m_bApplyRootMotion;
    pLocalTransforms->m_fOverallWeight = 1.0f;
    pLocalTransforms->m_vRootMotion = pAnimClip->GetDescriptor().m_vConstantRootMotion * tDiff.AsFloatInSeconds() * fPlaySpeed;
    pLocalTransforms->m_CommandID = cmd.GetCommandID();

    m_OutPose.SetPose(ref_graph, pLocalTransforms);
  }
}

void nsSampleAnimClipAnimNode::SetClip(const char* szClip)
{
  m_sClip.Assign(szClip);
}

const char* nsSampleAnimClipAnimNode::GetClip() const
{
  return m_sClip.GetData();
}

bool nsSampleAnimClipAnimNode::GetInstanceDataDesc(nsInstanceDataDesc& out_desc) const
{
  out_desc.FillFromType<InstanceState>();
  return true;
}


NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_AnimGraph_AnimNodes2_SampleAnimClipAnimNode);

#include <RendererCore/RendererCorePCH.h>

#include <Core/World/GameObject.h>
#include <Core/World/World.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimController.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphInstance.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimNodes2/SampleBlendSpace1DAnimNode.h>
#include <RendererCore/AnimationSystem/AnimPoseGenerator.h>
#include <RendererCore/AnimationSystem/SkeletonResource.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsAnimationClip1D, nsNoBase, 1, nsRTTIDefaultAllocator<nsAnimationClip1D>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Clip", GetAnimationFile, SetAnimationFile)->AddAttributes(new nsDynamicStringEnumAttribute("AnimationClipMappingEnum")),
    NS_MEMBER_PROPERTY("Position", m_fPosition),
    NS_MEMBER_PROPERTY("Speed", m_fSpeed)->AddAttributes(new nsDefaultValueAttribute(1.0f)),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSampleBlendSpace1DAnimNode, 1, nsRTTIDefaultAllocator<nsSampleBlendSpace1DAnimNode>)
  {
    NS_BEGIN_PROPERTIES
    {
      NS_MEMBER_PROPERTY("Loop", m_bLoop)->AddAttributes(new nsDefaultValueAttribute(true)),
      NS_MEMBER_PROPERTY("PlaybackSpeed", m_fPlaybackSpeed)->AddAttributes(new nsDefaultValueAttribute(1.0f), new nsClampValueAttribute(0.0f, {})),
      NS_MEMBER_PROPERTY("ApplyRootMotion", m_bApplyRootMotion),
      NS_ARRAY_MEMBER_PROPERTY("Clips", m_Clips),

      NS_MEMBER_PROPERTY("InStart", m_InStart)->AddAttributes(new nsHiddenAttribute()),
      NS_MEMBER_PROPERTY("InLoop", m_InLoop)->AddAttributes(new nsHiddenAttribute()),
      NS_MEMBER_PROPERTY("InSpeed", m_InSpeed)->AddAttributes(new nsHiddenAttribute()),
      NS_MEMBER_PROPERTY("InLerp", m_InLerp)->AddAttributes(new nsHiddenAttribute()),

      NS_MEMBER_PROPERTY("OutPose", m_OutPose)->AddAttributes(new nsHiddenAttribute()),
      NS_MEMBER_PROPERTY("OutOnStarted", m_OutOnStarted)->AddAttributes(new nsHiddenAttribute()),
      NS_MEMBER_PROPERTY("OutOnFinished", m_OutOnFinished)->AddAttributes(new nsHiddenAttribute()),
    }
    NS_END_PROPERTIES;
    NS_BEGIN_ATTRIBUTES
    {
      new nsCategoryAttribute("Pose Generation"),
      new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Blue)),
      new nsTitleAttribute("BlendSpace 1D: '{Clips[0]}' '{Clips[1]}' '{Clips[2]}'"),
    }
    NS_END_ATTRIBUTES;
  }
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void nsAnimationClip1D::SetAnimationFile(const char* szFile)
{
  m_sClip.Assign(szFile);
}

const char* nsAnimationClip1D::GetAnimationFile() const
{
  return m_sClip;
}

nsSampleBlendSpace1DAnimNode::nsSampleBlendSpace1DAnimNode() = default;
nsSampleBlendSpace1DAnimNode::~nsSampleBlendSpace1DAnimNode() = default;

nsResult nsSampleBlendSpace1DAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  stream << m_Clips.GetCount();
  for (nsUInt32 i = 0; i < m_Clips.GetCount(); ++i)
  {
    stream << m_Clips[i].m_sClip;
    stream << m_Clips[i].m_fPosition;
    stream << m_Clips[i].m_fSpeed;
  }

  stream << m_bLoop;
  stream << m_bApplyRootMotion;
  stream << m_fPlaybackSpeed;

  NS_SUCCEED_OR_RETURN(m_InStart.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_InLoop.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_InSpeed.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_InLerp.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutPose.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnStarted.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnFinished.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsSampleBlendSpace1DAnimNode::DeserializeNode(nsStreamReader& stream)
{
  const auto version = stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  nsUInt32 num = 0;
  stream >> num;
  m_Clips.SetCount(num);
  for (nsUInt32 i = 0; i < m_Clips.GetCount(); ++i)
  {
    stream >> m_Clips[i].m_sClip;
    stream >> m_Clips[i].m_fPosition;
    stream >> m_Clips[i].m_fSpeed;
  }

  stream >> m_bLoop;
  stream >> m_bApplyRootMotion;
  stream >> m_fPlaybackSpeed;

  NS_SUCCEED_OR_RETURN(m_InStart.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_InLoop.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_InSpeed.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_InLerp.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutPose.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnStarted.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnFinished.Deserialize(stream));

  return NS_SUCCESS;
}

void nsSampleBlendSpace1DAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  if (!m_OutPose.IsConnected() || !m_InLerp.IsConnected() || m_Clips.IsEmpty())
    return;

  InstanceState* pState = ref_graph.GetAnimNodeInstanceData<InstanceState>(*this);

  if (m_InStart.IsTriggered(ref_graph))
  {
    pState->m_PlaybackTime = nsTime::MakeZero();

    m_OutOnStarted.SetTriggered(ref_graph);
  }

  const bool bLoop = m_InLoop.GetBool(ref_graph, m_bLoop);

  nsUInt32 uiClip1 = 0;
  nsUInt32 uiClip2 = 0;

  const float fLerpPos = (float)m_InLerp.GetNumber(ref_graph);

  if (m_Clips.GetCount() > 1)
  {
    float fDist1 = 1000000.0f;
    float fDist2 = 1000000.0f;

    for (nsUInt32 i = 0; i < m_Clips.GetCount(); ++i)
    {
      const float dist = nsMath::Abs(m_Clips[i].m_fPosition - fLerpPos);

      if (dist < fDist1)
      {
        fDist2 = fDist1;
        uiClip2 = uiClip1;

        fDist1 = dist;
        uiClip1 = i;
      }
      else if (dist < fDist2)
      {
        fDist2 = dist;
        uiClip2 = i;
      }
    }

    if (nsMath::IsZero(fDist1, nsMath::SmallEpsilon<float>()))
    {
      uiClip2 = uiClip1;
    }
  }

  const auto& clip1 = ref_controller.GetAnimationClipInfo(m_Clips[uiClip1].m_sClip);
  const auto& clip2 = ref_controller.GetAnimationClipInfo(m_Clips[uiClip2].m_sClip);

  if (!clip1.m_hClip.IsValid() || !clip2.m_hClip.IsValid())
    return;

  nsResourceLock<nsAnimationClipResource> pAnimClip1(clip1.m_hClip, nsResourceAcquireMode::BlockTillLoaded);
  nsResourceLock<nsAnimationClipResource> pAnimClip2(clip2.m_hClip, nsResourceAcquireMode::BlockTillLoaded);

  if (pAnimClip1.GetAcquireResult() != nsResourceAcquireResult::Final || pAnimClip2.GetAcquireResult() != nsResourceAcquireResult::Final)
    return;

  float fLerpFactor = 0.0f;

  if (uiClip1 != uiClip2)
  {
    const float len = m_Clips[uiClip2].m_fPosition - m_Clips[uiClip1].m_fPosition;
    fLerpFactor = (fLerpPos - m_Clips[uiClip1].m_fPosition) / len;

    // clamp and reduce to single sample when possible
    if (fLerpFactor <= 0.0f)
    {
      fLerpFactor = 0.0f;
      uiClip2 = uiClip1;
    }
    else if (fLerpFactor >= 1.0f)
    {
      fLerpFactor = 1.0f;
      uiClip1 = uiClip2;
    }
  }

  const float fAvgClipSpeed = nsMath::Lerp(m_Clips[uiClip1].m_fSpeed, m_Clips[uiClip2].m_fSpeed, fLerpFactor);
  const float fSpeed = static_cast<float>(m_InSpeed.GetNumber(ref_graph, m_fPlaybackSpeed)) * fAvgClipSpeed;

  const auto& animDesc1 = pAnimClip1->GetDescriptor();
  const auto& animDesc2 = pAnimClip2->GetDescriptor();

  const nsTime avgDuration = nsMath::Lerp(animDesc1.GetDuration(), animDesc2.GetDuration(), fLerpFactor);
  const float fInvDuration = 1.0f / avgDuration.AsFloatInSeconds();

  const nsTime tPrevPlayback = pState->m_PlaybackTime;
  pState->m_PlaybackTime += tDiff * fSpeed;

  nsAnimPoseEventTrackSampleMode eventSampling = nsAnimPoseEventTrackSampleMode::OnlyBetween;

  if (pState->m_PlaybackTime >= avgDuration)
  {
    if (bLoop)
    {
      pState->m_PlaybackTime -= avgDuration;
      eventSampling = nsAnimPoseEventTrackSampleMode::LoopAtEnd;
      m_OutOnStarted.SetTriggered(ref_graph);
    }
    else
    {
      pState->m_PlaybackTime = avgDuration;

      if (tPrevPlayback < avgDuration)
      {
        m_OutOnFinished.SetTriggered(ref_graph);
      }
      else
      {
        // if we are already holding the last frame, we can skip event sampling
        eventSampling = nsAnimPoseEventTrackSampleMode::None;
      }
    }
  }

  nsAnimGraphPinDataLocalTransforms* pOutputTransform = ref_controller.AddPinDataLocalTransforms();

  auto& poseGen = ref_controller.GetPoseGenerator();

  if (clip1.m_hClip == clip2.m_hClip)
  {
    const void* pThis = this;
    auto& cmd = poseGen.AllocCommandSampleTrack(nsHashingUtils::xxHash32(&pThis, sizeof(pThis), 0));
    cmd.m_hAnimationClip = clip1.m_hClip;
    cmd.m_fPreviousNormalizedSamplePos = tPrevPlayback.AsFloatInSeconds() * fInvDuration;
    cmd.m_fNormalizedSamplePos = pState->m_PlaybackTime.AsFloatInSeconds() * fInvDuration;
    cmd.m_EventSampling = eventSampling;

    pOutputTransform->m_CommandID = cmd.GetCommandID();
  }
  else
  {
    auto& cmdCmb = poseGen.AllocCommandCombinePoses();
    pOutputTransform->m_CommandID = cmdCmb.GetCommandID();

    // sample animation 1
    {
      const void* pThis = this;
      auto& cmd = poseGen.AllocCommandSampleTrack(nsHashingUtils::xxHash32(&pThis, sizeof(pThis), 0));
      cmd.m_hAnimationClip = clip1.m_hClip;
      cmd.m_fPreviousNormalizedSamplePos = tPrevPlayback.AsFloatInSeconds() * fInvDuration;
      cmd.m_fNormalizedSamplePos = pState->m_PlaybackTime.AsFloatInSeconds() * fInvDuration;
      cmd.m_EventSampling = fLerpFactor <= 0.5f ? eventSampling : nsAnimPoseEventTrackSampleMode::None; // only the stronger influence will trigger events

      cmdCmb.m_Inputs.PushBack(cmd.GetCommandID());
      cmdCmb.m_InputWeights.PushBack(1.0f - fLerpFactor);
    }

    // sample animation 2
    {
      const void* pThis = this;
      auto& cmd = poseGen.AllocCommandSampleTrack(nsHashingUtils::xxHash32(&pThis, sizeof(pThis), 1));
      cmd.m_hAnimationClip = clip2.m_hClip;
      cmd.m_fPreviousNormalizedSamplePos = tPrevPlayback.AsFloatInSeconds() * fInvDuration;
      cmd.m_fNormalizedSamplePos = pState->m_PlaybackTime.AsFloatInSeconds() * fInvDuration;
      cmd.m_EventSampling = fLerpFactor > 0.5f ? eventSampling : nsAnimPoseEventTrackSampleMode::None; // only the stronger influence will trigger events

      cmdCmb.m_Inputs.PushBack(cmd.GetCommandID());
      cmdCmb.m_InputWeights.PushBack(fLerpFactor);
    }
  }

  // send to output
  {
    if (m_bApplyRootMotion)
    {
      pOutputTransform->m_bUseRootMotion = true;

      pOutputTransform->m_vRootMotion = nsMath::Lerp(animDesc1.m_vConstantRootMotion, animDesc2.m_vConstantRootMotion, fLerpFactor) * tDiff.AsFloatInSeconds() * fSpeed;
    }

    m_OutPose.SetPose(ref_graph, pOutputTransform);
  }
}

bool nsSampleBlendSpace1DAnimNode::GetInstanceDataDesc(nsInstanceDataDesc& out_desc) const
{
  out_desc.FillFromType<InstanceState>();
  return true;
}


NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_AnimGraph_AnimNodes2_SampleBlendSpace1DAnimNode);

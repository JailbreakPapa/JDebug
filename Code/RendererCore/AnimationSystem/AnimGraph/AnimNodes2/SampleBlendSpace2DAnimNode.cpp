#include <RendererCore/RendererCorePCH.h>

#include <Core/World/GameObject.h>
#include <Core/World/World.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimController.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphInstance.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimNodes2/SampleBlendSpace2DAnimNode.h>
#include <RendererCore/AnimationSystem/AnimPoseGenerator.h>
#include <RendererCore/AnimationSystem/AnimationClipResource.h>
#include <RendererCore/AnimationSystem/SkeletonResource.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsAnimationClip2D, nsNoBase, 1, nsRTTIDefaultAllocator<nsAnimationClip2D>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Clip", GetAnimationFile, SetAnimationFile)->AddAttributes(new nsDynamicStringEnumAttribute("AnimationClipMappingEnum")),
    NS_MEMBER_PROPERTY("Position", m_vPosition),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSampleBlendSpace2DAnimNode, 1, nsRTTIDefaultAllocator<nsSampleBlendSpace2DAnimNode>)
  {
    NS_BEGIN_PROPERTIES
    {
      NS_MEMBER_PROPERTY("Loop", m_bLoop)->AddAttributes(new nsDefaultValueAttribute(true)),
      NS_MEMBER_PROPERTY("PlaybackSpeed", m_fPlaybackSpeed)->AddAttributes(new nsDefaultValueAttribute(1.0f), new nsClampValueAttribute(0.0f, {})),
      NS_MEMBER_PROPERTY("ApplyRootMotion", m_bApplyRootMotion),
      NS_MEMBER_PROPERTY("InputResponse", m_InputResponse)->AddAttributes(new nsDefaultValueAttribute(nsTime::MakeFromMilliseconds(100))),
    NS_ACCESSOR_PROPERTY("CenterClip", GetCenterClipFile, SetCenterClipFile)->AddAttributes(new nsDynamicStringEnumAttribute("AnimationClipMappingEnum")),
      NS_ARRAY_MEMBER_PROPERTY("Clips", m_Clips),

      NS_MEMBER_PROPERTY("InStart", m_InStart)->AddAttributes(new nsHiddenAttribute()),
      NS_MEMBER_PROPERTY("InLoop", m_InLoop)->AddAttributes(new nsHiddenAttribute()),
      NS_MEMBER_PROPERTY("InSpeed", m_InSpeed)->AddAttributes(new nsHiddenAttribute()),
      NS_MEMBER_PROPERTY("X", m_InCoordX)->AddAttributes(new nsHiddenAttribute()),
      NS_MEMBER_PROPERTY("Y", m_InCoordY)->AddAttributes(new nsHiddenAttribute()),

      NS_MEMBER_PROPERTY("OutPose", m_OutPose)->AddAttributes(new nsHiddenAttribute()),
      NS_MEMBER_PROPERTY("OutOnStarted", m_OutOnStarted)->AddAttributes(new nsHiddenAttribute()),
      NS_MEMBER_PROPERTY("OutOnFinished", m_OutOnFinished)->AddAttributes(new nsHiddenAttribute()),
    }
    NS_END_PROPERTIES;
    NS_BEGIN_ATTRIBUTES
    {
      new nsCategoryAttribute("Pose Generation"),
      new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Blue)),
      new nsTitleAttribute("BlendSpace 2D: '{CenterClip}'"),
    }
    NS_END_ATTRIBUTES;
  }
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void nsAnimationClip2D::SetAnimationFile(const char* szFile)
{
  m_sClip.Assign(szFile);
}

const char* nsAnimationClip2D::GetAnimationFile() const
{
  return m_sClip;
}

nsSampleBlendSpace2DAnimNode::nsSampleBlendSpace2DAnimNode() = default;
nsSampleBlendSpace2DAnimNode::~nsSampleBlendSpace2DAnimNode() = default;

nsResult nsSampleBlendSpace2DAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  stream << m_sCenterClip;

  stream << m_Clips.GetCount();
  for (nsUInt32 i = 0; i < m_Clips.GetCount(); ++i)
  {
    stream << m_Clips[i].m_sClip;
    stream << m_Clips[i].m_vPosition;
  }

  stream << m_bLoop;
  stream << m_bApplyRootMotion;
  stream << m_fPlaybackSpeed;
  stream << m_InputResponse;

  NS_SUCCEED_OR_RETURN(m_InStart.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_InLoop.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_InSpeed.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_InCoordX.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_InCoordY.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutPose.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnStarted.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnFinished.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsSampleBlendSpace2DAnimNode::DeserializeNode(nsStreamReader& stream)
{
  const auto version = stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  stream >> m_sCenterClip;

  nsUInt32 num = 0;
  stream >> num;
  m_Clips.SetCount(num);
  for (nsUInt32 i = 0; i < m_Clips.GetCount(); ++i)
  {
    stream >> m_Clips[i].m_sClip;
    stream >> m_Clips[i].m_vPosition;
  }

  stream >> m_bLoop;
  stream >> m_bApplyRootMotion;
  stream >> m_fPlaybackSpeed;
  stream >> m_InputResponse;

  NS_SUCCEED_OR_RETURN(m_InStart.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_InLoop.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_InSpeed.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_InCoordX.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_InCoordY.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutPose.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnStarted.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnFinished.Deserialize(stream));

  return NS_SUCCESS;
}

void nsSampleBlendSpace2DAnimNode::SetCenterClipFile(const char* szFile)
{
  m_sCenterClip.Assign(szFile);
}

const char* nsSampleBlendSpace2DAnimNode::GetCenterClipFile() const
{
  return m_sCenterClip;
}

void nsSampleBlendSpace2DAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  if (!m_OutPose.IsConnected() || (!m_InCoordX.IsConnected() && !m_InCoordY.IsConnected()) || m_Clips.IsEmpty())
    return;

  InstanceState* pState = ref_graph.GetAnimNodeInstanceData<InstanceState>(*this);

  if (m_InStart.IsTriggered(ref_graph))
  {
    pState->m_CenterPlaybackTime = nsTime::MakeZero();
    pState->m_fOtherPlaybackPosNorm = 0.0f;

    m_OutOnStarted.SetTriggered(ref_graph);
  }

  const float x = static_cast<float>(m_InCoordX.GetNumber(ref_graph));
  const float y = static_cast<float>(m_InCoordY.GetNumber(ref_graph));

  if (m_InputResponse.IsZeroOrNegative())
  {
    pState->m_fLastValueX = x;
    pState->m_fLastValueY = y;
  }
  else
  {
    const float lerp = static_cast<float>(nsMath::Min(1.0, tDiff.GetSeconds() * (1.0 / m_InputResponse.GetSeconds())));
    pState->m_fLastValueX = nsMath::Lerp(pState->m_fLastValueX, x, lerp);
    pState->m_fLastValueY = nsMath::Lerp(pState->m_fLastValueY, y, lerp);
  }

  const auto& centerInfo = ref_controller.GetAnimationClipInfo(m_sCenterClip);

  nsUInt32 uiMaxWeightClip = 0;
  nsHybridArray<ClipToPlay, 8> clips;
  ComputeClipsAndWeights(ref_controller, centerInfo, nsVec2(pState->m_fLastValueX, pState->m_fLastValueY), clips, uiMaxWeightClip);

  PlayClips(ref_controller, centerInfo, pState, ref_graph, tDiff, clips, uiMaxWeightClip);
}

void nsSampleBlendSpace2DAnimNode::ComputeClipsAndWeights(nsAnimController& ref_controller, const nsAnimController::AnimClipInfo& centerInfo, const nsVec2& p, nsDynamicArray<ClipToPlay>& clips, nsUInt32& out_uiMaxWeightClip) const
{
  out_uiMaxWeightClip = 0;
  float fMaxWeight = -1.0f;

  if (m_Clips.GetCount() == 1 && !centerInfo.m_hClip.IsValid())
  {
    auto& clip = clips.ExpandAndGetRef();
    clip.m_uiIndex = 0;
    clip.m_pClipInfo = &centerInfo;
  }
  else
  {
    // this algorithm is taken from http://runevision.com/thesis chapter 6.3 "Gradient Band Interpolation"
    // also see http://answers.unity.com/answers/1208837/view.html

    float fWeightNormalization = 0.0f;

    for (nsUInt32 i = 0; i < m_Clips.GetCount(); ++i)
    {
      const auto& clipInfo = ref_controller.GetAnimationClipInfo(m_Clips[i].m_sClip);
      if (!clipInfo.m_hClip.IsValid())
        continue;

      const nsVec2 pi = m_Clips[i].m_vPosition;
      float fMinWeight = 1.0f;

      for (nsUInt32 j = 0; j < m_Clips.GetCount(); ++j)
      {
        const nsVec2 pj = m_Clips[j].m_vPosition;

        const float fLenSqr = (pi - pj).GetLengthSquared();
        const float fProjLenSqr = (pi - p).Dot(pi - pj);

        // filters out both (i == j) and cases where another clip is in the same place and would result in division by zero
        if (fLenSqr <= 0.0f)
          continue;

        const float fWeight = 1.0f - (fProjLenSqr / fLenSqr);
        fMinWeight = nsMath::Min(fMinWeight, fWeight);
      }

      // also check against center clip
      if (centerInfo.m_hClip.IsValid())
      {
        const float fLenSqr = pi.GetLengthSquared();
        const float fProjLenSqr = (pi - p).Dot(pi);

        // filters out both (i == j) and cases where another clip is in the same place and would result in division by zero
        if (fLenSqr <= 0.0f)
          continue;

        const float fWeight = 1.0f - (fProjLenSqr / fLenSqr);
        fMinWeight = nsMath::Min(fMinWeight, fWeight);
      }

      if (fMinWeight > 0.0f)
      {
        auto& c = clips.ExpandAndGetRef();
        c.m_uiIndex = i;
        c.m_fWeight = fMinWeight;
        c.m_pClipInfo = &clipInfo;

        fWeightNormalization += fMinWeight;
      }
    }

    // also compute weight for center clip
    if (centerInfo.m_hClip.IsValid())
    {
      float fMinWeight = 1.0f;

      for (nsUInt32 j = 0; j < m_Clips.GetCount(); ++j)
      {
        const nsVec2 pj = m_Clips[j].m_vPosition;

        const float fLenSqr = pj.GetLengthSquared();
        const float fProjLenSqr = (-p).Dot(-pj);

        // filters out both (i == j) and cases where another clip is in the same place and would result in division by zero
        if (fLenSqr <= 0.0f)
          continue;

        const float fWeight = 1.0f - (fProjLenSqr / fLenSqr);
        fMinWeight = nsMath::Min(fMinWeight, fWeight);
      }

      if (fMinWeight > 0.0f)
      {
        auto& c = clips.ExpandAndGetRef();
        c.m_uiIndex = 0xFFFFFFFF;
        c.m_fWeight = fMinWeight;
        c.m_pClipInfo = &centerInfo;

        fWeightNormalization += fMinWeight;
      }
    }

    fWeightNormalization = 1.0f / fWeightNormalization;

    for (nsUInt32 i = 0; i < clips.GetCount(); ++i)
    {
      auto& c = clips[i];

      c.m_fWeight *= fWeightNormalization;

      if (c.m_fWeight > fMaxWeight)
      {
        fMaxWeight = c.m_fWeight;
        out_uiMaxWeightClip = i;
      }
    }
  }
}

void nsSampleBlendSpace2DAnimNode::PlayClips(nsAnimController& ref_controller, const nsAnimController::AnimClipInfo& centerInfo, InstanceState* pState, nsAnimGraphInstance& ref_graph, nsTime tDiff, nsArrayPtr<ClipToPlay> clips, nsUInt32 uiMaxWeightClip) const
{
  const bool bLoop = m_InLoop.GetBool(ref_graph, m_bLoop);
  const float fSpeed = static_cast<float>(m_InSpeed.GetNumber(ref_graph, m_fPlaybackSpeed));

  nsTime tAvgDuration = nsTime::MakeZero();

  nsHybridArray<nsAnimPoseGeneratorCommandSampleTrack*, 8> pSampleTrack;
  pSampleTrack.SetCountUninitialized(clips.GetCount());

  nsVec3 vRootMotion = nsVec3::MakeZero();
  nsUInt32 uiNumAvgClips = 0;

  for (nsUInt32 i = 0; i < clips.GetCount(); ++i)
  {
    const auto& c = clips[i];

    const nsHashedString sClip = c.m_uiIndex >= 0xFF ? m_sCenterClip : m_Clips[c.m_uiIndex].m_sClip;

    const auto& clipInfo = *clips[i].m_pClipInfo;

    nsResourceLock<nsAnimationClipResource> pClip(clipInfo.m_hClip, nsResourceAcquireMode::BlockTillLoaded);

    if (c.m_uiIndex < 0xFF) // center clip should not contribute to the average time
    {
      ++uiNumAvgClips;
      tAvgDuration += pClip->GetDescriptor().GetDuration();
    }

    const void* pThis = this;
    auto& cmd = ref_controller.GetPoseGenerator().AllocCommandSampleTrack(nsHashingUtils::xxHash32(&pThis, sizeof(pThis), i));
    cmd.m_hAnimationClip = clipInfo.m_hClip;
    cmd.m_fNormalizedSamplePos = pClip->GetDescriptor().GetDuration().AsFloatInSeconds(); // will be combined with actual pos below

    pSampleTrack[i] = &cmd;
    vRootMotion += pClip->GetDescriptor().m_vConstantRootMotion * c.m_fWeight;
  }

  if (uiNumAvgClips > 0)
  {
    tAvgDuration = tAvgDuration / uiNumAvgClips;
  }

  tAvgDuration = nsMath::Max(tAvgDuration, nsTime::MakeFromMilliseconds(16));

  const nsTime fPrevCenterPlaybackPos = pState->m_CenterPlaybackTime;
  const float fPrevPlaybackPosNorm = pState->m_fOtherPlaybackPosNorm;

  nsAnimPoseEventTrackSampleMode eventSamplingCenter = nsAnimPoseEventTrackSampleMode::OnlyBetween;
  nsAnimPoseEventTrackSampleMode eventSampling = nsAnimPoseEventTrackSampleMode::OnlyBetween;

  const float fInvAvgDuration = 1.0f / tAvgDuration.AsFloatInSeconds();
  const float tDiffNorm = tDiff.AsFloatInSeconds() * fInvAvgDuration;

  // now that we know the duration, we can finally update the playback state
  pState->m_fOtherPlaybackPosNorm += tDiffNorm * fSpeed;
  while (pState->m_fOtherPlaybackPosNorm >= 1.0f)
  {
    if (bLoop)
    {
      pState->m_fOtherPlaybackPosNorm -= 1.0f;
      m_OutOnStarted.SetTriggered(ref_graph);
      eventSampling = nsAnimPoseEventTrackSampleMode::LoopAtEnd;
    }
    else
    {
      pState->m_fOtherPlaybackPosNorm = 1.0f;

      if (fPrevPlaybackPosNorm < 1.0f)
      {
        m_OutOnFinished.SetTriggered(ref_graph);
      }
      else
      {
        eventSampling = nsAnimPoseEventTrackSampleMode::None;
      }

      break;
    }
  }

  UpdateCenterClipPlaybackTime(centerInfo, pState, ref_graph, tDiff, eventSamplingCenter);

  for (nsUInt32 i = 0; i < clips.GetCount(); ++i)
  {
    if (pSampleTrack[i]->m_hAnimationClip == centerInfo.m_hClip)
    {
      pSampleTrack[i]->m_fPreviousNormalizedSamplePos = fPrevCenterPlaybackPos.AsFloatInSeconds() / pSampleTrack[i]->m_fNormalizedSamplePos;
      pSampleTrack[i]->m_fNormalizedSamplePos = pState->m_CenterPlaybackTime.AsFloatInSeconds() / pSampleTrack[i]->m_fNormalizedSamplePos;
      pSampleTrack[i]->m_EventSampling = uiMaxWeightClip == i ? eventSamplingCenter : nsAnimPoseEventTrackSampleMode::None;
    }
    else
    {
      pSampleTrack[i]->m_fPreviousNormalizedSamplePos = fPrevPlaybackPosNorm;
      pSampleTrack[i]->m_fNormalizedSamplePos = pState->m_fOtherPlaybackPosNorm;
      pSampleTrack[i]->m_EventSampling = uiMaxWeightClip == i ? eventSampling : nsAnimPoseEventTrackSampleMode::None;
    }
  }

  nsAnimGraphPinDataLocalTransforms* pOutputTransform = ref_controller.AddPinDataLocalTransforms();

  if (m_bApplyRootMotion)
  {
    pOutputTransform->m_bUseRootMotion = true;

    const float fSpeed = static_cast<float>(m_InSpeed.GetNumber(ref_graph, m_fPlaybackSpeed));

    pOutputTransform->m_vRootMotion = tDiff.AsFloatInSeconds() * vRootMotion * fSpeed;
  }

  if (clips.GetCount() == 1)
  {
    pOutputTransform->m_CommandID = pSampleTrack[0]->GetCommandID();
  }
  else
  {
    auto& cmdCmb = ref_controller.GetPoseGenerator().AllocCommandCombinePoses();
    pOutputTransform->m_CommandID = cmdCmb.GetCommandID();

    cmdCmb.m_InputWeights.SetCountUninitialized(clips.GetCount());
    cmdCmb.m_Inputs.SetCountUninitialized(clips.GetCount());

    for (nsUInt32 i = 0; i < clips.GetCount(); ++i)
    {
      cmdCmb.m_InputWeights[i] = clips[i].m_fWeight;
      cmdCmb.m_Inputs[i] = pSampleTrack[i]->GetCommandID();
    }
  }

  m_OutPose.SetPose(ref_graph, pOutputTransform);
}

void nsSampleBlendSpace2DAnimNode::UpdateCenterClipPlaybackTime(const nsAnimController::AnimClipInfo& centerInfo, InstanceState* pState, nsAnimGraphInstance& ref_graph, nsTime tDiff, nsAnimPoseEventTrackSampleMode& out_eventSamplingCenter) const
{
  const float fSpeed = static_cast<float>(m_InSpeed.GetNumber(ref_graph, m_fPlaybackSpeed));

  if (centerInfo.m_hClip.IsValid())
  {
    nsResourceLock<nsAnimationClipResource> pClip(centerInfo.m_hClip, nsResourceAcquireMode::BlockTillLoaded);

    const nsTime tDur = pClip->GetDescriptor().GetDuration();

    pState->m_CenterPlaybackTime += tDiff * fSpeed;

    // always loop the center clip
    while (pState->m_CenterPlaybackTime > tDur)
    {
      pState->m_CenterPlaybackTime -= tDur;
      out_eventSamplingCenter = nsAnimPoseEventTrackSampleMode::LoopAtEnd;
    }
  }
}

bool nsSampleBlendSpace2DAnimNode::GetInstanceDataDesc(nsInstanceDataDesc& out_desc) const
{
  out_desc.FillFromType<InstanceState>();
  return true;
}


NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_AnimGraph_AnimNodes2_SampleBlendSpace2DAnimNode);

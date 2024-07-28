#include <RendererCore/RendererCorePCH.h>

#include <Core/World/GameObject.h>
#include <Core/World/World.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimController.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphInstance.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimNodes2/SampleAnimClipSequenceAnimNode.h>
#include <RendererCore/AnimationSystem/AnimPoseGenerator.h>
#include <RendererCore/AnimationSystem/SkeletonResource.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSampleAnimClipSequenceAnimNode, 1, nsRTTIDefaultAllocator<nsSampleAnimClipSequenceAnimNode>)
  {
    NS_BEGIN_PROPERTIES
    {
      NS_MEMBER_PROPERTY("PlaybackSpeed", m_fPlaybackSpeed)->AddAttributes(new nsDefaultValueAttribute(1.0f), new nsClampValueAttribute(0.0f, {})),
      NS_MEMBER_PROPERTY("Loop", m_bLoop),
      //NS_MEMBER_PROPERTY("ApplyRootMotion", m_bApplyRootMotion),
      NS_ACCESSOR_PROPERTY("StartClip", GetStartClip, SetStartClip)->AddAttributes(new nsDynamicStringEnumAttribute("AnimationClipMappingEnum")),
      NS_ARRAY_ACCESSOR_PROPERTY("MiddleClips", Clips_GetCount, Clips_GetValue, Clips_SetValue, Clips_Insert, Clips_Remove)->AddAttributes(new nsDynamicStringEnumAttribute("AnimationClipMappingEnum")),
      NS_ACCESSOR_PROPERTY("EndClip", GetEndClip, SetEndClip)->AddAttributes(new nsDynamicStringEnumAttribute("AnimationClipMappingEnum")),

      NS_MEMBER_PROPERTY("InStart", m_InStart)->AddAttributes(new nsHiddenAttribute()),
      NS_MEMBER_PROPERTY("InLoop", m_InLoop)->AddAttributes(new nsHiddenAttribute()),
      NS_MEMBER_PROPERTY("InSpeed", m_InSpeed)->AddAttributes(new nsHiddenAttribute()),
      NS_MEMBER_PROPERTY("ClipIndex", m_ClipIndexPin)->AddAttributes(new nsHiddenAttribute()),

      NS_MEMBER_PROPERTY("OutPose", m_OutPose)->AddAttributes(new nsHiddenAttribute()),
      NS_MEMBER_PROPERTY("OutOnMiddleStarted", m_OutOnMiddleStarted)->AddAttributes(new nsHiddenAttribute()),
      NS_MEMBER_PROPERTY("OutOnEndStarted", m_OutOnEndStarted)->AddAttributes(new nsHiddenAttribute()),
      NS_MEMBER_PROPERTY("OutOnFinished", m_OutOnFinished)->AddAttributes(new nsHiddenAttribute()),
    }
    NS_END_PROPERTIES;
    NS_BEGIN_ATTRIBUTES
    {
      new nsCategoryAttribute("Pose Generation"),
      new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Blue)),
      new nsTitleAttribute("Sample Sequence: '{StartClip}' '{Clip}' '{EndClip}'"),
    }
    NS_END_ATTRIBUTES;
  }
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsSampleAnimClipSequenceAnimNode::nsSampleAnimClipSequenceAnimNode() = default;
nsSampleAnimClipSequenceAnimNode::~nsSampleAnimClipSequenceAnimNode() = default;

nsResult nsSampleAnimClipSequenceAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  stream << m_sStartClip;
  NS_SUCCEED_OR_RETURN(stream.WriteArray(m_Clips));
  stream << m_sEndClip;
  stream << m_bApplyRootMotion;
  stream << m_bLoop;
  stream << m_fPlaybackSpeed;

  NS_SUCCEED_OR_RETURN(m_InStart.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_InLoop.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_InSpeed.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_ClipIndexPin.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutPose.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnMiddleStarted.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnEndStarted.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnFinished.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsSampleAnimClipSequenceAnimNode::DeserializeNode(nsStreamReader& stream)
{
  const auto version = stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  stream >> m_sStartClip;
  NS_SUCCEED_OR_RETURN(stream.ReadArray(m_Clips));
  stream >> m_sEndClip;
  stream >> m_bApplyRootMotion;
  stream >> m_bLoop;
  stream >> m_fPlaybackSpeed;

  NS_SUCCEED_OR_RETURN(m_InStart.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_InLoop.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_InSpeed.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_ClipIndexPin.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutPose.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnMiddleStarted.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnEndStarted.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnFinished.Deserialize(stream));

  return NS_SUCCESS;
}

nsUInt32 nsSampleAnimClipSequenceAnimNode::Clips_GetCount() const
{
  return m_Clips.GetCount();
}

const char* nsSampleAnimClipSequenceAnimNode::Clips_GetValue(nsUInt32 uiIndex) const
{
  return m_Clips[uiIndex];
}

void nsSampleAnimClipSequenceAnimNode::Clips_SetValue(nsUInt32 uiIndex, const char* szValue)
{
  m_Clips[uiIndex].Assign(szValue);
}

void nsSampleAnimClipSequenceAnimNode::Clips_Insert(nsUInt32 uiIndex, const char* szValue)
{
  nsHashedString s;
  s.Assign(szValue);
  m_Clips.InsertAt(uiIndex, s);
}

void nsSampleAnimClipSequenceAnimNode::Clips_Remove(nsUInt32 uiIndex)
{
  m_Clips.RemoveAtAndCopy(uiIndex);
}

void nsSampleAnimClipSequenceAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  if (!m_OutPose.IsConnected())
    return;

  InstanceState* pState = ref_graph.GetAnimNodeInstanceData<InstanceState>(*this);

  if (m_InStart.IsTriggered(ref_graph))
  {
    pState->m_PlaybackTime = nsTime::MakeZero();
    pState->m_State = State::Start;
  }

  const bool bLoop = m_InLoop.GetBool(ref_graph, m_bLoop);

  // currently we only support playing clips forwards
  const float fPlaySpeed = nsMath::Max(0.0f, static_cast<float>(m_InSpeed.GetNumber(ref_graph, m_fPlaybackSpeed)));

  nsTime tPrevSamplePos = pState->m_PlaybackTime;
  pState->m_PlaybackTime += tDiff * fPlaySpeed;

  nsAnimationClipResourceHandle hCurClip;
  nsTime tCurDuration;

  while (pState->m_State != State::Off)
  {
    if (pState->m_State == State::Start)
    {
      const auto& startClip = ref_controller.GetAnimationClipInfo(m_sStartClip);

      if (!startClip.m_hClip.IsValid())
      {
        if (!m_Clips.IsEmpty())
        {
          pState->m_uiMiddleClipIdx = m_ClipIndexPin.GetNumber(ref_graph, 0xFF);
          if (pState->m_uiMiddleClipIdx >= m_Clips.GetCount())
          {
            pState->m_uiMiddleClipIdx = pTarget->GetWorld()->GetRandomNumberGenerator().UIntInRange(m_Clips.GetCount());
          }
        }

        pState->m_State = State::Middle;
        m_OutOnMiddleStarted.SetTriggered(ref_graph);
        continue;
      }

      nsResourceLock<nsAnimationClipResource> pAnimClip(startClip.m_hClip, nsResourceAcquireMode::BlockTillLoaded_NeverFail);
      if (pAnimClip.GetAcquireResult() != nsResourceAcquireResult::Final)
      {
        if (!m_Clips.IsEmpty())
        {
          pState->m_uiMiddleClipIdx = m_ClipIndexPin.GetNumber(ref_graph, 0xFF);
          if (pState->m_uiMiddleClipIdx >= m_Clips.GetCount())
          {
            pState->m_uiMiddleClipIdx = pTarget->GetWorld()->GetRandomNumberGenerator().UIntInRange(m_Clips.GetCount());
          }
        }

        pState->m_State = State::Middle;
        m_OutOnMiddleStarted.SetTriggered(ref_graph);
        continue;
      }

      tCurDuration = pAnimClip->GetDescriptor().GetDuration();
      NS_ASSERT_DEBUG(tCurDuration >= nsTime::MakeFromMilliseconds(5), "Too short clip");

      if (pState->m_PlaybackTime >= tCurDuration)
      {
        // TODO: sample anim events of previous clip
        m_OutOnMiddleStarted.SetTriggered(ref_graph);
        tPrevSamplePos = nsTime::MakeZero();
        pState->m_PlaybackTime -= tCurDuration;
        pState->m_State = State::Middle;

        if (!m_Clips.IsEmpty())
        {
          pState->m_uiMiddleClipIdx = m_ClipIndexPin.GetNumber(ref_graph, 0xFF);
          if (pState->m_uiMiddleClipIdx >= m_Clips.GetCount())
          {
            pState->m_uiMiddleClipIdx = pTarget->GetWorld()->GetRandomNumberGenerator().UIntInRange(m_Clips.GetCount());
          }
        }
        continue;
      }

      hCurClip = startClip.m_hClip;
      break;
    }

    if (pState->m_State == State::Middle)
    {
      if (m_Clips.IsEmpty())
      {
        pState->m_State = State::End;
        m_OutOnEndStarted.SetTriggered(ref_graph);
        continue;
      }

      const auto& clipInfo = ref_controller.GetAnimationClipInfo(m_Clips[pState->m_uiMiddleClipIdx]);

      if (!clipInfo.m_hClip.IsValid())
      {
        pState->m_State = State::End;
        m_OutOnEndStarted.SetTriggered(ref_graph);
        continue;
      }

      nsResourceLock<nsAnimationClipResource> pAnimClip(clipInfo.m_hClip, nsResourceAcquireMode::BlockTillLoaded_NeverFail);
      if (pAnimClip.GetAcquireResult() != nsResourceAcquireResult::Final)
      {
        pState->m_State = State::End;
        m_OutOnEndStarted.SetTriggered(ref_graph);
        continue;
      }

      tCurDuration = pAnimClip->GetDescriptor().GetDuration();
      NS_ASSERT_DEBUG(tCurDuration >= nsTime::MakeFromMilliseconds(5), "Too short clip");

      if (pState->m_PlaybackTime >= tCurDuration)
      {
        // TODO: sample anim events of previous clip
        tPrevSamplePos = nsTime::MakeZero();
        pState->m_PlaybackTime -= tCurDuration;

        if (bLoop)
        {
          m_OutOnMiddleStarted.SetTriggered(ref_graph);
          pState->m_State = State::Middle;

          pState->m_uiMiddleClipIdx = m_ClipIndexPin.GetNumber(ref_graph, 0xFF);
          if (pState->m_uiMiddleClipIdx >= m_Clips.GetCount())
          {
            pState->m_uiMiddleClipIdx = pTarget->GetWorld()->GetRandomNumberGenerator().UIntInRange(m_Clips.GetCount());
          }
        }
        else
        {
          m_OutOnEndStarted.SetTriggered(ref_graph);
          pState->m_State = State::End;
        }
        continue;
      }

      hCurClip = clipInfo.m_hClip;
      break;
    }

    if (pState->m_State == State::End)
    {
      const auto& endClip = ref_controller.GetAnimationClipInfo(m_sEndClip);

      if (!endClip.m_hClip.IsValid())
      {
        pState->m_State = State::HoldMiddleFrame;
        m_OutOnFinished.SetTriggered(ref_graph);
        continue;
      }

      nsResourceLock<nsAnimationClipResource> pAnimClip(endClip.m_hClip, nsResourceAcquireMode::BlockTillLoaded_NeverFail);
      if (pAnimClip.GetAcquireResult() != nsResourceAcquireResult::Final)
      {
        pState->m_State = State::HoldMiddleFrame;
        m_OutOnFinished.SetTriggered(ref_graph);
        continue;
      }

      tCurDuration = pAnimClip->GetDescriptor().GetDuration();
      NS_ASSERT_DEBUG(tCurDuration >= nsTime::MakeFromMilliseconds(5), "Too short clip");

      if (pState->m_PlaybackTime >= tCurDuration)
      {
        // TODO: sample anim events of previous clip
        m_OutOnFinished.SetTriggered(ref_graph);
        pState->m_State = State::HoldEndFrame;
        continue;
      }

      hCurClip = endClip.m_hClip;
      break;
    }

    if (pState->m_State == State::HoldEndFrame)
    {
      const auto& endClip = ref_controller.GetAnimationClipInfo(m_sEndClip);
      hCurClip = endClip.m_hClip;

      nsResourceLock<nsAnimationClipResource> pAnimClip(endClip.m_hClip, nsResourceAcquireMode::BlockTillLoaded_NeverFail);
      tCurDuration = pAnimClip->GetDescriptor().GetDuration();
      pState->m_PlaybackTime = tCurDuration;
      break;
    }

    if (pState->m_State == State::HoldMiddleFrame)
    {
      if (m_Clips.IsEmpty() || pState->m_uiMiddleClipIdx >= m_Clips.GetCount())
      {
        pState->m_State = State::HoldStartFrame;
        continue;
      }

      const auto& clipInfo = ref_controller.GetAnimationClipInfo(m_Clips[pState->m_uiMiddleClipIdx]);

      if (!clipInfo.m_hClip.IsValid())
      {
        pState->m_State = State::HoldStartFrame;
        continue;
      }

      nsResourceLock<nsAnimationClipResource> pAnimClip(clipInfo.m_hClip, nsResourceAcquireMode::BlockTillLoaded_NeverFail);
      if (pAnimClip.GetAcquireResult() != nsResourceAcquireResult::Final)
      {
        pState->m_State = State::HoldStartFrame;
        continue;
      }

      tCurDuration = pAnimClip->GetDescriptor().GetDuration();
      pState->m_PlaybackTime = tCurDuration;
      hCurClip = clipInfo.m_hClip;
      break;
    }

    if (pState->m_State == State::HoldStartFrame)
    {
      const auto& startClip = ref_controller.GetAnimationClipInfo(m_sStartClip);

      if (!startClip.m_hClip.IsValid())
      {
        pState->m_State = State::Off;
        continue;
      }

      nsResourceLock<nsAnimationClipResource> pAnimClip(startClip.m_hClip, nsResourceAcquireMode::BlockTillLoaded_NeverFail);
      if (pAnimClip.GetAcquireResult() != nsResourceAcquireResult::Final)
      {
        pState->m_State = State::Off;
        continue;
      }

      tCurDuration = pAnimClip->GetDescriptor().GetDuration();
      NS_ASSERT_DEBUG(tCurDuration >= nsTime::MakeFromMilliseconds(5), "Too short clip");

      hCurClip = startClip.m_hClip;
      pState->m_PlaybackTime = tCurDuration;
      break;
    }
  }

  if (!hCurClip.IsValid())
    return;

  const float fInvDuration = 1.0f / tCurDuration.AsFloatInSeconds();

  const void* pThis = this;
  auto& cmd = ref_controller.GetPoseGenerator().AllocCommandSampleTrack(nsHashingUtils::xxHash32(&pThis, sizeof(pThis)));
  cmd.m_EventSampling = nsAnimPoseEventTrackSampleMode::OnlyBetween;

  cmd.m_hAnimationClip = hCurClip;
  cmd.m_fPreviousNormalizedSamplePos = nsMath::Clamp(tPrevSamplePos.AsFloatInSeconds() * fInvDuration, 0.0f, 1.0f);
  cmd.m_fNormalizedSamplePos = nsMath::Clamp(pState->m_PlaybackTime.AsFloatInSeconds() * fInvDuration, 0.0f, 1.0f);

  {
    nsAnimGraphPinDataLocalTransforms* pLocalTransforms = ref_controller.AddPinDataLocalTransforms();

    pLocalTransforms->m_pWeights = nullptr;
    pLocalTransforms->m_bUseRootMotion = false; // m_bApplyRootMotion;
    pLocalTransforms->m_fOverallWeight = 1.0f;
    // pLocalTransforms->m_vRootMotion = pAnimClip->GetDescriptor().m_vConstantRootMotion * tDiff.AsFloatInSeconds() * fPlaySpeed;
    pLocalTransforms->m_CommandID = cmd.GetCommandID();

    m_OutPose.SetPose(ref_graph, pLocalTransforms);
  }
}

void nsSampleAnimClipSequenceAnimNode::SetStartClip(const char* szClip)
{
  m_sStartClip.Assign(szClip);
}

const char* nsSampleAnimClipSequenceAnimNode::GetStartClip() const
{
  return m_sStartClip;
}

void nsSampleAnimClipSequenceAnimNode::SetEndClip(const char* szClip)
{
  m_sEndClip.Assign(szClip);
}

const char* nsSampleAnimClipSequenceAnimNode::GetEndClip() const
{
  return m_sEndClip;
}

bool nsSampleAnimClipSequenceAnimNode::GetInstanceDataDesc(nsInstanceDataDesc& out_desc) const
{
  out_desc.FillFromType<InstanceState>();
  return true;
}


NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_AnimGraph_AnimNodes2_SampleAnimClipSequenceAnimNode);

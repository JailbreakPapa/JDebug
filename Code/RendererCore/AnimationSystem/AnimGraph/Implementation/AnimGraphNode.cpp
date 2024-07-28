#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>
#include <RendererCore/AnimationSystem/AnimationClipResource.h>
#include <RendererCore/AnimationSystem/SkeletonResource.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAnimGraphNode, 1, nsRTTINoAllocator)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("CustomTitle", GetCustomNodeTitle, SetCustomNodeTitle),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsAnimGraphNode::nsAnimGraphNode() = default;
nsAnimGraphNode::~nsAnimGraphNode() = default;

nsResult nsAnimGraphNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  // no need to serialize this, not used at runtime
  // stream << m_CustomNodeTitle;

  return NS_SUCCESS;
}

nsResult nsAnimGraphNode::DeserializeNode(nsStreamReader& stream)
{
  stream.ReadVersion(1);

  // no need to serialize this, not used at runtime
  // stream >> m_CustomNodeTitle;

  return NS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsAnimState, nsNoBase, 1, nsRTTIDefaultAllocator<nsAnimState>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Loop", m_bLoop),
    NS_MEMBER_PROPERTY("ApplyRootMotion", m_bApplyRootMotion),
    NS_MEMBER_PROPERTY("PlaybackSpeed", m_fPlaybackSpeed)->AddAttributes(new nsDefaultValueAttribute(1.0f)),
    NS_MEMBER_PROPERTY("FadeIn", m_FadeIn),
    NS_MEMBER_PROPERTY("FadeOut", m_FadeOut),
    NS_MEMBER_PROPERTY("ImmediateFadeIn", m_bImmediateFadeIn),
    NS_MEMBER_PROPERTY("ImmediateFadeOut", m_bImmediateFadeOut),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

void nsAnimState::RampWeightUpOrDown(float& inout_fWeight, float fTargetWeight, nsTime tDiff) const
{
  if (inout_fWeight < fTargetWeight)
  {
    if (m_FadeIn.IsZeroOrNegative())
    {
      inout_fWeight = fTargetWeight;
    }
    else
    {
      inout_fWeight += tDiff.AsFloatInSeconds() / m_FadeIn.AsFloatInSeconds();
      inout_fWeight = nsMath::Min(inout_fWeight, fTargetWeight);
    }
  }
  else if (inout_fWeight > fTargetWeight)
  {
    if (m_FadeOut.IsZeroOrNegative())
    {
      inout_fWeight = fTargetWeight;
    }
    else
    {
      inout_fWeight -= tDiff.AsFloatInSeconds() / m_FadeOut.AsFloatInSeconds();
      inout_fWeight = nsMath::Max(fTargetWeight, inout_fWeight);
    }
  }
}

bool nsAnimState::WillStateBeOff(bool bTriggerActive) const
{
  return m_State == State::Off && !bTriggerActive;
}

void nsAnimState::UpdateState(nsTime diff)
{
  NS_ASSERT_DEV(!m_Duration.IsZeroOrNegative(), "Invalid animation clip duration");

  m_bHasTransitioned = false;
  m_bHasLoopedStart = false;
  m_bHasLoopedEnd = false;

  // how much time delta to apply to the weight ramp
  nsTime tRampUpDownDiff = diff;

  // first update the state machine transitions

  if (m_State == State::Off)
  {
    // in the OFF state we can only transition to the ramping up state

    if (m_bTriggerActive == false)
      return;

    m_fNormalizedPlaybackPosition = 0.0f;
    m_State = State::StartedRampUp;
  }
  else if (m_State == State::StartedRampUp || m_State == State::RampingUp)
  {
    // StartedRampUp MUST be active for a full frame, even if the weight immediately reaches maximum
    // because other code might use this state to fire events

    m_State = State::RampingUp;

    // we must transition to Running here, instead of after the weight update, otherwise the StartedRampUp state could get skipped
    if (m_fCurWeight >= 1.0f)
    {
      m_bRequireLoopForRampDown = true; // force one frame evaluation, before ramp down is allowed
      m_State = State::Running;
    }

    // already taken care of in the Running state check below
    // if (!m_bTriggerActive && m_bImmediateFadeOut)
    //  m_State = State::StartedRampDown;
  }
  else if (m_State == State::StartedRampDown || m_State == State::RampingDown)
  {
    // StartedRampDown MUST be active for a full frame, even if the weight immediately reaches minimum
    // because other code might use this state to fire events

    m_State = State::RampingDown;

    // we must transition to Finished here, instead of after the weight update, otherwise the StartedRampDown state could get skipped
    if (m_fCurWeight <= 0.0f)
      m_State = State::Finished;

    // can only ramp up at arbitrary times, if the animation is looped
    if (m_bTriggerActive && m_bImmediateFadeIn && m_bLoop)
      m_State = State::StartedRampUp;
  }
  else if (m_State == State::Finished)
  {
    // in the finished state we can either switch off or restart
    // if m_bImmediateFadeIn is off, the Finished state should be visible for a full frame

    if (m_bTriggerActive && m_bImmediateFadeIn)
    {
      m_State = State::StartedRampUp;
    }
    else
    {
      m_State = State::Off;
      return;
    }
  }

  // no "else if" here, because the Running state can be skipped and transition immediately to the StartedRampDown state
  if (m_State == State::Running)
  {
    if (!m_bTriggerActive && m_bImmediateFadeOut)
    {
      m_State = State::StartedRampDown;
    }
  }

  const float fSpeed = m_fPlaybackSpeed * m_fPlaybackSpeedFactor;

  float fInvDuration = 1.0f / m_Duration.AsFloatInSeconds();
  float fNormalizedStep = diff.AsFloatInSeconds() * fInvDuration;

  // calculate the new playback position
  {
    m_fNormalizedPlaybackPosition += fSpeed * fNormalizedStep;

    if (m_fNormalizedPlaybackPosition > 1.0f && m_DurationOfQueued.IsPositive())
    {
      m_fNormalizedPlaybackPosition -= 1.0f;

      m_bHasTransitioned = true;

      diff = m_fNormalizedPlaybackPosition * m_Duration;

      m_Duration = m_DurationOfQueued;
      fInvDuration = 1.0f / m_Duration.AsFloatInSeconds();
      fNormalizedStep = diff.AsFloatInSeconds() * fInvDuration;
    }
  }

  if (m_State == State::Running)
  {
    bool bIsInRampDownArea = false;

    const float tRampDownNorm = m_FadeOut.AsFloatInSeconds() * fInvDuration;
    float tInRampDownArea;

    if (fSpeed >= 0)
    {
      // ramp down area at the end of the clip
      const float tEnd = 1.0f - tRampDownNorm;

      bIsInRampDownArea = m_fNormalizedPlaybackPosition > tEnd;
      tInRampDownArea = nsMath::Min(fNormalizedStep, m_fNormalizedPlaybackPosition - tEnd);
    }
    else
    {
      // ramp down area at the beginning of the clip

      bIsInRampDownArea = m_fNormalizedPlaybackPosition < tRampDownNorm;
      tInRampDownArea = nsMath::Min(fNormalizedStep, tRampDownNorm - m_fNormalizedPlaybackPosition);
    }

    if (m_bLoop)
    {
      // exit the loop, if the user deactivated the clip
      if (!m_bTriggerActive)
      {
        if (!m_bRequireLoopForRampDown && bIsInRampDownArea)
        {
          m_State = State::StartedRampDown;

          // recompute tRampUpDownDiff for exact adjustment
          tRampUpDownDiff = tInRampDownArea * m_Duration;
        }
      }
    }
    else
    {
      if (!m_bTriggerActive && bIsInRampDownArea)
      {
        m_State = State::StartedRampDown;

        // recompute tRampUpDownDiff for exact adjustment
        tRampUpDownDiff = tInRampDownArea * m_Duration;
      }
    }

    m_bRequireLoopForRampDown = bIsInRampDownArea;
  }

  // update the overall weight
  {
    if (m_State == State::StartedRampUp || m_State == State::RampingUp)
    {
      RampWeightUpOrDown(m_fCurWeight, 1.0f, tRampUpDownDiff);
    }
    else if (m_State == State::StartedRampDown || m_State == State::RampingDown)
    {
      RampWeightUpOrDown(m_fCurWeight, 0.0f, tRampUpDownDiff);
    }
  }

  // finally loop or clamp the playback position
  if (m_bLoop)
  {
    // if playback speed is positive
    while (m_fNormalizedPlaybackPosition > 1.0f)
    {
      m_fNormalizedPlaybackPosition -= 1.0f;
      m_bRequireLoopForRampDown = false;
      m_bHasLoopedEnd = true;
    }

    // if playback speed is negative
    while (m_fNormalizedPlaybackPosition < 0.0)
    {
      m_fNormalizedPlaybackPosition += 1.0f;
      m_bRequireLoopForRampDown = false;
      m_bHasLoopedStart = true;
    }
  }
  else
  {
    if (m_fNormalizedPlaybackPosition > 1.0f)
    {
      // if playback speed is positive
      m_fNormalizedPlaybackPosition = 1.0f;
    }
    else if (m_fNormalizedPlaybackPosition < 0.0f)
    {
      // if playback speed is negative
      m_fNormalizedPlaybackPosition = 0.0f;
    }
  }
}

nsResult nsAnimState::Serialize(nsStreamWriter& inout_stream) const
{
  inout_stream.WriteVersion(1);

  inout_stream << m_FadeIn;
  inout_stream << m_FadeOut;

  inout_stream << m_bImmediateFadeIn;
  inout_stream << m_bImmediateFadeOut;
  inout_stream << m_bLoop;
  inout_stream << m_fPlaybackSpeed;
  inout_stream << m_bApplyRootMotion;

  return NS_SUCCESS;
}

nsResult nsAnimState::Deserialize(nsStreamReader& inout_stream)
{
  inout_stream.ReadVersion(1);

  inout_stream >> m_FadeIn;
  inout_stream >> m_FadeOut;

  inout_stream >> m_bImmediateFadeIn;
  inout_stream >> m_bImmediateFadeOut;
  inout_stream >> m_bLoop;
  inout_stream >> m_fPlaybackSpeed;
  inout_stream >> m_bApplyRootMotion;

  return NS_SUCCESS;
}


NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_AnimGraph_Implementation_AnimGraphNode);

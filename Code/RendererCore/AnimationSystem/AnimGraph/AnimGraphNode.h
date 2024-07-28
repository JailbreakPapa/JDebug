#pragma once

#include <Core/ResourceManager/ResourceHandle.h>
#include <Foundation/Time/Time.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphPins.h>

class nsSkeletonResource;
class nsGameObject;
class nsAnimGraphInstance;
class nsAnimController;
class nsStreamWriter;
class nsStreamReader;
struct nsAnimGraphPinDataLocalTransforms;
struct nsAnimGraphPinDataBoneWeights;
class nsAnimationClipResource;
struct nsInstanceDataDesc;

using nsAnimationClipResourceHandle = nsTypedResourceHandle<class nsAnimationClipResource>;

namespace ozz
{
  namespace animation
  {
    class Animation;
  }
} // namespace ozz

/// \brief Base class for all nodes in an nsAnimGraphInstance
///
/// These nodes are used to configure which skeletal animations can be played on an object,
/// and how they would be played back exactly.
/// The nodes implement different functionality. For example logic nodes are used to figure out how to play an animation,
/// other nodes then sample and combining animation poses, and yet other nodes can inform the user about events
/// or they write state back to the animation graph's blackboard.
class NS_RENDERERCORE_DLL nsAnimGraphNode : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsAnimGraphNode, nsReflectedClass);

public:
  nsAnimGraphNode();
  virtual ~nsAnimGraphNode();

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

  const char* GetCustomNodeTitle() const { return m_sCustomNodeTitle.GetString(); }
  void SetCustomNodeTitle(const char* szSz) { m_sCustomNodeTitle.Assign(szSz); }

protected:
  friend class nsAnimGraphInstance;
  friend class nsAnimGraph;
  friend class nsAnimGraphResource;

  nsHashedString m_sCustomNodeTitle;
  nsUInt32 m_uiInstanceDataOffset = nsInvalidIndex;

  virtual nsResult SerializeNode(nsStreamWriter& stream) const = 0;
  virtual nsResult DeserializeNode(nsStreamReader& stream) = 0;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const = 0;
  virtual bool GetInstanceDataDesc(nsInstanceDataDesc& out_desc) const { return false; }
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct NS_RENDERERCORE_DLL nsAnimState
{
  enum class State
  {
    Off,
    StartedRampUp,
    RampingUp,
    Running,
    StartedRampDown,
    RampingDown,
    Finished,
  };

  // Properties:
  nsTime m_FadeIn;                  // [ property ]
  nsTime m_FadeOut;                 // [ property ]
  bool m_bImmediateFadeIn = false;  // [ property ]
  bool m_bImmediateFadeOut = false; // [ property ]
  bool m_bLoop = false;             // [ property ]
  float m_fPlaybackSpeed = 1.0f;    // [ property ]
  bool m_bApplyRootMotion = false;  // [ property ]

  // Inputs:
  bool m_bTriggerActive = false;
  float m_fPlaybackSpeedFactor = 1.0f;
  nsTime m_Duration;
  nsTime m_DurationOfQueued;

  bool WillStateBeOff(bool bTriggerActive) const;
  void UpdateState(nsTime diff);
  State GetCurrentState() const { return m_State; }
  float GetWeight() const { return m_fCurWeight; }
  float GetNormalizedPlaybackPosition() const { return m_fNormalizedPlaybackPosition; }
  bool HasTransitioned() const { return m_bHasTransitioned; }
  bool HasLoopedStart() const { return m_bHasLoopedStart; }
  bool HasLoopedEnd() const { return m_bHasLoopedEnd; }
  float GetFinalSpeed() const { return m_fPlaybackSpeed * m_fPlaybackSpeedFactor; }

  nsResult Serialize(nsStreamWriter& inout_stream) const;
  nsResult Deserialize(nsStreamReader& inout_stream);

private:
  void RampWeightUpOrDown(float& inout_fWeight, float fTargetWeight, nsTime tDiff) const;

  State m_State = State::Off;
  float m_fNormalizedPlaybackPosition = 0.0f;
  bool m_bRequireLoopForRampDown = true;
  bool m_bHasTransitioned = false;
  bool m_bHasLoopedStart = false;
  bool m_bHasLoopedEnd = false;
  float m_fCurWeight = 0.0f;
};

NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsAnimState);

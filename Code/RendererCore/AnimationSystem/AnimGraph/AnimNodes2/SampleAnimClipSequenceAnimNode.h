#pragma once

#include <Core/ResourceManager/ResourceHandle.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>
#include <RendererCore/AnimationSystem/AnimationClipResource.h>

class NS_RENDERERCORE_DLL nsSampleAnimClipSequenceAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsSampleAnimClipSequenceAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;
  virtual bool GetInstanceDataDesc(nsInstanceDataDesc& out_desc) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsSampleAnimClipSequenceAnimNode

public:
  nsSampleAnimClipSequenceAnimNode();
  ~nsSampleAnimClipSequenceAnimNode();

  void SetStartClip(const char* szClip);
  const char* GetStartClip() const;

  nsUInt32 Clips_GetCount() const;                            // [ property ]
  const char* Clips_GetValue(nsUInt32 uiIndex) const;         // [ property ]
  void Clips_SetValue(nsUInt32 uiIndex, const char* szValue); // [ property ]
  void Clips_Insert(nsUInt32 uiIndex, const char* szValue);   // [ property ]
  void Clips_Remove(nsUInt32 uiIndex);                        // [ property ]

  void SetEndClip(const char* szClip);
  const char* GetEndClip() const;

private:
  nsHashedString m_sStartClip;                      // [ property ]
  nsHybridArray<nsHashedString, 1> m_Clips;         // [ property ]
  nsHashedString m_sEndClip;                        // [ property ]
  bool m_bApplyRootMotion = false;                  // [ property ]
  bool m_bLoop = false;                             // [ property ]
  float m_fPlaybackSpeed = 1.0f;                    // [ property ]

  nsAnimGraphTriggerInputPin m_InStart;             // [ property ]
  nsAnimGraphBoolInputPin m_InLoop;                 // [ property ]
  nsAnimGraphNumberInputPin m_InSpeed;              // [ property ]
  nsAnimGraphNumberInputPin m_ClipIndexPin;         // [ property ]

  nsAnimGraphLocalPoseOutputPin m_OutPose;          // [ property ]
  nsAnimGraphTriggerOutputPin m_OutOnMiddleStarted; // [ property ]
  nsAnimGraphTriggerOutputPin m_OutOnEndStarted;    // [ property ]
  nsAnimGraphTriggerOutputPin m_OutOnFinished;      // [ property ]

  enum class State : nsUInt8
  {
    Off,
    Start,
    Middle,
    End,
    HoldStartFrame,
    HoldMiddleFrame,
    HoldEndFrame,
  };

  struct InstanceState
  {
    nsTime m_PlaybackTime = nsTime::MakeZero();
    State m_State = State::Start;
    nsUInt8 m_uiMiddleClipIdx = 0;
  };
};

#pragma once

#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>

class NS_RENDERERCORE_DLL nsPoseResultAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsPoseResultAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;
  virtual bool GetInstanceDataDesc(nsInstanceDataDesc& out_desc) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsPoseResultAnimNode

public:
  nsPoseResultAnimNode();
  ~nsPoseResultAnimNode();

private:
  nsTime m_FadeDuration = nsTime::MakeFromMilliseconds(200); // [ property ]

  nsAnimGraphLocalPoseInputPin m_InPose;                     // [ property ]
  nsAnimGraphNumberInputPin m_InTargetWeight;                // [ property ]
  nsAnimGraphNumberInputPin m_InFadeDuration;                // [ property ]
  nsAnimGraphBoneWeightsInputPin m_InWeights;                // [ property ]
  nsAnimGraphTriggerOutputPin m_OutOnFadedOut;               // [ property ]
  nsAnimGraphTriggerOutputPin m_OutOnFadedIn;                // [ property ]
  nsAnimGraphNumberOutputPin m_OutCurrentWeight;             // [ property ]

  struct InstanceData
  {
    float m_fStartWeight = 0.0f;
    float m_fEndWeight = 0.0f;
    nsTime m_PlayTime = nsTime::MakeZero();
    nsTime m_EndTime = nsTime::MakeZero();
  };
};

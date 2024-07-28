#pragma once

#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>

class NS_RENDERERCORE_DLL nsLerpPosesAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsLerpPosesAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsLerpPosesAnimNode

public:
  nsLerpPosesAnimNode();
  ~nsLerpPosesAnimNode();

  float m_fLerp = 0.5f;                                     // [ property ]

private:
  nsUInt8 m_uiPosesCount = 0;                               // [ property ]
  nsHybridArray<nsAnimGraphLocalPoseInputPin, 2> m_InPoses; // [ property ]
  nsAnimGraphNumberInputPin m_InLerp;                       // [ property ]
  nsAnimGraphLocalPoseOutputPin m_OutPose;                  // [ property ]
};

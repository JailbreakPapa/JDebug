#pragma once

#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>

class NS_RENDERERCORE_DLL nsSwitchPoseAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsSwitchPoseAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;
  virtual bool GetInstanceDataDesc(nsInstanceDataDesc& out_desc) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsSelectPoseAnimNode

private:
  nsTime m_TransitionDuration = nsTime::MakeFromMilliseconds(200); // [ property ]
  nsUInt8 m_uiPosesCount = 0;                                      // [ property ]
  nsHybridArray<nsAnimGraphLocalPoseInputPin, 4> m_InPoses;        // [ property ]
  nsAnimGraphNumberInputPin m_InIndex;                             // [ property ]
  nsAnimGraphLocalPoseOutputPin m_OutPose;                         // [ property ]

  struct InstanceData
  {
    nsTime m_TransitionTime;
    nsInt8 m_iTransitionFromIndex = -1;
    nsInt8 m_iTransitionToIndex = -1;
  };
};

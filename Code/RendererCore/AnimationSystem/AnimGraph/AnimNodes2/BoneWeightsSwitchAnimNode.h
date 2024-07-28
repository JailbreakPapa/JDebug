#pragma once

#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>

class NS_RENDERERCORE_DLL nsSwitchBoneWeightsAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsSwitchBoneWeightsAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsSwitchBoneWeightsAnimNode

private:
  nsAnimGraphNumberInputPin m_InIndex;                          // [ property ]
  nsUInt8 m_uiWeightsCount = 0;                                 // [ property ]
  nsHybridArray<nsAnimGraphBoneWeightsInputPin, 2> m_InWeights; // [ property ]
  nsAnimGraphBoneWeightsOutputPin m_OutWeights;                 // [ property ]
};

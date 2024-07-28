#pragma once

#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>

class NS_RENDERERCORE_DLL nsLogAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsLogAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  //////////////////////////////////////////////////////////////////////////
  // nsLogAnimNode

protected:
  nsString m_sText;                                        // [ property ]
  nsAnimGraphTriggerInputPin m_InActivate;                 // [ property ]
  nsUInt8 m_uiNumberCount = 1;                             // [ property ]
  nsHybridArray<nsAnimGraphNumberInputPin, 2> m_InNumbers; // [ property ]
};

class NS_RENDERERCORE_DLL nsLogInfoAnimNode : public nsLogAnimNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsLogInfoAnimNode, nsLogAnimNode);

  //////////////////////////////////////////////////////////////////////////
  // nsLogAnimNode

protected:
  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;
};

class NS_RENDERERCORE_DLL nsLogErrorAnimNode : public nsLogAnimNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsLogErrorAnimNode, nsLogAnimNode);

  //////////////////////////////////////////////////////////////////////////
  // nsLogAnimNode

protected:
  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;
};

#pragma once

#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>

class NS_RENDERERCORE_DLL nsRootRotationAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsRootRotationAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsRootRotationAnimNode

public:
  nsRootRotationAnimNode();
  ~nsRootRotationAnimNode();

private:
  nsAnimGraphNumberInputPin m_InRotateX; // [ property ]
  nsAnimGraphNumberInputPin m_InRotateY; // [ property ]
  nsAnimGraphNumberInputPin m_InRotateZ; // [ property ]
};

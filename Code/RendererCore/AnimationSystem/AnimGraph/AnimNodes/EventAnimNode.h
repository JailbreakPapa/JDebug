#pragma once

#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>

class NS_RENDERERCORE_DLL nsSendEventAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsSendEventAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsSendEventAnimNode

public:
  void SetEventName(const char* szSz) { m_sEventName.Assign(szSz); }
  const char* GetEventName() const { return m_sEventName.GetString(); }

private:
  nsHashedString m_sEventName;             // [ property ]
  nsAnimGraphTriggerInputPin m_InActivate; // [ property ]
};

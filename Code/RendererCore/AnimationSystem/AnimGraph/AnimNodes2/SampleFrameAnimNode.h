#pragma once

#include <Core/ResourceManager/ResourceHandle.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>
#include <RendererCore/AnimationSystem/AnimationClipResource.h>

class NS_RENDERERCORE_DLL nsSampleFrameAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsSampleFrameAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsSampleFrameAnimNode

public:
  void SetClip(const char* szClip);
  const char* GetClip() const;

  nsHashedString m_sClip;                                 // [ property ]
  float m_fNormalizedSamplePosition = 0.0f;               // [ property ]

private:
  nsAnimGraphNumberInputPin m_InNormalizedSamplePosition; // [ property ]
  nsAnimGraphNumberInputPin m_InAbsoluteSamplePosition;   // [ property ]
  nsAnimGraphLocalPoseOutputPin m_OutPose;                // [ property ]
};

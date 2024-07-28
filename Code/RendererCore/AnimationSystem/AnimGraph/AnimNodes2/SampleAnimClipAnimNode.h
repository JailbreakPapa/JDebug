#pragma once

#include <Core/ResourceManager/ResourceHandle.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>
#include <RendererCore/AnimationSystem/AnimationClipResource.h>

class NS_RENDERERCORE_DLL nsSampleAnimClipAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsSampleAnimClipAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;
  virtual bool GetInstanceDataDesc(nsInstanceDataDesc& out_desc) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsSampleAnimClipAnimNode

  void SetClip(const char* szClip);
  const char* GetClip() const;

public:
  nsSampleAnimClipAnimNode();
  ~nsSampleAnimClipAnimNode();

private:
  nsHashedString m_sClip;                      // [ property ]
  bool m_bLoop = true;                         // [ property ]
  bool m_bApplyRootMotion = false;             // [ property ]
  float m_fPlaybackSpeed = 1.0f;               // [ property ]

  nsAnimGraphTriggerInputPin m_InStart;        // [ property ]
  nsAnimGraphBoolInputPin m_InLoop;            // [ property ]
  nsAnimGraphNumberInputPin m_InSpeed;         // [ property ]

  nsAnimGraphLocalPoseOutputPin m_OutPose;     // [ property ]
  nsAnimGraphTriggerOutputPin m_OutOnStarted;  // [ property ]
  nsAnimGraphTriggerOutputPin m_OutOnFinished; // [ property ]

  struct InstanceState
  {
    nsTime m_PlaybackTime = nsTime::MakeZero();
  };
};

#pragma once

#include <Core/ResourceManager/ResourceHandle.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>
#include <RendererCore/AnimationSystem/AnimationClipResource.h>

struct NS_RENDERERCORE_DLL nsAnimationClip1D
{
  nsHashedString m_sClip;
  float m_fPosition = 0.0f;
  float m_fSpeed = 1.0f;

  void SetAnimationFile(const char* szFile);
  const char* GetAnimationFile() const;
};

NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsAnimationClip1D);

class NS_RENDERERCORE_DLL nsSampleBlendSpace1DAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsSampleBlendSpace1DAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;
  virtual bool GetInstanceDataDesc(nsInstanceDataDesc& out_desc) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsSampleBlendSpace1DAnimNode

public:
  nsSampleBlendSpace1DAnimNode();
  ~nsSampleBlendSpace1DAnimNode();

private:
  nsHybridArray<nsAnimationClip1D, 4> m_Clips; // [ property ]
  bool m_bLoop = true;                         // [ property ]
  bool m_bApplyRootMotion = false;             // [ property ]
  float m_fPlaybackSpeed = 1.0f;               // [ property ]

  nsAnimGraphTriggerInputPin m_InStart;        // [ property ]
  nsAnimGraphBoolInputPin m_InLoop;            // [ property ]
  nsAnimGraphNumberInputPin m_InSpeed;         // [ property ]
  nsAnimGraphNumberInputPin m_InLerp;          // [ property ]
  nsAnimGraphLocalPoseOutputPin m_OutPose;     // [ property ]
  nsAnimGraphTriggerOutputPin m_OutOnStarted;  // [ property ]
  nsAnimGraphTriggerOutputPin m_OutOnFinished; // [ property ]


  struct InstanceState
  {
    nsTime m_PlaybackTime = nsTime::MakeZero();
  };
};

#pragma once

#include <Core/ResourceManager/ResourceHandle.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimController.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>
#include <RendererCore/AnimationSystem/AnimationClipResource.h>

struct NS_RENDERERCORE_DLL nsAnimationClip2D
{
  nsHashedString m_sClip;
  nsVec2 m_vPosition;

  void SetAnimationFile(const char* szFile);
  const char* GetAnimationFile() const;
};

NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsAnimationClip2D);

class NS_RENDERERCORE_DLL nsSampleBlendSpace2DAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsSampleBlendSpace2DAnimNode, nsAnimGraphNode);

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
  nsSampleBlendSpace2DAnimNode();
  ~nsSampleBlendSpace2DAnimNode();

  void SetCenterClipFile(const char* szFile);
  const char* GetCenterClipFile() const;

private:
  nsHashedString m_sCenterClip;                               // [ property ]
  nsHybridArray<nsAnimationClip2D, 8> m_Clips;                // [ property ]
  nsTime m_InputResponse = nsTime::MakeFromMilliseconds(100); // [ property ]
  bool m_bLoop = true;                                        // [ property ]
  bool m_bApplyRootMotion = false;                            // [ property ]
  float m_fPlaybackSpeed = 1.0f;                              // [ property ]

  nsAnimGraphTriggerInputPin m_InStart;                       // [ property ]
  nsAnimGraphBoolInputPin m_InLoop;                           // [ property ]
  nsAnimGraphNumberInputPin m_InSpeed;                        // [ property ]
  nsAnimGraphNumberInputPin m_InCoordX;                       // [ property ]
  nsAnimGraphNumberInputPin m_InCoordY;                       // [ property ]
  nsAnimGraphLocalPoseOutputPin m_OutPose;                    // [ property ]
  nsAnimGraphTriggerOutputPin m_OutOnStarted;                 // [ property ]
  nsAnimGraphTriggerOutputPin m_OutOnFinished;                // [ property ]

  struct ClipToPlay
  {
    NS_DECLARE_POD_TYPE();

    nsUInt32 m_uiIndex;
    float m_fWeight = 1.0f;
    const nsAnimController::AnimClipInfo* m_pClipInfo = nullptr;
  };

  struct InstanceState
  {
    nsTime m_CenterPlaybackTime = nsTime::MakeZero();
    float m_fOtherPlaybackPosNorm = 0.0f;
    float m_fLastValueX = 0.0f;
    float m_fLastValueY = 0.0f;
  };

  void UpdateCenterClipPlaybackTime(const nsAnimController::AnimClipInfo& centerInfo, InstanceState* pState, nsAnimGraphInstance& ref_graph, nsTime tDiff, nsAnimPoseEventTrackSampleMode& out_eventSamplingCenter) const;
  void PlayClips(nsAnimController& ref_controller, const nsAnimController::AnimClipInfo& centerInfo, InstanceState* pState, nsAnimGraphInstance& ref_graph, nsTime tDiff, nsArrayPtr<ClipToPlay> clips, nsUInt32 uiMaxWeightClip) const;
  void ComputeClipsAndWeights(nsAnimController& ref_controller, const nsAnimController::AnimClipInfo& centerInfo, const nsVec2& p, nsDynamicArray<ClipToPlay>& out_Clips, nsUInt32& out_uiMaxWeightClip) const;
};

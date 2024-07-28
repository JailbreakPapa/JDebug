#pragma once

#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>

class nsSkeletonResource;
class nsStreamWriter;
class nsStreamReader;

class NS_RENDERERCORE_DLL nsBoneWeightsAnimNode : public nsAnimGraphNode
{
  NS_ADD_DYNAMIC_REFLECTION(nsBoneWeightsAnimNode, nsAnimGraphNode);

  //////////////////////////////////////////////////////////////////////////
  // nsAnimGraphNode

protected:
  virtual nsResult SerializeNode(nsStreamWriter& stream) const override;
  virtual nsResult DeserializeNode(nsStreamReader& stream) override;

  virtual void Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const override;
  virtual bool GetInstanceDataDesc(nsInstanceDataDesc& out_desc) const override;

  //////////////////////////////////////////////////////////////////////////
  // nsBoneWeightsAnimNode

public:
  nsBoneWeightsAnimNode();
  ~nsBoneWeightsAnimNode();

  float m_fWeight = 1.0f;                                       // [ property ]

  nsUInt32 RootBones_GetCount() const;                          // [ property ]
  const char* RootBones_GetValue(nsUInt32 uiIndex) const;       // [ property ]
  void RootBones_SetValue(nsUInt32 uiIndex, const char* value); // [ property ]
  void RootBones_Insert(nsUInt32 uiIndex, const char* value);   // [ property ]
  void RootBones_Remove(nsUInt32 uiIndex);                      // [ property ]

private:
  nsAnimGraphBoneWeightsOutputPin m_WeightsPin;                 // [ property ]
  nsAnimGraphBoneWeightsOutputPin m_InverseWeightsPin;          // [ property ]

  nsHybridArray<nsHashedString, 2> m_RootBones;

  struct InstanceData
  {
    nsSharedPtr<nsAnimGraphSharedBoneWeights> m_pSharedBoneWeights;
    nsSharedPtr<nsAnimGraphSharedBoneWeights> m_pSharedInverseBoneWeights;
  };
};

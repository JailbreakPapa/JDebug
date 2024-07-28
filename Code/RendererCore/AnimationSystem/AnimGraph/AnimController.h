#pragma once

#include <RendererCore/RendererCoreDLL.h>

#include <Core/Utils/Blackboard.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/HashTable.h>
#include <Foundation/Memory/AllocatorWrapper.h>
#include <Foundation/Types/Delegate.h>
#include <Foundation/Types/SharedPtr.h>
#include <Foundation/Types/UniquePtr.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>
#include <RendererCore/AnimationSystem/AnimPoseGenerator.h>

class nsGameObject;
class nsAnimGraph;

using nsAnimGraphResourceHandle = nsTypedResourceHandle<class nsAnimGraphResource>;
using nsSkeletonResourceHandle = nsTypedResourceHandle<class nsSkeletonResource>;

NS_DEFINE_AS_POD_TYPE(ozz::math::SimdFloat4);

struct nsAnimGraphPinDataBoneWeights
{
  nsUInt16 m_uiOwnIndex = 0xFFFF;
  float m_fOverallWeight = 1.0f;
  const nsAnimGraphSharedBoneWeights* m_pSharedBoneWeights = nullptr;
};

struct nsAnimGraphPinDataLocalTransforms
{
  nsUInt16 m_uiOwnIndex = 0xFFFF;
  nsAnimPoseGeneratorCommandID m_CommandID;
  const nsAnimGraphPinDataBoneWeights* m_pWeights = nullptr;
  float m_fOverallWeight = 1.0f;
  nsVec3 m_vRootMotion = nsVec3::MakeZero();
  bool m_bUseRootMotion = false;
};

struct nsAnimGraphPinDataModelTransforms
{
  nsUInt16 m_uiOwnIndex = 0xFFFF;
  nsAnimPoseGeneratorCommandID m_CommandID;
  nsVec3 m_vRootMotion = nsVec3::MakeZero();
  nsAngle m_RootRotationX;
  nsAngle m_RootRotationY;
  nsAngle m_RootRotationZ;
  bool m_bUseRootMotion = false;
};

class NS_RENDERERCORE_DLL nsAnimController
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsAnimController);

public:
  nsAnimController();
  ~nsAnimController();

  void Initialize(const nsSkeletonResourceHandle& hSkeleton, nsAnimPoseGenerator& ref_poseGenerator, const nsSharedPtr<nsBlackboard>& pBlackboard = nullptr);

  void Update(nsTime diff, nsGameObject* pTarget, bool bEnableIK);

  void GetRootMotion(nsVec3& ref_vTranslation, nsAngle& ref_rotationX, nsAngle& ref_rotationY, nsAngle& ref_rotationZ) const;

  const nsSharedPtr<nsBlackboard>& GetBlackboard() { return m_pBlackboard; }

  nsAnimPoseGenerator& GetPoseGenerator() { return *m_pPoseGenerator; }

  static nsSharedPtr<nsAnimGraphSharedBoneWeights> CreateBoneWeights(const char* szUniqueName, const nsSkeletonResource& skeleton, nsDelegate<void(nsAnimGraphSharedBoneWeights&)> fill);

  void SetOutputModelTransform(nsAnimGraphPinDataModelTransforms* pModelTransform);
  void SetRootMotion(const nsVec3& vTranslation, nsAngle rotationX, nsAngle rotationY, nsAngle rotationZ);

  void AddOutputLocalTransforms(nsAnimGraphPinDataLocalTransforms* pLocalTransforms);

  nsAnimGraphPinDataBoneWeights* AddPinDataBoneWeights();
  nsAnimGraphPinDataLocalTransforms* AddPinDataLocalTransforms();
  nsAnimGraphPinDataModelTransforms* AddPinDataModelTransforms();

  void AddAnimGraph(const nsAnimGraphResourceHandle& hGraph);
  // TODO void RemoveAnimGraph(const nsAnimGraphResource& hGraph);

  struct AnimClipInfo
  {
    nsAnimationClipResourceHandle m_hClip;
  };

  const AnimClipInfo& GetAnimationClipInfo(nsTempHashedString sClipName) const;

private:
  void GenerateLocalResultProcessors(const nsSkeletonResource* pSkeleton);

  nsSkeletonResourceHandle m_hSkeleton;
  nsAnimGraphPinDataModelTransforms* m_pCurrentModelTransforms = nullptr;

  nsVec3 m_vRootMotion = nsVec3::MakeZero();
  nsAngle m_RootRotationX;
  nsAngle m_RootRotationY;
  nsAngle m_RootRotationZ;

  nsDynamicArray<ozz::math::SimdFloat4, nsAlignedAllocatorWrapper> m_BlendMask;

  nsAnimPoseGenerator* m_pPoseGenerator = nullptr;
  nsSharedPtr<nsBlackboard> m_pBlackboard = nullptr;

  nsHybridArray<nsUInt32, 8> m_CurrentLocalTransformOutputs;

  static nsMutex s_SharedDataMutex;
  static nsHashTable<nsString, nsSharedPtr<nsAnimGraphSharedBoneWeights>> s_SharedBoneWeights;

  struct GraphInstance
  {
    nsAnimGraphResourceHandle m_hAnimGraph;
    nsUniquePtr<nsAnimGraphInstance> m_pInstance;
  };

  nsHybridArray<GraphInstance, 2> m_Instances;

  AnimClipInfo m_InvalidClipInfo;
  nsHashTable<nsHashedString, AnimClipInfo> m_AnimationClipMapping;

private:
  friend class nsAnimGraphTriggerOutputPin;
  friend class nsAnimGraphTriggerInputPin;
  friend class nsAnimGraphBoneWeightsInputPin;
  friend class nsAnimGraphBoneWeightsOutputPin;
  friend class nsAnimGraphLocalPoseInputPin;
  friend class nsAnimGraphLocalPoseOutputPin;
  friend class nsAnimGraphModelPoseInputPin;
  friend class nsAnimGraphModelPoseOutputPin;
  friend class nsAnimGraphLocalPoseMultiInputPin;
  friend class nsAnimGraphNumberInputPin;
  friend class nsAnimGraphNumberOutputPin;
  friend class nsAnimGraphBoolInputPin;
  friend class nsAnimGraphBoolOutputPin;

  nsHybridArray<nsAnimGraphPinDataBoneWeights, 4> m_PinDataBoneWeights;
  nsHybridArray<nsAnimGraphPinDataLocalTransforms, 4> m_PinDataLocalTransforms;
  nsHybridArray<nsAnimGraphPinDataModelTransforms, 2> m_PinDataModelTransforms;
};

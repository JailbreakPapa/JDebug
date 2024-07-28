#pragma once

#include <RendererCore/RendererCoreDLL.h>

#include <Foundation/Containers/Blob.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Memory/InstanceDataAllocator.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphNode.h>

class nsGameObject;
class nsAnimGraph;
class nsAnimController;

class NS_RENDERERCORE_DLL nsAnimGraphInstance
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsAnimGraphInstance);

public:
  nsAnimGraphInstance();
  ~nsAnimGraphInstance();

  void Configure(const nsAnimGraph& animGraph);

  void Update(nsAnimController& ref_controller, nsTime diff, nsGameObject* pTarget, const nsSkeletonResource* pSekeltonResource);

  template <typename T>
  T* GetAnimNodeInstanceData(const nsAnimGraphNode& node)
  {
    return reinterpret_cast<T*>(nsInstanceDataAllocator::GetInstanceData(m_InstanceData.GetByteBlobPtr(), node.m_uiInstanceDataOffset));
  }


private:
  const nsAnimGraph* m_pAnimGraph = nullptr;

  nsBlob m_InstanceData;

  // EXTEND THIS if a new type is introduced
  nsInt8* m_pTriggerInputPinStates = nullptr;
  double* m_pNumberInputPinStates = nullptr;
  bool* m_pBoolInputPinStates = nullptr;
  nsUInt16* m_pBoneWeightInputPinStates = nullptr;
  nsDynamicArray<nsHybridArray<nsUInt16, 1>> m_LocalPoseInputPinStates;
  nsUInt16* m_pModelPoseInputPinStates = nullptr;

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
};

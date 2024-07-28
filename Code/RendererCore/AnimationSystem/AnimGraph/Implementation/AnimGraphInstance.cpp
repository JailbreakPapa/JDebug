#include <RendererCore/RendererCorePCH.h>

#include <Core/World/GameObject.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphInstance.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphResource.h>
#include <RendererCore/AnimationSystem/SkeletonResource.h>
#include <ozz/animation/runtime/skeleton.h>

nsAnimGraphInstance::nsAnimGraphInstance() = default;

nsAnimGraphInstance::~nsAnimGraphInstance()
{
  if (m_pAnimGraph)
  {
    m_pAnimGraph->GetInstanceDataAlloator().DestructAndDeallocate(m_InstanceData);
  }
}

void nsAnimGraphInstance::Configure(const nsAnimGraph& animGraph)
{
  m_pAnimGraph = &animGraph;

  m_InstanceData = m_pAnimGraph->GetInstanceDataAlloator().AllocateAndConstruct();

  // EXTEND THIS if a new type is introduced
  m_pTriggerInputPinStates = (nsInt8*)nsInstanceDataAllocator::GetInstanceData(m_InstanceData.GetByteBlobPtr(), m_pAnimGraph->m_uiPinInstanceDataOffset[nsAnimGraphPin::Type::Trigger]);
  m_pNumberInputPinStates = (double*)nsInstanceDataAllocator::GetInstanceData(m_InstanceData.GetByteBlobPtr(), m_pAnimGraph->m_uiPinInstanceDataOffset[nsAnimGraphPin::Type::Number]);
  m_pBoolInputPinStates = (bool*)nsInstanceDataAllocator::GetInstanceData(m_InstanceData.GetByteBlobPtr(), m_pAnimGraph->m_uiPinInstanceDataOffset[nsAnimGraphPin::Type::Bool]);
  m_pBoneWeightInputPinStates = (nsUInt16*)nsInstanceDataAllocator::GetInstanceData(m_InstanceData.GetByteBlobPtr(), m_pAnimGraph->m_uiPinInstanceDataOffset[nsAnimGraphPin::Type::BoneWeights]);
  m_pModelPoseInputPinStates = (nsUInt16*)nsInstanceDataAllocator::GetInstanceData(m_InstanceData.GetByteBlobPtr(), m_pAnimGraph->m_uiPinInstanceDataOffset[nsAnimGraphPin::Type::ModelPose]);

  m_LocalPoseInputPinStates.SetCount(animGraph.m_uiInputPinCounts[nsAnimGraphPin::Type::LocalPose]);
}

void nsAnimGraphInstance::Update(nsAnimController& ref_controller, nsTime diff, nsGameObject* pTarget, const nsSkeletonResource* pSekeltonResource)
{
  // reset all pin states
  {
    // EXTEND THIS if a new type is introduced

    nsMemoryUtils::ZeroFill(m_pTriggerInputPinStates, m_pAnimGraph->m_uiInputPinCounts[nsAnimGraphPin::Type::Trigger]);
    nsMemoryUtils::ZeroFill(m_pNumberInputPinStates, m_pAnimGraph->m_uiInputPinCounts[nsAnimGraphPin::Type::Number]);
    nsMemoryUtils::ZeroFill(m_pBoolInputPinStates, m_pAnimGraph->m_uiInputPinCounts[nsAnimGraphPin::Type::Bool]);
    nsMemoryUtils::ZeroFill(m_pBoneWeightInputPinStates, m_pAnimGraph->m_uiInputPinCounts[nsAnimGraphPin::Type::BoneWeights]);
    nsMemoryUtils::PatternFill(m_pModelPoseInputPinStates, 0xFF, m_pAnimGraph->m_uiInputPinCounts[nsAnimGraphPin::Type::ModelPose]);

    for (auto& pins : m_LocalPoseInputPinStates)
    {
      pins.Clear();
    }
  }

  for (const auto& pNode : m_pAnimGraph->GetNodes())
  {
    pNode->Step(ref_controller, *this, diff, pSekeltonResource, pTarget);
  }
}

#include <RendererCore/RendererCorePCH.h>

#include <Core/World/GameObject.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimController.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphInstance.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphPins.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphResource.h>
#include <RendererCore/AnimationSystem/SkeletonResource.h>

#include <RendererCore/AnimationSystem/AnimPoseGenerator.h>
#include <ozz/animation/runtime/skeleton.h>

nsMutex nsAnimController::s_SharedDataMutex;
nsHashTable<nsString, nsSharedPtr<nsAnimGraphSharedBoneWeights>> nsAnimController::s_SharedBoneWeights;

nsAnimController::nsAnimController() = default;
nsAnimController::~nsAnimController() = default;

void nsAnimController::Initialize(const nsSkeletonResourceHandle& hSkeleton, nsAnimPoseGenerator& ref_poseGenerator, const nsSharedPtr<nsBlackboard>& pBlackboard /*= nullptr*/)
{
  m_hSkeleton = hSkeleton;
  m_pPoseGenerator = &ref_poseGenerator;
  m_pBlackboard = pBlackboard;
}

void nsAnimController::GetRootMotion(nsVec3& ref_vTranslation, nsAngle& ref_rotationX, nsAngle& ref_rotationY, nsAngle& ref_rotationZ) const
{
  ref_vTranslation = m_vRootMotion;
  ref_rotationX = m_RootRotationX;
  ref_rotationY = m_RootRotationY;
  ref_rotationZ = m_RootRotationZ;
}

void nsAnimController::Update(nsTime diff, nsGameObject* pTarget, bool bEnableIK)
{
  if (!m_hSkeleton.IsValid())
    return;

  nsResourceLock<nsSkeletonResource> pSkeleton(m_hSkeleton, nsResourceAcquireMode::BlockTillLoaded_NeverFail);
  if (pSkeleton.GetAcquireResult() != nsResourceAcquireResult::Final)
    return;

  m_pCurrentModelTransforms = nullptr;

  m_CurrentLocalTransformOutputs.Clear();

  m_vRootMotion = nsVec3::MakeZero();
  m_RootRotationX = {};
  m_RootRotationY = {};
  m_RootRotationZ = {};

  m_pPoseGenerator->Reset(pSkeleton.GetPointer(), pTarget);

  m_PinDataBoneWeights.Clear();
  m_PinDataLocalTransforms.Clear();
  m_PinDataModelTransforms.Clear();

  // TODO: step all instances

  for (auto& inst : m_Instances)
  {
    inst.m_pInstance->Update(*this, diff, pTarget, pSkeleton.GetPointer());
  }

  GenerateLocalResultProcessors(pSkeleton.GetPointer());

  {
    nsMsgAnimationPoseGeneration poseGenMsg;
    poseGenMsg.m_pGenerator = &GetPoseGenerator();
    pTarget->SendMessageRecursive(poseGenMsg);
  }

  GetPoseGenerator().UpdatePose(bEnableIK);

  if (auto newPose = GetPoseGenerator().GetCurrentPose(); !newPose.IsEmpty())
  {
    nsMsgAnimationPoseUpdated msg;
    msg.m_pSkeleton = &pSkeleton->GetDescriptor().m_Skeleton;
    msg.m_ModelTransforms = newPose;

    // TODO: root transform has to be applied first, only then can the world-space IK be done, and then the pose can be finalized
    msg.m_pRootTransform = &pSkeleton->GetDescriptor().m_RootTransform;

    // recursive, so that objects below the mesh can also listen in on these changes
    // for example bone attachments
    pTarget->SendMessageRecursive(msg);
  }
}

void nsAnimController::SetOutputModelTransform(nsAnimGraphPinDataModelTransforms* pModelTransform)
{
  m_pCurrentModelTransforms = pModelTransform;
}

void nsAnimController::SetRootMotion(const nsVec3& vTranslation, nsAngle rotationX, nsAngle rotationY, nsAngle rotationZ)
{
  m_vRootMotion = vTranslation;
  m_RootRotationX = rotationX;
  m_RootRotationY = rotationY;
  m_RootRotationZ = rotationZ;
}

void nsAnimController::AddOutputLocalTransforms(nsAnimGraphPinDataLocalTransforms* pLocalTransforms)
{
  m_CurrentLocalTransformOutputs.PushBack(pLocalTransforms->m_uiOwnIndex);
}

nsSharedPtr<nsAnimGraphSharedBoneWeights> nsAnimController::CreateBoneWeights(const char* szUniqueName, const nsSkeletonResource& skeleton, nsDelegate<void(nsAnimGraphSharedBoneWeights&)> fill)
{
  NS_LOCK(s_SharedDataMutex);

  nsSharedPtr<nsAnimGraphSharedBoneWeights>& bw = s_SharedBoneWeights[szUniqueName];

  if (bw == nullptr)
  {
    bw = NS_DEFAULT_NEW(nsAnimGraphSharedBoneWeights);
    bw->m_Weights.SetCountUninitialized(skeleton.GetDescriptor().m_Skeleton.GetOzzSkeleton().num_soa_joints());
    nsMemoryUtils::ZeroFill<ozz::math::SimdFloat4>(bw->m_Weights.GetData(), bw->m_Weights.GetCount());
  }

  fill(*bw);

  return bw;
}

void nsAnimController::GenerateLocalResultProcessors(const nsSkeletonResource* pSkeleton)
{
  if (m_CurrentLocalTransformOutputs.IsEmpty())
    return;

  nsAnimGraphPinDataLocalTransforms* pOut = &m_PinDataLocalTransforms[m_CurrentLocalTransformOutputs[0]];

  // combine multiple outputs
  if (m_CurrentLocalTransformOutputs.GetCount() > 1 || pOut->m_pWeights != nullptr)
  {
    const nsUInt32 m_uiMaxPoses = 6; // TODO

    pOut = AddPinDataLocalTransforms();
    pOut->m_vRootMotion.SetZero();

    float fSummedRootMotionWeight = 0.0f;

    // TODO: skip blending, if only a single animation is played
    // unless the weight is below 1.0 and the bind pose should be faded in

    auto& cmd = GetPoseGenerator().AllocCommandCombinePoses();

    struct PinWeight
    {
      nsUInt32 m_uiPinIdx;
      float m_fPinWeight = 0.0f;
    };

    nsHybridArray<PinWeight, 16> pw;
    pw.SetCount(m_CurrentLocalTransformOutputs.GetCount());

    for (nsUInt32 i = 0; i < m_CurrentLocalTransformOutputs.GetCount(); ++i)
    {
      pw[i].m_uiPinIdx = i;

      const nsAnimGraphPinDataLocalTransforms* pTransforms = &m_PinDataLocalTransforms[m_CurrentLocalTransformOutputs[i]];

      if (pTransforms != nullptr)
      {
        pw[i].m_fPinWeight = pTransforms->m_fOverallWeight;

        if (pTransforms->m_pWeights)
        {
          pw[i].m_fPinWeight *= pTransforms->m_pWeights->m_fOverallWeight;
        }
      }
    }

    if (pw.GetCount() > m_uiMaxPoses)
    {
      pw.Sort([](const PinWeight& lhs, const PinWeight& rhs)
        { return lhs.m_fPinWeight > rhs.m_fPinWeight; });
      pw.SetCount(m_uiMaxPoses);
    }

    nsArrayPtr<const ozz::math::SimdFloat4> invWeights;

    for (const auto& in : pw)
    {
      const nsAnimGraphPinDataLocalTransforms* pTransforms = &m_PinDataLocalTransforms[m_CurrentLocalTransformOutputs[in.m_uiPinIdx]];

      if (in.m_fPinWeight > 0 && pTransforms->m_pWeights)
      {
        // only initialize and use the inverse mask, when it is actually needed
        if (invWeights.IsEmpty())
        {
          m_BlendMask.SetCountUninitialized(pSkeleton->GetDescriptor().m_Skeleton.GetOzzSkeleton().num_soa_joints());

          for (auto& sj : m_BlendMask)
          {
            sj = ozz::math::simd_float4::one();
          }

          invWeights = m_BlendMask;
        }

        const ozz::math::SimdFloat4 factor = ozz::math::simd_float4::Load1(in.m_fPinWeight);

        const nsArrayPtr<const ozz::math::SimdFloat4> weights = pTransforms->m_pWeights->m_pSharedBoneWeights->m_Weights;

        for (nsUInt32 i = 0; i < m_BlendMask.GetCount(); ++i)
        {
          const auto& weight = weights[i];
          auto& mask = m_BlendMask[i];

          const auto oneMinusWeight = ozz::math::NMAdd(factor, weight, ozz::math::simd_float4::one());

          mask = ozz::math::Min(mask, oneMinusWeight);
        }
      }
    }

    for (const auto& in : pw)
    {
      if (in.m_fPinWeight > 0)
      {
        const nsAnimGraphPinDataLocalTransforms* pTransforms = &m_PinDataLocalTransforms[m_CurrentLocalTransformOutputs[in.m_uiPinIdx]];

        if (pTransforms->m_pWeights)
        {
          const nsArrayPtr<const ozz::math::SimdFloat4> weights = pTransforms->m_pWeights->m_pSharedBoneWeights->m_Weights;

          cmd.m_InputBoneWeights.PushBack(weights);
        }
        else
        {
          cmd.m_InputBoneWeights.PushBack(invWeights);
        }

        if (pTransforms->m_bUseRootMotion)
        {
          fSummedRootMotionWeight += in.m_fPinWeight;
          pOut->m_vRootMotion += pTransforms->m_vRootMotion * in.m_fPinWeight;

          // TODO: combining quaternions is mathematically tricky
          // could maybe use multiple slerps to concatenate weighted quaternions \_(ツ)_/

          pOut->m_bUseRootMotion = true;
        }

        cmd.m_Inputs.PushBack(pTransforms->m_CommandID);
        cmd.m_InputWeights.PushBack(in.m_fPinWeight);
      }
    }

    if (fSummedRootMotionWeight > 1.0f) // normalize down, but not up
    {
      pOut->m_vRootMotion /= fSummedRootMotionWeight;
    }

    pOut->m_CommandID = cmd.GetCommandID();
  }

  nsAnimGraphPinDataModelTransforms* pModelTransform = AddPinDataModelTransforms();

  // local space to model space
  {
    if (pOut->m_bUseRootMotion)
    {
      pModelTransform->m_bUseRootMotion = true;
      pModelTransform->m_vRootMotion = pOut->m_vRootMotion;
    }

    auto& cmd = GetPoseGenerator().AllocCommandLocalToModelPose();
    cmd.m_Inputs.PushBack(pOut->m_CommandID);

    pModelTransform->m_CommandID = cmd.GetCommandID();
  }

  // model space to output
  {
    nsVec3 rootMotion = nsVec3::MakeZero();
    nsAngle rootRotationX;
    nsAngle rootRotationY;
    nsAngle rootRotationZ;
    GetRootMotion(rootMotion, rootRotationX, rootRotationY, rootRotationZ);

    GetPoseGenerator().SetFinalCommand(pModelTransform->m_CommandID);

    if (pModelTransform->m_bUseRootMotion)
    {
      rootMotion += pModelTransform->m_vRootMotion;
      rootRotationX += pModelTransform->m_RootRotationX;
      rootRotationY += pModelTransform->m_RootRotationY;
      rootRotationZ += pModelTransform->m_RootRotationZ;
    }

    SetOutputModelTransform(pModelTransform);

    SetRootMotion(rootMotion, rootRotationX, rootRotationY, rootRotationZ);
  }
}

nsAnimGraphPinDataBoneWeights* nsAnimController::AddPinDataBoneWeights()
{
  nsAnimGraphPinDataBoneWeights* pData = &m_PinDataBoneWeights.ExpandAndGetRef();
  pData->m_uiOwnIndex = static_cast<nsUInt16>(m_PinDataBoneWeights.GetCount()) - 1;
  return pData;
}

nsAnimGraphPinDataLocalTransforms* nsAnimController::AddPinDataLocalTransforms()
{
  nsAnimGraphPinDataLocalTransforms* pData = &m_PinDataLocalTransforms.ExpandAndGetRef();
  pData->m_uiOwnIndex = static_cast<nsUInt16>(m_PinDataLocalTransforms.GetCount()) - 1;
  return pData;
}

nsAnimGraphPinDataModelTransforms* nsAnimController::AddPinDataModelTransforms()
{
  nsAnimGraphPinDataModelTransforms* pData = &m_PinDataModelTransforms.ExpandAndGetRef();
  pData->m_uiOwnIndex = static_cast<nsUInt16>(m_PinDataModelTransforms.GetCount()) - 1;
  return pData;
}

void nsAnimController::AddAnimGraph(const nsAnimGraphResourceHandle& hGraph)
{
  if (!hGraph.IsValid())
    return;

  for (auto& inst : m_Instances)
  {
    if (inst.m_hAnimGraph == hGraph)
      return;
  }

  nsResourceLock<nsAnimGraphResource> pAnimGraph(hGraph, nsResourceAcquireMode::BlockTillLoaded_NeverFail);
  if (pAnimGraph.GetAcquireResult() != nsResourceAcquireResult::Final)
    return;

  auto& inst = m_Instances.ExpandAndGetRef();
  inst.m_hAnimGraph = hGraph;
  inst.m_pInstance = NS_DEFAULT_NEW(nsAnimGraphInstance);
  inst.m_pInstance->Configure(pAnimGraph->GetAnimationGraph());

  for (auto& clip : pAnimGraph->GetAnimationClipMapping())
  {
    bool bExisted = false;
    auto& info = m_AnimationClipMapping.FindOrAdd(clip.m_sClipName, &bExisted);
    if (!bExisted)
    {
      info.m_hClip = clip.m_hClip;
    }
  }

  for (auto& ig : pAnimGraph->GetIncludeGraphs())
  {
    AddAnimGraph(nsResourceManager::LoadResource<nsAnimGraphResource>(ig));
  }
}

const nsAnimController::AnimClipInfo& nsAnimController::GetAnimationClipInfo(nsTempHashedString sClipName) const
{
  auto it = m_AnimationClipMapping.Find(sClipName);
  if (!it.IsValid())
    return m_InvalidClipInfo;

  return it.Value();
}

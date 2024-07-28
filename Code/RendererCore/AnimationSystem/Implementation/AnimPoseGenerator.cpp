#include <RendererCore/RendererCorePCH.h>

#include <Core/Messages/CommonMessages.h>
#include <Core/World/GameObject.h>
#include <RendererCore/AnimationSystem/AnimPoseGenerator.h>
#include <RendererCore/AnimationSystem/AnimationClipResource.h>
#include <RendererCore/AnimationSystem/Declarations.h>
#include <RendererCore/AnimationSystem/SkeletonResource.h>
#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/blending_job.h>
#include <ozz/animation/runtime/ik_aim_job.h>
#include <ozz/animation/runtime/ik_two_bone_job.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/base/maths/simd_quaternion.h>
#include <ozz/base/span.h>

void nsAnimPoseGenerator::Reset(const nsSkeletonResource* pSkeleton, nsGameObject* pTarget)
{
  m_pSkeleton = pSkeleton;
  m_pTargetGameObject = pTarget;
  m_LocalPoseCounter = 0;
  m_ModelPoseCounter = 0;
  m_FinalCommand = 0;

  m_CommandsSampleTrack.Clear();
  m_CommandsRestPose.Clear();
  m_CommandsCombinePoses.Clear();
  m_CommandsLocalToModelPose.Clear();
  m_CommandsSampleEventTrack.Clear();
  m_CommandsAimIK.Clear();

  m_UsedLocalTransforms.Clear();

  m_OutputPose.Clear();

  // don't clear these arrays, they are reused
  // m_UsedModelTransforms.Clear();
  // m_SamplingCaches.Clear();
}

static NS_ALWAYS_INLINE nsAnimPoseGeneratorCommandID CreateCommandID(nsAnimPoseGeneratorCommandType type, nsUInt32 uiIndex)
{
  return (static_cast<nsUInt32>(type) << 24u) | uiIndex;
}

static NS_ALWAYS_INLINE nsUInt32 GetCommandIndex(nsAnimPoseGeneratorCommandID id)
{
  return static_cast<nsUInt32>(id) & 0x00FFFFFFu;
}

static NS_ALWAYS_INLINE nsAnimPoseGeneratorCommandType GetCommandType(nsAnimPoseGeneratorCommandID id)
{
  return static_cast<nsAnimPoseGeneratorCommandType>(static_cast<nsUInt32>(id) >> 24u);
}

nsAnimPoseGeneratorCommandSampleTrack& nsAnimPoseGenerator::AllocCommandSampleTrack(nsUInt32 uiDeterministicID)
{
  auto& cmd = m_CommandsSampleTrack.ExpandAndGetRef();
  cmd.m_Type = nsAnimPoseGeneratorCommandType::SampleTrack;
  cmd.m_CommandID = CreateCommandID(cmd.m_Type, m_CommandsSampleTrack.GetCount() - 1);
  cmd.m_LocalPoseOutput = m_LocalPoseCounter++;
  cmd.m_uiUniqueID = uiDeterministicID;

  return cmd;
}

nsAnimPoseGeneratorCommandRestPose& nsAnimPoseGenerator::AllocCommandRestPose()
{
  auto& cmd = m_CommandsRestPose.ExpandAndGetRef();
  cmd.m_Type = nsAnimPoseGeneratorCommandType::RestPose;
  cmd.m_CommandID = CreateCommandID(cmd.m_Type, m_CommandsRestPose.GetCount() - 1);
  cmd.m_LocalPoseOutput = m_LocalPoseCounter++;

  return cmd;
}

nsAnimPoseGeneratorCommandCombinePoses& nsAnimPoseGenerator::AllocCommandCombinePoses()
{
  auto& cmd = m_CommandsCombinePoses.ExpandAndGetRef();
  cmd.m_Type = nsAnimPoseGeneratorCommandType::CombinePoses;
  cmd.m_CommandID = CreateCommandID(cmd.m_Type, m_CommandsCombinePoses.GetCount() - 1);
  cmd.m_LocalPoseOutput = m_LocalPoseCounter++;

  return cmd;
}

nsAnimPoseGeneratorCommandLocalToModelPose& nsAnimPoseGenerator::AllocCommandLocalToModelPose()
{
  auto& cmd = m_CommandsLocalToModelPose.ExpandAndGetRef();
  cmd.m_Type = nsAnimPoseGeneratorCommandType::LocalToModelPose;
  cmd.m_CommandID = CreateCommandID(cmd.m_Type, m_CommandsLocalToModelPose.GetCount() - 1);
  cmd.m_ModelPoseOutput = m_ModelPoseCounter++;

  return cmd;
}

nsAnimPoseGeneratorCommandSampleEventTrack& nsAnimPoseGenerator::AllocCommandSampleEventTrack()
{
  auto& cmd = m_CommandsSampleEventTrack.ExpandAndGetRef();
  cmd.m_Type = nsAnimPoseGeneratorCommandType::SampleEventTrack;
  cmd.m_CommandID = CreateCommandID(cmd.m_Type, m_CommandsSampleEventTrack.GetCount() - 1);

  return cmd;
}

nsAnimPoseGeneratorCommandAimIK& nsAnimPoseGenerator::AllocCommandAimIK()
{
  auto& cmd = m_CommandsAimIK.ExpandAndGetRef();
  cmd.m_Type = nsAnimPoseGeneratorCommandType::AimIK;
  cmd.m_CommandID = CreateCommandID(cmd.m_Type, m_CommandsAimIK.GetCount() - 1);

  return cmd;
}

nsAnimPoseGeneratorCommandTwoBoneIK& nsAnimPoseGenerator::AllocCommandTwoBoneIK()
{
  auto& cmd = m_CommandsTwoBoneIK.ExpandAndGetRef();
  cmd.m_Type = nsAnimPoseGeneratorCommandType::TwoBoneIK;
  cmd.m_CommandID = CreateCommandID(cmd.m_Type, m_CommandsTwoBoneIK.GetCount() - 1);

  return cmd;
}

nsAnimPoseGenerator::nsAnimPoseGenerator() = default;

nsAnimPoseGenerator::~nsAnimPoseGenerator()
{
  for (nsUInt32 i = 0; i < m_SamplingCaches.GetCount(); ++i)
  {
    NS_DEFAULT_DELETE(m_SamplingCaches.GetValue(i));
  }
  m_SamplingCaches.Clear();
}

void nsAnimPoseGenerator::Validate() const
{
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)

  for (auto& cmd : m_CommandsSampleTrack)
  {
    NS_ASSERT_DEV(cmd.m_hAnimationClip.IsValid(), "Invalid animation clips are not allowed.");
    // NS_ASSERT_DEV(cmd.m_Inputs.IsEmpty(), "Track samplers can't have inputs.");
    NS_ASSERT_DEV(cmd.m_LocalPoseOutput != nsInvalidIndex, "Output pose not allocated.");
  }

  for (auto& cmd : m_CommandsCombinePoses)
  {
    // NS_ASSERT_DEV(cmd.m_Inputs.GetCount() >= 1, "Must combine at least one pose.");
    NS_ASSERT_DEV(cmd.m_LocalPoseOutput != nsInvalidIndex, "Output pose not allocated.");
    NS_ASSERT_DEV(cmd.m_Inputs.GetCount() == cmd.m_InputWeights.GetCount(), "Number of inputs and weights must match.");

    for (auto id : cmd.m_Inputs)
    {
      auto type = GetCommand(id).GetType();
      NS_ASSERT_DEV(type == nsAnimPoseGeneratorCommandType::SampleTrack || type == nsAnimPoseGeneratorCommandType::CombinePoses || type == nsAnimPoseGeneratorCommandType::RestPose, "Unsupported input type");
    }
  }

  for (auto& cmd : m_CommandsLocalToModelPose)
  {
    NS_ASSERT_DEV(cmd.m_Inputs.GetCount() == 1, "Exactly one input must be provided.");
    NS_ASSERT_DEV(cmd.m_ModelPoseOutput != nsInvalidIndex, "Output pose not allocated.");

    for (auto id : cmd.m_Inputs)
    {
      auto type = GetCommand(id).GetType();
      NS_ASSERT_DEV(type == nsAnimPoseGeneratorCommandType::SampleTrack || type == nsAnimPoseGeneratorCommandType::CombinePoses || type == nsAnimPoseGeneratorCommandType::RestPose, "Unsupported input type");
    }
  }

  for (auto& cmd : m_CommandsAimIK)
  {
    NS_ASSERT_DEV(cmd.m_Inputs.GetCount() == 1, "Exactly one input must be provided.");

    for (auto id : cmd.m_Inputs)
    {
      auto type = GetCommand(id).GetType();
      NS_ASSERT_DEV(type == nsAnimPoseGeneratorCommandType::LocalToModelPose || type == nsAnimPoseGeneratorCommandType::AimIK || type == nsAnimPoseGeneratorCommandType::TwoBoneIK, "Unsupported input type");
    }
  }

  for (auto& cmd : m_CommandsTwoBoneIK)
  {
    NS_ASSERT_DEV(cmd.m_Inputs.GetCount() == 1, "Exactly one input must be provided.");

    for (auto id : cmd.m_Inputs)
    {
      auto type = GetCommand(id).GetType();
      NS_ASSERT_DEV(type == nsAnimPoseGeneratorCommandType::LocalToModelPose || type == nsAnimPoseGeneratorCommandType::AimIK || type == nsAnimPoseGeneratorCommandType::TwoBoneIK, "Unsupported input type");
    }
  }

  for (auto& cmd : m_CommandsSampleEventTrack)
  {
    NS_ASSERT_DEV(cmd.m_hAnimationClip.IsValid(), "Invalid animation clips are not allowed.");
  }

#endif
}

const nsAnimPoseGeneratorCommand& nsAnimPoseGenerator::GetCommand(nsAnimPoseGeneratorCommandID id) const
{
  return const_cast<nsAnimPoseGenerator*>(this)->GetCommand(id);
}

nsAnimPoseGeneratorCommand& nsAnimPoseGenerator::GetCommand(nsAnimPoseGeneratorCommandID id)
{
  NS_ASSERT_DEV(id != nsInvalidIndex, "Invalid command ID");

  switch (GetCommandType(id))
  {
    case nsAnimPoseGeneratorCommandType::SampleTrack:
      return m_CommandsSampleTrack[GetCommandIndex(id)];

    case nsAnimPoseGeneratorCommandType::RestPose:
      return m_CommandsRestPose[GetCommandIndex(id)];

    case nsAnimPoseGeneratorCommandType::CombinePoses:
      return m_CommandsCombinePoses[GetCommandIndex(id)];

    case nsAnimPoseGeneratorCommandType::LocalToModelPose:
      return m_CommandsLocalToModelPose[GetCommandIndex(id)];

    case nsAnimPoseGeneratorCommandType::SampleEventTrack:
      return m_CommandsSampleEventTrack[GetCommandIndex(id)];

    case nsAnimPoseGeneratorCommandType::AimIK:
      return m_CommandsAimIK[GetCommandIndex(id)];

    case nsAnimPoseGeneratorCommandType::TwoBoneIK:
      return m_CommandsTwoBoneIK[GetCommandIndex(id)];

      NS_DEFAULT_CASE_NOT_IMPLEMENTED;
  }

  NS_REPORT_FAILURE("Invalid command ID");
  return m_CommandsSampleTrack[0];
}

void nsAnimPoseGenerator::UpdatePose(bool bRequestExternalPoseGeneration)
{
  if (m_FinalCommand == 0)
    return;

  Validate();

  Execute(GetCommand(m_FinalCommand));

  if (bRequestExternalPoseGeneration && m_pTargetGameObject)
  {
    nsMsgAnimationPoseGeneration poseGenMsg;
    poseGenMsg.m_pGenerator = this;
    m_pTargetGameObject->SendMessageRecursive(poseGenMsg);

    // update the pose once again afterwards
    Execute(GetCommand(m_FinalCommand));
  }
}

void nsAnimPoseGenerator::Execute(nsAnimPoseGeneratorCommand& cmd)
{
  if (cmd.m_bExecuted)
    return;

  // TODO: validate for circular dependencies
  cmd.m_bExecuted = true;

  for (auto id : cmd.m_Inputs)
  {
    Execute(GetCommand(id));
  }

  // TODO: build a task graph and execute multi-threaded

  switch (cmd.GetType())
  {
    case nsAnimPoseGeneratorCommandType::SampleTrack:
      ExecuteCmd(static_cast<nsAnimPoseGeneratorCommandSampleTrack&>(cmd));
      break;

    case nsAnimPoseGeneratorCommandType::RestPose:
      ExecuteCmd(static_cast<nsAnimPoseGeneratorCommandRestPose&>(cmd));
      break;

    case nsAnimPoseGeneratorCommandType::CombinePoses:
      ExecuteCmd(static_cast<nsAnimPoseGeneratorCommandCombinePoses&>(cmd));
      break;

    case nsAnimPoseGeneratorCommandType::LocalToModelPose:
      ExecuteCmd(static_cast<nsAnimPoseGeneratorCommandLocalToModelPose&>(cmd));
      break;

    case nsAnimPoseGeneratorCommandType::SampleEventTrack:
      ExecuteCmd(static_cast<nsAnimPoseGeneratorCommandSampleEventTrack&>(cmd));
      break;

    case nsAnimPoseGeneratorCommandType::AimIK:
      ExecuteCmd(static_cast<nsAnimPoseGeneratorCommandAimIK&>(cmd));
      break;

    case nsAnimPoseGeneratorCommandType::TwoBoneIK:
      ExecuteCmd(static_cast<nsAnimPoseGeneratorCommandTwoBoneIK&>(cmd));
      break;

      NS_DEFAULT_CASE_NOT_IMPLEMENTED;
  }
}

void nsAnimPoseGenerator::ExecuteCmd(nsAnimPoseGeneratorCommandSampleTrack& cmd)
{
  nsResourceLock<nsAnimationClipResource> pResource(cmd.m_hAnimationClip, nsResourceAcquireMode::BlockTillLoaded);

  const ozz::animation::Animation& ozzAnim = pResource->GetDescriptor().GetMappedOzzAnimation(*m_pSkeleton);

  cmd.m_bAdditive = pResource->GetDescriptor().m_bAdditive;

  auto transforms = AcquireLocalPoseTransforms(cmd.m_LocalPoseOutput);

  auto& pSampler = m_SamplingCaches[cmd.m_uiUniqueID];

  if (pSampler == nullptr)
  {
    pSampler = NS_DEFAULT_NEW(ozz::animation::SamplingJob::Context);
  }

  if (pSampler->max_tracks() != ozzAnim.num_tracks())
  {
    pSampler->Resize(ozzAnim.num_tracks());
  }

  ozz::animation::SamplingJob job;
  job.animation = &ozzAnim;
  job.context = pSampler;
  job.ratio = cmd.m_fNormalizedSamplePos;
  job.output = ozz::span<ozz::math::SoaTransform>(transforms.GetPtr(), transforms.GetCount());

  if (!job.Validate())
    return;

  NS_ASSERT_DEBUG(job.Validate(), "");
  job.Run();

  SampleEventTrack(pResource.GetPointer(), cmd.m_EventSampling, cmd.m_fPreviousNormalizedSamplePos, cmd.m_fNormalizedSamplePos);
}

void nsAnimPoseGenerator::ExecuteCmd(nsAnimPoseGeneratorCommandRestPose& cmd)
{
  auto transforms = AcquireLocalPoseTransforms(cmd.m_LocalPoseOutput);

  const auto restPose = m_pSkeleton->GetDescriptor().m_Skeleton.GetOzzSkeleton().joint_rest_poses();

  transforms.CopyFrom(nsArrayPtr<const ozz::math::SoaTransform>(restPose.begin(), (nsUInt32)restPose.size()));
}

void nsAnimPoseGenerator::ExecuteCmd(nsAnimPoseGeneratorCommandCombinePoses& cmd)
{
  auto transforms = AcquireLocalPoseTransforms(cmd.m_LocalPoseOutput);

  nsHybridArray<ozz::animation::BlendingJob::Layer, 8> bl;
  nsHybridArray<ozz::animation::BlendingJob::Layer, 8> blAdd;

  for (nsUInt32 i = 0; i < cmd.m_Inputs.GetCount(); ++i)
  {
    const auto& cmdIn = GetCommand(cmd.m_Inputs[i]);

    if (cmdIn.GetType() == nsAnimPoseGeneratorCommandType::SampleEventTrack)
      continue;

    ozz::animation::BlendingJob::Layer* layer = nullptr;

    switch (cmdIn.GetType())
    {
      case nsAnimPoseGeneratorCommandType::SampleTrack:
      {
        if (static_cast<const nsAnimPoseGeneratorCommandSampleTrack&>(cmdIn).m_bAdditive)
        {
          layer = &blAdd.ExpandAndGetRef();
        }
        else
        {
          layer = &bl.ExpandAndGetRef();
        }

        auto transform = AcquireLocalPoseTransforms(static_cast<const nsAnimPoseGeneratorCommandSampleTrack&>(cmdIn).m_LocalPoseOutput);
        layer->transform = ozz::span<const ozz::math::SoaTransform>(transform.GetPtr(), transform.GetCount());
      }
      break;

      case nsAnimPoseGeneratorCommandType::RestPose:
      {
        layer = &bl.ExpandAndGetRef();

        auto transform = AcquireLocalPoseTransforms(static_cast<const nsAnimPoseGeneratorCommandRestPose&>(cmdIn).m_LocalPoseOutput);
        layer->transform = ozz::span<const ozz::math::SoaTransform>(transform.GetPtr(), transform.GetCount());
      }
      break;

      case nsAnimPoseGeneratorCommandType::CombinePoses:
      {
        layer = &bl.ExpandAndGetRef();

        auto transform = AcquireLocalPoseTransforms(static_cast<const nsAnimPoseGeneratorCommandCombinePoses&>(cmdIn).m_LocalPoseOutput);
        layer->transform = ozz::span<const ozz::math::SoaTransform>(transform.GetPtr(), transform.GetCount());
      }
      break;

        NS_DEFAULT_CASE_NOT_IMPLEMENTED;
    }

    layer->weight = cmd.m_InputWeights[i];

    if (cmd.m_InputBoneWeights.GetCount() > i && !cmd.m_InputBoneWeights[i].IsEmpty())
    {
      layer->joint_weights = ozz::span(cmd.m_InputBoneWeights[i].GetPtr(), cmd.m_InputBoneWeights[i].GetEndPtr());
    }
  }

  ozz::animation::BlendingJob job;
  job.threshold = 1.0f;
  job.layers = ozz::span<const ozz::animation::BlendingJob::Layer>(begin(bl), end(bl));
  job.additive_layers = ozz::span<const ozz::animation::BlendingJob::Layer>(begin(blAdd), end(blAdd));
  job.rest_pose = m_pSkeleton->GetDescriptor().m_Skeleton.GetOzzSkeleton().joint_rest_poses();
  job.output = ozz::span<ozz::math::SoaTransform>(transforms.GetPtr(), transforms.GetCount());
  NS_ASSERT_DEBUG(job.Validate(), "");
  job.Run();
}

void nsAnimPoseGenerator::ExecuteCmd(nsAnimPoseGeneratorCommandLocalToModelPose& cmd)
{
  ozz::animation::LocalToModelJob job;

  const auto& cmdIn = GetCommand(cmd.m_Inputs[0]);

  switch (cmdIn.GetType())
  {
    case nsAnimPoseGeneratorCommandType::SampleTrack:
    {
      cmd.m_LocalPoseOutput = static_cast<const nsAnimPoseGeneratorCommandSampleTrack&>(cmdIn).m_LocalPoseOutput;
    }
    break;

    case nsAnimPoseGeneratorCommandType::RestPose:
    {
      cmd.m_LocalPoseOutput = static_cast<const nsAnimPoseGeneratorCommandRestPose&>(cmdIn).m_LocalPoseOutput;
    }
    break;

    case nsAnimPoseGeneratorCommandType::CombinePoses:
    {
      cmd.m_LocalPoseOutput = static_cast<const nsAnimPoseGeneratorCommandCombinePoses&>(cmdIn).m_LocalPoseOutput;
    }
    break;

      NS_DEFAULT_CASE_NOT_IMPLEMENTED;
  }

  auto transform = AcquireLocalPoseTransforms(cmd.m_LocalPoseOutput);
  job.input = ozz::span<const ozz::math::SoaTransform>(transform.GetPtr(), transform.GetCount());

  if (cmd.m_pSendLocalPoseMsgTo || m_pTargetGameObject)
  {
    nsMsgAnimationPosePreparing msg;
    msg.m_pSkeleton = &m_pSkeleton->GetDescriptor().m_Skeleton;
    msg.m_LocalTransforms = nsMakeArrayPtr(const_cast<ozz::math::SoaTransform*>(job.input.data()), (nsUInt32)job.input.size());

    if (m_pTargetGameObject)
      m_pTargetGameObject->SendMessageRecursive(msg);
    else
      cmd.m_pSendLocalPoseMsgTo->SendMessageRecursive(msg);
  }

  m_OutputPose = AcquireModelPoseTransforms(cmd.m_ModelPoseOutput);

  job.output = ozz::span<ozz::math::Float4x4>(reinterpret_cast<ozz::math::Float4x4*>(m_OutputPose.GetPtr()), m_OutputPose.GetCount());
  job.skeleton = &m_pSkeleton->GetDescriptor().m_Skeleton.GetOzzSkeleton();
  NS_ASSERT_DEBUG(job.Validate(), "");
  job.Run();
}

void nsAnimPoseGenerator::ExecuteCmd(nsAnimPoseGeneratorCommandSampleEventTrack& cmd)
{
  nsResourceLock<nsAnimationClipResource> pResource(cmd.m_hAnimationClip, nsResourceAcquireMode::BlockTillLoaded);

  SampleEventTrack(pResource.GetPointer(), cmd.m_EventSampling, cmd.m_fPreviousNormalizedSamplePos, cmd.m_fNormalizedSamplePos);
}

void MultiplySoATransformQuaternion(nsUInt32 uiIndex, const ozz::math::SimdQuaternion& quat, nsArrayPtr<ozz::math::SoaTransform>& ref_transforms)
{
  NS_ASSERT_DEBUG(uiIndex < ref_transforms.GetCount() * 4, "Joint index out of bound.");

  // Convert SOA to AOS in order to perform quaternion multiplication, and get back to SOA.
  ozz::math::SoaTransform& soa_transform_ref = ref_transforms[uiIndex / 4];
  ozz::math::SimdQuaternion aos_quats[4];
  ozz::math::Transpose4x4(&soa_transform_ref.rotation.x, &aos_quats->xyzw);

  ozz::math::SimdQuaternion& aos_quat_ref = aos_quats[uiIndex & 3];
  aos_quat_ref = aos_quat_ref * quat;

  ozz::math::Transpose4x4(&aos_quats->xyzw, &soa_transform_ref.rotation.x);
}

void nsAnimPoseGenerator::ExecuteCmd(nsAnimPoseGeneratorCommandAimIK& cmd)
{
  const auto& cmdIn = GetCommand(cmd.m_Inputs[0]);

  switch (cmdIn.GetType())
  {
    case nsAnimPoseGeneratorCommandType::LocalToModelPose:
    {
      const nsAnimPoseGeneratorCommandLocalToModelPose& cmdIn2 = static_cast<const nsAnimPoseGeneratorCommandLocalToModelPose&>(cmdIn);
      cmd.m_LocalPoseOutput = cmdIn2.m_LocalPoseOutput;
      cmd.m_ModelPoseOutput = cmdIn2.m_ModelPoseOutput;
      m_OutputPose = AcquireModelPoseTransforms(cmd.m_ModelPoseOutput);
      break;
    }

    case nsAnimPoseGeneratorCommandType::AimIK:
    {
      const nsAnimPoseGeneratorCommandAimIK& cmdIn2 = static_cast<const nsAnimPoseGeneratorCommandAimIK&>(cmdIn);
      cmd.m_LocalPoseOutput = cmdIn2.m_LocalPoseOutput;
      cmd.m_ModelPoseOutput = cmdIn2.m_ModelPoseOutput;
      m_OutputPose = AcquireModelPoseTransforms(cmd.m_ModelPoseOutput);
      break;
    }

    case nsAnimPoseGeneratorCommandType::TwoBoneIK:
    {
      const nsAnimPoseGeneratorCommandTwoBoneIK& cmdIn2 = static_cast<const nsAnimPoseGeneratorCommandTwoBoneIK&>(cmdIn);
      cmd.m_LocalPoseOutput = cmdIn2.m_LocalPoseOutput;
      cmd.m_ModelPoseOutput = cmdIn2.m_ModelPoseOutput;
      m_OutputPose = AcquireModelPoseTransforms(cmd.m_ModelPoseOutput);
      break;
    }

      NS_DEFAULT_CASE_NOT_IMPLEMENTED;
  }

  NS_ASSERT_DEBUG(cmd.m_uiJointIdx < m_OutputPose.GetCount(), "Invalid joint index");

  auto transform = AcquireLocalPoseTransforms(cmd.m_LocalPoseOutput);

  const nsMat4* pJoint = &m_OutputPose[cmd.m_uiJointIdx];

  ozz::math::SimdQuaternion correction;
  bool bReached = false;

  // patch the local poses
  {
    ozz::animation::IKAimJob job;
    job.weight = cmd.m_fWeight;
    job.target = ozz::math::simd_float4::Load3PtrU(cmd.m_vTargetPosition.GetData());
    job.forward = ozz::math::simd_float4::Load3PtrU(cmd.m_vForwardVector.GetData());
    job.up = ozz::math::simd_float4::Load3PtrU(cmd.m_vUpVector.GetData());
    job.pole_vector = ozz::math::simd_float4::Load3PtrU(cmd.m_vPoleVector.GetData());
    job.joint_correction = &correction;
    job.joint = reinterpret_cast<const ozz::math::Float4x4*>(pJoint);
    job.reached = &bReached;
    NS_ASSERT_DEBUG(job.Validate(), "");
    job.Run();

    MultiplySoATransformQuaternion(cmd.m_uiJointIdx, correction, transform);
  }

  // rebuild the model poses
  {
    ozz::animation::LocalToModelJob job;
    job.from = (int)cmd.m_uiJointIdx;
    job.to = (int)cmd.m_uiRecalcModelPoseToJointIdx;
    job.input = ozz::span<const ozz::math::SoaTransform>(transform.GetPtr(), transform.GetCount());
    job.output = ozz::span<ozz::math::Float4x4>(reinterpret_cast<ozz::math::Float4x4*>(m_OutputPose.GetPtr()), m_OutputPose.GetCount());
    job.skeleton = &m_pSkeleton->GetDescriptor().m_Skeleton.GetOzzSkeleton();
    NS_ASSERT_DEBUG(job.Validate(), "");
    job.Run();
  }
}

void nsAnimPoseGenerator::ExecuteCmd(nsAnimPoseGeneratorCommandTwoBoneIK& cmd)
{
  const auto& cmdIn = GetCommand(cmd.m_Inputs[0]);

  switch (cmdIn.GetType())
  {
    case nsAnimPoseGeneratorCommandType::LocalToModelPose:
    {
      const nsAnimPoseGeneratorCommandLocalToModelPose& cmdIn2 = static_cast<const nsAnimPoseGeneratorCommandLocalToModelPose&>(cmdIn);
      cmd.m_LocalPoseOutput = cmdIn2.m_LocalPoseOutput;
      cmd.m_ModelPoseOutput = cmdIn2.m_ModelPoseOutput;
      m_OutputPose = AcquireModelPoseTransforms(cmd.m_ModelPoseOutput);
      break;
    }

    case nsAnimPoseGeneratorCommandType::AimIK:
    {
      const nsAnimPoseGeneratorCommandAimIK& cmdIn2 = static_cast<const nsAnimPoseGeneratorCommandAimIK&>(cmdIn);
      cmd.m_LocalPoseOutput = cmdIn2.m_LocalPoseOutput;
      cmd.m_ModelPoseOutput = cmdIn2.m_ModelPoseOutput;
      m_OutputPose = AcquireModelPoseTransforms(cmd.m_ModelPoseOutput);
      break;
    }

    case nsAnimPoseGeneratorCommandType::TwoBoneIK:
    {
      const nsAnimPoseGeneratorCommandTwoBoneIK& cmdIn2 = static_cast<const nsAnimPoseGeneratorCommandTwoBoneIK&>(cmdIn);
      cmd.m_LocalPoseOutput = cmdIn2.m_LocalPoseOutput;
      cmd.m_ModelPoseOutput = cmdIn2.m_ModelPoseOutput;
      m_OutputPose = AcquireModelPoseTransforms(cmd.m_ModelPoseOutput);
      break;
    }

      NS_DEFAULT_CASE_NOT_IMPLEMENTED;
  }

  auto transform = AcquireLocalPoseTransforms(cmd.m_LocalPoseOutput);

  NS_ASSERT_DEBUG(cmd.m_uiJointIdxStart < m_OutputPose.GetCount(), "Invalid joint index");
  NS_ASSERT_DEBUG(cmd.m_uiJointIdxMiddle < m_OutputPose.GetCount(), "Invalid joint index");
  NS_ASSERT_DEBUG(cmd.m_uiJointIdxEnd < m_OutputPose.GetCount(), "Invalid joint index");

  const nsMat4* pJointStart = &m_OutputPose[cmd.m_uiJointIdxStart];
  const nsMat4* pJointMiddle = &m_OutputPose[cmd.m_uiJointIdxMiddle];
  const nsMat4* pJointEnd = &m_OutputPose[cmd.m_uiJointIdxEnd];

  ozz::math::SimdQuaternion correctionStart, correctionMiddle;
  bool bReached = false;

  // patch the local poses
  {
    ozz::animation::IKTwoBoneJob job;
    job.reached = &bReached;
    job.weight = cmd.m_fWeight;
    job.target = ozz::math::simd_float4::Load3PtrU(cmd.m_vTargetPosition.GetData());
    job.start_joint = reinterpret_cast<const ozz::math::Float4x4*>(pJointStart);
    job.mid_joint = reinterpret_cast<const ozz::math::Float4x4*>(pJointMiddle);
    job.end_joint = reinterpret_cast<const ozz::math::Float4x4*>(pJointEnd);
    job.start_joint_correction = &correctionStart;
    job.mid_joint_correction = &correctionMiddle;
    job.mid_axis = ozz::math::simd_float4::Load3PtrU(cmd.m_vMidAxis.GetData());
    job.pole_vector = ozz::math::simd_float4::Load3PtrU(cmd.m_vPoleVector.GetData());
    job.soften = cmd.m_fSoften;
    job.twist_angle = cmd.m_TwistAngle.GetRadian();
    NS_ASSERT_DEBUG(job.Validate(), "");
    job.Run();

    MultiplySoATransformQuaternion(cmd.m_uiJointIdxStart, correctionStart, transform);
    MultiplySoATransformQuaternion(cmd.m_uiJointIdxMiddle, correctionMiddle, transform);
  }

  // rebuild the model poses
  {
    ozz::animation::LocalToModelJob job;
    job.from = (int)cmd.m_uiJointIdxStart;
    job.to = (int)cmd.m_uiRecalcModelPoseToJointIdx;
    job.input = ozz::span<const ozz::math::SoaTransform>(transform.GetPtr(), transform.GetCount());
    job.output = ozz::span<ozz::math::Float4x4>(reinterpret_cast<ozz::math::Float4x4*>(m_OutputPose.GetPtr()), m_OutputPose.GetCount());
    job.skeleton = &m_pSkeleton->GetDescriptor().m_Skeleton.GetOzzSkeleton();
    NS_ASSERT_DEBUG(job.Validate(), "");
    job.Run();
  }
}

void nsAnimPoseGenerator::SampleEventTrack(const nsAnimationClipResource* pResource, nsAnimPoseEventTrackSampleMode mode, float fPrevPos, float fCurPos)
{
  const auto& et = pResource->GetDescriptor().m_EventTrack;

  if (mode == nsAnimPoseEventTrackSampleMode::None || et.IsEmpty())
    return;

  const nsTime duration = pResource->GetDescriptor().GetDuration();

  const nsTime tPrev = fPrevPos * duration;
  const nsTime tNow = fCurPos * duration;
  const nsTime tStart = nsTime::MakeZero();
  const nsTime tEnd = duration + nsTime::MakeFromSeconds(1.0); // sampling position is EXCLUSIVE

  nsHybridArray<nsHashedString, 16> events;

  switch (mode)
  {
    case nsAnimPoseEventTrackSampleMode::OnlyBetween:
      et.Sample(tPrev, tNow, events);
      break;

    case nsAnimPoseEventTrackSampleMode::LoopAtEnd:
      et.Sample(tPrev, tEnd, events);
      et.Sample(tStart, tNow, events);
      break;

    case nsAnimPoseEventTrackSampleMode::LoopAtStart:
      et.Sample(tPrev, tStart, events);
      et.Sample(tStart, tNow, events);
      break;

    case nsAnimPoseEventTrackSampleMode::BounceAtEnd:
      et.Sample(tPrev, tEnd, events);
      et.Sample(tEnd, tNow, events);
      break;

    case nsAnimPoseEventTrackSampleMode::BounceAtStart:
      et.Sample(tPrev, tStart, events);
      et.Sample(tStart, tNow, events);
      break;

      NS_DEFAULT_CASE_NOT_IMPLEMENTED;
  }

  nsMsgGenericEvent msg;

  for (const auto& hs : events)
  {
    msg.m_sMessage = hs;

    m_pTargetGameObject->SendEventMessage(msg, nullptr);
  }
}

nsArrayPtr<ozz::math::SoaTransform> nsAnimPoseGenerator::AcquireLocalPoseTransforms(nsAnimPoseGeneratorLocalPoseID id)
{
  m_UsedLocalTransforms.EnsureCount(id + 1);

  if (m_UsedLocalTransforms[id].IsEmpty())
  {
    using T = ozz::math::SoaTransform;
    const nsUInt32 num = m_pSkeleton->GetDescriptor().m_Skeleton.GetOzzSkeleton().num_soa_joints();
    m_UsedLocalTransforms[id] = NS_NEW_ARRAY(nsFrameAllocator::GetCurrentAllocator(), T, num);
  }

  return m_UsedLocalTransforms[id];
}

nsArrayPtr<nsMat4> nsAnimPoseGenerator::AcquireModelPoseTransforms(nsAnimPoseGeneratorModelPoseID id)
{
  m_UsedModelTransforms.EnsureCount(id + 1);

  m_UsedModelTransforms[id].SetCountUninitialized(m_pSkeleton->GetDescriptor().m_Skeleton.GetOzzSkeleton().num_joints());

  return m_UsedModelTransforms[id];
}

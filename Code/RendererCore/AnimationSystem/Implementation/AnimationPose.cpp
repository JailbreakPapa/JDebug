#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/AnimationSystem/AnimationPose.h>
#include <RendererCore/AnimationSystem/Skeleton.h>
#include <RendererFoundation/Shader/Types.h>

// clang-format off
NS_IMPLEMENT_MESSAGE_TYPE(nsMsgAnimationPosePreparing);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgAnimationPosePreparing, 1, nsRTTIDefaultAllocator<nsMsgAnimationPosePreparing>)
{
  NS_BEGIN_ATTRIBUTES
  {
    new nsExcludeFromScript()
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_IMPLEMENT_MESSAGE_TYPE(nsMsgAnimationPoseGeneration);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgAnimationPoseGeneration, 1, nsRTTIDefaultAllocator<nsMsgAnimationPoseGeneration>)
{
  NS_BEGIN_ATTRIBUTES
  {
    new nsExcludeFromScript()
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_IMPLEMENT_MESSAGE_TYPE(nsMsgAnimationPoseUpdated);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgAnimationPoseUpdated, 1, nsRTTIDefaultAllocator<nsMsgAnimationPoseUpdated>)
{
  NS_BEGIN_ATTRIBUTES
  {
    new nsExcludeFromScript()
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_IMPLEMENT_MESSAGE_TYPE(nsMsgRopePoseUpdated);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgRopePoseUpdated, 1, nsRTTIDefaultAllocator<nsMsgRopePoseUpdated>)
{
  NS_BEGIN_ATTRIBUTES
  {
    new nsExcludeFromScript()
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_IMPLEMENT_MESSAGE_TYPE(nsMsgQueryAnimationSkeleton);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgQueryAnimationSkeleton, 1, nsRTTIDefaultAllocator<nsMsgQueryAnimationSkeleton>)
{
  NS_BEGIN_ATTRIBUTES
  {
    new nsExcludeFromScript()
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_IMPLEMENT_MESSAGE_TYPE(nsMsgApplyRootMotion);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgApplyRootMotion, 1, nsRTTIDefaultAllocator<nsMsgApplyRootMotion>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Translation", m_vTranslation),
    NS_MEMBER_PROPERTY("RotationX", m_RotationX),
    NS_MEMBER_PROPERTY("RotationY", m_RotationY),
    NS_MEMBER_PROPERTY("RotationZ", m_RotationZ),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_IMPLEMENT_MESSAGE_TYPE(nsMsgRetrieveBoneState);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgRetrieveBoneState, 1, nsRTTIDefaultAllocator<nsMsgRetrieveBoneState>)
{
  NS_BEGIN_ATTRIBUTES
  {
    new nsExcludeFromScript()
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_STATIC_REFLECTED_ENUM(nsAnimationInvisibleUpdateRate, 1)
  NS_ENUM_CONSTANT(nsAnimationInvisibleUpdateRate::FullUpdate),
  NS_ENUM_CONSTANT(nsAnimationInvisibleUpdateRate::Max60FPS),
  NS_ENUM_CONSTANT(nsAnimationInvisibleUpdateRate::Max30FPS),
  NS_ENUM_CONSTANT(nsAnimationInvisibleUpdateRate::Max15FPS),
  NS_ENUM_CONSTANT(nsAnimationInvisibleUpdateRate::Max10FPS),
  NS_ENUM_CONSTANT(nsAnimationInvisibleUpdateRate::Max5FPS),
  NS_ENUM_CONSTANT(nsAnimationInvisibleUpdateRate::Pause),
NS_END_STATIC_REFLECTED_ENUM;
// clang-format on

nsTime nsAnimationInvisibleUpdateRate::GetTimeStep(nsAnimationInvisibleUpdateRate::Enum value)
{
  switch (value)
  {
    case nsAnimationInvisibleUpdateRate::FullUpdate:
      return nsTime::MakeZero();
    case nsAnimationInvisibleUpdateRate::Max60FPS:
      return nsTime::MakeFromSeconds(1.0 / 60.0);
    case nsAnimationInvisibleUpdateRate::Max30FPS:
      return nsTime::MakeFromSeconds(1.0 / 30.0);
    case nsAnimationInvisibleUpdateRate::Max15FPS:
      return nsTime::MakeFromSeconds(1.0 / 15.0);
    case nsAnimationInvisibleUpdateRate::Max10FPS:
      return nsTime::MakeFromSeconds(1.0 / 10.0);

    case nsAnimationInvisibleUpdateRate::Max5FPS:
    case nsAnimationInvisibleUpdateRate::Pause: // full pausing should be handled separately, and if something isn't fully paused, it should behave like a very low update rate
      return nsTime::MakeFromSeconds(1.0 / 5.0);

      NS_DEFAULT_CASE_NOT_IMPLEMENTED;
  }

  return nsTime::MakeZero();
}

void nsMsgAnimationPoseUpdated::ComputeFullBoneTransform(nsUInt32 uiJointIndex, nsMat4& ref_mFullTransform) const
{
  ref_mFullTransform = m_pRootTransform->GetAsMat4() * m_ModelTransforms[uiJointIndex];
}

void nsMsgAnimationPoseUpdated::ComputeFullBoneTransform(const nsMat4& mRootTransform, const nsMat4& mModelTransform, nsMat4& ref_mFullTransform, nsQuat& ref_qRotationOnly)
{
  ref_mFullTransform = mRootTransform * mModelTransform;

  // the bone might contain (non-uniform) scaling and mirroring, which the quaternion can't represent
  // so reconstruct a representable rotation matrix
  ref_qRotationOnly.ReconstructFromMat4(ref_mFullTransform);
}

void nsMsgAnimationPoseUpdated::ComputeFullBoneTransform(nsUInt32 uiJointIndex, nsMat4& ref_mFullTransform, nsQuat& ref_qRotationOnly) const
{
  ComputeFullBoneTransform(m_pRootTransform->GetAsMat4(), m_ModelTransforms[uiJointIndex], ref_mFullTransform, ref_qRotationOnly);
}

NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_Implementation_AnimationPose);

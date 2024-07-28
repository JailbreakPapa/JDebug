#include <RendererCore/RendererCorePCH.h>

#include <Core/Physics/SurfaceResource.h>
#include <Core/ResourceManager/ResourceManager.h>
#include <RendererCore/AnimationSystem/SkeletonBuilder.h>

nsSkeletonBuilder::nsSkeletonBuilder() = default;
nsSkeletonBuilder::~nsSkeletonBuilder() = default;

nsUInt16 nsSkeletonBuilder::AddJoint(nsStringView sName, const nsTransform& localRestPose, nsUInt16 uiParentIndex /*= nsInvalidJointIndex*/)
{
  NS_ASSERT_DEV(uiParentIndex == nsInvalidJointIndex || uiParentIndex < m_Joints.GetCount(), "Invalid parent index for joint");

  auto& joint = m_Joints.ExpandAndGetRef();

  joint.m_RestPoseLocal = localRestPose;
  joint.m_RestPoseGlobal = localRestPose;
  joint.m_sName.Assign(sName);
  joint.m_uiParentIndex = uiParentIndex;

  if (uiParentIndex != nsInvalidJointIndex)
  {
    joint.m_RestPoseGlobal = m_Joints[joint.m_uiParentIndex].m_RestPoseGlobal * joint.m_RestPoseLocal;
  }

  joint.m_InverseRestPoseGlobal = joint.m_RestPoseGlobal.GetInverse();

  return static_cast<nsUInt16>(m_Joints.GetCount() - 1);
}

void nsSkeletonBuilder::SetJointLimit(nsUInt16 uiJointIndex, const nsQuat& qLocalOrientation, nsSkeletonJointType::Enum jointType, nsAngle halfSwingLimitY, nsAngle halfSwingLimitZ, nsAngle twistLimitHalfAngle, nsAngle twistLimitCenterAngle, float fStiffness)
{
  auto& j = m_Joints[uiJointIndex];
  j.m_qLocalJointOrientation = qLocalOrientation;
  j.m_JointType = jointType;
  j.m_HalfSwingLimitY = halfSwingLimitY;
  j.m_HalfSwingLimitZ = halfSwingLimitZ;
  j.m_TwistLimitHalfAngle = twistLimitHalfAngle;
  j.m_TwistLimitCenterAngle = twistLimitCenterAngle;
  j.m_fStiffness = fStiffness;
}


void nsSkeletonBuilder::SetJointSurface(nsUInt16 uiJointIndex, nsStringView sSurface)
{
  auto& j = m_Joints[uiJointIndex];
  j.m_sSurface = sSurface;
}

void nsSkeletonBuilder::SetJointCollisionLayer(nsUInt16 uiJointIndex, nsUInt8 uiCollsionLayer)
{
  auto& j = m_Joints[uiJointIndex];
  j.m_uiCollisionLayer = uiCollsionLayer;
}

void nsSkeletonBuilder::BuildSkeleton(nsSkeleton& ref_skeleton) const
{
  // NS_ASSERT_DEV(HasJoints(), "Can't build a skeleton with no joints!");

  const nsUInt32 numJoints = m_Joints.GetCount();

  // Copy joints to skeleton
  ref_skeleton.m_Joints.SetCount(numJoints);

  for (nsUInt32 i = 0; i < numJoints; ++i)
  {
    ref_skeleton.m_Joints[i].m_sName = m_Joints[i].m_sName;
    ref_skeleton.m_Joints[i].m_uiParentIndex = m_Joints[i].m_uiParentIndex;
    ref_skeleton.m_Joints[i].m_RestPoseLocal = m_Joints[i].m_RestPoseLocal;

    ref_skeleton.m_Joints[i].m_JointType = m_Joints[i].m_JointType;
    ref_skeleton.m_Joints[i].m_qLocalJointOrientation = m_Joints[i].m_qLocalJointOrientation;
    ref_skeleton.m_Joints[i].m_HalfSwingLimitY = m_Joints[i].m_HalfSwingLimitY;
    ref_skeleton.m_Joints[i].m_HalfSwingLimitZ = m_Joints[i].m_HalfSwingLimitZ;
    ref_skeleton.m_Joints[i].m_TwistLimitHalfAngle = m_Joints[i].m_TwistLimitHalfAngle;
    ref_skeleton.m_Joints[i].m_TwistLimitCenterAngle = m_Joints[i].m_TwistLimitCenterAngle;

    ref_skeleton.m_Joints[i].m_uiCollisionLayer = m_Joints[i].m_uiCollisionLayer;
    ref_skeleton.m_Joints[i].m_hSurface = nsResourceManager::LoadResource<nsSurfaceResource>(m_Joints[i].m_sSurface);
    ref_skeleton.m_Joints[i].m_fStiffness = m_Joints[i].m_fStiffness;
  }
}

bool nsSkeletonBuilder::HasJoints() const
{
  return !m_Joints.IsEmpty();
}

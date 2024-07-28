#pragma once

#include <Foundation/Types/UniquePtr.h>
#include <RendererCore/AnimationSystem/Skeleton.h>
#include <RendererCore/RendererCoreDLL.h>

/// \brief The skeleton builder class provides the means to build skeleton instances from scratch.
/// This class is not necessary to use skeletons, usually they should be deserialized from data created by the tools.
class NS_RENDERERCORE_DLL nsSkeletonBuilder
{

public:
  nsSkeletonBuilder();
  ~nsSkeletonBuilder();

  /// \brief Adds a joint to the skeleton
  /// Since the only way to add a joint with a parent is through this method the order of joints in the array is guaranteed
  /// so that child joints always come after their parent joints
  nsUInt16 AddJoint(nsStringView sName, const nsTransform& localRestPose, nsUInt16 uiParentIndex = nsInvalidJointIndex);

  void SetJointLimit(nsUInt16 uiJointIndex, const nsQuat& qLocalOrientation, nsSkeletonJointType::Enum jointType, nsAngle halfSwingLimitY, nsAngle halfSwingLimitZ, nsAngle twistLimitHalfAngle, nsAngle twistLimitCenterAngle, float fStiffness);

  void SetJointSurface(nsUInt16 uiJointIndex, nsStringView sSurface);
  void SetJointCollisionLayer(nsUInt16 uiJointIndex, nsUInt8 uiCollsionLayer);

  /// \brief Creates a skeleton from the accumulated data.
  void BuildSkeleton(nsSkeleton& ref_skeleton) const;

  /// \brief Returns true if there any joints have been added to the skeleton builder
  bool HasJoints() const;

protected:
  struct BuilderJoint
  {
    nsTransform m_RestPoseLocal;
    nsTransform m_RestPoseGlobal; // this one is temporary and not stored in the final nsSkeleton
    nsTransform m_InverseRestPoseGlobal;
    nsUInt16 m_uiParentIndex = nsInvalidJointIndex;
    nsHashedString m_sName;
    nsEnum<nsSkeletonJointType> m_JointType;
    nsQuat m_qLocalJointOrientation = nsQuat::MakeIdentity();
    nsAngle m_HalfSwingLimitZ;
    nsAngle m_HalfSwingLimitY;
    nsAngle m_TwistLimitHalfAngle;
    nsAngle m_TwistLimitCenterAngle;
    float m_fStiffness = 0.0f;

    nsString m_sSurface;
    nsUInt8 m_uiCollisionLayer = 0;
  };

  nsDeque<BuilderJoint> m_Joints;
};

#pragma once

#include <RendererCore/RendererCoreDLL.h>

#include <Foundation/Math/Mat3.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/UniquePtr.h>
#include <RendererCore/AnimationSystem/Declarations.h>

class nsStreamWriter;
class nsStreamReader;
class nsSkeletonBuilder;
class nsSkeleton;

using nsSurfaceResourceHandle = nsTypedResourceHandle<class nsSurfaceResource>;

namespace ozz::animation
{
  class Skeleton;
}

/// \brief Describes a single joint.
/// The transforms of the joints are in their local space and thus need to be correctly multiplied with their parent transforms to get the
/// final transform.
class NS_RENDERERCORE_DLL nsSkeletonJoint
{
public:
  const nsTransform& GetRestPoseLocalTransform() const { return m_RestPoseLocal; }

  /// \brief Returns nsInvalidJointIndex if no parent
  nsUInt16 GetParentIndex() const { return m_uiParentIndex; }

  bool IsRootJoint() const { return m_uiParentIndex == nsInvalidJointIndex; }
  const nsHashedString& GetName() const { return m_sName; }

  nsAngle GetHalfSwingLimitY() const { return m_HalfSwingLimitY; }
  nsAngle GetHalfSwingLimitZ() const { return m_HalfSwingLimitZ; }
  nsAngle GetTwistLimitHalfAngle() const { return m_TwistLimitHalfAngle; }
  nsAngle GetTwistLimitCenterAngle() const { return m_TwistLimitCenterAngle; }
  nsAngle GetTwistLimitLow() const;
  nsAngle GetTwistLimitHigh() const;
  nsEnum<nsSkeletonJointType> GetJointType() const { return m_JointType; }

  nsQuat GetLocalOrientation() const { return m_qLocalJointOrientation; }

  nsSurfaceResourceHandle GetSurface() const { return m_hSurface; }
  nsUInt8 GetCollisionLayer() const { return m_uiCollisionLayer; }

  float GetStiffness() const { return m_fStiffness; }
  void SetStiffness(float fValue) { m_fStiffness = fValue; }

private:
  friend nsSkeleton;
  friend nsSkeletonBuilder;

  nsTransform m_RestPoseLocal;
  nsUInt16 m_uiParentIndex = nsInvalidJointIndex;
  nsHashedString m_sName;

  nsSurfaceResourceHandle m_hSurface;
  nsUInt8 m_uiCollisionLayer = 0;

  nsEnum<nsSkeletonJointType> m_JointType;
  nsQuat m_qLocalJointOrientation = nsQuat::MakeIdentity();
  nsAngle m_HalfSwingLimitY;
  nsAngle m_HalfSwingLimitZ;
  nsAngle m_TwistLimitHalfAngle;
  nsAngle m_TwistLimitCenterAngle;
  float m_fStiffness = 0.0f;
};

/// \brief The skeleton class encapsulates the information about the joint structure for a model.
class NS_RENDERERCORE_DLL nsSkeleton
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsSkeleton);

public:
  nsSkeleton();
  nsSkeleton(nsSkeleton&& rhs);
  ~nsSkeleton();

  void operator=(nsSkeleton&& rhs);

  /// \brief Returns the number of joints in the skeleton.
  nsUInt16 GetJointCount() const { return static_cast<nsUInt16>(m_Joints.GetCount()); }

  /// \brief Returns the nth joint.
  const nsSkeletonJoint& GetJointByIndex(nsUInt16 uiIndex) const { return m_Joints[uiIndex]; }

  /// \brief Allows to find a specific joint in the skeleton by name. Returns nsInvalidJointIndex if not found
  nsUInt16 FindJointByName(const nsTempHashedString& sName) const;

  /// \brief Checks if two skeletons are compatible (same joint count and hierarchy)
  // bool IsCompatibleWith(const nsSkeleton& other) const;

  /// \brief Saves the skeleton in a given stream.
  void Save(nsStreamWriter& inout_stream) const;

  /// \brief Loads the skeleton from the given stream.
  void Load(nsStreamReader& inout_stream);

  bool IsJointDescendantOf(nsUInt16 uiJoint, nsUInt16 uiExpectedParent) const;

  const ozz::animation::Skeleton& GetOzzSkeleton() const;

  nsUInt64 GetHeapMemoryUsage() const;

  /// \brief The direction in which the bones shall point for visualization
  nsEnum<nsBasisAxis> m_BoneDirection;

protected:
  friend nsSkeletonBuilder;

  nsDynamicArray<nsSkeletonJoint> m_Joints;
  mutable nsUniquePtr<ozz::animation::Skeleton> m_pOzzSkeleton;
};

#pragma once

#include <RendererCore/RendererCoreDLL.h>

#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/VariantTypeRegistry.h>
#include <RendererCore/AnimationSystem/Declarations.h>

class nsSkeletonBuilder;
class nsSkeleton;

namespace ozz::animation
{
  class Skeleton;

  namespace offline
  {
    struct RawSkeleton;
  }
} // namespace ozz::animation

NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsSkeletonJointGeometryType);

struct NS_RENDERERCORE_DLL nsEditableSkeletonBoneShape : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsEditableSkeletonBoneShape, nsReflectedClass);

  nsEnum<nsSkeletonJointGeometryType> m_Geometry;

  nsVec3 m_vOffset = nsVec3::MakeZero();
  nsQuat m_qRotation = nsQuat::MakeIdentity();

  float m_fLength = 0;    // Box, Capsule; 0 means parent joint to this joint (auto mode)
  float m_fWidth = 0;     // Box
  float m_fThickness = 0; // Sphere radius, Capsule radius
};

struct NS_RENDERERCORE_DLL nsEditableSkeletonBoneCollider : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsEditableSkeletonBoneCollider, nsReflectedClass);

  nsString m_sIdentifier;
  nsDynamicArray<nsVec3> m_VertexPositions;
  nsDynamicArray<nsUInt8> m_TriangleIndices;
};

class NS_RENDERERCORE_DLL nsEditableSkeletonJoint : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsEditableSkeletonJoint, nsReflectedClass);

public:
  nsEditableSkeletonJoint();
  ~nsEditableSkeletonJoint();

  const char* GetName() const;
  void SetName(const char* szSz);

  void ClearJoints();

  // copies the properties for geometry etc. from another joint
  // does NOT copy the name, the transform or the children
  void CopyPropertiesFrom(const nsEditableSkeletonJoint* pJoint);

  nsHashedString m_sName;
  nsTransform m_LocalTransform = nsTransform::MakeIdentity();

  nsEnum<nsSkeletonJointType> m_JointType;

  float m_fStiffness = 0.0f;

  nsAngle m_TwistLimitHalfAngle;
  nsAngle m_TwistLimitCenterAngle;
  nsAngle m_SwingLimitY;
  nsAngle m_SwingLimitZ;

  nsVec3 m_vGizmoOffsetPositionRO = nsVec3::MakeZero();
  nsQuat m_qGizmoOffsetRotationRO = nsQuat::MakeIdentity();

  nsQuat m_qLocalJointRotation = nsQuat::MakeIdentity();

  nsHybridArray<nsEditableSkeletonJoint*, 4> m_Children;
  nsHybridArray<nsEditableSkeletonBoneShape, 1> m_BoneShapes;
  nsDynamicArray<nsEditableSkeletonBoneCollider> m_BoneColliders;

  bool m_bOverrideSurface = false;
  bool m_bOverrideCollisionLayer = false;
  nsString m_sSurfaceOverride;
  nsUInt8 m_uiCollisionLayerOverride;
};

class NS_RENDERERCORE_DLL nsEditableSkeleton : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsEditableSkeleton, nsReflectedClass);

public:
  nsEditableSkeleton();
  ~nsEditableSkeleton();

  void ClearJoints();
  void FillResourceDescriptor(nsSkeletonResourceDescriptor& ref_desc) const;
  void GenerateRawOzzSkeleton(ozz::animation::offline::RawSkeleton& out_skeleton) const;
  void GenerateOzzSkeleton(ozz::animation::Skeleton& out_skeleton) const;
  void CreateJointsRecursive(nsSkeletonBuilder& ref_sb, nsSkeletonResourceDescriptor& ref_desc, const nsEditableSkeletonJoint* pParentJoint, const nsEditableSkeletonJoint* pThisJoint, nsUInt16 uiThisJointIdx, const nsQuat& qParentAccuRot, const nsMat4& mRootTransform) const;

  nsString m_sSourceFile;
  nsString m_sPreviewMesh;

  nsString m_sSurfaceFile;
  nsUInt8 m_uiCollisionLayer = 0;

  float m_fUniformScaling = 1.0f;
  float m_fMaxImpulse = 100.0f;

  nsEnum<nsBasisAxis> m_RightDir;
  nsEnum<nsBasisAxis> m_UpDir;
  bool m_bFlipForwardDir = false;
  nsEnum<nsBasisAxis> m_BoneDirection;

  nsHybridArray<nsEditableSkeletonJoint*, 4> m_Children;
};

struct NS_RENDERERCORE_DLL nsExposedBone
{
  nsString m_sName;
  nsString m_sParent;
  nsTransform m_Transform;
  // when adding new values, the hash function below has to be adjusted
};

NS_DECLARE_CUSTOM_VARIANT_TYPE(nsExposedBone);

NS_RENDERERCORE_DLL void operator<<(nsStreamWriter& inout_stream, const nsExposedBone& bone);
NS_RENDERERCORE_DLL void operator>>(nsStreamReader& inout_stream, nsExposedBone& ref_bone);
NS_RENDERERCORE_DLL bool operator==(const nsExposedBone& lhs, const nsExposedBone& rhs);

template <>
struct nsHashHelper<nsExposedBone>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(const nsExposedBone& value)
  {
    return nsHashingUtils::xxHash32String(value.m_sName) + nsHashingUtils::xxHash32String(value.m_sParent) + nsHashingUtils::xxHash32(&value, sizeof(nsTransform));
  }

  NS_ALWAYS_INLINE static bool Equal(const nsExposedBone& a, const nsExposedBone& b) { return a == b; }
};

NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsExposedBone);

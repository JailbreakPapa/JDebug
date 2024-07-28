#include <RendererCore/RendererCorePCH.h>

#include <Core/Physics/SurfaceResource.h>
#include <Foundation/IO/Stream.h>
#include <Foundation/Types/VariantTypeRegistry.h>
#include <RendererCore/AnimationSystem/EditableSkeleton.h>
#include <RendererCore/AnimationSystem/Implementation/OzzUtils.h>
#include <RendererCore/AnimationSystem/SkeletonBuilder.h>
#include <RendererCore/AnimationSystem/SkeletonResource.h>
#include <ozz/animation/offline/raw_skeleton.h>
#include <ozz/animation/offline/skeleton_builder.h>
#include <ozz/animation/runtime/skeleton.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_ENUM(nsSkeletonJointGeometryType, 1)
NS_ENUM_CONSTANTS(nsSkeletonJointGeometryType::None, nsSkeletonJointGeometryType::Capsule, nsSkeletonJointGeometryType::Sphere, nsSkeletonJointGeometryType::Box)
NS_END_STATIC_REFLECTED_ENUM;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsEditableSkeletonBoneShape, 1, nsRTTIDefaultAllocator<nsEditableSkeletonBoneShape>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ENUM_MEMBER_PROPERTY("Geometry", nsSkeletonJointGeometryType, m_Geometry),
    NS_MEMBER_PROPERTY("Offset", m_vOffset),
    NS_MEMBER_PROPERTY("Rotation", m_qRotation),
    NS_MEMBER_PROPERTY("Length", m_fLength)->AddAttributes(new nsDefaultValueAttribute(0.1f), new nsClampValueAttribute(0.01f, 10.0f)),
    NS_MEMBER_PROPERTY("Width", m_fWidth)->AddAttributes(new nsDefaultValueAttribute(0.05f), new nsClampValueAttribute(0.01f, 10.0f)),
    NS_MEMBER_PROPERTY("Thickness", m_fThickness)->AddAttributes(new nsDefaultValueAttribute(0.05f), new nsClampValueAttribute(0.01f, 10.0f)),

  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsEditableSkeletonBoneCollider, 1, nsRTTIDefaultAllocator<nsEditableSkeletonBoneCollider>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Identifier", m_sIdentifier)->AddAttributes(new nsHiddenAttribute()),
    NS_ARRAY_MEMBER_PROPERTY("VertexPositions", m_VertexPositions)->AddAttributes(new nsHiddenAttribute()),
    NS_ARRAY_MEMBER_PROPERTY("TriangleIndices", m_TriangleIndices)->AddAttributes(new nsHiddenAttribute()),

  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsEditableSkeletonJoint, 2, nsRTTIDefaultAllocator<nsEditableSkeletonJoint>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Name", GetName, SetName)->AddAttributes(new nsReadOnlyAttribute()),
    NS_MEMBER_PROPERTY("Transform", m_LocalTransform)->AddFlags(nsPropertyFlags::Hidden)->AddAttributes(new nsDefaultValueAttribute(nsTransform::MakeIdentity())),
    NS_MEMBER_PROPERTY_READ_ONLY("GizmoOffsetTranslationRO", m_vGizmoOffsetPositionRO)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY_READ_ONLY("GizmoOffsetRotationRO", m_qGizmoOffsetRotationRO)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("LocalRotation", m_qLocalJointRotation),
    NS_ENUM_MEMBER_PROPERTY("JointType", nsSkeletonJointType, m_JointType),
    NS_MEMBER_PROPERTY("Stiffness", m_fStiffness)->AddAttributes(new nsDefaultValueAttribute(10.0f)),
    NS_MEMBER_PROPERTY("SwingLimitY", m_SwingLimitY)->AddAttributes(new nsClampValueAttribute(nsAngle(), nsAngle::MakeFromDegree(170)), new nsDefaultValueAttribute(nsAngle::MakeFromDegree(30))),
    NS_MEMBER_PROPERTY("SwingLimitZ", m_SwingLimitZ)->AddAttributes(new nsClampValueAttribute(nsAngle(), nsAngle::MakeFromDegree(170)), new nsDefaultValueAttribute(nsAngle::MakeFromDegree(30))),
    NS_MEMBER_PROPERTY("TwistLimitHalfAngle", m_TwistLimitHalfAngle)->AddAttributes(new nsClampValueAttribute(nsAngle::MakeFromDegree(10), nsAngle::MakeFromDegree(170)), new nsDefaultValueAttribute(nsAngle::MakeFromDegree(30))),
    NS_MEMBER_PROPERTY("TwistLimitCenterAngle", m_TwistLimitCenterAngle)->AddAttributes(new nsClampValueAttribute(-nsAngle::MakeFromDegree(170), nsAngle::MakeFromDegree(170))),

    NS_MEMBER_PROPERTY("OverrideSurface", m_bOverrideSurface),
    NS_MEMBER_PROPERTY("Surface", m_sSurfaceOverride)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Surface", nsDependencyFlags::Package)),
    NS_MEMBER_PROPERTY("OverrideCollisionLayer", m_bOverrideCollisionLayer),
    NS_MEMBER_PROPERTY("CollisionLayer", m_uiCollisionLayerOverride)->AddAttributes(new nsDynamicEnumAttribute("PhysicsCollisionLayer")),

    NS_ARRAY_MEMBER_PROPERTY("Children", m_Children)->AddFlags(nsPropertyFlags::PointerOwner | nsPropertyFlags::Hidden),
    NS_ARRAY_MEMBER_PROPERTY("BoneShapes", m_BoneShapes),
    NS_ARRAY_MEMBER_PROPERTY("Colliders", m_BoneColliders)->AddAttributes(new nsContainerAttribute(false, false, false)),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsTransformManipulatorAttribute(nullptr, "LocalRotation", nullptr, "GizmoOffsetTranslationRO", "GizmoOffsetRotationRO"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsEditableSkeleton, 1, nsRTTIDefaultAllocator<nsEditableSkeleton>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("File", m_sSourceFile)->AddAttributes(new nsFileBrowserAttribute("Select Mesh", nsFileBrowserAttribute::MeshesWithAnimations)),
    NS_ENUM_MEMBER_PROPERTY("RightDir", nsBasisAxis, m_RightDir)->AddAttributes(new nsDefaultValueAttribute((int)nsBasisAxis::PositiveX)),
    NS_ENUM_MEMBER_PROPERTY("UpDir", nsBasisAxis, m_UpDir)->AddAttributes(new nsDefaultValueAttribute((int)nsBasisAxis::PositiveY)),
    NS_MEMBER_PROPERTY("FlipForwardDir", m_bFlipForwardDir),
    NS_MEMBER_PROPERTY("UniformScaling", m_fUniformScaling)->AddAttributes(new nsDefaultValueAttribute(1.0f), new nsClampValueAttribute(0.0001f, 10000.0f)),
    NS_ENUM_MEMBER_PROPERTY("BoneDirection", nsBasisAxis, m_BoneDirection)->AddAttributes(new nsDefaultValueAttribute((int)nsBasisAxis::PositiveY)),
    NS_MEMBER_PROPERTY("PreviewMesh", m_sPreviewMesh)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Mesh_Skinned", nsDependencyFlags::None)),
    NS_MEMBER_PROPERTY("CollisionLayer", m_uiCollisionLayer)->AddAttributes(new nsDynamicEnumAttribute("PhysicsCollisionLayer")),
    NS_MEMBER_PROPERTY("Surface", m_sSurfaceFile)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Surface", nsDependencyFlags::Package)),
    NS_MEMBER_PROPERTY("MaxImpulse", m_fMaxImpulse)->AddAttributes(new nsDefaultValueAttribute(100.f)),

    NS_ARRAY_MEMBER_PROPERTY("Children", m_Children)->AddFlags(nsPropertyFlags::PointerOwner | nsPropertyFlags::Hidden),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_STATIC_REFLECTED_TYPE(nsExposedBone, nsNoBase, 1, nsRTTIDefaultAllocator<nsExposedBone>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Name", m_sName),
    NS_MEMBER_PROPERTY("Parent", m_sParent),
    NS_MEMBER_PROPERTY("Transform", m_Transform),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;

NS_DEFINE_CUSTOM_VARIANT_TYPE(nsExposedBone);
// clang-format on


void operator<<(nsStreamWriter& inout_stream, const nsExposedBone& bone)
{
  inout_stream << bone.m_sName;
  inout_stream << bone.m_sParent;
  inout_stream << bone.m_Transform;
}

void operator>>(nsStreamReader& inout_stream, nsExposedBone& ref_bone)
{
  inout_stream >> ref_bone.m_sName;
  inout_stream >> ref_bone.m_sParent;
  inout_stream >> ref_bone.m_Transform;
}

bool operator==(const nsExposedBone& lhs, const nsExposedBone& rhs)
{
  if (lhs.m_sName != rhs.m_sName)
    return false;
  if (lhs.m_sParent != rhs.m_sParent)
    return false;
  if (lhs.m_Transform != rhs.m_Transform)
    return false;
  return true;
}

nsEditableSkeleton::nsEditableSkeleton() = default;
nsEditableSkeleton::~nsEditableSkeleton()
{
  ClearJoints();
}

void nsEditableSkeleton::ClearJoints()
{
  for (nsEditableSkeletonJoint* pChild : m_Children)
  {
    NS_DEFAULT_DELETE(pChild);
  }

  m_Children.Clear();
}

void nsEditableSkeleton::CreateJointsRecursive(nsSkeletonBuilder& ref_sb, nsSkeletonResourceDescriptor& ref_desc, const nsEditableSkeletonJoint* pParentJoint, const nsEditableSkeletonJoint* pThisJoint, nsUInt16 uiThisJointIdx, const nsQuat& qParentAccuRot, const nsMat4& mRootTransform) const
{
  for (auto& shape : pThisJoint->m_BoneShapes)
  {
    auto& geo = ref_desc.m_Geometry.ExpandAndGetRef();

    geo.m_Type = shape.m_Geometry;
    geo.m_uiAttachedToJoint = static_cast<nsUInt16>(uiThisJointIdx);
    geo.m_Transform.SetIdentity();
    geo.m_Transform.m_vScale.Set(shape.m_fLength, shape.m_fWidth, shape.m_fThickness);
    geo.m_Transform.m_vPosition = shape.m_vOffset;
    geo.m_Transform.m_qRotation = shape.m_qRotation;
  }

  for (auto& shape : pThisJoint->m_BoneColliders)
  {
    auto& geo = ref_desc.m_Geometry.ExpandAndGetRef();
    geo.m_Type = nsSkeletonJointGeometryType::ConvexMesh;
    geo.m_uiAttachedToJoint = static_cast<nsUInt16>(uiThisJointIdx);
    geo.m_Transform.SetIdentity();
    geo.m_VertexPositions = shape.m_VertexPositions;
    geo.m_TriangleIndices = shape.m_TriangleIndices;
  }

  const nsVec3 s = pThisJoint->m_LocalTransform.m_vScale;
  if (!s.IsEqual(nsVec3(1), 0.1f))
  {
    // nsLog::Warning("Mesh bone '{}' has scaling values of {}/{}/{} - this is not supported.", pThisJoint->m_sName, s.x, s.y, s.z);
  }

  const nsQuat qThisAccuRot = qParentAccuRot * pThisJoint->m_LocalTransform.m_qRotation;
  nsQuat qParentGlobalRot;

  {
    // as always, the root transform is the bane of my existence
    // since it can contain mirroring, the final global rotation of a joint will be incorrect if we don't incorporate the root scale
    // unfortunately this can't be done once for the first node, but has to be done on the result instead

    nsMat4 full;
    nsMsgAnimationPoseUpdated::ComputeFullBoneTransform(mRootTransform, qParentAccuRot.GetAsMat4(), full, qParentGlobalRot);
  }

  ref_sb.SetJointLimit(uiThisJointIdx, pThisJoint->m_qLocalJointRotation, pThisJoint->m_JointType, pThisJoint->m_SwingLimitY, pThisJoint->m_SwingLimitZ, pThisJoint->m_TwistLimitHalfAngle, pThisJoint->m_TwistLimitCenterAngle, pThisJoint->m_fStiffness);

  ref_sb.SetJointCollisionLayer(uiThisJointIdx, pThisJoint->m_bOverrideCollisionLayer ? pThisJoint->m_uiCollisionLayerOverride : m_uiCollisionLayer);
  ref_sb.SetJointSurface(uiThisJointIdx, pThisJoint->m_bOverrideSurface ? pThisJoint->m_sSurfaceOverride : m_sSurfaceFile);

  for (const auto* pChildJoint : pThisJoint->m_Children)
  {
    const nsUInt16 uiChildJointIdx = ref_sb.AddJoint(pChildJoint->GetName(), pChildJoint->m_LocalTransform, uiThisJointIdx);

    CreateJointsRecursive(ref_sb, ref_desc, pThisJoint, pChildJoint, uiChildJointIdx, qThisAccuRot, mRootTransform);
  }
}

void nsEditableSkeleton::FillResourceDescriptor(nsSkeletonResourceDescriptor& ref_desc) const
{
  ref_desc.m_fMaxImpulse = m_fMaxImpulse;
  ref_desc.m_Geometry.Clear();

  nsSkeletonBuilder sb;
  for (const auto* pJoint : m_Children)
  {
    const nsUInt16 idx = sb.AddJoint(pJoint->GetName(), pJoint->m_LocalTransform);

    CreateJointsRecursive(sb, ref_desc, nullptr, pJoint, idx, nsQuat::MakeIdentity(), ref_desc.m_RootTransform.GetAsMat4());
  }

  sb.BuildSkeleton(ref_desc.m_Skeleton);
  ref_desc.m_Skeleton.m_BoneDirection = m_BoneDirection;
}

static void BuildOzzRawSkeleton(const nsEditableSkeletonJoint& srcJoint, ozz::animation::offline::RawSkeleton::Joint& ref_dstJoint)
{
  ref_dstJoint.name = srcJoint.m_sName.GetString();
  ref_dstJoint.transform.translation.x = srcJoint.m_LocalTransform.m_vPosition.x;
  ref_dstJoint.transform.translation.y = srcJoint.m_LocalTransform.m_vPosition.y;
  ref_dstJoint.transform.translation.z = srcJoint.m_LocalTransform.m_vPosition.z;
  ref_dstJoint.transform.rotation.x = srcJoint.m_LocalTransform.m_qRotation.x;
  ref_dstJoint.transform.rotation.y = srcJoint.m_LocalTransform.m_qRotation.y;
  ref_dstJoint.transform.rotation.z = srcJoint.m_LocalTransform.m_qRotation.z;
  ref_dstJoint.transform.rotation.w = srcJoint.m_LocalTransform.m_qRotation.w;
  ref_dstJoint.transform.scale.x = srcJoint.m_LocalTransform.m_vScale.x;
  ref_dstJoint.transform.scale.y = srcJoint.m_LocalTransform.m_vScale.y;
  ref_dstJoint.transform.scale.z = srcJoint.m_LocalTransform.m_vScale.z;

  ref_dstJoint.children.resize((size_t)srcJoint.m_Children.GetCount());

  for (nsUInt32 b = 0; b < srcJoint.m_Children.GetCount(); ++b)
  {
    BuildOzzRawSkeleton(*srcJoint.m_Children[b], ref_dstJoint.children[b]);
  }
}

void nsEditableSkeleton::GenerateRawOzzSkeleton(ozz::animation::offline::RawSkeleton& out_skeleton) const
{
  out_skeleton.roots.resize((size_t)m_Children.GetCount());

  for (nsUInt32 b = 0; b < m_Children.GetCount(); ++b)
  {
    BuildOzzRawSkeleton(*m_Children[b], out_skeleton.roots[b]);
  }
}

void nsEditableSkeleton::GenerateOzzSkeleton(ozz::animation::Skeleton& out_skeleton) const
{
  ozz::animation::offline::RawSkeleton rawSkeleton;
  GenerateRawOzzSkeleton(rawSkeleton);

  ozz::animation::offline::SkeletonBuilder skeletonBuilder;
  auto pNewOzzSkeleton = skeletonBuilder(rawSkeleton);

  nsOzzUtils::CopySkeleton(&out_skeleton, pNewOzzSkeleton.get());
}

nsEditableSkeletonJoint::nsEditableSkeletonJoint() = default;

nsEditableSkeletonJoint::~nsEditableSkeletonJoint()
{
  ClearJoints();
}

const char* nsEditableSkeletonJoint::GetName() const
{
  return m_sName.GetData();
}

void nsEditableSkeletonJoint::SetName(const char* szSz)
{
  m_sName.Assign(szSz);
}

void nsEditableSkeletonJoint::ClearJoints()
{
  for (nsEditableSkeletonJoint* pChild : m_Children)
  {
    NS_DEFAULT_DELETE(pChild);
  }
  m_Children.Clear();
}

void nsEditableSkeletonJoint::CopyPropertiesFrom(const nsEditableSkeletonJoint* pJoint)
{
  // copy existing (user edited) properties from pJoint into this joint
  // which has just been imported from file

  // do not copy:
  //  name
  //  transform
  //  children
  //  bone collider geometry (vertices, indices)

  // synchronize user config of bone colliders
  for (nsUInt32 i = 0; i < m_BoneColliders.GetCount(); ++i)
  {
    auto& dst = m_BoneColliders[i];

    for (nsUInt32 j = 0; j < pJoint->m_BoneColliders.GetCount(); ++j)
    {
      const auto& src = pJoint->m_BoneColliders[j];

      if (dst.m_sIdentifier == src.m_sIdentifier)
      {
        // dst.m_bOverrideSurface = src.m_bOverrideSurface;
        // dst.m_bOverrideCollisionLayer = src.m_bOverrideCollisionLayer;
        // dst.m_sSurfaceOverride = src.m_sSurfaceOverride;
        // dst.m_uiCollisionLayerOverride = src.m_uiCollisionLayerOverride;
        break;
      }
    }
  }

  m_BoneShapes = pJoint->m_BoneShapes;
  m_qLocalJointRotation = pJoint->m_qLocalJointRotation;
  m_JointType = pJoint->m_JointType;
  m_SwingLimitY = pJoint->m_SwingLimitY;
  m_SwingLimitZ = pJoint->m_SwingLimitZ;
  m_TwistLimitHalfAngle = pJoint->m_TwistLimitHalfAngle;
  m_TwistLimitCenterAngle = pJoint->m_TwistLimitCenterAngle;
  m_fStiffness = pJoint->m_fStiffness;

  m_bOverrideSurface = pJoint->m_bOverrideSurface;
  m_bOverrideCollisionLayer = pJoint->m_bOverrideCollisionLayer;
  m_sSurfaceOverride = pJoint->m_sSurfaceOverride;
  m_uiCollisionLayerOverride = pJoint->m_uiCollisionLayerOverride;
}

NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_Implementation_EditableSkeleton);

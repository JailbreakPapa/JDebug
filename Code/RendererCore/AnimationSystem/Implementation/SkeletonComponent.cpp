#include <RendererCore/RendererCorePCH.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/AnimationSystem/AnimationPose.h>
#include <RendererCore/AnimationSystem/SkeletonComponent.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/RenderWorld/RenderWorld.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/animation/runtime/skeleton_utils.h>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/simd_math.h>

// clang-format off
NS_BEGIN_COMPONENT_TYPE(nsSkeletonComponent, 5, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Skeleton", GetSkeletonFile, SetSkeletonFile)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Mesh_Skeleton")),
    NS_MEMBER_PROPERTY("VisualizeSkeleton", m_bVisualizeBones)->AddAttributes(new nsDefaultValueAttribute(true)),
    NS_MEMBER_PROPERTY("VisualizeColliders", m_bVisualizeColliders),
    NS_MEMBER_PROPERTY("VisualizeJoints", m_bVisualizeJoints),
    NS_MEMBER_PROPERTY("VisualizeSwingLimits", m_bVisualizeSwingLimits),
    NS_MEMBER_PROPERTY("VisualizeTwistLimits", m_bVisualizeTwistLimits),
    NS_ACCESSOR_PROPERTY("BonesToHighlight", GetBonesToHighlight, SetBonesToHighlight),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgAnimationPoseUpdated, OnAnimationPoseUpdated),
    NS_MESSAGE_HANDLER(nsMsgQueryAnimationSkeleton, OnQueryAnimationSkeleton)
  }
  NS_END_MESSAGEHANDLERS;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Animation"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsSkeletonComponent::nsSkeletonComponent() = default;
nsSkeletonComponent::~nsSkeletonComponent() = default;

nsResult nsSkeletonComponent::GetLocalBounds(nsBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, nsMsgUpdateLocalBounds& ref_msg)
{
  if (m_MaxBounds.IsValid())
  {
    nsBoundingBox bbox = m_MaxBounds;
    ref_bounds = nsBoundingBoxSphere::MakeFromBox(bbox);
    ref_bounds.Transform(m_RootTransform.GetAsMat4());
    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

void nsSkeletonComponent::Update()
{
  if (m_hSkeleton.IsValid() && (m_bVisualizeBones || m_bVisualizeColliders || m_bVisualizeJoints || m_bVisualizeSwingLimits || m_bVisualizeTwistLimits))
  {
    nsResourceLock<nsSkeletonResource> pSkeleton(m_hSkeleton, nsResourceAcquireMode::AllowLoadingFallback_NeverFail);

    if (pSkeleton.GetAcquireResult() != nsResourceAcquireResult::Final)
      return;

    if (m_uiSkeletonChangeCounter != pSkeleton->GetCurrentResourceChangeCounter())
    {
      VisualizeSkeletonDefaultState();
    }

    const nsQuat qBoneDir = nsBasisAxis::GetBasisRotation_PosX(pSkeleton->GetDescriptor().m_Skeleton.m_BoneDirection);
    const nsVec3 vBoneDir = qBoneDir * nsVec3(1, 0, 0);
    const nsVec3 vBoneTangent = qBoneDir * nsVec3(0, 1, 0);

    nsDebugRenderer::DrawLines(GetWorld(), m_LinesSkeleton, nsColor::White, GetOwner()->GetGlobalTransform());

    for (const auto& shape : m_SpheresShapes)
    {
      nsDebugRenderer::DrawLineSphere(GetWorld(), shape.m_Shape, shape.m_Color, GetOwner()->GetGlobalTransform() * shape.m_Transform);
    }

    for (const auto& shape : m_BoxShapes)
    {
      nsDebugRenderer::DrawLineBox(GetWorld(), shape.m_Shape, shape.m_Color, GetOwner()->GetGlobalTransform() * shape.m_Transform);
    }

    for (const auto& shape : m_CapsuleShapes)
    {
      nsDebugRenderer::DrawLineCapsuleZ(GetWorld(), shape.m_fLength, shape.m_fRadius, shape.m_Color, GetOwner()->GetGlobalTransform() * shape.m_Transform);
    }

    for (const auto& shape : m_AngleShapes)
    {
      nsDebugRenderer::DrawAngle(GetWorld(), shape.m_StartAngle, shape.m_EndAngle, nsColor::MakeZero(), shape.m_Color, GetOwner()->GetGlobalTransform() * shape.m_Transform, vBoneTangent, vBoneDir);
    }

    for (const auto& shape : m_ConeLimitShapes)
    {
      nsDebugRenderer::DrawLimitCone(GetWorld(), shape.m_Angle1, shape.m_Angle2, nsColor::MakeZero(), shape.m_Color, GetOwner()->GetGlobalTransform() * shape.m_Transform);
    }

    for (const auto& shape : m_CylinderShapes)
    {
      nsDebugRenderer::DrawCylinder(GetWorld(), shape.m_fRadius1, shape.m_fRadius2, shape.m_fLength, shape.m_Color, nsColor::MakeZero(), GetOwner()->GetGlobalTransform() * shape.m_Transform, false, false);
    }
  }
}

void nsSkeletonComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);

  auto& s = inout_stream.GetStream();

  s << m_hSkeleton;
  s << m_bVisualizeBones;
  s << m_sBonesToHighlight;
  s << m_bVisualizeColliders;
  s << m_bVisualizeJoints;
  s << m_bVisualizeSwingLimits;
  s << m_bVisualizeTwistLimits;
}

void nsSkeletonComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  const nsUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());

  if (uiVersion <= 4)
    return;

  auto& s = inout_stream.GetStream();

  s >> m_hSkeleton;
  s >> m_bVisualizeBones;
  s >> m_sBonesToHighlight;
  s >> m_bVisualizeColliders;
  s >> m_bVisualizeJoints;
  s >> m_bVisualizeSwingLimits;
  s >> m_bVisualizeTwistLimits;
}

void nsSkeletonComponent::OnActivated()
{
  SUPER::OnActivated();

  m_MaxBounds = nsBoundingBox::MakeInvalid();
  VisualizeSkeletonDefaultState();
}

void nsSkeletonComponent::SetSkeletonFile(const char* szFile)
{
  nsSkeletonResourceHandle hResource;

  if (!nsStringUtils::IsNullOrEmpty(szFile))
  {
    hResource = nsResourceManager::LoadResource<nsSkeletonResource>(szFile);
  }

  SetSkeleton(hResource);
}

const char* nsSkeletonComponent::GetSkeletonFile() const
{
  if (!m_hSkeleton.IsValid())
    return "";

  return m_hSkeleton.GetResourceID();
}


void nsSkeletonComponent::SetSkeleton(const nsSkeletonResourceHandle& hResource)
{
  if (m_hSkeleton != hResource)
  {
    m_hSkeleton = hResource;

    m_MaxBounds = nsBoundingBox::MakeInvalid();
    VisualizeSkeletonDefaultState();
  }
}

void nsSkeletonComponent::SetBonesToHighlight(const char* szFilter)
{
  if (m_sBonesToHighlight != szFilter)
  {
    m_sBonesToHighlight = szFilter;

    m_uiSkeletonChangeCounter = 0xFFFFFFFF;

    VisualizeSkeletonDefaultState();
  }
}

const char* nsSkeletonComponent::GetBonesToHighlight() const
{
  return m_sBonesToHighlight;
}

void nsSkeletonComponent::OnAnimationPoseUpdated(nsMsgAnimationPoseUpdated& msg)
{
  m_LinesSkeleton.Clear();
  m_SpheresShapes.Clear();
  m_BoxShapes.Clear();
  m_CapsuleShapes.Clear();
  m_AngleShapes.Clear();
  m_ConeLimitShapes.Clear();
  m_CylinderShapes.Clear();

  m_RootTransform = *msg.m_pRootTransform;

  BuildSkeletonVisualization(msg);
  BuildColliderVisualization(msg);
  BuildJointVisualization(msg);

  nsBoundingBox poseBounds;
  poseBounds = nsBoundingBox::MakeInvalid();

  for (const auto& bone : msg.m_ModelTransforms)
  {
    poseBounds.ExpandToInclude(bone.GetTranslationVector());
  }

  if (poseBounds.IsValid() && (!m_MaxBounds.IsValid() || !m_MaxBounds.Contains(poseBounds)))
  {
    m_MaxBounds.ExpandToInclude(poseBounds);
    TriggerLocalBoundsUpdate();
  }
  else if (((nsRenderWorld::GetFrameCounter() + GetUniqueIdForRendering()) & (NS_BIT(10) - 1)) == 0) // reset the bbox every once in a while
  {
    m_MaxBounds = poseBounds;
    TriggerLocalBoundsUpdate();
  }
}

void nsSkeletonComponent::BuildSkeletonVisualization(nsMsgAnimationPoseUpdated& msg)
{
  if (!m_bVisualizeBones || !msg.m_pSkeleton)
    return;

  nsStringBuilder tmp;

  struct Bone
  {
    nsVec3 pos = nsVec3::MakeZero();
    nsVec3 dir = nsVec3::MakeZero();
    float distToParent = 0.0f;
    float minDistToChild = 10.0f;
    bool highlight = false;
  };

  nsHybridArray<Bone, 128> bones;

  bones.SetCount(msg.m_pSkeleton->GetJointCount());
  m_LinesSkeleton.Reserve(m_LinesSkeleton.GetCount() + msg.m_pSkeleton->GetJointCount());

  const nsVec3 vBoneDir = nsBasisAxis::GetBasisVector(msg.m_pSkeleton->m_BoneDirection);

  auto renderBone = [&](int iCurrentBone, int iParentBone)
  {
    if (iParentBone == ozz::animation::Skeleton::kNoParent)
      return;

    const nsVec3 v0 = *msg.m_pRootTransform * msg.m_ModelTransforms[iParentBone].GetTranslationVector();
    const nsVec3 v1 = *msg.m_pRootTransform * msg.m_ModelTransforms[iCurrentBone].GetTranslationVector();

    nsVec3 dirToBone = (v1 - v0);

    auto& bone = bones[iCurrentBone];
    bone.pos = v1;
    bone.distToParent = dirToBone.GetLength();
    bone.dir = *msg.m_pRootTransform * msg.m_ModelTransforms[iCurrentBone].TransformDirection(vBoneDir);
    bone.dir.NormalizeIfNotZero(nsVec3::MakeZero()).IgnoreResult();

    auto& pb = bones[iParentBone];

    if (!pb.dir.IsZero() && dirToBone.NormalizeIfNotZero(nsVec3::MakeZero()).Succeeded())
    {
      if (pb.dir.GetAngleBetween(dirToBone) < nsAngle::MakeFromDegree(45))
      {
        nsPlane plane;
        plane = nsPlane::MakeFromNormalAndPoint(pb.dir, pb.pos);
        pb.minDistToChild = nsMath::Min(pb.minDistToChild, plane.GetDistanceTo(v1));
      }
    }
  };

  ozz::animation::IterateJointsDF(msg.m_pSkeleton->GetOzzSkeleton(), renderBone);

  if (m_sBonesToHighlight == "*")
  {
    for (nsUInt32 b = 0; b < bones.GetCount(); ++b)
    {
      bones[b].highlight = true;
    }
  }
  else if (!m_sBonesToHighlight.IsEmpty())
  {
    const nsStringBuilder mask(";", m_sBonesToHighlight, ";");

    for (nsUInt16 b = 0; b < static_cast<nsUInt16>(bones.GetCount()); ++b)
    {
      const nsString currentName = msg.m_pSkeleton->GetJointByIndex(b).GetName().GetString();

      tmp.Set(";", currentName, ";");

      if (mask.FindSubString(tmp))
      {
        bones[b].highlight = true;
      }
    }
  }

  for (nsUInt32 b = 0; b < bones.GetCount(); ++b)
  {
    const auto& bone = bones[b];

    if (!bone.highlight)
    {
      float len = 0.3f;

      if (bone.minDistToChild < 10.0f)
      {
        len = bone.minDistToChild;
      }
      else if (bone.distToParent > 0)
      {
        len = nsMath::Max(bone.distToParent * 0.5f, 0.1f);
      }
      else
      {
        len = 0.1f;
      }

      nsVec3 v0 = bone.pos;
      nsVec3 v1 = bone.pos + bone.dir * len;

      m_LinesSkeleton.PushBack(nsDebugRenderer::Line(v0, v1));
      m_LinesSkeleton.PeekBack().m_startColor = nsColor::DarkCyan;
      m_LinesSkeleton.PeekBack().m_endColor = nsColor::DarkCyan;
    }
  }

  for (nsUInt32 b = 0; b < bones.GetCount(); ++b)
  {
    const auto& bone = bones[b];

    if (bone.highlight && !bone.dir.IsZero(0.0001f))
    {
      float len = 0.3f;

      if (bone.minDistToChild < 10.0f)
      {
        len = bone.minDistToChild;
      }
      else if (bone.distToParent > 0)
      {
        len = nsMath::Max(bone.distToParent * 0.5f, 0.1f);
      }
      else
      {
        len = 0.1f;
      }

      nsVec3 v0 = bone.pos;
      nsVec3 v1 = bone.pos + bone.dir * len;

      const nsVec3 vO1 = bone.dir.GetOrthogonalVector().GetNormalized();
      const nsVec3 vO2 = bone.dir.CrossRH(vO1).GetNormalized();

      nsVec3 s[4];
      s[0] = v0 + vO1 * len * 0.1f + bone.dir * len * 0.1f;
      s[1] = v0 + vO2 * len * 0.1f + bone.dir * len * 0.1f;
      s[2] = v0 - vO1 * len * 0.1f + bone.dir * len * 0.1f;
      s[3] = v0 - vO2 * len * 0.1f + bone.dir * len * 0.1f;

      m_LinesSkeleton.PushBack(nsDebugRenderer::Line(v0, v1));
      m_LinesSkeleton.PeekBack().m_startColor = nsColor::DarkCyan;
      m_LinesSkeleton.PeekBack().m_endColor = nsColor::DarkCyan;

      for (nsUInt32 si = 0; si < 4; ++si)
      {
        m_LinesSkeleton.PushBack(nsDebugRenderer::Line(v0, s[si]));
        m_LinesSkeleton.PeekBack().m_startColor = nsColor::Chartreuse;
        m_LinesSkeleton.PeekBack().m_endColor = nsColor::Chartreuse;

        m_LinesSkeleton.PushBack(nsDebugRenderer::Line(s[si], v1));
        m_LinesSkeleton.PeekBack().m_startColor = nsColor::Chartreuse;
        m_LinesSkeleton.PeekBack().m_endColor = nsColor::Chartreuse;
      }
    }
  }
}

void nsSkeletonComponent::BuildColliderVisualization(nsMsgAnimationPoseUpdated& msg)
{
  if (!m_bVisualizeColliders || !msg.m_pSkeleton || !m_hSkeleton.IsValid())
    return;

  nsResourceLock<nsSkeletonResource> pSkeleton(m_hSkeleton, nsResourceAcquireMode::BlockTillLoaded);

  const auto srcBoneDir = pSkeleton->GetDescriptor().m_Skeleton.m_BoneDirection;
  const nsQuat qBoneDirAdjustment = nsBasisAxis::GetBasisRotation(nsBasisAxis::PositiveX, srcBoneDir);

  nsStringBuilder bonesToHighlight(";", m_sBonesToHighlight, ";");
  nsStringBuilder boneName;
  if (m_sBonesToHighlight == "*")
    bonesToHighlight.Clear();

  nsQuat qRotZtoX; // the capsule should extend along X, but the debug renderer draws them along Z
  qRotZtoX = nsQuat::MakeFromAxisAndAngle(nsVec3(0, 1, 0), nsAngle::MakeFromDegree(-90));

  for (const auto& geo : pSkeleton->GetDescriptor().m_Geometry)
  {
    if (geo.m_Type == nsSkeletonJointGeometryType::None)
      continue;

    nsMat4 boneTrans;
    nsQuat boneRot;
    msg.ComputeFullBoneTransform(geo.m_uiAttachedToJoint, boneTrans, boneRot);

    boneName.Set(";", msg.m_pSkeleton->GetJointByIndex(geo.m_uiAttachedToJoint).GetName().GetString(), ";");
    const bool bHighlight = bonesToHighlight.IsEmpty() || bonesToHighlight.FindLastSubString(boneName) != nullptr;
    const nsColor hlS = nsMath::Lerp(nsColor::DimGrey, nsColor::Yellow, bHighlight ? 1.0f : 0.2f);

    const nsQuat qFinalBoneRot = boneRot * qBoneDirAdjustment;

    nsTransform st;
    st.SetIdentity();
    st.m_vPosition = boneTrans.GetTranslationVector() + qFinalBoneRot * geo.m_Transform.m_vPosition;
    st.m_qRotation = qFinalBoneRot * geo.m_Transform.m_qRotation;

    if (geo.m_Type == nsSkeletonJointGeometryType::Sphere)
    {
      auto& shape = m_SpheresShapes.ExpandAndGetRef();
      shape.m_Transform = st;
      shape.m_Shape = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3::MakeZero(), geo.m_Transform.m_vScale.z);
      shape.m_Color = hlS;
    }

    if (geo.m_Type == nsSkeletonJointGeometryType::Box)
    {
      auto& shape = m_BoxShapes.ExpandAndGetRef();

      nsVec3 ext;
      ext.x = geo.m_Transform.m_vScale.x * 0.5f;
      ext.y = geo.m_Transform.m_vScale.y * 0.5f;
      ext.z = geo.m_Transform.m_vScale.z * 0.5f;

      // TODO: if offset desired
      st.m_vPosition += qFinalBoneRot * nsVec3(geo.m_Transform.m_vScale.x * 0.5f, 0, 0);

      shape.m_Transform = st;
      shape.m_Shape = nsBoundingBox::MakeFromCenterAndHalfExtents(nsVec3::MakeZero(), ext);
      shape.m_Color = hlS;
    }

    if (geo.m_Type == nsSkeletonJointGeometryType::Capsule)
    {
      st.m_qRotation = st.m_qRotation * qRotZtoX;

      // TODO: if offset desired
      st.m_vPosition += qFinalBoneRot * nsVec3(geo.m_Transform.m_vScale.x * 0.5f, 0, 0);

      auto& shape = m_CapsuleShapes.ExpandAndGetRef();
      shape.m_Transform = st;
      shape.m_fLength = geo.m_Transform.m_vScale.x;
      shape.m_fRadius = geo.m_Transform.m_vScale.z;
      shape.m_Color = hlS;
    }

    if (geo.m_Type == nsSkeletonJointGeometryType::ConvexMesh)
    {
      st.SetIdentity();
      st = *msg.m_pRootTransform;

      for (nsUInt32 f = 0; f < geo.m_TriangleIndices.GetCount(); f += 3)
      {
        const nsUInt32 i0 = geo.m_TriangleIndices[f + 0];
        const nsUInt32 i1 = geo.m_TriangleIndices[f + 1];
        const nsUInt32 i2 = geo.m_TriangleIndices[f + 2];

        {
          auto& l = m_LinesSkeleton.ExpandAndGetRef();
          l.m_startColor = l.m_endColor = hlS;
          l.m_start = st * geo.m_VertexPositions[i0];
          l.m_end = st * geo.m_VertexPositions[i1];
        }
        {
          auto& l = m_LinesSkeleton.ExpandAndGetRef();
          l.m_startColor = l.m_endColor = hlS;
          l.m_start = st * geo.m_VertexPositions[i1];
          l.m_end = st * geo.m_VertexPositions[i2];
        }
        {
          auto& l = m_LinesSkeleton.ExpandAndGetRef();
          l.m_startColor = l.m_endColor = hlS;
          l.m_start = st * geo.m_VertexPositions[i2];
          l.m_end = st * geo.m_VertexPositions[i0];
        }
      }
    }
  }
}

void nsSkeletonComponent::BuildJointVisualization(nsMsgAnimationPoseUpdated& msg)
{
  if (!m_hSkeleton.IsValid() || (!m_bVisualizeJoints && !m_bVisualizeSwingLimits && !m_bVisualizeTwistLimits))
    return;

  nsResourceLock<nsSkeletonResource> pSkeleton(m_hSkeleton, nsResourceAcquireMode::BlockTillLoaded);
  const auto& skel = pSkeleton->GetDescriptor().m_Skeleton;

  nsStringBuilder bonesToHighlight(";", m_sBonesToHighlight, ";");
  nsStringBuilder boneName;
  if (m_sBonesToHighlight == "*")
    bonesToHighlight.Clear();

  const nsQuat qBoneDir = nsBasisAxis::GetBasisRotation_PosX(pSkeleton->GetDescriptor().m_Skeleton.m_BoneDirection);
  const nsQuat qBoneDirT = nsBasisAxis::GetBasisRotation(nsBasisAxis::PositiveY, pSkeleton->GetDescriptor().m_Skeleton.m_BoneDirection);
  const nsQuat qBoneDirBT = nsBasisAxis::GetBasisRotation(nsBasisAxis::PositiveZ, pSkeleton->GetDescriptor().m_Skeleton.m_BoneDirection);
  const nsQuat qBoneDirT2 = nsBasisAxis::GetBasisRotation(nsBasisAxis::NegativeY, pSkeleton->GetDescriptor().m_Skeleton.m_BoneDirection);

  for (nsUInt16 uiJointIdx = 0; uiJointIdx < skel.GetJointCount(); ++uiJointIdx)
  {
    const auto& thisJoint = skel.GetJointByIndex(uiJointIdx);
    const nsUInt16 uiParentIdx = thisJoint.GetParentIndex();

    if (thisJoint.IsRootJoint())
      continue;

    boneName.Set(";", thisJoint.GetName().GetString(), ";");

    const bool bHighlight = bonesToHighlight.IsEmpty() || bonesToHighlight.FindSubString(boneName) != nullptr;

    nsMat4 parentTrans;
    nsQuat parentRot; // contains root transform
    msg.ComputeFullBoneTransform(uiParentIdx, parentTrans, parentRot);

    nsMat4 thisTrans; // contains root transform
    nsQuat thisRot;   // contains root transform
    msg.ComputeFullBoneTransform(uiJointIdx, thisTrans, thisRot);

    const nsVec3 vJointPos = thisTrans.GetTranslationVector();
    const nsQuat qLimitRot = parentRot * thisJoint.GetLocalOrientation();

    // main directions
    if (m_bVisualizeJoints)
    {
      const nsColor hlM = nsMath::Lerp(nsColor::OrangeRed, nsColor::DimGrey, bHighlight ? 0 : 0.8f);
      const nsColor hlT = nsMath::Lerp(nsColor::LawnGreen, nsColor::DimGrey, bHighlight ? 0 : 0.8f);
      const nsColor hlBT = nsMath::Lerp(nsColor::BlueViolet, nsColor::DimGrey, bHighlight ? 0 : 0.8f);

      {
        auto& cyl = m_CylinderShapes.ExpandAndGetRef();
        cyl.m_Color = hlM;
        cyl.m_fLength = 0.07f;
        cyl.m_fRadius1 = 0.002f;
        cyl.m_fRadius2 = 0.0f;
        cyl.m_Transform.m_vPosition = vJointPos;
        cyl.m_Transform.m_qRotation = thisRot * qBoneDir;
        cyl.m_Transform.m_vScale.Set(1);
      }

      {
        auto& cyl = m_CylinderShapes.ExpandAndGetRef();
        cyl.m_Color = hlT;
        cyl.m_fLength = 0.07f;
        cyl.m_fRadius1 = 0.002f;
        cyl.m_fRadius2 = 0.0f;
        cyl.m_Transform.m_vPosition = vJointPos;
        cyl.m_Transform.m_qRotation = thisRot * qBoneDirT;
        cyl.m_Transform.m_vScale.Set(1);
      }

      {
        auto& cyl = m_CylinderShapes.ExpandAndGetRef();
        cyl.m_Color = hlBT;
        cyl.m_fLength = 0.07f;
        cyl.m_fRadius1 = 0.002f;
        cyl.m_fRadius2 = 0.0f;
        cyl.m_Transform.m_vPosition = vJointPos;
        cyl.m_Transform.m_qRotation = thisRot * qBoneDirBT;
        cyl.m_Transform.m_vScale.Set(1);
      }
    }

    // swing limit
    if (m_bVisualizeSwingLimits && (thisJoint.GetHalfSwingLimitY() > nsAngle() || thisJoint.GetHalfSwingLimitZ() > nsAngle()))
    {
      auto& shape = m_ConeLimitShapes.ExpandAndGetRef();
      shape.m_Angle1 = thisJoint.GetHalfSwingLimitY();
      shape.m_Angle2 = thisJoint.GetHalfSwingLimitZ();
      shape.m_Color = nsMath::Lerp(nsColor::DimGrey, nsColor::DeepPink, bHighlight ? 1.0f : 0.2f);
      shape.m_Transform.m_vScale.Set(0.05f);
      shape.m_Transform.m_vPosition = vJointPos;
      shape.m_Transform.m_qRotation = qLimitRot * qBoneDir;

      const nsColor hlM = nsMath::Lerp(nsColor::OrangeRed, nsColor::DimGrey, bHighlight ? 0 : 0.8f);

      {
        auto& cyl = m_CylinderShapes.ExpandAndGetRef();
        cyl.m_Color = hlM;
        cyl.m_fLength = 0.07f;
        cyl.m_fRadius1 = 0.002f;
        cyl.m_fRadius2 = 0.0f;
        cyl.m_Transform.m_vPosition = vJointPos;
        cyl.m_Transform.m_qRotation = thisRot * qBoneDir;
        cyl.m_Transform.m_vScale.Set(1);
      }
    }

    // twist limit
    if (m_bVisualizeTwistLimits && thisJoint.GetTwistLimitHalfAngle() > nsAngle::MakeFromDegree(0))
    {
      auto& shape = m_AngleShapes.ExpandAndGetRef();
      shape.m_StartAngle = thisJoint.GetTwistLimitLow();
      shape.m_EndAngle = thisJoint.GetTwistLimitHigh();
      shape.m_Color = nsMath::Lerp(nsColor::DimGrey, nsColor::LightPink, bHighlight ? 0.8f : 0.2f);
      shape.m_Transform.m_vScale.Set(0.04f);
      shape.m_Transform.m_vPosition = vJointPos;
      shape.m_Transform.m_qRotation = qLimitRot;

      const nsColor hlT = nsMath::Lerp(nsColor::DimGrey, nsColor::LightPink, bHighlight ? 1.0f : 0.4f);

      {
        auto& cyl = m_CylinderShapes.ExpandAndGetRef();
        cyl.m_Color = hlT;
        cyl.m_fLength = 0.07f;
        cyl.m_fRadius1 = 0.002f;
        cyl.m_fRadius2 = 0.0f;
        cyl.m_Transform.m_vPosition = vJointPos;
        cyl.m_Transform.m_qRotation = thisRot * qBoneDirT2;
        cyl.m_Transform.m_vScale.Set(1);

        nsVec3 vDir = cyl.m_Transform.m_qRotation * nsVec3(1, 0, 0);
        vDir.Normalize();

        nsVec3 vDirRef = shape.m_Transform.m_qRotation * qBoneDir * nsVec3(0, 1, 0);
        vDirRef.Normalize();

        const nsVec3 vRotDir = shape.m_Transform.m_qRotation * qBoneDir * nsVec3(1, 0, 0);
        nsQuat qRotRef = nsQuat::MakeFromAxisAndAngle(vRotDir, thisJoint.GetTwistLimitCenterAngle());
        vDirRef = qRotRef * vDirRef;

        // if the current twist is outside the twist limit range, highlight the bone
        if (vDir.GetAngleBetween(vDirRef) > thisJoint.GetTwistLimitHalfAngle())
        {
          cyl.m_Color = nsColor::Orange;
        }
      }
    }
  }
}

void nsSkeletonComponent::VisualizeSkeletonDefaultState()
{
  if (!IsActiveAndInitialized())
    return;

  m_uiSkeletonChangeCounter = 0;

  if (m_hSkeleton.IsValid())
  {
    nsResourceLock<nsSkeletonResource> pSkeleton(m_hSkeleton, nsResourceAcquireMode::BlockTillLoaded_NeverFail);
    if (pSkeleton.GetAcquireResult() == nsResourceAcquireResult::Final)
    {
      m_uiSkeletonChangeCounter = pSkeleton->GetCurrentResourceChangeCounter();

      if (pSkeleton->GetDescriptor().m_Skeleton.GetJointCount() > 0)
      {
        ozz::vector<ozz::math::Float4x4> modelTransforms;
        modelTransforms.resize(pSkeleton->GetDescriptor().m_Skeleton.GetJointCount());

        {
          ozz::animation::LocalToModelJob job;
          job.input = pSkeleton->GetDescriptor().m_Skeleton.GetOzzSkeleton().joint_rest_poses();
          job.output = make_span(modelTransforms);
          job.skeleton = &pSkeleton->GetDescriptor().m_Skeleton.GetOzzSkeleton();
          job.Run();
        }

        nsMsgAnimationPoseUpdated msg;
        msg.m_pRootTransform = &pSkeleton->GetDescriptor().m_RootTransform;
        msg.m_pSkeleton = &pSkeleton->GetDescriptor().m_Skeleton;
        msg.m_ModelTransforms = nsArrayPtr<const nsMat4>(reinterpret_cast<const nsMat4*>(&modelTransforms[0]), (nsUInt32)modelTransforms.size());

        OnAnimationPoseUpdated(msg);
      }
    }
  }

  TriggerLocalBoundsUpdate();
}

nsDebugRenderer::Line& nsSkeletonComponent::AddLine(const nsVec3& vStart, const nsVec3& vEnd, const nsColor& color)
{
  auto& line = m_LinesSkeleton.ExpandAndGetRef();
  line.m_start = vStart;
  line.m_end = vEnd;
  line.m_startColor = color;
  line.m_endColor = color;
  return line;
}

void nsSkeletonComponent::OnQueryAnimationSkeleton(nsMsgQueryAnimationSkeleton& msg)
{
  // if we have a skeleton, always overwrite it any incoming message with that
  if (m_hSkeleton.IsValid())
  {
    msg.m_hSkeleton = m_hSkeleton;
  }
}

NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_Implementation_SkeletonComponent);

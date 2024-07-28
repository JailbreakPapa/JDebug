#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Utilities/AssetFileHeader.h>
#include <RendererCore/AnimationSystem/AnimationClipResource.h>
#include <RendererCore/AnimationSystem/AnimationPose.h>
#include <RendererCore/AnimationSystem/Skeleton.h>
#include <RendererCore/AnimationSystem/SkeletonResource.h>
#include <ozz/animation/offline/animation_builder.h>
#include <ozz/animation/offline/animation_optimizer.h>
#include <ozz/animation/offline/raw_animation.h>
#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/skeleton.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAnimationClipResource, 1, nsRTTIDefaultAllocator<nsAnimationClipResource>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_RESOURCE_IMPLEMENT_COMMON_CODE(nsAnimationClipResource);
// clang-format on

nsAnimationClipResource::nsAnimationClipResource()
  : nsResource(DoUpdate::OnAnyThread, 1)
{
}

NS_RESOURCE_IMPLEMENT_CREATEABLE(nsAnimationClipResource, nsAnimationClipResourceDescriptor)
{
  m_pDescriptor = NS_DEFAULT_NEW(nsAnimationClipResourceDescriptor);
  *m_pDescriptor = std::move(descriptor);

  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Loaded;

  return res;
}

nsResourceLoadDesc nsAnimationClipResource::UnloadData(Unload WhatToUnload)
{
  m_pDescriptor.Clear();

  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Unloaded;

  return res;
}

nsResourceLoadDesc nsAnimationClipResource::UpdateContent(nsStreamReader* Stream)
{
  NS_LOG_BLOCK("nsAnimationClipResource::UpdateContent", GetResourceIdOrDescription());

  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;

  if (Stream == nullptr)
  {
    res.m_State = nsResourceState::LoadedResourceMissing;
    return res;
  }

  // skip the absolute file path data that the standard file reader writes into the stream
  {
    nsStringBuilder sAbsFilePath;
    (*Stream) >> sAbsFilePath;
  }

  // skip the asset file header at the start of the file
  nsAssetFileHeader AssetHash;
  AssetHash.Read(*Stream).IgnoreResult();

  m_pDescriptor = NS_DEFAULT_NEW(nsAnimationClipResourceDescriptor);
  m_pDescriptor->Deserialize(*Stream).IgnoreResult();

  res.m_State = nsResourceState::Loaded;
  return res;
}

void nsAnimationClipResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  out_NewMemoryUsage.m_uiMemoryGPU = 0;
  out_NewMemoryUsage.m_uiMemoryCPU = sizeof(nsAnimationClipResource);

  if (m_pDescriptor)
  {
    out_NewMemoryUsage.m_uiMemoryCPU += static_cast<nsUInt32>(m_pDescriptor->GetHeapMemoryUsage());
  }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct nsAnimationClipResourceDescriptor::OzzImpl
{
  struct CachedAnim
  {
    nsUInt32 m_uiResourceChangeCounter = 0;
    ozz::unique_ptr<ozz::animation::Animation> m_pAnim;
  };

  nsMap<const nsSkeletonResource*, CachedAnim> m_MappedOzzAnimations;
};

nsAnimationClipResourceDescriptor::nsAnimationClipResourceDescriptor()
{
  m_pOzzImpl = NS_DEFAULT_NEW(OzzImpl);
}

nsAnimationClipResourceDescriptor::nsAnimationClipResourceDescriptor(nsAnimationClipResourceDescriptor&& rhs)
{
  *this = std::move(rhs);
}

nsAnimationClipResourceDescriptor::~nsAnimationClipResourceDescriptor() = default;

void nsAnimationClipResourceDescriptor::operator=(nsAnimationClipResourceDescriptor&& rhs) noexcept
{
  m_pOzzImpl = std::move(rhs.m_pOzzImpl);

  m_JointInfos = std::move(rhs.m_JointInfos);
  m_Transforms = std::move(rhs.m_Transforms);
  m_uiNumTotalPositions = rhs.m_uiNumTotalPositions;
  m_uiNumTotalRotations = rhs.m_uiNumTotalRotations;
  m_uiNumTotalScales = rhs.m_uiNumTotalScales;
  m_Duration = rhs.m_Duration;
}

nsResult nsAnimationClipResourceDescriptor::Serialize(nsStreamWriter& inout_stream) const
{
  inout_stream.WriteVersion(9);

  const nsUInt16 uiNumJoints = static_cast<nsUInt16>(m_JointInfos.GetCount());
  inout_stream << uiNumJoints;
  for (nsUInt32 i = 0; i < m_JointInfos.GetCount(); ++i)
  {
    const auto& val = m_JointInfos.GetValue(i);

    inout_stream << m_JointInfos.GetKey(i);
    inout_stream << val.m_uiPositionIdx;
    inout_stream << val.m_uiPositionCount;
    inout_stream << val.m_uiRotationIdx;
    inout_stream << val.m_uiRotationCount;
    inout_stream << val.m_uiScaleIdx;
    inout_stream << val.m_uiScaleCount;
  }

  inout_stream << m_Duration;
  inout_stream << m_uiNumTotalPositions;
  inout_stream << m_uiNumTotalRotations;
  inout_stream << m_uiNumTotalScales;

  NS_SUCCEED_OR_RETURN(inout_stream.WriteArray(m_Transforms));

  inout_stream << m_vConstantRootMotion;

  m_EventTrack.Save(inout_stream);

  inout_stream << m_bAdditive;

  return NS_SUCCESS;
}

nsResult nsAnimationClipResourceDescriptor::Deserialize(nsStreamReader& inout_stream)
{
  const nsTypeVersion uiVersion = inout_stream.ReadVersion(9);

  if (uiVersion < 6)
    return NS_FAILURE;

  nsUInt16 uiNumJoints = 0;
  inout_stream >> uiNumJoints;

  m_JointInfos.Reserve(uiNumJoints);

  nsHashedString hs;

  for (nsUInt16 i = 0; i < uiNumJoints; ++i)
  {
    inout_stream >> hs;

    JointInfo ji;
    inout_stream >> ji.m_uiPositionIdx;
    inout_stream >> ji.m_uiPositionCount;
    inout_stream >> ji.m_uiRotationIdx;
    inout_stream >> ji.m_uiRotationCount;
    inout_stream >> ji.m_uiScaleIdx;
    inout_stream >> ji.m_uiScaleCount;

    m_JointInfos.Insert(hs, ji);
  }

  m_JointInfos.Sort();

  inout_stream >> m_Duration;
  inout_stream >> m_uiNumTotalPositions;
  inout_stream >> m_uiNumTotalRotations;
  inout_stream >> m_uiNumTotalScales;

  NS_SUCCEED_OR_RETURN(inout_stream.ReadArray(m_Transforms));

  if (uiVersion >= 7)
  {
    inout_stream >> m_vConstantRootMotion;
  }

  if (uiVersion >= 8)
  {
    m_EventTrack.Load(inout_stream);
  }

  if (uiVersion >= 9)
  {
    inout_stream >> m_bAdditive;
  }

  return NS_SUCCESS;
}

nsUInt64 nsAnimationClipResourceDescriptor::GetHeapMemoryUsage() const
{
  return m_Transforms.GetHeapMemoryUsage() + m_JointInfos.GetHeapMemoryUsage() + m_pOzzImpl->m_MappedOzzAnimations.GetHeapMemoryUsage();
}

nsUInt16 nsAnimationClipResourceDescriptor::GetNumJoints() const
{
  return static_cast<nsUInt16>(m_JointInfos.GetCount());
}

nsTime nsAnimationClipResourceDescriptor::GetDuration() const
{
  return m_Duration;
}

void nsAnimationClipResourceDescriptor::SetDuration(nsTime duration)
{
  m_Duration = duration;
}

NS_FORCE_INLINE void ns2ozz(const nsVec3& vIn, ozz::math::Float3& ref_out)
{
  ref_out.x = vIn.x;
  ref_out.y = vIn.y;
  ref_out.z = vIn.z;
}

NS_FORCE_INLINE void ns2ozz(const nsQuat& qIn, ozz::math::Quaternion& ref_out)
{
  ref_out.x = qIn.x;
  ref_out.y = qIn.y;
  ref_out.z = qIn.z;
  ref_out.w = qIn.w;
}

const ozz::animation::Animation& nsAnimationClipResourceDescriptor::GetMappedOzzAnimation(const nsSkeletonResource& skeleton) const
{
  auto it = m_pOzzImpl->m_MappedOzzAnimations.Find(&skeleton);
  if (it.IsValid())
  {
    if (it.Value().m_uiResourceChangeCounter == skeleton.GetCurrentResourceChangeCounter())
    {
      return *it.Value().m_pAnim.get();
    }
  }

  auto pOzzSkeleton = &skeleton.GetDescriptor().m_Skeleton.GetOzzSkeleton();
  const nsUInt32 uiNumJoints = pOzzSkeleton->num_joints();

  ozz::animation::offline::RawAnimation rawAnim;
  rawAnim.duration = nsMath::Max(1.0f / 60.0f, m_Duration.AsFloatInSeconds());
  rawAnim.tracks.resize(uiNumJoints);

  for (nsUInt32 j = 0; j < uiNumJoints; ++j)
  {
    auto& dstTrack = rawAnim.tracks[j];

    const nsTempHashedString sJointName = nsTempHashedString(pOzzSkeleton->joint_names()[j]);

    const JointInfo* pJointInfo = GetJointInfo(sJointName);

    if (pJointInfo == nullptr)
    {
      dstTrack.translations.resize(1);
      dstTrack.rotations.resize(1);
      dstTrack.scales.resize(1);

      const nsUInt16 uiFallbackIdx = skeleton.GetDescriptor().m_Skeleton.FindJointByName(sJointName);

      NS_ASSERT_DEV(uiFallbackIdx != nsInvalidJointIndex, "");

      const auto& fallbackJoint = skeleton.GetDescriptor().m_Skeleton.GetJointByIndex(uiFallbackIdx);

      const nsTransform& fallbackTransform = m_bAdditive ? nsTransform::MakeIdentity() : fallbackJoint.GetRestPoseLocalTransform();

      auto& dstT = dstTrack.translations[0];
      auto& dstR = dstTrack.rotations[0];
      auto& dstS = dstTrack.scales[0];

      dstT.time = 0.0f;
      dstR.time = 0.0f;
      dstS.time = 0.0f;

      ns2ozz(fallbackTransform.m_vPosition, dstT.value);
      ns2ozz(fallbackTransform.m_qRotation, dstR.value);
      ns2ozz(fallbackTransform.m_vScale, dstS.value);
    }
    else
    {
      // positions
      {
        dstTrack.translations.resize(pJointInfo->m_uiPositionCount);
        const nsArrayPtr<const KeyframeVec3> keyframes = GetPositionKeyframes(*pJointInfo);

        for (nsUInt32 i = 0; i < pJointInfo->m_uiPositionCount; ++i)
        {
          auto& dst = dstTrack.translations[i];

          dst.time = keyframes[i].m_fTimeInSec;
          ns2ozz(keyframes[i].m_Value, dst.value);
        }
      }

      // rotations
      {
        dstTrack.rotations.resize(pJointInfo->m_uiRotationCount);
        const nsArrayPtr<const KeyframeQuat> keyframes = GetRotationKeyframes(*pJointInfo);

        for (nsUInt32 i = 0; i < pJointInfo->m_uiRotationCount; ++i)
        {
          auto& dst = dstTrack.rotations[i];

          dst.time = keyframes[i].m_fTimeInSec;
          ns2ozz(keyframes[i].m_Value, dst.value);
        }
      }

      // scales
      {
        dstTrack.scales.resize(pJointInfo->m_uiScaleCount);
        const nsArrayPtr<const KeyframeVec3> keyframes = GetScaleKeyframes(*pJointInfo);

        for (nsUInt32 i = 0; i < pJointInfo->m_uiScaleCount; ++i)
        {
          auto& dst = dstTrack.scales[i];

          dst.time = keyframes[i].m_fTimeInSec;
          ns2ozz(keyframes[i].m_Value, dst.value);
        }
      }
    }
  }

  ozz::animation::offline::AnimationBuilder animBuilder;

  NS_ASSERT_DEBUG(rawAnim.Validate(), "Invalid animation data");

  auto& cached = m_pOzzImpl->m_MappedOzzAnimations[&skeleton];
  cached.m_pAnim = std::move(animBuilder(rawAnim));
  cached.m_uiResourceChangeCounter = skeleton.GetCurrentResourceChangeCounter();

  return *cached.m_pAnim.get();
}

nsAnimationClipResourceDescriptor::JointInfo nsAnimationClipResourceDescriptor::CreateJoint(const nsHashedString& sJointName, nsUInt16 uiNumPositions, nsUInt16 uiNumRotations, nsUInt16 uiNumScales)
{
  JointInfo ji;
  ji.m_uiPositionIdx = m_uiNumTotalPositions;
  ji.m_uiRotationIdx = m_uiNumTotalRotations;
  ji.m_uiScaleIdx = m_uiNumTotalScales;

  ji.m_uiPositionCount = uiNumPositions;
  ji.m_uiRotationCount = uiNumRotations;
  ji.m_uiScaleCount = uiNumScales;

  m_uiNumTotalPositions += uiNumPositions;
  m_uiNumTotalRotations += uiNumRotations;
  m_uiNumTotalScales += uiNumScales;

  m_JointInfos.Insert(sJointName, ji);

  return ji;
}

const nsAnimationClipResourceDescriptor::JointInfo* nsAnimationClipResourceDescriptor::GetJointInfo(const nsTempHashedString& sJointName) const
{
  nsUInt32 uiIndex = m_JointInfos.Find(sJointName);

  if (uiIndex == nsInvalidIndex)
    return nullptr;

  return &m_JointInfos.GetValue(uiIndex);
}

void nsAnimationClipResourceDescriptor::AllocateJointTransforms()
{
  const nsUInt32 uiNumBytes = m_uiNumTotalPositions * sizeof(KeyframeVec3) + m_uiNumTotalRotations * sizeof(KeyframeQuat) + m_uiNumTotalScales * sizeof(KeyframeVec3);

  m_Transforms.SetCountUninitialized(uiNumBytes);
}

nsArrayPtr<nsAnimationClipResourceDescriptor::KeyframeVec3> nsAnimationClipResourceDescriptor::GetPositionKeyframes(const JointInfo& jointInfo)
{
  NS_ASSERT_DEBUG(!m_Transforms.IsEmpty(), "Joint transforms have not been allocated yet.");

  nsUInt32 uiByteOffsetStart = 0;
  uiByteOffsetStart += sizeof(KeyframeVec3) * jointInfo.m_uiPositionIdx;

  return nsArrayPtr<KeyframeVec3>(reinterpret_cast<KeyframeVec3*>(m_Transforms.GetData() + uiByteOffsetStart), jointInfo.m_uiPositionCount);
}

nsArrayPtr<nsAnimationClipResourceDescriptor::KeyframeQuat> nsAnimationClipResourceDescriptor::GetRotationKeyframes(const JointInfo& jointInfo)
{
  NS_ASSERT_DEBUG(!m_Transforms.IsEmpty(), "Joint transforms have not been allocated yet.");

  nsUInt32 uiByteOffsetStart = 0;
  uiByteOffsetStart += sizeof(KeyframeVec3) * m_uiNumTotalPositions;
  uiByteOffsetStart += sizeof(KeyframeQuat) * jointInfo.m_uiRotationIdx;

  return nsArrayPtr<KeyframeQuat>(reinterpret_cast<KeyframeQuat*>(m_Transforms.GetData() + uiByteOffsetStart), jointInfo.m_uiRotationCount);
}

nsArrayPtr<nsAnimationClipResourceDescriptor::KeyframeVec3> nsAnimationClipResourceDescriptor::GetScaleKeyframes(const JointInfo& jointInfo)
{
  NS_ASSERT_DEBUG(!m_Transforms.IsEmpty(), "Joint transforms have not been allocated yet.");

  nsUInt32 uiByteOffsetStart = 0;
  uiByteOffsetStart += sizeof(KeyframeVec3) * m_uiNumTotalPositions;
  uiByteOffsetStart += sizeof(KeyframeQuat) * m_uiNumTotalRotations;
  uiByteOffsetStart += sizeof(KeyframeVec3) * jointInfo.m_uiScaleIdx;

  return nsArrayPtr<KeyframeVec3>(reinterpret_cast<KeyframeVec3*>(m_Transforms.GetData() + uiByteOffsetStart), jointInfo.m_uiScaleCount);
}

nsArrayPtr<const nsAnimationClipResourceDescriptor::KeyframeVec3> nsAnimationClipResourceDescriptor::GetPositionKeyframes(const JointInfo& jointInfo) const
{
  nsUInt32 uiByteOffsetStart = 0;
  uiByteOffsetStart += sizeof(KeyframeVec3) * jointInfo.m_uiPositionIdx;

  return nsArrayPtr<const KeyframeVec3>(reinterpret_cast<const KeyframeVec3*>(m_Transforms.GetData() + uiByteOffsetStart), jointInfo.m_uiPositionCount);
}

nsArrayPtr<const nsAnimationClipResourceDescriptor::KeyframeQuat> nsAnimationClipResourceDescriptor::GetRotationKeyframes(const JointInfo& jointInfo) const
{
  nsUInt32 uiByteOffsetStart = 0;
  uiByteOffsetStart += sizeof(KeyframeVec3) * m_uiNumTotalPositions;
  uiByteOffsetStart += sizeof(KeyframeQuat) * jointInfo.m_uiRotationIdx;

  return nsArrayPtr<const KeyframeQuat>(reinterpret_cast<const KeyframeQuat*>(m_Transforms.GetData() + uiByteOffsetStart), jointInfo.m_uiRotationCount);
}

nsArrayPtr<const nsAnimationClipResourceDescriptor::KeyframeVec3> nsAnimationClipResourceDescriptor::GetScaleKeyframes(const JointInfo& jointInfo) const
{
  nsUInt32 uiByteOffsetStart = 0;
  uiByteOffsetStart += sizeof(KeyframeVec3) * m_uiNumTotalPositions;
  uiByteOffsetStart += sizeof(KeyframeQuat) * m_uiNumTotalRotations;
  uiByteOffsetStart += sizeof(KeyframeVec3) * jointInfo.m_uiScaleIdx;

  return nsArrayPtr<const KeyframeVec3>(reinterpret_cast<const KeyframeVec3*>(m_Transforms.GetData() + uiByteOffsetStart), jointInfo.m_uiScaleCount);
}

// bool nsAnimationClipResourceDescriptor::HasRootMotion() const
//{
//  return m_JointNameToIndex.Contains(nsTempHashedString("nsRootMotionTransform"));
//}
//
// nsUInt16 nsAnimationClipResourceDescriptor::GetRootMotionJoint() const
//{
//  nsUInt16 jointIdx = 0;
//
// #if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
//
//  const nsUInt32 idx = m_JointNameToIndex.Find(nsTempHashedString("nsRootMotionTransform"));
//  NS_ASSERT_DEBUG(idx != nsInvalidIndex, "Animation Clip has no root motion transforms");
//
//  jointIdx = m_JointNameToIndex.GetValue(idx);
//  NS_ASSERT_DEBUG(jointIdx == 0, "The root motion joint should always be at index 0");
// #endif
//
//  return jointIdx;
//}

NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_Implementation_AnimationClipResource);

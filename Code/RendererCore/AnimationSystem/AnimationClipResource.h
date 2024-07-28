#pragma once

#include <RendererCore/RendererCoreDLL.h>

#include <Core/ResourceManager/Resource.h>
#include <Foundation/Containers/ArrayMap.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Tracks/EventTrack.h>

class nsSkeletonResource;

namespace ozz::animation
{
  class Animation;
}

struct NS_RENDERERCORE_DLL nsAnimationClipResourceDescriptor
{
public:
  nsAnimationClipResourceDescriptor();
  nsAnimationClipResourceDescriptor(nsAnimationClipResourceDescriptor&& rhs);
  ~nsAnimationClipResourceDescriptor();

  void operator=(nsAnimationClipResourceDescriptor&& rhs) noexcept;

  nsResult Serialize(nsStreamWriter& inout_stream) const;
  nsResult Deserialize(nsStreamReader& inout_stream);

  nsUInt64 GetHeapMemoryUsage() const;

  nsUInt16 GetNumJoints() const;
  nsTime GetDuration() const;
  void SetDuration(nsTime duration);

  const ozz::animation::Animation& GetMappedOzzAnimation(const nsSkeletonResource& skeleton) const;

  struct JointInfo
  {
    nsUInt32 m_uiPositionIdx = 0;
    nsUInt32 m_uiRotationIdx = 0;
    nsUInt32 m_uiScaleIdx = 0;
    nsUInt16 m_uiPositionCount = 0;
    nsUInt16 m_uiRotationCount = 0;
    nsUInt16 m_uiScaleCount = 0;
  };

  struct KeyframeVec3
  {
    float m_fTimeInSec;
    nsVec3 m_Value;
  };

  struct KeyframeQuat
  {
    float m_fTimeInSec;
    nsQuat m_Value;
  };

  JointInfo CreateJoint(const nsHashedString& sJointName, nsUInt16 uiNumPositions, nsUInt16 uiNumRotations, nsUInt16 uiNumScales);
  const JointInfo* GetJointInfo(const nsTempHashedString& sJointName) const;
  void AllocateJointTransforms();

  nsArrayPtr<KeyframeVec3> GetPositionKeyframes(const JointInfo& jointInfo);
  nsArrayPtr<KeyframeQuat> GetRotationKeyframes(const JointInfo& jointInfo);
  nsArrayPtr<KeyframeVec3> GetScaleKeyframes(const JointInfo& jointInfo);

  nsArrayPtr<const KeyframeVec3> GetPositionKeyframes(const JointInfo& jointInfo) const;
  nsArrayPtr<const KeyframeQuat> GetRotationKeyframes(const JointInfo& jointInfo) const;
  nsArrayPtr<const KeyframeVec3> GetScaleKeyframes(const JointInfo& jointInfo) const;

  nsVec3 m_vConstantRootMotion = nsVec3::MakeZero();

  nsEventTrack m_EventTrack;

  bool m_bAdditive = false;

private:
  nsArrayMap<nsHashedString, JointInfo> m_JointInfos;
  nsDataBuffer m_Transforms;
  nsUInt32 m_uiNumTotalPositions = 0;
  nsUInt32 m_uiNumTotalRotations = 0;
  nsUInt32 m_uiNumTotalScales = 0;
  nsTime m_Duration;

  struct OzzImpl;
  nsUniquePtr<OzzImpl> m_pOzzImpl;
};

//////////////////////////////////////////////////////////////////////////

using nsAnimationClipResourceHandle = nsTypedResourceHandle<class nsAnimationClipResource>;

class NS_RENDERERCORE_DLL nsAnimationClipResource : public nsResource
{
  NS_ADD_DYNAMIC_REFLECTION(nsAnimationClipResource, nsResource);
  NS_RESOURCE_DECLARE_COMMON_CODE(nsAnimationClipResource);
  NS_RESOURCE_DECLARE_CREATEABLE(nsAnimationClipResource, nsAnimationClipResourceDescriptor);

public:
  nsAnimationClipResource();

  const nsAnimationClipResourceDescriptor& GetDescriptor() const { return *m_pDescriptor; }

private:
  virtual nsResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual nsResourceLoadDesc UpdateContent(nsStreamReader* Stream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

  nsUniquePtr<nsAnimationClipResourceDescriptor> m_pDescriptor;
};

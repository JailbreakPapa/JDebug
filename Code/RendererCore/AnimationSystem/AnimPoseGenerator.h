#pragma once

#include <Core/ResourceManager/ResourceHandle.h>
#include <Foundation/Containers/ArrayMap.h>
#include <Foundation/Types/UniquePtr.h>
#include <RendererCore/AnimationSystem/Declarations.h>
#include <RendererCore/RendererCoreDLL.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/base/maths/soa_float.h>
#include <ozz/base/maths/soa_transform.h>

NS_DEFINE_AS_POD_TYPE(ozz::math::SoaTransform);

class nsSkeletonResource;
class nsAnimPoseGenerator;
class nsGameObject;

using nsAnimationClipResourceHandle = nsTypedResourceHandle<class nsAnimationClipResource>;

using nsAnimPoseGeneratorLocalPoseID = nsUInt32;
using nsAnimPoseGeneratorModelPoseID = nsUInt32;
using nsAnimPoseGeneratorCommandID = nsUInt32;

/// \brief The type of nsAnimPoseGeneratorCommand
enum class nsAnimPoseGeneratorCommandType
{
  Invalid,
  SampleTrack,
  RestPose,
  CombinePoses,
  LocalToModelPose,
  SampleEventTrack,
  AimIK,
  TwoBoneIK,
};

enum class nsAnimPoseEventTrackSampleMode : nsUInt8
{
  None,         ///< Don't sample the event track at all
  OnlyBetween,  ///< Sample the event track only between PrevSamplePos and SamplePos
  LoopAtEnd,    ///< Sample the event track between PrevSamplePos and End, then Start and SamplePos
  LoopAtStart,  ///< Sample the event track between PrevSamplePos and Start, then End and SamplePos
  BounceAtEnd,  ///< Sample the event track between PrevSamplePos and End, then End and SamplePos
  BounceAtStart ///< Sample the event track between PrevSamplePos and Start, then Start and SamplePos
};

/// \brief Base class for all pose generator commands
///
/// All commands have a unique command ID with which they are referenced.
/// All commands can have zero or N other commands set as *inputs*.
/// Every type of command only accepts certain types and amount of inputs.
///
/// The pose generation graph is built by allocating commands on the graph and then setting up
/// which command is an input to which other node.
/// A command can be an input to multiple other commands. It will be evaluated only once.
struct NS_RENDERERCORE_DLL nsAnimPoseGeneratorCommand
{
  nsHybridArray<nsAnimPoseGeneratorCommandID, 4> m_Inputs;

  nsAnimPoseGeneratorCommandID GetCommandID() const { return m_CommandID; }
  nsAnimPoseGeneratorCommandType GetType() const { return m_Type; }

private:
  friend class nsAnimPoseGenerator;

  bool m_bExecuted = false;
  nsAnimPoseGeneratorCommandID m_CommandID = nsInvalidIndex;
  nsAnimPoseGeneratorCommandType m_Type = nsAnimPoseGeneratorCommandType::Invalid;
};

/// \brief Returns the rest pose (also often called 'bind pose').
///
/// The command has to be added as an input to one of
/// * nsAnimPoseGeneratorCommandCombinePoses
/// * nsAnimPoseGeneratorCommandLocalToModelPose
struct NS_RENDERERCORE_DLL nsAnimPoseGeneratorCommandRestPose final : public nsAnimPoseGeneratorCommand
{
private:
  friend class nsAnimPoseGenerator;

  nsAnimPoseGeneratorLocalPoseID m_LocalPoseOutput = nsInvalidIndex;
};

/// \brief Samples an animation clip at a given time and optionally also its event track.
///
/// The command has to be added as an input to one of
/// * nsAnimPoseGeneratorCommandCombinePoses
/// * nsAnimPoseGeneratorCommandLocalToModelPose
///
/// If the event track shall be sampled as well, event messages are sent to the nsGameObject for which the pose is generated.
///
/// This command can optionally have input commands of type nsAnimPoseGeneratorCommandSampleEventTrack.
struct NS_RENDERERCORE_DLL nsAnimPoseGeneratorCommandSampleTrack final : public nsAnimPoseGeneratorCommand
{
  nsAnimationClipResourceHandle m_hAnimationClip;
  float m_fNormalizedSamplePos;
  float m_fPreviousNormalizedSamplePos;

  nsAnimPoseEventTrackSampleMode m_EventSampling = nsAnimPoseEventTrackSampleMode::None;

private:
  friend class nsAnimPoseGenerator;

  bool m_bAdditive = false;
  nsUInt32 m_uiUniqueID = 0;
  nsAnimPoseGeneratorLocalPoseID m_LocalPoseOutput = nsInvalidIndex;
};

/// \brief Combines all the local space poses that are given as input into one local pose.
///
/// The input commands must be of type
/// * nsAnimPoseGeneratorCommandSampleTrack
/// * nsAnimPoseGeneratorCommandCombinePoses
/// * nsAnimPoseGeneratorCommandRestPose
///
/// Every input pose gets both an overall weight, as well as optionally a per-bone weight mask.
/// If a per-bone mask is used, the respective input pose will only affect those bones.
struct NS_RENDERERCORE_DLL nsAnimPoseGeneratorCommandCombinePoses final : public nsAnimPoseGeneratorCommand
{
  nsHybridArray<float, 4> m_InputWeights;
  nsHybridArray<nsArrayPtr<const ozz::math::SimdFloat4>, 4> m_InputBoneWeights;

private:
  friend class nsAnimPoseGenerator;

  nsAnimPoseGeneratorLocalPoseID m_LocalPoseOutput = nsInvalidIndex;
};

/// \brief Samples the event track of an animation clip but doesn't generate an animation pose.
///
/// Commands of this type can be added as inputs to commands of type
/// * nsAnimPoseGeneratorCommandSampleTrack
/// * nsAnimPoseGeneratorCommandSampleEventTrack
///
/// They are used to sample event tracks only.
struct NS_RENDERERCORE_DLL nsAnimPoseGeneratorCommandSampleEventTrack final : public nsAnimPoseGeneratorCommand
{
  nsAnimationClipResourceHandle m_hAnimationClip;
  float m_fNormalizedSamplePos;
  float m_fPreviousNormalizedSamplePos;

  nsAnimPoseEventTrackSampleMode m_EventSampling = nsAnimPoseEventTrackSampleMode::None;

private:
  friend class nsAnimPoseGenerator;

  nsUInt32 m_uiUniqueID = 0;
};

/// \brief Base class for commands that produce or update a model pose.
struct NS_RENDERERCORE_DLL nsAnimPoseGeneratorCommandModelPose : public nsAnimPoseGeneratorCommand
{
protected:
  friend class nsAnimPoseGenerator;

  nsAnimPoseGeneratorModelPoseID m_ModelPoseOutput = nsInvalidIndex;
  nsAnimPoseGeneratorLocalPoseID m_LocalPoseOutput = nsInvalidIndex;
};

/// \brief Accepts a single input in local space and converts it to model space.
///
/// The input command must be of type
/// * nsAnimPoseGeneratorCommandSampleTrack
/// * nsAnimPoseGeneratorCommandCombinePoses
/// * nsAnimPoseGeneratorCommandRestPose
struct NS_RENDERERCORE_DLL nsAnimPoseGeneratorCommandLocalToModelPose final : public nsAnimPoseGeneratorCommandModelPose
{
  nsGameObject* m_pSendLocalPoseMsgTo = nullptr;
};

/// \brief Accepts a single input in model space and applies aim IK (inverse kinematics) on it. Updates the model pose in place.
///
/// The input command must be of type
/// * nsAnimPoseGeneratorCommandLocalToModelPose
/// * nsAnimPoseGeneratorCommandAimIK
/// * nsAnimPoseGeneratorCommandTwoBoneIK
struct NS_RENDERERCORE_DLL nsAnimPoseGeneratorCommandAimIK final : public nsAnimPoseGeneratorCommandModelPose
{
  nsVec3 m_vTargetPosition;                                     ///< The position for the bone to point at. Must be in model space of the skeleton, ie even the m_RootTransform must have been removed.
  nsUInt16 m_uiJointIdx;                                        ///< The index of the joint to aim.
  nsUInt16 m_uiRecalcModelPoseToJointIdx = nsInvalidJointIndex; ///< Optimization hint to prevent unnecessary recalculation of model poses for joints that get updated later again.
  float m_fWeight = 1.0f;                                       ///< Factor between 0 and 1 for how much to apply the IK.
  nsVec3 m_vForwardVector = nsVec3::MakeAxisX();                ///< The local joint direction that should aim at the target. Typically there is a convention to use +X, +Y or +Z.
  nsVec3 m_vUpVector = nsVec3::MakeAxisZ();                     ///< The local joint direction that should point towards the pole vector. Must be orthogonal to the forward vector.
  nsVec3 m_vPoleVector = nsVec3::MakeAxisY();                   ///< In the same space as the target position, a position that the up vector of the joint should (roughly) point towards. Used to have bones point into the right direction, for example to make an elbow point properly sideways.
};

/// \brief Accepts a single input in model space and applies two-bone IK (inverse kinematics) on it. Updates the model pose in place.
///
/// The input command must be of type
/// * nsAnimPoseGeneratorCommandLocalToModelPose
/// * nsAnimPoseGeneratorCommandAimIK
/// * nsAnimPoseGeneratorCommandTwoBoneIK
struct NS_RENDERERCORE_DLL nsAnimPoseGeneratorCommandTwoBoneIK final : public nsAnimPoseGeneratorCommandModelPose
{
  nsVec3 m_vTargetPosition;                                     ///< The position for the 'end' joint to try to reach. Must be in model space of the skeleton, ie even the m_RootTransform must have been removed.
  nsUInt16 m_uiJointIdxStart;                                   ///< Index of the top joint in a chain of three joints. The IK result may freely rotate around this joint into any (unnatural direction).
  nsUInt16 m_uiJointIdxMiddle;                                  ///< Index of the middle joint in a chain of three joints. The IK result will bend at this joint around the joint local mid-axis.
  nsUInt16 m_uiJointIdxEnd;                                     ///< Index of the end joint that is supposed to reach the target.
  nsUInt16 m_uiRecalcModelPoseToJointIdx = nsInvalidJointIndex; ///< Optimization hint to prevent unnecessary recalculation of model poses for joints that get updated later again.
  nsVec3 m_vMidAxis = nsVec3::MakeAxisZ();                      ///< The local joint direction around which to bend the middle joint. Typically there is a convention to use +X, +Y or +Z to bend around.
  nsVec3 m_vPoleVector = nsVec3::MakeAxisY();                   ///< In the same space as the target position, a position that the middle joint should (roughly) point towards. Used to have bones point into the right direction, for example to make a knee point properly forwards.
  float m_fWeight = 1.0f;                                       ///< Factor between 0 and 1 for how much to apply the IK.
  float m_fSoften = 1.0f;                                       ///< Factor between 0 and 1. See OZZ for details.
  nsAngle m_TwistAngle;                                         ///< After IK how much to rotate the chain. Seems to be redundant with the pole vector. See OZZ for details.
};

/// \brief Low-level infrastructure to generate animation poses from animation clips and other inputs.
///
/// Even though instances of this class should be reused over frames, it is assumed that all commands are recreated every frame, to build a new pose.
/// Some commands take predecessor commands as inputs to combine. If a command turns out not be actually needed, it won't be evaluated.
class NS_RENDERERCORE_DLL nsAnimPoseGenerator final
{
public:
  nsAnimPoseGenerator();
  ~nsAnimPoseGenerator();

  void Reset(const nsSkeletonResource* pSkeleton, nsGameObject* pTarget);

  const nsSkeletonResource* GetSkeleton() const { return m_pSkeleton; }
  const nsGameObject* GetTargetObject() const { return m_pTargetGameObject; }

  nsAnimPoseGeneratorCommandSampleTrack& AllocCommandSampleTrack(nsUInt32 uiDeterministicID);
  nsAnimPoseGeneratorCommandRestPose& AllocCommandRestPose();
  nsAnimPoseGeneratorCommandCombinePoses& AllocCommandCombinePoses();
  nsAnimPoseGeneratorCommandLocalToModelPose& AllocCommandLocalToModelPose();
  nsAnimPoseGeneratorCommandSampleEventTrack& AllocCommandSampleEventTrack();
  nsAnimPoseGeneratorCommandAimIK& AllocCommandAimIK();
  nsAnimPoseGeneratorCommandTwoBoneIK& AllocCommandTwoBoneIK();

  const nsAnimPoseGeneratorCommand& GetCommand(nsAnimPoseGeneratorCommandID id) const;
  nsAnimPoseGeneratorCommand& GetCommand(nsAnimPoseGeneratorCommandID id);

  void UpdatePose(bool bRequestExternalPoseGeneration);

  nsArrayPtr<nsMat4> GetCurrentPose() const { return m_OutputPose; }

  void SetFinalCommand(nsAnimPoseGeneratorCommandID cmdId) { m_FinalCommand = cmdId; }
  nsAnimPoseGeneratorCommandID GetFinalCommand() const { return m_FinalCommand; }

private:
  void Validate() const;

  void Execute(nsAnimPoseGeneratorCommand& cmd);
  void ExecuteCmd(nsAnimPoseGeneratorCommandSampleTrack& cmd);
  void ExecuteCmd(nsAnimPoseGeneratorCommandRestPose& cmd);
  void ExecuteCmd(nsAnimPoseGeneratorCommandCombinePoses& cmd);
  void ExecuteCmd(nsAnimPoseGeneratorCommandLocalToModelPose& cmd);
  void ExecuteCmd(nsAnimPoseGeneratorCommandSampleEventTrack& cmd);
  void ExecuteCmd(nsAnimPoseGeneratorCommandAimIK& cmd);
  void ExecuteCmd(nsAnimPoseGeneratorCommandTwoBoneIK& cmd);
  void SampleEventTrack(const nsAnimationClipResource* pResource, nsAnimPoseEventTrackSampleMode mode, float fPrevPos, float fCurPos);

  nsArrayPtr<ozz::math::SoaTransform> AcquireLocalPoseTransforms(nsAnimPoseGeneratorLocalPoseID id);
  nsArrayPtr<nsMat4> AcquireModelPoseTransforms(nsAnimPoseGeneratorModelPoseID id);

  nsGameObject* m_pTargetGameObject = nullptr;
  const nsSkeletonResource* m_pSkeleton = nullptr;

  nsArrayPtr<nsMat4> m_OutputPose;

  nsAnimPoseGeneratorLocalPoseID m_LocalPoseCounter = 0;
  nsAnimPoseGeneratorModelPoseID m_ModelPoseCounter = 0;

  nsAnimPoseGeneratorCommandID m_FinalCommand = 0;

  nsHybridArray<nsArrayPtr<ozz::math::SoaTransform>, 8> m_UsedLocalTransforms;
  nsHybridArray<nsDynamicArray<nsMat4, nsAlignedAllocatorWrapper>, 2> m_UsedModelTransforms;

  nsHybridArray<nsAnimPoseGeneratorCommandSampleTrack, 4> m_CommandsSampleTrack;
  nsHybridArray<nsAnimPoseGeneratorCommandRestPose, 1> m_CommandsRestPose;
  nsHybridArray<nsAnimPoseGeneratorCommandCombinePoses, 1> m_CommandsCombinePoses;
  nsHybridArray<nsAnimPoseGeneratorCommandLocalToModelPose, 1> m_CommandsLocalToModelPose;
  nsHybridArray<nsAnimPoseGeneratorCommandSampleEventTrack, 2> m_CommandsSampleEventTrack;
  nsHybridArray<nsAnimPoseGeneratorCommandAimIK, 2> m_CommandsAimIK;
  nsHybridArray<nsAnimPoseGeneratorCommandTwoBoneIK, 2> m_CommandsTwoBoneIK;

  nsArrayMap<nsUInt32, ozz::animation::SamplingJob::Context*> m_SamplingCaches;
};

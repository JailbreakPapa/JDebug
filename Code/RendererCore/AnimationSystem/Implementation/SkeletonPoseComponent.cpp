#include <RendererCore/RendererCorePCH.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/AnimationSystem/AnimationPose.h>
#include <RendererCore/AnimationSystem/SkeletonPoseComponent.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/soa_transform.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_ENUM(nsSkeletonPoseMode, 1)
  NS_ENUM_CONSTANTS(nsSkeletonPoseMode::CustomPose, nsSkeletonPoseMode::RestPose, nsSkeletonPoseMode::Disabled)
NS_END_STATIC_REFLECTED_ENUM;

NS_BEGIN_COMPONENT_TYPE(nsSkeletonPoseComponent, 4, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Skeleton", GetSkeletonFile, SetSkeletonFile)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Mesh_Skeleton")),
    NS_ENUM_ACCESSOR_PROPERTY("Mode", nsSkeletonPoseMode, GetPoseMode, SetPoseMode),
    NS_MEMBER_PROPERTY("EditBones", m_fDummy),
    NS_MAP_ACCESSOR_PROPERTY("Bones", GetBones, GetBone, SetBone, RemoveBone)->AddAttributes(new nsExposedParametersAttribute("Skeleton"), new nsContainerAttribute(false, true, false)),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Animation"),
    new nsBoneManipulatorAttribute("Bones", "EditBones"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsSkeletonPoseComponent::nsSkeletonPoseComponent() = default;
nsSkeletonPoseComponent::~nsSkeletonPoseComponent() = default;

void nsSkeletonPoseComponent::Update()
{
  if (m_uiResendPose == 0)
    return;

  if (--m_uiResendPose > 0)
  {
    static_cast<nsSkeletonPoseComponentManager*>(GetOwningManager())->EnqueueUpdate(GetHandle());
  }

  if (m_PoseMode == nsSkeletonPoseMode::RestPose)
  {
    SendRestPose();
    return;
  }

  if (m_PoseMode == nsSkeletonPoseMode::CustomPose)
  {
    SendCustomPose();
    return;
  }
}

void nsSkeletonPoseComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);

  auto& s = inout_stream.GetStream();

  s << m_hSkeleton;
  s << m_PoseMode;

  m_Bones.Sort();
  nsUInt16 numBones = static_cast<nsUInt16>(m_Bones.GetCount());
  s << numBones;

  for (nsUInt16 i = 0; i < numBones; ++i)
  {
    s << m_Bones.GetKey(i);
    s << m_Bones.GetValue(i).m_sName;
    s << m_Bones.GetValue(i).m_sParent;
    s << m_Bones.GetValue(i).m_Transform;
  }
}

void nsSkeletonPoseComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  // const nsUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());

  auto& s = inout_stream.GetStream();

  s >> m_hSkeleton;
  s >> m_PoseMode;

  nsHashedString sKey;
  nsExposedBone bone;

  nsUInt16 numBones = 0;
  s >> numBones;
  m_Bones.Reserve(numBones);

  for (nsUInt16 i = 0; i < numBones; ++i)
  {
    s >> sKey;
    s >> bone.m_sName;
    s >> bone.m_sParent;
    s >> bone.m_Transform;

    m_Bones[sKey] = bone;
  }
  ResendPose();
}

void nsSkeletonPoseComponent::OnActivated()
{
  SUPER::OnActivated();

  ResendPose();
}

void nsSkeletonPoseComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();

  ResendPose();
}

void nsSkeletonPoseComponent::SetSkeletonFile(const char* szFile)
{
  nsSkeletonResourceHandle hResource;

  if (!nsStringUtils::IsNullOrEmpty(szFile))
  {
    hResource = nsResourceManager::LoadResource<nsSkeletonResource>(szFile);
  }

  SetSkeleton(hResource);
}

const char* nsSkeletonPoseComponent::GetSkeletonFile() const
{
  if (!m_hSkeleton.IsValid())
    return "";

  return m_hSkeleton.GetResourceID();
}

void nsSkeletonPoseComponent::SetSkeleton(const nsSkeletonResourceHandle& hResource)
{
  if (m_hSkeleton != hResource)
  {
    m_hSkeleton = hResource;
    ResendPose();
  }
}

void nsSkeletonPoseComponent::SetPoseMode(nsEnum<nsSkeletonPoseMode> mode)
{
  m_PoseMode = mode;
  ResendPose();
}

void nsSkeletonPoseComponent::ResendPose()
{
  if (m_uiResendPose == 2)
    return;

  m_uiResendPose = 2;
  static_cast<nsSkeletonPoseComponentManager*>(GetOwningManager())->EnqueueUpdate(GetHandle());
}

const nsRangeView<const char*, nsUInt32> nsSkeletonPoseComponent::GetBones() const
{
  return nsRangeView<const char*, nsUInt32>([]() -> nsUInt32
    { return 0; },
    [this]() -> nsUInt32
    { return m_Bones.GetCount(); },
    [](nsUInt32& ref_uiIt)
    { ++ref_uiIt; },
    [this](const nsUInt32& uiIt) -> const char*
    { return m_Bones.GetKey(uiIt).GetString().GetData(); });
}

void nsSkeletonPoseComponent::SetBone(const char* szKey, const nsVariant& value)
{
  nsHashedString hs;
  hs.Assign(szKey);

  if (value.GetReflectedType() == nsGetStaticRTTI<nsExposedBone>())
  {
    m_Bones[hs] = *reinterpret_cast<const nsExposedBone*>(value.GetData());
  }

  // TODO
  // if (IsActiveAndInitialized())
  //{
  //  // only add to update list, if not yet activated,
  //  // since OnActivate will do the instantiation anyway
  //  GetWorld()->GetComponentManager<nsPrefabReferenceComponentManager>()->AddToUpdateList(this);
  //}
  ResendPose();
}

void nsSkeletonPoseComponent::RemoveBone(const char* szKey)
{
  if (m_Bones.RemoveAndCopy(nsTempHashedString(szKey)))
  {
    // TODO
    // if (IsActiveAndInitialized())
    //{
    //  // only add to update list, if not yet activated,
    //  // since OnActivate will do the instantiation anyway
    //  GetWorld()->GetComponentManager<nsPrefabReferenceComponentManager>()->AddToUpdateList(this);
    //}

    ResendPose();
  }
}

bool nsSkeletonPoseComponent::GetBone(const char* szKey, nsVariant& out_value) const
{
  nsUInt32 it = m_Bones.Find(szKey);

  if (it == nsInvalidIndex)
    return false;

  out_value.CopyTypedObject(&m_Bones.GetValue(it), nsGetStaticRTTI<nsExposedBone>());
  return true;
}

void nsSkeletonPoseComponent::SendRestPose()
{
  if (!m_hSkeleton.IsValid())
    return;

  nsResourceLock<nsSkeletonResource> pSkeleton(m_hSkeleton, nsResourceAcquireMode::BlockTillLoaded_NeverFail);
  if (pSkeleton.GetAcquireResult() != nsResourceAcquireResult::Final)
    return;

  const auto& desc = pSkeleton->GetDescriptor();
  const auto& skel = desc.m_Skeleton;

  if (skel.GetJointCount() == 0)
    return;

  nsHybridArray<nsMat4, 32> finalTransforms(nsFrameAllocator::GetCurrentAllocator());
  finalTransforms.SetCountUninitialized(skel.GetJointCount());

  {
    ozz::animation::LocalToModelJob job;
    job.input = skel.GetOzzSkeleton().joint_rest_poses();
    job.output = ozz::span<ozz::math::Float4x4>(reinterpret_cast<ozz::math::Float4x4*>(finalTransforms.GetData()), finalTransforms.GetCount());
    job.skeleton = &skel.GetOzzSkeleton();
    job.Run();
  }

  nsMsgAnimationPoseUpdated msg;
  msg.m_pRootTransform = &desc.m_RootTransform;
  msg.m_pSkeleton = &skel;
  msg.m_ModelTransforms = finalTransforms;

  GetOwner()->SendMessage(msg);

  if (msg.m_bContinueAnimating == false)
    m_PoseMode = nsSkeletonPoseMode::Disabled;
}

void nsSkeletonPoseComponent::SendCustomPose()
{
  if (!m_hSkeleton.IsValid())
    return;

  nsResourceLock<nsSkeletonResource> pSkeleton(m_hSkeleton, nsResourceAcquireMode::BlockTillLoaded);
  const auto& desc = pSkeleton->GetDescriptor();
  const auto& skel = desc.m_Skeleton;

  nsHybridArray<nsMat4, 32> finalTransforms(nsFrameAllocator::GetCurrentAllocator());
  finalTransforms.SetCountUninitialized(skel.GetJointCount());

  for (nsUInt32 i = 0; i < finalTransforms.GetCount(); ++i)
  {
    finalTransforms[i].SetIdentity();
  }

  ozz::vector<ozz::math::SoaTransform> ozzLocalTransforms;
  ozzLocalTransforms.resize((skel.GetJointCount() + 3) / 4);

  auto restPoses = skel.GetOzzSkeleton().joint_rest_poses();

  // initialize the skeleton with the rest pose
  for (nsUInt32 i = 0; i < ozzLocalTransforms.size(); ++i)
  {
    ozzLocalTransforms[i] = restPoses[i];
  }

  for (const auto& boneIt : m_Bones)
  {
    const nsUInt16 uiBone = skel.FindJointByName(boneIt.key);
    if (uiBone == nsInvalidJointIndex)
      continue;

    const nsExposedBone& thisBone = boneIt.value;

    // this can happen when the property was reverted
    if (thisBone.m_sName.IsEmpty() || thisBone.m_sParent.IsEmpty())
      continue;

    NS_ASSERT_DEBUG(!thisBone.m_Transform.m_qRotation.IsNaN(), "Invalid bone transform in pose component");

    const nsQuat& boneRot = thisBone.m_Transform.m_qRotation;

    const nsUInt32 idx0 = uiBone / 4;
    const nsUInt32 idx1 = uiBone % 4;

    ozz::math::SoaQuaternion& q = ozzLocalTransforms[idx0].rotation;
    reinterpret_cast<float*>(&q.x)[idx1] = boneRot.x;
    reinterpret_cast<float*>(&q.y)[idx1] = boneRot.y;
    reinterpret_cast<float*>(&q.z)[idx1] = boneRot.z;
    reinterpret_cast<float*>(&q.w)[idx1] = boneRot.w;
  }

  ozz::animation::LocalToModelJob job;
  job.input = ozz::span<const ozz::math::SoaTransform>(ozzLocalTransforms.data(), ozzLocalTransforms.size());
  job.output = ozz::span<ozz::math::Float4x4>(reinterpret_cast<ozz::math::Float4x4*>(finalTransforms.GetData()), finalTransforms.GetCount());
  job.skeleton = &skel.GetOzzSkeleton();
  NS_ASSERT_DEBUG(job.Validate(), "");
  job.Run();


  nsMsgAnimationPoseUpdated msg;
  msg.m_pRootTransform = &desc.m_RootTransform;
  msg.m_pSkeleton = &skel;
  msg.m_ModelTransforms = finalTransforms;

  GetOwner()->SendMessage(msg);

  if (msg.m_bContinueAnimating == false)
    m_PoseMode = nsSkeletonPoseMode::Disabled;
}

//////////////////////////////////////////////////////////////////////////

void nsSkeletonPoseComponentManager::Update(const nsWorldModule::UpdateContext& context)
{
  nsDeque<nsComponentHandle> requireUpdate;

  {
    NS_LOCK(m_Mutex);
    requireUpdate.Swap(m_RequireUpdate);
  }

  for (const auto& hComp : requireUpdate)
  {
    nsSkeletonPoseComponent* pComp = nullptr;
    if (!TryGetComponent(hComp, pComp) || !pComp->IsActiveAndInitialized())
      continue;

    pComp->Update();
  }
}

void nsSkeletonPoseComponentManager::EnqueueUpdate(nsComponentHandle hComponent)
{
  NS_LOCK(m_Mutex);

  if (m_RequireUpdate.IndexOf(hComponent) != nsInvalidIndex)
    return;

  m_RequireUpdate.PushBack(hComponent);
}

void nsSkeletonPoseComponentManager::Initialize()
{
  SUPER::Initialize();

  nsWorldModule::UpdateFunctionDesc desc = NS_CREATE_MODULE_UPDATE_FUNCTION_DESC(nsSkeletonPoseComponentManager::Update, this);
  desc.m_Phase = UpdateFunctionDesc::Phase::PreAsync;

  RegisterUpdateFunction(desc);
}


NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_Implementation_SkeletonPoseComponent);

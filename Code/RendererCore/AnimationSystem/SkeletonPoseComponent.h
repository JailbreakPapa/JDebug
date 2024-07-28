#pragma once

#include <Core/World/ComponentManager.h>
#include <Foundation/Containers/ArrayMap.h>
#include <Foundation/Types/RangeView.h>
#include <RendererCore/AnimationSystem/EditableSkeleton.h>
#include <RendererCore/AnimationSystem/SkeletonResource.h>

class nsSkeletonPoseComponentManager : public nsComponentManager<class nsSkeletonPoseComponent, nsBlockStorageType::Compact>
{
public:
  using SUPER = nsComponentManager<nsSkeletonPoseComponent, nsBlockStorageType::Compact>;

  nsSkeletonPoseComponentManager(nsWorld* pWorld)
    : SUPER(pWorld)
  {
  }

  void Update(const nsWorldModule::UpdateContext& context);
  void EnqueueUpdate(nsComponentHandle hComponent);

private:
  mutable nsMutex m_Mutex;
  nsDeque<nsComponentHandle> m_RequireUpdate;

protected:
  virtual void Initialize() override;
};

//////////////////////////////////////////////////////////////////////////

/// \brief Which pose to apply to an animated mesh.
struct nsSkeletonPoseMode
{
  using StorageType = nsUInt8;

  enum Enum
  {
    CustomPose, ///< Set a custom pose on the mesh.
    RestPose,   ///< Set the rest pose (bind pose) on the mesh.
    Disabled,   ///< Don't set any pose.
    Default = CustomPose
  };
};

NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsSkeletonPoseMode);

/// \brief Used in conjunction with an nsAnimatedMeshComponent to set a specific pose for the animated mesh.
///
/// This component is used to set one, static pose for an animated mesh. The pose is applied once at startup.
/// This can be used to either just pose a mesh in a certain way, or to set a start pose that is then used
/// by other systems, for example a ragdoll component, to generate further poses.
///
/// The component needs to be attached to the same game object where the animated mesh component is attached.
class NS_RENDERERCORE_DLL nsSkeletonPoseComponent : public nsComponent
{
  NS_DECLARE_COMPONENT_TYPE(nsSkeletonPoseComponent, nsComponent, nsSkeletonPoseComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // nsComponent

public:
  virtual void SerializeComponent(nsWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(nsWorldReader& inout_stream) override;

protected:
  virtual void OnActivated() override;
  virtual void OnSimulationStarted() override;

  //////////////////////////////////////////////////////////////////////////
  // nsSkeletonPoseComponent

public:
  nsSkeletonPoseComponent();
  ~nsSkeletonPoseComponent();

  /// \brief Sets the asset GUID or path for the nsSkeletonResource to use.
  void SetSkeletonFile(const char* szFile); // [ property ]
  const char* GetSkeletonFile() const;      // [ property ]

  /// \brief Sets the nsSkeletonResource to use.
  void SetSkeleton(const nsSkeletonResourceHandle& hResource);
  const nsSkeletonResourceHandle& GetSkeleton() const { return m_hSkeleton; }

  /// \brief Configures which pose to apply to the animated mesh.
  void SetPoseMode(nsEnum<nsSkeletonPoseMode> mode);
  nsEnum<nsSkeletonPoseMode> GetPoseMode() const { return m_PoseMode; }

  const nsRangeView<const char*, nsUInt32> GetBones() const;   // [ property ] (exposed bones)
  void SetBone(const char* szKey, const nsVariant& value);     // [ property ] (exposed bones)
  void RemoveBone(const char* szKey);                          // [ property ] (exposed bones)
  bool GetBone(const char* szKey, nsVariant& out_value) const; // [ property ] (exposed bones)

  /// \brief Instructs the component to apply the pose to the animated mesh again.
  void ResendPose();

protected:
  void Update();
  void SendRestPose();
  void SendCustomPose();

  float m_fDummy = 0;
  nsUInt8 m_uiResendPose = 0;
  nsSkeletonResourceHandle m_hSkeleton;
  nsArrayMap<nsHashedString, nsExposedBone> m_Bones; // [ property ]
  nsEnum<nsSkeletonPoseMode> m_PoseMode;             // [ property ]
};

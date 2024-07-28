#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <RendererCore/Meshes/MeshComponentBase.h>

struct nsPerInstanceData;
struct nsRenderWorldRenderEvent;
class nsInstancedMeshComponent;
struct nsMsgExtractGeometry;
class nsStreamWriter;
class nsStreamReader;

struct NS_RENDERERCORE_DLL nsMeshInstanceData
{
  void SetLocalPosition(nsVec3 vPosition);
  nsVec3 GetLocalPosition() const;

  void SetLocalRotation(nsQuat qRotation);
  nsQuat GetLocalRotation() const;

  void SetLocalScaling(nsVec3 vScaling);
  nsVec3 GetLocalScaling() const;

  nsResult Serialize(nsStreamWriter& ref_writer) const;
  nsResult Deserialize(nsStreamReader& ref_reader);

  nsTransform m_transform;

  nsColor m_color;
};

NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsMeshInstanceData);

//////////////////////////////////////////////////////////////////////////

class NS_RENDERERCORE_DLL nsInstancedMeshRenderData : public nsMeshRenderData
{
  NS_ADD_DYNAMIC_REFLECTION(nsInstancedMeshRenderData, nsMeshRenderData);

public:
  virtual void FillBatchIdAndSortingKey() override;

  nsInstanceData* m_pExplicitInstanceData = nullptr;
  nsUInt32 m_uiExplicitInstanceCount = 0;
};

//////////////////////////////////////////////////////////////////////////

class NS_RENDERERCORE_DLL nsInstancedMeshComponentManager : public nsComponentManager<class nsInstancedMeshComponent, nsBlockStorageType::Compact>
{
public:
  using SUPER = nsComponentManager<nsInstancedMeshComponent, nsBlockStorageType::Compact>;

  nsInstancedMeshComponentManager(nsWorld* pWorld);

  void EnqueueUpdate(const nsInstancedMeshComponent* pComponent) const;

private:
  struct ComponentToUpdate
  {
    nsComponentHandle m_hComponent;
    nsArrayPtr<nsPerInstanceData> m_InstanceData;
  };

  mutable nsMutex m_Mutex;
  mutable nsDeque<ComponentToUpdate> m_RequireUpdate;

protected:
  void OnRenderEvent(const nsRenderWorldRenderEvent& e);

  virtual void Initialize() override;
  virtual void Deinitialize() override;
};

/// \brief Renders multiple instances of the same mesh.
///
/// This is used as an optimization to render many instances of the same (usually small mesh).
/// For example, if you need to render 1000 pieces of grass in a small area,
/// instead of creating 1000 game objects each with a mesh component,
/// it is more efficient to create one game object with an instanced mesh component and give it the locations of the 1000 pieces.
/// Due to the small area, there is no benefit in culling the instances separately.
///
/// However, editing instanced mesh components isn't very convenient, so usually this component would be created and configured
/// in code, rather than by hand in the editor. For example a procedural plant placement system could use this.
class NS_RENDERERCORE_DLL nsInstancedMeshComponent : public nsMeshComponentBase
{
  NS_DECLARE_COMPONENT_TYPE(nsInstancedMeshComponent, nsMeshComponentBase, nsInstancedMeshComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // nsComponent

public:
  virtual void SerializeComponent(nsWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(nsWorldReader& inout_stream) override;

protected:
  virtual void OnActivated() override;
  virtual void OnDeactivated() override;


  //////////////////////////////////////////////////////////////////////////
  // nsRenderComponent

public:
  virtual nsResult GetLocalBounds(nsBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, nsMsgUpdateLocalBounds& ref_msg) override;

  //////////////////////////////////////////////////////////////////////////
  // nsMeshComponentBase

protected:
  virtual nsMeshRenderData* CreateRenderData() const override;

  //////////////////////////////////////////////////////////////////////////
  // nsInstancedMeshComponent

public:
  nsInstancedMeshComponent();
  ~nsInstancedMeshComponent();

  /// \brief Extracts the render geometry for export etc.
  void OnMsgExtractGeometry(nsMsgExtractGeometry& ref_msg); // [ msg handler ]

protected:
  void OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const;

  nsUInt32 Instances_GetCount() const;                                 // [ property ]
  nsMeshInstanceData Instances_GetValue(nsUInt32 uiIndex) const;       // [ property ]
  void Instances_SetValue(nsUInt32 uiIndex, nsMeshInstanceData value); // [ property ]
  void Instances_Insert(nsUInt32 uiIndex, nsMeshInstanceData value);   // [ property ]
  void Instances_Remove(nsUInt32 uiIndex);                             // [ property ]

  nsArrayPtr<nsPerInstanceData> GetInstanceData() const;

  // Unpacked, reflected instance data for editing and ease of access
  nsDynamicArray<nsMeshInstanceData> m_RawInstancedData;

  nsInstanceData* m_pExplicitInstanceData = nullptr;

  mutable nsUInt64 m_uiEnqueuedFrame = nsUInt64(-1);
};

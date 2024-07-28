#pragma once

#include <Core/World/World.h>
#include <RendererCore/Meshes/MeshComponentBase.h>

using nsLodMeshComponentManager = nsComponentManager<class nsLodMeshComponent, nsBlockStorageType::Compact>;

struct nsLodMeshLod
{
  const char* GetMeshFile() const;
  void SetMeshFile(const char* szFile);

  nsMeshResourceHandle m_hMesh;
  float m_fThreshold;
};

NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsLodMeshLod);

/// \brief Renders one of several level-of-detail meshes depending on the distance to the camera.
///
/// This component is very similar to the nsLodComponent, please read it's description for details.
/// The difference is, that this component doesn't switch child object on and off, but rather only selects between different render-meshes.
/// As such there is less performance impact for switching between meshes and also the memory overhead for storing LOD information is smaller.
/// If it is only desired to switch between meshes, it is also more convenient to work with just a single component.
///
/// The component does not allow to place the LOD meshes differently, they all need to have the same origin.
/// Compared with the regular nsMeshComponent there is also no way to override the used materials, since each LOD mesh may use different materials.
class NS_RENDERERCORE_DLL nsLodMeshComponent : public nsRenderComponent
{
  NS_DECLARE_COMPONENT_TYPE(nsLodMeshComponent, nsRenderComponent, nsLodMeshComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // nsComponent

public:
  virtual void SerializeComponent(nsWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(nsWorldReader& inout_stream) override;

  //////////////////////////////////////////////////////////////////////////
  // nsRenderComponent

public:
  virtual nsResult GetLocalBounds(nsBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, nsMsgUpdateLocalBounds& ref_msg) override;

  //////////////////////////////////////////////////////////////////////////
  // nsLodMeshComponent

public:
  nsLodMeshComponent();
  ~nsLodMeshComponent();

  /// \brief An additional tint color passed to the renderer to modify the mesh.
  void SetColor(const nsColor& color); // [ property ]
  const nsColor& GetColor() const;     // [ property ]

  /// \brief An additional vec4 passed to the renderer that can be used by custom material shaders for effects.
  void SetCustomData(const nsVec4& vData); // [ property ]
  const nsVec4& GetCustomData() const;     // [ property ]

  /// \brief The sorting depth offset allows to tweak the order in which this mesh is rendered relative to other meshes.
  ///
  /// This is mainly useful for transparent objects to render them before or after other meshes.
  void SetSortingDepthOffset(float fOffset); // [ property ]
  float GetSortingDepthOffset() const;       // [ property ]

  /// \brief Enables text output to show the current coverage value and selected LOD.
  void SetShowDebugInfo(bool bShow); // [ property ]
  bool GetShowDebugInfo() const;     // [ property ]

  /// \brief Disabling the LOD range overlap functionality can make it easier to determine the desired coverage thresholds.
  void SetOverlapRanges(bool bOverlap);                 // [ property ]
  bool GetOverlapRanges() const;                        // [ property ]

  void OnMsgSetColor(nsMsgSetColor& ref_msg);           // [ msg handler ]
  void OnMsgSetCustomData(nsMsgSetCustomData& ref_msg); // [ msg handler ]

protected:
  virtual nsMeshRenderData* CreateRenderData() const;

  void UpdateSelectedLod(const nsView& view) const;
  void OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const;

  mutable nsInt32 m_iCurLod = 0;
  nsDynamicArray<nsLodMeshLod> m_Meshes;
  nsColor m_Color = nsColor::White;
  nsVec4 m_vCustomData = nsVec4(0, 1, 0, 1);
  float m_fSortingDepthOffset = 0.0f;
  nsVec3 m_vBoundsOffset = nsVec3::MakeZero();
  float m_fBoundsRadius = 1.0f;
};

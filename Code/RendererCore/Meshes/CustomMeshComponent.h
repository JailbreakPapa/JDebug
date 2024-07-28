#pragma once

#include <RendererCore/Meshes/MeshComponentBase.h>
#include <RendererCore/Pipeline/Renderer.h>

using nsDynamicMeshBufferResourceHandle = nsTypedResourceHandle<class nsDynamicMeshBufferResource>;
using nsCustomMeshComponentManager = nsComponentManager<class nsCustomMeshComponent, nsBlockStorageType::Compact>;

/// \brief This component is used to render custom geometry.
///
/// Sometimes game code needs to build geometry on the fly to visualize dynamic things.
/// The nsDynamicMeshBufferResource is an easy to use resource to build geometry and change it frequently.
/// This component takes such a resource and takes care of rendering it.
/// The same resource can be set on multiple components to instantiate it in different locations.
class NS_RENDERERCORE_DLL nsCustomMeshComponent : public nsRenderComponent
{
  NS_DECLARE_COMPONENT_TYPE(nsCustomMeshComponent, nsRenderComponent, nsCustomMeshComponentManager);

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
  // nsCustomMeshComponent

public:
  nsCustomMeshComponent();
  ~nsCustomMeshComponent();

  /// \brief Creates a new dynamic mesh buffer.
  ///
  /// The new buffer can hold the given number of vertices and indices (either 16 bit or 32 bit).
  nsDynamicMeshBufferResourceHandle CreateMeshResource(nsGALPrimitiveTopology::Enum topology, nsUInt32 uiMaxVertices, nsUInt32 uiMaxPrimitives, nsGALIndexType::Enum indexType);

  /// \brief Returns the currently set mesh resource.
  nsDynamicMeshBufferResourceHandle GetMeshResource() const { return m_hDynamicMesh; }

  /// \brief Sets which mesh buffer to use.
  ///
  /// This can be used to have multiple nsCustomMeshComponent's reference the same mesh buffer,
  /// such that the object gets instanced in different locations.
  void SetMeshResource(const nsDynamicMeshBufferResourceHandle& hMesh);

  /// \brief Configures the component to render only a subset of the primitives in the mesh buffer.
  void SetUsePrimitiveRange(nsUInt32 uiFirstPrimitive = 0, nsUInt32 uiNumPrimitives = nsMath::MaxValue<nsUInt32>());

  /// \brief Sets the bounds that are used for culling.
  ///
  /// Note: It is very important that this is called whenever the mesh buffer is modified and the size of
  /// the mesh has changed, otherwise the object might not appear or be culled incorrectly.
  void SetBounds(const nsBoundingBoxSphere& bounds);

  /// \brief Sets the material for rendering.
  void SetMaterial(const nsMaterialResourceHandle& hMaterial);

  /// \brief Returns the material that is used for rendering.
  nsMaterialResourceHandle GetMaterial() const;

  void SetMaterialFile(const char* szMaterial); // [ property ]
  const char* GetMaterialFile() const;          // [ property ]

  /// \brief Sets the mesh instance color.
  void SetColor(const nsColor& color); // [ property ]

  /// \brief Returns the mesh instance color.
  const nsColor& GetColor() const; // [ property ]

  /// \brief An additional vec4 passed to the renderer that can be used by custom material shaders for effects.
  void SetCustomData(const nsVec4& vData);                  // [ property ]
  const nsVec4& GetCustomData() const;                      // [ property ]

  void OnMsgSetMeshMaterial(nsMsgSetMeshMaterial& ref_msg); // [ msg handler ]
  void OnMsgSetColor(nsMsgSetColor& ref_msg);               // [ msg handler ]
  void OnMsgSetCustomData(nsMsgSetCustomData& ref_msg);     // [ msg handler ]

protected:
  void OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const;

  nsMaterialResourceHandle m_hMaterial;
  nsColor m_Color = nsColor::White;
  nsVec4 m_vCustomData = nsVec4(0, 1, 0, 1);
  nsUInt32 m_uiFirstPrimitive = 0;
  nsUInt32 m_uiNumPrimitives = 0xFFFFFFFF;
  nsBoundingBoxSphere m_Bounds;

  nsDynamicMeshBufferResourceHandle m_hDynamicMesh;

  virtual void OnActivated() override;
};

/// \brief Temporary data used to feed the nsCustomMeshRenderer.
class NS_RENDERERCORE_DLL nsCustomMeshRenderData : public nsRenderData
{
  NS_ADD_DYNAMIC_REFLECTION(nsCustomMeshRenderData, nsRenderData);

public:
  virtual void FillBatchIdAndSortingKey();

  nsDynamicMeshBufferResourceHandle m_hMesh;
  nsMaterialResourceHandle m_hMaterial;
  nsColor m_Color = nsColor::White;
  nsVec4 m_vCustomData = nsVec4(0, 1, 0, 1);

  nsUInt32 m_uiFlipWinding : 1;
  nsUInt32 m_uiUniformScale : 1;

  nsUInt32 m_uiFirstPrimitive = 0;
  nsUInt32 m_uiNumPrimitives = 0xFFFFFFFF;

  nsUInt32 m_uiUniqueID = 0;
};

/// \brief A renderer that handles all nsCustomMeshRenderData.
class NS_RENDERERCORE_DLL nsCustomMeshRenderer : public nsRenderer
{
  NS_ADD_DYNAMIC_REFLECTION(nsCustomMeshRenderer, nsRenderer);
  NS_DISALLOW_COPY_AND_ASSIGN(nsCustomMeshRenderer);

public:
  nsCustomMeshRenderer();
  ~nsCustomMeshRenderer();

  virtual void GetSupportedRenderDataCategories(nsHybridArray<nsRenderData::Category, 8>& ref_categories) const override;
  virtual void GetSupportedRenderDataTypes(nsHybridArray<const nsRTTI*, 8>& ref_types) const override;
  virtual void RenderBatch(const nsRenderViewContext& renderContext, const nsRenderPipelinePass* pPass, const nsRenderDataBatch& batch) const override;
};

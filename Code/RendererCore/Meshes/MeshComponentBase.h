#pragma once

#include <Core/World/World.h>
#include <RendererCore/Components/RenderComponent.h>
#include <RendererCore/Material/MaterialResource.h>
#include <RendererCore/Meshes/MeshResource.h>
#include <RendererCore/Pipeline/RenderData.h>

struct nsMsgSetColor;
struct nsMsgSetCustomData;
struct nsInstanceData;

class NS_RENDERERCORE_DLL nsMeshRenderData : public nsRenderData
{
  NS_ADD_DYNAMIC_REFLECTION(nsMeshRenderData, nsRenderData);

public:
  virtual void FillBatchIdAndSortingKey();

  nsMeshResourceHandle m_hMesh;
  nsMaterialResourceHandle m_hMaterial;
  nsColor m_Color = nsColor::White;
  nsVec4 m_vCustomData = nsVec4(0, 1, 0, 1);

  nsUInt32 m_uiSubMeshIndex : 30;
  nsUInt32 m_uiFlipWinding : 1;
  nsUInt32 m_uiUniformScale : 1;

  nsUInt32 m_uiUniqueID = 0;

protected:
  NS_FORCE_INLINE void FillBatchIdAndSortingKeyInternal(nsUInt32 uiAdditionalBatchData)
  {
    m_uiFlipWinding = m_GlobalTransform.ContainsNegativeScale() ? 1 : 0;
    m_uiUniformScale = m_GlobalTransform.ContainsUniformScale() ? 1 : 0;

    const nsUInt32 uiMeshIDHash = nsHashingUtils::StringHashTo32(m_hMesh.GetResourceIDHash());
    const nsUInt32 uiMaterialIDHash = m_hMaterial.IsValid() ? nsHashingUtils::StringHashTo32(m_hMaterial.GetResourceIDHash()) : 0;

    // Generate batch id from mesh, material and part index.
    nsUInt32 data[] = {uiMeshIDHash, uiMaterialIDHash, m_uiSubMeshIndex, m_uiFlipWinding, uiAdditionalBatchData};
    m_uiBatchId = nsHashingUtils::xxHash32(data, sizeof(data));

    // Sort by material and then by mesh
    m_uiSortingKey = (uiMaterialIDHash << 16) | ((uiMeshIDHash + m_uiSubMeshIndex) & 0xFFFE) | m_uiFlipWinding;
  }
};

/// \brief This message is used to replace the material on a mesh.
struct NS_RENDERERCORE_DLL nsMsgSetMeshMaterial : public nsMessage
{
  NS_DECLARE_MESSAGE_TYPE(nsMsgSetMeshMaterial, nsMessage);

  void SetMaterialFile(const char* szFile);
  const char* GetMaterialFile() const;

  /// The material to be used.
  nsMaterialResourceHandle m_hMaterial;

  /// The slot on the mesh component where the material should be set.
  nsUInt32 m_uiMaterialSlot = 0xFFFFFFFFu;

  virtual void Serialize(nsStreamWriter& inout_stream) const override;
  virtual void Deserialize(nsStreamReader& inout_stream, nsUInt8 uiTypeVersion) override;
};

/// \brief Base class for components that render static or animated meshes.
class NS_RENDERERCORE_DLL nsMeshComponentBase : public nsRenderComponent
{
  NS_DECLARE_ABSTRACT_COMPONENT_TYPE(nsMeshComponentBase, nsRenderComponent);

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
  // nsRenderMeshComponent

public:
  nsMeshComponentBase();
  ~nsMeshComponentBase();

  /// \brief Changes which mesh to render.
  void SetMesh(const nsMeshResourceHandle& hMesh);
  NS_ALWAYS_INLINE const nsMeshResourceHandle& GetMesh() const { return m_hMesh; }

  /// \brief Sets the material that should be used for the sub-mesh with the given index.
  void SetMaterial(nsUInt32 uiIndex, const nsMaterialResourceHandle& hMaterial);
  nsMaterialResourceHandle GetMaterial(nsUInt32 uiIndex) const;

  void SetMeshFile(const char* szFile); // [ property ]
  const char* GetMeshFile() const;      // [ property ]

  /// \brief An additional tint color passed to the renderer to modify the mesh.
  void SetColor(const nsColor& color); // [ property ]
  const nsColor& GetColor() const;     // [ property ]

  /// \brief An additional vec4 passed to the renderer that can be used by custom material shaders for effects.
  void SetCustomData(const nsVec4& vData); // [ property ]
  const nsVec4& GetCustomData() const;     // [ property ]

  /// \brief The sorting depth offset allows to tweak the order in which this mesh is rendered relative to other meshes.
  ///
  /// This is mainly useful for transparent objects to render them before or after other meshes.
  void SetSortingDepthOffset(float fOffset);                // [ property ]
  float GetSortingDepthOffset() const;                      // [ property ]

  void OnMsgSetMeshMaterial(nsMsgSetMeshMaterial& ref_msg); // [ msg handler ]
  void OnMsgSetColor(nsMsgSetColor& ref_msg);               // [ msg handler ]
  void OnMsgSetCustomData(nsMsgSetCustomData& ref_msg);     // [ msg handler ]

protected:
  virtual nsMeshRenderData* CreateRenderData() const;

  nsUInt32 Materials_GetCount() const;                          // [ property ]
  const char* Materials_GetValue(nsUInt32 uiIndex) const;       // [ property ]
  void Materials_SetValue(nsUInt32 uiIndex, const char* value); // [ property ]
  void Materials_Insert(nsUInt32 uiIndex, const char* value);   // [ property ]
  void Materials_Remove(nsUInt32 uiIndex);                      // [ property ]

  void OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const;

  nsMeshResourceHandle m_hMesh;
  nsDynamicArray<nsMaterialResourceHandle> m_Materials;
  nsColor m_Color = nsColor::White;
  nsVec4 m_vCustomData = nsVec4(0, 1, 0, 1);
  float m_fSortingDepthOffset = 0.0f;
};

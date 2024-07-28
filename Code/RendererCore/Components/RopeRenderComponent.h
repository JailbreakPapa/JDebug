#pragma once

#include <RendererCore/Components/RenderComponent.h>
#include <RendererCore/Meshes/MeshResource.h>
#include <RendererCore/Meshes/SkinnedMeshComponent.h>
#include <memory>

struct nsMsgExtractRenderData;
struct nsMsgSetColor;
struct nsMsgSetMeshMaterial;
struct nsMsgRopePoseUpdated;
class nsShaderTransform;

using nsRopeRenderComponentManager = nsComponentManager<class nsRopeRenderComponent, nsBlockStorageType::Compact>;

/// \brief Used to render a rope or cable.
///
/// This is needed to visualize the nsFakeRopeComponent or nsJoltRopeComponent.
/// The component handles the message nsMsgRopePoseUpdated to generate an animated mesh and apply the pose.
/// The component has to be attached to the same object as the rope simulation component.
class NS_RENDERERCORE_DLL nsRopeRenderComponent : public nsRenderComponent
{
  NS_DECLARE_COMPONENT_TYPE(nsRopeRenderComponent, nsRenderComponent, nsRopeRenderComponentManager);

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

protected:
  virtual nsResult GetLocalBounds(nsBoundingBoxSphere& bounds, bool& bAlwaysVisible, nsMsgUpdateLocalBounds& msg) override;
  void OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const; // [ msg handler ]

  //////////////////////////////////////////////////////////////////////////
  // nsRopeRenderComponent

public:
  nsRopeRenderComponent();
  ~nsRopeRenderComponent();

  nsColor m_Color = nsColor::White;         // [ property ]

  void SetMaterialFile(const char* szFile); // [ property ]
  const char* GetMaterialFile() const;      // [ property ]

  void SetMaterial(const nsMaterialResourceHandle& hMaterial) { m_hMaterial = hMaterial; }
  nsMaterialResourceHandle GetMaterial() const { return m_hMaterial; }

  /// \brief Changes how thick the rope visualization is. This is independent of the simulated rope thickness.
  void SetThickness(float fThickness);                // [ property ]
  float GetThickness() const { return m_fThickness; } // [ property ]

  /// \brief Sets how round the rope shall be.
  void SetDetail(nsUInt32 uiDetail);                // [ property ]
  nsUInt32 GetDetail() const { return m_uiDetail; } // [ property ]

  /// \brief If enabled, the rendered mesh will be slightly more detailed along the rope.
  void SetSubdivide(bool bSubdivide);                // [ property ]
  bool GetSubdivide() const { return m_bSubdivide; } // [ property ]

  /// \brief How often to repeat the U texture coordinate along the rope's length.
  void SetUScale(float fUScale);                            // [ property ]
  float GetUScale() const { return m_fUScale; }             // [ property ]

  void OnMsgSetColor(nsMsgSetColor& ref_msg);               // [ msg handler ]
  void OnMsgSetMeshMaterial(nsMsgSetMeshMaterial& ref_msg); // [ msg handler ]

private:
  void OnRopePoseUpdated(nsMsgRopePoseUpdated& msg);        // [ msg handler ]
  void GenerateRenderMesh(nsUInt32 uiNumRopePieces);
  void UpdateSkinningTransformBuffer(nsArrayPtr<const nsTransform> skinningTransforms);

  nsBoundingBoxSphere m_LocalBounds;

  nsSkinningState m_SkinningState;

  nsMeshResourceHandle m_hMesh;
  nsMaterialResourceHandle m_hMaterial;

  float m_fThickness = 0.05f;
  nsUInt32 m_uiDetail = 6;
  bool m_bSubdivide = false;

  float m_fUScale = 1.0f;
};

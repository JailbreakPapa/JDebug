#pragma once

#include <RendererCore/RendererCoreDLL.h>

#include <Core/Messages/EventMessage.h>
#include <RendererCore/Components/RenderComponent.h>
#include <RendererCore/Material/MaterialResource.h>
#include <RendererCore/Meshes/MeshResource.h>

using nsBeamComponentManager = nsComponentManagerSimple<class nsBeamComponent, nsComponentUpdateType::Always>;

struct nsMsgExtractRenderData;
class nsGeometry;
class nsMeshResourceDescriptor;

/// \brief Renders a thick line from its own location to the position of another game object.
///
/// This is meant for simple effects, like laser beams. The geometry is very low resolution and won't look good close up.
/// When possible, use a highly emissive material without any pattern, where the bloom will hide the simple geometry.
///
/// For doing dynamic laser beams, you can combine it with the nsRaycastComponent, which will move the target component.
class NS_RENDERERCORE_DLL nsBeamComponent : public nsRenderComponent
{
  NS_DECLARE_COMPONENT_TYPE(nsBeamComponent, nsRenderComponent, nsBeamComponentManager);

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
  // nsBeamComponent

public:
  nsBeamComponent();
  ~nsBeamComponent();

  /// \brief Sets the GUID of the target object to which to draw the beam.
  void SetTargetObject(const char* szReference); // [ property ]

  /// \brief How wide to make the beam geometry
  void SetWidth(float fWidth); // [ property ]
  float GetWidth() const;      // [ property ]

  /// \brief How many world units the texture coordinates should take up, for using a repeatable texture for the beam.
  void SetUVUnitsPerWorldUnit(float fUVUnitsPerWorldUnit); // [ property ]
  float GetUVUnitsPerWorldUnit() const;                    // [ property ]

  /// \brief Which material asset to use for rendering the beam geometry.
  void SetMaterialFile(const char* szFile); // [ property ]
  const char* GetMaterialFile() const;      // [ property ]

  nsMaterialResourceHandle GetMaterial() const;

  /// \brief The object to which to draw the beam.
  nsGameObjectHandle m_hTargetObject; // [ property ]

  /// \brief Optional color to tint the beam.
  nsColor m_Color = nsColor::White; // [ property ]

protected:
  void Update();

  void OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const;

  float m_fWidth = 0.1f;                // [ property ]
  float m_fUVUnitsPerWorldUnit = 1.0f;  // [ property ]

  nsMaterialResourceHandle m_hMaterial; // [ property ]

  const float m_fDistanceUpdateEpsilon = 0.02f;

  nsMeshResourceHandle m_hMesh;

  nsVec3 m_vLastOwnerPosition = nsVec3::MakeZero();
  nsVec3 m_vLastTargetPosition = nsVec3::MakeZero();

  void CreateMeshes();
  void BuildMeshResourceFromGeometry(nsGeometry& Geometry, nsMeshResourceDescriptor& MeshDesc) const;
  void ReinitMeshes();
  void Cleanup();

  const char* DummyGetter() const { return nullptr; }
};

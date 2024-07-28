#pragma once

#include <Foundation/Math/Color16f.h>
#include <Foundation/Types/VarianceTypes.h>
#include <RendererCore/Components/RenderComponent.h>
#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/RenderData.h>

class nsAbstractObjectNode;
struct nsMsgComponentInternalTrigger;
struct nsMsgOnlyApplyToObject;
struct nsMsgSetColor;

class NS_RENDERERCORE_DLL nsDecalComponentManager final : public nsComponentManager<class nsDecalComponent, nsBlockStorageType::Compact>
{
public:
  nsDecalComponentManager(nsWorld* pWorld);

  virtual void Initialize() override;

private:
  friend class nsDecalComponent;
  nsDecalAtlasResourceHandle m_hDecalAtlas;
};

class NS_RENDERERCORE_DLL nsDecalRenderData : public nsRenderData
{
  NS_ADD_DYNAMIC_REFLECTION(nsDecalRenderData, nsRenderData);

public:
  nsUInt32 m_uiApplyOnlyToId;
  nsUInt32 m_uiFlags;
  nsUInt32 m_uiAngleFadeParams;

  nsColorLinearUB m_BaseColor;
  nsColorLinear16f m_EmissiveColor;

  nsUInt32 m_uiBaseColorAtlasScale;
  nsUInt32 m_uiBaseColorAtlasOffset;

  nsUInt32 m_uiNormalAtlasScale;
  nsUInt32 m_uiNormalAtlasOffset;

  nsUInt32 m_uiORMAtlasScale;
  nsUInt32 m_uiORMAtlasOffset;
};

/// \brief Projects a decal texture onto geometry within a box volume.
///
/// This is used to add dirt, scratches, signs and other surface imperfections to geometry.
/// The component uses a box shape to define the position and volume and projection direction.
/// This can be set up in a level to add detail, but it can also be used by dynamic effects such as bullet hits,
/// to visualize the impact. To add variety a prefab may use different textures and vary in size.
class NS_RENDERERCORE_DLL nsDecalComponent final : public nsRenderComponent
{
  NS_DECLARE_COMPONENT_TYPE(nsDecalComponent, nsRenderComponent, nsDecalComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // nsComponent

public:
  virtual void SerializeComponent(nsWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(nsWorldReader& inout_stream) override;

protected:
  virtual void OnSimulationStarted() override;
  virtual void OnActivated() override;

  //////////////////////////////////////////////////////////////////////////
  // nsRenderComponent

protected:
  virtual nsResult GetLocalBounds(nsBoundingBoxSphere& bounds, bool& bAlwaysVisible, nsMsgUpdateLocalBounds& msg) override;
  void OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const;


  //////////////////////////////////////////////////////////////////////////
  // nsDecalComponent

public:
  nsDecalComponent();
  ~nsDecalComponent();

  /// \brief Sets the extents of the box inside which to project the decal.
  void SetExtents(const nsVec3& value); // [ property ]
  const nsVec3& GetExtents() const;     // [ property ]

  /// \brief The size variance defines how much the size may randomly deviate, such that the decals look different.
  void SetSizeVariance(float fVariance); // [ property ]
  float GetSizeVariance() const;         // [ property ]

  /// \brief An additional tint color for the decal.
  void SetColor(nsColorGammaUB color); // [ property ]
  nsColorGammaUB GetColor() const;     // [ property ]

  /// \brief An additional emissive color to make the decal glow.
  void SetEmissiveColor(nsColor color); // [ property ]
  nsColor GetEmissiveColor() const;     // [ property ]

  /// \brief At which angle between the decal orientation and the surface it is projected onto, to start fading the decal out.
  void SetInnerFadeAngle(nsAngle fadeAngle); // [ property ]
  nsAngle GetInnerFadeAngle() const;         // [ property ]

  /// \brief At which angle between the decal orientation and the surface it is projected onto, to fully fade out the decal.
  void SetOuterFadeAngle(nsAngle fadeAngle); // [ property ]
  nsAngle GetOuterFadeAngle() const;         // [ property ]

  /// \brief If multiple decals are in the same location, this allows to tweak which one is rendered on top.
  void SetSortOrder(float fOrder); // [ property ]
  float GetSortOrder() const;      // [ property ]

  /// \brief Whether the decal projection should use a kind of three-way texture mapping to wrap the image around curved geometry.
  void SetWrapAround(bool bWrapAround);         // [ property ]
  bool GetWrapAround() const;                   // [ property ]

  void SetMapNormalToGeometry(bool bMapNormal); // [ property ]
  bool GetMapNormalToGeometry() const;          // [ property ]

  /// \brief Sets the decal resource to use. If more than one is set, a random one will be chosen.
  ///
  /// Indices that are written to will be created on-demand.
  void SetDecal(nsUInt32 uiIndex, const nsDecalResourceHandle& hResource); // [ property ]
  const nsDecalResourceHandle& GetDecal(nsUInt32 uiIndex) const;           // [ property ]

  /// If non-zero, the decal fades out after this time and then vanishes.
  nsVarianceTypeTime m_FadeOutDelay; // [ property ]

  /// How much time the fade out takes.
  nsTime m_FadeOutDuration; // [ property ]

  /// If fade-out is used, the decal may delete itself afterwards.
  nsEnum<nsOnComponentFinishedAction> m_OnFinishedAction; // [ property ]

  /// \brief Sets the cardinal axis into which the decal projection should be.
  void SetProjectionAxis(nsEnum<nsBasisAxis> projectionAxis); // [ property ]
  nsEnum<nsBasisAxis> GetProjectionAxis() const;              // [ property ]

  /// \brief If set, the decal only appears on the given object.
  ///
  /// This is typically used to limit the decal to a single dynamic object, such that damage decals don't project
  /// onto static geometry and other objects.
  void SetApplyOnlyTo(nsGameObjectHandle hObject);
  nsGameObjectHandle GetApplyOnlyTo() const;

  nsUInt32 DecalFile_GetCount() const;                         // [ property ]
  const char* DecalFile_Get(nsUInt32 uiIndex) const;           // [ property ]
  void DecalFile_Set(nsUInt32 uiIndex, const char* szFile);    // [ property ]
  void DecalFile_Insert(nsUInt32 uiIndex, const char* szFile); // [ property ]
  void DecalFile_Remove(nsUInt32 uiIndex);                     // [ property ]


protected:
  void SetApplyToRef(const char* szReference); // [ property ]
  void UpdateApplyTo();

  void OnTriggered(nsMsgComponentInternalTrigger& msg);
  void OnMsgDeleteGameObject(nsMsgDeleteGameObject& msg);
  void OnMsgOnlyApplyToObject(nsMsgOnlyApplyToObject& msg);
  void OnMsgSetColor(nsMsgSetColor& msg);

  nsVec3 m_vExtents = nsVec3(1.0f);
  float m_fSizeVariance = 0;
  nsColorGammaUB m_Color = nsColor::White;
  nsColor m_EmissiveColor = nsColor::Black;
  nsAngle m_InnerFadeAngle = nsAngle::MakeFromDegree(50.0f);
  nsAngle m_OuterFadeAngle = nsAngle::MakeFromDegree(80.0f);
  float m_fSortOrder = 0;
  bool m_bWrapAround = false;
  bool m_bMapNormalToGeometry = false;
  nsUInt8 m_uiRandomDecalIdx = 0xFF;
  nsEnum<nsBasisAxis> m_ProjectionAxis;
  nsHybridArray<nsDecalResourceHandle, 1> m_Decals;

  nsGameObjectHandle m_hApplyOnlyToObject;
  nsUInt32 m_uiApplyOnlyToId = 0;

  nsTime m_StartFadeOutTime;
  nsUInt32 m_uiInternalSortKey = 0;

private:
  const char* DummyGetter() const { return nullptr; }
};

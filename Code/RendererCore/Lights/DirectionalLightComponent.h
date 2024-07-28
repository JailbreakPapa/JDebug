#pragma once

#include <RendererCore/Lights/LightComponent.h>
#include <RendererCore/Textures/Texture2DResource.h>

using nsDirectionalLightComponentManager = nsComponentManager<class nsDirectionalLightComponent, nsBlockStorageType::Compact>;

/// \brief The render data object for directional lights.
class NS_RENDERERCORE_DLL nsDirectionalLightRenderData : public nsLightRenderData
{
  NS_ADD_DYNAMIC_REFLECTION(nsDirectionalLightRenderData, nsLightRenderData);

public:
};

/// \brief A directional lightsource shines light into one fixed direction and has infinite size. It is usually used for sunlight.
///
/// It is very rare to use more than one directional lightsource at the same time.
/// Directional lightsources are used to fake the large scale light of the sun (or moon).
/// They use cascaded shadow maps to reduce the performance overhead for dynamic shadows of such large lights.
class NS_RENDERERCORE_DLL nsDirectionalLightComponent : public nsLightComponent
{
  NS_DECLARE_COMPONENT_TYPE(nsDirectionalLightComponent, nsLightComponent, nsDirectionalLightComponentManager);

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
  // nsDirectionalLightComponent

public:
  nsDirectionalLightComponent();
  ~nsDirectionalLightComponent();

  /// \brief Sets how many shadow map cascades to use. Typically between 2 and 4.
  void SetNumCascades(nsUInt32 uiNumCascades); // [ property ]
  nsUInt32 GetNumCascades() const;             // [ property ]

  /// \brief Sets the distance around the main camera in which to apply dynamic shadows.
  void SetMinShadowRange(float fMinShadowRange); // [ property ]
  float GetMinShadowRange() const;               // [ property ]

  /// \brief The factor (0 to 1) at which relative distance to start fading out the shadow map. Typically 0.8 or 0.9.
  void SetFadeOutStart(float fFadeOutStart); // [ property ]
  float GetFadeOutStart() const;             // [ property ]

  /// \brief Has something to do with shadow map cascades (TODO: figure out what).
  void SetSplitModeWeight(float fSplitModeWeight); // [ property ]
  float GetSplitModeWeight() const;                // [ property ]

  /// \brief Has something to do with shadow map cascades (TODO: figure out what).
  void SetNearPlaneOffset(float fNearPlaneOffset); // [ property ]
  float GetNearPlaneOffset() const;                // [ property ]

protected:
  void OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const;

  nsUInt32 m_uiNumCascades = 3;
  float m_fMinShadowRange = 50.0f;
  float m_fFadeOutStart = 0.8f;
  float m_fSplitModeWeight = 0.7f;
  float m_fNearPlaneOffset = 100.0f;
};

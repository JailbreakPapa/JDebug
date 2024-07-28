#pragma once

#include <RendererCore/Lights/LightComponent.h>
#include <RendererCore/Pipeline/Declarations.h>
#include <RendererCore/Textures/TextureCubeResource.h>

using nsPointLightComponentManager = nsComponentManager<class nsPointLightComponent, nsBlockStorageType::Compact>;

/// \brief The render data object for point lights.
class NS_RENDERERCORE_DLL nsPointLightRenderData : public nsLightRenderData
{
  NS_ADD_DYNAMIC_REFLECTION(nsPointLightRenderData, nsLightRenderData);

public:
  float m_fRange;
  // nsTextureCubeResourceHandle m_hProjectedTexture;
};

/// \brief Adds a dynamic point light to the scene, optionally casting shadows.
///
/// For performance reasons, prefer to use nsSpotLightComponent where possible.
/// Do not use shadows just to limit the light cone, when a spot light could achieve the same.
class NS_RENDERERCORE_DLL nsPointLightComponent : public nsLightComponent
{
  NS_DECLARE_COMPONENT_TYPE(nsPointLightComponent, nsLightComponent, nsPointLightComponentManager);

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
  // nsPointLightComponent

public:
  nsPointLightComponent();
  ~nsPointLightComponent();

  /// \brief Sets the radius of the lightsource. If zero, the radius is automatically determined from the intensity.
  void SetRange(float fRange); // [ property ]
  float GetRange() const;      // [ property ]

  /// \brief Returns the final radius of the lightsource.
  float GetEffectiveRange() const;

  // void SetProjectedTextureFile(const char* szFile); // [ property ]
  // const char* GetProjectedTextureFile() const;      // [ property ]

  // void SetProjectedTexture(const nsTextureCubeResourceHandle& hProjectedTexture);
  // const nsTextureCubeResourceHandle& GetProjectedTexture() const;

protected:
  void OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const;

  float m_fRange = 0.0f;
  float m_fEffectiveRange = 0.0f;

  // nsTextureCubeResourceHandle m_hProjectedTexture;
};

/// \brief A special visualizer attribute for point lights
class NS_RENDERERCORE_DLL nsPointLightVisualizerAttribute : public nsVisualizerAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsPointLightVisualizerAttribute, nsVisualizerAttribute);

public:
  nsPointLightVisualizerAttribute();
  nsPointLightVisualizerAttribute(const char* szRangeProperty, const char* szIntensityProperty, const char* szColorProperty);

  const nsUntrackedString& GetRangeProperty() const { return m_sProperty1; }
  const nsUntrackedString& GetIntensityProperty() const { return m_sProperty2; }
  const nsUntrackedString& GetColorProperty() const { return m_sProperty3; }
};

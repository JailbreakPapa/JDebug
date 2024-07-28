#pragma once

#include <RendererCore/Lights/LightComponent.h>
#include <RendererCore/Pipeline/Declarations.h>
#include <RendererCore/Textures/Texture2DResource.h>

using nsSpotLightComponentManager = nsComponentManager<class nsSpotLightComponent, nsBlockStorageType::Compact>;

/// \brief The render data object for spot lights.
class NS_RENDERERCORE_DLL nsSpotLightRenderData : public nsLightRenderData
{
  NS_ADD_DYNAMIC_REFLECTION(nsSpotLightRenderData, nsLightRenderData);

public:
  float m_fRange;
  nsAngle m_InnerSpotAngle;
  nsAngle m_OuterSpotAngle;
  // nsTexture2DResourceHandle m_hProjectedTexture;
};

/// \brief Adds a spotlight to the scene, optionally casting shadows.
class NS_RENDERERCORE_DLL nsSpotLightComponent : public nsLightComponent
{
  NS_DECLARE_COMPONENT_TYPE(nsSpotLightComponent, nsLightComponent, nsSpotLightComponentManager);

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
  // nsSpotLightComponent

public:
  nsSpotLightComponent();
  ~nsSpotLightComponent();

  /// \brief Sets the radius (or length of the cone) of the lightsource. If zero, it is automatically determined from the intensity.
  void SetRange(float fRange); // [ property ]
  float GetRange() const;      // [ property ]

  /// \brief Returns the final radius of the lightsource.
  float GetEffectiveRange() const;

  /// \brief Sets the inner angle where the spotlight has equal brightness.
  void SetInnerSpotAngle(nsAngle spotAngle); // [ property ]
  nsAngle GetInnerSpotAngle() const;         // [ property ]

  /// \brief Sets the outer angle of the spotlight's cone. The light will fade out between the inner and outer angle.
  void SetOuterSpotAngle(nsAngle spotAngle); // [ property ]
  nsAngle GetOuterSpotAngle() const;         // [ property ]

  // void SetProjectedTextureFile(const char* szFile); // [ property ]
  // const char* GetProjectedTextureFile() const;      // [ property ]

  // void SetProjectedTexture(const nsTexture2DResourceHandle& hProjectedTexture);
  // const nsTexture2DResourceHandle& GetProjectedTexture() const;

protected:
  void OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const;
  nsBoundingSphere CalculateBoundingSphere(const nsTransform& t, float fRange) const;

  float m_fRange = 0.0f;
  float m_fEffectiveRange = 0.0f;

  nsAngle m_InnerSpotAngle = nsAngle::MakeFromDegree(15.0f);
  nsAngle m_OuterSpotAngle = nsAngle::MakeFromDegree(30.0f);

  // nsTexture2DResourceHandle m_hProjectedTexture;
};

/// \brief A special visualizer attribute for spot lights
class NS_RENDERERCORE_DLL nsSpotLightVisualizerAttribute : public nsVisualizerAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsSpotLightVisualizerAttribute, nsVisualizerAttribute);

public:
  nsSpotLightVisualizerAttribute();
  nsSpotLightVisualizerAttribute(
    const char* szAngleProperty, const char* szRangeProperty, const char* szIntensityProperty, const char* szColorProperty);

  const nsUntrackedString& GetAngleProperty() const { return m_sProperty1; }
  const nsUntrackedString& GetRangeProperty() const { return m_sProperty2; }
  const nsUntrackedString& GetIntensityProperty() const { return m_sProperty3; }
  const nsUntrackedString& GetColorProperty() const { return m_sProperty4; }
};

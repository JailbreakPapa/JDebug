#pragma once

#include <RendererCore/Components/RenderComponent.h>
#include <RendererCore/Pipeline/RenderData.h>

struct nsMsgSetColor;

/// \brief Base class for light render data objects.
class NS_RENDERERCORE_DLL nsLightRenderData : public nsRenderData
{
  NS_ADD_DYNAMIC_REFLECTION(nsLightRenderData, nsRenderData);

public:
  void FillBatchIdAndSortingKey(float fScreenSpaceSize);

  nsColor m_LightColor;
  float m_fIntensity;
  float m_fSpecularMultiplier;
  nsUInt32 m_uiShadowDataOffset;
};

/// \brief Base class for dynamic light components.
class NS_RENDERERCORE_DLL nsLightComponent : public nsRenderComponent
{
  NS_DECLARE_ABSTRACT_COMPONENT_TYPE(nsLightComponent, nsRenderComponent);

  //////////////////////////////////////////////////////////////////////////
  // nsComponent

public:
  virtual void SerializeComponent(nsWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(nsWorldReader& inout_stream) override;

  //////////////////////////////////////////////////////////////////////////
  // nsLightComponent

public:
  nsLightComponent();
  ~nsLightComponent();

  /// \brief Used to enable kelvin color values. This is a physical representation of light color using.
  /// for more detail: https://wikipedia.org/wiki/Color_temperature
  void SetUsingColorTemperature(bool bUseColorTemperature);
  bool GetUsingColorTemperature() const;

  void SetLightColor(nsColorGammaUB lightColor); // [ property ]
  nsColorGammaUB GetBaseLightColor() const;      // [ property ]

  nsColorGammaUB GetLightColor() const;

  void SetTemperature(nsUInt32 uiTemperature); // [ property ]
  nsUInt32 GetTemperature() const;             // [ property ]

  /// \brief Sets the brightness of the lightsource.
  void SetIntensity(float fIntensity);                   // [ property ]
  float GetIntensity() const;                            // [ property ]

  void SetSpecularMultiplier(float fSpecularMultiplier); // [ property ]
  float GetSpecularMultiplier() const;                   // [ property ]

  /// \brief Sets whether the lightsource shall cast dynamic shadows.
  void SetCastShadows(bool bCastShadows); // [ property ]
  bool GetCastShadows() const;            // [ property ]

  /// \brief Sets the fuzziness of the shadow edges.
  void SetPenumbraSize(float fPenumbraSize); // [ property ]
  float GetPenumbraSize() const;             // [ property ]

  /// \brief Allows to tweak how dynamic shadows are applied to reduce artifacts.
  void SetSlopeBias(float fShadowBias); // [ property ]
  float GetSlopeBias() const;           // [ property ]

  /// \brief Allows to tweak how dynamic shadows are applied to reduce artifacts.
  void SetConstantBias(float fShadowBias);    // [ property ]
  float GetConstantBias() const;              // [ property ]

  void OnMsgSetColor(nsMsgSetColor& ref_msg); // [ msg handler ]

  /// \brief Calculates how far a lightsource would shine given the specified range and intensity.
  ///
  /// If fRange is zero, the range needed for the given intensity is returned.
  /// Otherwise the smaller value of that and fRange is returned.
  static float CalculateEffectiveRange(float fRange, float fIntensity);

  /// \brief Calculates how large on screen (in pixels) the lightsource would be.
  static float CalculateScreenSpaceSize(const nsBoundingSphere& sphere, const nsCamera& camera);

protected:
  nsColorGammaUB m_LightColor = nsColor::White;
  nsUInt32 m_uiTemperature = 6550;
  float m_fIntensity = 10.0f;
  float m_fSpecularMultiplier = 1.0f;
  float m_fPenumbraSize = 0.1f;
  float m_fSlopeBias = 0.25f;
  float m_fConstantBias = 0.1f;
  bool m_bCastShadows = false;
  bool m_bUseColorTemperature = false;
};

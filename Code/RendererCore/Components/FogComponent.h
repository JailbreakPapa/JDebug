#pragma once

#include <Core/World/SettingsComponent.h>
#include <Core/World/SettingsComponentManager.h>
#include <RendererCore/Pipeline/RenderData.h>

struct nsMsgUpdateLocalBounds;

using nsFogComponentManager = nsSettingsComponentManager<class nsFogComponent>;

/// \brief The render data object for ambient light.
class NS_RENDERERCORE_DLL nsFogRenderData : public nsRenderData
{
  NS_ADD_DYNAMIC_REFLECTION(nsFogRenderData, nsRenderData);

public:
  nsColor m_Color;
  float m_fDensity;
  float m_fHeightFalloff;
  float m_fInvSkyDistance;
};

class NS_RENDERERCORE_DLL nsFogComponent : public nsSettingsComponent
{
  NS_DECLARE_COMPONENT_TYPE(nsFogComponent, nsSettingsComponent, nsFogComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // nsComponent

public:
  virtual void SerializeComponent(nsWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(nsWorldReader& inout_stream) override;

protected:
  virtual void Deinitialize() override;
  virtual void OnActivated() override;
  virtual void OnDeactivated() override;


  //////////////////////////////////////////////////////////////////////////
  // nsFogComponent

public:
  nsFogComponent();
  ~nsFogComponent();

  void SetColor(nsColor color);                 // [ property ]
  nsColor GetColor() const;                     // [ property ]

  void SetDensity(float fDensity);              // [ property ]
  float GetDensity() const;                     // [ property ]

  void SetHeightFalloff(float fHeightFalloff);  // [ property ]
  float GetHeightFalloff() const;               // [ property ]

  void SetModulateWithSkyColor(bool bModulate); // [ property ]
  bool GetModulateWithSkyColor() const;         // [ property ]

  void SetSkyDistance(float fDistance);         // [ property ]
  float GetSkyDistance() const;                 // [ property ]

protected:
  void OnUpdateLocalBounds(nsMsgUpdateLocalBounds& msg);
  void OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const;

  nsColor m_Color = nsColor(0.2f, 0.2f, 0.3f);
  float m_fDensity = 1.0f;
  float m_fHeightFalloff = 10.0f;
  float m_fSkyDistance = 1000.0f;
  bool m_bModulateWithSkyColor = false;
};

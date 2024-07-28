#pragma once

#include <Core/World/SettingsComponent.h>
#include <Core/World/SettingsComponentManager.h>
#include <RendererCore/RendererCoreDLL.h>

struct nsMsgUpdateLocalBounds;

using nsAmbientLightComponentManager = nsSettingsComponentManager<class nsAmbientLightComponent>;

class NS_RENDERERCORE_DLL nsAmbientLightComponent : public nsSettingsComponent
{
  NS_DECLARE_COMPONENT_TYPE(nsAmbientLightComponent, nsSettingsComponent, nsAmbientLightComponentManager);

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
  // nsAmbientLightComponent

public:
  nsAmbientLightComponent();
  ~nsAmbientLightComponent();

  void SetTopColor(nsColorGammaUB color);    // [ property ]
  nsColorGammaUB GetTopColor() const;        // [ property ]

  void SetBottomColor(nsColorGammaUB color); // [ property ]
  nsColorGammaUB GetBottomColor() const;     // [ property ]

  void SetIntensity(float fIntensity);       // [ property ]
  float GetIntensity() const;                // [ property ]

private:
  void OnUpdateLocalBounds(nsMsgUpdateLocalBounds& msg);
  void UpdateSkyIrradiance();

  nsColorGammaUB m_TopColor = nsColor(0.2f, 0.2f, 0.3f);
  nsColorGammaUB m_BottomColor = nsColor(0.1f, 0.1f, 0.15f);
  float m_fIntensity = 1.0f;
};

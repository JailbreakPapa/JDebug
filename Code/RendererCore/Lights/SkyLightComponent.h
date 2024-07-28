#pragma once

#include <Core/World/SettingsComponent.h>
#include <Core/World/SettingsComponentManager.h>
#include <RendererCore/Lights/Implementation/ReflectionProbeData.h>
#include <RendererCore/Textures/TextureCubeResource.h>

struct nsMsgUpdateLocalBounds;
struct nsMsgExtractRenderData;
struct nsMsgTransformChanged;

using nsSkyLightComponentManager = nsSettingsComponentManager<class nsSkyLightComponent>;

class NS_RENDERERCORE_DLL nsSkyLightComponent : public nsSettingsComponent
{
  NS_DECLARE_COMPONENT_TYPE(nsSkyLightComponent, nsSettingsComponent, nsSkyLightComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // nsComponent

public:
  virtual void SerializeComponent(nsWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(nsWorldReader& inout_stream) override;

protected:
  virtual void OnActivated() override;
  virtual void OnDeactivated() override;


  //////////////////////////////////////////////////////////////////////////
  // nsSkyLightComponent

public:
  nsSkyLightComponent();
  ~nsSkyLightComponent();

  void SetReflectionProbeMode(nsEnum<nsReflectionProbeMode> mode); // [ property ]
  nsEnum<nsReflectionProbeMode> GetReflectionProbeMode() const;    // [ property ]

  void SetIntensity(float fIntensity);                             // [ property ]
  float GetIntensity() const;                                      // [ property ]

  void SetSaturation(float fSaturation);                           // [ property ]
  float GetSaturation() const;                                     // [ property ]

  const nsTagSet& GetIncludeTags() const;                          // [ property ]
  void InsertIncludeTag(const char* szTag);                        // [ property ]
  void RemoveIncludeTag(const char* szTag);                        // [ property ]

  const nsTagSet& GetExcludeTags() const;                          // [ property ]
  void InsertExcludeTag(const char* szTag);                        // [ property ]
  void RemoveExcludeTag(const char* szTag);                        // [ property ]

  void SetShowDebugInfo(bool bShowDebugInfo);                      // [ property ]
  bool GetShowDebugInfo() const;                                   // [ property ]

  void SetShowMipMaps(bool bShowMipMaps);                          // [ property ]
  bool GetShowMipMaps() const;                                     // [ property ]

  void SetCubeMapFile(const char* szFile);                         // [ property ]
  const char* GetCubeMapFile() const;                              // [ property ]
  nsTextureCubeResourceHandle GetCubeMap() const
  {
    return m_hCubeMap;
  }

  float GetNearPlane() const { return m_Desc.m_fNearPlane; } // [ property ]
  void SetNearPlane(float fNearPlane);                       // [ property ]

  float GetFarPlane() const { return m_Desc.m_fFarPlane; }   // [ property ]
  void SetFarPlane(float fFarPlane);                         // [ property ]

protected:
  void OnUpdateLocalBounds(nsMsgUpdateLocalBounds& msg);
  void OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const;
  void OnTransformChanged(nsMsgTransformChanged& msg);

  nsReflectionProbeDesc m_Desc;
  nsTextureCubeResourceHandle m_hCubeMap;

  nsReflectionProbeId m_Id;

  mutable bool m_bStatesDirty = true;
};

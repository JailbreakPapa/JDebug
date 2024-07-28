#pragma once

#include <RendererCore/Lights/ReflectionProbeComponentBase.h>

class NS_RENDERERCORE_DLL nsSphereReflectionProbeComponentManager final : public nsComponentManager<class nsSphereReflectionProbeComponent, nsBlockStorageType::Compact>
{
public:
  nsSphereReflectionProbeComponentManager(nsWorld* pWorld);
};

//////////////////////////////////////////////////////////////////////////
// nsSphereReflectionProbeComponent

/// \brief Sphere reflection probe component.
///
/// The generated reflection cube map is is projected to infinity. So parallax correction takes place.
class NS_RENDERERCORE_DLL nsSphereReflectionProbeComponent : public nsReflectionProbeComponentBase
{
  NS_DECLARE_COMPONENT_TYPE(nsSphereReflectionProbeComponent, nsReflectionProbeComponentBase, nsSphereReflectionProbeComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // nsComponent

public:
  virtual void SerializeComponent(nsWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(nsWorldReader& inout_stream) override;

protected:
  virtual void OnActivated() override;
  virtual void OnDeactivated() override;


  //////////////////////////////////////////////////////////////////////////
  // nsSphereReflectionProbeComponent

public:
  nsSphereReflectionProbeComponent();
  ~nsSphereReflectionProbeComponent();

  void SetRadius(float fRadius);                                   // [ property ]
  float GetRadius() const;                                         // [ property ]

  void SetFalloff(float fFalloff);                                 // [ property ]
  float GetFalloff() const { return m_fFalloff; }                  // [ property ]

  void SetSphereProjection(bool bSphereProjection);                // [ property ]
  bool GetSphereProjection() const { return m_bSphereProjection; } // [ property ]

protected:
  //////////////////////////////////////////////////////////////////////////
  // Editor
  void OnObjectCreated(const nsAbstractObjectNode& node);

protected:
  void OnUpdateLocalBounds(nsMsgUpdateLocalBounds& msg);
  void OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const;
  void OnTransformChanged(nsMsgTransformChanged& msg);
  float m_fRadius = 5.0f;
  float m_fFalloff = 0.1f;
  bool m_bSphereProjection = true;
};

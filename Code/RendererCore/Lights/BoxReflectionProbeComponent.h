#pragma once

#include <RendererCore/Lights/ReflectionProbeComponentBase.h>

class NS_RENDERERCORE_DLL nsBoxReflectionProbeComponentManager final : public nsComponentManager<class nsBoxReflectionProbeComponent, nsBlockStorageType::Compact>
{
public:
  nsBoxReflectionProbeComponentManager(nsWorld* pWorld);
};

/// \brief Box reflection probe component.
///
/// The generated reflection cube map is projected on a box defined by this component's extents. The influence volume can be smaller than the projection which is defined by a scale and shift parameter. Each side of the influence volume has a separate falloff parameter to smoothly blend the probe into others.
class NS_RENDERERCORE_DLL nsBoxReflectionProbeComponent : public nsReflectionProbeComponentBase
{
  NS_DECLARE_COMPONENT_TYPE(nsBoxReflectionProbeComponent, nsReflectionProbeComponentBase, nsBoxReflectionProbeComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // nsComponent

public:
  virtual void SerializeComponent(nsWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(nsWorldReader& inout_stream) override;

protected:
  virtual void OnActivated() override;
  virtual void OnDeactivated() override;


  //////////////////////////////////////////////////////////////////////////
  // nsBoxReflectionProbeComponent

public:
  nsBoxReflectionProbeComponent();
  ~nsBoxReflectionProbeComponent();

  const nsVec3& GetExtents() const;                                       // [ property ]
  void SetExtents(const nsVec3& vExtents);                                // [ property ]

  const nsVec3& GetInfluenceScale() const;                                // [ property ]
  void SetInfluenceScale(const nsVec3& vInfluenceScale);                  // [ property ]
  const nsVec3& GetInfluenceShift() const;                                // [ property ]
  void SetInfluenceShift(const nsVec3& vInfluenceShift);                  // [ property ]

  void SetPositiveFalloff(const nsVec3& vFalloff);                        // [ property ]
  const nsVec3& GetPositiveFalloff() const { return m_vPositiveFalloff; } // [ property ]
  void SetNegativeFalloff(const nsVec3& vFalloff);                        // [ property ]
  const nsVec3& GetNegativeFalloff() const { return m_vNegativeFalloff; } // [ property ]

  void SetBoxProjection(bool bBoxProjection);                             // [ property ]
  bool GetBoxProjection() const { return m_bBoxProjection; }              // [ property ]

protected:
  //////////////////////////////////////////////////////////////////////////
  // Editor
  void OnObjectCreated(const nsAbstractObjectNode& node);

protected:
  void OnUpdateLocalBounds(nsMsgUpdateLocalBounds& msg);
  void OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const;
  void OnTransformChanged(nsMsgTransformChanged& msg);

protected:
  nsVec3 m_vExtents = nsVec3(5.0f);
  nsVec3 m_vInfluenceScale = nsVec3(1.0f);
  nsVec3 m_vInfluenceShift = nsVec3(0.0f);
  nsVec3 m_vPositiveFalloff = nsVec3(0.1f, 0.1f, 0.0f);
  nsVec3 m_vNegativeFalloff = nsVec3(0.1f, 0.1f, 0.0f);
  bool m_bBoxProjection = true;
};

/// \brief A special visualizer attribute for box reflection probes
class NS_RENDERERCORE_DLL nsBoxReflectionProbeVisualizerAttribute : public nsVisualizerAttribute
{
  NS_ADD_DYNAMIC_REFLECTION(nsBoxReflectionProbeVisualizerAttribute, nsVisualizerAttribute);

public:
  nsBoxReflectionProbeVisualizerAttribute();

  nsBoxReflectionProbeVisualizerAttribute(const char* szExtentsProperty, const char* szInfluenceScaleProperty, const char* szInfluenceShiftProperty);

  const nsUntrackedString& GetExtentsProperty() const { return m_sProperty1; }
  const nsUntrackedString& GetInfluenceScaleProperty() const { return m_sProperty2; }
  const nsUntrackedString& GetInfluenceShiftProperty() const { return m_sProperty3; }
};

#pragma once

#include <RendererCore/RendererCoreDLL.h>

#include <RendererCore/Components/RenderComponent.h>

using nsLodComponentManager = nsComponentManager<class nsLodComponent, nsBlockStorageType::FreeList>;

struct nsMsgExtractRenderData;
struct nsMsgComponentInternalTrigger;

/// \brief Switches child objects named 'LODn' (with n from 0 to 4) on and off, depending on how close this object is to the main camera.
///
/// The LOD (level-of-detail) component is used to reduce the performance impact of complex objects when they are far away.
/// To do so, a "LOD object" consists of multiple states, from highly detailed to very coarse.
/// By convention the highly detailed object is called 'LOD0' and the lesser detailed objects are called 'LOD1', 'LOD2', 'LOD3' and finally 'LOD4'.
///
/// This component calculates how large it roughly appears on screen (at it's current distance). This is called the 'coverage' value.
/// Using user defined coverage thresholds, it then selects which LOD child object to activate. All others get deactivated.
/// The LODs have to be direct child objects called 'LOD0' to 'LOD4'. Other child objects are not affected.
///
/// How many LODs are used depends on the number of elements in the 'LodThresholds' array.
/// The array describes up to which coverage value each LOD is used.
/// Thus if it contains one value, two LODs will be used, LOD0 for coverage values above the specified threshold, and LOD1 at lower coverage values.
///
/// To see the current coverage, enable the debug drawing.
///
/// The coverage calculation uses spherical bounds. It should be configured to encompass the geometry of all LODs.
///
/// To prevent LODs switching back and forth at one exact boundary, the LOD ranges may overlap by a fixed percentage.
/// This way once one LOD gets activated, the coverage value has to change back quite a bit, before the previous LOD gets activated.
/// Since this behavior can make it harder to set up the LOD thresholds, it can be deactivated, but in practice it should stay enabled.
class NS_RENDERERCORE_DLL nsLodComponent : public nsRenderComponent
{
  NS_DECLARE_COMPONENT_TYPE(nsLodComponent, nsRenderComponent, nsLodComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // nsComponent

public:
  virtual void SerializeComponent(nsWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(nsWorldReader& inout_stream) override;

protected:
  virtual void OnActivated() override;
  virtual void OnDeactivated() override;

  //////////////////////////////////////////////////////////////////////////
  // nsRenderComponent

public:
  virtual nsResult GetLocalBounds(nsBoundingBoxSphere& out_bounds, bool& out_bAlwaysVisible, nsMsgUpdateLocalBounds& ref_msg) override;


  //////////////////////////////////////////////////////////////////////////
  // nsLodComponent

public:
  nsLodComponent();
  ~nsLodComponent();

  /// \brief Enables text output to show the current coverage value and selected LOD.
  void SetShowDebugInfo(bool bShow); // [ property ]
  bool GetShowDebugInfo() const;     // [ property ]

  /// \brief Disabling the LOD range overlap functionality can make it easier to determine the desired coverage thresholds.
  void SetOverlapRanges(bool bOverlap); // [ property ]
  bool GetOverlapRanges() const;        // [ property ]

protected:
  void OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const;
  void OnMsgComponentInternalTrigger(nsMsgComponentInternalTrigger& msg);

  nsInt8 m_iCurLod = -1;
  nsVec3 m_vBoundsOffset = nsVec3::MakeZero();
  float m_fBoundsRadius = 1.0f;
  nsStaticArray<float, 4> m_LodThresholds;
};

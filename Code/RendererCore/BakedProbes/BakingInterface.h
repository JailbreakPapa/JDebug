#pragma once

#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Utilities/Progress.h>
#include <RendererCore/RendererCoreDLL.h>

struct NS_RENDERERCORE_DLL nsBakingSettings
{
  nsVec3 m_vProbeSpacing = nsVec3(4);
  nsUInt32 m_uiNumSamplesPerProbe = 128;
  float m_fMaxRayDistance = 1000.0f;

  nsResult Serialize(nsStreamWriter& inout_stream) const;
  nsResult Deserialize(nsStreamReader& inout_stream);
};

NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsBakingSettings);

class nsWorld;

class nsBakingInterface
{
public:
  /// \brief Renders a debug view of the baking scene
  virtual nsResult RenderDebugView(const nsWorld& world, const nsMat4& mInverseViewProjection, nsUInt32 uiWidth, nsUInt32 uiHeight, nsDynamicArray<nsColorGammaUB>& out_pixels, nsProgress& ref_progress) const = 0;
};

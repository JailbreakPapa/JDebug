#pragma once

enum class nsSVSMLightProjectionType
{
  Directional,
  Point,
  Spot,
  Area, ///< Not Supported within the engine yet.
};

enum class nsSVSMShadowSupport
{
  None,
  SVSM,
  SVSMWithAdaptiveSampling,
  SVSMWithAdaptiveSamplingAndTemporalReprojection,
  SVSMWithRayTracing, ///< Not Supported within Slipstream Engine yet, but will be back-ported soon. /// NOTE: SVSM with Ray Tracing is not supported on all platforms, and is very buggy on PS5. dont backport Yet.
};

struct nsVirtualTextureInfo
{
public:
  int VTSize = 0;
  int TileSize;
  int BorderSize;

  int PageSize = TileSize + 2 * BorderSize;
  int PageTableSize = VTSize / TileSize;
};
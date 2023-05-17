#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>

class QRectF;
class QScreen;

namespace wdWidgetUtils
{
  /// \brief Contrary to QApplication::screenAt() this function will always succeed with a valid cursor positions
  /// and also with out of bounds cursor positions.
  WD_GUIFOUNDATION_DLL QScreen& GetClosestScreen(const QPoint& point);

  WD_GUIFOUNDATION_DLL void AdjustGridDensity(
    double& ref_fFinestDensity, double& ref_fRoughDensity, wdUInt32 uiWindowWidth, double fViewportSceneWidth, wdUInt32 uiMinPixelsForStep);

  WD_GUIFOUNDATION_DLL void ComputeGridExtentsX(const QRectF& viewportSceneRect, double fGridStops, double& out_fMinX, double& out_fMaxX);

  WD_GUIFOUNDATION_DLL void ComputeGridExtentsY(const QRectF& viewportSceneRect, double fGridStops, double& out_fMinY, double& out_fMaxY);
} // namespace wdWidgetUtils

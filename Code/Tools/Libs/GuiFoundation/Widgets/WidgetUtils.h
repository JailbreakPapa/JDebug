/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>

class QRectF;
class QScreen;

namespace nsWidgetUtils
{
  /// \brief Contrary to QApplication::screenAt() this function will always succeed with a valid cursor positions
  /// and also with out of bounds cursor positions.
  NS_GUIFOUNDATION_DLL QScreen& GetClosestScreen(const QPoint& point);

  NS_GUIFOUNDATION_DLL void AdjustGridDensity(
    double& ref_fFinestDensity, double& ref_fRoughDensity, nsUInt32 uiWindowWidth, double fViewportSceneWidth, nsUInt32 uiMinPixelsForStep);

  NS_GUIFOUNDATION_DLL void ComputeGridExtentsX(const QRectF& viewportSceneRect, double fGridStops, double& out_fMinX, double& out_fMaxX);

  NS_GUIFOUNDATION_DLL void ComputeGridExtentsY(const QRectF& viewportSceneRect, double fGridStops, double& out_fMinY, double& out_fMaxY);
} // namespace nsWidgetUtils

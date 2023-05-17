#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Math/BoundingBox.h>
#include <Foundation/Math/Declarations.h>
#include <GuiFoundation/Widgets/WidgetUtils.h>
#include <QApplication>
#include <QRect>

QScreen& wdWidgetUtils::GetClosestScreen(const QPoint& point)
{
  QScreen* pClosestScreen = QApplication::screenAt(point);
  if (pClosestScreen == nullptr)
  {
    QList<QScreen*> screens = QApplication::screens();
    float fShortestDistance = wdMath::Infinity<float>();
    for (QScreen* pScreen : screens)
    {
      const QRect geom = pScreen->geometry();
      wdBoundingBox wdGeom;
      wdGeom.SetCenterAndHalfExtents(wdVec3(geom.center().x(), geom.center().y(), 0), wdVec3(geom.width() / 2.0f, geom.height() / 2.0f, 0));
      const wdVec3 wdPoint(point.x(), point.y(), 0);
      if (wdGeom.Contains(wdPoint))
      {
        return *pScreen;
      }
      float fDistance = wdGeom.GetDistanceSquaredTo(wdPoint);
      if (fDistance < fShortestDistance)
      {
        fShortestDistance = fDistance;
        pClosestScreen = pScreen;
      }
    }
    WD_ASSERT_DEV(pClosestScreen != nullptr, "There are no screens connected, UI cannot function.");
  }
  return *pClosestScreen;
}

void wdWidgetUtils::AdjustGridDensity(
  double& ref_fFinestDensity, double& ref_fRoughDensity, wdUInt32 uiWindowWidth, double fViewportSceneWidth, wdUInt32 uiMinPixelsForStep)
{
  const double fMaxStepsFitInWindow = (double)uiWindowWidth / (double)uiMinPixelsForStep;

  const double fStartDensity = ref_fFinestDensity;

  wdInt32 iFactor = 1;
  double fNewDensity = ref_fFinestDensity;
  wdInt32 iFactors[2] = {5, 2};
  wdInt32 iLastFactor = 0;

  while (true)
  {
    const double fStepsAtDensity = fViewportSceneWidth / fNewDensity;

    if (fStepsAtDensity < fMaxStepsFitInWindow)
      break;

    iFactor *= iFactors[iLastFactor];
    fNewDensity = fStartDensity * iFactor;

    iLastFactor = (iLastFactor + 1) % 2;
  }

  ref_fFinestDensity = fStartDensity * iFactor;

  iFactor *= iFactors[iLastFactor];
  ref_fRoughDensity = fStartDensity * iFactor;
}

void wdWidgetUtils::ComputeGridExtentsX(const QRectF& viewportSceneRect, double fGridStops, double& out_fMinX, double& out_fMaxX)
{
  out_fMinX = wdMath::RoundDown((double)viewportSceneRect.left(), fGridStops);
  out_fMaxX = wdMath::RoundUp((double)viewportSceneRect.right(), fGridStops);
}

void wdWidgetUtils::ComputeGridExtentsY(const QRectF& viewportSceneRect, double fGridStops, double& out_fMinY, double& out_fMaxY)
{
  out_fMinY = wdMath::RoundDown((double)viewportSceneRect.top(), fGridStops);
  out_fMaxY = wdMath::RoundUp((double)viewportSceneRect.bottom(), fGridStops);
}

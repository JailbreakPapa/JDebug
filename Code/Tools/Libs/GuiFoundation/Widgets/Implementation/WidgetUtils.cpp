#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Math/BoundingBox.h>
#include <Foundation/Math/Declarations.h>
#include <GuiFoundation/Widgets/WidgetUtils.h>
#include <QApplication>
#include <QRect>

QScreen& nsWidgetUtils::GetClosestScreen(const QPoint& point)
{
  QScreen* pClosestScreen = QApplication::screenAt(point);
  if (pClosestScreen == nullptr)
  {
    QList<QScreen*> screens = QApplication::screens();
    float fShortestDistance = nsMath::Infinity<float>();
    for (QScreen* pScreen : screens)
    {
      const QRect geom = pScreen->geometry();
      nsBoundingBox nsGeom = nsBoundingBox::MakeFromCenterAndHalfExtents(nsVec3(geom.center().x(), geom.center().y(), 0), nsVec3(geom.width() / 2.0f, geom.height() / 2.0f, 0));
      const nsVec3 nsPoint(point.x(), point.y(), 0);
      if (nsGeom.Contains(nsPoint))
      {
        return *pScreen;
      }
      float fDistance = nsGeom.GetDistanceSquaredTo(nsPoint);
      if (fDistance < fShortestDistance)
      {
        fShortestDistance = fDistance;
        pClosestScreen = pScreen;
      }
    }
    NS_ASSERT_DEV(pClosestScreen != nullptr, "There are no screens connected, UI cannot function.");
  }
  return *pClosestScreen;
}

void nsWidgetUtils::AdjustGridDensity(
  double& ref_fFinestDensity, double& ref_fRoughDensity, nsUInt32 uiWindowWidth, double fViewportSceneWidth, nsUInt32 uiMinPixelsForStep)
{
  const double fMaxStepsFitInWindow = (double)uiWindowWidth / (double)uiMinPixelsForStep;

  const double fStartDensity = ref_fFinestDensity;

  nsInt32 iFactor = 1;
  double fNewDensity = ref_fFinestDensity;
  nsInt32 iFactors[2] = {5, 2};
  nsInt32 iLastFactor = 0;

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

void nsWidgetUtils::ComputeGridExtentsX(const QRectF& viewportSceneRect, double fGridStops, double& out_fMinX, double& out_fMaxX)
{
  out_fMinX = nsMath::RoundDown((double)viewportSceneRect.left(), fGridStops);
  out_fMaxX = nsMath::RoundUp((double)viewportSceneRect.right(), fGridStops);
}

void nsWidgetUtils::ComputeGridExtentsY(const QRectF& viewportSceneRect, double fGridStops, double& out_fMinY, double& out_fMaxY)
{
  out_fMinY = nsMath::RoundDown((double)viewportSceneRect.top(), fGridStops);
  out_fMaxY = nsMath::RoundUp((double)viewportSceneRect.bottom(), fGridStops);
}

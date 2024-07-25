#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Math/Color8UNorm.h>
#include <Foundation/Math/Math.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <GuiFoundation/Widgets/Curve1DEditorWidget.moc.h>
#include <QGraphicsItem>
#include <QGraphicsSceneEvent>
#include <QMenu>
#include <QPainterPath>
#include <ToolsFoundation/Project/ToolsProject.h>

nsDynamicArray<nsString> nsQtCurve1DEditorWidget::s_CurvePresets;

nsQtCurve1DEditorWidget::nsQtCurve1DEditorWidget(QWidget* pParent)
  : QWidget(pParent)
{
  setupUi(this);

  CurveEdit->SetGridBarWidget(GridBarWidget);

  connect(CurveEdit, &nsQtCurveEditWidget::DeleteControlPointsEvent, this, &nsQtCurve1DEditorWidget::onDeleteControlPoints);
  connect(CurveEdit, &nsQtCurveEditWidget::DoubleClickEvent, this, &nsQtCurve1DEditorWidget::onDoubleClick);
  connect(CurveEdit, &nsQtCurveEditWidget::MoveControlPointsEvent, this, &nsQtCurve1DEditorWidget::onMoveControlPoints);
  connect(CurveEdit, &nsQtCurveEditWidget::MoveTangentsEvent, this, &nsQtCurve1DEditorWidget::onMoveTangents);
  connect(CurveEdit, &nsQtCurveEditWidget::BeginOperationEvent, this, &nsQtCurve1DEditorWidget::onBeginOperation);
  connect(CurveEdit, &nsQtCurveEditWidget::EndOperationEvent, this, &nsQtCurve1DEditorWidget::onEndOperation);
  connect(CurveEdit, &nsQtCurveEditWidget::ScaleControlPointsEvent, this, &nsQtCurve1DEditorWidget::onScaleControlPoints);
  connect(CurveEdit, &nsQtCurveEditWidget::ContextMenuEvent, this, &nsQtCurve1DEditorWidget::onContextMenu);
  connect(CurveEdit, &nsQtCurveEditWidget::SelectionChangedEvent, this, &nsQtCurve1DEditorWidget::onSelectionChanged);
  connect(CurveEdit, &nsQtCurveEditWidget::MoveCurveEvent, this, &nsQtCurve1DEditorWidget::onMoveCurve);

  LinePosition->setEnabled(false);
  LineValue->setEnabled(false);

  if (s_CurvePresets.IsEmpty())
  {
    FindAllPresets();
  }
}

nsQtCurve1DEditorWidget::~nsQtCurve1DEditorWidget() = default;

void nsQtCurve1DEditorWidget::SetCurveExtents(double fLowerBound, double fUpperBound, bool bLowerIsFixed, bool bUpperIsFixed)
{
  CurveEdit->m_fLowerExtent = fLowerBound;
  CurveEdit->m_fUpperExtent = fUpperBound;
  CurveEdit->m_bLowerExtentFixed = bLowerIsFixed;
  CurveEdit->m_bUpperExtentFixed = bUpperIsFixed;
}

void nsQtCurve1DEditorWidget::SetCurveRanges(double fLowerRange, double fUpperRange)
{
  CurveEdit->m_fLowerRange = fLowerRange;
  CurveEdit->m_fUpperRange = fUpperRange;
}

void nsQtCurve1DEditorWidget::SetCurves(const nsCurveGroupData& curves)
{
  nsQtScopedUpdatesDisabled ud(this);
  nsQtScopedBlockSignals bs(this);

  m_Curves.CloneFrom(curves);

  CurveEdit->SetCurves(&m_Curves);
  m_fCurveDuration = CurveEdit->GetMaxCurveExtent();

  UpdateSpinBoxes();
}


void nsQtCurve1DEditorWidget::SetScrubberPosition(nsUInt64 uiTick)
{
  CurveEdit->SetScrubberPosition(uiTick / 4800.0);
}


void nsQtCurve1DEditorWidget::ClearSelection()
{
  CurveEdit->ClearSelection();
}

void nsQtCurve1DEditorWidget::FrameCurve()
{
  CurveEdit->FrameCurve();
}

void nsQtCurve1DEditorWidget::FrameSelection()
{
  CurveEdit->FrameSelection();
}

void nsQtCurve1DEditorWidget::MakeRepeatable(bool bAdjustLastPoint)
{
  Q_EMIT BeginOperationEvent("Make Curve Repeatable");

  for (nsUInt32 iCurveIdx = 0; iCurveIdx < m_Curves.m_Curves.GetCount(); ++iCurveIdx)
  {
    const auto& curve = *m_Curves.m_Curves[iCurveIdx];

    const nsUInt32 uiNumCps = curve.m_ControlPoints.GetCount();
    if (uiNumCps < 2)
      continue;

    nsInt64 iMinTick = curve.m_ControlPoints[0].m_iTick;
    nsInt64 iMaxTick = curve.m_ControlPoints[0].m_iTick;
    nsUInt32 uiMinCp = 0;
    nsUInt32 uiMaxCp = 0;

    for (nsUInt32 uiCpIdx = 1; uiCpIdx < uiNumCps; ++uiCpIdx)
    {
      const nsInt64 x = curve.m_ControlPoints[uiCpIdx].m_iTick;

      if (x < iMinTick)
      {
        iMinTick = x;
        uiMinCp = uiCpIdx;
      }
      if (x > iMaxTick)
      {
        iMaxTick = x;
        uiMaxCp = uiCpIdx;
      }
    }

    if (uiMinCp == uiMaxCp)
      continue;

    // copy data, the first Q_EMIT may change the backing store
    const nsCurveControlPointData cpLeft = curve.m_ControlPoints[uiMinCp];
    const nsCurveControlPointData cpRight = curve.m_ControlPoints[uiMaxCp];

    if (bAdjustLastPoint)
    {
      Q_EMIT CpMovedEvent(iCurveIdx, uiMaxCp, (nsInt64)(m_fCurveDuration * 4800.0), cpLeft.m_fValue);
      Q_EMIT TangentMovedEvent(iCurveIdx, uiMaxCp, -cpLeft.m_RightTangent.x, -cpLeft.m_RightTangent.y, false);
    }
    else
    {
      Q_EMIT CpMovedEvent(iCurveIdx, uiMinCp, 0, cpRight.m_fValue);
      Q_EMIT TangentMovedEvent(iCurveIdx, uiMinCp, -cpRight.m_LeftTangent.x, -cpRight.m_LeftTangent.y, true);
    }
  }

  Q_EMIT EndOperationEvent(true);
}

void nsQtCurve1DEditorWidget::NormalizeCurveX(nsUInt32 uiActiveCurve)
{
  if (uiActiveCurve >= m_Curves.m_Curves.GetCount())
    return;

  nsCurve1D CurveData;
  m_Curves.ConvertToRuntimeData(uiActiveCurve, CurveData);

  const nsUInt32 numCPs = CurveData.GetNumControlPoints();

  if (numCPs < 2)
    return;

  CurveData.RecomputeExtents();

  double minX, maxX;
  CurveData.QueryExtents(minX, maxX);

  if (minX == 0 && maxX == 1)
    return;

  Q_EMIT BeginOperationEvent("Normalize Curve (X)");

  const float rangeNorm = 1.0f / (maxX - minX);

  for (nsUInt32 i = 0; i < numCPs; ++i)
  {
    const auto& cp = CurveData.GetControlPoint(i);

    nsVec2d pos = cp.m_Position;
    pos.x -= minX;
    pos.x *= rangeNorm;

    Q_EMIT CpMovedEvent(uiActiveCurve, i, m_Curves.TickFromTime(nsTime::MakeFromSeconds(pos.x)), pos.y);

    nsVec2 lt = cp.m_LeftTangent;
    lt.x *= rangeNorm;
    Q_EMIT TangentMovedEvent(uiActiveCurve, i, lt.x, lt.y, false);

    nsVec2 rt = cp.m_RightTangent;
    rt.x *= rangeNorm;
    Q_EMIT TangentMovedEvent(uiActiveCurve, i, rt.x, rt.y, true);
  }

  Q_EMIT EndOperationEvent(true);

  FrameCurve();
}

void nsQtCurve1DEditorWidget::NormalizeCurveY(nsUInt32 uiActiveCurve)
{
  if (uiActiveCurve >= m_Curves.m_Curves.GetCount())
    return;

  nsCurve1D CurveData;
  m_Curves.ConvertToRuntimeData(uiActiveCurve, CurveData);

  const nsUInt32 numCPs = CurveData.GetNumControlPoints();

  if (numCPs < 2)
    return;

  nsCurve1D CurveDataSorted = CurveData;
  CurveDataSorted.SortControlPoints();
  CurveDataSorted.CreateLinearApproximation();

  double minY, maxY;
  CurveDataSorted.QueryExtremeValues(minY, maxY);

  if (minY == 0 && maxY == 1)
    return;

  Q_EMIT BeginOperationEvent("Normalize Curve (Y)");

  const float rangeNorm = 1.0f / (maxY - minY);

  for (nsUInt32 i = 0; i < numCPs; ++i)
  {
    const auto& cp = CurveData.GetControlPoint(i);

    nsVec2d pos = cp.m_Position;
    pos.y -= minY;
    pos.y *= rangeNorm;

    Q_EMIT CpMovedEvent(uiActiveCurve, i, m_Curves.TickFromTime(nsTime::MakeFromSeconds(pos.x)), pos.y);

    nsVec2 lt = cp.m_LeftTangent;
    lt.y *= rangeNorm;
    Q_EMIT TangentMovedEvent(uiActiveCurve, i, lt.x, lt.y, false);

    nsVec2 rt = cp.m_RightTangent;
    rt.y *= rangeNorm;
    Q_EMIT TangentMovedEvent(uiActiveCurve, i, rt.x, rt.y, true);
  }

  Q_EMIT EndOperationEvent(true);

  FrameCurve();
}

struct PtToDelete
{
  NS_DECLARE_POD_TYPE();

  nsUInt32 m_uiCurveIdx;
  nsUInt32 m_uiPointIdx;

  bool operator<(const PtToDelete& rhs) const { return m_uiPointIdx > rhs.m_uiPointIdx; }
};

void nsQtCurve1DEditorWidget::ClearAllPoints()
{
  Q_EMIT BeginCpChangesEvent("Delete Points");

  nsHybridArray<PtToDelete, 16> delOrder;

  for (nsUInt32 curveIdx = 0; curveIdx < m_Curves.m_Curves.GetCount(); ++curveIdx)
  {
    nsCurve1D curveData;
    m_Curves.m_Curves[curveIdx]->ConvertToRuntimeData(curveData);

    for (nsUInt32 i = 0; i < curveData.GetNumControlPoints(); ++i)
    {
      auto& pt = delOrder.ExpandAndGetRef();
      pt.m_uiCurveIdx = curveIdx;
      pt.m_uiPointIdx = i;
    }
  }

  delOrder.Sort();

  // Delete sorted from back to front to prevent point indices becoming invalidated
  for (const auto& pt : delOrder)
  {
    Q_EMIT CpDeletedEvent(pt.m_uiCurveIdx, pt.m_uiPointIdx);
  }

  m_Curves.Clear();

  Q_EMIT EndCpChangesEvent();
}

void nsQtCurve1DEditorWidget::MirrorHorizontally(nsUInt32 uiActiveCurve)
{
  if (uiActiveCurve >= m_Curves.m_Curves.GetCount())
    return;

  nsCurve1D CurveData;
  m_Curves.ConvertToRuntimeData(uiActiveCurve, CurveData);

  const nsUInt32 numCPs = CurveData.GetNumControlPoints();

  if (numCPs < 2)
    return;

  CurveData.RecomputeExtents();

  double minX, maxX;
  CurveData.QueryExtents(minX, maxX);

  double centerX = minX + (maxX - minX) * 0.5;

  Q_EMIT BeginOperationEvent("Mirror Curve Horizontally");

  for (nsUInt32 i = 0; i < numCPs; ++i)
  {
    const auto& cp = CurveData.GetControlPoint(i);

    // mirror position around center
    nsVec2d pos = cp.m_Position;
    pos.x = centerX - (pos.x - centerX);

    Q_EMIT CpMovedEvent(uiActiveCurve, i, m_Curves.TickFromTime(nsTime::MakeFromSeconds(pos.x)), pos.y);

    nsVec2 lt = cp.m_RightTangent;
    nsVec2 rt = cp.m_LeftTangent;

    lt.x = -lt.x;
    rt.x = -rt.x;

    // swap tangents from left to right
    Q_EMIT TangentMovedEvent(uiActiveCurve, i, lt.x, lt.y, false);
    Q_EMIT TangentMovedEvent(uiActiveCurve, i, rt.x, rt.y, true);

    // swap tangent modes from left to right
    Q_EMIT CpTangentModeEvent(uiActiveCurve, i, false, (int)cp.m_TangentModeRight.GetValue());
    Q_EMIT CpTangentModeEvent(uiActiveCurve, i, true, (int)cp.m_TangentModeLeft.GetValue());
  }

  Q_EMIT EndOperationEvent(true);
}

void nsQtCurve1DEditorWidget::MirrorVertically(nsUInt32 uiActiveCurve)
{
  if (uiActiveCurve >= m_Curves.m_Curves.GetCount())
    return;

  nsCurve1D CurveData;
  m_Curves.ConvertToRuntimeData(uiActiveCurve, CurveData);

  const nsUInt32 numCPs = CurveData.GetNumControlPoints();

  if (numCPs < 2)
    return;

  CurveData.SortControlPoints();
  CurveData.CreateLinearApproximation();

  double minY, maxY;
  CurveData.QueryExtremeValues(minY, maxY);

  double centerY = minY + (maxY - minY) * 0.5;

  Q_EMIT BeginOperationEvent("Mirror Curve Vertically");

  for (nsUInt32 i = 0; i < numCPs; ++i)
  {
    const auto& cp = CurveData.GetControlPoint(i);

    // mirror value around center
    nsVec2d pos = cp.m_Position;
    pos.y = centerY - (pos.y - centerY);

    Q_EMIT CpMovedEvent(uiActiveCurve, i, m_Curves.TickFromTime(nsTime::MakeFromSeconds(pos.x)), pos.y);

    nsVec2 lt = cp.m_LeftTangent;
    nsVec2 rt = cp.m_RightTangent;

    lt.y = -lt.y;
    rt.y = -rt.y;

    // swap tangents Y directions
    Q_EMIT TangentMovedEvent(uiActiveCurve, i, lt.x, lt.y, false);
    Q_EMIT TangentMovedEvent(uiActiveCurve, i, rt.x, rt.y, true);
  }

  Q_EMIT EndOperationEvent(true);
}

void nsQtCurve1DEditorWidget::onDeleteControlPoints()
{
  const auto selection = CurveEdit->GetSelection();

  if (selection.IsEmpty())
    return;

  CurveEdit->ClearSelection();

  Q_EMIT BeginCpChangesEvent("Delete Points");

  nsHybridArray<PtToDelete, 16> delOrder;

  for (const auto& item : selection)
  {
    auto& pt = delOrder.ExpandAndGetRef();
    pt.m_uiCurveIdx = item.m_uiCurve;
    pt.m_uiPointIdx = item.m_uiPoint;
  }

  delOrder.Sort();

  // delete sorted from back to front to prevent point indices becoming invalidated
  for (const auto& pt : delOrder)
  {
    Q_EMIT CpDeletedEvent(pt.m_uiCurveIdx, pt.m_uiPointIdx);
  }

  Q_EMIT EndCpChangesEvent();
}

void nsQtCurve1DEditorWidget::onDoubleClick(const QPointF& scenePos, const QPointF& epsilon)
{
  Q_EMIT BeginCpChangesEvent("Add Control Point");

  InsertCpAt(scenePos.x(), scenePos.y(), nsVec2d(nsMath::Abs(epsilon.x()), nsMath::Abs(epsilon.y())));

  Q_EMIT EndCpChangesEvent();
}

void nsQtCurve1DEditorWidget::onMoveControlPoints(double x, double y)
{
  m_vControlPointMove += nsVec2d(x, y);

  const auto selection = CurveEdit->GetSelection();

  if (selection.IsEmpty())
    return;

  Q_EMIT BeginCpChangesEvent("Move Points");

  for (const auto& cpSel : selection)
  {
    const auto& cp = m_CurvesBackup.m_Curves[cpSel.m_uiCurve]->m_ControlPoints[cpSel.m_uiPoint];
    nsVec2d newPos = nsVec2d(cp.GetTickAsTime().GetSeconds(), cp.m_fValue) + m_vControlPointMove;

    ClampPoint(newPos.x, newPos.y);

    Q_EMIT CpMovedEvent(cpSel.m_uiCurve, cpSel.m_uiPoint, m_Curves.TickFromTime(nsTime::MakeFromSeconds(newPos.x)), newPos.y);
  }

  Q_EMIT EndCpChangesEvent();
}

void nsQtCurve1DEditorWidget::onScaleControlPoints(QPointF refPt, double scaleX, double scaleY)
{
  const auto selection = CurveEdit->GetSelection();

  if (selection.IsEmpty())
    return;

  const nsVec2d ref(refPt.x(), refPt.y());
  const nsVec2d scale(scaleX, scaleY);

  Q_EMIT BeginCpChangesEvent("Scale Points");

  for (const auto& cpSel : selection)
  {
    const auto& cp = m_CurvesBackup.m_Curves[cpSel.m_uiCurve]->m_ControlPoints[cpSel.m_uiPoint];
    nsVec2d newPos = ref + (nsVec2d(cp.GetTickAsTime().GetSeconds(), cp.m_fValue) - ref).CompMul(scale);

    ClampPoint(newPos.x, newPos.y);

    Q_EMIT CpMovedEvent(cpSel.m_uiCurve, cpSel.m_uiPoint, m_Curves.TickFromTime(nsTime::MakeFromSeconds(newPos.x)), newPos.y);
  }

  Q_EMIT EndCpChangesEvent();
}

void nsQtCurve1DEditorWidget::onMoveTangents(float x, float y)
{
  m_vTangentMove += nsVec2(x, y);

  nsInt32 iCurve;
  nsInt32 iPoint;
  bool bLeftTangent;

  if (!CurveEdit->GetSelectedTangent(iCurve, iPoint, bLeftTangent))
    return;

  Q_EMIT BeginCpChangesEvent("Move Tangents");

  {
    const auto& cp = m_CurvesBackup.m_Curves[iCurve]->m_ControlPoints[iPoint];
    nsVec2 newPos;

    if (bLeftTangent)
      newPos = cp.m_LeftTangent + m_vTangentMove;
    else
      newPos = cp.m_RightTangent + m_vTangentMove;

    newPos.y = nsMath::Clamp(newPos.y, -100000.0f, +100000.0f);

    Q_EMIT TangentMovedEvent(iCurve, iPoint, newPos.x, newPos.y, !bLeftTangent);

    if (cp.m_bTangentsLinked)
    {
      Q_EMIT TangentMovedEvent(iCurve, iPoint, -newPos.x, -newPos.y, bLeftTangent);
    }
  }

  Q_EMIT EndCpChangesEvent();
}

void nsQtCurve1DEditorWidget::onBeginOperation(QString name)
{
  m_CurvesBackup.CloneFrom(m_Curves);
  m_vTangentMove.SetZero();
  m_vControlPointMove.SetZero();

  Q_EMIT BeginOperationEvent(name);
}

void nsQtCurve1DEditorWidget::onEndOperation(bool commit)
{
  Q_EMIT EndOperationEvent(commit);
}

void nsQtCurve1DEditorWidget::onContextMenu(QPoint pos, QPointF scenePos)
{
  const bool bIsCurveNonEmpty = !m_Curves.m_Curves.IsEmpty() && !m_Curves.m_Curves[0]->m_ControlPoints.IsEmpty();

  m_ContextMenuScenePos = scenePos;

  QMenu m(this);
  m.setDefaultAction(m.addAction("Add Point\tDbl Click", this, SLOT(onAddPoint())));

  const auto& selection = CurveEdit->GetSelection();

  if (bIsCurveNonEmpty)
  {
    QMenu* cmSel = m.addMenu("Selection");
    cmSel->addAction("Select All\tCtrl+A", this, [this]()
      { CurveEdit->SelectAll(); });

    if (!selection.IsEmpty())
    {
      cmSel->addAction("Clear Selection\tESC", this, [this]()
        { CurveEdit->ClearSelection(); });

      cmSel->addAction(
        "Frame Selection\tShift+F", this, [this]()
        { FrameSelection(); });

      cmSel->addSeparator();

      cmSel->addAction("Delete Points\tDel", this, SLOT(onDeleteControlPoints()));
      cmSel->addSeparator();
      cmSel->addAction("Link Tangents", this, SLOT(onLinkTangents()));
      cmSel->addAction("Break Tangents", this, SLOT(onBreakTangents()));
      cmSel->addAction("Flatten Tangents", this, SLOT(onFlattenTangents()));

      QMenu* cmLT = cmSel->addMenu("Left Tangents");
      QMenu* cmRT = cmSel->addMenu("Right Tangents");
      QMenu* cmBT = cmSel->addMenu("Both Tangents");

      cmLT->addAction("Auto", this, [this]()
        { SetTangentMode(nsCurveTangentMode::Auto, true, false); });
      cmLT->addAction("Bnsier", this, [this]()
        { SetTangentMode(nsCurveTangentMode::Bnsier, true, false); });
      cmLT->addAction("Fixed Length", this, [this]()
        { SetTangentMode(nsCurveTangentMode::FixedLength, true, false); });
      cmLT->addAction("Linear", this, [this]()
        { SetTangentMode(nsCurveTangentMode::Linear, true, false); });

      cmRT->addAction("Auto", this, [this]()
        { SetTangentMode(nsCurveTangentMode::Auto, false, true); });
      cmRT->addAction("Bnsier", this, [this]()
        { SetTangentMode(nsCurveTangentMode::Bnsier, false, true); });
      cmRT->addAction("Fixed Length", this, [this]()
        { SetTangentMode(nsCurveTangentMode::FixedLength, false, true); });
      cmRT->addAction("Linear", this, [this]()
        { SetTangentMode(nsCurveTangentMode::Linear, false, true); });

      cmBT->addAction("Auto", this, [this]()
        { SetTangentMode(nsCurveTangentMode::Auto, true, true); });
      cmBT->addAction("Bnsier", this, [this]()
        { SetTangentMode(nsCurveTangentMode::Bnsier, true, true); });
      cmBT->addAction("Fixed Length", this, [this]()
        { SetTangentMode(nsCurveTangentMode::FixedLength, true, true); });
      cmBT->addAction("Linear", this, [this]()
        { SetTangentMode(nsCurveTangentMode::Linear, true, true); });
    }

    {
      QMenu* cm = m.addMenu("Curve");
      cm->addSeparator();
      cm->addAction("Mirror Horizontally", this, [this]()
        { MirrorHorizontally(0); });
      cm->addAction("Mirror Vertically", this, [this]()
        { MirrorVertically(0); });
      cm->addAction("Normalize X", this, [this]()
        { NormalizeCurveX(0); });
      cm->addAction("Normalize Y", this, [this]()
        { NormalizeCurveY(0); });
      cm->addAction("Loop: Adjust Last Point", this, [this]()
        { MakeRepeatable(true); });
      cm->addAction("Loop: Adjust First Point", this, [this]()
        { MakeRepeatable(false); });
      cm->addAction("Clear Curve", this, [this]()
        { ClearAllPoints(); });

      cm->addAction("Frame Curve\tCtrl+F", this, [this]()
        { FrameCurve(); });
    }
  }

  QMenu* presentsMenu = m.addMenu("Presets");

  {
    if (bIsCurveNonEmpty)
    {
      presentsMenu->addAction("Save As Preset...", this, &nsQtCurve1DEditorWidget::onSaveAsPreset);
    }

    presentsMenu->addAction("Load Preset...", this, &nsQtCurve1DEditorWidget::onLoadPreset);
    presentsMenu->addSeparator();
  }

  {
    QMenu* curveMenu = presentsMenu->addMenu("0 -> 1 (slow)");

    // clang-format off
    curveMenu->addAction("Linear", this, [this]() { onGenerateCurve(nsCurveFunction::Linear, false); });
    curveMenu->addAction("Sine", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInSine, false); });
    curveMenu->addAction("Quad", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInQuad, false); });
    curveMenu->addAction("Cubic", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInCubic, false); });
    curveMenu->addAction("Quartic", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInQuartic, false); });
    curveMenu->addAction("Quintic", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInQuintic, false); });
    curveMenu->addAction("Exponential", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInExpo, false); });
    curveMenu->addAction("Overshoot", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInBack, false); });
    curveMenu->addAction("Elastic", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInElastic, false); });
    curveMenu->addAction("Bounce", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInBounce, false); });
    // clang-format on
  }

  {
    QMenu* curveMenu = presentsMenu->addMenu("0 -> 1 (fast)");

    // clang-format off
    curveMenu->addAction("Linear", this, [this]() { onGenerateCurve(nsCurveFunction::Linear, false); });
    curveMenu->addAction("Sine", this, [this]() { onGenerateCurve(nsCurveFunction::EaseOutSine, false); });
    curveMenu->addAction("Quad", this, [this]() { onGenerateCurve(nsCurveFunction::EaseOutQuad, false); });
    curveMenu->addAction("Cubic", this, [this]() { onGenerateCurve(nsCurveFunction::EaseOutCubic, false); });
    curveMenu->addAction("Quartic", this, [this]() { onGenerateCurve(nsCurveFunction::EaseOutQuartic, false); });
    curveMenu->addAction("Quintic", this, [this]() { onGenerateCurve(nsCurveFunction::EaseOutQuintic, false); });
    curveMenu->addAction("Exponential", this, [this]() { onGenerateCurve(nsCurveFunction::EaseOutExpo, false); });
    curveMenu->addAction("Overshoot", this, [this]() { onGenerateCurve(nsCurveFunction::EaseOutBack, false); });
    curveMenu->addAction("Elastic", this, [this]() { onGenerateCurve(nsCurveFunction::EaseOutElastic, false); });
    curveMenu->addAction("Bounce", this, [this]() { onGenerateCurve(nsCurveFunction::EaseOutBounce, false); });
    // clang-format on
  }

  {
    QMenu* curveMenu = presentsMenu->addMenu("0 -> 1 (s/f/s)");

    // clang-format off
    curveMenu->addAction("Sine", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInOutSine, false); });
    curveMenu->addAction("Quad", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInOutQuad, false); });
    curveMenu->addAction("Cubic", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInOutCubic, false); });
    curveMenu->addAction("Quartic", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInOutQuartic, false); });
    curveMenu->addAction("Quintic", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInOutQuintic, false); });
    curveMenu->addAction("Exponential", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInOutExpo, false); });
    curveMenu->addAction("Overshoot", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInOutBack, false); });
    curveMenu->addAction("Elastic", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInOutElastic, false); });
    curveMenu->addAction("Bounce", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInOutBounce, false); });
    // clang-format on
  }

  {
    QMenu* curveMenu = presentsMenu->addMenu("1 -> 0 (slow)");

    // clang-format off
    curveMenu->addAction("Linear", this, [this]() { onGenerateCurve(nsCurveFunction::Linear, true); });
    curveMenu->addAction("Sine", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInSine, true); });
    curveMenu->addAction("Quad", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInQuad, true); });
    curveMenu->addAction("Cubic", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInCubic, true); });
    curveMenu->addAction("Quartic", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInQuartic, true); });
    curveMenu->addAction("Quintic", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInQuintic, true); });
    curveMenu->addAction("Exponential", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInExpo, true); });
    curveMenu->addAction("Overshoot", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInBack, true); });
    curveMenu->addAction("Elastic", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInElastic, true); });
    curveMenu->addAction("Bounce", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInBounce, true); });
    // clang-format on
  }

  {
    QMenu* curveMenu = presentsMenu->addMenu("1 -> 0 (fast)");

    // clang-format off
    curveMenu->addAction("Linear", this, [this]() { onGenerateCurve(nsCurveFunction::Linear, true); });
    curveMenu->addAction("Sine", this, [this]() { onGenerateCurve(nsCurveFunction::EaseOutSine, true); });
    curveMenu->addAction("Quad", this, [this]() { onGenerateCurve(nsCurveFunction::EaseOutQuad, true); });
    curveMenu->addAction("Cubic", this, [this]() { onGenerateCurve(nsCurveFunction::EaseOutCubic, true); });
    curveMenu->addAction("Quartic", this, [this]() { onGenerateCurve(nsCurveFunction::EaseOutQuartic, true); });
    curveMenu->addAction("Quintic", this, [this]() { onGenerateCurve(nsCurveFunction::EaseOutQuintic, true); });
    curveMenu->addAction("Exponential", this, [this]() { onGenerateCurve(nsCurveFunction::EaseOutExpo, true); });
    curveMenu->addAction("Overshoot", this, [this]() { onGenerateCurve(nsCurveFunction::EaseOutBack, true); });
    curveMenu->addAction("Elastic", this, [this]() { onGenerateCurve(nsCurveFunction::EaseOutElastic, true); });
    curveMenu->addAction("Bounce", this, [this]() { onGenerateCurve(nsCurveFunction::EaseOutBounce, true); });
    // clang-format on
  }

  {
    QMenu* curveMenu = presentsMenu->addMenu("1 -> 0 (s/f/s)");

    // clang-format off
    curveMenu->addAction("Sine", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInOutSine, true); });
    curveMenu->addAction("Quad", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInOutQuad, true); });
    curveMenu->addAction("Cubic", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInOutCubic, true); });
    curveMenu->addAction("Quartic", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInOutQuartic, true); });
    curveMenu->addAction("Quintic", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInOutQuintic, true); });
    curveMenu->addAction("Exponential", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInOutExpo, true); });
    curveMenu->addAction("Overshoot", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInOutBack, true); });
    curveMenu->addAction("Elastic", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInOutElastic, true); });
    curveMenu->addAction("Bounce", this, [this]() { onGenerateCurve(nsCurveFunction::EaseInOutBounce, true); });
    // clang-format on
  }

  {
    QMenu* curveMenu = presentsMenu->addMenu("0 -> 1 -> 0");

    // clang-format off
    curveMenu->addAction("Conical", this, [this]() { onGenerateCurve(nsCurveFunction::Conical, false); });
    curveMenu->addAction("Fade In / Fade Out", this, [this]() { onGenerateCurve(nsCurveFunction::FadeInFadeOut, false); });
    curveMenu->addAction("Fade In / Hold / Fade Out", this, [this]() { onGenerateCurve(nsCurveFunction::FadeInHoldFadeOut, false); });
    curveMenu->addAction("Bell", this, [this]() { onGenerateCurve(nsCurveFunction::Bell, false); });
    // clang-format on
  }

  // Show all available presets from disk in a hierarchical menu structure
  {
    nsMap<nsString, QMenu*> subMenus;
    subMenus[""] = presentsMenu;

    auto GetSubMenu = [&](const nsStringBuilder& sPath, auto getSubMenu2)
    {
      auto it = subMenus.Find(sPath);
      if (it.IsValid())
        return it.Value();

      nsStringBuilder parent = sPath;
      parent.PathParentDirectory();
      parent.Trim("/");

      QMenu* pParentMenu = getSubMenu2(parent, getSubMenu2);
      QMenu* pMenu = pParentMenu->addMenu(sPath.GetFileName().GetData(parent));
      subMenus[sPath] = pMenu;

      return pMenu;
    };

    nsStringBuilder sPresetName, sPresetPath;
    for (const auto& preset : s_CurvePresets)
    {
      sPresetPath = nsPathUtils::GetFileDirectory(preset);
      sPresetName = nsPathUtils::GetFileName(preset);

      sPresetPath.Trim("/");

      GetSubMenu(sPresetPath, GetSubMenu)->addAction(sPresetName.GetData(), [this, preset]()
        { LoadCurvePreset(preset).IgnoreResult(); });
    }
  }

  m.exec(pos);
}

void nsQtCurve1DEditorWidget::onAddPoint()
{
  Q_EMIT BeginCpChangesEvent("Add Control Point");

  InsertCpAt(m_ContextMenuScenePos.x(), m_ContextMenuScenePos.y(), nsVec2d::MakeZero());

  Q_EMIT EndCpChangesEvent();
}

void nsQtCurve1DEditorWidget::onLinkTangents()
{
  const auto& selection = CurveEdit->GetSelection();

  Q_EMIT BeginOperationEvent("Link Tangents");

  for (const auto& cpSel : selection)
  {
    Q_EMIT TangentLinkEvent(cpSel.m_uiCurve, cpSel.m_uiPoint, true);
  }

  Q_EMIT EndOperationEvent(true);
}

void nsQtCurve1DEditorWidget::onBreakTangents()
{
  const auto& selection = CurveEdit->GetSelection();

  Q_EMIT BeginOperationEvent("Break Tangents");

  for (const auto& cpSel : selection)
  {
    Q_EMIT TangentLinkEvent(cpSel.m_uiCurve, cpSel.m_uiPoint, false);
  }

  Q_EMIT EndOperationEvent(true);
}


void nsQtCurve1DEditorWidget::onFlattenTangents()
{
  const auto& selection = CurveEdit->GetSelection();

  Q_EMIT BeginOperationEvent("Flatten Tangents");

  for (const auto& cpSel : selection)
  {
    // don't use references, the signals may move the data in memory
    const nsVec2 tL = m_Curves.m_Curves[cpSel.m_uiCurve]->m_ControlPoints[cpSel.m_uiPoint].m_LeftTangent;
    const nsVec2 tR = m_Curves.m_Curves[cpSel.m_uiCurve]->m_ControlPoints[cpSel.m_uiPoint].m_RightTangent;

    // clamp the X position, to prevent tangents with zero length
    Q_EMIT TangentMovedEvent(cpSel.m_uiCurve, cpSel.m_uiPoint, nsMath::Min(tL.x, -0.02f), 0, false);
    Q_EMIT TangentMovedEvent(cpSel.m_uiCurve, cpSel.m_uiPoint, nsMath::Max(tR.x, +0.02f), 0, true);
  }

  Q_EMIT EndOperationEvent(true);
}

void nsQtCurve1DEditorWidget::InsertCpAt(double posX, double value, nsVec2d epsilon)
{
  int curveIdx = 0, cpIdx = 0;

  ClampPoint(posX, value);

  // do not insert at a point where a CP already exists
  if (PickControlPointAt(posX, value, epsilon, curveIdx, cpIdx))
    return;

  if (!PickCurveAt(posX, value, epsilon.y, curveIdx, value))
  {
    // by default insert into curve 0
    curveIdx = 0;
  }

  Q_EMIT InsertCpEvent(curveIdx, m_Curves.TickFromTime(nsTime::MakeFromSeconds(posX)), value);
}


bool nsQtCurve1DEditorWidget::PickCurveAt(double x, double y, double fMaxDistanceY, nsInt32& out_iCurveIdx, double& out_ValueY) const
{
  out_iCurveIdx = -1;
  nsCurve1D CurveData;

  for (nsUInt32 i = 0; i < m_Curves.m_Curves.GetCount(); ++i)
  {
    m_Curves.ConvertToRuntimeData(i, CurveData);

    CurveData.SortControlPoints();
    CurveData.CreateLinearApproximation();

    double minVal, maxVal;
    CurveData.QueryExtents(minVal, maxVal);

    const double val = CurveData.Evaluate(x);

    const double dist = nsMath::Abs(val - y);
    if (dist < fMaxDistanceY)
    {
      fMaxDistanceY = dist;
      out_iCurveIdx = i;
      out_ValueY = val;
    }
  }

  return out_iCurveIdx >= 0;
}

bool nsQtCurve1DEditorWidget::PickControlPointAt(double x, double y, nsVec2d vMaxDistance, nsInt32& out_iCurveIdx, nsInt32& out_iCpIdx) const
{
  const nsVec2d at(x, y);

  out_iCurveIdx = -1;
  out_iCpIdx = -1;

  nsCurve1D CurveData;

  for (nsUInt32 iCurve = 0; iCurve < m_Curves.m_Curves.GetCount(); ++iCurve)
  {
    m_Curves.ConvertToRuntimeData(iCurve, CurveData);

    for (nsUInt32 iCP = 0; iCP < CurveData.GetNumControlPoints(); ++iCP)
    {
      const auto& cp = CurveData.GetControlPoint(iCP);
      const nsVec2d dist = cp.m_Position - at;

      if (nsMath::Abs(dist.x) <= vMaxDistance.x && nsMath::Abs(dist.y) <= vMaxDistance.y)
      {
        vMaxDistance.x = nsMath::Abs(dist.x);
        vMaxDistance.y = nsMath::Abs(dist.y);

        out_iCurveIdx = iCurve;
        out_iCpIdx = iCP;
      }
    }
  }

  return out_iCpIdx >= 0;
}

void nsQtCurve1DEditorWidget::onSelectionChanged()
{
  UpdateSpinBoxes();
}


void nsQtCurve1DEditorWidget::onMoveCurve(nsInt32 iCurve, double moveY)
{
  m_vControlPointMove.y += moveY;

  Q_EMIT BeginCpChangesEvent("Move Curve");

  const auto& curve = *m_CurvesBackup.m_Curves[iCurve];
  nsUInt32 uiNumCps = curve.m_ControlPoints.GetCount();
  for (nsUInt32 i = 0; i < uiNumCps; ++i)
  {
    const nsInt64 x = curve.m_ControlPoints[i].m_iTick;
    const float y = curve.m_ControlPoints[i].m_fValue + m_vControlPointMove.y;

    Q_EMIT CpMovedEvent(iCurve, i, x, y);
  }

  Q_EMIT EndCpChangesEvent();
}

void nsQtCurve1DEditorWidget::onGenerateCurve(nsCurveFunction::Enum function, bool inverse)
{
  Q_EMIT BeginCpChangesEvent("Generate Curve");

  // Delete all existing control points
  ClearAllPoints();

  nsCurve1D cmp;

  const nsUInt32 uiFrames = m_Curves.m_uiFramesPerSecond / 2;
  const double invFps = 1.0 / uiFrames;

  struct Sample
  {
    double m_fPos = 0;
    double m_fCorrectValue = 0;
    bool m_bInserted = false;
  };

  nsHybridArray<Sample, 60> samples;
  samples.SetCount(uiFrames + 1);

  for (nsUInt32 i = 0; i <= uiFrames; ++i)
  {
    const double x = i * invFps;

    samples[i].m_fPos = x;
    samples[i].m_fCorrectValue = nsCurveFunction::GetValue(function, x, inverse);
  }

  auto AddPt = [&](nsUInt32 uiIdx)
  {
    samples[uiIdx].m_bInserted = true;
    const double x = samples[uiIdx].m_fPos;
    const double y = samples[uiIdx].m_fCorrectValue;

    cmp.AddControlPoint(x).m_Position.y = y;
    InsertCpAt(x, y, nsVec2d::MakeZero());
  };

  AddPt(0);
  AddPt(samples.GetCount() - 1);

  // only add points that are necessary to reach the target curve within a certain error threshold
  // we find the point that has the highest error (along Y) and insert that
  // then we check all points again and find the next worst point, until no point violates the error threshold anymore
  // this loop is O(n*n), but apparently no problem for the 30 samples that we currently use
  // this loop is O(n*n), but apparently no problem for the 30 samples that we currently use
  while (true)
  {
    cmp.SortControlPoints();
    cmp.CreateLinearApproximation();

    double fMaxError = 0.03; // this is the error threshold
    nsUInt32 uiMaxErrorIdx = 0xffffffff;

    for (nsUInt32 idx = 0; idx < samples.GetCount(); ++idx)
    {
      auto& sample = samples[idx];
      if (sample.m_bInserted)
        continue;

      const double eval = cmp.Evaluate(sample.m_fPos);
      const double err = nsMath::Abs(eval - sample.m_fCorrectValue);

      if (err > fMaxError)
      {
        fMaxError = eval;
        uiMaxErrorIdx = idx;
      }
    }

    if (uiMaxErrorIdx == 0xffffffff)
      break;

    AddPt(uiMaxErrorIdx);
  }

  Q_EMIT EndCpChangesEvent();
}

static QString s_sPresetSaveDir;

void nsQtCurve1DEditorWidget::onSaveAsPreset()
{
  if (s_sPresetSaveDir.isEmpty())
  {
    s_sPresetSaveDir = nsToolsProject::GetSingleton()->GetProjectDirectory().GetData();
    s_sPresetSaveDir.append("/Editor/Presets/Curves");

    nsOSFile::CreateDirectoryStructure(s_sPresetSaveDir.toUtf8().data()).IgnoreResult();
  }

  QString sFile = QFileDialog::getSaveFileName(QApplication::activeWindow(), "Save Curve as Preset", s_sPresetSaveDir, "Curve Presets (*.nsCurvePreset)", nullptr, QFileDialog::Option::DontResolveSymlinks);

  if (sFile.isEmpty())
    return;

  s_sPresetSaveDir = sFile;

  SaveCurvePreset(sFile.toUtf8().data());

  FindAllPresets();
}

void nsQtCurve1DEditorWidget::SaveCurvePreset(const char* szFile) const
{
  nsFileWriter file;
  if (file.Open(szFile).Failed())
    return;

  file.WriteVersion(1);

  file << m_Curves.m_uiFramesPerSecond;
  file << m_Curves.m_Curves.GetCount();

  NS_ASSERT_DEBUG(m_Curves.m_Curves.GetCount() == 1, "Only one curve at a time is currently supported.");

  for (nsUInt32 curveIdx = 0; curveIdx < m_Curves.m_Curves.GetCount(); ++curveIdx)
  {
    const auto& curve = m_Curves.m_Curves[curveIdx];
    file << curve->m_CurveColor;
    file << curve->m_ControlPoints.GetCount();

    for (nsUInt32 cpIdx = 0; cpIdx < curve->m_ControlPoints.GetCount(); ++cpIdx)
    {
      const auto& cp = curve->m_ControlPoints[cpIdx];

      file << cp.m_iTick;
      file << cp.m_fValue;
      file << cp.m_bTangentsLinked;
      file << cp.m_LeftTangentMode;
      file << cp.m_RightTangentMode;
      file << cp.m_LeftTangent;
      file << cp.m_RightTangent;
    }
  }
}

void nsQtCurve1DEditorWidget::onLoadPreset()
{
  if (s_sPresetSaveDir.isEmpty())
  {
    s_sPresetSaveDir = nsToolsProject::GetSingleton()->GetProjectDirectory().GetData();
    s_sPresetSaveDir.append("/Editor/Presets/Curves");

    if (!nsOSFile::ExistsDirectory(s_sPresetSaveDir.toUtf8().data()))
    {
      // maybe fall back to the Base directory instead ?
      nsOSFile::CreateDirectoryStructure(s_sPresetSaveDir.toUtf8().data()).IgnoreResult();
    }
  }

  QString sFile = QFileDialog::getOpenFileName(QApplication::activeWindow(), "Load Curve from Preset", s_sPresetSaveDir, "Curve Presets (*.nsCurvePreset)", nullptr, QFileDialog::Option::DontResolveSymlinks);

  if (sFile.isEmpty())
    return;

  s_sPresetSaveDir = sFile;

  LoadCurvePreset(sFile.toUtf8().data()).IgnoreResult();
}

nsResult nsQtCurve1DEditorWidget::LoadCurvePreset(const char* szFile)
{
  nsStringBuilder sPath = szFile;

  if (!sPath.IsAbsolutePath())
  {
    sPath.Prepend("Editor/Presets/Curves/");
  }

  nsFileReader file;
  if (file.Open(sPath).Failed())
    return NS_FAILURE;

  const nsTypeVersion version = file.ReadVersion(1);
  NS_IGNORE_UNUSED(version);

  Q_EMIT BeginCpChangesEvent("Load Preset");

  // Delete all existing control points
  ClearAllPoints();

  file >> m_Curves.m_uiFramesPerSecond;

  nsUInt32 uiNumCurves = 0;
  file >> uiNumCurves;

  NS_ASSERT_DEBUG(uiNumCurves == 1, "Only one curve at a time is currently supported.");
  uiNumCurves = 1;

  for (nsUInt32 curveIdx = 0; curveIdx < uiNumCurves; ++curveIdx)
  {
    nsColorGammaUB curveColor;
    nsUInt32 uiNumCPs = 0;
    file >> curveColor;
    file >> uiNumCPs;

    for (nsUInt32 cpIdx = 0; cpIdx < uiNumCPs; ++cpIdx)
    {
      nsInt64 iTick = 0;
      double fValue = 0;
      bool bTangentsLinked = false;
      nsEnum<nsCurveTangentMode> LeftTangentMode;
      nsEnum<nsCurveTangentMode> RightTangentMode;
      nsVec2 LeftTangent;
      nsVec2 RightTangent;

      file >> iTick;
      file >> fValue;
      file >> bTangentsLinked;
      file >> LeftTangentMode;
      file >> RightTangentMode;
      file >> LeftTangent;
      file >> RightTangent;

      Q_EMIT InsertCpEvent(curveIdx, iTick, fValue);
      Q_EMIT TangentLinkEvent(curveIdx, cpIdx, bTangentsLinked);
      Q_EMIT CpTangentModeEvent(curveIdx, cpIdx, false, LeftTangentMode.GetValue());
      Q_EMIT CpTangentModeEvent(curveIdx, cpIdx, true, RightTangentMode.GetValue());
      Q_EMIT TangentMovedEvent(curveIdx, cpIdx, LeftTangent.x, LeftTangent.y, false);
      Q_EMIT TangentMovedEvent(curveIdx, cpIdx, RightTangent.x, RightTangent.y, true);
    }
  }

  Q_EMIT EndCpChangesEvent();

  return NS_SUCCESS;
}

void nsQtCurve1DEditorWidget::FindAllPresets()
{
  s_CurvePresets.Clear();

#if NS_ENABLED(NS_SUPPORTS_FILE_ITERATORS)

  nsFileSystemIterator fsIt;

  nsFileSystem::StartSearch(fsIt, "Editor/Presets/Curves", nsFileSystemIteratorFlags::ReportFilesRecursive);

  nsStringBuilder sFilePath;

  for (; fsIt.IsValid(); fsIt.Next())
  {
    if (!nsPathUtils::HasExtension(fsIt.GetStats().m_sName, "nsCurvePreset"))
      continue;

    fsIt.GetStats().GetFullPath(sFilePath);
    sFilePath.MakeCleanPath();

    sFilePath.MakeRelativeTo(fsIt.GetCurrentSearchTerm()).AssertSuccess();

    s_CurvePresets.PushBack(sFilePath);
  }

  s_CurvePresets.Sort();

#endif
}

void nsQtCurve1DEditorWidget::UpdateSpinBoxes()
{
  const auto& selection = CurveEdit->GetSelection();

  nsQtScopedBlockSignals _1(LinePosition, LineValue);

  if (selection.IsEmpty())
  {
    LinePosition->setText(QString());
    LineValue->setText(QString());

    LinePosition->setEnabled(false);
    LineValue->setEnabled(false);
    return;
  }

  const auto& pt0 = m_Curves.m_Curves[selection[0].m_uiCurve]->m_ControlPoints[selection[0].m_uiPoint];
  const double fPos = pt0.GetTickAsTime().GetSeconds();
  const double fVal = pt0.m_fValue;

  LinePosition->setEnabled(true);
  LineValue->setEnabled(true);

  bool bMultipleTicks = false;
  for (nsUInt32 i = 1; i < selection.GetCount(); ++i)
  {
    const auto& pt = m_Curves.m_Curves[selection[i].m_uiCurve]->m_ControlPoints[selection[i].m_uiPoint];

    if (pt.GetTickAsTime().GetSeconds() != fPos)
    {
      bMultipleTicks = true;
      break;
    }
  }

  bool bMultipleValues = false;
  for (nsUInt32 i = 1; i < selection.GetCount(); ++i)
  {
    const auto& pt = m_Curves.m_Curves[selection[i].m_uiCurve]->m_ControlPoints[selection[i].m_uiPoint];

    if (pt.m_fValue != fVal)
    {
      bMultipleValues = true;
      LineValue->setText(QString());
      break;
    }
  }

  LinePosition->setText(bMultipleTicks ? QString() : QString::number(fPos, 'f', 2));
  LineValue->setText(bMultipleValues ? QString() : QString::number(fVal, 'f', 3));
}

void nsQtCurve1DEditorWidget::on_LinePosition_editingFinished()
{
  QString sValue = LinePosition->text();

  bool ok = false;
  const double value = sValue.toDouble(&ok);
  if (!ok)
    return;

  if (value < 0)
    return;

  const auto& selection = CurveEdit->GetSelection();
  if (selection.IsEmpty())
    return;

  Q_EMIT BeginCpChangesEvent("Set Time");

  for (const auto& cpSel : selection)
  {
    const auto& cp = m_Curves.m_Curves[cpSel.m_uiCurve]->m_ControlPoints[cpSel.m_uiPoint];

    const nsInt64 iTick = m_Curves.TickFromTime(nsTime::MakeFromSeconds(value));
    if (cp.m_iTick != iTick)
      Q_EMIT CpMovedEvent(cpSel.m_uiCurve, cpSel.m_uiPoint, iTick, cp.m_fValue);
  }

  Q_EMIT EndCpChangesEvent();
}

void nsQtCurve1DEditorWidget::on_LineValue_editingFinished()
{
  QString sValue = LineValue->text();

  bool ok = false;
  const double value = sValue.toDouble(&ok);
  if (!ok)
    return;

  const auto& selection = CurveEdit->GetSelection();
  if (selection.IsEmpty())
    return;

  Q_EMIT BeginCpChangesEvent("Set Value");

  for (const auto& cpSel : selection)
  {
    const auto& cp = m_Curves.m_Curves[cpSel.m_uiCurve]->m_ControlPoints[cpSel.m_uiPoint];

    if (cp.m_fValue != value)
      Q_EMIT CpMovedEvent(cpSel.m_uiCurve, cpSel.m_uiPoint, cp.m_iTick, value);
  }

  Q_EMIT EndCpChangesEvent();
}

void nsQtCurve1DEditorWidget::SetTangentMode(nsCurveTangentMode::Enum mode, bool bLeft, bool bRight)
{
  const auto& selection = CurveEdit->GetSelection();
  if (selection.IsEmpty())
    return;

  Q_EMIT BeginCpChangesEvent("Set Tangent Mode");

  for (const auto& cpSel : selection)
  {
    if (bLeft)
      Q_EMIT CpTangentModeEvent(cpSel.m_uiCurve, cpSel.m_uiPoint, false, (int)mode);

    if (bRight)
      Q_EMIT CpTangentModeEvent(cpSel.m_uiCurve, cpSel.m_uiPoint, true, (int)mode);
  }

  Q_EMIT EndCpChangesEvent();
}

void nsQtCurve1DEditorWidget::ClampPoint(double& x, double& y) const
{
  if (CurveEdit->m_bLowerExtentFixed)
    x = nsMath::Max(x, CurveEdit->m_fLowerExtent);
  if (CurveEdit->m_bUpperExtentFixed)
    x = nsMath::Min(x, CurveEdit->m_fUpperExtent);

  y = nsMath::Clamp(y, CurveEdit->m_fLowerRange, CurveEdit->m_fUpperRange);
}

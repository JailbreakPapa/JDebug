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

wdDynamicArray<wdString> wdQtCurve1DEditorWidget::s_CurvePresets;

wdQtCurve1DEditorWidget::wdQtCurve1DEditorWidget(QWidget* pParent)
  : QWidget(pParent)
{
  setupUi(this);

  CurveEdit->SetGridBarWidget(GridBarWidget);

  connect(CurveEdit, &wdQtCurveEditWidget::DeleteControlPointsEvent, this, &wdQtCurve1DEditorWidget::onDeleteControlPoints);
  connect(CurveEdit, &wdQtCurveEditWidget::DoubleClickEvent, this, &wdQtCurve1DEditorWidget::onDoubleClick);
  connect(CurveEdit, &wdQtCurveEditWidget::MoveControlPointsEvent, this, &wdQtCurve1DEditorWidget::onMoveControlPoints);
  connect(CurveEdit, &wdQtCurveEditWidget::MoveTangentsEvent, this, &wdQtCurve1DEditorWidget::onMoveTangents);
  connect(CurveEdit, &wdQtCurveEditWidget::BeginOperationEvent, this, &wdQtCurve1DEditorWidget::onBeginOperation);
  connect(CurveEdit, &wdQtCurveEditWidget::EndOperationEvent, this, &wdQtCurve1DEditorWidget::onEndOperation);
  connect(CurveEdit, &wdQtCurveEditWidget::ScaleControlPointsEvent, this, &wdQtCurve1DEditorWidget::onScaleControlPoints);
  connect(CurveEdit, &wdQtCurveEditWidget::ContextMenuEvent, this, &wdQtCurve1DEditorWidget::onContextMenu);
  connect(CurveEdit, &wdQtCurveEditWidget::SelectionChangedEvent, this, &wdQtCurve1DEditorWidget::onSelectionChanged);
  connect(CurveEdit, &wdQtCurveEditWidget::MoveCurveEvent, this, &wdQtCurve1DEditorWidget::onMoveCurve);

  LinePosition->setEnabled(false);
  LineValue->setEnabled(false);

  if (s_CurvePresets.IsEmpty())
  {
    FindAllPresets();
  }
}

wdQtCurve1DEditorWidget::~wdQtCurve1DEditorWidget() {}

void wdQtCurve1DEditorWidget::SetCurveExtents(double fLowerBound, double fUpperBound, bool bLowerIsFixed, bool bUpperIsFixed)
{
  CurveEdit->m_fLowerExtent = fLowerBound;
  CurveEdit->m_fUpperExtent = fUpperBound;
  CurveEdit->m_bLowerExtentFixed = bLowerIsFixed;
  CurveEdit->m_bUpperExtentFixed = bUpperIsFixed;
}

void wdQtCurve1DEditorWidget::SetCurveRanges(double fLowerRange, double fUpperRange)
{
  CurveEdit->m_fLowerRange = fLowerRange;
  CurveEdit->m_fUpperRange = fUpperRange;
}

void wdQtCurve1DEditorWidget::SetCurves(const wdCurveGroupData& curves)
{
  wdQtScopedUpdatesDisabled ud(this);
  wdQtScopedBlockSignals bs(this);

  m_Curves.CloneFrom(curves);

  CurveEdit->SetCurves(&m_Curves);
  m_fCurveDuration = CurveEdit->GetMaxCurveExtent();

  UpdateSpinBoxes();
}


void wdQtCurve1DEditorWidget::SetScrubberPosition(wdUInt64 uiTick)
{
  CurveEdit->SetScrubberPosition(uiTick / 4800.0);
}


void wdQtCurve1DEditorWidget::ClearSelection()
{
  CurveEdit->ClearSelection();
}

void wdQtCurve1DEditorWidget::FrameCurve()
{
  CurveEdit->FrameCurve();
}

void wdQtCurve1DEditorWidget::FrameSelection()
{
  CurveEdit->FrameSelection();
}

void wdQtCurve1DEditorWidget::MakeRepeatable(bool bAdjustLastPoint)
{
  Q_EMIT BeginOperationEvent("Make Curve Repeatable");

  for (wdUInt32 iCurveIdx = 0; iCurveIdx < m_Curves.m_Curves.GetCount(); ++iCurveIdx)
  {
    const auto& curve = *m_Curves.m_Curves[iCurveIdx];

    const wdUInt32 uiNumCps = curve.m_ControlPoints.GetCount();
    if (uiNumCps < 2)
      continue;

    wdInt64 iMinTick = curve.m_ControlPoints[0].m_iTick;
    wdInt64 iMaxTick = curve.m_ControlPoints[0].m_iTick;
    wdUInt32 uiMinCp = 0;
    wdUInt32 uiMaxCp = 0;

    for (wdUInt32 uiCpIdx = 1; uiCpIdx < uiNumCps; ++uiCpIdx)
    {
      const wdInt64 x = curve.m_ControlPoints[uiCpIdx].m_iTick;

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
    const wdCurveControlPointData cpLeft = curve.m_ControlPoints[uiMinCp];
    const wdCurveControlPointData cpRight = curve.m_ControlPoints[uiMaxCp];

    if (bAdjustLastPoint)
    {
      Q_EMIT CpMovedEvent(iCurveIdx, uiMaxCp, (wdInt64)(m_fCurveDuration * 4800.0), cpLeft.m_fValue);
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

void wdQtCurve1DEditorWidget::NormalizeCurveX(wdUInt32 uiActiveCurve)
{
  if (uiActiveCurve >= m_Curves.m_Curves.GetCount())
    return;

  wdCurve1D CurveData;
  m_Curves.ConvertToRuntimeData(uiActiveCurve, CurveData);

  const wdUInt32 numCPs = CurveData.GetNumControlPoints();

  if (numCPs < 2)
    return;

  CurveData.RecomputeExtents();

  double minX, maxX;
  CurveData.QueryExtents(minX, maxX);

  if (minX == 0 && maxX == 1)
    return;

  Q_EMIT BeginOperationEvent("Normalize Curve (X)");

  const float rangeNorm = 1.0f / (maxX - minX);

  for (wdUInt32 i = 0; i < numCPs; ++i)
  {
    const auto& cp = CurveData.GetControlPoint(i);

    wdVec2d pos = cp.m_Position;
    pos.x -= minX;
    pos.x *= rangeNorm;

    Q_EMIT CpMovedEvent(uiActiveCurve, i, m_Curves.TickFromTime(wdTime::Seconds(pos.x)), pos.y);

    wdVec2 lt = cp.m_LeftTangent;
    lt.x *= rangeNorm;
    Q_EMIT TangentMovedEvent(uiActiveCurve, i, lt.x, lt.y, false);

    wdVec2 rt = cp.m_RightTangent;
    rt.x *= rangeNorm;
    Q_EMIT TangentMovedEvent(uiActiveCurve, i, rt.x, rt.y, true);
  }

  Q_EMIT EndOperationEvent(true);

  FrameCurve();
}

void wdQtCurve1DEditorWidget::NormalizeCurveY(wdUInt32 uiActiveCurve)
{
  if (uiActiveCurve >= m_Curves.m_Curves.GetCount())
    return;

  wdCurve1D CurveData;
  m_Curves.ConvertToRuntimeData(uiActiveCurve, CurveData);

  const wdUInt32 numCPs = CurveData.GetNumControlPoints();

  if (numCPs < 2)
    return;

  wdCurve1D CurveDataSorted = CurveData;
  CurveDataSorted.SortControlPoints();
  CurveDataSorted.CreateLinearApproximation();

  double minY, maxY;
  CurveDataSorted.QueryExtremeValues(minY, maxY);

  if (minY == 0 && maxY == 1)
    return;

  Q_EMIT BeginOperationEvent("Normalize Curve (Y)");

  const float rangeNorm = 1.0f / (maxY - minY);

  for (wdUInt32 i = 0; i < numCPs; ++i)
  {
    const auto& cp = CurveData.GetControlPoint(i);

    wdVec2d pos = cp.m_Position;
    pos.y -= minY;
    pos.y *= rangeNorm;

    Q_EMIT CpMovedEvent(uiActiveCurve, i, m_Curves.TickFromTime(wdTime::Seconds(pos.x)), pos.y);

    wdVec2 lt = cp.m_LeftTangent;
    lt.y *= rangeNorm;
    Q_EMIT TangentMovedEvent(uiActiveCurve, i, lt.x, lt.y, false);

    wdVec2 rt = cp.m_RightTangent;
    rt.y *= rangeNorm;
    Q_EMIT TangentMovedEvent(uiActiveCurve, i, rt.x, rt.y, true);
  }

  Q_EMIT EndOperationEvent(true);

  FrameCurve();
}

struct PtToDelete
{
  WD_DECLARE_POD_TYPE();

  wdUInt32 m_uiCurveIdx;
  wdUInt32 m_uiPointIdx;

  bool operator<(const PtToDelete& rhs) const { return m_uiPointIdx > rhs.m_uiPointIdx; }
};

void wdQtCurve1DEditorWidget::ClearAllPoints()
{
  Q_EMIT BeginCpChangesEvent("Delete Points");

  wdHybridArray<PtToDelete, 16> delOrder;

  for (wdUInt32 curveIdx = 0; curveIdx < m_Curves.m_Curves.GetCount(); ++curveIdx)
  {
    wdCurve1D curveData;
    m_Curves.m_Curves[curveIdx]->ConvertToRuntimeData(curveData);

    for (wdUInt32 i = 0; i < curveData.GetNumControlPoints(); ++i)
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

void wdQtCurve1DEditorWidget::MirrorHorizontally(wdUInt32 uiActiveCurve)
{
  if (uiActiveCurve >= m_Curves.m_Curves.GetCount())
    return;

  wdCurve1D CurveData;
  m_Curves.ConvertToRuntimeData(uiActiveCurve, CurveData);

  const wdUInt32 numCPs = CurveData.GetNumControlPoints();

  if (numCPs < 2)
    return;

  CurveData.RecomputeExtents();

  double minX, maxX;
  CurveData.QueryExtents(minX, maxX);

  double centerX = minX + (maxX - minX) * 0.5;

  Q_EMIT BeginOperationEvent("Mirror Curve Horizontally");

  for (wdUInt32 i = 0; i < numCPs; ++i)
  {
    const auto& cp = CurveData.GetControlPoint(i);

    // mirror position around center
    wdVec2d pos = cp.m_Position;
    pos.x = centerX - (pos.x - centerX);

    Q_EMIT CpMovedEvent(uiActiveCurve, i, m_Curves.TickFromTime(wdTime::Seconds(pos.x)), pos.y);

    wdVec2 lt = cp.m_RightTangent;
    wdVec2 rt = cp.m_LeftTangent;

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

void wdQtCurve1DEditorWidget::MirrorVertically(wdUInt32 uiActiveCurve)
{
  if (uiActiveCurve >= m_Curves.m_Curves.GetCount())
    return;

  wdCurve1D CurveData;
  m_Curves.ConvertToRuntimeData(uiActiveCurve, CurveData);

  const wdUInt32 numCPs = CurveData.GetNumControlPoints();

  if (numCPs < 2)
    return;

  CurveData.SortControlPoints();
  CurveData.CreateLinearApproximation();

  double minY, maxY;
  CurveData.QueryExtremeValues(minY, maxY);

  double centerY = minY + (maxY - minY) * 0.5;

  Q_EMIT BeginOperationEvent("Mirror Curve Vertically");

  for (wdUInt32 i = 0; i < numCPs; ++i)
  {
    const auto& cp = CurveData.GetControlPoint(i);

    // mirror value around center
    wdVec2d pos = cp.m_Position;
    pos.y = centerY - (pos.y - centerY);

    Q_EMIT CpMovedEvent(uiActiveCurve, i, m_Curves.TickFromTime(wdTime::Seconds(pos.x)), pos.y);

    wdVec2 lt = cp.m_LeftTangent;
    wdVec2 rt = cp.m_RightTangent;

    lt.y = -lt.y;
    rt.y = -rt.y;

    // swap tangents Y directions
    Q_EMIT TangentMovedEvent(uiActiveCurve, i, lt.x, lt.y, false);
    Q_EMIT TangentMovedEvent(uiActiveCurve, i, rt.x, rt.y, true);
  }

  Q_EMIT EndOperationEvent(true);
}

void wdQtCurve1DEditorWidget::onDeleteControlPoints()
{
  const auto selection = CurveEdit->GetSelection();

  if (selection.IsEmpty())
    return;

  CurveEdit->ClearSelection();

  Q_EMIT BeginCpChangesEvent("Delete Points");

  wdHybridArray<PtToDelete, 16> delOrder;

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

void wdQtCurve1DEditorWidget::onDoubleClick(const QPointF& scenePos, const QPointF& epsilon)
{
  Q_EMIT BeginCpChangesEvent("Add Control Point");

  InsertCpAt(scenePos.x(), scenePos.y(), wdVec2d(wdMath::Abs(epsilon.x()), wdMath::Abs(epsilon.y())));

  Q_EMIT EndCpChangesEvent();
}

void wdQtCurve1DEditorWidget::onMoveControlPoints(double x, double y)
{
  m_vControlPointMove += wdVec2d(x, y);

  const auto selection = CurveEdit->GetSelection();

  if (selection.IsEmpty())
    return;

  Q_EMIT BeginCpChangesEvent("Move Points");

  for (const auto& cpSel : selection)
  {
    const auto& cp = m_CurvesBackup.m_Curves[cpSel.m_uiCurve]->m_ControlPoints[cpSel.m_uiPoint];
    wdVec2d newPos = wdVec2d(cp.GetTickAsTime().GetSeconds(), cp.m_fValue) + m_vControlPointMove;

    ClampPoint(newPos.x, newPos.y);

    Q_EMIT CpMovedEvent(cpSel.m_uiCurve, cpSel.m_uiPoint, m_Curves.TickFromTime(wdTime::Seconds(newPos.x)), newPos.y);
  }

  Q_EMIT EndCpChangesEvent();
}

void wdQtCurve1DEditorWidget::onScaleControlPoints(QPointF refPt, double scaleX, double scaleY)
{
  const auto selection = CurveEdit->GetSelection();

  if (selection.IsEmpty())
    return;

  const wdVec2d ref(refPt.x(), refPt.y());
  const wdVec2d scale(scaleX, scaleY);

  Q_EMIT BeginCpChangesEvent("Scale Points");

  for (const auto& cpSel : selection)
  {
    const auto& cp = m_CurvesBackup.m_Curves[cpSel.m_uiCurve]->m_ControlPoints[cpSel.m_uiPoint];
    wdVec2d newPos = ref + (wdVec2d(cp.GetTickAsTime().GetSeconds(), cp.m_fValue) - ref).CompMul(scale);

    ClampPoint(newPos.x, newPos.y);

    Q_EMIT CpMovedEvent(cpSel.m_uiCurve, cpSel.m_uiPoint, m_Curves.TickFromTime(wdTime::Seconds(newPos.x)), newPos.y);
  }

  Q_EMIT EndCpChangesEvent();
}

void wdQtCurve1DEditorWidget::onMoveTangents(float x, float y)
{
  m_vTangentMove += wdVec2(x, y);

  wdInt32 iCurve;
  wdInt32 iPoint;
  bool bLeftTangent;

  if (!CurveEdit->GetSelectedTangent(iCurve, iPoint, bLeftTangent))
    return;

  Q_EMIT BeginCpChangesEvent("Move Tangents");

  {
    const auto& cp = m_CurvesBackup.m_Curves[iCurve]->m_ControlPoints[iPoint];
    wdVec2 newPos;

    if (bLeftTangent)
      newPos = cp.m_LeftTangent + m_vTangentMove;
    else
      newPos = cp.m_RightTangent + m_vTangentMove;

    newPos.y = wdMath::Clamp(newPos.y, -100000.0f, +100000.0f);

    Q_EMIT TangentMovedEvent(iCurve, iPoint, newPos.x, newPos.y, !bLeftTangent);

    if (cp.m_bTangentsLinked)
    {
      Q_EMIT TangentMovedEvent(iCurve, iPoint, -newPos.x, -newPos.y, bLeftTangent);
    }
  }

  Q_EMIT EndCpChangesEvent();
}

void wdQtCurve1DEditorWidget::onBeginOperation(QString name)
{
  m_CurvesBackup.CloneFrom(m_Curves);
  m_vTangentMove.SetZero();
  m_vControlPointMove.SetZero();

  Q_EMIT BeginOperationEvent(name);
}

void wdQtCurve1DEditorWidget::onEndOperation(bool commit)
{
  Q_EMIT EndOperationEvent(commit);
}

void wdQtCurve1DEditorWidget::onContextMenu(QPoint pos, QPointF scenePos)
{
  const bool bIsCurveNonEmpty = !m_Curves.m_Curves.IsEmpty() && !m_Curves.m_Curves[0]->m_ControlPoints.IsEmpty();

  m_ContextMenuScenePos = scenePos;

  QMenu m(this);
  m.setDefaultAction(m.addAction("Add Point\tDbl Click", this, SLOT(onAddPoint())));

  const auto& selection = CurveEdit->GetSelection();

  if (bIsCurveNonEmpty)
  {
    QMenu* cmSel = m.addMenu("Selection");
    cmSel->addAction("Select All\tCtrl+A", this, [this]() { CurveEdit->SelectAll(); });

    if (!selection.IsEmpty())
    {
      cmSel->addAction("Clear Selection\tESC", this, [this]() { CurveEdit->ClearSelection(); });

      cmSel->addAction(
        "Frame Selection\tShift+F", this, [this]() { FrameSelection(); });

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
        { SetTangentMode(wdCurveTangentMode::Auto, true, false); });
      cmLT->addAction("Bwdier", this, [this]()
        { SetTangentMode(wdCurveTangentMode::Bwdier, true, false); });
      cmLT->addAction("Fixed Length", this, [this]()
        { SetTangentMode(wdCurveTangentMode::FixedLength, true, false); });
      cmLT->addAction("Linear", this, [this]()
        { SetTangentMode(wdCurveTangentMode::Linear, true, false); });

      cmRT->addAction("Auto", this, [this]()
        { SetTangentMode(wdCurveTangentMode::Auto, false, true); });
      cmRT->addAction("Bwdier", this, [this]()
        { SetTangentMode(wdCurveTangentMode::Bwdier, false, true); });
      cmRT->addAction("Fixed Length", this, [this]()
        { SetTangentMode(wdCurveTangentMode::FixedLength, false, true); });
      cmRT->addAction("Linear", this, [this]()
        { SetTangentMode(wdCurveTangentMode::Linear, false, true); });

      cmBT->addAction("Auto", this, [this]()
        { SetTangentMode(wdCurveTangentMode::Auto, true, true); });
      cmBT->addAction("Bwdier", this, [this]()
        { SetTangentMode(wdCurveTangentMode::Bwdier, true, true); });
      cmBT->addAction("Fixed Length", this, [this]()
        { SetTangentMode(wdCurveTangentMode::FixedLength, true, true); });
      cmBT->addAction("Linear", this, [this]()
        { SetTangentMode(wdCurveTangentMode::Linear, true, true); });
    }

    {
      QMenu* cm = m.addMenu("Curve");
      cm->addSeparator();
      cm->addAction("Mirror Horizontally", this, [this]() { MirrorHorizontally(0); });
      cm->addAction("Mirror Vertically", this, [this]() { MirrorVertically(0); });
      cm->addAction("Normalize X", this, [this]() { NormalizeCurveX(0); });
      cm->addAction("Normalize Y", this, [this]() { NormalizeCurveY(0); });
      cm->addAction("Loop: Adjust Last Point", this, [this]() { MakeRepeatable(true); });
      cm->addAction("Loop: Adjust First Point", this, [this]() { MakeRepeatable(false); });
      cm->addAction("Clear Curve", this, [this]() { ClearAllPoints(); });

      cm->addAction("Frame Curve\tCtrl+F", this, [this]() { FrameCurve(); });
    }
  }

  QMenu* presentsMenu = m.addMenu("Presets");

  {
    if (bIsCurveNonEmpty)
    {
      presentsMenu->addAction("Save As Preset...", this, &wdQtCurve1DEditorWidget::onSaveAsPreset);
    }

    presentsMenu->addAction("Load Preset...", this, &wdQtCurve1DEditorWidget::onLoadPreset);
    presentsMenu->addSeparator();
  }

  {
    QMenu* curveMenu = presentsMenu->addMenu("0 -> 1 (slow)");

    // clang-format off
    curveMenu->addAction("Linear", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::Linear, false); });
    curveMenu->addAction("Sine", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInSine, false); });
    curveMenu->addAction("Quad", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInQuad, false); });
    curveMenu->addAction("Cubic", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInCubic, false); });
    curveMenu->addAction("Quartic", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInQuartic, false); });
    curveMenu->addAction("Quintic", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInQuintic, false); });
    curveMenu->addAction("Exponential", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInExpo, false); });
    curveMenu->addAction("Overshoot", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInBack, false); });
    curveMenu->addAction("Elastic", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInElastic, false); });
    curveMenu->addAction("Bounce", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInBounce, false); });
    // clang-format on
  }

  {
    QMenu* curveMenu = presentsMenu->addMenu("0 -> 1 (fast)");

    // clang-format off
    curveMenu->addAction("Linear", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::Linear, false); });
    curveMenu->addAction("Sine", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseOutSine, false); });
    curveMenu->addAction("Quad", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseOutQuad, false); });
    curveMenu->addAction("Cubic", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseOutCubic, false); });
    curveMenu->addAction("Quartic", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseOutQuartic, false); });
    curveMenu->addAction("Quintic", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseOutQuintic, false); });
    curveMenu->addAction("Exponential", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseOutExpo, false); });
    curveMenu->addAction("Overshoot", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseOutBack, false); });
    curveMenu->addAction("Elastic", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseOutElastic, false); });
    curveMenu->addAction("Bounce", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseOutBounce, false); });
    // clang-format on
  }

  {
    QMenu* curveMenu = presentsMenu->addMenu("0 -> 1 (s/f/s)");

    // clang-format off
    curveMenu->addAction("Sine", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInOutSine, false); });
    curveMenu->addAction("Quad", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInOutQuad, false); });
    curveMenu->addAction("Cubic", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInOutCubic, false); });
    curveMenu->addAction("Quartic", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInOutQuartic, false); });
    curveMenu->addAction("Quintic", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInOutQuintic, false); });
    curveMenu->addAction("Exponential", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInOutExpo, false); });
    curveMenu->addAction("Overshoot", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInOutBack, false); });
    curveMenu->addAction("Elastic", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInOutElastic, false); });
    curveMenu->addAction("Bounce", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInOutBounce, false); });
    // clang-format on
  }

  {
    QMenu* curveMenu = presentsMenu->addMenu("1 -> 0 (slow)");

    // clang-format off
    curveMenu->addAction("Linear", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::Linear, true); });
    curveMenu->addAction("Sine", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInSine, true); });
    curveMenu->addAction("Quad", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInQuad, true); });
    curveMenu->addAction("Cubic", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInCubic, true); });
    curveMenu->addAction("Quartic", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInQuartic, true); });
    curveMenu->addAction("Quintic", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInQuintic, true); });
    curveMenu->addAction("Exponential", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInExpo, true); });
    curveMenu->addAction("Overshoot", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInBack, true); });
    curveMenu->addAction("Elastic", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInElastic, true); });
    curveMenu->addAction("Bounce", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInBounce, true); });
    // clang-format on
  }

  {
    QMenu* curveMenu = presentsMenu->addMenu("1 -> 0 (fast)");

    // clang-format off
    curveMenu->addAction("Linear", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::Linear, true); });
    curveMenu->addAction("Sine", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseOutSine, true); });
    curveMenu->addAction("Quad", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseOutQuad, true); });
    curveMenu->addAction("Cubic", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseOutCubic, true); });
    curveMenu->addAction("Quartic", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseOutQuartic, true); });
    curveMenu->addAction("Quintic", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseOutQuintic, true); });
    curveMenu->addAction("Exponential", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseOutExpo, true); });
    curveMenu->addAction("Overshoot", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseOutBack, true); });
    curveMenu->addAction("Elastic", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseOutElastic, true); });
    curveMenu->addAction("Bounce", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseOutBounce, true); });
    // clang-format on
  }

  {
    QMenu* curveMenu = presentsMenu->addMenu("1 -> 0 (s/f/s)");

    // clang-format off
    curveMenu->addAction("Sine", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInOutSine, true); });
    curveMenu->addAction("Quad", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInOutQuad, true); });
    curveMenu->addAction("Cubic", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInOutCubic, true); });
    curveMenu->addAction("Quartic", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInOutQuartic, true); });
    curveMenu->addAction("Quintic", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInOutQuintic, true); });
    curveMenu->addAction("Exponential", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInOutExpo, true); });
    curveMenu->addAction("Overshoot", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInOutBack, true); });
    curveMenu->addAction("Elastic", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInOutElastic, true); });
    curveMenu->addAction("Bounce", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::EaseInOutBounce, true); });
    // clang-format on
  }

  {
    QMenu* curveMenu = presentsMenu->addMenu("0 -> 1 -> 0");

    // clang-format off
    curveMenu->addAction("Conical", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::Conical, false); });
    curveMenu->addAction("Fade In / Fade Out", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::FadeInFadeOut, false); });
    curveMenu->addAction("Fade In / Hold / Fade Out", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::FadeInHoldFadeOut, false); });
    curveMenu->addAction("Bell", this, [this]() { onGenerateCurve(wdMath::wdCurveFunction::Bell, false); });
    // clang-format on
  }

  // Show all available presets from disk in a hierarchical menu structure
  {
    wdMap<wdString, QMenu*> subMenus;
    subMenus[""] = presentsMenu;

    auto GetSubMenu = [&](const wdStringBuilder& sPath, auto getSubMenu2) {
      auto it = subMenus.Find(sPath);
      if (it.IsValid())
        return it.Value();

      wdStringBuilder parent = sPath;
      parent.PathParentDirectory();
      parent.Trim("/");

      QMenu* pParentMenu = getSubMenu2(parent, getSubMenu2);
      QMenu* pMenu = pParentMenu->addMenu(sPath.GetFileName().GetData(parent));
      subMenus[sPath] = pMenu;

      return pMenu;
    };

    wdStringBuilder sPresetName, sPresetPath;
    for (const auto& preset : s_CurvePresets)
    {
      sPresetPath = wdPathUtils::GetFileDirectory(preset);
      sPresetName = wdPathUtils::GetFileName(preset);

      sPresetPath.Trim("/");

      GetSubMenu(sPresetPath, GetSubMenu)->addAction(sPresetName.GetData(), [this, preset]() { LoadCurvePreset(preset).IgnoreResult(); });
    }
  }

  m.exec(pos);
}

void wdQtCurve1DEditorWidget::onAddPoint()
{
  Q_EMIT BeginCpChangesEvent("Add Control Point");

  InsertCpAt(m_ContextMenuScenePos.x(), m_ContextMenuScenePos.y(), wdVec2d::ZeroVector());

  Q_EMIT EndCpChangesEvent();
}

void wdQtCurve1DEditorWidget::onLinkTangents()
{
  const auto& selection = CurveEdit->GetSelection();

  Q_EMIT BeginOperationEvent("Link Tangents");

  for (const auto& cpSel : selection)
  {
    Q_EMIT TangentLinkEvent(cpSel.m_uiCurve, cpSel.m_uiPoint, true);
  }

  Q_EMIT EndOperationEvent(true);
}

void wdQtCurve1DEditorWidget::onBreakTangents()
{
  const auto& selection = CurveEdit->GetSelection();

  Q_EMIT BeginOperationEvent("Break Tangents");

  for (const auto& cpSel : selection)
  {
    Q_EMIT TangentLinkEvent(cpSel.m_uiCurve, cpSel.m_uiPoint, false);
  }

  Q_EMIT EndOperationEvent(true);
}


void wdQtCurve1DEditorWidget::onFlattenTangents()
{
  const auto& selection = CurveEdit->GetSelection();

  Q_EMIT BeginOperationEvent("Flatten Tangents");

  for (const auto& cpSel : selection)
  {
    // don't use references, the signals may move the data in memory
    const wdVec2 tL = m_Curves.m_Curves[cpSel.m_uiCurve]->m_ControlPoints[cpSel.m_uiPoint].m_LeftTangent;
    const wdVec2 tR = m_Curves.m_Curves[cpSel.m_uiCurve]->m_ControlPoints[cpSel.m_uiPoint].m_RightTangent;

    // clamp the X position, to prevent tangents with zero length
    Q_EMIT TangentMovedEvent(cpSel.m_uiCurve, cpSel.m_uiPoint, wdMath::Min(tL.x, -0.02f), 0, false);
    Q_EMIT TangentMovedEvent(cpSel.m_uiCurve, cpSel.m_uiPoint, wdMath::Max(tR.x, +0.02f), 0, true);
  }

  Q_EMIT EndOperationEvent(true);
}

void wdQtCurve1DEditorWidget::InsertCpAt(double posX, double value, wdVec2d epsilon)
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

  Q_EMIT InsertCpEvent(curveIdx, m_Curves.TickFromTime(wdTime::Seconds(posX)), value);
}


bool wdQtCurve1DEditorWidget::PickCurveAt(double x, double y, double fMaxDistanceY, wdInt32& out_iCurveIdx, double& out_ValueY) const
{
  out_iCurveIdx = -1;
  wdCurve1D CurveData;

  for (wdUInt32 i = 0; i < m_Curves.m_Curves.GetCount(); ++i)
  {
    m_Curves.ConvertToRuntimeData(i, CurveData);

    CurveData.SortControlPoints();
    CurveData.CreateLinearApproximation();

    double minVal, maxVal;
    CurveData.QueryExtents(minVal, maxVal);

    const double val = CurveData.Evaluate(x);

    const double dist = wdMath::Abs(val - y);
    if (dist < fMaxDistanceY)
    {
      fMaxDistanceY = dist;
      out_iCurveIdx = i;
      out_ValueY = val;
    }
  }

  return out_iCurveIdx >= 0;
}

bool wdQtCurve1DEditorWidget::PickControlPointAt(double x, double y, wdVec2d vMaxDistance, wdInt32& out_iCurveIdx, wdInt32& out_iCpIdx) const
{
  const wdVec2d at(x, y);

  out_iCurveIdx = -1;
  out_iCpIdx = -1;

  wdCurve1D CurveData;

  for (wdUInt32 iCurve = 0; iCurve < m_Curves.m_Curves.GetCount(); ++iCurve)
  {
    m_Curves.ConvertToRuntimeData(iCurve, CurveData);

    for (wdUInt32 iCP = 0; iCP < CurveData.GetNumControlPoints(); ++iCP)
    {
      const auto& cp = CurveData.GetControlPoint(iCP);
      const wdVec2d dist = cp.m_Position - at;

      if (wdMath::Abs(dist.x) <= vMaxDistance.x && wdMath::Abs(dist.y) <= vMaxDistance.y)
      {
        vMaxDistance.x = wdMath::Abs(dist.x);
        vMaxDistance.y = wdMath::Abs(dist.y);

        out_iCurveIdx = iCurve;
        out_iCpIdx = iCP;
      }
    }
  }

  return out_iCpIdx >= 0;
}

void wdQtCurve1DEditorWidget::onSelectionChanged()
{
  UpdateSpinBoxes();
}


void wdQtCurve1DEditorWidget::onMoveCurve(wdInt32 iCurve, double moveY)
{
  m_vControlPointMove.y += moveY;

  Q_EMIT BeginCpChangesEvent("Move Curve");

  const auto& curve = *m_CurvesBackup.m_Curves[iCurve];
  wdUInt32 uiNumCps = curve.m_ControlPoints.GetCount();
  for (wdUInt32 i = 0; i < uiNumCps; ++i)
  {
    const wdInt64 x = curve.m_ControlPoints[i].m_iTick;
    const float y = curve.m_ControlPoints[i].m_fValue + m_vControlPointMove.y;

    Q_EMIT CpMovedEvent(iCurve, i, x, y);
  }

  Q_EMIT EndCpChangesEvent();
}

void wdQtCurve1DEditorWidget::onGenerateCurve(wdMath::wdCurveFunction function, bool inverse)
{
  Q_EMIT BeginCpChangesEvent("Generate Curve");

  // Delete all existing control points
  ClearAllPoints();

  wdCurve1D cmp;

  const wdUInt32 uiFrames = m_Curves.m_uiFramesPerSecond / 2;
  const double invFps = 1.0 / uiFrames;

  struct Sample
  {
    double m_fPos = 0;
    double m_fCorrectValue = 0;
    bool m_bInserted = false;
  };

  wdHybridArray<Sample, 60> samples;
  samples.SetCount(uiFrames + 1);

  for (wdUInt32 i = 0; i <= uiFrames; ++i)
  {
    const double x = i * invFps;

    samples[i].m_fPos = x;
    samples[i].m_fCorrectValue = GetCurveValue(function, x, inverse);
  }

  auto AddPt = [&](wdUInt32 uiIdx) {
    samples[uiIdx].m_bInserted = true;
    const double x = samples[uiIdx].m_fPos;
    const double y = samples[uiIdx].m_fCorrectValue;

    cmp.AddControlPoint(x).m_Position.y = y;
    InsertCpAt(x, y, wdVec2d::ZeroVector());
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
    wdUInt32 uiMaxErrorIdx = 0xffffffff;

    for (wdUInt32 idx = 0; idx < samples.GetCount(); ++idx)
    {
      auto& sample = samples[idx];
      if (sample.m_bInserted)
        continue;

      const double eval = cmp.Evaluate(sample.m_fPos);
      const double err = wdMath::Abs(eval - sample.m_fCorrectValue);

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

void wdQtCurve1DEditorWidget::onSaveAsPreset()
{
  if (s_sPresetSaveDir.isEmpty())
  {
    s_sPresetSaveDir = wdToolsProject::GetSingleton()->GetProjectDirectory().GetData();
    s_sPresetSaveDir.append("/Editor/Presets/Curves");

    wdOSFile::CreateDirectoryStructure(s_sPresetSaveDir.toUtf8().data()).IgnoreResult();
  }

  QString sFile = QFileDialog::getSaveFileName(QApplication::activeWindow(), "Save Curve as Preset", s_sPresetSaveDir, "Curve Presets (*.wdCurvePreset)", nullptr, QFileDialog::Option::DontResolveSymlinks);

  if (sFile.isEmpty())
    return;

  s_sPresetSaveDir = sFile;

  SaveCurvePreset(sFile.toUtf8().data());

  FindAllPresets();
}

void wdQtCurve1DEditorWidget::SaveCurvePreset(const char* szFile) const
{
  wdFileWriter file;
  if (file.Open(szFile).Failed())
    return;

  file.WriteVersion(1);

  file << m_Curves.m_uiFramesPerSecond;
  file << m_Curves.m_Curves.GetCount();

  WD_ASSERT_DEBUG(m_Curves.m_Curves.GetCount() == 1, "Only one curve at a time is currently supported.");

  for (wdUInt32 curveIdx = 0; curveIdx < m_Curves.m_Curves.GetCount(); ++curveIdx)
  {
    const auto& curve = m_Curves.m_Curves[curveIdx];
    file << curve->m_CurveColor;
    file << curve->m_ControlPoints.GetCount();

    for (wdUInt32 cpIdx = 0; cpIdx < curve->m_ControlPoints.GetCount(); ++cpIdx)
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

void wdQtCurve1DEditorWidget::onLoadPreset()
{
  if (s_sPresetSaveDir.isEmpty())
  {
    s_sPresetSaveDir = wdToolsProject::GetSingleton()->GetProjectDirectory().GetData();
    s_sPresetSaveDir.append("/Editor/Presets/Curves");

    if (!wdOSFile::ExistsDirectory(s_sPresetSaveDir.toUtf8().data()))
    {
      // maybe fall back to the Base directory instead ?
      wdOSFile::CreateDirectoryStructure(s_sPresetSaveDir.toUtf8().data()).IgnoreResult();
    }
  }

  QString sFile = QFileDialog::getOpenFileName(QApplication::activeWindow(), "Load Curve from Preset", s_sPresetSaveDir, "Curve Presets (*.wdCurvePreset)", nullptr, QFileDialog::Option::DontResolveSymlinks);

  if (sFile.isEmpty())
    return;

  s_sPresetSaveDir = sFile;

  LoadCurvePreset(sFile.toUtf8().data()).IgnoreResult();
}

wdResult wdQtCurve1DEditorWidget::LoadCurvePreset(const char* szFile)
{
  wdStringBuilder sPath = szFile;

  if (!sPath.IsAbsolutePath())
  {
    sPath.Prepend("Editor/Presets/Curves/");
  }

  wdFileReader file;
  if (file.Open(sPath).Failed())
    return WD_FAILURE;

  const wdTypeVersion version = file.ReadVersion(1);

  Q_EMIT BeginCpChangesEvent("Load Preset");

  // Delete all existing control points
  ClearAllPoints();

  file >> m_Curves.m_uiFramesPerSecond;

  wdUInt32 uiNumCurves = 0;
  file >> uiNumCurves;

  WD_ASSERT_DEBUG(uiNumCurves == 1, "Only one curve at a time is currently supported.");
  uiNumCurves = 1;

  for (wdUInt32 curveIdx = 0; curveIdx < uiNumCurves; ++curveIdx)
  {
    wdColorGammaUB curveColor;
    wdUInt32 uiNumCPs = 0;
    file >> curveColor;
    file >> uiNumCPs;

    for (wdUInt32 cpIdx = 0; cpIdx < uiNumCPs; ++cpIdx)
    {
      wdInt64 iTick = 0;
      double fValue = 0;
      bool bTangentsLinked = false;
      wdEnum<wdCurveTangentMode> LeftTangentMode;
      wdEnum<wdCurveTangentMode> RightTangentMode;
      wdVec2 LeftTangent;
      wdVec2 RightTangent;

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

  return WD_SUCCESS;
}

void wdQtCurve1DEditorWidget::FindAllPresets()
{
  s_CurvePresets.Clear();

#if WD_ENABLED(WD_SUPPORTS_FILE_ITERATORS)

  wdFileSystemIterator fsIt;

  wdFileSystem::StartSearch(fsIt, "Editor/Presets/Curves", wdFileSystemIteratorFlags::ReportFilesRecursive);

  wdStringBuilder sFilePath;

  for (; fsIt.IsValid(); fsIt.Next())
  {
    if (!wdPathUtils::HasExtension(fsIt.GetStats().m_sName, "wdCurvePreset"))
      continue;

    fsIt.GetStats().GetFullPath(sFilePath);
    sFilePath.MakeCleanPath();

    sFilePath.MakeRelativeTo(fsIt.GetCurrentSearchTerm()).AssertSuccess();

    s_CurvePresets.PushBack(sFilePath);
  }

  s_CurvePresets.Sort();

#endif
}

void wdQtCurve1DEditorWidget::UpdateSpinBoxes()
{
  const auto& selection = CurveEdit->GetSelection();

  wdQtScopedBlockSignals _1(LinePosition, LineValue);

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
  for (wdUInt32 i = 1; i < selection.GetCount(); ++i)
  {
    const auto& pt = m_Curves.m_Curves[selection[i].m_uiCurve]->m_ControlPoints[selection[i].m_uiPoint];

    if (pt.GetTickAsTime().GetSeconds() != fPos)
    {
      bMultipleTicks = true;
      break;
    }
  }

  bool bMultipleValues = false;
  for (wdUInt32 i = 1; i < selection.GetCount(); ++i)
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

void wdQtCurve1DEditorWidget::on_LinePosition_editingFinished()
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

    wdInt32 iTick = m_Curves.TickFromTime(wdTime::Seconds(value));
    if (cp.m_iTick != iTick)
      Q_EMIT CpMovedEvent(cpSel.m_uiCurve, cpSel.m_uiPoint, iTick, cp.m_fValue);
  }

  Q_EMIT EndCpChangesEvent();
}

void wdQtCurve1DEditorWidget::on_LineValue_editingFinished()
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

void wdQtCurve1DEditorWidget::SetTangentMode(wdCurveTangentMode::Enum mode, bool bLeft, bool bRight)
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

void wdQtCurve1DEditorWidget::ClampPoint(double& x, double& y) const
{
  if (CurveEdit->m_bLowerExtentFixed)
    x = wdMath::Max(x, CurveEdit->m_fLowerExtent);
  if (CurveEdit->m_bUpperExtentFixed)
    x = wdMath::Min(x, CurveEdit->m_fUpperExtent);

  y = wdMath::Clamp(y, CurveEdit->m_fLowerRange, CurveEdit->m_fUpperRange);
}

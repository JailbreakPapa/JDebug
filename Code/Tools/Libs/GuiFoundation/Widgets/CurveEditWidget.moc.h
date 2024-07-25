#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Math/Vec2.h>
#include <Foundation/Tracks/Curve1D.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/Widgets/CurveEditData.h>

#include <QBrush>
#include <QPen>
#include <QWidget>

class nsQGridBarWidget;
class QRubberBand;

class NS_GUIFOUNDATION_DLL nsQtCurveEditWidget : public QWidget
{
  Q_OBJECT

public:
  nsQtCurveEditWidget(QWidget* pParent);

  double m_fLowerRange = -nsMath::HighValue<double>();
  double m_fUpperRange = nsMath::HighValue<double>();
  double m_fLowerExtent = 0.0;
  double m_fUpperExtent = 1.0;
  bool m_bLowerExtentFixed = false;
  bool m_bUpperExtentFixed = false;

  void SetCurves(nsCurveGroupData* pCurveEditData);
  void SetGridBarWidget(nsQGridBarWidget* pGridBar) { m_pGridBar = pGridBar; }

  void SetScrubberPosition(double fPosition);

  double GetMinCurveExtent() const { return m_fMinExtentValue; }
  double GetMaxCurveExtent() const { return m_fMaxExtentValue; }

  void FrameCurve();
  void FrameSelection();
  void Frame(double fOffsetX, double fOffsetY, double fWidth, double fHeight);

  QPoint MapFromScene(const QPointF& pos) const;
  QPoint MapFromScene(const nsVec2d& vPos) const { return MapFromScene(QPointF(vPos.x, vPos.y)); }
  QPointF MapToScene(const QPoint& pos) const;
  nsVec2 MapDirFromScene(const nsVec2& vPos) const;

  void ClearSelection();
  void SelectAll();
  const nsDynamicArray<nsSelectedCurveCP>& GetSelection() const { return m_SelectedCPs; }
  bool IsSelected(const nsSelectedCurveCP& cp) const;
  void SetSelection(const nsSelectedCurveCP& cp);
  void ToggleSelected(const nsSelectedCurveCP& cp);
  void SetSelected(const nsSelectedCurveCP& cp, bool bSet);

  bool GetSelectedTangent(nsInt32& out_iCurve, nsInt32& out_iPoint, bool& out_bLeftTangent) const;

Q_SIGNALS:
  void DoubleClickEvent(const QPointF& scenePos, const QPointF& epsilon);
  void DeleteControlPointsEvent();
  void MoveControlPointsEvent(double fMoveX, double fMoveY);
  void MoveTangentsEvent(double fMoveX, double fMoveY);
  void BeginOperationEvent(QString sName);
  void EndOperationEvent(bool bCommit);
  void ScaleControlPointsEvent(const QPointF& centerPos, double fScaleX, double fScaleY);
  void ContextMenuEvent(QPoint pos, QPointF scenePos);
  void SelectionChangedEvent();
  void MoveCurveEvent(nsInt32 iCurve, double fMoveY);

protected:
  virtual void paintEvent(QPaintEvent* e) override;
  virtual void mousePressEvent(QMouseEvent* e) override;
  virtual void mouseReleaseEvent(QMouseEvent* e) override;
  virtual void mouseMoveEvent(QMouseEvent* e) override;
  virtual void mouseDoubleClickEvent(QMouseEvent* e) override;
  virtual void wheelEvent(QWheelEvent* e) override;
  virtual void keyPressEvent(QKeyEvent* e) override;

private:
  enum class ClickTarget
  {
    Nothing,
    SelectedPoint,
    TangentHandle
  };
  enum class EditState
  {
    None,
    DraggingPoints,
    DraggingPointsHorz,
    DraggingPointsVert,
    DraggingTangents,
    MultiSelect,
    RightClick,
    Panning,
    ScaleLeftRight,
    ScaleUpDown,
    DraggingCurve
  };
  enum class SelectArea
  {
    None,
    Center,
    Top,
    Bottom,
    Left,
    Right
  };

  void PaintCurveSegments(QPainter* painter, float fOffsetX, nsUInt8 alpha) const;
  void PaintOutsideAreaOverlay(QPainter* painter) const;
  void PaintControlPoints(QPainter* painter) const;
  void PaintSelectedControlPoints(QPainter* painter) const;
  void PaintSelectedTangentLines(QPainter* painter) const;
  void PaintSelectedTangentHandles(QPainter* painter) const;
  void PaintMultiSelectionSquare(QPainter* painter) const;
  void PaintScrubber(QPainter& p) const;
  void RenderVerticalGrid(QPainter* painter, const QRectF& viewportSceneRect, double fRoughGridDensity);
  void RenderSideLinesAndText(QPainter* painter, const QRectF& viewportSceneRect);
  void RenderValueRanges(QPainter* painter);
  QRectF ComputeViewportSceneRect() const;
  bool PickCpAt(const QPoint& pos, float fMaxPixelDistance, nsSelectedCurveCP& out_Result) const;
  ClickTarget DetectClickTarget(const QPoint& pos);
  void ExecMultiSelection(nsDynamicArray<nsSelectedCurveCP>& out_Selection);
  bool CombineSelectionAdd(nsDynamicArray<nsSelectedCurveCP>& inout_Selection, const nsDynamicArray<nsSelectedCurveCP>& change);
  bool CombineSelectionRemove(nsDynamicArray<nsSelectedCurveCP>& inout_Selection, const nsDynamicArray<nsSelectedCurveCP>& change);
  bool CombineSelectionToggle(nsDynamicArray<nsSelectedCurveCP>& inout_Selection, const nsDynamicArray<nsSelectedCurveCP>& change);
  void ComputeSelectionRect();
  SelectArea WhereIsPoint(QPoint pos) const;
  nsInt32 PickCurveAt(QPoint pos) const;
  void ClampZoomPan();

  nsQGridBarWidget* m_pGridBar = nullptr;

  EditState m_State = EditState::None;
  nsInt32 m_iDraggedCurve;

  nsCurveGroupData* m_pCurveEditData;
  nsHybridArray<nsCurve1D, 4> m_Curves;
  nsHybridArray<nsCurve1D, 4> m_CurvesSorted;
  nsHybridArray<nsVec2d, 4> m_CurveExtents;
  double m_fMinExtentValue;
  double m_fMaxExtentValue;
  double m_fMinValue, m_fMaxValue;


  QPointF m_SceneTranslation;
  QPointF m_SceneToPixelScale;
  QPoint m_LastMousePos;

  QBrush m_ControlPointBrush;
  QBrush m_SelectedControlPointBrush;
  QPen m_TangentLinePen;
  QBrush m_TangentHandleBrush;

  nsDynamicArray<nsSelectedCurveCP> m_SelectedCPs;
  nsInt32 m_iSelectedTangentCurve = -1;
  nsInt32 m_iSelectedTangentPoint = -1;
  bool m_bSelectedTangentLeft = false;
  bool m_bBegunChanges = false;
  bool m_bFrameBeforePaint = true;

  QPoint m_MultiSelectionStart;
  QRect m_MultiSelectRect;
  QRectF m_SelectionBRect;
  QPointF m_ScaleReferencePoint;
  QPointF m_ScaleStartPoint;
  QPointF m_TotalPointDrag;
  QRubberBand* m_pRubberband = nullptr;

  bool m_bShowScrubber = false;
  double m_fScrubberPosition = 0;
};

#pragma once

#include <Foundation/Math/CurveFunctions.h>
#include <Foundation/Tracks/Curve1D.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/ui_Curve1DEditorWidget.h>

#include <QWidget>

class WD_GUIFOUNDATION_DLL wdQtCurve1DEditorWidget : public QWidget, public Ui_Curve1DEditorWidget
{
  Q_OBJECT

public:
  explicit wdQtCurve1DEditorWidget(QWidget* pParent);
  ~wdQtCurve1DEditorWidget();

  void SetCurveExtents(double fLowerBound, double fUpperBound, bool bLowerIsFixed, bool bUpperIsFixed);
  void SetCurveRanges(double fLowerRange, double fUpperRange);

  void SetCurves(const wdCurveGroupData& curveData);
  void SetScrubberPosition(wdUInt64 uiTick);
  void ClearSelection();

  void FrameCurve();
  void FrameSelection();
  void MakeRepeatable(bool bAdjustLastPoint);
  void NormalizeCurveX(wdUInt32 uiActiveCurve);
  void NormalizeCurveY(wdUInt32 uiActiveCurve);
  void ClearAllPoints();
  void MirrorHorizontally(wdUInt32 uiActiveCurve);
  void MirrorVertically(wdUInt32 uiActiveCurve);

Q_SIGNALS:
  void CpMovedEvent(wdUInt32 uiCurveIdx, wdUInt32 uiIdx, wdInt64 iTickX, double fNewPosY);
  void CpDeletedEvent(wdUInt32 uiCurveIdx, wdUInt32 uiIdx);
  void TangentMovedEvent(wdUInt32 uiCurveIdx, wdUInt32 uiIdx, float fNewPosX, float fNewPosY, bool bRightTangent);
  void InsertCpEvent(wdUInt32 uiCurveIdx, wdInt64 iTickX, double value);
  void TangentLinkEvent(wdUInt32 uiCurveIdx, wdUInt32 uiIdx, bool bLink);
  void CpTangentModeEvent(wdUInt32 uiCurveIdx, wdUInt32 uiIdx, bool bRightTangent, int iMode); // wdCurveTangentMode

  void BeginCpChangesEvent(QString sName);
  void EndCpChangesEvent();

  void BeginOperationEvent(QString sName);
  void EndOperationEvent(bool bCommit);

private Q_SLOTS:
  void on_LinePosition_editingFinished();
  void on_LineValue_editingFinished();
  void onDeleteControlPoints();
  void onDoubleClick(const QPointF& scenePos, const QPointF& epsilon);
  void onMoveControlPoints(double x, double y);
  void onMoveTangents(float x, float y);
  void onBeginOperation(QString name);
  void onEndOperation(bool commit);
  void onScaleControlPoints(QPointF refPt, double scaleX, double scaleY);
  void onContextMenu(QPoint pos, QPointF scenePos);
  void onAddPoint();
  void onLinkTangents();
  void onBreakTangents();
  void onFlattenTangents();
  void onSelectionChanged();
  void onMoveCurve(wdInt32 iCurve, double moveY);
  void onGenerateCurve(wdMath::wdCurveFunction function, bool inverse);
  void onSaveAsPreset();
  void onLoadPreset();

private:
  void InsertCpAt(double posX, double value, wdVec2d epsilon);
  bool PickCurveAt(double x, double y, double fMaxDistanceY, wdInt32& out_iCurveIdx, double& out_ValueY) const;
  bool PickControlPointAt(double x, double y, wdVec2d vMaxDistance, wdInt32& out_iCurveIdx, wdInt32& out_iCpIdx) const;
  void UpdateSpinBoxes();
  void SetTangentMode(wdCurveTangentMode::Enum mode, bool bLeft, bool bRight);
  void ClampPoint(double& x, double& y) const;
  void SaveCurvePreset(const char* szFile) const;
  wdResult LoadCurvePreset(const char* szFile);
  void FindAllPresets();

  double m_fCurveDuration;
  wdVec2 m_vTangentMove;
  wdVec2d m_vControlPointMove;
  wdCurveGroupData m_Curves;
  wdCurveGroupData m_CurvesBackup;
  QPointF m_ContextMenuScenePos;

  static wdDynamicArray<wdString> s_CurvePresets;
};

#pragma once

#include <Foundation/Math/CurveFunctions.h>
#include <Foundation/Tracks/Curve1D.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/ui_Curve1DEditorWidget.h>

#include <QWidget>

class NS_GUIFOUNDATION_DLL nsQtCurve1DEditorWidget : public QWidget, public Ui_Curve1DEditorWidget
{
  Q_OBJECT

public:
  explicit nsQtCurve1DEditorWidget(QWidget* pParent);
  ~nsQtCurve1DEditorWidget();

  void SetCurveExtents(double fLowerBound, double fUpperBound, bool bLowerIsFixed, bool bUpperIsFixed);
  void SetCurveRanges(double fLowerRange, double fUpperRange);

  void SetCurves(const nsCurveGroupData& curveData);
  void SetScrubberPosition(nsUInt64 uiTick);
  void ClearSelection();

  void FrameCurve();
  void FrameSelection();
  void MakeRepeatable(bool bAdjustLastPoint);
  void NormalizeCurveX(nsUInt32 uiActiveCurve);
  void NormalizeCurveY(nsUInt32 uiActiveCurve);
  void ClearAllPoints();
  void MirrorHorizontally(nsUInt32 uiActiveCurve);
  void MirrorVertically(nsUInt32 uiActiveCurve);

Q_SIGNALS:
  void CpMovedEvent(nsUInt32 uiCurveIdx, nsUInt32 uiIdx, nsInt64 iTickX, double fNewPosY);
  void CpDeletedEvent(nsUInt32 uiCurveIdx, nsUInt32 uiIdx);
  void TangentMovedEvent(nsUInt32 uiCurveIdx, nsUInt32 uiIdx, float fNewPosX, float fNewPosY, bool bRightTangent);
  void InsertCpEvent(nsUInt32 uiCurveIdx, nsInt64 iTickX, double value);
  void TangentLinkEvent(nsUInt32 uiCurveIdx, nsUInt32 uiIdx, bool bLink);
  void CpTangentModeEvent(nsUInt32 uiCurveIdx, nsUInt32 uiIdx, bool bRightTangent, int iMode); // nsCurveTangentMode

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
  void onMoveCurve(nsInt32 iCurve, double moveY);
  void onGenerateCurve(nsCurveFunction::Enum function, bool inverse);
  void onSaveAsPreset();
  void onLoadPreset();

private:
  void InsertCpAt(double posX, double value, nsVec2d epsilon);
  bool PickCurveAt(double x, double y, double fMaxDistanceY, nsInt32& out_iCurveIdx, double& out_ValueY) const;
  bool PickControlPointAt(double x, double y, nsVec2d vMaxDistance, nsInt32& out_iCurveIdx, nsInt32& out_iCpIdx) const;
  void UpdateSpinBoxes();
  void SetTangentMode(nsCurveTangentMode::Enum mode, bool bLeft, bool bRight);
  void ClampPoint(double& x, double& y) const;
  void SaveCurvePreset(const char* szFile) const;
  nsResult LoadCurvePreset(const char* szFile);
  void FindAllPresets();

  double m_fCurveDuration;
  nsVec2 m_vTangentMove;
  nsVec2d m_vControlPointMove;
  nsCurveGroupData m_Curves;
  nsCurveGroupData m_CurvesBackup;
  QPointF m_ContextMenuScenePos;

  static nsDynamicArray<nsString> s_CurvePresets;
};

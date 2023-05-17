#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/ui_EventTrackEditorWidget.h>

#include <QWidget>

class WD_GUIFOUNDATION_DLL wdQtEventTrackEditorWidget : public QWidget, public Ui_EventTrackEditorWidget
{
  Q_OBJECT

public:
  explicit wdQtEventTrackEditorWidget(QWidget* pParent);
  ~wdQtEventTrackEditorWidget();

  void SetData(const wdEventTrackData& data, double fMinCurveLength);
  void SetScrubberPosition(wdUInt64 uiTick);
  void SetScrubberPosition(wdTime time);
  void ClearSelection();

  void FrameCurve();

Q_SIGNALS:
  void CpMovedEvent(wdUInt32 uiIdx, wdInt64 iTickX);
  void CpDeletedEvent(wdUInt32 uiIdx);
  void InsertCpEvent(wdInt64 iTickX, const char* value);

  void BeginCpChangesEvent(QString sName);
  void EndCpChangesEvent();

  void BeginOperationEvent(QString sName);
  void EndOperationEvent(bool bCommit);

private Q_SLOTS:
  void on_LinePosition_editingFinished();
  void on_AddEventButton_clicked();
  void onDeleteControlPoints();
  void onDoubleClick(double scenePosX, double epsilon);
  void onMoveControlPoints(double x);
  void onBeginOperation(QString name);
  void onEndOperation(bool commit);
  // void onScaleControlPoints(QPointF refPt, double scaleX);
  void onContextMenu(QPoint pos, QPointF scenePos);
  void onAddPoint();
  void onSelectionChanged();

private:
  void InsertCpAt(double posX, double epsilon);
  void UpdateSpinBoxes();
  void DetermineAvailableEvents();
  void FillEventComboBox(const char* szCurrent = nullptr);

  const wdEventTrackData* m_pData = nullptr;
  wdEventTrackData m_DataCopy;

  double m_fControlPointMove;
  QPointF m_ContextMenuScenePos;
  wdEventSet m_EventSet;
};


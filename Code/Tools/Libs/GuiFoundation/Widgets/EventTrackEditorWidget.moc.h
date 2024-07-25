#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/ui_EventTrackEditorWidget.h>

#include <QWidget>

class NS_GUIFOUNDATION_DLL nsQtEventTrackEditorWidget : public QWidget, public Ui_EventTrackEditorWidget
{
  Q_OBJECT

public:
  explicit nsQtEventTrackEditorWidget(QWidget* pParent);
  ~nsQtEventTrackEditorWidget();

  void SetData(const nsEventTrackData& data, double fMinCurveLength);
  void SetScrubberPosition(nsUInt64 uiTick);
  void SetScrubberPosition(nsTime time);
  void ClearSelection();

  void FrameCurve();

Q_SIGNALS:
  void CpMovedEvent(nsUInt32 uiIdx, nsInt64 iTickX);
  void CpDeletedEvent(nsUInt32 uiIdx);
  void InsertCpEvent(nsInt64 iTickX, const char* value);

  void BeginCpChangesEvent(QString sName);
  void EndCpChangesEvent();

  void BeginOperationEvent(QString sName);
  void EndOperationEvent(bool bCommit);

private Q_SLOTS:
  void on_LinePosition_editingFinished();
  void on_AddEventButton_clicked();
  void on_InsertEventButton_clicked();
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

  const nsEventTrackData* m_pData = nullptr;
  nsEventTrackData m_DataCopy;

  double m_fControlPointMove;
  QPointF m_ContextMenuScenePos;
  nsEventSet m_EventSet;
};

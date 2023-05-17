#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Math/Math.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <GuiFoundation/Widgets/EventTrackEditorWidget.moc.h>
#include <QGraphicsItem>
#include <QGraphicsSceneEvent>
#include <QInputDialog>
#include <QMenu>
#include <QPainterPath>

wdQtEventTrackEditorWidget::wdQtEventTrackEditorWidget(QWidget* pParent)
  : QWidget(pParent)
{
  setupUi(this);

  EventTrackEdit->SetGridBarWidget(GridBarWidget);

  connect(EventTrackEdit, &wdQtEventTrackWidget::DeleteControlPointsEvent, this, &wdQtEventTrackEditorWidget::onDeleteControlPoints);
  connect(EventTrackEdit, &wdQtEventTrackWidget::DoubleClickEvent, this, &wdQtEventTrackEditorWidget::onDoubleClick);
  connect(EventTrackEdit, &wdQtEventTrackWidget::MoveControlPointsEvent, this, &wdQtEventTrackEditorWidget::onMoveControlPoints);
  connect(EventTrackEdit, &wdQtEventTrackWidget::BeginOperationEvent, this, &wdQtEventTrackEditorWidget::onBeginOperation);
  connect(EventTrackEdit, &wdQtEventTrackWidget::EndOperationEvent, this, &wdQtEventTrackEditorWidget::onEndOperation);
  // connect(EventTrackEdit, &wdQtEventTrackWidget::ScaleControlPointsEvent, this, &wdQtEventTrackEditorWidget::onScaleControlPoints);
  connect(EventTrackEdit, &wdQtEventTrackWidget::ContextMenuEvent, this, &wdQtEventTrackEditorWidget::onContextMenu);
  connect(EventTrackEdit, &wdQtEventTrackWidget::SelectionChangedEvent, this, &wdQtEventTrackEditorWidget::onSelectionChanged);

  LinePosition->setEnabled(false);

  DetermineAvailableEvents();
}

wdQtEventTrackEditorWidget::~wdQtEventTrackEditorWidget() = default;

void wdQtEventTrackEditorWidget::SetData(const wdEventTrackData& trackData, double fMinCurveLength)
{
  wdQtScopedUpdatesDisabled ud(this);
  wdQtScopedBlockSignals bs(this);

  m_pData = &trackData;
  EventTrackEdit->SetData(&trackData, fMinCurveLength);

  UpdateSpinBoxes();
}

void wdQtEventTrackEditorWidget::SetScrubberPosition(wdUInt64 uiTick)
{
  EventTrackEdit->SetScrubberPosition(uiTick / 4800.0);
}

void wdQtEventTrackEditorWidget::SetScrubberPosition(wdTime time)
{
  EventTrackEdit->SetScrubberPosition(time.GetSeconds());
}

void wdQtEventTrackEditorWidget::ClearSelection()
{
  EventTrackEdit->ClearSelection();
}

void wdQtEventTrackEditorWidget::FrameCurve()
{
  EventTrackEdit->FrameCurve();
}

void wdQtEventTrackEditorWidget::on_AddEventButton_clicked()
{
  QString name = QInputDialog::getText(this, "Add Type", "Event Type Name:");

  m_EventSet.AddAvailableEvent(name.toUtf8().data());

  if (m_EventSet.IsModified())
  {
    m_EventSet.WriteToDDL(":project/Editor/Events.ddl").IgnoreResult();

    FillEventComboBox(name.toUtf8().data());
  }
}

void wdQtEventTrackEditorWidget::onDeleteControlPoints()
{
  wdHybridArray<wdUInt32, 32> selection;
  EventTrackEdit->GetSelection(selection);

  if (selection.IsEmpty())
    return;

  EventTrackEdit->ClearSelection();

  Q_EMIT BeginCpChangesEvent("Delete Events");

  selection.Sort([](wdUInt32 lhs, wdUInt32 rhs) -> bool { return lhs > rhs; });

  // delete sorted from back to front to prevent point indices becoming invalidated
  for (wdUInt32 pt : selection)
  {
    Q_EMIT CpDeletedEvent(pt);
  }

  Q_EMIT EndCpChangesEvent();
}

void wdQtEventTrackEditorWidget::onDoubleClick(double scenePosX, double epsilon)
{
  InsertCpAt(scenePosX, wdMath::Abs(epsilon));
}

void wdQtEventTrackEditorWidget::onMoveControlPoints(double x)
{
  m_fControlPointMove += x;

  wdHybridArray<wdUInt32, 32> selection;
  EventTrackEdit->GetSelection(selection);

  if (selection.IsEmpty())
    return;

  Q_EMIT BeginCpChangesEvent("Move Events");

  for (const auto& cpSel : selection)
  {
    auto& cp = m_DataCopy.m_ControlPoints[cpSel];

    double newPos = cp.GetTickAsTime().GetSeconds() + m_fControlPointMove;
    newPos = wdMath::Max(newPos, 0.0);

    Q_EMIT CpMovedEvent(cpSel, m_pData->TickFromTime(wdTime::Seconds(newPos)));
  }

  Q_EMIT EndCpChangesEvent();
}

// void wdQtEventTrackEditorWidget::onScaleControlPoints(QPointF refPt, double scaleX, double scaleY)
//{
//  const auto selection = EventTrackEdit->GetSelection();
//
//  if (selection.IsEmpty())
//    return;
//
//  const wdVec2d ref(refPt.x(), refPt.y());
//  const wdVec2d scale(scaleX, scaleY);
//
//  Q_EMIT BeginCpChangesEvent("Scale Points");
//
//  for (const auto& cpSel : selection)
//  {
//    const auto& cp = m_CurvesBackup.m_Curves[cpSel.m_uiCurve]->m_ControlPoints[cpSel.m_uiPoint];
//    wdVec2d newPos = ref + (wdVec2d(cp.GetTickAsTime(), cp.m_fValue) - ref).CompMul(scale);
//    newPos.x = wdMath::Max(newPos.x, 0.0);
//    newPos.y = wdMath::Clamp(newPos.y, -100000.0, +100000.0);
//
//    Q_EMIT CpMovedEvent(cpSel.m_uiCurve, cpSel.m_uiPoint, m_Curves.TickFromTime(newPos.x), newPos.y);
//  }
//
//  Q_EMIT EndCpChangesEvent();
//}

void wdQtEventTrackEditorWidget::onBeginOperation(QString name)
{
  m_fControlPointMove = 0;
  m_DataCopy = *m_pData;

  Q_EMIT BeginOperationEvent(name);
}

void wdQtEventTrackEditorWidget::onEndOperation(bool commit)
{
  Q_EMIT EndOperationEvent(commit);
}

void wdQtEventTrackEditorWidget::onContextMenu(QPoint pos, QPointF scenePos)
{
  m_ContextMenuScenePos = scenePos;

  QMenu m(this);
  m.setDefaultAction(m.addAction("Add Event", this, SLOT(onAddPoint())));

  wdHybridArray<wdUInt32, 32> selection;
  EventTrackEdit->GetSelection(selection);

  if (!selection.IsEmpty())
  {
    m.addAction("Delete Events", this, SLOT(onDeleteControlPoints()), QKeySequence(Qt::Key_Delete));
  }

  m.addSeparator();

  m.addAction(
    "Frame", this, [this]() { FrameCurve(); }, QKeySequence(Qt::ControlModifier | Qt::Key_F));

  m.exec(pos);
}

void wdQtEventTrackEditorWidget::onAddPoint()
{
  InsertCpAt(m_ContextMenuScenePos.x(), 0.0f);
}

void wdQtEventTrackEditorWidget::InsertCpAt(double posX, double epsilon)
{
  int curveIdx = 0, cpIdx = 0;
  posX = wdMath::Max(posX, 0.0);

  Q_EMIT InsertCpEvent(m_pData->TickFromTime(wdTime::Seconds(posX)), ComboType->currentText().toUtf8().data());
}

void wdQtEventTrackEditorWidget::onSelectionChanged()
{
  UpdateSpinBoxes();
}

void wdQtEventTrackEditorWidget::UpdateSpinBoxes()
{
  wdHybridArray<wdUInt32, 32> selection;
  EventTrackEdit->GetSelection(selection);

  wdQtScopedBlockSignals _1(LinePosition, SelectedTypeLabel);

  if (selection.IsEmpty())
  {
    LinePosition->setText(QString());
    LinePosition->setEnabled(false);
    SelectedTypeLabel->setText("Event: none");
    return;
  }

  const double fPos = m_pData->m_ControlPoints[selection[0]].GetTickAsTime().GetSeconds();

  LinePosition->setEnabled(true);

  wdStringBuilder labelText("Event: ", m_pData->m_ControlPoints[selection[0]].m_sEvent.GetString());

  bool bMultipleTicks = false;
  for (wdUInt32 i = 1; i < selection.GetCount(); ++i)
  {
    const wdString& sName = m_pData->m_ControlPoints[selection[i]].m_sEvent.GetString();
    const double fPos2 = m_pData->m_ControlPoints[selection[i]].GetTickAsTime().GetSeconds();

    if (!labelText.FindSubString(sName))
    {
      labelText.Append(", ", sName);
    }

    if (fPos2 != fPos)
    {
      bMultipleTicks = true;
      break;
    }
  }

  LinePosition->setText(bMultipleTicks ? QString() : QString::number(fPos, 'f', 2));
  SelectedTypeLabel->setText(labelText.GetData());
}

void wdQtEventTrackEditorWidget::DetermineAvailableEvents()
{
  m_EventSet.ReadFromDDL(":project/Editor/Events.ddl").IgnoreResult();

  FillEventComboBox(nullptr);
}

void wdQtEventTrackEditorWidget::FillEventComboBox(const char* szCurrent)
{
  QString prev = szCurrent;

  if (prev.isEmpty())
    prev = ComboType->currentText();

  ComboType->clear();

  for (const wdString& type : m_EventSet.GetAvailableEvents())
  {
    ComboType->addItem(type.GetData());
  }

  ComboType->setCurrentText(prev);
}

void wdQtEventTrackEditorWidget::on_LinePosition_editingFinished()
{
  QString sValue = LinePosition->text();

  bool ok = false;
  const double value = sValue.toDouble(&ok);
  if (!ok)
    return;

  if (value < 0)
    return;

  wdHybridArray<wdUInt32, 32> selection;
  EventTrackEdit->GetSelection(selection);
  if (selection.IsEmpty())
    return;

  Q_EMIT BeginCpChangesEvent("Set Event Time");

  wdInt64 tick = m_pData->TickFromTime(wdTime::Seconds(value));

  for (const auto& cpSel : selection)
  {
    if (m_pData->m_ControlPoints[cpSel].m_iTick != tick)
      Q_EMIT CpMovedEvent(cpSel, tick);
  }

  Q_EMIT EndCpChangesEvent();
}

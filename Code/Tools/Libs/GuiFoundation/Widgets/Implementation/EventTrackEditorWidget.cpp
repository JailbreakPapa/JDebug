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

nsQtEventTrackEditorWidget::nsQtEventTrackEditorWidget(QWidget* pParent)
  : QWidget(pParent)
{
  setupUi(this);

  EventTrackEdit->SetGridBarWidget(GridBarWidget);

  // make sure the track is visible and not completely squashed
  EventTrackEdit->setMinimumHeight(50);

  connect(EventTrackEdit, &nsQtEventTrackWidget::DeleteControlPointsEvent, this, &nsQtEventTrackEditorWidget::onDeleteControlPoints);
  connect(EventTrackEdit, &nsQtEventTrackWidget::DoubleClickEvent, this, &nsQtEventTrackEditorWidget::onDoubleClick);
  connect(EventTrackEdit, &nsQtEventTrackWidget::MoveControlPointsEvent, this, &nsQtEventTrackEditorWidget::onMoveControlPoints);
  connect(EventTrackEdit, &nsQtEventTrackWidget::BeginOperationEvent, this, &nsQtEventTrackEditorWidget::onBeginOperation);
  connect(EventTrackEdit, &nsQtEventTrackWidget::EndOperationEvent, this, &nsQtEventTrackEditorWidget::onEndOperation);
  // connect(EventTrackEdit, &nsQtEventTrackWidget::ScaleControlPointsEvent, this, &nsQtEventTrackEditorWidget::onScaleControlPoints);
  connect(EventTrackEdit, &nsQtEventTrackWidget::ContextMenuEvent, this, &nsQtEventTrackEditorWidget::onContextMenu);
  connect(EventTrackEdit, &nsQtEventTrackWidget::SelectionChangedEvent, this, &nsQtEventTrackEditorWidget::onSelectionChanged);

  LinePosition->setEnabled(false);

  DetermineAvailableEvents();
}

nsQtEventTrackEditorWidget::~nsQtEventTrackEditorWidget() = default;

void nsQtEventTrackEditorWidget::SetData(const nsEventTrackData& trackData, double fMinCurveLength)
{
  nsQtScopedUpdatesDisabled ud(this);
  nsQtScopedBlockSignals bs(this);

  m_pData = &trackData;
  EventTrackEdit->SetData(&trackData, fMinCurveLength);

  UpdateSpinBoxes();
}

void nsQtEventTrackEditorWidget::SetScrubberPosition(nsUInt64 uiTick)
{
  EventTrackEdit->SetScrubberPosition(uiTick / 4800.0);
}

void nsQtEventTrackEditorWidget::SetScrubberPosition(nsTime time)
{
  EventTrackEdit->SetScrubberPosition(time.GetSeconds());
}

void nsQtEventTrackEditorWidget::ClearSelection()
{
  EventTrackEdit->ClearSelection();
}

void nsQtEventTrackEditorWidget::FrameCurve()
{
  EventTrackEdit->FrameCurve();
}

void nsQtEventTrackEditorWidget::on_AddEventButton_clicked()
{
  QString name = QInputDialog::getText(this, "Add Type", "Event Type Name:");

  m_EventSet.AddAvailableEvent(name.toUtf8().data());

  if (m_EventSet.IsModified())
  {
    m_EventSet.WriteToDDL(":project/Editor/Events.ddl").IgnoreResult();

    FillEventComboBox(name.toUtf8().data());
  }
}

void nsQtEventTrackEditorWidget::on_InsertEventButton_clicked()
{
  int curveIdx = 0, cpIdx = 0;
  double posX = nsMath::Max(EventTrackEdit->GetScrubberPosition(), 0.0);

  Q_EMIT InsertCpEvent(m_pData->TickFromTime(nsTime::MakeFromSeconds(posX)), ComboType->currentText().toUtf8().data());
}

void nsQtEventTrackEditorWidget::onDeleteControlPoints()
{
  nsHybridArray<nsUInt32, 32> selection;
  EventTrackEdit->GetSelection(selection);

  if (selection.IsEmpty())
    return;

  EventTrackEdit->ClearSelection();

  Q_EMIT BeginCpChangesEvent("Delete Events");

  selection.Sort([](nsUInt32 lhs, nsUInt32 rhs) -> bool
    { return lhs > rhs; });

  // delete sorted from back to front to prevent point indices becoming invalidated
  for (nsUInt32 pt : selection)
  {
    Q_EMIT CpDeletedEvent(pt);
  }

  Q_EMIT EndCpChangesEvent();
}

void nsQtEventTrackEditorWidget::onDoubleClick(double scenePosX, double epsilon)
{
  InsertCpAt(scenePosX, nsMath::Abs(epsilon));
}

void nsQtEventTrackEditorWidget::onMoveControlPoints(double x)
{
  m_fControlPointMove += x;

  nsHybridArray<nsUInt32, 32> selection;
  EventTrackEdit->GetSelection(selection);

  if (selection.IsEmpty())
    return;

  Q_EMIT BeginCpChangesEvent("Move Events");

  for (const auto& cpSel : selection)
  {
    auto& cp = m_DataCopy.m_ControlPoints[cpSel];

    double newPos = cp.GetTickAsTime().GetSeconds() + m_fControlPointMove;
    newPos = nsMath::Max(newPos, 0.0);

    Q_EMIT CpMovedEvent(cpSel, m_pData->TickFromTime(nsTime::MakeFromSeconds(newPos)));
  }

  Q_EMIT EndCpChangesEvent();
}

// void nsQtEventTrackEditorWidget::onScaleControlPoints(QPointF refPt, double scaleX, double scaleY)
//{
//  const auto selection = EventTrackEdit->GetSelection();
//
//  if (selection.IsEmpty())
//    return;
//
//  const nsVec2d ref(refPt.x(), refPt.y());
//  const nsVec2d scale(scaleX, scaleY);
//
//  Q_EMIT BeginCpChangesEvent("Scale Points");
//
//  for (const auto& cpSel : selection)
//  {
//    const auto& cp = m_CurvesBackup.m_Curves[cpSel.m_uiCurve]->m_ControlPoints[cpSel.m_uiPoint];
//    nsVec2d newPos = ref + (nsVec2d(cp.GetTickAsTime(), cp.m_fValue) - ref).CompMul(scale);
//    newPos.x = nsMath::Max(newPos.x, 0.0);
//    newPos.y = nsMath::Clamp(newPos.y, -100000.0, +100000.0);
//
//    Q_EMIT CpMovedEvent(cpSel.m_uiCurve, cpSel.m_uiPoint, m_Curves.TickFromTime(newPos.x), newPos.y);
//  }
//
//  Q_EMIT EndCpChangesEvent();
//}

void nsQtEventTrackEditorWidget::onBeginOperation(QString name)
{
  m_fControlPointMove = 0;
  m_DataCopy = *m_pData;

  Q_EMIT BeginOperationEvent(name);
}

void nsQtEventTrackEditorWidget::onEndOperation(bool commit)
{
  Q_EMIT EndOperationEvent(commit);
}

void nsQtEventTrackEditorWidget::onContextMenu(QPoint pos, QPointF scenePos)
{
  m_ContextMenuScenePos = scenePos;

  QMenu m(this);
  m.setDefaultAction(m.addAction("Add Event", this, SLOT(onAddPoint())));

  nsHybridArray<nsUInt32, 32> selection;
  EventTrackEdit->GetSelection(selection);

  if (!selection.IsEmpty())
  {
    m.addAction("Delete Events", QKeySequence(Qt::Key_Delete), this, SLOT(onDeleteControlPoints()));
  }

  m.addSeparator();

  m.addAction("Frame", QKeySequence(Qt::ControlModifier | Qt::Key_F), this, [this]()
    { FrameCurve(); });

  m.exec(pos);
}

void nsQtEventTrackEditorWidget::onAddPoint()
{
  InsertCpAt(m_ContextMenuScenePos.x(), 0.0f);
}

void nsQtEventTrackEditorWidget::InsertCpAt(double posX, double epsilon)
{
  int curveIdx = 0, cpIdx = 0;
  posX = nsMath::Max(posX, 0.0);

  Q_EMIT InsertCpEvent(m_pData->TickFromTime(nsTime::MakeFromSeconds(posX)), ComboType->currentText().toUtf8().data());
}

void nsQtEventTrackEditorWidget::onSelectionChanged()
{
  UpdateSpinBoxes();
}

void nsQtEventTrackEditorWidget::UpdateSpinBoxes()
{
  nsHybridArray<nsUInt32, 32> selection;
  EventTrackEdit->GetSelection(selection);

  nsQtScopedBlockSignals _1(LinePosition, SelectedTypeLabel);

  if (selection.IsEmpty())
  {
    LinePosition->setText(QString());
    LinePosition->setEnabled(false);
    SelectedTypeLabel->setText("Event: none");
    return;
  }

  const double fPos = m_pData->m_ControlPoints[selection[0]].GetTickAsTime().GetSeconds();

  LinePosition->setEnabled(true);

  nsStringBuilder labelText("Event: ", m_pData->m_ControlPoints[selection[0]].m_sEvent.GetString());

  bool bMultipleTicks = false;
  for (nsUInt32 i = 1; i < selection.GetCount(); ++i)
  {
    const nsString& sName = m_pData->m_ControlPoints[selection[i]].m_sEvent.GetString();
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

void nsQtEventTrackEditorWidget::DetermineAvailableEvents()
{
  m_EventSet.ReadFromDDL(":project/Editor/Events.ddl").IgnoreResult();

  FillEventComboBox(nullptr);
}

void nsQtEventTrackEditorWidget::FillEventComboBox(const char* szCurrent)
{
  QString prev = szCurrent;

  if (prev.isEmpty())
    prev = ComboType->currentText();

  ComboType->clear();

  for (const nsString& type : m_EventSet.GetAvailableEvents())
  {
    ComboType->addItem(type.GetData());
  }

  ComboType->setCurrentText(prev);
}

void nsQtEventTrackEditorWidget::on_LinePosition_editingFinished()
{
  QString sValue = LinePosition->text();

  bool ok = false;
  const double value = sValue.toDouble(&ok);
  if (!ok)
    return;

  if (value < 0)
    return;

  nsHybridArray<nsUInt32, 32> selection;
  EventTrackEdit->GetSelection(selection);
  if (selection.IsEmpty())
    return;

  Q_EMIT BeginCpChangesEvent("Set Event Time");

  nsInt64 tick = m_pData->TickFromTime(nsTime::MakeFromSeconds(value));

  for (const auto& cpSel : selection)
  {
    if (m_pData->m_ControlPoints[cpSel].m_iTick != tick)
      Q_EMIT CpMovedEvent(cpSel, tick);
  }

  Q_EMIT EndCpChangesEvent();
}

#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/Dialogs/CurveEditDlg.moc.h>
#include <GuiFoundation/Widgets/Curve1DEditorWidget.moc.h>
#include <GuiFoundation/Widgets/CurveEditData.h>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/Object/ObjectAccessorBase.h>

QByteArray wdQtCurveEditDlg::s_LastDialogGeometry;

wdQtCurveEditDlg::wdQtCurveEditDlg(wdObjectAccessorBase* pObjectAccessor, const wdDocumentObject* pCurveObject, QWidget* pParent)
  : QDialog(pParent)
{
  m_pObjectAccessor = pObjectAccessor;
  m_pCurveObject = pCurveObject;

  setupUi(this);

  wdQtCurve1DEditorWidget* pEdit = CurveEditor;

  connect(pEdit, &wdQtCurve1DEditorWidget::CpMovedEvent, this, &wdQtCurveEditDlg::OnCpMovedEvent);
  connect(pEdit, &wdQtCurve1DEditorWidget::CpDeletedEvent, this, &wdQtCurveEditDlg::OnCpDeletedEvent);
  connect(pEdit, &wdQtCurve1DEditorWidget::TangentMovedEvent, this, &wdQtCurveEditDlg::OnTangentMovedEvent);
  connect(pEdit, &wdQtCurve1DEditorWidget::InsertCpEvent, this, &wdQtCurveEditDlg::OnInsertCpEvent);
  connect(pEdit, &wdQtCurve1DEditorWidget::TangentLinkEvent, this, &wdQtCurveEditDlg::OnTangentLinkEvent);
  connect(pEdit, &wdQtCurve1DEditorWidget::CpTangentModeEvent, this, &wdQtCurveEditDlg::OnCpTangentModeEvent);
  connect(pEdit, &wdQtCurve1DEditorWidget::BeginCpChangesEvent, this, &wdQtCurveEditDlg::OnBeginCpChangesEvent);
  connect(pEdit, &wdQtCurve1DEditorWidget::EndCpChangesEvent, this, &wdQtCurveEditDlg::OnEndCpChangesEvent);
  connect(pEdit, &wdQtCurve1DEditorWidget::BeginOperationEvent, this, &wdQtCurveEditDlg::OnBeginOperationEvent);
  connect(pEdit, &wdQtCurve1DEditorWidget::EndOperationEvent, this, &wdQtCurveEditDlg::OnEndOperationEvent);

  m_pShortcutUndo = new QShortcut(QKeySequence("Ctrl+Z"), this);
  m_pShortcutRedo = new QShortcut(QKeySequence("Ctrl+Y"), this);

  connect(m_pShortcutUndo, &QShortcut::activated, this, &wdQtCurveEditDlg::on_actionUndo_triggered);
  connect(m_pShortcutRedo, &QShortcut::activated, this, &wdQtCurveEditDlg::on_actionRedo_triggered);

  m_Curves.m_Curves.PushBack(WD_DEFAULT_NEW(wdSingleCurveData));

  RetrieveCurveState();

  m_uiActionsUndoBaseline = m_pObjectAccessor->GetObjectManager()->GetDocument()->GetCommandHistory()->GetUndoStackSize();
}

void wdQtCurveEditDlg::RetrieveCurveState()
{
  auto& curve = m_Curves.m_Curves.PeekBack();

  wdInt32 iNumPoints = 0;
  m_pObjectAccessor->GetCount(m_pCurveObject, "ControlPoints", iNumPoints);
  curve->m_ControlPoints.SetCount(iNumPoints);

  wdVariant v;

  // get a local representation of the curve once, so that we can update the preview more efficiently
  for (wdInt32 i = 0; i < iNumPoints; ++i)
  {
    const wdDocumentObject* pPoint = m_pObjectAccessor->GetChildObject(m_pCurveObject, "ControlPoints", i);

    m_pObjectAccessor->GetValue(pPoint, "Tick", v);
    curve->m_ControlPoints[i].m_iTick = v.ConvertTo<wdInt32>();

    m_pObjectAccessor->GetValue(pPoint, "Value", v);
    curve->m_ControlPoints[i].m_fValue = v.ConvertTo<double>();

    m_pObjectAccessor->GetValue(pPoint, "LeftTangent", v);
    curve->m_ControlPoints[i].m_LeftTangent = v.ConvertTo<wdVec2>();

    m_pObjectAccessor->GetValue(pPoint, "RightTangent", v);
    curve->m_ControlPoints[i].m_RightTangent = v.ConvertTo<wdVec2>();

    m_pObjectAccessor->GetValue(pPoint, "Linked", v);
    curve->m_ControlPoints[i].m_bTangentsLinked = v.ConvertTo<bool>();

    m_pObjectAccessor->GetValue(pPoint, "LeftTangentMode", v);
    curve->m_ControlPoints[i].m_LeftTangentMode = (wdCurveTangentMode::Enum)v.ConvertTo<wdInt32>();

    m_pObjectAccessor->GetValue(pPoint, "RightTangentMode", v);
    curve->m_ControlPoints[i].m_RightTangentMode = (wdCurveTangentMode::Enum)v.ConvertTo<wdInt32>();
  }
}

wdQtCurveEditDlg::~wdQtCurveEditDlg()
{
  s_LastDialogGeometry = saveGeometry();
}

void wdQtCurveEditDlg::SetCurveColor(const wdColor& color)
{
  m_Curves.m_Curves.PeekBack()->m_CurveColor = color;
}

void wdQtCurveEditDlg::SetCurveExtents(double fLower, bool bLowerFixed, double fUpper, bool bUpperFixed)
{
  m_fLowerExtents = fLower;
  m_fUpperExtents = fUpper;
  m_bLowerFixed = bLowerFixed;
  m_bUpperFixed = bUpperFixed;
}

void wdQtCurveEditDlg::SetCurveRanges(double fLower, double fUpper)
{
  m_fLowerRange = fLower;
  m_fUpperRange = fUpper;
}

void wdQtCurveEditDlg::reject()
{
  // ignore
}

void wdQtCurveEditDlg::accept()
{
  // ignore
}

void wdQtCurveEditDlg::cancel()
{
  auto& cmd = *m_pObjectAccessor->GetObjectManager()->GetDocument()->GetCommandHistory();
  cmd.Undo(cmd.GetUndoStackSize() - m_uiActionsUndoBaseline);

  QDialog::reject();
}

void wdQtCurveEditDlg::UpdatePreview()
{
  wdQtCurve1DEditorWidget* pEdit = CurveEditor;
  pEdit->SetCurveExtents(m_fLowerExtents, m_fUpperExtents, m_bLowerFixed, m_bUpperFixed);
  pEdit->SetCurveRanges(m_fLowerRange, m_fUpperRange);
  pEdit->SetCurves(m_Curves);
}

void wdQtCurveEditDlg::closeEvent(QCloseEvent*)
{
  cancel();
}

void wdQtCurveEditDlg::showEvent(QShowEvent* e)
{
  QDialog::showEvent(e);

  UpdatePreview();
}

void wdQtCurveEditDlg::OnCpMovedEvent(wdUInt32 curveIdx, wdUInt32 cpIdx, wdInt64 iTickX, double newPosY)
{
  // update the local representation
  {
    auto& cp = m_Curves.m_Curves[curveIdx]->m_ControlPoints[cpIdx];

    if (cp.m_iTick != iTickX || cp.m_fValue != newPosY)
    {
      cp.m_iTick = iTickX;
      cp.m_fValue = newPosY;
    }
  }

  // update the actual object
  {
    const wdDocumentObject* pPoint = m_pObjectAccessor->GetChildObject(m_pCurveObject, "ControlPoints", cpIdx);

    m_pObjectAccessor->SetValue(pPoint, "Tick", iTickX);
    m_pObjectAccessor->SetValue(pPoint, "Value", newPosY);
  }
}

void wdQtCurveEditDlg::OnCpDeletedEvent(wdUInt32 curveIdx, wdUInt32 cpIdx)
{
  // update the local representation
  {
    m_Curves.m_Curves[curveIdx]->m_ControlPoints.RemoveAtAndCopy(cpIdx);
  }

  // update the actual object
  {
    const wdDocumentObject* pPoint = m_pObjectAccessor->GetChildObject(m_pCurveObject, "ControlPoints", cpIdx);
    m_pObjectAccessor->RemoveObject(pPoint);
  }
}

void wdQtCurveEditDlg::OnTangentMovedEvent(wdUInt32 curveIdx, wdUInt32 cpIdx, float newPosX, float newPosY, bool rightTangent)
{
  // update the local representation
  {
    auto& cp = m_Curves.m_Curves[curveIdx]->m_ControlPoints[cpIdx];

    if (rightTangent)
      cp.m_RightTangent.Set(newPosX, newPosY);
    else
      cp.m_LeftTangent.Set(newPosX, newPosY);
  }

  // update the actual object
  {
    const wdDocumentObject* pPoint = m_pObjectAccessor->GetChildObject(m_pCurveObject, "ControlPoints", cpIdx);

    if (rightTangent)
      m_pObjectAccessor->SetValue(pPoint, "RightTangent", wdVec2(newPosX, newPosY));
    else
      m_pObjectAccessor->SetValue(pPoint, "LeftTangent", wdVec2(newPosX, newPosY));
  }
}

void wdQtCurveEditDlg::OnInsertCpEvent(wdUInt32 curveIdx, wdInt64 tickX, double value)
{
  // update the local representation
  {
    wdCurveControlPointData cp;
    cp.m_iTick = tickX;
    cp.m_fValue = value;

    m_Curves.m_Curves[curveIdx]->m_ControlPoints.PushBack(cp);
  }

  // update the actual object
  {
    wdUuid guid;
    m_pObjectAccessor->AddObject(m_pCurveObject, "ControlPoints", -1, wdGetStaticRTTI<wdCurveControlPointData>(), guid);

    const wdDocumentObject* pPoint = m_pObjectAccessor->GetObject(guid);

    m_pObjectAccessor->SetValue(pPoint, "Tick", tickX);
    m_pObjectAccessor->SetValue(pPoint, "Value", value);
  }
}

void wdQtCurveEditDlg::OnTangentLinkEvent(wdUInt32 curveIdx, wdUInt32 cpIdx, bool bLink)
{
  // update the local representation
  {
    auto& cp = m_Curves.m_Curves[curveIdx]->m_ControlPoints[cpIdx];
    cp.m_bTangentsLinked = bLink;
  }

  // update the actual object
  {
    const wdDocumentObject* pPoint = m_pObjectAccessor->GetChildObject(m_pCurveObject, "ControlPoints", cpIdx);

    m_pObjectAccessor->SetValue(pPoint, "Linked", bLink);
  }
}

void wdQtCurveEditDlg::OnCpTangentModeEvent(wdUInt32 curveIdx, wdUInt32 cpIdx, bool rightTangent, int mode)
{
  // update the local representation
  {
    auto& cp = m_Curves.m_Curves[curveIdx]->m_ControlPoints[cpIdx];

    if (rightTangent)
      cp.m_RightTangentMode = (wdCurveTangentMode::Enum)mode;
    else
      cp.m_LeftTangentMode = (wdCurveTangentMode::Enum)mode;
  }

  // update the actual object
  {
    const wdDocumentObject* pPoint = m_pObjectAccessor->GetChildObject(m_pCurveObject, "ControlPoints", cpIdx);

    if (rightTangent)
      m_pObjectAccessor->SetValue(pPoint, "RightTangentMode", mode);
    else
      m_pObjectAccessor->SetValue(pPoint, "LeftTangentMode", mode);
  }
}

void wdQtCurveEditDlg::OnBeginCpChangesEvent(QString name)
{
  m_pObjectAccessor->StartTransaction(name.toUtf8().data());
}

void wdQtCurveEditDlg::OnEndCpChangesEvent()
{
  m_pObjectAccessor->FinishTransaction();

  UpdatePreview();
}

void wdQtCurveEditDlg::OnBeginOperationEvent(QString name)
{
  m_pObjectAccessor->BeginTemporaryCommands(name.toUtf8().data());
}

void wdQtCurveEditDlg::OnEndOperationEvent(bool commit)
{
  if (commit)
    m_pObjectAccessor->FinishTemporaryCommands();
  else
    m_pObjectAccessor->CancelTemporaryCommands();

  UpdatePreview();
}

void wdQtCurveEditDlg::on_actionUndo_triggered()
{
  auto& cmd = *m_pObjectAccessor->GetObjectManager()->GetDocument()->GetCommandHistory();

  if (cmd.CanUndo() && cmd.GetUndoStackSize() > m_uiActionsUndoBaseline)
  {
    cmd.Undo();

    RetrieveCurveState();
    UpdatePreview();
  }
}

void wdQtCurveEditDlg::on_actionRedo_triggered()
{
  auto& cmd = *m_pObjectAccessor->GetObjectManager()->GetDocument()->GetCommandHistory();

  if (cmd.CanRedo())
  {
    cmd.Redo();

    RetrieveCurveState();
    UpdatePreview();
  }
}

void wdQtCurveEditDlg::on_ButtonOk_clicked()
{
  QDialog::accept();
}

void wdQtCurveEditDlg::on_ButtonCancel_clicked()
{
  cancel();
}

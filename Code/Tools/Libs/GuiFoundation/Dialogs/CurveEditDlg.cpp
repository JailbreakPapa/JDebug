#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/Dialogs/CurveEditDlg.moc.h>
#include <GuiFoundation/Widgets/Curve1DEditorWidget.moc.h>
#include <GuiFoundation/Widgets/CurveEditData.h>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/Object/ObjectAccessorBase.h>

QByteArray nsQtCurveEditDlg::s_LastDialogGeometry;

nsQtCurveEditDlg::nsQtCurveEditDlg(nsObjectAccessorBase* pObjectAccessor, const nsDocumentObject* pCurveObject, QWidget* pParent)
  : QDialog(pParent)
{
  m_pObjectAccessor = pObjectAccessor;
  m_pCurveObject = pCurveObject;

  setupUi(this);

  nsQtCurve1DEditorWidget* pEdit = CurveEditor;

  connect(pEdit, &nsQtCurve1DEditorWidget::CpMovedEvent, this, &nsQtCurveEditDlg::OnCpMovedEvent);
  connect(pEdit, &nsQtCurve1DEditorWidget::CpDeletedEvent, this, &nsQtCurveEditDlg::OnCpDeletedEvent);
  connect(pEdit, &nsQtCurve1DEditorWidget::TangentMovedEvent, this, &nsQtCurveEditDlg::OnTangentMovedEvent);
  connect(pEdit, &nsQtCurve1DEditorWidget::InsertCpEvent, this, &nsQtCurveEditDlg::OnInsertCpEvent);
  connect(pEdit, &nsQtCurve1DEditorWidget::TangentLinkEvent, this, &nsQtCurveEditDlg::OnTangentLinkEvent);
  connect(pEdit, &nsQtCurve1DEditorWidget::CpTangentModeEvent, this, &nsQtCurveEditDlg::OnCpTangentModeEvent);
  connect(pEdit, &nsQtCurve1DEditorWidget::BeginCpChangesEvent, this, &nsQtCurveEditDlg::OnBeginCpChangesEvent);
  connect(pEdit, &nsQtCurve1DEditorWidget::EndCpChangesEvent, this, &nsQtCurveEditDlg::OnEndCpChangesEvent);
  connect(pEdit, &nsQtCurve1DEditorWidget::BeginOperationEvent, this, &nsQtCurveEditDlg::OnBeginOperationEvent);
  connect(pEdit, &nsQtCurve1DEditorWidget::EndOperationEvent, this, &nsQtCurveEditDlg::OnEndOperationEvent);

  m_pShortcutUndo = new QShortcut(QKeySequence("Ctrl+Z"), this);
  m_pShortcutRedo = new QShortcut(QKeySequence("Ctrl+Y"), this);

  connect(m_pShortcutUndo, &QShortcut::activated, this, &nsQtCurveEditDlg::on_actionUndo_triggered);
  connect(m_pShortcutRedo, &QShortcut::activated, this, &nsQtCurveEditDlg::on_actionRedo_triggered);

  m_Curves.m_Curves.PushBack(NS_DEFAULT_NEW(nsSingleCurveData));

  RetrieveCurveState();

  m_uiActionsUndoBaseline = m_pObjectAccessor->GetObjectManager()->GetDocument()->GetCommandHistory()->GetUndoStackSize();
}

void nsQtCurveEditDlg::RetrieveCurveState()
{
  auto& curve = m_Curves.m_Curves.PeekBack();

  nsInt32 iNumPoints = 0;
  m_pObjectAccessor->GetCount(m_pCurveObject, "ControlPoints", iNumPoints).AssertSuccess();
  curve->m_ControlPoints.SetCount(iNumPoints);

  nsVariant v;

  // get a local representation of the curve once, so that we can update the preview more efficiently
  for (nsInt32 i = 0; i < iNumPoints; ++i)
  {
    const nsDocumentObject* pPoint = m_pObjectAccessor->GetChildObject(m_pCurveObject, "ControlPoints", i);

    m_pObjectAccessor->GetValue(pPoint, "Tick", v).AssertSuccess();
    curve->m_ControlPoints[i].m_iTick = v.ConvertTo<nsInt32>();

    m_pObjectAccessor->GetValue(pPoint, "Value", v).AssertSuccess();
    curve->m_ControlPoints[i].m_fValue = v.ConvertTo<double>();

    m_pObjectAccessor->GetValue(pPoint, "LeftTangent", v).AssertSuccess();
    curve->m_ControlPoints[i].m_LeftTangent = v.ConvertTo<nsVec2>();

    m_pObjectAccessor->GetValue(pPoint, "RightTangent", v).AssertSuccess();
    curve->m_ControlPoints[i].m_RightTangent = v.ConvertTo<nsVec2>();

    m_pObjectAccessor->GetValue(pPoint, "Linked", v).AssertSuccess();
    curve->m_ControlPoints[i].m_bTangentsLinked = v.ConvertTo<bool>();

    m_pObjectAccessor->GetValue(pPoint, "LeftTangentMode", v).AssertSuccess();
    curve->m_ControlPoints[i].m_LeftTangentMode = (nsCurveTangentMode::Enum)v.ConvertTo<nsInt32>();

    m_pObjectAccessor->GetValue(pPoint, "RightTangentMode", v).AssertSuccess();
    curve->m_ControlPoints[i].m_RightTangentMode = (nsCurveTangentMode::Enum)v.ConvertTo<nsInt32>();
  }
}

nsQtCurveEditDlg::~nsQtCurveEditDlg()
{
  s_LastDialogGeometry = saveGeometry();
}

void nsQtCurveEditDlg::SetCurveColor(const nsColor& color)
{
  m_Curves.m_Curves.PeekBack()->m_CurveColor = color;
}

void nsQtCurveEditDlg::SetCurveExtents(double fLower, bool bLowerFixed, double fUpper, bool bUpperFixed)
{
  m_fLowerExtents = fLower;
  m_fUpperExtents = fUpper;
  m_bLowerFixed = bLowerFixed;
  m_bUpperFixed = bUpperFixed;
}

void nsQtCurveEditDlg::SetCurveRanges(double fLower, double fUpper)
{
  m_fLowerRange = fLower;
  m_fUpperRange = fUpper;
}

void nsQtCurveEditDlg::reject()
{
  // ignore
}

void nsQtCurveEditDlg::accept()
{
  // ignore
}

void nsQtCurveEditDlg::cancel()
{
  auto& cmd = *m_pObjectAccessor->GetObjectManager()->GetDocument()->GetCommandHistory();
  cmd.Undo(cmd.GetUndoStackSize() - m_uiActionsUndoBaseline).AssertSuccess();

  QDialog::reject();
}

void nsQtCurveEditDlg::UpdatePreview()
{
  nsQtCurve1DEditorWidget* pEdit = CurveEditor;
  pEdit->SetCurveExtents(m_fLowerExtents, m_fUpperExtents, m_bLowerFixed, m_bUpperFixed);
  pEdit->SetCurveRanges(m_fLowerRange, m_fUpperRange);
  pEdit->SetCurves(m_Curves);
}

void nsQtCurveEditDlg::closeEvent(QCloseEvent*)
{
  cancel();
}

void nsQtCurveEditDlg::showEvent(QShowEvent* e)
{
  QDialog::showEvent(e);

  UpdatePreview();
}

void nsQtCurveEditDlg::OnCpMovedEvent(nsUInt32 curveIdx, nsUInt32 cpIdx, nsInt64 iTickX, double newPosY)
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
    const nsDocumentObject* pPoint = m_pObjectAccessor->GetChildObject(m_pCurveObject, "ControlPoints", cpIdx);

    m_pObjectAccessor->SetValue(pPoint, "Tick", iTickX).AssertSuccess();
    m_pObjectAccessor->SetValue(pPoint, "Value", newPosY).AssertSuccess();
  }
}

void nsQtCurveEditDlg::OnCpDeletedEvent(nsUInt32 curveIdx, nsUInt32 cpIdx)
{
  // update the local representation
  {
    m_Curves.m_Curves[curveIdx]->m_ControlPoints.RemoveAtAndCopy(cpIdx);
  }

  // update the actual object
  {
    const nsDocumentObject* pPoint = m_pObjectAccessor->GetChildObject(m_pCurveObject, "ControlPoints", cpIdx);
    m_pObjectAccessor->RemoveObject(pPoint).AssertSuccess();
  }
}

void nsQtCurveEditDlg::OnTangentMovedEvent(nsUInt32 curveIdx, nsUInt32 cpIdx, float newPosX, float newPosY, bool rightTangent)
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
    const nsDocumentObject* pPoint = m_pObjectAccessor->GetChildObject(m_pCurveObject, "ControlPoints", cpIdx);

    if (rightTangent)
      m_pObjectAccessor->SetValue(pPoint, "RightTangent", nsVec2(newPosX, newPosY)).AssertSuccess();
    else
      m_pObjectAccessor->SetValue(pPoint, "LeftTangent", nsVec2(newPosX, newPosY)).AssertSuccess();
  }
}

void nsQtCurveEditDlg::OnInsertCpEvent(nsUInt32 curveIdx, nsInt64 tickX, double value)
{
  // update the local representation
  {
    nsCurveControlPointData cp;
    cp.m_iTick = tickX;
    cp.m_fValue = value;

    m_Curves.m_Curves[curveIdx]->m_ControlPoints.PushBack(cp);
  }

  // update the actual object
  {
    nsUuid guid;
    m_pObjectAccessor->AddObject(m_pCurveObject, "ControlPoints", -1, nsGetStaticRTTI<nsCurveControlPointData>(), guid).AssertSuccess();

    const nsDocumentObject* pPoint = m_pObjectAccessor->GetObject(guid);

    m_pObjectAccessor->SetValue(pPoint, "Tick", tickX).AssertSuccess();
    m_pObjectAccessor->SetValue(pPoint, "Value", value).AssertSuccess();
  }
}

void nsQtCurveEditDlg::OnTangentLinkEvent(nsUInt32 curveIdx, nsUInt32 cpIdx, bool bLink)
{
  // update the local representation
  {
    auto& cp = m_Curves.m_Curves[curveIdx]->m_ControlPoints[cpIdx];
    cp.m_bTangentsLinked = bLink;
  }

  // update the actual object
  {
    const nsDocumentObject* pPoint = m_pObjectAccessor->GetChildObject(m_pCurveObject, "ControlPoints", cpIdx);

    m_pObjectAccessor->SetValue(pPoint, "Linked", bLink).AssertSuccess();
  }
}

void nsQtCurveEditDlg::OnCpTangentModeEvent(nsUInt32 curveIdx, nsUInt32 cpIdx, bool rightTangent, int mode)
{
  // update the local representation
  {
    auto& cp = m_Curves.m_Curves[curveIdx]->m_ControlPoints[cpIdx];

    if (rightTangent)
      cp.m_RightTangentMode = (nsCurveTangentMode::Enum)mode;
    else
      cp.m_LeftTangentMode = (nsCurveTangentMode::Enum)mode;
  }

  // update the actual object
  {
    const nsDocumentObject* pPoint = m_pObjectAccessor->GetChildObject(m_pCurveObject, "ControlPoints", cpIdx);

    if (rightTangent)
      m_pObjectAccessor->SetValue(pPoint, "RightTangentMode", mode).AssertSuccess();
    else
      m_pObjectAccessor->SetValue(pPoint, "LeftTangentMode", mode).AssertSuccess();
  }
}

void nsQtCurveEditDlg::OnBeginCpChangesEvent(QString name)
{
  m_pObjectAccessor->StartTransaction(name.toUtf8().data());
}

void nsQtCurveEditDlg::OnEndCpChangesEvent()
{
  m_pObjectAccessor->FinishTransaction();

  UpdatePreview();
}

void nsQtCurveEditDlg::OnBeginOperationEvent(QString name)
{
  m_pObjectAccessor->BeginTemporaryCommands(name.toUtf8().data());
}

void nsQtCurveEditDlg::OnEndOperationEvent(bool commit)
{
  if (commit)
    m_pObjectAccessor->FinishTemporaryCommands();
  else
    m_pObjectAccessor->CancelTemporaryCommands();

  UpdatePreview();
}

void nsQtCurveEditDlg::on_actionUndo_triggered()
{
  auto& cmd = *m_pObjectAccessor->GetObjectManager()->GetDocument()->GetCommandHistory();

  if (cmd.CanUndo() && cmd.GetUndoStackSize() > m_uiActionsUndoBaseline)
  {
    cmd.Undo().IgnoreResult();

    RetrieveCurveState();
    UpdatePreview();
  }
}

void nsQtCurveEditDlg::on_actionRedo_triggered()
{
  auto& cmd = *m_pObjectAccessor->GetObjectManager()->GetDocument()->GetCommandHistory();

  if (cmd.CanRedo())
  {
    cmd.Redo().IgnoreResult();

    RetrieveCurveState();
    UpdatePreview();
  }
}

void nsQtCurveEditDlg::on_ButtonOk_clicked()
{
  QDialog::accept();
}

void nsQtCurveEditDlg::on_ButtonCancel_clicked()
{
  cancel();
}

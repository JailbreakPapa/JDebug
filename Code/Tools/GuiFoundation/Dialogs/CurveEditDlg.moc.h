#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/Widgets/CurveEditData.h>
#include <GuiFoundation/ui_CurveEditDlg.h>
#include <QDialog>

class wdCurveGroupData;
class wdObjectAccessorBase;
class wdDocumentObject;

class WD_GUIFOUNDATION_DLL wdQtCurveEditDlg : public QDialog, Ui_CurveEditDlg
{
  Q_OBJECT
public:
  wdQtCurveEditDlg(wdObjectAccessorBase* pObjectAccessor, const wdDocumentObject* pCurveObject, QWidget* pParent);
  ~wdQtCurveEditDlg();

  static QByteArray GetLastDialogGeometry() { return s_LastDialogGeometry; }

  void SetCurveColor(const wdColor& color);
  void SetCurveExtents(double fLower, bool bLowerFixed, double fUpper, bool bUpperFixed);
  void SetCurveRanges(double fLower, double fUpper);

  virtual void reject() override;
  virtual void accept() override;

  void cancel();

Q_SIGNALS:

private Q_SLOTS:
  void OnCpMovedEvent(wdUInt32 curveIdx, wdUInt32 cpIdx, wdInt64 iTickX, double newPosY);
  void OnCpDeletedEvent(wdUInt32 curveIdx, wdUInt32 cpIdx);
  void OnTangentMovedEvent(wdUInt32 curveIdx, wdUInt32 cpIdx, float newPosX, float newPosY, bool rightTangent);
  void OnInsertCpEvent(wdUInt32 uiCurveIdx, wdInt64 tickX, double value);
  void OnTangentLinkEvent(wdUInt32 curveIdx, wdUInt32 cpIdx, bool bLink);
  void OnCpTangentModeEvent(wdUInt32 curveIdx, wdUInt32 cpIdx, bool rightTangent, int mode); // wdCurveTangentMode

  void OnBeginCpChangesEvent(QString name);
  void OnEndCpChangesEvent();

  void OnBeginOperationEvent(QString name);
  void OnEndOperationEvent(bool commit);

  void on_actionUndo_triggered();
  void on_actionRedo_triggered();
  void on_ButtonOk_clicked();
  void on_ButtonCancel_clicked();

private:
  static QByteArray s_LastDialogGeometry;

  void RetrieveCurveState();
  void UpdatePreview();

  double m_fLowerRange = -wdMath::HighValue<double>();
  double m_fUpperRange = wdMath::HighValue<double>();
  double m_fLowerExtents = 0.0;
  double m_fUpperExtents = 1.0;
  bool m_bLowerFixed = false;
  bool m_bUpperFixed = false;
  bool m_bCurveLengthIsFixed = false;
  wdCurveGroupData m_Curves;
  wdUInt32 m_uiActionsUndoBaseline = 0;

  QShortcut* m_pShortcutUndo = nullptr;
  QShortcut* m_pShortcutRedo = nullptr;

  wdObjectAccessorBase* m_pObjectAccessor = nullptr;
  const wdDocumentObject* m_pCurveObject = nullptr;

protected:
  virtual void closeEvent(QCloseEvent* e) override;
  virtual void showEvent(QShowEvent* e) override;
};

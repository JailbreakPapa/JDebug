#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/Widgets/CurveEditData.h>
#include <GuiFoundation/ui_CurveEditDlg.h>
#include <QDialog>

class nsCurveGroupData;
class nsObjectAccessorBase;
class nsDocumentObject;

class NS_GUIFOUNDATION_DLL nsQtCurveEditDlg : public QDialog, Ui_CurveEditDlg
{
  Q_OBJECT
public:
  nsQtCurveEditDlg(nsObjectAccessorBase* pObjectAccessor, const nsDocumentObject* pCurveObject, QWidget* pParent);
  ~nsQtCurveEditDlg();

  static QByteArray GetLastDialogGeometry() { return s_LastDialogGeometry; }

  void SetCurveColor(const nsColor& color);
  void SetCurveExtents(double fLower, bool bLowerFixed, double fUpper, bool bUpperFixed);
  void SetCurveRanges(double fLower, double fUpper);

  virtual void reject() override;
  virtual void accept() override;

  void cancel();

Q_SIGNALS:

private Q_SLOTS:
  void OnCpMovedEvent(nsUInt32 curveIdx, nsUInt32 cpIdx, nsInt64 iTickX, double newPosY);
  void OnCpDeletedEvent(nsUInt32 curveIdx, nsUInt32 cpIdx);
  void OnTangentMovedEvent(nsUInt32 curveIdx, nsUInt32 cpIdx, float newPosX, float newPosY, bool rightTangent);
  void OnInsertCpEvent(nsUInt32 uiCurveIdx, nsInt64 tickX, double value);
  void OnTangentLinkEvent(nsUInt32 curveIdx, nsUInt32 cpIdx, bool bLink);
  void OnCpTangentModeEvent(nsUInt32 curveIdx, nsUInt32 cpIdx, bool rightTangent, int mode); // nsCurveTangentMode

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

  double m_fLowerRange = -nsMath::HighValue<double>();
  double m_fUpperRange = nsMath::HighValue<double>();
  double m_fLowerExtents = 0.0;
  double m_fUpperExtents = 1.0;
  bool m_bLowerFixed = false;
  bool m_bUpperFixed = false;
  bool m_bCurveLengthIsFixed = false;
  nsCurveGroupData m_Curves;
  nsUInt32 m_uiActionsUndoBaseline = 0;

  QShortcut* m_pShortcutUndo = nullptr;
  QShortcut* m_pShortcutRedo = nullptr;

  nsObjectAccessorBase* m_pObjectAccessor = nullptr;
  const nsDocumentObject* m_pCurveObject = nullptr;

protected:
  virtual void closeEvent(QCloseEvent* e) override;
  virtual void showEvent(QShowEvent* e) override;
};

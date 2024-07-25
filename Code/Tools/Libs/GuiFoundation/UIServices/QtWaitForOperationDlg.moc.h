#pragma once

#include <Foundation/Strings/String.h>
#include <Foundation/Types/Delegate.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/ui_QtWaitForOperationDlg.h>

class QWinTaskbarProgress;
class QWinTaskbarButton;

class NS_GUIFOUNDATION_DLL nsQtWaitForOperationDlg : public QDialog, public Ui_QtWaitForOperationDlg
{
  Q_OBJECT

public:
  nsQtWaitForOperationDlg(QWidget* pParent);
  ~nsQtWaitForOperationDlg();

  nsDelegate<bool()> m_OnIdle;

private Q_SLOTS:
  void on_ButtonCancel_clicked();
  void onIdle();
};

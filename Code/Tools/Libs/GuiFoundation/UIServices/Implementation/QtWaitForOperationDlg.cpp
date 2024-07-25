#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/UIServices/QtWaitForOperationDlg.moc.h>
#include <QTimer>

nsQtWaitForOperationDlg::nsQtWaitForOperationDlg(QWidget* pParent)
  : QDialog(pParent)
{
  setupUi(this);

  QTimer::singleShot(10, this, &nsQtWaitForOperationDlg::onIdle);
}

nsQtWaitForOperationDlg::~nsQtWaitForOperationDlg() = default;

void nsQtWaitForOperationDlg::on_ButtonCancel_clicked()
{
  reject();
}

void nsQtWaitForOperationDlg::onIdle()
{
  if (m_OnIdle())
  {
    QTimer::singleShot(10, this, &nsQtWaitForOperationDlg::onIdle);
  }
  else
  {
    accept();
  }
}

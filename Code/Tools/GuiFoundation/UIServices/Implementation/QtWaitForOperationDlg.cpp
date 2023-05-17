#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/UIServices/QtWaitForOperationDlg.moc.h>
#include <QTimer>

wdQtWaitForOperationDlg::wdQtWaitForOperationDlg(QWidget* pParent)
  : QDialog(pParent)
{
  setupUi(this);

  QTimer::singleShot(10, this, &wdQtWaitForOperationDlg::onIdle);
}

wdQtWaitForOperationDlg::~wdQtWaitForOperationDlg()
{
}

void wdQtWaitForOperationDlg::on_ButtonCancel_clicked()
{
  reject();
}

void wdQtWaitForOperationDlg::onIdle()
{
  if (m_OnIdle())
  {
    QTimer::singleShot(10, this, &wdQtWaitForOperationDlg::onIdle);
  }
  else
  {
    accept();
  }
}

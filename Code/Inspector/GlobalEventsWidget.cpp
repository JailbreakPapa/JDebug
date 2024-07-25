#include <Inspector/InspectorPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <Inspector/GlobalEventsWidget.moc.h>
#include <Inspector/MainWindow.moc.h>
#include <qlistwidget.h>

nsQtGlobalEventsWidget* nsQtGlobalEventsWidget::s_pWidget = nullptr;

nsQtGlobalEventsWidget::nsQtGlobalEventsWidget(QWidget* pParent)
  : ads::CDockWidget("Global Events", pParent)
{
  s_pWidget = this;

  setupUi(this);
  setWidget(GlobalEventsFrame);

  setIcon(QIcon(":/Icons/Icons/GlobalEvent.svg"));

  ResetStats();
}

void nsQtGlobalEventsWidget::ResetStats()
{
  m_Events.Clear();
  TableEvents->clear();

  {
    QStringList Headers;
    Headers.append(" Event ");
    Headers.append(" Times Fired ");
    Headers.append(" # Handlers ");
    Headers.append(" # Handlers Once ");

    TableEvents->setColumnCount(static_cast<int>(Headers.size()));

    TableEvents->setHorizontalHeaderLabels(Headers);
    TableEvents->horizontalHeader()->show();
  }
}

void nsQtGlobalEventsWidget::ProcessTelemetry(void* pUnuseed)
{
  if (!s_pWidget)
    return;

  nsTelemetryMessage msg;

  bool bUpdateTable = false;
  bool bFillTable = false;

  while (nsTelemetry::RetrieveMessage('EVNT', msg) == NS_SUCCESS)
  {
    if (msg.GetMessageID() == ' CLR')
    {
      s_pWidget->m_Events.Clear();
    }

    if (msg.GetMessageID() == 'DATA')
    {
      nsString sName;
      msg.GetReader() >> sName;

      GlobalEventsData& sd = s_pWidget->m_Events[sName];

      msg.GetReader() >> sd.m_uiTimesFired;
      msg.GetReader() >> sd.m_uiNumHandlers;
      msg.GetReader() >> sd.m_uiNumHandlersOnce;

      if (sd.m_iTableRow == -1)
        bUpdateTable = true;

      bFillTable = true;
    }
  }

  if (bUpdateTable)
    s_pWidget->UpdateTable(true);
  else if (bFillTable)
    s_pWidget->UpdateTable(false);
}

void nsQtGlobalEventsWidget::UpdateTable(bool bRecreate)
{
  nsQtScopedUpdatesDisabled _1(TableEvents);

  if (bRecreate)
  {
    TableEvents->clear();
    TableEvents->setRowCount(m_Events.GetCount());

    QStringList Headers;
    Headers.append(" ");
    Headers.append(" Event ");
    Headers.append(" Times Fired ");
    Headers.append(" # Handlers ");
    Headers.append(" # Handlers Once ");

    TableEvents->setColumnCount(static_cast<int>(Headers.size()));

    TableEvents->setHorizontalHeaderLabels(Headers);
    TableEvents->horizontalHeader()->show();

    nsStringBuilder sTemp;

    nsInt32 iRow = 0;
    for (nsMap<nsString, GlobalEventsData>::Iterator it = m_Events.GetIterator(); it.IsValid(); ++it)
    {
      it.Value().m_iTableRow = iRow;

      QLabel* pIcon = new QLabel();
      QIcon icon = nsQtUiServices::GetCachedIconResource(":/Icons/Icons/GlobalEvent.svg");
      pIcon->setPixmap(icon.pixmap(QSize(24, 24)));
      pIcon->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
      TableEvents->setCellWidget(iRow, 0, pIcon);

      sTemp.SetFormat("  {0}  ", it.Key());
      TableEvents->setCellWidget(iRow, 1, new QLabel(sTemp.GetData())); // Event

      sTemp.SetFormat("  {0}  ", it.Value().m_uiTimesFired);
      TableEvents->setCellWidget(iRow, 2, new QLabel(sTemp.GetData()));

      sTemp.SetFormat("  {0}  ", it.Value().m_uiNumHandlers);
      TableEvents->setCellWidget(iRow, 3, new QLabel(sTemp.GetData()));

      sTemp.SetFormat("  {0}  ", it.Value().m_uiNumHandlersOnce);
      TableEvents->setCellWidget(iRow, 4, new QLabel(sTemp.GetData()));

      ++iRow;
    }

    TableEvents->resizeColumnsToContents();
  }
  else
  {
    nsStringBuilder sTemp;

    nsInt32 iRow = 0;
    for (nsMap<nsString, GlobalEventsData>::Iterator it = m_Events.GetIterator(); it.IsValid(); ++it)
    {
      sTemp.SetFormat("  {0}  ", it.Value().m_uiTimesFired);
      ((QLabel*)TableEvents->cellWidget(iRow, 2))->setText(sTemp.GetData());

      sTemp.SetFormat("  {0}  ", it.Value().m_uiNumHandlers);
      ((QLabel*)TableEvents->cellWidget(iRow, 3))->setText(sTemp.GetData());

      sTemp.SetFormat("  {0}  ", it.Value().m_uiNumHandlersOnce);
      ((QLabel*)TableEvents->cellWidget(iRow, 4))->setText(sTemp.GetData());

      ++iRow;
    }
  }
}

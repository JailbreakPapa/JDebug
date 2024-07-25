#include <Inspector/InspectorPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <Inspector/MainWindow.moc.h>
#include <Inspector/SubsystemsWidget.moc.h>

nsQtSubsystemsWidget* nsQtSubsystemsWidget::s_pWidget = nullptr;

nsQtSubsystemsWidget::nsQtSubsystemsWidget(QWidget* pParent)
  : ads::CDockWidget("Subsystem Widget", pParent)
{
  s_pWidget = this;

  setupUi(this);
  setWidget(TableSubsystems);

  setIcon(QIcon(":/Icons/Icons/Subsystem.svg"));

  ResetStats();
}

void nsQtSubsystemsWidget::ResetStats()
{
  m_bUpdateSubsystems = true;
  m_Subsystems.Clear();
}


void nsQtSubsystemsWidget::UpdateStats()
{
  UpdateSubSystems();
}

void nsQtSubsystemsWidget::UpdateSubSystems()
{
  if (!m_bUpdateSubsystems)
    return;

  m_bUpdateSubsystems = false;

  nsQtScopedUpdatesDisabled _1(TableSubsystems);

  TableSubsystems->clear();

  TableSubsystems->setRowCount(m_Subsystems.GetCount());

  QStringList Headers;
  Headers.append("");
  Headers.append(" SubSystem ");
  Headers.append(" Plugin ");
  Headers.append(" Startup Done ");
  Headers.append(" Dependencies ");

  TableSubsystems->setColumnCount(static_cast<int>(Headers.size()));

  TableSubsystems->setHorizontalHeaderLabels(Headers);

  {
    nsStringBuilder sTemp;
    nsInt32 iRow = 0;

    for (nsMap<nsString, SubsystemData>::Iterator it = m_Subsystems.GetIterator(); it.IsValid(); ++it)
    {
      const SubsystemData& ssd = it.Value();

      QLabel* pIcon = new QLabel();
      QIcon icon = nsQtUiServices::GetCachedIconResource(":/Icons/Icons/Subsystem.svg");
      pIcon->setPixmap(icon.pixmap(QSize(24, 24)));
      pIcon->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
      TableSubsystems->setCellWidget(iRow, 0, pIcon);

      sTemp.SetFormat("  {0}  ", it.Key());
      TableSubsystems->setCellWidget(iRow, 1, new QLabel(sTemp.GetData()));

      sTemp.SetFormat("  {0}  ", ssd.m_sPlugin);
      TableSubsystems->setCellWidget(iRow, 2, new QLabel(sTemp.GetData()));

      if (ssd.m_bStartupDone[nsStartupStage::HighLevelSystems])
        TableSubsystems->setCellWidget(iRow, 3, new QLabel("<p><span style=\"font-weight:600; color:#00aa00;\">  Engine  </span></p>"));
      else if (ssd.m_bStartupDone[nsStartupStage::CoreSystems])
        TableSubsystems->setCellWidget(iRow, 3, new QLabel("<p><span style=\"font-weight:600; color:#5555ff;\">  Core  </span></p>"));
      else if (ssd.m_bStartupDone[nsStartupStage::BaseSystems])
        TableSubsystems->setCellWidget(iRow, 3, new QLabel("<p><span style=\"font-weight:600; color:#cece00;\">  Base  </span></p>"));
      else
        TableSubsystems->setCellWidget(iRow, 3, new QLabel("<p><span style=\"font-weight:600; color:#ff0000;\">Not Initialized</span></p>"));

      ((QLabel*)TableSubsystems->cellWidget(iRow, 3))->setAlignment(Qt::AlignHCenter);

      sTemp.SetFormat("  {0}  ", ssd.m_sDependencies);
      TableSubsystems->setCellWidget(iRow, 4, new QLabel(sTemp.GetData()));

      ++iRow;
    }
  }

  TableSubsystems->resizeColumnsToContents();
}

void nsQtSubsystemsWidget::ProcessTelemetry(void* pUnuseed)
{
  if (!s_pWidget)
    return;

  nsTelemetryMessage Msg;

  while (nsTelemetry::RetrieveMessage('STRT', Msg) == NS_SUCCESS)
  {
    switch (Msg.GetMessageID())
    {
      case ' CLR':
      {
        s_pWidget->m_Subsystems.Clear();
        s_pWidget->m_bUpdateSubsystems = true;
      }
      break;

      case 'SYST':
      {
        nsString sGroup, sSystem;

        Msg.GetReader() >> sGroup;
        Msg.GetReader() >> sSystem;

        nsStringBuilder sFinalName = sGroup.GetData();
        sFinalName.Append("::");
        sFinalName.Append(sSystem.GetData());

        SubsystemData& ssd = s_pWidget->m_Subsystems[sFinalName.GetData()];

        Msg.GetReader() >> ssd.m_sPlugin;

        for (nsUInt32 i = 0; i < nsStartupStage::ENUM_COUNT; ++i)
          Msg.GetReader() >> ssd.m_bStartupDone[i];

        nsUInt8 uiDependencies = 0;
        Msg.GetReader() >> uiDependencies;

        nsStringBuilder sAllDeps;

        nsString sDep;
        for (nsUInt8 i = 0; i < uiDependencies; ++i)
        {
          Msg.GetReader() >> sDep;

          if (sAllDeps.IsEmpty())
            sAllDeps = sDep.GetData();
          else
          {
            sAllDeps.Append(" | ");
            sAllDeps.Append(sDep.GetData());
          }
        }

        ssd.m_sDependencies = sAllDeps.GetData();

        s_pWidget->m_bUpdateSubsystems = true;
      }
      break;
    }
  }
}

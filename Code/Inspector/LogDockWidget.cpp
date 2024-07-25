#include <Inspector/InspectorPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <Foundation/Logging/LogEntry.h>
#include <GuiFoundation/Models/LogModel.moc.h>
#include <Inspector/LogDockWidget.moc.h>
#include <Inspector/MainWindow.moc.h>
#include <qlistwidget.h>

nsQtLogDockWidget* nsQtLogDockWidget::s_pWidget = nullptr;

nsQtLogDockWidget::nsQtLogDockWidget(QWidget* pParent)
  : ads::CDockWidget("Log", pParent)
{
  s_pWidget = this;
  setupUi(this);
  LogWidget->GetSearchWidget()->setPlaceholderText(QStringLiteral("Search Log"));

  setIcon(QIcon(":/Icons/Icons/Log.svg"));

  this->setWidget(LogWidget);
}

void nsQtLogDockWidget::ResetStats()
{
  LogWidget->GetLog()->Clear();
}

void nsQtLogDockWidget::Log(const nsFormatString& text)
{
  nsStringBuilder tmp;

  nsLogEntry lm;
  lm.m_sMsg = text.GetText(tmp);
  lm.m_Type = nsLogMsgType::InfoMsg;
  lm.m_uiIndentation = 0;
  LogWidget->GetLog()->AddLogMsg(lm);
}

void nsQtLogDockWidget::ProcessTelemetry(void* pUnuseed)
{
  if (!s_pWidget)
    return;

  nsTelemetryMessage Msg;

  while (nsTelemetry::RetrieveMessage(' LOG', Msg) == NS_SUCCESS)
  {
    nsLogEntry lm;
    nsInt8 iEventType = 0;

    Msg.GetReader() >> iEventType;
    Msg.GetReader() >> lm.m_uiIndentation;
    Msg.GetReader() >> lm.m_sTag;
    Msg.GetReader() >> lm.m_sMsg;

    if (iEventType == nsLogMsgType::EndGroup)
      Msg.GetReader() >> lm.m_fSeconds;

    lm.m_Type = (nsLogMsgType::Enum)iEventType;
    s_pWidget->LogWidget->GetLog()->AddLogMsg(lm);
  }
}

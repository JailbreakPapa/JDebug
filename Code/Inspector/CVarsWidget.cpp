#include <Inspector/InspectorPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <Inspector/CVarsWidget.moc.h>
#include <Inspector/MainWindow.moc.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlistwidget.h>
#include <qspinbox.h>

class nsCommandInterpreterInspector : public nsCommandInterpreter
{
public:
  virtual void Interpret(nsCommandInterpreterState& inout_state) override
  {
    nsTelemetryMessage Msg;
    Msg.SetMessageID('CMD', 'EXEC');
    Msg.GetWriter() << inout_state.m_sInput;
    nsTelemetry::SendToServer(Msg);
  }

  virtual void AutoComplete(nsCommandInterpreterState& inout_state) override
  {
    nsTelemetryMessage Msg;
    Msg.SetMessageID('CMD', 'COMP');
    Msg.GetWriter() << inout_state.m_sInput;
    nsTelemetry::SendToServer(Msg);
  }
};

nsQtCVarsWidget* nsQtCVarsWidget::s_pWidget = nullptr;

nsQtCVarsWidget::nsQtCVarsWidget(QWidget* pParent)
  : ads::CDockWidget("CVars", pParent)
{
  s_pWidget = this;

  setupUi(this);
  setWidget(CVarWidget);

  setIcon(QIcon(":/GuiFoundation/Icons/CVar.svg"));

  connect(CVarWidget, &nsQtCVarWidget::onBoolChanged, this, &nsQtCVarsWidget::BoolChanged);
  connect(CVarWidget, &nsQtCVarWidget::onFloatChanged, this, &nsQtCVarsWidget::FloatChanged);
  connect(CVarWidget, &nsQtCVarWidget::onIntChanged, this, &nsQtCVarsWidget::IntChanged);
  connect(CVarWidget, &nsQtCVarWidget::onStringChanged, this, &nsQtCVarsWidget::StringChanged);

  CVarWidget->GetConsole().SetCommandInterpreter(NS_DEFAULT_NEW(nsCommandInterpreterInspector));

  ResetStats();
}

void nsQtCVarsWidget::ResetStats()
{
  m_CVarsBackup = m_CVars;
  m_CVars.Clear();
  CVarWidget->Clear();
}

void nsQtCVarsWidget::ProcessTelemetry(void* pUnuseed)
{
  if (!s_pWidget)
    return;

  nsTelemetryMessage msg;

  bool bUpdateCVarsTable = false;
  bool bFillCVarsTable = false;

  while (nsTelemetry::RetrieveMessage('CVAR', msg) == NS_SUCCESS)
  {
    if (msg.GetMessageID() == ' CLR')
    {
      s_pWidget->m_CVars.Clear();
    }

    if (msg.GetMessageID() == 'SYNC')
    {
      for (auto it = s_pWidget->m_CVars.GetIterator(); it.IsValid(); ++it)
      {
        auto var = s_pWidget->m_CVarsBackup.Find(it.Key());

        if (var.IsValid() && it.Value().m_uiType == var.Value().m_uiType)
        {
          it.Value().m_bValue = var.Value().m_bValue;
          it.Value().m_fValue = var.Value().m_fValue;
          it.Value().m_sValue = var.Value().m_sValue;
          it.Value().m_iValue = var.Value().m_iValue;
        }
      }

      s_pWidget->CVarWidget->RebuildCVarUI(s_pWidget->m_CVars);

      s_pWidget->SyncAllCVarsToServer();
    }

    if (msg.GetMessageID() == 'DATA')
    {
      nsString sName;
      msg.GetReader() >> sName;

      nsCVarWidgetData& sd = s_pWidget->m_CVars[sName];

      msg.GetReader() >> sd.m_sPlugin;
      msg.GetReader() >> sd.m_uiType;
      msg.GetReader() >> sd.m_sDescription;

      switch (sd.m_uiType)
      {
        case nsCVarType::Bool:
          msg.GetReader() >> sd.m_bValue;
          break;
        case nsCVarType::Float:
          msg.GetReader() >> sd.m_fValue;
          break;
        case nsCVarType::Int:
          msg.GetReader() >> sd.m_iValue;
          break;
        case nsCVarType::String:
          msg.GetReader() >> sd.m_sValue;
          break;
      }

      if (sd.m_bNewEntry)
        bUpdateCVarsTable = true;

      bFillCVarsTable = true;
    }
  }

  if (bUpdateCVarsTable)
    s_pWidget->CVarWidget->RebuildCVarUI(s_pWidget->m_CVars);
  else if (bFillCVarsTable)
    s_pWidget->CVarWidget->UpdateCVarUI(s_pWidget->m_CVars);
}

void nsQtCVarsWidget::ProcessTelemetryConsole(void* pUnuseed)
{
  if (!s_pWidget)
    return;

  nsTelemetryMessage msg;
  nsStringBuilder tmp;

  while (nsTelemetry::RetrieveMessage('CMD', msg) == NS_SUCCESS)
  {
    if (msg.GetMessageID() == 'RES')
    {
      msg.GetReader() >> tmp;
      s_pWidget->CVarWidget->AddConsoleStrings(tmp);
    }
  }
}

void nsQtCVarsWidget::SyncAllCVarsToServer()
{
  for (auto it = m_CVars.GetIterator(); it.IsValid(); ++it)
    SendCVarUpdateToServer(it.Key().GetData(), it.Value());
}

void nsQtCVarsWidget::SendCVarUpdateToServer(nsStringView sName, const nsCVarWidgetData& cvd)
{
  nsTelemetryMessage Msg;
  Msg.SetMessageID('SVAR', ' SET');
  Msg.GetWriter() << sName;
  Msg.GetWriter() << cvd.m_uiType;

  switch (cvd.m_uiType)
  {
    case nsCVarType::Bool:
      Msg.GetWriter() << cvd.m_bValue;
      break;

    case nsCVarType::Float:
      Msg.GetWriter() << cvd.m_fValue;
      break;

    case nsCVarType::Int:
      Msg.GetWriter() << cvd.m_iValue;
      break;

    case nsCVarType::String:
      Msg.GetWriter() << cvd.m_sValue;
      break;
  }

  nsTelemetry::SendToServer(Msg);
}

void nsQtCVarsWidget::BoolChanged(nsStringView sCVar, bool newValue)
{
  auto& cvarData = m_CVars[sCVar];
  cvarData.m_bValue = newValue;
  SendCVarUpdateToServer(sCVar, cvarData);
}

void nsQtCVarsWidget::FloatChanged(nsStringView sCVar, float newValue)
{
  auto& cvarData = m_CVars[sCVar];
  cvarData.m_fValue = newValue;
  SendCVarUpdateToServer(sCVar, cvarData);
}

void nsQtCVarsWidget::IntChanged(nsStringView sCVar, int newValue)
{
  auto& cvarData = m_CVars[sCVar];
  cvarData.m_iValue = newValue;
  SendCVarUpdateToServer(sCVar, cvarData);
}

void nsQtCVarsWidget::StringChanged(nsStringView sCVar, nsStringView sNewValue)
{
  auto& cvarData = m_CVars[sCVar];
  cvarData.m_sValue = sNewValue;
  SendCVarUpdateToServer(sCVar, cvarData);
}

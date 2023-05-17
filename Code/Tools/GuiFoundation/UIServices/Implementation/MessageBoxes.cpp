#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Application/Application.h>
#include <Foundation/Logging/Log.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <ToolsFoundation/Application/ApplicationServices.h>

void wdQtUiServices::MessageBoxStatus(const wdStatus& s, const char* szFailureMsg, const char* szSuccessMsg, bool bOnlySuccessMsgIfDetails)
{
  wdStringBuilder sResult;

  if (s.m_Result.Succeeded())
  {
    if (wdStringUtils::IsNullOrEmpty(szSuccessMsg))
      return;

    if (bOnlySuccessMsgIfDetails && s.m_sMessage.IsEmpty())
      return;

    sResult = szSuccessMsg;

    if (!s.m_sMessage.IsEmpty())
      sResult.AppendFormat("\n\nDetails:\n{0}", s.m_sMessage);

    MessageBoxInformation(sResult);
  }
  else
  {
    sResult = szFailureMsg;

    if (!s.m_sMessage.IsEmpty())
      sResult.AppendFormat("\n\nDetails:\n{0}", s.m_sMessage);

    MessageBoxWarning(sResult);
  }
}

void wdQtUiServices::MessageBoxInformation(const wdFormatString& msg)
{
  wdStringBuilder tmp;

  if (s_bHeadless)
    wdLog::Info(msg.GetText(tmp));
  else
    QMessageBox::information(QApplication::activeWindow(), wdApplication::GetApplicationInstance()->GetApplicationName().GetData(),
      QString::fromUtf8(msg.GetText(tmp)), QMessageBox::StandardButton::Ok);
}

void wdQtUiServices::MessageBoxWarning(const wdFormatString& msg)
{
  wdStringBuilder tmp;

  if (s_bHeadless)
    wdLog::Warning(msg.GetText(tmp));
  else
    QMessageBox::warning(QApplication::activeWindow(), wdApplication::GetApplicationInstance()->GetApplicationName().GetData(),
      QString::fromUtf8(msg.GetText(tmp)), QMessageBox::StandardButton::Ok);
}

QMessageBox::StandardButton wdQtUiServices::MessageBoxQuestion(
  const wdFormatString& msg, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
  if (s_bHeadless)
  {
    return defaultButton;
  }
  else
  {
    wdStringBuilder tmp;
    return QMessageBox::question(QApplication::activeWindow(), wdApplication::GetApplicationInstance()->GetApplicationName().GetData(),
      QString::fromUtf8(msg.GetText(tmp)), buttons, defaultButton);
  }
}

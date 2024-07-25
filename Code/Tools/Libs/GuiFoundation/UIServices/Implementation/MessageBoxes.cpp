#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Application/Application.h>
#include <Foundation/Logging/Log.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <ToolsFoundation/Application/ApplicationServices.h>

void nsQtUiServices::MessageBoxStatus(const nsStatus& s, const char* szFailureMsg, const char* szSuccessMsg, bool bOnlySuccessMsgIfDetails)
{
  nsStringBuilder sResult;

  if (s.m_Result.Succeeded())
  {
    if (nsStringUtils::IsNullOrEmpty(szSuccessMsg))
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

void nsQtUiServices::MessageBoxInformation(const nsFormatString& msg)
{
  nsStringBuilder tmp;

  if (s_bHeadless)
    nsLog::Info(msg.GetText(tmp));
  else
  {
    QMessageBox::information(QApplication::activeWindow(), nsApplication::GetApplicationInstance()->GetApplicationName().GetData(), QString::fromUtf8(msg.GetTextCStr(tmp)), QMessageBox::StandardButton::Ok);
  }
}

void nsQtUiServices::MessageBoxWarning(const nsFormatString& msg)
{
  nsStringBuilder tmp;

  if (s_bHeadless)
    nsLog::Warning(msg.GetText(tmp));
  else
  {
    QMessageBox::warning(QApplication::activeWindow(), nsApplication::GetApplicationInstance()->GetApplicationName().GetData(), QString::fromUtf8(msg.GetTextCStr(tmp)), QMessageBox::StandardButton::Ok);
  }
}

QMessageBox::StandardButton nsQtUiServices::MessageBoxQuestion(
  const nsFormatString& msg, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
  if (s_bHeadless)
  {
    return defaultButton;
  }
  else
  {
    nsStringBuilder tmp;

    return QMessageBox::question(QApplication::activeWindow(), nsApplication::GetApplicationInstance()->GetApplicationName().GetData(), QString::fromUtf8(msg.GetTextCStr(tmp)), buttons, defaultButton);
  }
}

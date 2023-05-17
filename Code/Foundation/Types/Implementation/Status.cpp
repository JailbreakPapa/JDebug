#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Types/Status.h>

void wdResult::AssertSuccess(const char* szMsg /*= nullptr*/, const char* szDetails /*= nullptr*/) const
{
  if (Succeeded())
    return;

  if (szMsg)
  {
    WD_REPORT_FAILURE(szMsg, szDetails);
  }
  else
  {
    WD_REPORT_FAILURE("An operation failed unexpectedly.");
  }
}

wdStatus::wdStatus(const wdFormatString& fmt)
  : m_Result(WD_FAILURE)
{
  wdStringBuilder sMsg;
  m_sMessage = fmt.GetText(sMsg);
}

void wdStatus::LogFailure(wdLogInterface* pLog)
{
  if (Failed())
  {
    wdLogInterface* pInterface = pLog ? pLog : wdLog::GetThreadLocalLogSystem();
    wdLog::Error(pInterface, "{0}", m_sMessage);
  }
}


WD_STATICLINK_FILE(Foundation, Foundation_Types_Implementation_Status);

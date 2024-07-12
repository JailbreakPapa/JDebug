#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Types/Status.h>

void nsResult::AssertSuccess(const char* szMsg /*= nullptr*/, const char* szDetails /*= nullptr*/) const
{
  if (Succeeded())
    return;

  if (szMsg)
  {
    NS_REPORT_FAILURE(szMsg, szDetails);
  }
  else
  {
    NS_REPORT_FAILURE("An operation failed unexpectedly.");
  }
}

nsStatus::nsStatus(const nsFormatString& fmt)
  : m_Result(NS_FAILURE)
{
  nsStringBuilder sMsg;
  m_sMessage = fmt.GetText(sMsg);
}

bool nsStatus::LogFailure(nsLogInterface* pLog)
{
  if (Failed())
  {
    nsLogInterface* pInterface = pLog ? pLog : nsLog::GetThreadLocalLogSystem();
    nsLog::Error(pInterface, "{0}", m_sMessage);
  }

  return Failed();
}

void nsStatus::AssertSuccess(const char* szMsg /*= nullptr*/) const
{
  if (Succeeded())
    return;

  if (szMsg)
  {
    NS_REPORT_FAILURE(szMsg, m_sMessage.GetData());
  }
  else
  {
    NS_REPORT_FAILURE("An operation failed unexpectedly.", m_sMessage.GetData());
  }
}

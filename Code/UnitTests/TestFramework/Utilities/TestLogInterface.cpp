#include <TestFramework/TestFrameworkPCH.h>

#include <TestFramework/Utilities/TestLogInterface.h>

#include <TestFramework/Framework/TestFramework.h>

nsTestLogInterface::~nsTestLogInterface()
{
  for (const ExpectedMsg& msg : m_ExpectedMessages)
  {
    nsInt32 count = msg.m_iCount;
    NS_TEST_BOOL_MSG(count == 0, "Message \"%s\" was logged %d times %s than expected.", msg.m_sMsgSubString.GetData(), count < 0 ? -count : count,
      count < 0 ? "more" : "less");
  }
}

void nsTestLogInterface::HandleLogMessage(const nsLoggingEventData& le)
{
  {
    // in case this interface is used with nsTestLogSystemScope to override the nsGlobalLog (see nsGlobalLog::SetGlobalLogOverride)
    // it must be thread-safe
    NS_LOCK(m_Mutex);

    for (ExpectedMsg& msg : m_ExpectedMessages)
    {
      if (msg.m_Type != nsLogMsgType::All && le.m_EventType != msg.m_Type)
        continue;

      if (le.m_sText.FindSubString(msg.m_sMsgSubString))
      {
        --msg.m_iCount;

        // filter out error and warning messages entirely
        if (le.m_EventType >= nsLogMsgType::ErrorMsg && le.m_EventType <= nsLogMsgType::WarningMsg)
          return;

        // pass all other messages along to the parent log
        break;
      }
    }
  }

  if (m_pParentLog)
  {
    m_pParentLog->HandleLogMessage(le);
  }
}

void nsTestLogInterface::ExpectMessage(const char* szMsg, nsLogMsgType::Enum type /*= nsLogMsgType::All*/, nsInt32 iCount /*= 1*/)
{
  NS_LOCK(m_Mutex);

  // Do not allow initial count to be less than 1, but use signed int to keep track
  // of error messages that were encountered more often than expected.
  NS_ASSERT_DEV(iCount >= 1, "Message needs to be expected at least once");

  ExpectedMsg& em = m_ExpectedMessages.ExpandAndGetRef();
  em.m_sMsgSubString = szMsg;
  em.m_iCount = iCount;
  em.m_Type = type;
}

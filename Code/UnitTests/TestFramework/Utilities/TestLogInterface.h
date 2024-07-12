#pragma once
#include <Foundation/Logging/Log.h>
#include <TestFramework/TestFrameworkDLL.h>

/// \brief An nsLogInterface that expects and handles error messages during test runs. Can be
/// used to ensure that expected error messages are produced by the tested functionality.
/// Expected error messages are not passed on and do not cause tests to fail.
class NS_TEST_DLL nsTestLogInterface : public nsLogInterface
{
public:
  nsTestLogInterface() = default;
  ~nsTestLogInterface();
  virtual void HandleLogMessage(const nsLoggingEventData& le) override;

  /// \brief Add expected message. Will fail the test when the expected message is not
  /// encountered. Can take an optional count, if messages are expected multiple times
  void ExpectMessage(const char* szMsg, nsLogMsgType::Enum type = nsLogMsgType::All, nsInt32 iCount = 1);

  /// \brief Set the log interface that unhandled messages are forwarded to.
  void SetParentLog(nsLogInterface* pInterface) { m_pParentLog = pInterface; }

private:
  nsLogInterface* m_pParentLog = nullptr;

  struct ExpectedMsg
  {
    nsInt32 m_iCount = 0;
    nsString m_sMsgSubString;
    nsLogMsgType::Enum m_Type = nsLogMsgType::All;
  };

  mutable nsMutex m_Mutex;
  nsHybridArray<ExpectedMsg, 8> m_ExpectedMessages;
};

/// \brief A class that sets a custom nsTestLogInterface as the thread local default log system,
/// and resets the previous system when it goes out of scope. The test version passes the previous
/// nsLogInterface on to the nsTestLogInterface to enable passing on unhandled messages.
///
/// If bCatchMessagesGlobally is false, the system only intercepts messages on the current thread.
/// If bCatchMessagesGlobally is true, it will also intercept messages from other threads, as long as they
/// go through nsGlobalLog. See nsGlobalLog::SetGlobalLogOverride().
class NS_TEST_DLL nsTestLogSystemScope : public nsLogSystemScope
{
public:
  explicit nsTestLogSystemScope(nsTestLogInterface* pInterface, bool bCatchMessagesGlobally = false)
    : nsLogSystemScope(pInterface)
  {
    m_bCatchMessagesGlobally = bCatchMessagesGlobally;
    pInterface->SetParentLog(m_pPrevious);

    if (m_bCatchMessagesGlobally)
    {
      nsGlobalLog::SetGlobalLogOverride(pInterface);
    }
  }

  ~nsTestLogSystemScope()
  {
    if (m_bCatchMessagesGlobally)
    {
      nsGlobalLog::SetGlobalLogOverride(nullptr);
    }
  }

private:
  bool m_bCatchMessagesGlobally = false;
};

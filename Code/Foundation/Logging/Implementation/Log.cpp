#include <Foundation/FoundationPCH.h>

#include <Foundation/Application/Application.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Strings/StringConversion.h>
#include <Foundation/Time/Time.h>
#include <Foundation/Time/Timestamp.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS) || NS_ENABLED(NS_PLATFORM_LINUX)
#  include <Foundation/Logging/ETWWriter.h>
#endif
#if NS_ENABLED(NS_PLATFORM_ANDROID)
#  include <android/log.h>
#endif

#include <stdarg.h>

// Comment in to log into nsLog::Print any message that is output while no logger is registered.
// #define DEBUG_STARTUP_LOGGING

nsLogMsgType::Enum nsLog::s_DefaultLogLevel = nsLogMsgType::All;
nsLog::PrintFunction nsLog::s_CustomPrintFunction = nullptr;
nsAtomicInteger32 nsGlobalLog::s_uiMessageCount[nsLogMsgType::ENUM_COUNT];
nsLoggingEvent nsGlobalLog::s_LoggingEvent;
nsLogInterface* nsGlobalLog::s_pOverrideLog = nullptr;
static thread_local bool s_bAllowOverrideLog = true;
static nsMutex s_OverrideLogMutex;

/// \brief The log system that messages are sent to when the user specifies no system himself.
static thread_local nsLogInterface* s_DefaultLogSystem = nullptr;


nsEventSubscriptionID nsGlobalLog::AddLogWriter(nsLoggingEvent::Handler handler)
{
  if (s_LoggingEvent.HasEventHandler(handler))
    return 0;

  return s_LoggingEvent.AddEventHandler(handler);
}

void nsGlobalLog::RemoveLogWriter(nsLoggingEvent::Handler handler)
{
  if (!s_LoggingEvent.HasEventHandler(handler))
    return;

  s_LoggingEvent.RemoveEventHandler(handler);
}

void nsGlobalLog::RemoveLogWriter(nsEventSubscriptionID& ref_subscriptionID)
{
  s_LoggingEvent.RemoveEventHandler(ref_subscriptionID);
}

void nsGlobalLog::SetGlobalLogOverride(nsLogInterface* pInterface)
{
  NS_LOCK(s_OverrideLogMutex);

  NS_ASSERT_DEV(pInterface == nullptr || s_pOverrideLog == nullptr, "Only one override log can be set at a time");
  s_pOverrideLog = pInterface;
}

void nsGlobalLog::HandleLogMessage(const nsLoggingEventData& le)
{
  if (s_pOverrideLog != nullptr && s_pOverrideLog != this && s_bAllowOverrideLog)
  {
    // only enter the lock when really necessary
    NS_LOCK(s_OverrideLogMutex);

    // since s_bAllowOverrideLog is thread_local we do not need to re-check it

    // check this again under the lock, to be safe
    if (s_pOverrideLog != nullptr && s_pOverrideLog != this)
    {
      // disable the override log for the period in which it handles the event
      // to prevent infinite recursions
      s_bAllowOverrideLog = false;
      s_pOverrideLog->HandleLogMessage(le);
      s_bAllowOverrideLog = true;

      return;
    }
  }

  // else
  {
    const nsLogMsgType::Enum ThisType = le.m_EventType;

    if ((ThisType > nsLogMsgType::None) && (ThisType < nsLogMsgType::All))
      s_uiMessageCount[ThisType].Increment();

#ifdef DEBUG_STARTUP_LOGGING
    if (s_LoggingEvent.IsEmpty())
    {
      nsStringBuilder stmp = le.m_sText;
      stmp.Append("\n");
      nsLog::Print(stmp);
    }
#endif
    s_LoggingEvent.Broadcast(le);
  }
}

nsLogBlock::nsLogBlock(nsStringView sName, nsStringView sContextInfo)
{
  m_pLogInterface = nsLog::GetThreadLocalLogSystem();

  if (!m_pLogInterface)
    return;

  m_sName = sName;
  m_sContextInfo = sContextInfo;
  m_bWritten = false;

  m_pParentBlock = m_pLogInterface->m_pCurrentBlock;
  m_pLogInterface->m_pCurrentBlock = this;

  m_uiBlockDepth = m_pParentBlock ? (m_pParentBlock->m_uiBlockDepth + 1) : 0;

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  m_fSeconds = nsTime::Now().GetSeconds();
#endif
}


nsLogBlock::nsLogBlock(nsLogInterface* pInterface, nsStringView sName, nsStringView sContextInfo)
{
  m_pLogInterface = pInterface;

  if (!m_pLogInterface)
    return;

  m_sName = sName;
  m_sContextInfo = sContextInfo;
  m_bWritten = false;

  m_pParentBlock = m_pLogInterface->m_pCurrentBlock;
  m_pLogInterface->m_pCurrentBlock = this;

  m_uiBlockDepth = m_pParentBlock ? (m_pParentBlock->m_uiBlockDepth + 1) : 0;

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  m_fSeconds = nsTime::Now().GetSeconds();
#endif
}

nsLogBlock::~nsLogBlock()
{
  if (!m_pLogInterface)
    return;

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  m_fSeconds = nsTime::Now().GetSeconds() - m_fSeconds;
#endif

  m_pLogInterface->m_pCurrentBlock = m_pParentBlock;

  nsLog::EndLogBlock(m_pLogInterface, this);
}


void nsLog::EndLogBlock(nsLogInterface* pInterface, nsLogBlock* pBlock)
{
  if (pBlock->m_bWritten)
  {
    nsLoggingEventData le;
    le.m_EventType = nsLogMsgType::EndGroup;
    le.m_sText = pBlock->m_sName;
    le.m_uiIndentation = pBlock->m_uiBlockDepth;
    le.m_sTag = pBlock->m_sContextInfo;
#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
    le.m_fSeconds = pBlock->m_fSeconds;
#endif

    pInterface->HandleLogMessage(le);
  }
}

void nsLog::WriteBlockHeader(nsLogInterface* pInterface, nsLogBlock* pBlock)
{
  if (!pBlock || pBlock->m_bWritten)
    return;

  pBlock->m_bWritten = true;

  WriteBlockHeader(pInterface, pBlock->m_pParentBlock);

  nsLoggingEventData le;
  le.m_EventType = nsLogMsgType::BeginGroup;
  le.m_sText = pBlock->m_sName;
  le.m_uiIndentation = pBlock->m_uiBlockDepth;
  le.m_sTag = pBlock->m_sContextInfo;

  pInterface->HandleLogMessage(le);
}

void nsLog::BroadcastLoggingEvent(nsLogInterface* pInterface, nsLogMsgType::Enum type, nsStringView sString)
{
  nsLogBlock* pTopBlock = pInterface->m_pCurrentBlock;
  nsUInt8 uiIndentation = 0;

  if (pTopBlock)
  {
    uiIndentation = pTopBlock->m_uiBlockDepth + 1;

    WriteBlockHeader(pInterface, pTopBlock);
  }

  char szTag[32] = "";

  if (sString.StartsWith("["))
  {
    const char* szAfterTag = sString.GetStartPointer();

    ++szAfterTag;

    nsInt32 iPos = 0;

    // only treat it as a tag, if it is properly enclosed in square brackets and doesn't contain spaces
    while ((*szAfterTag != '\0') && (*szAfterTag != '[') && (*szAfterTag != ']') && (*szAfterTag != ' ') && (iPos < 31))
    {
      szTag[iPos] = *szAfterTag;
      ++szAfterTag;
      ++iPos;
    }

    if (*szAfterTag == ']')
    {
      szTag[iPos] = '\0';
      sString.SetStartPosition(szAfterTag + 1);
    }
    else
    {
      szTag[0] = '\0';
    }
  }

  nsLoggingEventData le;
  le.m_EventType = type;
  le.m_sText = sString;
  le.m_uiIndentation = uiIndentation;
  le.m_sTag = szTag;

  pInterface->HandleLogMessage(le);
  pInterface->m_uiLoggedMsgsSinceFlush++;
}

void nsLog::Print(const char* szText)
{
  printf("%s", szText);

#if NS_ENABLED(NS_PLATFORM_WINDOWS) || NS_ENABLED(NS_PLATFORM_LINUX)
  nsLogWriter::ETW::LogMessage(nsLogMsgType::ErrorMsg, 0, szText);
#endif
#if NS_ENABLED(NS_PLATFORM_WINDOWS)
  OutputDebugStringW(nsStringWChar(szText).GetData());
#endif
#if NS_ENABLED(NS_PLATFORM_ANDROID)
  __android_log_print(ANDROID_LOG_ERROR, "nsEngine", "%s", szText);
#endif

  if (s_CustomPrintFunction)
  {
    s_CustomPrintFunction(szText);
  }

  fflush(stdout);
  fflush(stderr);
}

void nsLog::Printf(const char* szFormat, ...)
{
  va_list args;
  va_start(args, szFormat);

  char buffer[4096];
  nsStringUtils::vsnprintf(buffer, NS_ARRAY_SIZE(buffer), szFormat, args);

  Print(buffer);

  va_end(args);
}

void nsLog::SetCustomPrintFunction(PrintFunction func)
{
  s_CustomPrintFunction = func;
}

void nsLog::OsMessageBox(const nsFormatString& text)
{
  nsStringBuilder tmp;
  nsStringBuilder display = text.GetText(tmp);
  display.Trim(" \n\r\t");

#if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)
  const char* title = "";
  if (nsApplication::GetApplicationInstance())
  {
    title = nsApplication::GetApplicationInstance()->GetApplicationName();
  }

  MessageBoxW(nullptr, nsStringWChar(display).GetData(), nsStringWChar(title), MB_OK);
#else
  nsLog::Print(display);
  NS_ASSERT_NOT_IMPLEMENTED;
#endif
}

void nsLog::GenerateFormattedTimestamp(TimestampMode mode, nsStringBuilder& ref_sTimestampOut)
{
  // if mode is 'None', early out to not even retrieve a timestamp
  if (mode == TimestampMode::None)
  {
    return;
  }

  const nsDateTime dateTime = nsDateTime::MakeFromTimestamp(nsTimestamp::CurrentTimestamp());

  switch (mode)
  {
    case TimestampMode::Numeric:
      ref_sTimestampOut.SetFormat("[{}] ", nsArgDateTime(dateTime, nsArgDateTime::ShowDate | nsArgDateTime::ShowMilliseconds | nsArgDateTime::ShowTimeZone));
      break;
    case TimestampMode::TimeOnly:
      ref_sTimestampOut.SetFormat("[{}] ", nsArgDateTime(dateTime, nsArgDateTime::ShowMilliseconds));
      break;
    case TimestampMode::Textual:
      ref_sTimestampOut.SetFormat(
        "[{}] ", nsArgDateTime(dateTime, nsArgDateTime::TextualDate | nsArgDateTime::ShowMilliseconds | nsArgDateTime::ShowTimeZone));
      break;
    default:
      NS_ASSERT_DEV(false, "Unknown timestamp mode.");
      break;
  }
}

void nsLog::SetThreadLocalLogSystem(nsLogInterface* pInterface)
{
  NS_ASSERT_DEV(pInterface != nullptr,
    "You cannot set a nullptr logging system. If you want to discard all log information, set a dummy system that does not do anything.");

  s_DefaultLogSystem = pInterface;
}

nsLogInterface* nsLog::GetThreadLocalLogSystem()
{
  if (s_DefaultLogSystem == nullptr)
  {
    // use new, not NS_DEFAULT_NEW, to prevent tracking
    s_DefaultLogSystem = new nsGlobalLog;
  }

  return s_DefaultLogSystem;
}

void nsLog::SetDefaultLogLevel(nsLogMsgType::Enum logLevel)
{
  NS_ASSERT_DEV(logLevel >= nsLogMsgType::None && logLevel <= nsLogMsgType::All, "Invalid default log level {}", (int)logLevel);

  s_DefaultLogLevel = logLevel;
}

nsLogMsgType::Enum nsLog::GetDefaultLogLevel()
{
  return s_DefaultLogLevel;
}

#define LOG_LEVEL_FILTER(MaxLevel)                                                                                                  \
  if (pInterface == nullptr)                                                                                                        \
    return;                                                                                                                         \
  if ((pInterface->GetLogLevel() == nsLogMsgType::GlobalDefault ? nsLog::s_DefaultLogLevel : pInterface->GetLogLevel()) < MaxLevel) \
    return;


void nsLog::Error(nsLogInterface* pInterface, const nsFormatString& string)
{
  LOG_LEVEL_FILTER(nsLogMsgType::ErrorMsg);

  nsStringBuilder tmp;
  BroadcastLoggingEvent(pInterface, nsLogMsgType::ErrorMsg, string.GetText(tmp));
}

void nsLog::SeriousWarning(nsLogInterface* pInterface, const nsFormatString& string)
{
  LOG_LEVEL_FILTER(nsLogMsgType::SeriousWarningMsg);

  nsStringBuilder tmp;
  BroadcastLoggingEvent(pInterface, nsLogMsgType::SeriousWarningMsg, string.GetText(tmp));
}

void nsLog::Warning(nsLogInterface* pInterface, const nsFormatString& string)
{
  LOG_LEVEL_FILTER(nsLogMsgType::WarningMsg);

  nsStringBuilder tmp;
  BroadcastLoggingEvent(pInterface, nsLogMsgType::WarningMsg, string.GetText(tmp));
}

void nsLog::Success(nsLogInterface* pInterface, const nsFormatString& string)
{
  LOG_LEVEL_FILTER(nsLogMsgType::SuccessMsg);

  nsStringBuilder tmp;
  BroadcastLoggingEvent(pInterface, nsLogMsgType::SuccessMsg, string.GetText(tmp));
}

void nsLog::Info(nsLogInterface* pInterface, const nsFormatString& string)
{
  LOG_LEVEL_FILTER(nsLogMsgType::InfoMsg);

  nsStringBuilder tmp;
  BroadcastLoggingEvent(pInterface, nsLogMsgType::InfoMsg, string.GetText(tmp));
}

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)

void nsLog::Dev(nsLogInterface* pInterface, const nsFormatString& string)
{
  LOG_LEVEL_FILTER(nsLogMsgType::DevMsg);

  nsStringBuilder tmp;
  BroadcastLoggingEvent(pInterface, nsLogMsgType::DevMsg, string.GetText(tmp));
}

#endif

#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)

void nsLog::Debug(nsLogInterface* pInterface, const nsFormatString& string)
{
  LOG_LEVEL_FILTER(nsLogMsgType::DebugMsg);

  nsStringBuilder tmp;
  BroadcastLoggingEvent(pInterface, nsLogMsgType::DebugMsg, string.GetText(tmp));
}

#endif

bool nsLog::Flush(nsUInt32 uiNumNewMsgThreshold, nsTime timeIntervalThreshold, nsLogInterface* pInterface /*= GetThreadLocalLogSystem()*/)
{
  if (pInterface == nullptr || pInterface->m_uiLoggedMsgsSinceFlush == 0) // if really nothing was logged, don't execute a flush
    return false;

  if (pInterface->m_uiLoggedMsgsSinceFlush <= uiNumNewMsgThreshold && nsTime::Now() - pInterface->m_LastFlushTime < timeIntervalThreshold)
    return false;

  BroadcastLoggingEvent(pInterface, nsLogMsgType::Flush, nullptr);

  pInterface->m_uiLoggedMsgsSinceFlush = 0;
  pInterface->m_LastFlushTime = nsTime::Now();

  return true;
}

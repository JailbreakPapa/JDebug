#include <Foundation/FoundationPCH.h>

#include <Foundation/Application/Application.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Strings/StringConversion.h>
#include <Foundation/Time/Time.h>
#include <Foundation/Time/Timestamp.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/Logging/Implementation/Win/ETWProvider_win.h>
#endif
#if WD_ENABLED(WD_PLATFORM_ANDROID)
#  include <android/log.h>
#endif

wdLogMsgType::Enum wdLog::s_DefaultLogLevel = wdLogMsgType::All;
wdLog::PrintFunction wdLog::s_CustomPrintFunction = nullptr;
wdAtomicInteger32 wdGlobalLog::s_uiMessageCount[wdLogMsgType::ENUM_COUNT];
wdLoggingEvent wdGlobalLog::s_LoggingEvent;
wdLogInterface* wdGlobalLog::s_pOverrideLog = nullptr;
static thread_local bool s_bAllowOverrideLog = true;
static wdMutex s_OverrideLogMutex;

/// \brief The log system that messages are sent to when the user specifies no system himself.
static thread_local wdLogInterface* s_DefaultLogSystem = nullptr;


wdEventSubscriptionID wdGlobalLog::AddLogWriter(wdLoggingEvent::Handler handler)
{
  if (s_LoggingEvent.HasEventHandler(handler))
    return 0;

  return s_LoggingEvent.AddEventHandler(handler);
}

void wdGlobalLog::RemoveLogWriter(wdLoggingEvent::Handler handler)
{
  if (!s_LoggingEvent.HasEventHandler(handler))
    return;

  s_LoggingEvent.RemoveEventHandler(handler);
}

void wdGlobalLog::RemoveLogWriter(wdEventSubscriptionID& ref_subscriptionID)
{
  s_LoggingEvent.RemoveEventHandler(ref_subscriptionID);
}

void wdGlobalLog::SetGlobalLogOverride(wdLogInterface* pInterface)
{
  WD_LOCK(s_OverrideLogMutex);

  WD_ASSERT_DEV(pInterface == nullptr || s_pOverrideLog == nullptr, "Only one override log can be set at a time");
  s_pOverrideLog = pInterface;
}

void wdGlobalLog::HandleLogMessage(const wdLoggingEventData& le)
{
  if (s_pOverrideLog != nullptr && s_pOverrideLog != this && s_bAllowOverrideLog)
  {
    // only enter the lock when really necessary
    WD_LOCK(s_OverrideLogMutex);

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
    const wdLogMsgType::Enum ThisType = le.m_EventType;

    if ((ThisType > wdLogMsgType::None) && (ThisType < wdLogMsgType::All))
      s_uiMessageCount[ThisType].Increment();

    s_LoggingEvent.Broadcast(le);
  }
}

wdLogBlock::wdLogBlock(wdStringView sName, wdStringView sContextInfo)
{
  m_pLogInterface = wdLog::GetThreadLocalLogSystem();

  if (!m_pLogInterface)
    return;

  m_sName = sName;
  m_sContextInfo = sContextInfo;
  m_bWritten = false;

  m_pParentBlock = m_pLogInterface->m_pCurrentBlock;
  m_pLogInterface->m_pCurrentBlock = this;

  m_uiBlockDepth = m_pParentBlock ? (m_pParentBlock->m_uiBlockDepth + 1) : 0;

#if WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT)
  m_fSeconds = wdTime::Now().GetSeconds();
#endif
}


wdLogBlock::wdLogBlock(wdLogInterface* pInterface, wdStringView sName, wdStringView sContextInfo)
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

#if WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT)
  m_fSeconds = wdTime::Now().GetSeconds();
#endif
}

wdLogBlock::~wdLogBlock()
{
  if (!m_pLogInterface)
    return;

#if WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT)
  m_fSeconds = wdTime::Now().GetSeconds() - m_fSeconds;
#endif

  m_pLogInterface->m_pCurrentBlock = m_pParentBlock;

  wdLog::EndLogBlock(m_pLogInterface, this);
}


void wdLog::EndLogBlock(wdLogInterface* pInterface, wdLogBlock* pBlock)
{
  if (pBlock->m_bWritten)
  {
    wdLoggingEventData le;
    le.m_EventType = wdLogMsgType::EndGroup;
    le.m_sText = pBlock->m_sName;
    le.m_uiIndentation = pBlock->m_uiBlockDepth;
    le.m_sTag = pBlock->m_sContextInfo;
#if WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT)
    le.m_fSeconds = pBlock->m_fSeconds;
#endif

    pInterface->HandleLogMessage(le);
  }
}

void wdLog::WriteBlockHeader(wdLogInterface* pInterface, wdLogBlock* pBlock)
{
  if (!pBlock || pBlock->m_bWritten)
    return;

  pBlock->m_bWritten = true;

  WriteBlockHeader(pInterface, pBlock->m_pParentBlock);

  wdLoggingEventData le;
  le.m_EventType = wdLogMsgType::BeginGroup;
  le.m_sText = pBlock->m_sName;
  le.m_uiIndentation = pBlock->m_uiBlockDepth;
  le.m_sTag = pBlock->m_sContextInfo;

  pInterface->HandleLogMessage(le);
}

void wdLog::BroadcastLoggingEvent(wdLogInterface* pInterface, wdLogMsgType::Enum type, const char* szString)
{
  wdLogBlock* pTopBlock = pInterface->m_pCurrentBlock;
  wdUInt8 uiIndentation = 0;

  if (pTopBlock)
  {
    uiIndentation = pTopBlock->m_uiBlockDepth + 1;

    WriteBlockHeader(pInterface, pTopBlock);
  }

  char szTag[32] = "";

  if (wdStringUtils::StartsWith(szString, "["))
  {
    const char* szAfterTag = szString;

    ++szAfterTag;

    wdInt32 iPos = 0;

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
      szString = szAfterTag + 1;
    }
    else
    {
      szTag[0] = '\0';
    }
  }

  wdLoggingEventData le;
  le.m_EventType = type;
  le.m_sText = szString;
  le.m_uiIndentation = uiIndentation;
  le.m_sTag = szTag;

  pInterface->HandleLogMessage(le);
  pInterface->m_uiLoggedMsgsSinceFlush++;
}

void wdLog::Print(const char* szText)
{
  printf("%s", szText);

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
  wdETWProvider::GetInstance().LogMessge(wdLogMsgType::ErrorMsg, 0, szText);
#endif
#if WD_ENABLED(WD_PLATFORM_WINDOWS)
  OutputDebugStringW(wdStringWChar(szText).GetData());
#endif
#if WD_ENABLED(WD_PLATFORM_ANDROID)
  __android_log_print(ANDROID_LOG_ERROR, "wdEngine", "%s", szText);
#endif

  if (s_CustomPrintFunction)
  {
    s_CustomPrintFunction(szText);
  }

  fflush(stdout);
  fflush(stderr);
}

void wdLog::Printf(const char* szFormat, ...)
{
  va_list args;
  va_start(args, szFormat);

  char buffer[4096];
  wdStringUtils::vsnprintf(buffer, WD_ARRAY_SIZE(buffer), szFormat, args);

  Print(buffer);

  va_end(args);
}

void wdLog::SetCustomPrintFunction(PrintFunction func)
{
  s_CustomPrintFunction = func;
}

void wdLog::OsMessageBox(const wdFormatString& text)
{
  wdStringBuilder tmp;
  wdStringBuilder display = text.GetText(tmp);
  display.Trim(" \n\r\t");

  const char* title = "";
  if (wdApplication::GetApplicationInstance())
  {
    title = wdApplication::GetApplicationInstance()->GetApplicationName();
  }

#if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)
  MessageBoxW(nullptr, wdStringWChar(display).GetData(), wdStringWChar(title), MB_OK);
#else
  wdLog::Print(display);
  WD_ASSERT_NOT_IMPLEMENTED;
#endif
}

void wdLog::GenerateFormattedTimestamp(TimestampMode mode, wdStringBuilder& ref_sTimestampOut)
{
  // if mode is 'None', early out to not even retrieve a timestamp
  if (mode == TimestampMode::None)
  {
    return;
  }

  const wdDateTime dateTime(wdTimestamp::CurrentTimestamp());

  switch (mode)
  {
    case TimestampMode::Numeric:
      ref_sTimestampOut.Format("[{}] ", wdArgDateTime(dateTime, wdArgDateTime::ShowDate | wdArgDateTime::ShowMilliseconds | wdArgDateTime::ShowTimeZone));
      break;
    case TimestampMode::TimeOnly:
      ref_sTimestampOut.Format("[{}] ", wdArgDateTime(dateTime, wdArgDateTime::ShowMilliseconds));
      break;
    case TimestampMode::Textual:
      ref_sTimestampOut.Format(
        "[{}] ", wdArgDateTime(dateTime, wdArgDateTime::TextualDate | wdArgDateTime::ShowMilliseconds | wdArgDateTime::ShowTimeZone));
      break;
    default:
      WD_ASSERT_DEV(false, "Unknown timestamp mode.");
      break;
  }
}

void wdLog::SetThreadLocalLogSystem(wdLogInterface* pInterface)
{
  WD_ASSERT_DEV(pInterface != nullptr,
    "You cannot set a nullptr logging system. If you want to discard all log information, set a dummy system that does not do anything.");

  s_DefaultLogSystem = pInterface;
}

wdLogInterface* wdLog::GetThreadLocalLogSystem()
{
  if (s_DefaultLogSystem == nullptr)
  {
    // use new, not WD_DEFAULT_NEW, to prevent tracking
    s_DefaultLogSystem = new wdGlobalLog;
  }

  return s_DefaultLogSystem;
}

void wdLog::SetDefaultLogLevel(wdLogMsgType::Enum logLevel)
{
  WD_ASSERT_DEV(logLevel >= wdLogMsgType::None && logLevel <= wdLogMsgType::All, "Invalid default log level {}", (int)logLevel);

  s_DefaultLogLevel = logLevel;
}

wdLogMsgType::Enum wdLog::GetDefaultLogLevel()
{
  return s_DefaultLogLevel;
}

#define LOG_LEVEL_FILTER(MaxLevel)                                                                                                  \
  if (pInterface == nullptr)                                                                                                        \
    return;                                                                                                                         \
  if ((pInterface->GetLogLevel() == wdLogMsgType::GlobalDefault ? wdLog::s_DefaultLogLevel : pInterface->GetLogLevel()) < MaxLevel) \
    return;


void wdLog::Error(wdLogInterface* pInterface, const wdFormatString& string)
{
  LOG_LEVEL_FILTER(wdLogMsgType::ErrorMsg);

  wdStringBuilder tmp;
  BroadcastLoggingEvent(pInterface, wdLogMsgType::ErrorMsg, string.GetText(tmp));
}

void wdLog::SeriousWarning(wdLogInterface* pInterface, const wdFormatString& string)
{
  LOG_LEVEL_FILTER(wdLogMsgType::SeriousWarningMsg);

  wdStringBuilder tmp;
  BroadcastLoggingEvent(pInterface, wdLogMsgType::SeriousWarningMsg, string.GetText(tmp));
}

void wdLog::Warning(wdLogInterface* pInterface, const wdFormatString& string)
{
  LOG_LEVEL_FILTER(wdLogMsgType::WarningMsg);

  wdStringBuilder tmp;
  BroadcastLoggingEvent(pInterface, wdLogMsgType::WarningMsg, string.GetText(tmp));
}

void wdLog::Success(wdLogInterface* pInterface, const wdFormatString& string)
{
  LOG_LEVEL_FILTER(wdLogMsgType::SuccessMsg);

  wdStringBuilder tmp;
  BroadcastLoggingEvent(pInterface, wdLogMsgType::SuccessMsg, string.GetText(tmp));
}

void wdLog::Info(wdLogInterface* pInterface, const wdFormatString& string)
{
  LOG_LEVEL_FILTER(wdLogMsgType::InfoMsg);

  wdStringBuilder tmp;
  BroadcastLoggingEvent(pInterface, wdLogMsgType::InfoMsg, string.GetText(tmp));
}

#if WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT)

void wdLog::Dev(wdLogInterface* pInterface, const wdFormatString& string)
{
  LOG_LEVEL_FILTER(wdLogMsgType::DevMsg);

  wdStringBuilder tmp;
  BroadcastLoggingEvent(pInterface, wdLogMsgType::DevMsg, string.GetText(tmp));
}

#endif

#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)

void wdLog::Debug(wdLogInterface* pInterface, const wdFormatString& string)
{
  LOG_LEVEL_FILTER(wdLogMsgType::DebugMsg);

  wdStringBuilder tmp;
  BroadcastLoggingEvent(pInterface, wdLogMsgType::DebugMsg, string.GetText(tmp));
}

#endif

bool wdLog::Flush(wdUInt32 uiNumNewMsgThreshold, wdTime timeIntervalThreshold, wdLogInterface* pInterface /*= GetThreadLocalLogSystem()*/)
{
  if (pInterface == nullptr || pInterface->m_uiLoggedMsgsSinceFlush == 0) // if really nothing was logged, don't execute a flush
    return false;

  if (pInterface->m_uiLoggedMsgsSinceFlush <= uiNumNewMsgThreshold && wdTime::Now() - pInterface->m_LastFlushTime < timeIntervalThreshold)
    return false;

  BroadcastLoggingEvent(pInterface, wdLogMsgType::Flush, nullptr);

  pInterface->m_uiLoggedMsgsSinceFlush = 0;
  pInterface->m_LastFlushTime = wdTime::Now();

  return true;
}

WD_STATICLINK_FILE(Foundation, Foundation_Logging_Implementation_Log);

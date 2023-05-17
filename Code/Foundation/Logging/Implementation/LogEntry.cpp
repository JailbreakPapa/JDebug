#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/LogEntry.h>

// clang-format off
WD_BEGIN_STATIC_REFLECTED_ENUM(wdLogMsgType, 1)
  WD_BITFLAGS_CONSTANTS(wdLogMsgType::Flush, wdLogMsgType::BeginGroup, wdLogMsgType::EndGroup, wdLogMsgType::None)
  WD_BITFLAGS_CONSTANTS(wdLogMsgType::ErrorMsg, wdLogMsgType::SeriousWarningMsg, wdLogMsgType::WarningMsg, wdLogMsgType::SuccessMsg, wdLogMsgType::InfoMsg, wdLogMsgType::DevMsg, wdLogMsgType::DebugMsg, wdLogMsgType::All)
WD_END_STATIC_REFLECTED_ENUM;

WD_BEGIN_STATIC_REFLECTED_TYPE(wdLogEntry, wdNoBase, 1, wdRTTIDefaultAllocator<wdLogEntry>)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Msg", m_sMsg),
    WD_MEMBER_PROPERTY("Tag", m_sTag),
    WD_ENUM_MEMBER_PROPERTY("Type", wdLogMsgType, m_Type),
    WD_MEMBER_PROPERTY("Indentation", m_uiIndentation),
    WD_MEMBER_PROPERTY("Time", m_fSeconds),
  }
  WD_END_PROPERTIES;
}
WD_END_STATIC_REFLECTED_TYPE;
// clang-format on

wdLogEntry::wdLogEntry() {}

wdLogEntry::wdLogEntry(const wdLoggingEventData& le)
{
  m_sMsg = le.m_sText;
  m_sTag = le.m_sTag;
  m_Type = le.m_EventType;
  m_uiIndentation = le.m_uiIndentation;
#if WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT)
  m_fSeconds = le.m_fSeconds;
#else
  m_fSeconds = 0.0f;
#endif
}

wdLogEntryDelegate::wdLogEntryDelegate(Callback callback, wdLogMsgType::Enum logLevel)
  : m_Callback(callback)
{
  SetLogLevel(logLevel);
}

void wdLogEntryDelegate::HandleLogMessage(const wdLoggingEventData& le)
{
  wdLogEntry e(le);
  m_Callback(e);
}

WD_STATICLINK_FILE(Foundation, Foundation_Logging_Implementation_LogEntry);

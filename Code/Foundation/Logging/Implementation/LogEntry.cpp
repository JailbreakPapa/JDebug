#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/LogEntry.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_ENUM(nsLogMsgType, 1)
  NS_BITFLAGS_CONSTANTS(nsLogMsgType::Flush, nsLogMsgType::BeginGroup, nsLogMsgType::EndGroup, nsLogMsgType::None)
  NS_BITFLAGS_CONSTANTS(nsLogMsgType::ErrorMsg, nsLogMsgType::SeriousWarningMsg, nsLogMsgType::WarningMsg, nsLogMsgType::SuccessMsg, nsLogMsgType::InfoMsg, nsLogMsgType::DevMsg, nsLogMsgType::DebugMsg, nsLogMsgType::All)
NS_END_STATIC_REFLECTED_ENUM;

NS_BEGIN_STATIC_REFLECTED_TYPE(nsLogEntry, nsNoBase, 1, nsRTTIDefaultAllocator<nsLogEntry>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Msg", m_sMsg),
    NS_MEMBER_PROPERTY("Tag", m_sTag),
    NS_ENUM_MEMBER_PROPERTY("Type", nsLogMsgType, m_Type),
    NS_MEMBER_PROPERTY("Indentation", m_uiIndentation),
    NS_MEMBER_PROPERTY("Time", m_fSeconds),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

nsLogEntry::nsLogEntry() = default;

nsLogEntry::nsLogEntry(const nsLoggingEventData& le)
{
  m_sMsg = le.m_sText;
  m_sTag = le.m_sTag;
  m_Type = le.m_EventType;
  m_uiIndentation = le.m_uiIndentation;
#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  m_fSeconds = le.m_fSeconds;
#else
  m_fSeconds = 0.0f;
#endif
}

nsLogEntryDelegate::nsLogEntryDelegate(Callback callback, nsLogMsgType::Enum logLevel)
  : m_Callback(callback)
{
  SetLogLevel(logLevel);
}

void nsLogEntryDelegate::HandleLogMessage(const nsLoggingEventData& le)
{
  nsLogEntry e(le);
  m_Callback(e);
}

NS_STATICLINK_FILE(Foundation, Foundation_Logging_Implementation_LogEntry);

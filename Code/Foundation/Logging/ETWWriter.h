#pragma once

#include <Foundation/Logging/Log.h>

namespace wdLogWriter
{

  /// \brief A simple log writer that outputs all log messages to the wd ETW provider.
  class WD_FOUNDATION_DLL ETW
  {
  public:
    /// \brief Register this at wdLog to write all log messages to ETW.
    static void LogMessageHandler(const wdLoggingEventData& eventData);

    /// \brief Log Message to ETW.
    static void LogMessage(wdLogMsgType::Enum eventType, wdUInt8 uiIndentation, wdStringView sText);
  };
} // namespace wdLogWriter

#pragma once

#include <Foundation/Logging/Log.h>

namespace nsLogWriter
{

  /// \brief A simple log writer that outputs all log messages to the ns ETW provider.
  class NS_FOUNDATION_DLL ETW
  {
  public:
    /// \brief Register this at nsLog to write all log messages to ETW.
    static void LogMessageHandler(const nsLoggingEventData& eventData);

    /// \brief Log Message to ETW.
    static void LogMessage(nsLogMsgType::Enum eventType, nsUInt8 uiIndentation, nsStringView sText);
  };
} // namespace nsLogWriter

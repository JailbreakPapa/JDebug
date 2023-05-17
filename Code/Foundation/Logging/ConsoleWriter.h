#pragma once

#include <Foundation/Logging/Log.h>

namespace wdLogWriter
{
  /// \brief A simple log writer that writes out log messages using printf.
  class WD_FOUNDATION_DLL Console
  {
  public:
    /// \brief Register this at wdLog to write all log messages to the console using printf.
    static void LogMessageHandler(const wdLoggingEventData& eventData);

    /// \brief Allows to indicate in what form timestamps should be added to log messages.
    static void SetTimestampMode(wdLog::TimestampMode mode);

  private:
    static wdLog::TimestampMode s_TimestampMode;
  };
} // namespace wdLogWriter

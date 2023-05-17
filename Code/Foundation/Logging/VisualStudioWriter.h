#pragma once

#include <Foundation/Logging/Log.h>

namespace wdLogWriter
{

  /// \brief A simple log writer that outputs all log messages to visual studios output window
  class WD_FOUNDATION_DLL VisualStudio
  {
  public:
    /// \brief Register this at wdLog to write all log messages to visual studios output window.
    static void LogMessageHandler(const wdLoggingEventData& eventData);
  };
} // namespace wdLogWriter

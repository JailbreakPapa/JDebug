#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/String.h>

WD_DECLARE_REFLECTABLE_TYPE(WD_FOUNDATION_DLL, wdLogMsgType);

/// \brief A persistent log entry created from a wdLoggingEventData.
/// Allows for a log event to survive for longer than just the event
/// and is reflected, allowing for it to be sent to remote targets.
struct WD_FOUNDATION_DLL wdLogEntry
{
  wdLogEntry();
  wdLogEntry(const wdLoggingEventData& le);

  wdString m_sMsg;
  wdString m_sTag;
  wdEnum<wdLogMsgType> m_Type;
  wdUInt8 m_uiIndentation = 0;
  double m_fSeconds = 0;
};

WD_DECLARE_REFLECTABLE_TYPE(WD_FOUNDATION_DLL, wdLogEntry);

/// \brief A log interface implementation that converts a log event into
/// a wdLogEntry and calls a delegate with it.
///
/// A typical use case is to re-route and store log messages in a scope:
/// \code{.cpp}
///   {
///     wdLogEntryDelegate logger(([&array](wdLogEntry& entry) -> void
///     {
///       array.PushBack(std::move(entry));
///     }));
///     wdLogSystemScope logScope(&logger);
///     *log something*
///   }
/// \endcode
class WD_FOUNDATION_DLL wdLogEntryDelegate : public wdLogInterface
{
public:
  typedef wdDelegate<void(wdLogEntry&)> Callback;
  /// \brief Log events will be delegated to the given callback.
  wdLogEntryDelegate(Callback callback, wdLogMsgType::Enum logLevel = wdLogMsgType::All);
  virtual void HandleLogMessage(const wdLoggingEventData& le) override;

private:
  Callback m_Callback;
};

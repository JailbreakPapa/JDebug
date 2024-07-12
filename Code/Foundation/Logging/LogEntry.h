#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/String.h>

NS_DECLARE_REFLECTABLE_TYPE(NS_FOUNDATION_DLL, nsLogMsgType);

/// \brief A persistent log entry created from a nsLoggingEventData.
/// Allows for a log event to survive for longer than just the event
/// and is reflected, allowing for it to be sent to remote targets.
struct NS_FOUNDATION_DLL nsLogEntry
{
  nsLogEntry();
  nsLogEntry(const nsLoggingEventData& le);

  nsString m_sMsg;
  nsString m_sTag;
  nsEnum<nsLogMsgType> m_Type;
  nsUInt8 m_uiIndentation = 0;
  double m_fSeconds = 0;
};

NS_DECLARE_REFLECTABLE_TYPE(NS_FOUNDATION_DLL, nsLogEntry);

/// \brief A log interface implementation that converts a log event into
/// a nsLogEntry and calls a delegate with it.
///
/// A typical use case is to re-route and store log messages in a scope:
/// \code{.cpp}
///   {
///     nsLogEntryDelegate logger(([&array](nsLogEntry& entry) -> void
///     {
///       array.PushBack(std::move(entry));
///     }));
///     nsLogSystemScope logScope(&logger);
///     *log something*
///   }
/// \endcode
class NS_FOUNDATION_DLL nsLogEntryDelegate : public nsLogInterface
{
public:
  using Callback = nsDelegate<void(nsLogEntry&)>;
  /// \brief Log events will be delegated to the given callback.
  nsLogEntryDelegate(Callback callback, nsLogMsgType::Enum logLevel = nsLogMsgType::All);
  virtual void HandleLogMessage(const nsLoggingEventData& le) override;

private:
  Callback m_Callback;
};

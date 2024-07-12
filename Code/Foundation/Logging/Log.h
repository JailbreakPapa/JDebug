#pragma once

#include <Foundation/Communication/Event.h>
#include <Foundation/Strings/FormatString.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Strings/StringUtils.h>
#include <Foundation/Threading/AtomicInteger.h>
#include <Foundation/Time/Time.h>

/// \brief Use this helper macro to easily create a scoped logging group. Will generate unique variable names to make the static code
/// analysis happy.
#define NS_LOG_BLOCK nsLogBlock NS_CONCAT(_logblock_, NS_SOURCE_LINE)

/// \brief Use this helper macro to easily mute all logging in a scope.
#define NS_LOG_BLOCK_MUTE()                            \
  nsMuteLog NS_CONCAT(_logmuteblock_, NS_SOURCE_LINE); \
  nsLogSystemScope NS_CONCAT(_logscope_, NS_SOURCE_LINE)(&NS_CONCAT(_logmuteblock_, NS_SOURCE_LINE))

// Forward declaration, class is at the end of this file
class nsLogBlock;


/// \brief Describes the types of events that nsLog sends.
struct NS_FOUNDATION_DLL nsLogMsgType
{
  using StorageType = nsInt8;

  enum Enum : nsInt8
  {
    GlobalDefault = -4,    ///< Takes the log level from the nsLog default value. See nsLog::SetDefaultLogLevel().
    Flush = -3,            ///< The user explicitly called nsLog::Flush() to instruct log writers to flush any cached output.
    BeginGroup = -2,       ///< A logging group has been opened.
    EndGroup = -1,         ///< A logging group has been closed.
    None = 0,              ///< Can be used to disable all log message types.
    ErrorMsg = 1,          ///< An error message.
    SeriousWarningMsg = 2, ///< A serious warning message.
    WarningMsg = 3,        ///< A warning message.
    SuccessMsg = 4,        ///< A success message.
    InfoMsg = 5,           ///< An info message.
    DevMsg = 6,            ///< A development message.
    DebugMsg = 7,          ///< A debug message.
    All = 8,               ///< Can be used to enable all log message types.
    ENUM_COUNT,
    Default = None,
  };
};

/// \brief The data that is sent through nsLogInterface.
struct NS_FOUNDATION_DLL nsLoggingEventData
{
  /// \brief The type of information that is sent.
  nsLogMsgType::Enum m_EventType = nsLogMsgType::None;

  /// \brief How many "levels" to indent.
  nsUInt8 m_uiIndentation = 0;

  /// \brief The information text.
  nsStringView m_sText;

  /// \brief An optional tag extracted from the log-string (if it started with "[SomeTag]Logging String.") Can be used by log-writers for
  /// additional configuration, or simply be ignored.
  nsStringView m_sTag;

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  /// \brief Used by log-blocks for profiling the duration of the block
  double m_fSeconds = 0;
#endif
};

using nsLoggingEvent = nsEvent<const nsLoggingEventData&, nsMutex>;

/// \brief Base class for all logging classes.
///
/// You can derive from this class to create your own logging system,
/// which you can pass to the functions in nsLog.
class NS_FOUNDATION_DLL nsLogInterface
{
public:
  /// \brief Override this function to handle logging events.
  virtual void HandleLogMessage(const nsLoggingEventData& le) = 0;

  /// \brief LogLevel is between nsLogEventType::None and nsLogEventType::All and defines which messages will be logged and which will be
  /// filtered out.
  NS_ALWAYS_INLINE void SetLogLevel(nsLogMsgType::Enum logLevel) { m_LogLevel = logLevel; }

  /// \brief Returns the currently set log level.
  NS_ALWAYS_INLINE nsLogMsgType::Enum GetLogLevel() { return m_LogLevel; }

private:
  friend class nsLog;
  friend class nsLogBlock;
  nsLogBlock* m_pCurrentBlock = nullptr;
  nsLogMsgType::Enum m_LogLevel = nsLogMsgType::GlobalDefault;
  nsUInt32 m_uiLoggedMsgsSinceFlush = 0;
  nsTime m_LastFlushTime;
};


/// \brief Used to ignore all log messages.
/// \sa NS_LOG_BLOCK_MUTE
class nsMuteLog : public nsLogInterface
{
public:
  nsMuteLog()
  {
    SetLogLevel(nsLogMsgType::None);
  }

  virtual void HandleLogMessage(const nsLoggingEventData&) override {}
};


/// \brief This is the standard log system that nsLog sends all messages to.
///
/// It allows to register log writers, such that you can be informed of all log messages and write them
/// to different outputs.
class NS_FOUNDATION_DLL nsGlobalLog : public nsLogInterface
{
public:
  virtual void HandleLogMessage(const nsLoggingEventData& le) override;

  /// \brief Allows to register a function as an event receiver.
  static nsEventSubscriptionID AddLogWriter(nsLoggingEvent::Handler handler);

  /// \brief Unregisters a previously registered receiver. It is an error to unregister a receiver that was not registered.
  static void RemoveLogWriter(nsLoggingEvent::Handler handler);

  /// \brief Unregisters a previously registered receiver. It is an error to unregister a receiver that was not registered.
  static void RemoveLogWriter(nsEventSubscriptionID& ref_subscriptionID);

  /// \brief Returns how many message of the given type occurred.
  static nsUInt32 GetMessageCount(nsLogMsgType::Enum messageType) { return s_uiMessageCount[messageType]; }

  /// nsLogInterfaces are thread_local and therefore a dedicated nsGlobalLog is created per thread.
  /// Especially during testing one may want to replace the log system everywhere, to catch certain messages, no matter on which thread they
  /// happen. Unfortunately that is not so easy, as one cannot modify the thread_local system for all the other threads. This function makes
  /// it possible to at least force all messages that go through any nsGlobalLog to be redirected to one other log interface. Be aware that
  /// that interface has to be thread-safe. Also, only one override can be set at a time, SetGlobalLogOverride() will assert that no other
  /// override is set at the moment.
  static void SetGlobalLogOverride(nsLogInterface* pInterface);

private:
  /// \brief Counts the number of messages of each type.
  static nsAtomicInteger32 s_uiMessageCount[nsLogMsgType::ENUM_COUNT];

  /// \brief Manages all the Event Handlers for the logging events.
  static nsLoggingEvent s_LoggingEvent;

  static nsLogInterface* s_pOverrideLog;

private:
  NS_DISALLOW_COPY_AND_ASSIGN(nsGlobalLog);

  friend class nsLog; // only nsLog may create instances of this class
  nsGlobalLog() = default;
};

/// \brief Static class that allows to write out logging information.
///
/// This class takes logging information, prepares it and then broadcasts it to all interested code
/// via the event interface. It does not write anything on disk or somewhere else, itself. Instead it
/// allows to register custom log writers that can then write it to disk, to console, send it over a
/// network or pop up a message box. Whatever suits the current situation.
/// Since event handlers can be registered only temporarily, it is also possible to just gather all
/// errors that occur during some operation and then unregister the event handler again.
class NS_FOUNDATION_DLL nsLog
{
public:
  /// \brief Allows to change which logging system is used by default on the current thread. If nothing is set, nsGlobalLog is used.
  ///
  /// Replacing the log system on a thread does not delete the previous system, so it can be reinstated later again.
  /// This can be used to temporarily route all logging to a custom system.
  static void SetThreadLocalLogSystem(nsLogInterface* pInterface);

  /// \brief Returns the currently set default logging system, or a thread local instance of nsGlobalLog, if nothing else was set.
  static nsLogInterface* GetThreadLocalLogSystem();

  /// \brief Sets the default log level which is used by all nsLogInterface's that have their log level set to nsLogMsgType::GlobalDefault
  static void SetDefaultLogLevel(nsLogMsgType::Enum logLevel);

  /// \brief Returns the currently set default log level.
  static nsLogMsgType::Enum GetDefaultLogLevel();

  /// \brief An error that needs to be fixed as soon as possible.
  static void Error(nsLogInterface* pInterface, const nsFormatString& string);

  /// \brief An error that needs to be fixed as soon as possible.
  template <typename... ARGS>
  static void Error(nsStringView sFormat, ARGS&&... args)
  {
    Error(GetThreadLocalLogSystem(), nsFormatStringImpl<ARGS...>(sFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Overload of Error() to output messages to a specific log.
  template <typename... ARGS>
  static void Error(nsLogInterface* pInterface, nsStringView sFormat, ARGS&&... args)
  {
    Error(pInterface, nsFormatStringImpl<ARGS...>(sFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Not an error, but definitely a big problem, that should be looked into very soon.
  static void SeriousWarning(nsLogInterface* pInterface, const nsFormatString& string);

  /// \brief Not an error, but definitely a big problem, that should be looked into very soon.
  template <typename... ARGS>
  static void SeriousWarning(nsStringView sFormat, ARGS&&... args)
  {
    SeriousWarning(GetThreadLocalLogSystem(), nsFormatStringImpl<ARGS...>(sFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Overload of SeriousWarning() to output messages to a specific log.
  template <typename... ARGS>
  static void SeriousWarning(nsLogInterface* pInterface, nsStringView sFormat, ARGS&&... args)
  {
    SeriousWarning(pInterface, nsFormatStringImpl<ARGS...>(sFormat, std::forward<ARGS>(args)...));
  }

  /// \brief A potential problem or a performance warning. Might be possible to ignore it.
  static void Warning(nsLogInterface* pInterface, const nsFormatString& string);

  /// \brief A potential problem or a performance warning. Might be possible to ignore it.
  template <typename... ARGS>
  static void Warning(nsStringView sFormat, ARGS&&... args)
  {
    Warning(GetThreadLocalLogSystem(), nsFormatStringImpl<ARGS...>(sFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Overload of Warning() to output messages to a specific log.
  template <typename... ARGS>
  static void Warning(nsLogInterface* pInterface, nsStringView sFormat, ARGS&&... args)
  {
    Warning(pInterface, nsFormatStringImpl<ARGS...>(sFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Status information that something was completed successfully.
  static void Success(nsLogInterface* pInterface, const nsFormatString& string);

  /// \brief Status information that something was completed successfully.
  template <typename... ARGS>
  static void Success(nsStringView sFormat, ARGS&&... args)
  {
    Success(GetThreadLocalLogSystem(), nsFormatStringImpl<ARGS...>(sFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Overload of Success() to output messages to a specific log.
  template <typename... ARGS>
  static void Success(nsLogInterface* pInterface, nsStringView sFormat, ARGS&&... args)
  {
    Success(pInterface, nsFormatStringImpl<ARGS...>(sFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Status information that is important.
  static void Info(nsLogInterface* pInterface, const nsFormatString& string);

  /// \brief Status information that is important.
  template <typename... ARGS>
  static void Info(nsStringView sFormat, ARGS&&... args)
  {
    Info(GetThreadLocalLogSystem(), nsFormatStringImpl<ARGS...>(sFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Overload of Info() to output messages to a specific log.
  template <typename... ARGS>
  static void Info(nsLogInterface* pInterface, nsStringView sFormat, ARGS&&... args)
  {
    Info(pInterface, nsFormatStringImpl<ARGS...>(sFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Status information that is nice to have during development.
  ///
  /// This function is compiled out in non-development builds.
  static void Dev(nsLogInterface* pInterface, const nsFormatString& string);

  /// \brief Status information that is nice to have during development.
  ///
  /// This function is compiled out in non-development builds.
  template <typename... ARGS>
  static void Dev(nsStringView sFormat, ARGS&&... args)
  {
    Dev(GetThreadLocalLogSystem(), nsFormatStringImpl<ARGS...>(sFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Overload of Dev() to output messages to a specific log.
  template <typename... ARGS>
  static void Dev(nsLogInterface* pInterface, nsStringView sFormat, ARGS&&... args)
  {
    Dev(pInterface, nsFormatStringImpl<ARGS...>(sFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Status information during debugging. Very verbose. Usually only temporarily added to the code.
  ///
  /// This function is compiled out in non-debug builds.
  static void Debug(nsLogInterface* pInterface, const nsFormatString& string);

  /// \brief Status information during debugging. Very verbose. Usually only temporarily added to the code.
  ///
  /// This function is compiled out in non-debug builds.
  template <typename... ARGS>
  static void Debug(nsStringView sFormat, ARGS&&... args)
  {
    Debug(GetThreadLocalLogSystem(), nsFormatStringImpl<ARGS...>(sFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Overload of Debug() to output messages to a specific log.
  template <typename... ARGS>
  static void Debug(nsLogInterface* pInterface, nsStringView sFormat, ARGS&&... args)
  {
    Debug(pInterface, nsFormatStringImpl<ARGS...>(sFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Instructs log writers to flush their caches, to ensure all log output (even non-critical information) is written.
  ///
  /// On some log writers this has no effect.
  /// Do not call this too frequently as it incurs a performance penalty.
  ///
  /// \param uiNumNewMsgThreshold
  ///   If this is set to a number larger than zero, the flush may be ignored if the given nsLogInterface
  ///   has logged fewer than this many messages since the last flush.
  /// \param timeIntervalThreshold
  ///   The flush may be ignored if less time has past than this, since the last flush.
  ///
  /// If either enough messages have been logged, or the flush interval has been exceeded, the flush is executed.
  /// To force a flush, set \a uiNumNewMsgThreshold  to zero.
  /// However, a flush is always ignored if not a single message was logged in between.
  ///
  /// \return Returns true if the flush is executed.
  static bool Flush(nsUInt32 uiNumNewMsgThreshold = 0, nsTime timeIntervalThreshold = nsTime::MakeFromSeconds(10), nsLogInterface* pInterface = GetThreadLocalLogSystem());

  /// \brief Usually called internally by the other log functions, but can be called directly, if the message type is already known.
  /// pInterface must be != nullptr.
  static void BroadcastLoggingEvent(nsLogInterface* pInterface, nsLogMsgType::Enum type, nsStringView sString);

  /// \brief Calls low-level OS functionality to print a string to the typical outputs, e.g. printf and OutputDebugString.
  ///
  /// Use this function to log unrecoverable errors like asserts, crash handlers etc.
  /// This function is meant for short term debugging when actual printing to the console is desired. Code using it should be temporary.
  /// This function flushes the output immediately, to ensure output is never lost during a crash. Consequently it has a high performance
  /// overhead.
  static void Print(const char* szText);

  /// \brief Calls low-level OS functionality to print a string to the typical outputs. Forwards to Print.
  /// \note This function uses actual printf formatting, not nsFormatString syntax.
  /// \sa nsLog::Print
  static void Printf(const char* szFormat, ...);

  /// \brief Signature of the custom print function used by nsLog::SetCustomPrintFunction.
  using PrintFunction = void (*)(const char* szText);

  /// \brief Sets a custom function that is called in addition to the default behavior of nsLog::Print.
  static void SetCustomPrintFunction(PrintFunction func);

  /// \brief Shows a simple message box using the OS functionality.
  ///
  /// This should only be used for critical information that can't be conveyed in another way.
  static void OsMessageBox(const nsFormatString& text);

  /// \brief This enum is used in context of outputting timestamp information to indicate a formatting for said timestamps.
  enum class TimestampMode
  {
    None = 0,     ///< No timestamp will be added at all.
    Numeric = 1,  ///< A purely numeric timestamp will be added. Ex.: [2019-08-16 13:40:30.345 (UTC)] Log message.
    Textual = 2,  ///< A timestamp with textual fields will be added. Ex.: [2019 Aug 16 (Fri) 13:40:30.345 (UTC)] Log message.
    TimeOnly = 3, ///< A short timestamp (time only, no timnsone indicator) is added. Ex: [13:40:30.345] Log message.
  };

  static void GenerateFormattedTimestamp(TimestampMode mode, nsStringBuilder& ref_sTimestampOut);

private:
  // Needed to call 'EndLogBlock'
  friend class nsLogBlock;

  /// \brief Which messages to filter out by default.
  static nsLogMsgType::Enum s_DefaultLogLevel;

  /// \brief Ends grouping log messages.
  static void EndLogBlock(nsLogInterface* pInterface, nsLogBlock* pBlock);

  static void WriteBlockHeader(nsLogInterface* pInterface, nsLogBlock* pBlock);

  static PrintFunction s_CustomPrintFunction;
};


/// \brief Instances of this class will group messages in a scoped block together.
class NS_FOUNDATION_DLL nsLogBlock
{
public:
  /// \brief Creates a named grouping block for log messages.
  ///
  /// Use the szContextInfo to pass in a string that can give additional context information (e.g. a file name).
  /// This string must point to valid memory until after the log block object is destroyed.
  /// Log writers get these strings provided through the nsLoggingEventData::m_szTag variable.
  /// \note The log block header (and context info) will not be printed until a message is successfully logged,
  /// i.e. as long as all messages in this block are filtered out (via the LogLevel setting), the log block
  /// header will not be printed, to prevent spamming the log.
  ///
  /// This constructor will output the log block data to the nsGlobalLog.
  nsLogBlock(nsStringView sName, nsStringView sContextInfo = {});

  /// \brief Creates a named grouping block for log messages.
  ///
  /// This variant of the constructor takes an explicit nsLogInterface to write the log messages to.
  nsLogBlock(nsLogInterface* pInterface, nsStringView sName, nsStringView sContextInfo = {});

  ~nsLogBlock();

private:
  friend class nsLog;

  nsLogInterface* m_pLogInterface;
  nsLogBlock* m_pParentBlock;
  nsStringView m_sName;
  nsStringView m_sContextInfo;
  nsUInt8 m_uiBlockDepth;
  bool m_bWritten;
#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  double m_fSeconds; // for profiling
#endif
};

/// \brief A class that sets a custom nsLogInterface as the thread local default log system,
/// and resets the previous system when it goes out of scope.
class NS_FOUNDATION_DLL nsLogSystemScope
{
public:
  /// \brief The given nsLogInterface is passed to nsLog::SetThreadLocalLogSystem().
  explicit nsLogSystemScope(nsLogInterface* pInterface)
  {
    m_pPrevious = nsLog::GetThreadLocalLogSystem();
    nsLog::SetThreadLocalLogSystem(pInterface);
  }

  /// \brief Resets the previous nsLogInterface through nsLog::SetThreadLocalLogSystem()
  ~nsLogSystemScope() { nsLog::SetThreadLocalLogSystem(m_pPrevious); }

protected:
  nsLogInterface* m_pPrevious;

private:
  NS_DISALLOW_COPY_AND_ASSIGN(nsLogSystemScope);
};


/// \brief A simple log interface implementation that gathers all messages in a string buffer.
class nsLogSystemToBuffer : public nsLogInterface
{
public:
  virtual void HandleLogMessage(const nsLoggingEventData& le) override
  {
    switch (le.m_EventType)
    {
      case nsLogMsgType::ErrorMsg:
        m_sBuffer.Append("Error: ", le.m_sText, "\n");
        break;
      case nsLogMsgType::SeriousWarningMsg:
      case nsLogMsgType::WarningMsg:
        m_sBuffer.Append("Warning: ", le.m_sText, "\n");
        break;
      case nsLogMsgType::SuccessMsg:
      case nsLogMsgType::InfoMsg:
      case nsLogMsgType::DevMsg:
      case nsLogMsgType::DebugMsg:
        m_sBuffer.Append(le.m_sText, "\n");
        break;
      default:
        break;
    }
  }

  nsStringBuilder m_sBuffer;
};

#include <Foundation/Logging/Implementation/Log_inl.h>

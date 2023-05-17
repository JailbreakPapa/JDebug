#pragma once

#include <Foundation/Communication/Event.h>
#include <Foundation/Strings/FormatString.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Strings/StringUtils.h>
#include <Foundation/Threading/AtomicInteger.h>
#include <Foundation/Time/Time.h>

/// \brief Use this helper macro to easily create a scoped logging group. Will generate unique variable names to make the static code
/// analysis happy.
#define WD_LOG_BLOCK wdLogBlock WD_CONCAT(_logblock_, WD_SOURCE_LINE)

/// \brief Use this helper macro to easily mute all logging in a scope.
#define WD_LOG_BLOCK_MUTE()                            \
  wdMuteLog WD_CONCAT(_logmuteblock_, WD_SOURCE_LINE); \
  wdLogSystemScope WD_CONCAT(_logscope_, WD_SOURCE_LINE)(&WD_CONCAT(_logmuteblock_, WD_SOURCE_LINE))

// Forward declaration, class is at the end of this file
class wdLogBlock;


/// \brief Describes the types of events that wdLog sends.
struct WD_FOUNDATION_DLL wdLogMsgType
{
  using StorageType = wdInt8;

  enum Enum : wdInt8
  {
    GlobalDefault = -4,    ///< Takes the log level from the wdLog default value. See wdLog::SetDefaultLogLevel().
    Flush = -3,            ///< The user explicitly called wdLog::Flush() to instruct log writers to flush any cached output.
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

/// \brief The data that is sent through wdLogInterface.
struct WD_FOUNDATION_DLL wdLoggingEventData
{
  /// \brief The type of information that is sent.
  wdLogMsgType::Enum m_EventType = wdLogMsgType::None;

  /// \brief How many "levels" to indent.
  wdUInt8 m_uiIndentation = 0;

  /// \brief The information text.
  wdStringView m_sText;

  /// \brief An optional tag extracted from the log-string (if it started with "[SomeTag]Logging String.") Can be used by log-writers for
  /// additional configuration, or simply be ignored.
  wdStringView m_sTag;

#if WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT)
  /// \brief Used by log-blocks for profiling the duration of the block
  double m_fSeconds = 0;
#endif
};

using wdLoggingEvent = wdEvent<const wdLoggingEventData&, wdMutex>;

/// \brief Base class for all logging classes.
///
/// You can derive from this class to create your own logging system,
/// which you can pass to the functions in wdLog.
class WD_FOUNDATION_DLL wdLogInterface
{
public:
  /// \brief Override this function to handle logging events.
  virtual void HandleLogMessage(const wdLoggingEventData& le) = 0;

  /// \brief LogLevel is between wdLogEventType::None and wdLogEventType::All and defines which messages will be logged and which will be
  /// filtered out.
  WD_ALWAYS_INLINE void SetLogLevel(wdLogMsgType::Enum logLevel) { m_LogLevel = logLevel; }

  /// \brief Returns the currently set log level.
  WD_ALWAYS_INLINE wdLogMsgType::Enum GetLogLevel() { return m_LogLevel; }

private:
  friend class wdLog;
  friend class wdLogBlock;
  wdLogBlock* m_pCurrentBlock = nullptr;
  wdLogMsgType::Enum m_LogLevel = wdLogMsgType::GlobalDefault;
  wdUInt32 m_uiLoggedMsgsSinceFlush = 0;
  wdTime m_LastFlushTime;
};


/// \brief Used to ignore all log messages.
/// \sa WD_LOG_BLOCK_MUTE
class wdMuteLog : public wdLogInterface
{
public:
  wdMuteLog()
  {
    SetLogLevel(wdLogMsgType::None);
  }

  virtual void HandleLogMessage(const wdLoggingEventData&) override {}
};


/// \brief This is the standard log system that wdLog sends all messages to.
///
/// It allows to register log writers, such that you can be informed of all log messages and write them
/// to different outputs.
class WD_FOUNDATION_DLL wdGlobalLog : public wdLogInterface
{
public:
  virtual void HandleLogMessage(const wdLoggingEventData& le) override;

  /// \brief Allows to register a function as an event receiver.
  static wdEventSubscriptionID AddLogWriter(wdLoggingEvent::Handler handler);

  /// \brief Unregisters a previously registered receiver. It is an error to unregister a receiver that was not registered.
  static void RemoveLogWriter(wdLoggingEvent::Handler handler);

  /// \brief Unregisters a previously registered receiver. It is an error to unregister a receiver that was not registered.
  static void RemoveLogWriter(wdEventSubscriptionID& ref_subscriptionID);

  /// \brief Returns how many message of the given type occurred.
  static wdUInt32 GetMessageCount(wdLogMsgType::Enum messageType) { return s_uiMessageCount[messageType]; }

  /// wdLogInterfaces are thread_local and therefore a dedicated wdGlobalLog is created per thread.
  /// Especially during testing one may want to replace the log system everywhere, to catch certain messages, no matter on which thread they
  /// happen. Unfortunately that is not so easy, as one cannot modify the thread_local system for all the other threads. This function makes
  /// it possible to at least force all messages that go through any wdGlobalLog to be redirected to one other log interface. Be aware that
  /// that interface has to be thread-safe. Also, only one override can be set at a time, SetGlobalLogOverride() will assert that no other
  /// override is set at the moment.
  static void SetGlobalLogOverride(wdLogInterface* pInterface);

private:
  /// \brief Counts the number of messages of each type.
  static wdAtomicInteger32 s_uiMessageCount[wdLogMsgType::ENUM_COUNT];

  /// \brief Manages all the Event Handlers for the logging events.
  static wdLoggingEvent s_LoggingEvent;

  static wdLogInterface* s_pOverrideLog;

private:
  WD_DISALLOW_COPY_AND_ASSIGN(wdGlobalLog);

  friend class wdLog; // only wdLog may create instances of this class
  wdGlobalLog() = default;
};

/// \brief Static class that allows to write out logging information.
///
/// This class takes logging information, prepares it and then broadcasts it to all interested code
/// via the event interface. It does not write anything on disk or somewhere else, itself. Instead it
/// allows to register custom log writers that can then write it to disk, to console, send it over a
/// network or pop up a message box. Whatever suits the current situation.
/// Since event handlers can be registered only temporarily, it is also possible to just gather all
/// errors that occur during some operation and then unregister the event handler again.
class WD_FOUNDATION_DLL wdLog
{
public:
  /// \brief Allows to change which logging system is used by default on the current thread. If nothing is set, wdGlobalLog is used.
  ///
  /// Replacing the log system on a thread does not delete the previous system, so it can be reinstated later again.
  /// This can be used to temporarily route all logging to a custom system.
  static void SetThreadLocalLogSystem(wdLogInterface* pInterface);

  /// \brief Returns the currently set default logging system, or a thread local instance of wdGlobalLog, if nothing else was set.
  static wdLogInterface* GetThreadLocalLogSystem();

  /// \brief Sets the default log level which is used by all wdLogInterface's that have their log level set to wdLogMsgType::GlobalDefault
  static void SetDefaultLogLevel(wdLogMsgType::Enum logLevel);

  /// \brief Returns the currently set default log level.
  static wdLogMsgType::Enum GetDefaultLogLevel();

  /// \brief An error that needs to be fixed as soon as possible.
  static void Error(wdLogInterface* pInterface, const wdFormatString& string);

  /// \brief An error that needs to be fixed as soon as possible.
  template <typename... ARGS>
  static void Error(const char* szFormat, ARGS&&... args)
  {
    Error(GetThreadLocalLogSystem(), wdFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Overload of Error() to output messages to a specific log.
  template <typename... ARGS>
  static void Error(wdLogInterface* pInterface, const char* szFormat, ARGS&&... args)
  {
    Error(pInterface, wdFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Not an error, but definitely a big problem, that should be looked into very soon.
  static void SeriousWarning(wdLogInterface* pInterface, const wdFormatString& string);

  /// \brief Not an error, but definitely a big problem, that should be looked into very soon.
  template <typename... ARGS>
  static void SeriousWarning(const char* szFormat, ARGS&&... args)
  {
    SeriousWarning(GetThreadLocalLogSystem(), wdFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Overload of SeriousWarning() to output messages to a specific log.
  template <typename... ARGS>
  static void SeriousWarning(wdLogInterface* pInterface, const char* szFormat, ARGS&&... args)
  {
    SeriousWarning(pInterface, wdFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...));
  }

  /// \brief A potential problem or a performance warning. Might be possible to ignore it.
  static void Warning(wdLogInterface* pInterface, const wdFormatString& string);

  /// \brief A potential problem or a performance warning. Might be possible to ignore it.
  template <typename... ARGS>
  static void Warning(const char* szFormat, ARGS&&... args)
  {
    Warning(GetThreadLocalLogSystem(), wdFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Overload of Warning() to output messages to a specific log.
  template <typename... ARGS>
  static void Warning(wdLogInterface* pInterface, const char* szFormat, ARGS&&... args)
  {
    Warning(pInterface, wdFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Status information that something was completed successfully.
  static void Success(wdLogInterface* pInterface, const wdFormatString& string);

  /// \brief Status information that something was completed successfully.
  template <typename... ARGS>
  static void Success(const char* szFormat, ARGS&&... args)
  {
    Success(GetThreadLocalLogSystem(), wdFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Overload of Success() to output messages to a specific log.
  template <typename... ARGS>
  static void Success(wdLogInterface* pInterface, const char* szFormat, ARGS&&... args)
  {
    Success(pInterface, wdFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Status information that is important.
  static void Info(wdLogInterface* pInterface, const wdFormatString& string);

  /// \brief Status information that is important.
  template <typename... ARGS>
  static void Info(const char* szFormat, ARGS&&... args)
  {
    Info(GetThreadLocalLogSystem(), wdFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Overload of Info() to output messages to a specific log.
  template <typename... ARGS>
  static void Info(wdLogInterface* pInterface, const char* szFormat, ARGS&&... args)
  {
    Info(pInterface, wdFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Status information that is nice to have during development.
  ///
  /// This function is compiled out in non-development builds.
  static void Dev(wdLogInterface* pInterface, const wdFormatString& string);

  /// \brief Status information that is nice to have during development.
  ///
  /// This function is compiled out in non-development builds.
  template <typename... ARGS>
  static void Dev(const char* szFormat, ARGS&&... args)
  {
    Dev(GetThreadLocalLogSystem(), wdFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Overload of Dev() to output messages to a specific log.
  template <typename... ARGS>
  static void Dev(wdLogInterface* pInterface, const char* szFormat, ARGS&&... args)
  {
    Dev(pInterface, wdFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Status information during debugging. Very verbose. Usually only temporarily added to the code.
  ///
  /// This function is compiled out in non-debug builds.
  static void Debug(wdLogInterface* pInterface, const wdFormatString& string);

  /// \brief Status information during debugging. Very verbose. Usually only temporarily added to the code.
  ///
  /// This function is compiled out in non-debug builds.
  template <typename... ARGS>
  static void Debug(const char* szFormat, ARGS&&... args)
  {
    Debug(GetThreadLocalLogSystem(), wdFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Overload of Debug() to output messages to a specific log.
  template <typename... ARGS>
  static void Debug(wdLogInterface* pInterface, const char* szFormat, ARGS&&... args)
  {
    Debug(pInterface, wdFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Instructs log writers to flush their caches, to ensure all log output (even non-critical information) is written.
  ///
  /// On some log writers this has no effect.
  /// Do not call this too frequently as it incurs a performance penalty.
  ///
  /// \param uiNumNewMsgThreshold
  ///   If this is set to a number larger than zero, the flush may be ignored if the given wdLogInterface
  ///   has logged fewer than this many messages since the last flush.
  /// \param timeIntervalThreshold
  ///   The flush may be ignored if less time has past than this, since the last flush.
  ///
  /// If either enough messages have been logged, or the flush interval has been exceeded, the flush is executed.
  /// To force a flush, set \a uiNumNewMsgThreshold  to zero.
  /// However, a flush is always ignored if not a single message was logged in between.
  ///
  /// \return Returns true if the flush is executed.
  static bool Flush(wdUInt32 uiNumNewMsgThreshold = 0, wdTime timeIntervalThreshold = wdTime::Seconds(10), wdLogInterface* pInterface = GetThreadLocalLogSystem());

  /// \brief Usually called internally by the other log functions, but can be called directly, if the message type is already known.
  /// pInterface must be != nullptr.
  static void BroadcastLoggingEvent(wdLogInterface* pInterface, wdLogMsgType::Enum type, const char* szString);

  /// \brief Calls low-level OS functionality to print a string to the typical outputs, e.g. printf and OutputDebugString.
  ///
  /// Use this function to log unrecoverable errors like asserts, crash handlers etc.
  /// This function is meant for short term debugging when actual printing to the console is desired. Code using it should be temporary.
  /// This function flushes the output immediately, to ensure output is never lost during a crash. Consequently it has a high performance
  /// overhead.
  static void Print(const char* szText);

  /// \brief Calls low-level OS functionality to print a string to the typical outputs. Forwards to Print.
  /// \note This function uses actual printf formatting, not wdFormatString syntax.
  /// \sa wdLog::Print
  static void Printf(const char* szFormat, ...);

  /// \brief Signature of the custom print function used by wdLog::SetCustomPrintFunction.
  using PrintFunction = void (*)(const char* szText);

  /// \brief Sets a custom function that is called in addition to the default behavior of wdLog::Print.
  static void SetCustomPrintFunction(PrintFunction func);

  /// \brief Shows a simple message box using the OS functionality.
  ///
  /// This should only be used for critical information that can't be conveyed in another way.
  static void OsMessageBox(const wdFormatString& text);

  /// \brief This enum is used in context of outputting timestamp information to indicate a formatting for said timestamps.
  enum class TimestampMode
  {
    None = 0,     ///< No timestamp will be added at all.
    Numeric = 1,  ///< A purely numeric timestamp will be added. Ex.: [2019-08-16 13:40:30.345 (UTC)] Log message.
    Textual = 2,  ///< A timestamp with textual fields will be added. Ex.: [2019 Aug 16 (Fri) 13:40:30.345 (UTC)] Log message.
    TimeOnly = 3, ///< A short timestamp (time only, no timwdone indicator) is added. Ex: [13:40:30.345] Log message.
  };

  static void GenerateFormattedTimestamp(TimestampMode mode, wdStringBuilder& ref_sTimestampOut);

private:
  // Needed to call 'EndLogBlock'
  friend class wdLogBlock;

  /// \brief Which messages to filter out by default.
  static wdLogMsgType::Enum s_DefaultLogLevel;

  /// \brief Ends grouping log messages.
  static void EndLogBlock(wdLogInterface* pInterface, wdLogBlock* pBlock);

  static void WriteBlockHeader(wdLogInterface* pInterface, wdLogBlock* pBlock);

  static PrintFunction s_CustomPrintFunction;
};


/// \brief Instances of this class will group messages in a scoped block together.
class WD_FOUNDATION_DLL wdLogBlock
{
public:
  /// \brief Creates a named grouping block for log messages.
  ///
  /// Use the szContextInfo to pass in a string that can give additional context information (e.g. a file name).
  /// This string must point to valid memory until after the log block object is destroyed.
  /// Log writers get these strings provided through the wdLoggingEventData::m_szTag variable.
  /// \note The log block header (and context info) will not be printed until a message is successfully logged,
  /// i.e. as long as all messages in this block are filtered out (via the LogLevel setting), the log block
  /// header will not be printed, to prevent spamming the log.
  ///
  /// This constructor will output the log block data to the wdGlobalLog.
  wdLogBlock(wdStringView sName, wdStringView sContextInfo = {});

  /// \brief Creates a named grouping block for log messages.
  ///
  /// This variant of the constructor takes an explicit wdLogInterface to write the log messages to.
  wdLogBlock(wdLogInterface* pInterface, wdStringView sName, wdStringView sContextInfo = {});

  ~wdLogBlock();

private:
  friend class wdLog;

  wdLogInterface* m_pLogInterface;
  wdLogBlock* m_pParentBlock;
  wdStringView m_sName;
  wdStringView m_sContextInfo;
  wdUInt8 m_uiBlockDepth;
  bool m_bWritten;
#if WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT)
  double m_fSeconds; // for profiling
#endif
};

/// \brief A class that sets a custom wdLogInterface as the thread local default log system,
/// and resets the previous system when it goes out of scope.
class WD_FOUNDATION_DLL wdLogSystemScope
{
public:
  /// \brief The given wdLogInterface is passed to wdLog::SetThreadLocalLogSystem().
  explicit wdLogSystemScope(wdLogInterface* pInterface)
  {
    m_pPrevious = wdLog::GetThreadLocalLogSystem();
    wdLog::SetThreadLocalLogSystem(pInterface);
  }

  /// \brief Resets the previous wdLogInterface through wdLog::SetThreadLocalLogSystem()
  ~wdLogSystemScope() { wdLog::SetThreadLocalLogSystem(m_pPrevious); }

protected:
  wdLogInterface* m_pPrevious;

private:
  WD_DISALLOW_COPY_AND_ASSIGN(wdLogSystemScope);
};


/// \brief A simple log interface implementation that gathers all messages in a string buffer.
class wdLogSystemToBuffer : public wdLogInterface
{
public:
  virtual void HandleLogMessage(const wdLoggingEventData& le) override
  {
    switch (le.m_EventType)
    {
      case wdLogMsgType::ErrorMsg:
        m_sBuffer.Append("Error: ", le.m_sText, "\n");
        break;
      case wdLogMsgType::SeriousWarningMsg:
      case wdLogMsgType::WarningMsg:
        m_sBuffer.Append("Warning: ", le.m_sText, "\n");
        break;
      case wdLogMsgType::SuccessMsg:
      case wdLogMsgType::InfoMsg:
      case wdLogMsgType::DevMsg:
      case wdLogMsgType::DebugMsg:
        m_sBuffer.Append(le.m_sText, "\n");
        break;
      default:
        break;
    }
  }

  wdStringBuilder m_sBuffer;
};

#include <Foundation/Logging/Implementation/Log_inl.h>

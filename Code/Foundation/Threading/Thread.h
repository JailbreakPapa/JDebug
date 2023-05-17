#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Communication/Event.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Threading/ThreadUtils.h>

#include <Foundation/Threading/Implementation/OSThread.h>

// Warning: 'this' used in member initialization list (is fine here since it is just stored and not
// accessed in the constructor (so no operations on a not completely initialized object happen)

#define WD_MSVC_WARNING_NUMBER 4355
#include <Foundation/Basics/Compiler/MSVC/DisableWarning_MSVC.h>

#ifndef WD_THREAD_CLASS_ENTRY_POINT
#  error "Definition for wdThreadClassEntryPoint is missing on this platform!"
#endif

WD_THREAD_CLASS_ENTRY_POINT;

struct wdThreadEvent
{
  enum class Type
  {
    ThreadCreated,     ///< Called on the thread that creates the wdThread instance
    ThreadDestroyed,   ///< Called on the thread that destroys the wdThread instance
    StartingExecution, ///< Called on the thread that executes the wdThread instance
    FinishedExecution, ///< Called on the thread that executes the wdThread instance
  };

  Type m_Type;
  wdThread* m_pThread = nullptr;
};

/// \brief This class is the base class for platform independent long running threads
///
/// Used by deriving from this class and overriding the Run() method.
class WD_FOUNDATION_DLL wdThread : public wdOSThread
{
public:
  /// \brief Describes the thread status
  enum wdThreadStatus
  {
    Created = 0,
    Running,
    Finished
  };

  /// \brief Initializes the runnable class
  wdThread(const char* szName = "wdThread", wdUInt32 uiStackSize = 128 * 1024);

  /// \brief Destructor checks if the thread is deleted while still running, which is not allowed as this is a data hazard
  virtual ~wdThread();

  /// \brief Returns the thread status
  inline wdThreadStatus GetThreadStatus() const { return m_ThreadStatus; }

  /// \brief Helper function to determine if the thread is running
  inline bool IsRunning() const { return m_ThreadStatus == Running; }

  /// \brief Returns the thread name
  inline const char* GetThreadName() const { return m_sName.GetData(); }

  /// \brief These events inform about threads starting and finishing.
  ///
  /// The events are raised on the executing thread! That means thread-specific code may be executed during the event callback,
  /// e.g. to set up thread-local functionality.
  static wdEvent<const wdThreadEvent&, wdMutex> s_ThreadEvents;

private:
  /// \brief The run function can be used to implement a long running task in a thread in a platform independent way
  virtual wdUInt32 Run() = 0;


  volatile wdThreadStatus m_ThreadStatus;

  wdString m_sName;

  friend wdUInt32 RunThread(wdThread* pThread);
};

#include <Foundation/Basics/Compiler/MSVC/RestoreWarning_MSVC.h>

#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Communication/Event.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Threading/ThreadUtils.h>

#include <Foundation/Threading/Implementation/OSThread.h>

// Warning: 'this' used in member initialization list (is fine here since it is just stored and not
// accessed in the constructor (so no operations on a not completely initialized object happen)

NS_WARNING_PUSH()
NS_WARNING_DISABLE_MSVC(4355)

#ifndef NS_THREAD_CLASS_ENTRY_POINT
#  error "Definition for nsThreadClassEntryPoint is missing on this platform!"
#endif

NS_THREAD_CLASS_ENTRY_POINT;

struct nsThreadEvent
{
  enum class Type
  {
    ThreadCreated,     ///< Called on the thread that creates the nsThread instance
    ThreadDestroyed,   ///< Called on the thread that destroys the nsThread instance
    StartingExecution, ///< Called on the thread that executes the nsThread instance
    FinishedExecution, ///< Called on the thread that executes the nsThread instance
  };

  Type m_Type;
  nsThread* m_pThread = nullptr;
};

/// \brief This class is the base class for platform independent long running threads
///
/// Used by deriving from this class and overriding the Run() method.
class NS_FOUNDATION_DLL nsThread : public nsOSThread
{
public:
  /// \brief Returns the current nsThread if the current platform thread is an nsThread. Returns nullptr otherwise.
  static const nsThread* GetCurrentThread();

  /// \brief Describes the thread status
  enum nsThreadStatus
  {
    Created = 0,
    Running,
    Finished
  };

  /// \brief Initializes the runnable class
  nsThread(nsStringView sName = "nsThread", nsUInt32 uiStackSize = 128 * 1024);

  /// \brief Destructor checks if the thread is deleted while still running, which is not allowed as this is a data hazard
  virtual ~nsThread();

  /// \brief Returns the thread status
  inline nsThreadStatus GetThreadStatus() const { return m_ThreadStatus; }

  /// \brief Helper function to determine if the thread is running
  inline bool IsRunning() const { return m_ThreadStatus == Running; }

  /// \brief Returns the thread name
  inline const char* GetThreadName() const { return m_sName.GetData(); }

  /// \brief These events inform about threads starting and finishing.
  ///
  /// The events are raised on the executing thread! That means thread-specific code may be executed during the event callback,
  /// e.g. to set up thread-local functionality.
  static nsEvent<const nsThreadEvent&, nsMutex> s_ThreadEvents;

private:
  /// \brief The run function can be used to implement a long running task in a thread in a platform independent way
  virtual nsUInt32 Run() = 0;


  volatile nsThreadStatus m_ThreadStatus = Created;

  nsString m_sName;

  friend nsUInt32 RunThread(nsThread* pThread);
};

NS_WARNING_POP()

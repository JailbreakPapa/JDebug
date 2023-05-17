#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Threading/Implementation/ThreadingDeclarations.h>
#include <Foundation/Threading/Mutex.h>

/// \brief Condition variables are used to put threads to sleep and wake them up upon certain events
///
/// The wdConditionVariable works in conjunction with a mutex. When waiting for a signal,
/// the OS typically puts the waiting thread to sleep.
/// Using SignalOne() or SignalAll() other threads can wake up one or all threads that are
/// currently waiting on the condition variable.
///
/// When a thread is woken up, it automatically holds the lock on the condition variable's mutex,
/// which can be used to safely access or modify certain state.
///
/// wdConditionVariable is a low-level threading construct. Higher level functionality such as
/// wdThreadSignal may be more suitable for most use cases.
///
/// \sa wdThreadSignal
class WD_FOUNDATION_DLL wdConditionVariable
{
  WD_DISALLOW_COPY_AND_ASSIGN(wdConditionVariable);

public:
  enum class WaitResult
  {
    Signaled,
    Timeout
  };

  wdConditionVariable();
  ~wdConditionVariable();

  /// \brief Locks the internal mutex. Recursive locking is allowed.
  void Lock();

  /// \brief Tries to lock the internal mutex. Recursive locking is allowed.
  wdResult TryLock();

  /// \brief Unlocks the internal mutex. Must be called as often as it was locked.
  void Unlock();

  /// \brief Wakes up one of the threads that are currently waiting for the variable.
  ///
  /// If no thread is currently waiting, this has no effect.
  void SignalOne();

  /// \brief Wakes up all the threads that are currently waiting for the variable.
  ///
  /// If no thread is currently waiting, this has no effect.
  void SignalAll();

  /// \brief Puts the calling thread to sleep and waits for the variable to get signaled.
  ///
  /// Asserts that the wdConditionVariable is locked when the function is called.
  /// The mutex will be unlocked and the thread is put to sleep.
  /// When the signal arrives, the thread is woken up and the mutex is locked again.
  void UnlockWaitForSignalAndLock() const;

  /// \brief Same as UnlockWaitForSignalAndLock() but with an additional timeout condition.
  ///
  /// If the timeout is reached before the signal arrived, the function returns with
  /// WaitResult::Timeout.
  ///
  /// \note If the timeout is reached, the mutex will still get locked!
  WaitResult UnlockWaitForSignalAndLock(wdTime timeout) const;

private:
  mutable wdInt32 m_iLockCount = 0;
  mutable wdMutex m_Mutex;
  mutable wdConditionVariableData m_Data;
};
